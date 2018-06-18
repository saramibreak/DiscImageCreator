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
#include "convert.h"
#include "execIoctl.h"
#include "execScsiCmd.h"
#include "execScsiCmdforDVD.h"
#include "execScsiCmdforFileSystem.h"
#include "get.h"
#include "output.h"
#include "outputScsiCmdLogforDVD.h"

#define GAMECUBE_SIZE	(712880)
#define WII_SL_SIZE		(2294912)
#define WII_DL_SIZE		(4155840)

BOOL ReadDVD(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath
) {
	FILE* fp = CreateOrOpenFile(
		pszFullPath, NULL, NULL, NULL, NULL, _T(".iso"), _T("wb"), 0, 0);
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	try {
		if (NULL == (pBuf = (LPBYTE)calloc(
			pDevice->dwMaxTransferLength + pDevice->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);

		CDB::_READ12 cdb = { 0 };
		cdb.OperationCode = SCSIOP_READ12;
		cdb.LogicalUnitNumber = pDevice->address.Lun;

		if (pExtArg->byFua) {
			cdb.ForceUnitAccess = TRUE;
		}
		if (!ReadDVDForFileSystem(pExecType, pExtArg, pDevice, pDisc, &cdb, lpBuf)) {
			throw FALSE;
		}

		DWORD dwLayer1MiddleZone =
			pDisc->DVD.dwXBOXStartPsn - pDisc->DVD.dwDVDStartPsn - pDisc->DVD.dwLayer0SectorLength;
		INT nAllLength = pDisc->SCSI.nAllLength;
		if (*pExecType == xbox) {
			OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(TotalLength)
				"\t    L0 + L1 data: %7d (%#x)\n"
				"\t+      L1 Middle: %7lu (%#lx)\n"
				"\t+       L1 Video: %7lu (%#lx)\n"
				"\t------------------------------------\n"
				
				, nAllLength, nAllLength
				, dwLayer1MiddleZone, dwLayer1MiddleZone
				, pDisc->DVD.dwLayer1SectorLength, pDisc->DVD.dwLayer1SectorLength);
			nAllLength += dwLayer1MiddleZone + pDisc->DVD.dwLayer1SectorLength;
			OutputDiscLogA(
				"\t                  %7u (%#x)\n", nAllLength, nAllLength);
		}
		FlushLog();

		DWORD dwTransferLen = pDevice->dwMaxTransferLength / DISC_RAW_READ_SIZE;
		REVERSE_BYTES(&cdb.TransferLength, &dwTransferLen);
		BYTE byScsiStatus = 0;
		INT i = 0;
		DWORD dwTransferLenOrg = dwTransferLen;

		for (INT nLBA = 0; nLBA < pDisc->SCSI.nAllLength; nLBA += dwTransferLen) {
			if (*pExecType == xbox) {
				if (pDisc->DVD.securitySectorRange[i][0] <= (DWORD)nLBA &&
					(DWORD)nLBA <= pDisc->DVD.securitySectorRange[i][1] + 1) {
					if ((DWORD)nLBA == pDisc->DVD.securitySectorRange[i][1] + 1) {
						i++;
					}
					else {
						if (dwTransferLen != dwTransferLenOrg) {
							dwTransferLen = dwTransferLenOrg;
							REVERSE_BYTES(&cdb.TransferLength, &dwTransferLen);
						}
						ZeroMemory(lpBuf, DISC_RAW_READ_SIZE * dwTransferLen);
						fwrite(lpBuf, sizeof(BYTE), (size_t)DISC_RAW_READ_SIZE * dwTransferLen, fp);
						continue;
					}
				}
				else if (dwTransferLen > (DWORD)(pDisc->DVD.securitySectorRange[i][0] - nLBA)) {
					dwTransferLen = (DWORD)(pDisc->DVD.securitySectorRange[i][0] - nLBA);
					REVERSE_BYTES(&cdb.TransferLength, &dwTransferLen);
				}
			}
			if (dwTransferLen > (DWORD)(pDisc->SCSI.nAllLength - nLBA)) {
				dwTransferLen = (DWORD)(pDisc->SCSI.nAllLength - nLBA);
				REVERSE_BYTES(&cdb.TransferLength, &dwTransferLen);
			}
			REVERSE_BYTES(&cdb.LogicalBlock, &nLBA);
			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				DISC_RAW_READ_SIZE * dwTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			fwrite(lpBuf, sizeof(BYTE), (size_t)DISC_RAW_READ_SIZE * dwTransferLen, fp);
			OutputString(_T("\rCreating iso(LBA) %8lu/%8u"), nLBA + dwTransferLen, nAllLength);
		}
		if (*pExecType == xbox) {
			if (!SetLockState(pExtArg, pDevice, 0)) {
				throw FALSE;
			}
			dwTransferLen = dwTransferLenOrg;
			ZeroMemory(lpBuf, DISC_RAW_READ_SIZE * dwTransferLen);
			DWORD dwEndOfMiddle = pDisc->SCSI.nAllLength + dwLayer1MiddleZone;

			for (DWORD j = (DWORD)pDisc->SCSI.nAllLength; j < dwEndOfMiddle; j += dwTransferLen) {
				if (dwTransferLen > dwEndOfMiddle - j) {
					dwTransferLen = dwEndOfMiddle - j;
				}
				fwrite(lpBuf, sizeof(BYTE), (size_t)DISC_RAW_READ_SIZE * dwTransferLen, fp);
				OutputString(_T("\rCreating iso(LBA) %8lu/%8u"), j + dwTransferLen, nAllLength);
			}

			dwTransferLen = dwTransferLenOrg;
			REVERSE_BYTES(&cdb.TransferLength, &dwTransferLen);
			DWORD dwEndOfDvd = pDisc->DVD.dwLayer0SectorLength + pDisc->DVD.dwLayer1SectorLength;

			for (DWORD k = pDisc->DVD.dwLayer0SectorLength; k < dwEndOfDvd; k += dwTransferLen) {
				if (dwTransferLen > dwEndOfDvd - k) {
					dwTransferLen = dwEndOfDvd - k;
					REVERSE_BYTES(&cdb.TransferLength, &dwTransferLen);
				}
				REVERSE_BYTES(&cdb.LogicalBlock, &k);
				if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
					DISC_RAW_READ_SIZE * dwTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					throw FALSE;
				}
				fwrite(lpBuf, sizeof(BYTE), (size_t)DISC_RAW_READ_SIZE * dwTransferLen, fp);
				OutputString(_T("\rCreating iso(LBA) %8lu/%8u")
					, dwEndOfMiddle + pDisc->DVD.dwLayer1SectorLength, nAllLength);
			}
		}
		OutputString(_T("\n"));
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FreeAndNull(pBuf);
	FcloseAndNull(fp);
	return bRet;
}

BOOL IsSupported0xE7Type1(
	PDEVICE pDevice
) {
	if (!strncmp(pDevice->szProductId, "RW/DVD GCC-4160N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-4240N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-4241N", DRIVE_PRODUCT_ID_SIZE)
		) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsSupported0xE7Type2(
	PDEVICE pDevice
) {
	if (!strncmp(pDevice->szProductId, "RW/DVD GCC-4242N", DRIVE_PRODUCT_ID_SIZE)
		) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsSupported0xE7Type3(
	PDEVICE pDevice
) {
	if (!strncmp(pDevice->szProductId, "CDRW-DVD GCC4244", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "CDRW-DVD GCC4247", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-4243N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-4244N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-4246N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-4247N", DRIVE_PRODUCT_ID_SIZE)
		) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsSupported0xE7Type4(
	PDEVICE pDevice
) {
	if (// comfirmed
		!strncmp(pDevice->szProductId, "DVD-ROM DU10N   ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8082N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8161B", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8162B", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8163B", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8164B", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR-T10N", DRIVE_PRODUCT_ID_SIZE) ||
		// not comfirmed
		!strncmp(pDevice->szProductId, "CDRW-DVD GCCT10N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "CDRW-DVD GCCT20N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM DH10N   ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM DH16NS10", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM DP10N   ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM DTA0N   ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM DU90N   ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR-D10N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR-D20N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR-M10N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR-R10N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR-T20N", DRIVE_PRODUCT_ID_SIZE) ||	
		!strncmp(pDevice->szProductId, "DVD-ROM GDR3120L", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8083N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8084N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8085N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8087N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDRH10N ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDRH20N ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-5241P", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-C10N ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-C20N ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-M10N ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-T10N ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-T20N ", DRIVE_PRODUCT_ID_SIZE)
		) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsSupported0xE7(
	PDEVICE pDevice
) {
	if (IsSupported0xE7Type1(pDevice) ||
		IsSupported0xE7Type2(pDevice) ||
		IsSupported0xE7Type3(pDevice) ||
		IsSupported0xE7Type4(pDevice)) {
		return TRUE;
	}
	return FALSE;
}

BOOL ReadDVDRaw(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath
) {
	FILE* fp = NULL;
	_TCHAR szMode[4] = _T("wb");
	if (pExtArg->byResume) {
		memcpy(szMode, _T("ab+"), 3);
	}
	else if (pExtArg->byFix) {
		memcpy(szMode, _T("rb+"), 3);
	}
	if (NULL == (fp = CreateOrOpenFile(pszFullPath, NULL, NULL, NULL, NULL, _T(".raw"), szMode, 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE pBuf = NULL;
	BOOL bRet = TRUE;
	try {
		DWORD dwTransferLen = 16;
		INT nMemBlkSize = 1;
		if (NULL == (pBuf = (LPBYTE)calloc(
			(size_t)DVD_RAW_READ * dwTransferLen * 5 + pDevice->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);
		INT nCmdType = 0;
		INT nBaseAddr = 0x80000000;
		BYTE cdblen = CDB10GENERIC_LENGTH;
		// for dumping from memory
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		INT nDriveSampleOffset = 0;
		if (!GetDriveOffsetAuto(pDevice->szProductId, &nDriveSampleOffset)) {
			GetDriveOffsetManually(&nDriveSampleOffset);
		}
		// Panasonic MN103S chip
		if (nDriveSampleOffset == 102) {
			if (IsSupported0xE7(pDevice)) {
				if (IsSupported0xE7Type1(pDevice)) {
					// address which disc data is cached
					// a08000 - a0ffff (8000), a13000 - a1ffff (d000), a23000 - a2ffff (d000)
					// a48000 - a4ffff (8000), a53000 - a5ffff (d000), a63000 - a6ffff (d000)
					// a88000 - a8ffff (8000), a93000 - a9ffff (d000), aa3000 - aaffff (d000)
					// ac8000 - acffff (8000), ad3000 - adffff (d000), ae3000 - aeffff (d000)
					// b08000 - b0ffff (8000), b13000 - b1ffff (d000), b23000 - b2ffff (d000)
					//  :
					nBaseAddr = 0xa13000;
				}
				else if (IsSupported0xE7Type2(pDevice)) {
					dwTransferLen = 8;
					nBaseAddr -= 0x4080;
//					dwTransferLen = 1;
//					nBaseAddr -= 0x810;
					nCmdType = 1;
				}
				else if (IsSupported0xE7Type3(pDevice) || IsSupported0xE7Type4(pDevice)) {
					nMemBlkSize = 5;
				}
				lpCmd[0] = 0xE7; // vendor specific command
				lpCmd[1] = 0x48; // H
				lpCmd[2] = 0x49; // I
				lpCmd[3] = 0x54; // T
				lpCmd[4] = 0x01; // read MCU memory sub-command
				lpCmd[10] = HIBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
				lpCmd[11] = LOBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
				cdblen = CDB12GENERIC_LENGTH;
			}
			else {
				lpCmd[0] = SCSIOP_READ_DATA_BUFF;
				lpCmd[1] = 0x01;
				lpCmd[6] = LOBYTE(HIWORD(DVD_RAW_READ * dwTransferLen));
				lpCmd[7] = HIBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
				lpCmd[8] = LOBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
				nCmdType = 2;
			}
		}
		// Renesas chip
		else if (nDriveSampleOffset == 667) {
			dwTransferLen = 1;
			lpCmd[0] = SCSIOP_READ_DATA_BUFF;
			lpCmd[1] = 0x05;
			lpCmd[6] = LOBYTE(HIWORD(DVD_RAW_READ * dwTransferLen));
			lpCmd[7] = HIBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
			lpCmd[8] = LOBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
//			lpCmd[9] = 0x44;
			nCmdType = 3;
		}
		// Mediatek MT chip
		else if (nDriveSampleOffset == 6) {
			dwTransferLen = 1;
			lpCmd[0] = SCSIOP_READ_DATA_BUFF;
			lpCmd[1] = 0x01;
			lpCmd[2] = 0x01;
			lpCmd[6] = LOBYTE(HIWORD(DVD_RAW_READ * dwTransferLen));
			lpCmd[7] = HIBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
			lpCmd[8] = LOBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
		}
		// Plextor etc.
		else {
			lpCmd[0] = SCSIOP_READ_DATA_BUFF;
			lpCmd[1] = 0x02;
			lpCmd[6] = LOBYTE(HIWORD(DVD_RAW_READ * dwTransferLen));
			lpCmd[7] = HIBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
			lpCmd[8] = LOBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
		}

		INT nLBA = 0;
		DWORD dwSectorNum = 0x30000;
		if (pExtArg->byFix) {
			DWORD dwPos = pDisc->DVD.dwFixNum * 16;
			nLBA = (INT)dwPos;
			dwSectorNum += dwPos;
		}
		// for dumping from the disc
		CDB::_READ12 cdb = { 0 };
		cdb.OperationCode = SCSIOP_READ12;
		cdb.LogicalUnitNumber = pDevice->address.Lun;
		REVERSE_BYTES(&cdb.TransferLength, &dwTransferLen);

		if ((pDisc->SCSI.nAllLength == GAMECUBE_SIZE ||
			pDisc->SCSI.nAllLength == WII_SL_SIZE ||
			pDisc->SCSI.nAllLength == WII_DL_SIZE)
			&& (IsSupported0xE7(pDevice))) {
				cdb.Streaming = TRUE;
		}
		else {
			cdb.ForceUnitAccess = TRUE;
		}
		FlushLog();

		BYTE byScsiStatus = 0;
		DWORD dwReadAddrForHG[5] = { 
			nBaseAddr + 0 * dwTransferLen * DVD_RAW_READ,
			nBaseAddr + 1 * dwTransferLen * DVD_RAW_READ,
			nBaseAddr + 2 * dwTransferLen * DVD_RAW_READ,
			nBaseAddr + 3 * dwTransferLen * DVD_RAW_READ,
			nBaseAddr + 4 * dwTransferLen * DVD_RAW_READ
		};
		DWORD dwTransferAndMemSize = dwTransferLen * nMemBlkSize;
		DWORD dwReadSize = (DWORD)DISC_RAW_READ_SIZE * dwTransferLen;
		DWORD dwRawReadSize = (DWORD)DVD_RAW_READ * dwTransferLen;
		DWORD dwRawWriteSize = (DWORD)DVD_RAW_READ * dwTransferLen * nMemBlkSize;
		DWORD dwOfs[16] = {
			0, DVD_RAW_READ, DVD_RAW_READ * 2, DVD_RAW_READ * 3, DVD_RAW_READ * 4,
			DVD_RAW_READ * 5, DVD_RAW_READ * 6, DVD_RAW_READ * 7, DVD_RAW_READ * 8,
			DVD_RAW_READ * 9, DVD_RAW_READ * 10, DVD_RAW_READ * 11, DVD_RAW_READ * 12,
			DVD_RAW_READ * 13, DVD_RAW_READ * 14, DVD_RAW_READ * 15
		};
		INT nRereadNum = 0;

		if (pExtArg->byResume) {
			INT64 size = (INT64)GetFileSize64(0, fp);
			_fseeki64(fp, size - DVD_RAW_READ, SEEK_SET);
			fread(lpBuf, sizeof(BYTE), DVD_RAW_READ, fp);
			_fseeki64(fp, size, SEEK_SET);
			dwSectorNum = MAKEDWORD(MAKEWORD(lpBuf[3], lpBuf[2]), MAKEWORD(lpBuf[1], 0)) + 1;
			nLBA = (INT)(size / DVD_RAW_READ);
#if 0
			_fseeki64(fp, 0, SEEK_SET);
			for (DWORD n = 0x30000; n < dwSectorNum; n++) {
				fread(lpBuf, sizeof(BYTE), DVD_RAW_READ, fp);
				DWORD dwNum = MAKEDWORD(MAKEWORD(lpBuf[3], lpBuf[2]), MAKEWORD(lpBuf[1], 0));
				if (n != dwNum) {
					OutputString(" Expected sector num: %6lx, Got sector num: %6lx\n", n, dwNum);
				}
				OutputString("\rChecking sector num: %6lx", n);
			}
			OutputString("\n");
			return FALSE;
#endif
		}

		BOOL bRetry = FALSE;
		for (; nLBA < pDisc->SCSI.nAllLength; nLBA += dwTransferAndMemSize) {
			if (pExtArg->byFix) {
				_fseeki64(fp, DVD_RAW_READ * nLBA, SEEK_SET);
			}
			if ((INT)dwSectorNum == pDisc->SCSI.nAllLength + 0x30000) {
				break;
			}
			if ((INT)dwTransferAndMemSize > pDisc->SCSI.nAllLength - nLBA) {
				nMemBlkSize = (pDisc->SCSI.nAllLength - nLBA) / (INT)dwTransferLen;
				dwRawWriteSize = (DWORD)DVD_RAW_READ * dwTransferLen * nMemBlkSize;
				dwTransferAndMemSize = dwTransferLen * nMemBlkSize;
			}
			REVERSE_BYTES(&cdb.LogicalBlock, &nLBA);
			// store the disc data to the drive cache memory
			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				dwReadSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			BOOL bCheckSectorNum = TRUE;
			DWORD dwGotSectorNum = 0;
			for (INT i = 0; i < nMemBlkSize; i++) {
				if (nCmdType == 0) {
					lpCmd[6] = HIBYTE(HIWORD(dwReadAddrForHG[i]));
					lpCmd[7] = LOBYTE(HIWORD(dwReadAddrForHG[i]));
					lpCmd[8] = HIBYTE(LOWORD(dwReadAddrForHG[i]));
					lpCmd[9] = LOBYTE(LOWORD(dwReadAddrForHG[i]));
				}
				else if (nCmdType == 1) {
					if (nRereadNum == 0) {
						nBaseAddr += 0x810 * dwTransferLen;
						if (nBaseAddr == 0x80008100) {
							nBaseAddr = 0x80000000;
						}
					}
					lpCmd[6] = HIBYTE(HIWORD(nBaseAddr));
					lpCmd[7] = LOBYTE(HIWORD(nBaseAddr));
					lpCmd[8] = HIBYTE(LOWORD(nBaseAddr));
					lpCmd[9] = LOBYTE(LOWORD(nBaseAddr));
				}
				else if (nCmdType == 2 || nCmdType == 3) {
					INT n = nLBA % 688;
					lpCmd[3] = LOBYTE(HIWORD(n * DVD_RAW_READ));
					lpCmd[4] = HIBYTE(LOWORD(n * DVD_RAW_READ));
					lpCmd[5] = LOBYTE(LOWORD(n * DVD_RAW_READ));
				}
				DWORD dwOfs2 = dwRawReadSize * i;
				// read the drive cache memory
				if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, cdblen, lpBuf + dwOfs2,
					dwRawReadSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					Sleep(10000);
					throw FALSE;
				}
#if 1
				for (DWORD j = 0; j < dwTransferLen; j++) {
					DWORD dwOfs3 = dwOfs2 + dwOfs[j];
					dwGotSectorNum = MAKEDWORD(MAKEWORD(lpBuf[3 + dwOfs3]
						, lpBuf[2 + dwOfs3]), MAKEWORD(lpBuf[1 + dwOfs3], 0));
					if (dwSectorNum != dwGotSectorNum - j - i * dwTransferLen) {
						OutputLogA(standardError | fileMainError
							, " Expected sector num: %6lx, Got sector num: %6lx "
							, dwSectorNum, dwGotSectorNum - j - i * dwTransferLen);
						OutputMainErrorLogA("\n");
						bCheckSectorNum = FALSE;
						break;
					}
				}
#endif
				if (!bCheckSectorNum) {
					break;
				}
			}
			if (bCheckSectorNum) {
				nRereadNum = 0;
				fwrite(lpBuf, sizeof(BYTE), dwRawWriteSize, fp);
				dwSectorNum += dwTransferAndMemSize;
			}
			else {
				if (++nRereadNum == 40) {
					OutputString("Max Reread %d. LBA: %7d\n", nRereadNum, nLBA);
					if (!bRetry && IsSupported0xE7Type2(pDevice)) {
						dwTransferLen = 1;
						dwTransferAndMemSize = dwTransferLen * nMemBlkSize;
						dwReadSize = (DWORD)DISC_RAW_READ_SIZE * dwTransferLen;
						dwRawReadSize = (DWORD)DVD_RAW_READ * dwTransferLen;
						dwRawWriteSize = (DWORD)DVD_RAW_READ * dwTransferLen * nMemBlkSize;
						nBaseAddr = 0x7FFFF7F0;
						nLBA = (INT)dwSectorNum - 0x30000 - 2;
						nRereadNum = 0;
						lpCmd[10] = HIBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
						lpCmd[11] = LOBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
						REVERSE_BYTES(&cdb.TransferLength, &dwTransferLen);
						bRetry = TRUE;
						continue;
					}
					else {
						bRet = FALSE;
						break;
					}
				}
				OutputString("Reread %d. LBA: %7d\n", nRereadNum, nLBA);
			}
			if (nRereadNum || IsSupported0xE7Type1(pDevice) ||
				IsSupported0xE7Type2(pDevice) || IsSupported0xE7Type3(pDevice)
				) {
				INT tmp = nLBA - (INT)dwTransferAndMemSize;
				if (tmp < 0) {
					if (IsSupported0xE7Type1(pDevice)) {
						tmp = 16;
					}
					else if (IsSupported0xE7Type2(pDevice) || IsSupported0xE7Type3(pDevice)) {
						tmp = 0;
					}
				}
				if (nRereadNum) {
					if (nCmdType == 0) {
						nLBA = tmp;
					}
#if 1
					else if (!bRetry && nCmdType == 1) {
						if (nRereadNum == 1) {
							nLBA = tmp;
						}
						else if (nRereadNum == 2) {
							nLBA -= 16;
						}
						else if (nRereadNum == 4) {
							nLBA -= 24;
						}
						else if (nRereadNum == 7) {
							nLBA -= 32;
						}
						else if (nRereadNum == 11) {
							nLBA -= 40;
						}
						else if (nRereadNum == 16) {
							nLBA -= 48;
						}
						else if (nRereadNum == 22) {
							nLBA -= 56;
						}
						else if (nRereadNum == 29) {
							nLBA -= 64;
						}
					}
					if (bRetry && nCmdType == 1) {
						if ((INT)(dwGotSectorNum - dwSectorNum) > 0) {
							nLBA -= 2;
						}
						else {
							nLBA = tmp + 1;
						}
					}
#endif
					else if (nCmdType == 2) {
						if ((INT)(dwGotSectorNum - dwSectorNum) > 0) {
							nLBA -= 32;
						}
					}
				}
				REVERSE_BYTES(&cdb.LogicalBlock, &tmp);
				// delete cache memory
				if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
					dwReadSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					throw FALSE;
				}
			}
			OutputString(_T("\rCreating raw(LBA) %7u/%7u")
				, nLBA + (INT)dwTransferAndMemSize, pDisc->SCSI.nAllLength);
			if (pExtArg->byFix) {
				if (nLBA == (INT)pDisc->DVD.dwFixNum * 16 + 16) {
					break;
				}
			}
		}
		OutputString(_T("\n"));
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pBuf);
	FcloseAndNull(fp);

	if ((pDisc->SCSI.nAllLength == GAMECUBE_SIZE ||
		pDisc->SCSI.nAllLength == WII_SL_SIZE ||
		pDisc->SCSI.nAllLength == WII_DL_SIZE)
		&& bRet && IsSupported0xE7(pDevice)) {
		TCHAR str[_MAX_PATH * 3] = { 0 };
		if (GetUnscCmd(str, pszFullPath)) {
			bRet = _tsystem(str);
			// unscrambler error code
			// 0 == no error
			// 1 == failed open .raw
			// 2 == failed open .iso
			// 3 == no enough cache space for this seed
			// 4 == no seed found for recording frame xx
			// 6 == can't write to .iso
			// frame num == error unscrambling recording frame xx
			OutputString("ret = %d\n", bRet);
		}
	}
	return bRet;
}

BOOL ReadDVDForCMI(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	CONST WORD wSize =
		sizeof(DVD_DESCRIPTOR_HEADER) + sizeof(DVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR);
	_declspec(align(4)) BYTE pBuf[wSize] = { 0 };

	CDB::_READ_DVD_STRUCTURE cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_DVD_STRUCTURE;
	cdb.Format = DvdMaxDescriptor;
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wSize);
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(CopyrightManagementInformation));

	BOOL bRet = TRUE;
	BYTE byScsiStatus = 0;
	for (INT nLBA = 0; nLBA < pDisc->SCSI.nAllLength; nLBA++) {
		REVERSE_BYTES(&cdb.RMDBlockNumber, &nLBA);
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, pBuf, 
			wSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputDiscWithLBALogA("Read error\n", nLBA);
		}
		else {
			OutputDVDCopyrightManagementInformation(
				(PDVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR)(pBuf + sizeof(DVD_DESCRIPTOR_HEADER)), nLBA);
		}
		OutputString(
			_T("\rWriting CMI log(LBA) %8u/%8u"), nLBA, pDisc->SCSI.nAllLength - 1);
	}
	OutputString(_T("\n"));
	return bRet;
}

BOOL ExecReadingKey(
	PDEVICE pDevice
) {
	BOOL bRet = TRUE;
	// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/content/ntddcdvd/ns-ntddcdvd-_dvd_copy_protect_key
	// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/content/ntddcdvd/ne-ntddcdvd-dvd_key_type
	PDVD_COPY_PROTECT_KEY dvdKey = (PDVD_COPY_PROTECT_KEY)calloc(DVD_DISK_KEY_LENGTH, sizeof(BYTE));
	try {
		if (!DvdStartSession(pDevice, dvdKey)) {
			throw FALSE;
		}

		for (int j = 13; 3 < j; j--) {
			dvdKey->KeyData[j] = (UCHAR)(j - 4);
		}

		dvdKey->KeyLength = DVD_CHALLENGE_KEY_LENGTH;
		dvdKey->KeyType = DvdChallengeKey;
		if (!SendKey(pDevice, dvdKey)) {
			throw FALSE;
		}

		dvdKey->KeyLength = DVD_BUS_KEY_LENGTH;
		dvdKey->KeyType = DvdBusKey1;
		if (!ReadKey(pDevice, dvdKey)) {
			throw FALSE;
		}

		dvdKey->KeyLength = DVD_CHALLENGE_KEY_LENGTH;
		dvdKey->KeyType = DvdChallengeKey;
		if (!ReadKey(pDevice, dvdKey)) {
			throw FALSE;
		}
#if 0
		// need to exec DeCSS
		// https://github.com/mpc-hc/mpc-hc/tree/develop/src/DeCSS
		// https://koeln.ccc.de/archiv/drt/css/
		// https://offog.org/git/dvdaexplorer/src/libdvdcpxm/src/
		// http://download.videolan.org/pub/videolan/libdvdcss/
		// https://www.videolan.org/developers/libdvdcss.html
		// https://github.com/allienx/libdvdcss-dll
		dvdKey->KeyLength = DVD_BUS_KEY_LENGTH;
		dvdKey->KeyType = DvdBusKey2;
		if (!SendKey(pDevice, dvdKey)) {
			throw FALSE;
		}
		dvdKey->KeyLength = DVD_TITLE_KEY_LENGTH;
		dvdKey->KeyType = DvdTitleKey;
		if (!ReadKey(pDevice, dvdKey)) {
			throw FALSE;
		}
		dvdKey->KeyLength = DVD_DISK_KEY_LENGTH;
		dvdKey->KeyType = DvdDiskKey;
		if (!ReadKey(pDevice, dvdKey)) {
			throw FALSE;
		}
		dvdKey->KeyLength = DVD_ASF_LENGTH;
		dvdKey->KeyType = DvdAsf;
		if (!ReadKey(pDevice, dvdKey)) {
			throw FALSE;
		}
#endif
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(dvdKey);
	return bRet;
}

BOOL ReadDiscStructure(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath
) {
	CONST WORD wMaxDVDStructureSize =
		sizeof(DVD_DESCRIPTOR_HEADER) + sizeof(DVD_STRUCTURE_LIST_ENTRY) * 0xff;
	_declspec(align(4)) BYTE pBuf[wMaxDVDStructureSize] = { 0 };

	CDB::_READ_DVD_STRUCTURE cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_DVD_STRUCTURE;
	if (*pExecType == bd) {
		cdb.Reserved1 = 1;
	}
	cdb.Format = 0xff;
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wMaxDVDStructureSize);

	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH,
		pBuf, wMaxDVDStructureSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}

	PDVD_DESCRIPTOR_HEADER pDescHeader = ((PDVD_DESCRIPTOR_HEADER)pBuf);
	REVERSE_SHORT(&pDescHeader->Length);
	WORD wDataSize = pDescHeader->Length - sizeof(pDescHeader->Length);
	WORD wEntrySize = wDataSize / sizeof(DVD_STRUCTURE_LIST_ENTRY);

	_TCHAR szPath[_MAX_PATH] = { 0 };
	_tcsncpy(szPath, pszFullPath, _MAX_PATH);
	if (!PathRemoveFileSpec(szPath)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fpPfi = NULL;
	FILE* fpDmi = NULL;
	FILE* fpPic = NULL;
	if (*pExecType == dvd || *pExecType == xbox) {
		fpPfi = CreateOrOpenFileA(szPath, "\\PFI", NULL, NULL, NULL, ".bin", "wb", 0, 0);
		if (!fpPfi) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		fpDmi = CreateOrOpenFileA(szPath, "\\DMI", NULL, NULL, NULL, ".bin", "wb", 0, 0);
		if (!fpDmi) {
			FcloseAndNull(fpPfi);
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	else if (*pExecType == bd) {
		fpPic = CreateOrOpenFileA(szPath, "\\PIC", NULL, NULL, NULL, ".bin", "wb", 0, 0);
		if (!fpPic) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}

	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DiscStructure));
	for (WORD i = 0; i < wEntrySize; i++) {
		PDVD_STRUCTURE_LIST_ENTRY pEntry = 
			((PDVD_STRUCTURE_LIST_ENTRY)pDescHeader->Data + i);
		WORD wFormatLen = MAKEWORD(pEntry->FormatLength[1], pEntry->FormatLength[0]);
		OutputDiscLogA(
			"FormatCode: %02x, Sendable: %3s, Readable: %3s, FormatLength: %u\n", 
			pEntry->FormatCode,	BOOLEAN_TO_STRING_YES_NO_A(pEntry->Sendable),
			BOOLEAN_TO_STRING_YES_NO_A(pEntry->Readable), wFormatLen);
		if (wFormatLen == 0) {
			OutputDiscLogA("Skiped because length is 0\n\n");
			continue;
		}
		if (*pExecType == dvd || *pExecType == xbox) {
			if (pEntry->FormatCode == 0xff) {
				OutputDiscLogA("Skiped\n\n");
				break;
			}
			else if (pEntry->FormatCode == 0x02) {
				if (pDisc->DVD.protect == css) {
					ExecReadingKey(pDevice);
				}
				else {
					OutputDiscLogA("Skiped because of DVD with CSS only\n\n");
					continue;
				}
			}
			else if (pEntry->FormatCode == 0x03 && !pDisc->DVD.ucBca && !pExtArg->byRawDump) {
				OutputDiscLogA("Skiped because of no BCA data\n\n");
				continue;
			}
			else if (pEntry->FormatCode == 0x05) {
				OutputDiscLogA("Skiped. If you see the detailed, use /c option\n\n");
				continue;
			}
			else if (pEntry->FormatCode == 0x06 || pEntry->FormatCode == 0x07) {
				if (pDisc->DVD.protect == cprm) {
					ExecReadingKey(pDevice);
				}
				else {
					OutputDiscLogA("Skiped because of DVD with CPRM only\n\n");
					continue;
				}
			}
			else if ((0x08 <= pEntry->FormatCode && pEntry->FormatCode <= 0x0b) &&
				pDisc->SCSI.wCurrentMedia != ProfileDvdRam) {
				OutputDiscLogA("Skiped because of DVD-RAM only\n\n");
				continue;
			}
			else if ((0x0c <= pEntry->FormatCode && pEntry->FormatCode <= 0x10) &&
				(pDisc->SCSI.wCurrentMedia != ProfileDvdRecordable) &&
				(pDisc->SCSI.wCurrentMedia != ProfileDvdRWSequential)) {
				OutputDiscLogA("Skiped because of DVD-R, RW only\n\n");
				continue;
			}
			else if ((pEntry->FormatCode == 0x11 || pEntry->FormatCode == 0x30) &&
				(pDisc->SCSI.wCurrentMedia != ProfileDvdPlusRW) &&
				(pDisc->SCSI.wCurrentMedia != ProfileDvdPlusR)) {
				OutputDiscLogA("Skiped because of DVD+R, RW only\n\n");
				continue;
			}
			else if ((0x20 <= pEntry->FormatCode && pEntry->FormatCode <= 0x24) &&
				pDisc->SCSI.wCurrentMedia != ProfileDvdDashRDualLayer) {
				OutputDiscLogA("Skiped because of DVD-R DL only\n\n");
				continue;
			}
			else if ((pEntry->FormatCode == 0xc0) &&
				pDisc->SCSI.wCurrentMedia != ProfileDvdRewritable) {
				OutputDiscLogA("Skiped because of DVD-R Rewritable only\n\n");
				continue;
			}
		}
		else if (*pExecType == bd) {
			if ((pEntry->FormatCode == 0x08 || pEntry->FormatCode == 0x0a ||
				pEntry->FormatCode == 0x12 || pEntry->FormatCode == 0x30) &&
				pDisc->SCSI.wCurrentMedia != ProfileBDRSequentialWritable &&
				pDisc->SCSI.wCurrentMedia != ProfileBDRRandomWritable &&
				pDisc->SCSI.wCurrentMedia != ProfileBDRewritable) {
				OutputDiscLogA("Skiped because of BD-R, RW only\n\n");
				continue;
			}
		}
		LPBYTE lpFormat = (LPBYTE)calloc(wFormatLen, sizeof(BYTE));
		if (!lpFormat) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		cdb.LayerNumber = 0;
		cdb.Format = pEntry->FormatCode;
		REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wFormatLen);

		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, 
			lpFormat, wFormatLen, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
			byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputLogA(standardError | fileDisc, "FormatCode: %02x failed\n", pEntry->FormatCode);
		}
		else {
			if (*pExecType == dvd || *pExecType == xbox) {
				if (pEntry->FormatCode == DvdPhysicalDescriptor) {
					// PFI doesn't include the header
					fwrite(lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), sizeof(BYTE), DISC_RAW_READ_SIZE, fpPfi);
				}
				else if (pEntry->FormatCode == DvdManufacturerDescriptor) {
					// DMI doesn't include the header
					fwrite(lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), sizeof(BYTE), DISC_RAW_READ_SIZE, fpDmi);
				}
				DWORD dwSectorLen = 0;
				OutputDVDStructureFormat(pDisc, pEntry->FormatCode, wFormatLen - sizeof(DVD_DESCRIPTOR_HEADER)
					, lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), &dwSectorLen);

				if (pEntry->FormatCode == DvdPhysicalDescriptor) {
					PDVD_FULL_LAYER_DESCRIPTOR dvdpd = (PDVD_FULL_LAYER_DESCRIPTOR)(lpFormat + sizeof(DVD_DESCRIPTOR_HEADER));
					// Parallel Track Path and Dual Layer
					if (dvdpd->commonHeader.TrackPath == 0 && dvdpd->commonHeader.NumberOfLayers == 1) {
						cdb.LayerNumber = 1;

						if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH,
							lpFormat, wFormatLen, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
							byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
							OutputLogA(standardError | fileDisc, "FormatCode: %02x failed\n", pEntry->FormatCode);
							if (pEntry->FormatCode == DvdPhysicalDescriptor) {
								ReadTOC(pExtArg, pExecType, pDevice, pDisc);
							}
						}
						else {
							DWORD dwSectorLen2 = 0;
							OutputDVDStructureFormat(pDisc, pEntry->FormatCode, wFormatLen - sizeof(DVD_DESCRIPTOR_HEADER)
								, lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), &dwSectorLen2);
							OutputDiscLogA("\tLayerAllSector : % 7lu (%#lx)\n", dwSectorLen + dwSectorLen2, dwSectorLen + dwSectorLen2);
							dwSectorLen += dwSectorLen2;
						}
					}
					pDisc->SCSI.nAllLength = (INT)dwSectorLen;
				}
			}
			else if (*pExecType == bd) {
				if (pEntry->FormatCode == 0) {
					// PIC includes the header
					fwrite(lpFormat, sizeof(BYTE), wFormatLen, fpPic);
				}
				OutputBDStructureFormat(pEntry->FormatCode,
					wFormatLen - sizeof(DVD_DESCRIPTOR_HEADER),	lpFormat + sizeof(DVD_DESCRIPTOR_HEADER));
			}
		}
		OutputDiscLogA("\n");
		FreeAndNull(lpFormat);
	}
	// FormatCode: 00 failed (for gamecube. wii disc does success)
	if (pDisc->SCSI.nAllLength == 0) {
		if (pDisc->DVD.disc == DISC_TYPE::gamecube) {
			pDisc->SCSI.nAllLength = GAMECUBE_SIZE;
		}
		else {
			pDisc->SCSI.nAllLength = WII_SL_SIZE;
		}
	}
	if (*pExecType == dvd || *pExecType == xbox) {
		FcloseAndNull(fpPfi);
		FcloseAndNull(fpDmi);
	}
	else if (*pExecType == bd) {
		FcloseAndNull(fpPic);
	}
	return TRUE;
}

