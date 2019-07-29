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

// ref: http://www.ioctls.net/
BOOL DiskGetMediaTypes(
	PDEVICE pDevice,
	PDWORD64 pDiskSize,
	LPDWORD pBytePerSector
) {
	DISK_GEOMETRY geom[20] = {};
	DWORD dwReturned = 0;
	BOOL bRet = DeviceIoControl(pDevice->hDevice,
		IOCTL_DISK_GET_MEDIA_TYPES, NULL, 0, &geom, sizeof(geom), &dwReturned, 0);
	if (bRet) {
		OutputDiskGeometry(geom, dwReturned / sizeof(DISK_GEOMETRY));
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	bRet = DeviceIoControl(pDevice->hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY,
		NULL, 0, &geom, sizeof(DISK_GEOMETRY), &dwReturned, 0);
	if (bRet) {
		OutputDiskGeometry(geom, 1);
		*pDiskSize = geom[0].Cylinders.u.LowPart *
			geom[0].TracksPerCylinder * geom[0].SectorsPerTrack * geom[0].BytesPerSector;
		*pBytePerSector = geom[0].BytesPerSector;
	}
	return bRet;
}

BOOL StorageGetMediaTypesEx(
	PDEVICE pDevice,
	PDWORD64 pDiskSize,
	LPDWORD pBytePerSector
) {
	GET_MEDIA_TYPES mediaTypes = {};
	DWORD dwReturned = 0;
	BOOL bRet = DeviceIoControl(pDevice->hDevice,
		IOCTL_STORAGE_GET_MEDIA_TYPES_EX, NULL, 0, &mediaTypes, sizeof(mediaTypes), &dwReturned, 0);
	if (bRet) {
		OutputRemovableDiskInfo(&mediaTypes);
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	DISK_GEOMETRY_EX geom = {};
	bRet = DeviceIoControl(pDevice->hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
		NULL, 0, &geom, sizeof(DISK_GEOMETRY_EX), &dwReturned, 0);
	if (bRet) {
		OutputDiskGeometryEx(&geom);
		*pDiskSize = (DWORD64)geom.DiskSize.QuadPart;
		*pBytePerSector = geom.Geometry.BytesPerSector;
	}
	return bRet;
}

BOOL ReadDirectoryRecord(
#ifdef _WIN32
	HANDLE handle,
#else
	int handle,
#endif
	LONG seekPos,
	DWORD dwBytesPerSector,
	PFAT pFat,
	LPBYTE pTab
) {
	LPBYTE lpBuf = (LPBYTE)calloc(dwBytesPerSector, sizeof(BYTE));
	if (!lpBuf) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	SetFilePointer(handle, seekPos, NULL, FILE_BEGIN);
	BOOL bRet = FALSE;
	DWORD dwBytesRead = 0;
	UINT cnt = pFat->RootEntCnt;
	if (cnt == 0) {
		cnt = (UINT)dwBytesPerSector;
	}
	BOOL bRoop = TRUE;
	while (bRoop) {
		bRet = ReadFile(handle, lpBuf, dwBytesPerSector, &dwBytesRead, 0);
		if (bRet) {
			if (dwBytesPerSector == dwBytesRead) {
				for (UINT i = 0; i < cnt; i += 32) {
					if (lpBuf[i] == 0) {
						bRoop = FALSE;
						break;
					}
					else if (lpBuf[i] == 0xe5) {
						OutputDiscLogA("%sDeteled Entry\n", (LPCH)&pTab[0]);
					}
					else if ((lpBuf[11 + i] & 0x0f) == 0x0f) {
						WCHAR fname[_MAX_FNAME] = {};
						OutputDiscLogA(
							"%s        LDIR_Ord: ", (LPCH)&pTab[0]);
						if ((lpBuf[i] & 0x40) == 0x40) {
							INT nCnt = (lpBuf[i] & 0x1f) - 1;
							for (INT h = 0, j = 32 * nCnt, k = 0; h <= nCnt; h++, j -= 32, k += 13) {
								memcpy(fname + k, (LPWCH)&lpBuf[1 + i + j], 10);
								memcpy(fname + 5 + k, (LPWCH)&lpBuf[14 + i + j], 12);
								memcpy(fname + 11 + k, (LPWCH)&lpBuf[28 + i + j], 4);
								OutputDiscLogA("0x%02x ", lpBuf[i + j]);
							}
							OutputDiscLogA("\n");
							i += 32 * ((lpBuf[i] & 0x0f) - 1);
						}
						else {
							OutputDiscLogA("%#02x\n", lpBuf[i]);
						}
						OutputDiscLogA(
							"%s       LDIR_Name: %ls\n"
							"%s       LDIR_Attr: 0x%02x\n"
							"%s       LDIR_Type: 0x%02x\n"
							"%s     LDIR_Chksum: 0x%02x\n"
							"%s  LDIR_FstClusLO: %d\n\n" 
							, (LPCH)&pTab[0], fname
							, (LPCH)&pTab[0], lpBuf[11 + i]
							, (LPCH)&pTab[0], lpBuf[12 + i]
							, (LPCH)&pTab[0], lpBuf[13 + i]
							, (LPCH)&pTab[0], lpBuf[26 + i]
						);
					}
					else {
						WORD FstClusLO = MAKEWORD(lpBuf[26 + i], lpBuf[27 + i]);
						OutputDiscLogA(
							"%s        DIR_Name: %.11s\n"
							"%s        DIR_Attr: 0x%02x\n"
							"%s       DIR_NTRes: %d\n"
							"%sDIR_CrtTimeTenth: %d\n"
							"%s     DIR_CrtTime: %02d:%02d:%02d\n"
							"%s     DIR_CrtDate: %04d/%02d/%02d\n"
							"%s  DIR_LstAccDate: %04d/%02d/%02d\n"
							"%s   DIR_FstClusHI: %d\n"
							"%s     DIR_WrtTime: %02d:%02d:%02d\n"
							"%s     DIR_WrtDate: %04d/%02d/%02d\n"
							"%s   DIR_FstClusLO: %d\n"
							"%s    DIR_FileSize: %d\n\n"
							, (LPCH)&pTab[0], (LPCH)&lpBuf[i]
							, (LPCH)&pTab[0], lpBuf[11 + i]
							, (LPCH)&pTab[0], lpBuf[12 + i]
							, (LPCH)&pTab[0], lpBuf[13 + i]
							, (LPCH)&pTab[0], ((lpBuf[15 + i] >> 3) & 0x1f), ((lpBuf[15 + i] << 3) & 0x38) | ((lpBuf[14 + i] >> 5) & 0x07), (lpBuf[14 + i] & 0x1f) / 2
							, (LPCH)&pTab[0], ((lpBuf[17 + i] >> 1) & 0x7f) + 1980, ((lpBuf[17 + i] << 3) & 0x08) | ((lpBuf[16 + i] >> 5) & 0x07), lpBuf[16 + i] & 0x1f
							, (LPCH)&pTab[0], ((lpBuf[19 + i] >> 1) & 0x7f) + 1980, ((lpBuf[19 + i] << 3) & 0x08) | ((lpBuf[18 + i] >> 5) & 0x07), lpBuf[18 + i] & 0x1f
							, (LPCH)&pTab[0], MAKEWORD(lpBuf[20 + i], lpBuf[21 + i])
							, (LPCH)&pTab[0], ((lpBuf[23 + i] >> 3) & 0x1f), ((lpBuf[23 + i] << 3) & 0x38) | ((lpBuf[22 + i] >> 5) & 0x07), (lpBuf[22 + i] & 0x1f) / 2
							, (LPCH)&pTab[0], ((lpBuf[25 + i] >> 1) & 0x7f) + 1980, ((lpBuf[25 + i] << 3) & 0x08) | ((lpBuf[24 + i] >> 5) & 0x07), lpBuf[24 + i] & 0x1f
							, (LPCH)&pTab[0], FstClusLO
							, (LPCH)&pTab[0], MAKEUINT(MAKEWORD(lpBuf[28 + i], lpBuf[29 + i]), MAKEWORD(lpBuf[30 + i], lpBuf[31 + i]))
						);
						if (FstClusLO != 0 && (lpBuf[11 + i] & 0x10) == 0x10 && lpBuf[i] != '.') {
							LONG seekPosNext = (LONG)((pFat->DataStartSector + (FstClusLO - 2) * pFat->SecPerClus) * dwBytesPerSector);
							size_t idx = strlen((LPCH)&pTab[0]);
							pTab[idx] = '\t';
							OutputDiscLogA("%s" OUTPUT_DHYPHEN_PLUS_STR(DirectoryEntry), (LPCH)&pTab[0]);
							ReadDirectoryRecord(handle, (LONG)seekPosNext, dwBytesPerSector, pFat, pTab);
							pTab[idx] = 0;
						}
					}
				}
			}
			else {
				OutputErrorString(
					_T("Read size is different. NumberOfBytesToRead: %ld, NumberOfBytesRead: %ld\n")
					, dwBytesPerSector, dwBytesRead);
			}
		}
		else {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		}
	}
	FreeAndNull(lpBuf);
	return bRet;
}

BOOL ReadFileAllocationTable(
#ifdef _WIN32
	HANDLE handle,
#else
	int handle,
#endif
	LPBYTE lpBuf,
	DWORD dwBytesPerSector
) {
	DWORD dwBytesRead = 0;
	FAT fat = {};
	BOOL bRet = ReadFile(handle, lpBuf, dwBytesPerSector, &dwBytesRead, 0);
	if (bRet) {
		if (dwBytesPerSector == dwBytesRead) {
			OutputFileAllocationTable(lpBuf, &fat);
		}
		else {
			OutputErrorString(
				_T("Read size is different. NumberOfBytesToRead: %ld, NumberOfBytesRead: %ld\n")
				, dwBytesPerSector, dwBytesRead);
		}
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	BYTE szTab[256] = {};
	szTab[0] = '\t';
	OutputDiscLogA("%s" OUTPUT_DHYPHEN_PLUS_STR(DirectoryEntry), szTab);
	ReadDirectoryRecord(handle, (LONG)(fat.RootDirStartSector * dwBytesPerSector), dwBytesPerSector, &fat, szTab);
	SetFilePointer(handle, 0, NULL, FILE_BEGIN);
	return bRet;
}

BOOL ReadDisk(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	LPCTSTR pszPath
) {
	FILE* fp = CreateOrOpenFile(
		pszPath, NULL, NULL, NULL, NULL, _T(".bin"), _T("wb"), 0, 0);
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = FALSE;
	DWORD64 dwDiskSize = 0;
	DWORD dwBytesPerSector = 0;
	if (*pExecType == fd) {
		bRet = DiskGetMediaTypes(pDevice, &dwDiskSize, &dwBytesPerSector);
	}
	else if (*pExecType == disk) {
		bRet = StorageGetMediaTypesEx(pDevice, &dwDiskSize, &dwBytesPerSector);
	}
	if (bRet) {
		DWORD dwReadSize = (DWORD)dwDiskSize;
		DWORD dwBlkSize = (DWORD)(dwDiskSize / dwBytesPerSector);
		DWORD dwRoopCnt = 1;
		DWORD dwRoopCntPlus = 0;
		DWORD coef = pDevice->dwMaxTransferLength / dwBytesPerSector;
		OutputString(
			_T("DiskSize: %lld bytes, BytesPerSector: %ld, BlockSize: %ld\n"), dwDiskSize, dwBytesPerSector, dwBlkSize);
		if (*pExecType == disk) {
			dwReadSize = dwBytesPerSector * coef;
			dwRoopCnt = dwBlkSize / coef;
			dwRoopCntPlus = dwBlkSize % coef;
		}
		LPBYTE lpBuf = (LPBYTE)calloc(dwReadSize, sizeof(BYTE));
		if (!lpBuf) {
			FcloseAndNull(fp);
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		ReadFileAllocationTable(pDevice->hDevice, lpBuf, dwBytesPerSector);
		FlushLog();

		DWORD dwBytesRead = 0;
		for (DWORD i = 0; i < dwRoopCnt; i++) {
			// unassigned partition can't read.
			bRet = ReadFile(pDevice->hDevice, lpBuf, dwReadSize, &dwBytesRead, 0);
			if (bRet) {
				if (dwReadSize == dwBytesRead) {
					fwrite(lpBuf, sizeof(BYTE), (size_t)dwReadSize, fp);
				}
				else {
					OutputErrorString(
						_T("[%ld] Read size is different. NumberOfBytesToRead: %ld, NumberOfBytesRead: %ld\n")
						, dwRoopCnt * coef + i, dwReadSize, dwBytesRead);
				}
			}
			else {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				break;
			}
			if (*pExecType == disk) {
				OutputString(_T("\rCreating .bin (Blocks) %6ld/%6ld"), (i + 1) * coef, dwBlkSize);
			}
		}
		for (DWORD i = 0; i < dwRoopCntPlus; i++) {
			bRet = ReadFile(pDevice->hDevice, lpBuf, dwBytesPerSector, &dwBytesRead, 0);
			if (bRet) {
				if (dwBytesPerSector == dwBytesRead) {
					fwrite(lpBuf, sizeof(BYTE), (size_t)dwBytesPerSector, fp);
				}
				else {
					OutputErrorString(
						_T("[%ld] Read size is different. NumberOfBytesToRead: %ld, NumberOfBytesRead: %ld\n")
						, dwRoopCnt * coef + i, dwBytesPerSector, dwBytesRead);
				}
			}
			else {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				break;
			}
			OutputString(_T("\rCreating .bin (Blocks) %6ld/%6ld"), dwRoopCnt * coef + i + 1, dwBlkSize);
		}
		OutputString(_T("\n"));
		FreeAndNull(lpBuf);
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	FcloseAndNull(fp);
	return bRet;
}

BOOL DVDGetRegion(
	PDEVICE pDevice
) {
	DVD_REGION dvdRegion = {};
#ifdef _WIN32
	DWORD dwReturned = 0;
	BOOL bRet = DeviceIoControl(pDevice->hDevice,
		IOCTL_DVD_GET_REGION, &dvdRegion, sizeof(DVD_REGION),
		&dvdRegion, sizeof(DVD_REGION), &dwReturned, NULL);
#else
	dvd_authinfo auth_info;

	memset(&auth_info, 0, sizeof(auth_info));
	auth_info.type = DVD_LU_SEND_RPC_STATE;

	int bRet = ioctl(pDevice->hDevice, DVD_AUTH, &auth_info);
	dvdRegion.SystemRegion = auth_info.lrpcs.region_mask;
	dvdRegion.ResetCount = auth_info.lrpcs.type;
#endif
	if (bRet) {
		OutputDVDGetRegion(&dvdRegion);
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
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
	INT nDataDirection,
	DWORD dwBufferLength,
	LPBYTE byScsiStatus,
	LPCTSTR pszFuncName,
	LONG lLineNum
) {
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb = {};
#ifdef _WIN32
	swb.ScsiPassThroughDirect.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	swb.ScsiPassThroughDirect.PathId = pDevice->address.PathId;
	swb.ScsiPassThroughDirect.TargetId = pDevice->address.TargetId;
	swb.ScsiPassThroughDirect.Lun = pDevice->address.Lun;
	swb.ScsiPassThroughDirect.CdbLength = byCdbLength;
	swb.ScsiPassThroughDirect.SenseInfoLength = SENSE_BUFFER_SIZE;
	swb.ScsiPassThroughDirect.DataIn = (UCHAR)nDataDirection;
	swb.ScsiPassThroughDirect.DataTransferLength = dwBufferLength;
	swb.ScsiPassThroughDirect.TimeOutValue = pDevice->dwTimeOutValue;
	swb.ScsiPassThroughDirect.DataBuffer = pvBuffer;
	swb.ScsiPassThroughDirect.SenseInfoOffset = 
		offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, SenseData);
	memcpy(swb.ScsiPassThroughDirect.Cdb, lpCdb, byCdbLength);
#else
	swb.io_hdr.interface_id = 'S';
	swb.io_hdr.dxfer_direction = nDataDirection;
	swb.io_hdr.cmd_len = byCdbLength;
	swb.io_hdr.mx_sb_len = sizeof(swb.Dummy);
	swb.io_hdr.dxfer_len = (unsigned int)dwBufferLength;
	swb.io_hdr.dxferp = pvBuffer;
	swb.io_hdr.cmdp = (unsigned char *)lpCdb;
	swb.io_hdr.sbp = swb.Dummy;
	swb.io_hdr.timeout = (unsigned int)pDevice->dwTimeOutValue;
//	swb.io_hdr.flags = SG_FLAG_DIRECT_IO;
#endif
	DWORD dwLength = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
	DWORD dwReturned = 0;
	BOOL bRet = TRUE;
	BOOL bNoSense = FALSE;
	SetLastError(NO_ERROR);
	if (!DeviceIoControl(pDevice->hDevice
		, IOCTL_SCSI_PASS_THROUGH_DIRECT, &swb, dwLength, &swb, dwLength, &dwReturned, NULL)) {
		OutputLastErrorNumAndString(pszFuncName, lLineNum);
		bRet = FALSE;
		if (!pExtArg->byScanProtectViaFile && /*!_tcscmp(_T("SetDiscSpeed"), pszFuncName) &&*/
			!pExtArg->byMultiSession) {
			// When semaphore time out occurred, if doesn't execute sleep,
			// UNIT_ATTENSION errors occurs next ScsiPassThroughDirect executing.
			UINT milliseconds = 25000;
			OutputErrorString(
				_T("Please wait for %u milliseconds until the device is returned\n"), milliseconds);
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
#ifdef _WIN32
		if (swb.ScsiPassThroughDirect.ScsiStatus >= SCSISTAT_CHECK_CONDITION && !bNoSense) {
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
#else
		if (swb.io_hdr.status >= SCSISTAT_CHECK_CONDITION && !bNoSense) {
			INT nLBA = 0;
			if (swb.io_hdr.cmdp[0] == 0xa8 ||
				swb.io_hdr.cmdp[0] == 0xad ||
				swb.io_hdr.cmdp[0] == 0xbe ||
				swb.io_hdr.cmdp[0] == 0xd8) {
				nLBA = (swb.io_hdr.cmdp[2] << 24)
					+ (swb.io_hdr.cmdp[3] << 16)
					+ (swb.io_hdr.cmdp[4] << 8)
					+ swb.io_hdr.cmdp[5];
			}
			OutputLog(standardError | fileMainError
				, _T("\rLBA[%06d, %#07x]: [F:%s][L:%ld]\n\tOpcode: %#02x\n")
				, nLBA, nLBA, pszFuncName, lLineNum, swb.io_hdr.cmdp[0]);
			OutputScsiStatus(swb.io_hdr.status);
#endif
			OutputSenseData(&swb.SenseData);
			if (swb.SenseData.SenseKey == SCSI_SENSE_UNIT_ATTENTION) {
				UINT milliseconds = 40000;
				OutputErrorString(
					_T("Please wait for %u milliseconds until the device is returned\n"), milliseconds);
				Sleep(milliseconds);
			}
		}
	}
	if (bNoSense) {
		*byScsiStatus = SCSISTAT_GOOD;
	}
	else {
#ifdef _WIN32
		*byScsiStatus = swb.ScsiPassThroughDirect.ScsiStatus;
#else
		*byScsiStatus = swb.io_hdr.status;
#endif
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

	STORAGE_DESCRIPTOR_HEADER header = {};
	DWORD dwReturned = 0;
	if (!DeviceIoControl(pDevice->hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &query
		, sizeof(STORAGE_PROPERTY_QUERY), &header, sizeof(STORAGE_DESCRIPTOR_HEADER), &dwReturned, FALSE)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE tmp = (LPBYTE)calloc(header.Size, sizeof(BYTE));
	if (!tmp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	PSTORAGE_ADAPTER_DESCRIPTOR adapterDescriptor = (PSTORAGE_ADAPTER_DESCRIPTOR)tmp;
	BOOL bRet = DeviceIoControl(pDevice->hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &query
		, sizeof(STORAGE_PROPERTY_QUERY), adapterDescriptor, header.Size, &dwReturned, FALSE);
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
#if 0
BOOL SetStreaming(
	PDEVICE pDevice,
	DWORD dwDiscSpeedNum
) {
#if 1
	_declspec(align(4)) CDROM_SET_STREAMING setstreaming;
#endif
#if 0
	CDB::_SET_STREAMING cdb = {};
	cdb.OperationCode = SCSIOP_SET_STREAMING;
	_declspec(align(4)) PERFORMANCE_DESCRIPTOR pd = {};
	//	CHAR pd[28] = {};
	size_t size = sizeof(PERFORMANCE_DESCRIPTOR);
	REVERSE_BYTES_SHORT(&cdb.ParameterListLength, &size);
#endif
#if 1
	setstreaming.RequestType = CdromSetStreaming;
	if (0 < dwDiscSpeedNum && dwDiscSpeedNum <= DVD_DRIVE_MAX_SPEED) {
		setstreaming.ReadSize = 1385 * dwDiscSpeedNum;
	}
	else {
		setstreaming.ReadSize = 1385 * DVD_DRIVE_MAX_SPEED;
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
		OutputString(_T("Set the drive speed: %luKB/sec\n"), setstreaming.ReadSize);
		OutputString(_T("dwReturned: %lu\n"), dwReturned);
	}
	return TRUE;
}
#endif
