/**
 * Copyright 2011-2018 sarami
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "struct.h"
#include "execIoctl.h"
#include "output.h"
#include "outputIoctlLog.h"

BOOL DiskGetMediaTypes(
	PDEVICE pDevice,
	LPCTSTR pszPath
) {
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
) {
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
) {
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
		if (!pExtArg->byScanProtectViaFile || !_tcscmp(_T("SetDiscSpeed"), pszFuncName)) {
			// When semaphore time out occurred, if doesn't execute sleep,
			// UNIT_ATTENSION errors occurs next ScsiPassThroughDirect executing.
			DWORD milliseconds = 25000;
			OutputErrorString(
				_T("Please wait for %lu milliseconds until the device is returned\n"), milliseconds);
			Sleep(milliseconds);
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
				swb.ScsiPassThroughDirect.Cdb[0] == 0xad ||
				swb.ScsiPassThroughDirect.Cdb[0] == 0xbe ||
				swb.ScsiPassThroughDirect.Cdb[0] == 0xd8) {
				nLBA = (swb.ScsiPassThroughDirect.Cdb[2] << 24)
					+ (swb.ScsiPassThroughDirect.Cdb[3] << 16)
					+ (swb.ScsiPassThroughDirect.Cdb[4] << 8)
					+ swb.ScsiPassThroughDirect.Cdb[5];
			}
			OutputLog(standardError | fileMainError
				, _T("\rLBA[%06d, %#07x]: [F:%s][L:%ld]\n\tOpcode: %#02x\n")
				, nLBA, nLBA, pszFuncName, lLineNum, swb.ScsiPassThroughDirect.Cdb[0]);
			OutputScsiStatus(swb.ScsiPassThroughDirect.ScsiStatus);
			OutputSenseData(&swb.SenseData);
			if (swb.SenseData.SenseKey == SCSI_SENSE_UNIT_ATTENTION) {
				DWORD milliseconds = 40000;
				OutputErrorString(
					_T("Please wait for %lu milliseconds until the device is returned\n"), milliseconds);
				Sleep(milliseconds);
			}
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

// https://support.microsoft.com/ja-jp/help/126369
// https://msdn.microsoft.com/en-us/library/windows/desktop/ff800832(v=vs.85).aspx
BOOL StorageQueryProperty(
	PDEVICE pDevice,
	LPBOOL lpBusTypeUSB
) {
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
		if (adapterDescriptor->MaximumTransferLength > 65536) {
			pDevice->dwMaxTransferLength = 65536;
			OutputDriveLogA("dwMaxTransferLength changed [%lu] -> [%lu]\n"
				, adapterDescriptor->MaximumTransferLength, pDevice->dwMaxTransferLength);
		}
		else {
			pDevice->dwMaxTransferLength = adapterDescriptor->MaximumTransferLength;
		}
		pDevice->AlignmentMask = (UINT_PTR)(adapterDescriptor->AlignmentMask);
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	FreeAndNull(adapterDescriptor);
	return bRet;
}

BOOL SetStreaming(
	PDEVICE pDevice,
	DWORD dwDiscSpeedNum
) {
#if 1
	_declspec(align(4)) CDROM_SET_STREAMING setstreaming;
#endif
#if 0
	CDB::_SET_STREAMING cdb = { 0 };
	cdb.OperationCode = SCSIOP_SET_STREAMING;
	_declspec(align(4)) PERFORMANCE_DESCRIPTOR pd = { 0 };
	//	CHAR pd[28] = { 0 };
	size_t size = sizeof(PERFORMANCE_DESCRIPTOR);
	REVERSE_BYTES_SHORT(&cdb.ParameterListLength, &size);
#endif
#if 1
	setstreaming.RequestType = CdromSetStreaming;
	if (0 < dwDiscSpeedNum && dwDiscSpeedNum <= DVD_DRIVE_MAX_SPEED) {
		setstreaming.ReadSize = DISC_RAW_READ_SIZE * 75 * 9 * dwDiscSpeedNum;
	}
	else {
		setstreaming.ReadSize = DISC_RAW_READ_SIZE * 75 * 9 * DVD_DRIVE_MAX_SPEED;
	}
	setstreaming.ReadTime = 1000;
	setstreaming.WriteSize = setstreaming.ReadSize;
	setstreaming.WriteTime = setstreaming.ReadTime;
	setstreaming.EndLba = 0xffffffff;
	setstreaming.RestoreDefaults = TRUE;
#endif
#if 0
	pd.RestoreDefaults = TRUE;
	pd.Exact = TRUE;
	INT nENDLba = 0x231260;
	REVERSE_BYTES(&pd.EndLba, &nENDLba);
	DWORD dwReadSize = 0;
	if (0 < dwDiscSpeedNum && dwDiscSpeedNum <= DVD_DRIVE_MAX_SPEED) {
		dwReadSize = 1385 * dwDiscSpeedNum;
	}
	else {
		//		dwReadSize = 1385;
	}
	REVERSE_BYTES(&pd.ReadSize, &dwReadSize);
	DWORD dwReadTime = 1000;
	REVERSE_BYTES(&pd.ReadTime, &dwReadTime);
	dwReadSize = 1385;
	REVERSE_BYTES(&pd.WriteSize, &dwReadSize);
	REVERSE_BYTES(&pd.WriteTime, &dwReadTime);
#endif
	DWORD dwReturned = 0;
	if (!DeviceIoControl(pDevice->hDevice, IOCTL_CDROM_SET_SPEED
		, &setstreaming, sizeof(setstreaming), NULL, 0, &dwReturned, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	else {
		OutputString(_T("Set the drive speed: %luKB/sec\n"), setstreaming.ReadSize / 1000);
	}
	return TRUE;
}

BOOL DvdStartSession(
	PDEVICE pDevice,
	PDVD_COPY_PROTECT_KEY dvdKey
) {
	DWORD dwReturned = 0;
	if (dvdKey->SessionId != DVD_END_ALL_SESSIONS) {
		if (!DeviceIoControl(pDevice->hDevice, IOCTL_DVD_END_SESSION
			, &dvdKey->SessionId, sizeof(dvdKey->SessionId), NULL, 0, &dwReturned, NULL)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		dvdKey->SessionId = DVD_END_ALL_SESSIONS;
	}

	if (!DeviceIoControl(pDevice->hDevice, IOCTL_DVD_START_SESSION, NULL
		, 0, &dvdKey->SessionId, sizeof(dvdKey->SessionId), &dwReturned, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	return TRUE;
}

BOOL ReadKey(
	PDEVICE pDevice,
	PDVD_COPY_PROTECT_KEY dvdKey
) {
	DWORD dwReturned = 0;
	if (!DeviceIoControl(pDevice->hDevice, IOCTL_DVD_READ_KEY, dvdKey
		, dvdKey->KeyLength, dvdKey, dvdKey->KeyLength, &dwReturned, 0)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	else {
		if (dvdKey->KeyType == DvdChallengeKey) {
			OutputDriveLogA("ChallengeKey: ");
			for (INT i = 4; i < 16; i++) {
				OutputDriveLogA("%02x", dvdKey->KeyData[i]);
			}
			OutputDriveLogA("\n");
		}
		else if (dvdKey->KeyType == DvdBusKey1) {
			OutputDriveLogA("BusKey1: %02x%02x%02x%02x%02x\n", dvdKey->KeyData[0]
				, dvdKey->KeyData[1], dvdKey->KeyData[2], dvdKey->KeyData[3], dvdKey->KeyData[4]);
		}
		else if (dvdKey->KeyType == DvdTitleKey) {
			INT cpm = dvdKey->KeyData[0] >> 7 & 0x01;
			INT cpsec = dvdKey->KeyData[0] >> 6 & 0x01;
			OutputDriveLogA(
				"CopyrightManagement: %s\n"
				"CopyrightSector: %s\n"
				, BOOLEAN_TO_STRING_YES_NO_A(cpm)
				, BOOLEAN_TO_STRING_YES_NO_A(cpsec));
			switch (dvdKey->KeyData[0] >> 4 & 0x03) {
			case 0:
				OutputDriveLogA("CopyingIsPermittedWithoutRestriction\n");
				break;
			case 1:
				OutputDriveLogA("Reserved\n");
				break;
			case 2:
				OutputDriveLogA("OneGenerationOfCopiesMayBeMade\n");
				break;
			case 3:
				OutputDriveLogA("NoCopyingIsAllowed\n");
				break;
			}
			switch (dvdKey->KeyData[0] & 0x0f) {
			case 0:
				OutputDriveLogA("CSS\n");
				break;
			case 1:
				OutputDriveLogA("CPPM\n");
				break;
			}
			OutputDriveLogA("TitleKey: %02x%02x%02x%02x%02x\n", dvdKey->KeyData[1]
				, dvdKey->KeyData[2], dvdKey->KeyData[3], dvdKey->KeyData[4], dvdKey->KeyData[5]);
		}
		else if (dvdKey->KeyType == DvdAsf) {
			INT asf = dvdKey->KeyData[3] & 0x01;
			OutputDriveLogA("AuthenticationSusscess: %s\n", BOOLEAN_TO_STRING_YES_NO_A(asf));
		}
	}
	return TRUE;
}

BOOL SendKey(
	PDEVICE pDevice,
	PDVD_COPY_PROTECT_KEY dvdKey
) {
	DWORD dwReturned = 0;
	if (!DeviceIoControl(pDevice->hDevice,
		IOCTL_DVD_SEND_KEY, dvdKey, dvdKey->KeyLength, NULL, 0, &dwReturned, 0)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	else {
		if (dvdKey->KeyType == DvdChallengeKey) {
			OutputDriveLogA("ChallengeKey: ");
			for (INT i = 4; i < 16; i++) {
				OutputDriveLogA("%02x", dvdKey->KeyData[i]);
			}
			OutputDriveLogA("\n");
		}
		else if (dvdKey->KeyType == DvdBusKey2) {
			OutputDriveLogA("BusKey2: %02x%02x%02x%02x%02x\n", dvdKey->KeyData[0]
				, dvdKey->KeyData[1], dvdKey->KeyData[2], dvdKey->KeyData[3], dvdKey->KeyData[4]);
		}
	}
	return TRUE;
}
