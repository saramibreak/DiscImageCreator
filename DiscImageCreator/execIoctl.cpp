/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "execIoctl.h"
#include "output.h"
#include "outputIoctlLog.h"

BOOL DiskGetMediaTypes(
	PDEVICE pDevice,
	LPCTSTR pszPath
	)
{
	FILE* fp = CreateOrOpenFile(
		pszPath, NULL, NULL, NULL, NULL, _T(".bin"), _T("wb"), 0, 0);
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	DISK_GEOMETRY geom[20] = { 0 };
	DWORD dwReturned = 0;
	BOOL bRet = DeviceIoControl(pDevice->hDevice,
		IOCTL_DISK_GET_MEDIA_TYPES, NULL, 0, &geom, sizeof(geom), &dwReturned, 0);
	if (bRet) {
		OutputFloppyInfo(geom, dwReturned / sizeof(DISK_GEOMETRY));
		bRet = DeviceIoControl(pDevice->hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, 
			NULL, 0, &geom, sizeof(DISK_GEOMETRY), &dwReturned, 0);
		if (bRet) {
			OutputFloppyInfo(geom, 1);
			FlushLog();
			DWORD dwFloppySize = geom[0].Cylinders.LowPart *
				geom[0].TracksPerCylinder * geom[0].SectorsPerTrack * geom[0].BytesPerSector;
			OutputString(_T("Floppy size: %ld byte\n"), dwFloppySize);
			LPBYTE lpBuf = (LPBYTE)calloc(dwFloppySize, sizeof(BYTE));
//			LPVOID lpBuf = VirtualAlloc(NULL, dwFloppySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (!lpBuf) {
				FcloseAndNull(fp);
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			DWORD dwBytesRead = 0;
			SetErrorMode(SEM_FAILCRITICALERRORS);
			bRet = ReadFile(pDevice->hDevice, lpBuf, dwFloppySize, &dwBytesRead, 0);
			OutputString(_T("  Read size: %ld byte\n"), dwBytesRead);
			if (bRet) {
				if (dwFloppySize == dwBytesRead) {
					DWORD dwBytesWrite = fwrite(lpBuf, sizeof(BYTE), dwFloppySize, fp);
					OutputString(_T(" Write size: %ld byte\n"), dwBytesWrite);
				}
				else {
					OutputErrorString(
						_T("Read size is different. Floppy size: %ld, Read size: %ld\n")
						, dwFloppySize, dwBytesRead);
				}
			}
			else {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			}
			FreeAndNull(lpBuf);
//			VirtualFree(lpBuf, 0, MEM_RELEASE);
		}
		else {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		}
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	FcloseAndNull(fp);
	return bRet;
}

BOOL ScsiGetAddress(
	PDEVICE pDevice
	)
{
	DWORD dwReturned = 0;
	BOOL bRet = DeviceIoControl(pDevice->hDevice,
		IOCTL_SCSI_GET_ADDRESS, &pDevice->address, sizeof(SCSI_ADDRESS),
		&pDevice->address, sizeof(SCSI_ADDRESS), &dwReturned, NULL);
	if (bRet) {
		OutputScsiAddress(pDevice);
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	// Because USB drive failed
	return TRUE;
}

BOOL ScsiPassThroughDirect(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPVOID lpCdb,
	BYTE byCdbLength,
	LPVOID pvBuffer,
	DWORD dwBufferLength,
	LPBYTE byScsiStatus,
	LPCTSTR pszFuncName,
	LONG lLineNum
	)
{
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb = { 0 };
	swb.ScsiPassThroughDirect.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	swb.ScsiPassThroughDirect.PathId = pDevice->address.PathId;
	swb.ScsiPassThroughDirect.TargetId = pDevice->address.TargetId;
	swb.ScsiPassThroughDirect.Lun = pDevice->address.Lun;
	swb.ScsiPassThroughDirect.CdbLength = byCdbLength;
	swb.ScsiPassThroughDirect.SenseInfoLength = SENSE_BUFFER_SIZE;
	swb.ScsiPassThroughDirect.DataIn = SCSI_IOCTL_DATA_IN;
	swb.ScsiPassThroughDirect.DataTransferLength = dwBufferLength;
	swb.ScsiPassThroughDirect.TimeOutValue = pDevice->dwTimeOutValue;
	swb.ScsiPassThroughDirect.DataBuffer = pvBuffer;
	swb.ScsiPassThroughDirect.SenseInfoOffset = 
		offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, SenseData);
	memcpy(swb.ScsiPassThroughDirect.Cdb, lpCdb, byCdbLength);

	DWORD dwLength = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
	DWORD dwReturned = 0;
	BOOL bRet = TRUE;
	BOOL bNoSense = FALSE;
	SetLastError(NO_ERROR);
	if (!DeviceIoControl(pDevice->hDevice, IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&swb, dwLength, &swb, dwLength, &dwReturned, NULL)) {
		OutputLastErrorNumAndString(pszFuncName, lLineNum);
		bRet = FALSE;
		if (!pExtArg->byReadContinue || !_tcscmp(_T("SetDiscSpeed"), pszFuncName)) {
			// When semaphore time out occurred, if doesn't execute sleep,
			// UNIT_ATTENSION errors occurs next ScsiPassThroughDirect executing.
			DWORD millisec = 30000;
			OutputErrorString(
				_T("Please wait for %lu milliseconds until the device is returned\n"), millisec);
			Sleep(millisec);
			pDevice->FEATURE.bySetCDSpeed = FALSE;
		}
	}
	else {
		if (swb.SenseData.SenseKey == SCSI_SENSE_NO_SENSE &&
			swb.SenseData.AdditionalSenseCode == SCSI_ADSENSE_NO_SENSE &&
			swb.SenseData.AdditionalSenseCodeQualifier == 0x00) {
			bNoSense = TRUE;
		}
		if (swb.ScsiPassThroughDirect.ScsiStatus >= SCSISTAT_CHECK_CONDITION &&
			!bNoSense) {
			INT nLBA = 0;
			if (swb.ScsiPassThroughDirect.Cdb[0] == 0xa8 ||
				swb.ScsiPassThroughDirect.Cdb[0] == 0xbe ||
				swb.ScsiPassThroughDirect.Cdb[0] == 0xd8) {
				nLBA = (swb.ScsiPassThroughDirect.Cdb[2] << 24)
					+ (swb.ScsiPassThroughDirect.Cdb[3] << 16)
					+ (swb.ScsiPassThroughDirect.Cdb[4] << 8)
					+ swb.ScsiPassThroughDirect.Cdb[5];
			}
			OutputLog(standardError | fileMainError
				, _T("\r"STR_LBA "[F:%s][L:%ld]\n\tOperationCode: %#02x\n")
				, nLBA, nLBA, pszFuncName, lLineNum, swb.ScsiPassThroughDirect.Cdb[0]);
			OutputScsiStatus(swb.ScsiPassThroughDirect.ScsiStatus);
			OutputSenseData(&swb.SenseData);
		}
	}
	if (bNoSense) {
		*byScsiStatus = SCSISTAT_GOOD;
	}
	else {
		*byScsiStatus = swb.ScsiPassThroughDirect.ScsiStatus;
	}
	return bRet;
}

BOOL StorageQueryProperty(
	PDEVICE pDevice,
	LPBOOL lpBusTypeUSB
	)
{
	STORAGE_PROPERTY_QUERY query;
	query.QueryType = PropertyStandardQuery;
	query.PropertyId = StorageAdapterProperty;

	STORAGE_DESCRIPTOR_HEADER header = { 0 };
	DWORD dwReturned = 0;
	if (!DeviceIoControl(pDevice->hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&query, sizeof(STORAGE_PROPERTY_QUERY), &header,
		sizeof(STORAGE_DESCRIPTOR_HEADER), &dwReturned, FALSE)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	PSTORAGE_ADAPTER_DESCRIPTOR adapterDescriptor =
		(PSTORAGE_ADAPTER_DESCRIPTOR)calloc(header.Size, sizeof(BYTE));
	if (!adapterDescriptor) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = DeviceIoControl(pDevice->hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&query, sizeof(STORAGE_PROPERTY_QUERY), adapterDescriptor, header.Size, &dwReturned, FALSE);
	if (bRet) {
		OutputStorageAdaptorDescriptor(adapterDescriptor, lpBusTypeUSB);
		pDevice->dwMaxTransferLength = adapterDescriptor->MaximumTransferLength;
		pDevice->AlignmentMask = (UINT_PTR)(adapterDescriptor->AlignmentMask);
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	FreeAndNull(adapterDescriptor);
	return bRet;
}
