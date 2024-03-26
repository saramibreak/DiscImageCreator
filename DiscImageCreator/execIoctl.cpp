/**
 * Copyright 2011-2024 sarami
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
#include "check.h"
#include "execIoctl.h"
#include "get.h"
#include "output.h"
#include "outputIoctlLog.h"
#include "outputFileSystem.h"

// ref: http://www.ioctls.net/
BOOL DiskGetMediaTypes(
	PDEVICE pDevice,
	PDISC pDisc,
	PDWORD64 pDiskSize
) {
	DISK_GEOMETRY geom[20] = {};
	DWORD dwReturned = 0;
	BOOL bRet = DeviceIoControl(pDevice->hDevice,
		IOCTL_DISK_GET_MEDIA_TYPES, NULL, 0, &geom, sizeof(geom), &dwReturned, 0);
	if (bRet) {
		OutputDiscLog(
			OUTPUT_DHYPHEN_PLUS_STR("DISK_GEOMETRY")
			"SupportedMediaType\n");
		OutputDiskGeometry(geom, dwReturned / sizeof(DISK_GEOMETRY));
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	bRet = DeviceIoControl(pDevice->hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY,
		NULL, 0, &geom, sizeof(DISK_GEOMETRY), &dwReturned, 0);
	if (bRet) {
		OutputDiscLog("CurrentMediaType\n");
		OutputDiskGeometry(geom, 1);
		*pDiskSize = geom[0].Cylinders.u.LowPart *
			geom[0].TracksPerCylinder * geom[0].SectorsPerTrack * geom[0].BytesPerSector;
		pDisc->dwBytesPerSector = geom[0].BytesPerSector;
	}
	return bRet;
}

BOOL StorageGetMediaTypesEx(
	PDEVICE pDevice,
	PDISC pDisc,
	PDWORD64 pDiskSize
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

	size_t geomSize = sizeof(DISK_GEOMETRY_EX) + sizeof(DISK_PARTITION_INFO) + sizeof(DISK_DETECTION_INFO);
	LPBYTE lpBuf = (LPBYTE)calloc(geomSize, sizeof(BYTE));
	if (!lpBuf) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	bRet = DeviceIoControl(pDevice->hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
		NULL, 0, lpBuf, (DWORD)geomSize, &dwReturned, 0);
	if (bRet) {
		PDISK_GEOMETRY_EX pGeom = (PDISK_GEOMETRY_EX)lpBuf;
//		OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("DISK_GEOMETRY"));
//		OutputDiskGeometry((PDISK_GEOMETRY)lpBuf, 1);
		OutputDiskGeometryEx(pGeom);
		*pDiskSize = (DWORD64)pGeom->DiskSize.QuadPart;
		pDisc->dwBytesPerSector = pGeom->Geometry.BytesPerSector;
	}
	return bRet;
}

BOOL Read10(
	PEXT_ARG pExtArg, 
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR szFnameAndExt,
	LPBYTE lpBuf,
	DWORD dwBlkSize,
	FILE* fp,
	PHASH pHash
) {
	CDB::_CDB10 cdb = {};
	cdb.OperationCode = SCSIOP_READ;
	DWORD dwTransferLen = pDevice->dwMaxTransferLength / pDisc->dwBytesPerSector;
	if (dwTransferLen > 0xffff) {
		dwTransferLen = 0xffff;
	}
	cdb.TransferBlocksMsb = (UCHAR)(dwTransferLen >> 8);
	cdb.TransferBlocksLsb = (UCHAR)dwTransferLen;

#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	CalcInit(pExtArg, &pHash->pHashChunk[pHash->uiIndex]);

	for (DWORD dwLBA = 0; dwLBA < dwBlkSize; dwLBA += dwTransferLen) {
		if (dwTransferLen > (DWORD)(dwBlkSize - dwLBA)) {
			dwTransferLen = (DWORD)(dwBlkSize - dwLBA);
			cdb.TransferBlocksMsb = (UCHAR)(dwTransferLen >> 8);
			cdb.TransferBlocksLsb = (UCHAR)dwTransferLen;
		}
		cdb.LogicalBlockByte0 = (UCHAR)(dwLBA >> 24);
		cdb.LogicalBlockByte1 = (UCHAR)(dwLBA >> 16);
		cdb.LogicalBlockByte2 = (UCHAR)(dwLBA >> 8);
		cdb.LogicalBlockByte3 = (UCHAR)dwLBA;
		if (!ScsiPassThroughDirect(NULL, pDevice, &cdb, CDB10GENERIC_LENGTH, lpBuf,
			direction, pDisc->dwBytesPerSector * dwTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			if (GetLastError() == 87) {
				OutputString("Change the transfer length: %lu -> ", dwTransferLen);
				dwTransferLen--;
				OutputString("%lu\n", dwTransferLen);
				cdb.TransferBlocksMsb = (UCHAR)(dwTransferLen >> 8);
				cdb.TransferBlocksLsb = (UCHAR)dwTransferLen;
				dwLBA -= dwTransferLen;
				continue;
			}
			return FALSE;
		}
		WriteBufWithCalc(pExtArg, lpBuf, pDisc->dwBytesPerSector, dwTransferLen, fp, pHash);
		OutputString("\rCreating bin (Block) %lu/%lu", dwLBA + dwTransferLen, dwBlkSize);
	}
	_tcsncpy(pHash->pHashChunk[pHash->uiIndex].szFnameAndExt, szFnameAndExt, _MAX_FNAME + _MAX_EXT - 1);
	pHash->pHashChunk[pHash->uiIndex].ui64FileSize = (UINT64)(dwBlkSize * pDisc->dwBytesPerSector);
	OutputString("\n");
	return TRUE;
}

BOOL ReadFATDirectoryRecord(
#ifdef _WIN32
	HANDLE handle,
#elif __linux__
	int handle,
#elif __MACH__
	SCSITaskInterface** handle,
#endif
	LARGE_INTEGER seekPos,
	DWORD dwBytesPerSector,
	PFAT pFat,
	_TCHAR* pTab
) {
	DWORD dwReadSize = dwBytesPerSector * pFat->SecPerClus;
	LPBYTE lpBuf = (LPBYTE)calloc(dwReadSize, sizeof(BYTE));
	if (!lpBuf) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	SetFilePointerEx(handle, seekPos, NULL, FILE_BEGIN);
	BOOL bRet = FALSE;
	DWORD dwBytesRead = 0;
	UINT cnt = pFat->RootEntCnt;
	if (cnt == 0) {
		cnt = (UINT)dwReadSize;
	}
	BOOL bRoop = TRUE;
	while (bRoop) {
		bRet = ReadFile(handle, lpBuf, dwReadSize, &dwBytesRead, 0);
		if (bRet) {
			if (dwReadSize == dwBytesRead) {
				for (UINT i = 0; i <= cnt; i += 32) {
					if (lpBuf[i] == 0 || i == cnt) {
						bRoop = FALSE;
						break;
					}
					else if (lpBuf[i] == 0xe5) {
						OutputVolDescLog("%sDeleted Entry\n", &pTab[0]);
					}
					else if ((lpBuf[11 + i] & 0x0f) == 0x0f) {
						OutputFsFATLDirEntry(lpBuf, &i, pTab);
					}
					else {
						OutputFsFATDirEntry(lpBuf, i, pTab);

						DWORD FstClus = MAKEDWORD(MAKEWORD(lpBuf[26 + i], lpBuf[27 + i]), MAKEWORD(lpBuf[20 + i], lpBuf[21 + i]));
						if (FstClus != 0 && (lpBuf[11 + i] & 0x10) == 0x10 && lpBuf[i] != '.') {
							INT lNextReadSec = (INT)((pFat->DataStartSector + (FstClus - 2) * pFat->SecPerClus));
							LARGE_INTEGER seekPosNext;
							LONGLONG tmp = (LONGLONG)lNextReadSec;
							seekPosNext.QuadPart = tmp * dwBytesPerSector;
							size_t idx = _tcslen(&pTab[0]);
							pTab[idx] = '\t';
							OutputVolDescLog("%s", &pTab[0]);
							OutputVolDescWithLBALog1("DirectoryEntry", lNextReadSec);
							ReadFATDirectoryRecord(handle, seekPosNext, dwBytesPerSector, pFat, pTab);
							pTab[idx] = 0;
						}
					}
				}
			}
			else {
				OutputErrorString(
					"Read size is different. NumberOfBytesToRead: %lu, NumberOfBytesRead: %lu\n"
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

BOOL ReadExFATDirectoryEntry(
#ifdef _WIN32
	HANDLE handle,
#elif __linux__
	int handle,
#elif __MACH__
	SCSITaskInterface** handle,
#endif
	DWORD dwBytesPerSector,
	PEXFAT pExFat,
	_TCHAR* pTab
) {
	LPBYTE lpBuf = (LPBYTE)calloc(dwBytesPerSector, sizeof(BYTE));
	if (!lpBuf) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = FALSE;
	DWORD dwBytesRead = 0;
	BOOL bEof = FALSE;
	INT nCnt = 0;
	BOOL bName1st = FALSE;
	INT NameLength = 0;
	WORD attr = 0;
	UINT FirstCluster = 0;

	while (1) {
		LARGE_INTEGER seekPos;
		seekPos.QuadPart = (LONGLONG)(pExFat->DirStartSector + nCnt) * dwBytesPerSector;
		SetFilePointerEx(handle, seekPos, NULL, FILE_BEGIN);

		bRet = ReadFile(handle, lpBuf, dwBytesPerSector, &dwBytesRead, 0);
		if (bRet) {
			if (dwBytesPerSector == dwBytesRead) {
				OutputMainChannel(fileMainInfo, lpBuf, _T("exFAT Directory Entry"), (INT)(pExFat->DirStartSector + nCnt), dwBytesPerSector);
				for (INT i = 0; i < 16; i++) {
					BYTE entry = lpBuf[i * 32];
					if (entry == 0) {
						bEof = TRUE;
						break;
					}
					else if (1 <= entry && entry <= 0x7f) {
						continue;
					}
					else if (entry == 0x81) {
						OutputFsExFATDirectoryEntry0x81(lpBuf, i, pTab);
					}
					else if (entry == 0x82) {
						OutputFsExFATDirectoryEntry0x82(lpBuf, i, pTab);
					}
					else if (entry == 0x83) {
						OutputFsExFATDirectoryEntry0x83(lpBuf, i, pTab);
					}
					else if (entry == 0x85) {
						attr = MAKEWORD(lpBuf[i * 32 + 4], lpBuf[i * 32 + 5]);
						OutputFsExFATDirectoryEntry0x85(lpBuf, attr, i, pTab);
					}
					else if (entry == 0xa0) {
						OutputFsExFATDirectoryEntry0xa0(lpBuf, i, pTab);
					}
					else if (entry == 0xc0) {
						NameLength = lpBuf[i * 32 + 3];
						FirstCluster = MAKEUINT(MAKEWORD(lpBuf[i * 32 + 20], lpBuf[i * 32 + 21]), MAKEWORD(lpBuf[i * 32 + 22], lpBuf[i * 32 + 23]));
						OutputFsExFATDirectoryEntry0xc0(lpBuf, NameLength, FirstCluster, i, pTab);
						bName1st = TRUE;
					}
					else if (entry == 0xc1) {
						OutputFsExFATDirectoryEntry0xc1(lpBuf, &bName1st, i, pTab);
						if (NameLength <= 15) {
							OutputVolDescLog("\n");
							if ((attr & 0x10) == 0x10) {
								size_t idx = _tcslen(&pTab[0]);
								pTab[idx] = '\t';
								UINT uiBackup = pExFat->DirStartSector;
								pExFat->DirStartSector = GetLBAfromClusterNumber(pExFat, FirstCluster);
								ReadExFATDirectoryEntry(handle, dwBytesPerSector, pExFat, pTab);
								pExFat->DirStartSector = uiBackup;
								pTab[idx] = 0;
							}
						}
						else {
							NameLength -= 15;
						}
					}
					else {
						OutputVolDescLog(
							"%sOther Entry\n"
							"%s\t                EntryType: %02x\n"
							, &pTab[0], &pTab[0], entry
						);
					}
				}

				if (bEof) {
					break;
				}
				else {
					nCnt++;
				}
			}
		}
	}
	FreeAndNull(lpBuf);
	return bRet;
}

BOOL ReadFileSystem(
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpBuf
) {
	DWORD dwBytesRead = 0;
	BOOL bHfs = FALSE;
	LONG firstPartition = 0;
	BOOL bRet = ReadFile(pDevice->hDevice, lpBuf, pDisc->dwBytesPerSector, &dwBytesRead, 0);
	if (bRet) {
		if (pDisc->dwBytesPerSector == dwBytesRead) {
			if (IsFat(lpBuf)) {
				_TCHAR szTab[256] = {};
				if (IsExFat(lpBuf)) {
					EXFAT exFat = { 0, 1, 0 };
					OutputFsExFAT(lpBuf, &exFat);

					szTab[0] = _T('\t');
					OutputVolDescLog("%s", szTab);
					OutputVolDescWithLBALog1("exFAT DirectoryEntry", (INT)exFat.DirStartSector);
					ReadExFATDirectoryEntry(pDevice->hDevice, pDisc->dwBytesPerSector, &exFat, szTab);
				}
				else {
					FAT fat = {};
					OutputFsFileAllocationTable(lpBuf, &fat);
					szTab[0] = _T('\t');
					OutputVolDescLog("%s", szTab);
					OutputVolDescWithLBALog1("FAT DirectoryEntry", (INT)fat.RootDirStartSector);
					LARGE_INTEGER seekPos;
					seekPos.QuadPart = fat.RootDirStartSector * pDisc->dwBytesPerSector;
					ReadFATDirectoryRecord(pDevice->hDevice, seekPos, pDisc->dwBytesPerSector, &fat, szTab);
				}
			}
			else if (IsDriverDescriptorRecord(lpBuf)) {
				OutputFsDriveDescriptorRecord(lpBuf);
				while (ReadFile(pDevice->hDevice, lpBuf, pDisc->dwBytesPerSector, &dwBytesRead, 0)) {
					if (pDisc->dwBytesPerSector == dwBytesRead) {
						if (IsApplePartionMap(lpBuf)) {
							OutputFsPartitionMap(lpBuf, &bHfs);
							if (bHfs && firstPartition == 0) {
								firstPartition = MAKELONG(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], lpBuf[8]));
							}
						}
						else if (IsValidMacDataHeader(lpBuf)) {
							UINT uiCatalogFileSize = 0;
							OutputFsMasterDirectoryBlocks(lpBuf, (INT)firstPartition, &uiCatalogFileSize);
							break;
						}
						else {
							if (bHfs) {
								SetFilePointer(pDevice->hDevice, firstPartition * (LONG)pDisc->dwBytesPerSector, NULL, FILE_BEGIN);
								bHfs = FALSE;
							}
						}
					}
				}
			}
		}
		else {
			OutputErrorString(
				"Read size is different. NumberOfBytesToRead: %lu, NumberOfBytesRead: %lu\n"
				, pDisc->dwBytesPerSector, dwBytesRead);
		}
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	SetFilePointer(pDevice->hDevice, 0, NULL, FILE_BEGIN);
	return bRet;
}

BOOL ReadDisk(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszPath,
	PHASH pHash
) {
	_TCHAR szFnameAndExt[_MAX_FNAME + _MAX_EXT] = {};
	FILE* fp = CreateOrOpenFile(
		pszPath, NULL, NULL, szFnameAndExt, NULL, _T(".bin"), _T("wb"), 0, 0);
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = FALSE;
	DWORD64 dwDiskSize = 0;
	if (*pExecType == fd) {
		bRet = DiskGetMediaTypes(pDevice, pDisc, &dwDiskSize);
	}
	else if (*pExecType == disk) {
		bRet = StorageGetMediaTypesEx(pDevice, pDisc, &dwDiskSize);
	}
	if (bRet) {
		DWORD dwReadSize = (DWORD)dwDiskSize;
		DWORD dwBlkSize = (DWORD)(dwDiskSize / pDisc->dwBytesPerSector);

		OutputString(
			"DiskSize: %llu bytes, BytesPerSector: %lu, BlockSize: %lu\n"
			, dwDiskSize, pDisc->dwBytesPerSector, dwBlkSize);
		if (*pExecType == disk) {
			dwReadSize = pDevice->dwMaxTransferLength;
		}
		LPBYTE lpBuf = (LPBYTE)calloc(dwReadSize, sizeof(BYTE));
		if (!lpBuf) {
			FcloseAndNull(fp);
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		ReadFileSystem(pDevice, pDisc, lpBuf);
		FlushLog();

		bRet = Read10(pExtArg, pDevice, pDisc, szFnameAndExt, lpBuf, dwBlkSize, fp, pHash);
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
#elif __linux__
	dvd_authinfo auth_info;

	memset(&auth_info, 0, sizeof(auth_info));
	auth_info.type = DVD_LU_SEND_RPC_STATE;

	int bRet = ioctl(pDevice->hDevice, DVD_AUTH, &auth_info);
	dvdRegion.SystemRegion = auth_info.lrpcs.region_mask;
	dvdRegion.ResetCount = auth_info.lrpcs.type;
#elif __MACH__
	int bRet = 0;
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
	LONG lLineNum,
	BOOL bOutputMsg
) {
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb = {};
#ifdef _WIN32
	swb.Sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	swb.Sptd.PathId = pDevice->address.PathId;
	swb.Sptd.TargetId = pDevice->address.TargetId;
	swb.Sptd.Lun = pDevice->address.Lun;
	swb.Sptd.CdbLength = byCdbLength;
	swb.Sptd.SenseInfoLength = SENSE_BUFFER_SIZE;
	swb.Sptd.DataIn = (UCHAR)nDataDirection;
	swb.Sptd.DataTransferLength = dwBufferLength;
	swb.Sptd.TimeOutValue = pDevice->dwTimeOutValue;
	swb.Sptd.DataBuffer = pvBuffer;
	swb.Sptd.SenseInfoOffset =
		offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, SenseData);
	memcpy(swb.Sptd.Cdb, lpCdb, byCdbLength);
#elif __linux__
	swb.io_hdr.interface_id = 'S';
	swb.io_hdr.dxfer_direction = nDataDirection;
	swb.io_hdr.cmd_len = byCdbLength;
	swb.io_hdr.mx_sb_len = sizeof(swb.Dummy);
	swb.io_hdr.dxfer_len = (unsigned int)dwBufferLength;
	swb.io_hdr.dxferp = pvBuffer;
	swb.io_hdr.cmdp = (unsigned char*)lpCdb;
	swb.io_hdr.sbp = swb.Dummy;
	swb.io_hdr.timeout = (unsigned int)pDevice->dwTimeOutValue;
//	swb.io_hdr.flags = SG_FLAG_DIRECT_IO;
#elif __MACH__
	// https://developer.apple.com/library/archive/documentation/DeviceDrivers/Conceptual/WorkingWithSAM/WWS_SAMDevInt/WWS_SAM_DevInt.html#//apple_ref/doc/uid/TP30000387-SW1
	IOReturn         err  = 0;
	IOVirtualRange* range = NULL;
	// Allocate a virtual range for the buffer. If we had more than 1 scatter-gather entry,
	// we would allocate more than 1 IOVirtualRange.
	if (NULL == (range = (IOVirtualRange*)malloc(sizeof(IOVirtualRange)))) {
		fprintf(stderr, "*********** ERROR Malloc'ing IOVirtualRange ***********\n\n");
		return FALSE;
	}
	// Set up the range. The address is just the buffer's address. The length is our request size.
	range->address = (IOVirtualAddress)pvBuffer;
	range->length  = dwBufferLength;

	// Set the actual CDB in the task
	SCSICommandDescriptorBlock cdb;
	memcpy(&cdb, lpCdb, byCdbLength);
	if (kIOReturnSuccess != (err = (*pDevice->hDevice)->SetCommandDescriptorBlock(pDevice->hDevice, cdb, byCdbLength))) {
		fprintf(stderr, "*********** ERROR Setting CDB ***********\n\n");
		return FALSE;
	}
	// Set the scatter-gather entry in the task
	if (kIOReturnSuccess != (err = (*pDevice->hDevice)->SetScatterGatherEntries(
		pDevice->hDevice, range, 1, dwBufferLength, kSCSIDataTransfer_FromTargetToInitiator))) {
		fprintf(stderr, "*********** ERROR Setting SG Entries ***********\n\n");
		return FALSE;
	}
	// Set the timeout in the task
	if (kIOReturnSuccess != (err = (*pDevice->hDevice)->SetTimeoutDuration(pDevice->hDevice, 10000))) {
		fprintf(stderr, "*********** ERROR Setting Timeout ***********\n\n");
		return FALSE;
	}
#endif
	DWORD dwLength = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
	DWORD dwReturned = 0;
	BOOL bRet = TRUE;
	BOOL bNoSense = FALSE;
	SetLastError(NO_ERROR);
	if (!DeviceIoControl(pDevice->hDevice
		, IOCTL_SCSI_PASS_THROUGH_DIRECT, &swb, dwLength, &swb, dwLength, &dwReturned, NULL)) {
		if (bOutputMsg) {
			OutputLastErrorNumAndString(pszFuncName, lLineNum);
		}
		bRet = FALSE;
		if (pExtArg) {
			if (!pExtArg->byScanProtectViaFile) {
				// When semaphore time out occurred, if doesn't execute sleep,
				// UNIT_ATTENSION errors occurs next ScsiPassThroughDirect executing.
				UINT milliseconds = 25000;
				OutputLog(standardError | fileMainError
					, "Please wait for %u milliseconds until the device is returned\n", milliseconds);
				Sleep(milliseconds);
				pDevice->FEATURE.bySetCDSpeed = FALSE;
			}
		}
	}
	else {
		if (swb.SenseData.SenseKey == SCSI_SENSE_NO_SENSE &&
			swb.SenseData.AdditionalSenseCode == SCSI_ADSENSE_NO_SENSE &&
			swb.SenseData.AdditionalSenseCodeQualifier == 0x00) {
			bNoSense = TRUE;
		}
#ifdef _WIN32
		if (swb.Sptd.ScsiStatus >= SCSISTAT_CHECK_CONDITION && !bNoSense) {
			INT nLBA = 0;
			if (swb.Sptd.Cdb[0] == 0x28 || swb.Sptd.Cdb[0] == 0xa8 || swb.Sptd.Cdb[0] == 0xad ||
				swb.Sptd.Cdb[0] == 0xbe || swb.Sptd.Cdb[0] == 0xd8) {
				nLBA = (swb.Sptd.Cdb[2] << 24) + (swb.Sptd.Cdb[3] << 16)
					+ (swb.Sptd.Cdb[4] << 8) + swb.Sptd.Cdb[5];
			}
			if (bOutputMsg) {
				OutputLog(standardError | fileMainError
					, "\r" STR_LBA "[F:%s][L:%ld]\n\tOpcode: %#02x\n"
					, nLBA, nLBA, pszFuncName, lLineNum, swb.Sptd.Cdb[0]);
				OutputScsiStatus(swb.Sptd.ScsiStatus);
			}
#elif __linux__
		if (swb.io_hdr.status >= SCSISTAT_CHECK_CONDITION && !bNoSense) {
			INT nLBA = 0;
			if (swb.io_hdr.cmdp[0] == 0xa8 || swb.io_hdr.cmdp[0] == 0xad ||
				swb.io_hdr.cmdp[0] == 0xbe || swb.io_hdr.cmdp[0] == 0xd8) {
				nLBA = (swb.io_hdr.cmdp[2] << 24) + (swb.io_hdr.cmdp[3] << 16)
					+ (swb.io_hdr.cmdp[4] << 8) + swb.io_hdr.cmdp[5];
			}
			if (bOutputMsg) {
				OutputLog(standardError | fileMainError
					, "\rLBA[%06d, %#07x]: [F:%s][L:%ld]\n\tOpcode: %#02x\n"
					, nLBA, (UINT)nLBA, pszFuncName, lLineNum, swb.io_hdr.cmdp[0]);
				OutputScsiStatus(swb.io_hdr.status);
			}
#elif __MACH__
		if (swb.taskStatus >= SCSISTAT_CHECK_CONDITION && !bNoSense) {
#endif
			if (bOutputMsg) {
				OutputSenseData(&swb.SenseData);
			}
			if (swb.SenseData.SenseKey == SCSI_SENSE_UNIT_ATTENTION) {
				UINT milliseconds = 40000;
				OutputLog(standardError | fileMainError
					, "Please wait for %u milliseconds until the device is returned\n", milliseconds);
				Sleep(milliseconds);
			}
		}
	}
	if (bNoSense) {
		*byScsiStatus = SCSISTAT_GOOD;
	}
	else {
#ifdef _WIN32
		*byScsiStatus = swb.Sptd.ScsiStatus;
#elif __linux__
		*byScsiStatus = swb.io_hdr.status;
#elif __MACH__
		*byScsiStatus = swb.taskStatus;
		FreeAndNull(range);
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
#if 0
		if (adapterDescriptor->MaximumTransferLength > 65536) {
			pDevice->dwMaxTransferLength = 65536;
			OutputDriveLog("dwMaxTransferLength changed [%lu] -> [%lu]\n"
				, adapterDescriptor->MaximumTransferLength, pDevice->dwMaxTransferLength);
		}
		else {
#endif
			pDevice->dwMaxTransferLength = adapterDescriptor->MaximumTransferLength;
#if 0
		}
#endif
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
		OutputString("Set the drive speed: %luKB/sec\n"), setstreaming.ReadSize);
		OutputString("dwReturned: %lu\n"), dwReturned);
	}
	return TRUE;
}
#endif