// http://www.geocities.co.jp/SiliconValley-SanJose/2771/scsiman.txt
BOOL ReadCapacity(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	CDB::_CDB10 cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_CAPACITY;

	BYTE buf[8] = { 0 };
	BYTE scsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH,
		&buf, sizeof(buf), &scsiStatus, _T(__FUNCTION__), __LINE__)
		|| scsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	DWORD len = MAKEDWORD(MAKEWORD(buf[3], buf[2]), MAKEWORD(buf[1], buf[0]));
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(ReadCapacity));
	OutputDiscLogA("\tMax LBA + 1: %ld (0x%lx)\n", len + 1, len + 1);
	return TRUE;
}

/*
AD 00 FF 02 FD FF FE 00 08 00 xx C0		, This is the well known SS extract commands from the xtreme FW.				  
*/
BOOL ExtractSecuritySector(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath
) {
	_TCHAR szPath[_MAX_PATH] = { 0 };
	_tcsncpy(szPath, pszFullPath, _MAX_PATH);
	if (!PathRemoveFileSpec(szPath)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fp = NULL;
	if (NULL == (fp = CreateOrOpenFile(
		szPath, _T("\\SS"), NULL, NULL, NULL, _T(".bin"), _T("wb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	lpCmd[0] = 0xad;
	lpCmd[2] = 0xff;
	lpCmd[3] = 0x02;
	lpCmd[4] = 0xfd;
	lpCmd[5] = 0xff;
	lpCmd[6] = 0xfe;
	lpCmd[8] = 0x08;
	lpCmd[11] = 0xc0;
	BYTE cmd[5] = { 0 };
	if (pDisc->SCSI.nAllLength == 3697696) {
		// http://beta.ivc.no/wiki/index.php/Xbox_360_Hacks#Save_security-sector
		// https://team-xecuter.com/forums/threads/42585-Xtreme-firmware-2-0-for-TS-H943-Xbox-360
		cmd[0] = 0x01;
		cmd[1] = 0x03;
		cmd[2] = 0x05;
		cmd[3] = 0x07;
	}
	BYTE buf[DISC_RAW_READ_SIZE] = { 0 };
	BYTE byScsiStatus = 0;
	INT i = 0;
	do {
		lpCmd[10] = cmd[i];
		if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB12GENERIC_LENGTH,
			buf, DISC_RAW_READ_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
	} while (cmd[++i] != 0);

	if (pDisc->SCSI.nAllLength == 3697696) {
		//Fix standard SSv1 ss.bin
		buf[552] = 0x01;
		buf[553] = 0x00;
		buf[555] = 0x00;
		buf[556] = 0x00;

		buf[561] = 0x5B;
		buf[562] = 0x00;
		buf[564] = 0x00;
		buf[565] = 0x00;

		buf[570] = 0xB5;
		buf[571] = 0x00;
		buf[573] = 0x00;
		buf[574] = 0x00;

		buf[579] = 0x0f;
		buf[580] = 0x01;
		buf[582] = 0x00;
		buf[583] = 0x00;
	}
	fwrite(buf, sizeof(BYTE), (size_t)DISC_RAW_READ_SIZE, fp);
	FcloseAndNull(fp);

	OutputXboxSecuritySector(pDisc, buf);
	return TRUE;
}

/*
FF 08 01 01				, 'Enable Unlock 1 (xtreme) state' as we already know it from the 360 xtreme modded drives. 
						  This command is supported for legacy reasons only. Custom applications should use the new
						  'Set lock state' instead.
*/

/*
FF 08 01 10				, 'Get Feature List' 
						  This command will return a list of the additional features supported by the drive.
						  All values returned are 16 bit values, and the list is terminated with null (0x0000)
						  The two first words of the returned list always reads as 0xA55A 0X5AA5 in order to guarantee
						  that a reply from a drive not supporting this command correctly isn't mistaken for a feature list.
												
						  An example feature list could be:
						  0xA55A, 0x5AA5, 0x0100, 0xF000, 0xF001, 0x0000
														
						  This list would indicate that the drive supports XBOX360 Unlock 1, Lock and Error Skip, as it can be seen 
						  from the values defined below:
*/
BOOL GetFeatureListForXBox(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	BYTE lpCmd[CDB6GENERIC_LENGTH] = { 0 };
	lpCmd[0] = 0xff;
	lpCmd[1] = 0x08;
	lpCmd[2] = 0x01;
	lpCmd[3] = 0x10;

	_declspec(align(4)) BYTE buf[26] = { 0 };
	BYTE scsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB6GENERIC_LENGTH,
		buf, 26, &scsiStatus, _T(__FUNCTION__), __LINE__)
		|| scsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputDriveLogA(OUTPUT_DHYPHEN_PLUS_STR(FeatureList));
	for (INT i = 0; i < 26; i += 2) {
		WORD list = MAKEWORD(buf[i + 1], buf[i]);
		switch (list) {
		case 0:
			break;
		case 0x100:
			OutputDriveLogA("0x%04x : The drive supports the unlock 1 state (xtreme) for XBOX 360\n", list);
			break;
		case 0x101:
			OutputDriveLogA("0x%04x : The drive supports the unlock 2 state (wxripper) for XBOX 360\n", list);
			break;
		case 0x120:
			OutputDriveLogA("0x%04x : The drive supports the unlock 1 state (xtreme) for XBOX 360\n", list);
			break;
		case 0x121:
			OutputDriveLogA("0x%04x : The drive has full challenge response functionality for XBOX 360\n", list);
			break;
		case 0x200:
			OutputDriveLogA("0x%04x : The drive supports the unlock 1 state (xtreme) for XBOX\n", list);
			break;
		case 0x201:
			OutputDriveLogA("0x%04x : The drive supports the unlock 2 state (wxripper) for XBOX\n", list);
			break;
		case 0x220:
			OutputDriveLogA("0x%04x : The drive supports the unlock 1 state (xtreme) for XBOX\n", list);
			break;
		case 0x221:
			OutputDriveLogA("0x%04x : The drive has full challenge response functionality for XBOX\n", list);
			break;
		case 0xF000:
			OutputDriveLogA("0x%04x : The drive supports the lock (cancel any unlock state) command\n", list);
			break;
		case 0xF001:
			OutputDriveLogA("0x%04x : The drive supports error skipping\n", list);
			break;
		}
	}
	OutputString("\n");
	return TRUE;
}

/*																				
FF 08 01 11 xx			, 'Set Lock State'
						  xx=00 - Drive locked (no unlock state)
						  xx=01 - Unlock State 1 (xtreme) enabled
						  xx=02 - Unlock state 2 (wxripper) enabled
*/
BOOL SetLockState(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BYTE byState
) {
	BYTE lpCmd[CDB6GENERIC_LENGTH] = { 0 };
	lpCmd[0] = 0xff;
	lpCmd[1] = 0x08;
	lpCmd[2] = 0x01;
	lpCmd[3] = 0x11;
	lpCmd[4] = byState;

	BYTE scsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB6GENERIC_LENGTH,
		NULL, 0, &scsiStatus, _T(__FUNCTION__), __LINE__)
		|| scsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

/*
FF 08 01 15 xx			, 'Set Error Skip State'
						  xx=00 - Error skip is disabled
						  xx=01 - Error skip is enabled
*/
BOOL SetErrorSkipState(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BYTE byState
) {
	BYTE lpCmd[CDB6GENERIC_LENGTH] = { 0 };
	lpCmd[0] = 0xff;
	lpCmd[1] = 0x08;
	lpCmd[2] = 0x01;
	lpCmd[3] = 0x15;
	lpCmd[4] = byState;

	BYTE scsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB6GENERIC_LENGTH,
		NULL, 0, &scsiStatus, _T(__FUNCTION__), __LINE__)
		|| scsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

BOOL ReadXboxDVD(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath
) {
	if (!GetFeatureListForXBox(pExtArg, pDevice)) {
		return FALSE;
	}

	if (!SetLockState(pExtArg, pDevice, 0)) {
		return FALSE;
	}
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(Lock state));
	if (!ReadCapacity(pExtArg, pDevice)) {
		return FALSE;
	}
	if (!ReadTOC(pExtArg, pExecType, pDevice, pDisc)) {
		return FALSE;
	}
	if (!ReadDiscStructure(pExecType, pExtArg, pDevice, pDisc, pszFullPath)) {
		return FALSE;
	}

	if (!SetLockState(pExtArg, pDevice, 1)) {
		return FALSE;
	}
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(Unlock 1 state(xtreme)));
	if (!ReadCapacity(pExtArg, pDevice)) {
		return FALSE;
	}
	if (!ReadTOC(pExtArg, pExecType, pDevice, pDisc)) {
		return FALSE;
	}

	if (!ExtractSecuritySector(pExtArg, pDevice, pDisc, pszFullPath)) {
		return FALSE;
	}

	if (!SetLockState(pExtArg, pDevice, 2)) {
		return FALSE;
	}
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(Unlock 2 state(wxripper)));
	if (!ReadCapacity(pExtArg, pDevice)) {
		return FALSE;
	}
	if (!ReadTOC(pExtArg, pExecType, pDevice, pDisc)) {
		return FALSE;
	}

	if (!SetErrorSkipState(pExtArg, pDevice, 1)) {
		return FALSE;
	}
	// xbox total size
	// dvd partition: layer0 + layer1 + middle zone (startPsn of xbox - startPsn of dvd - layer0)
	// xbox partition: layer0 + layer1 + middle zone (ditto)
	//
	// This func reads as follows
	// layer 0 of dvd [startPsn of dvd (0x30000) to startPsn of xbox (0x60600)] -> 0x30600
	// layer 0 of xbox [startPsn of xbox (0x60600) to the size of toc] -> depend on the disc
	// layer 1 of middle zone
	// layer 1 of dvd
	if (!ReadDVD(pExecType, pExtArg, pDevice, pDisc, pszFullPath)) {
		return FALSE;
	}
	return TRUE;
}
