/**
 * Copyright 2011-2021 sarami
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
#include "convert.h"
#include "execIoctl.h"
#include "execScsiCmd.h"
#include "execScsiCmdforDVD.h"
#include "execScsiCmdforFileSystem.h"
#include "get.h"
#include "output.h"
#include "outputScsiCmdLogforCD.h"
#include "outputScsiCmdLogforDVD.h"

#define GAMECUBE_SIZE		(712880)
#define WII_SL_SIZE			(2294912)
#define WII_DL_SIZE			(4155840)
#define XBOX_SIZE			(3820880)
#define XBOX_LAYER_BREAK	(1913776)
#define XGD2_LAYER_BREAK	(1913760)
#define XGD3_LAYER_BREAK	(2133520)

BOOL ReadDVDReverse(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPCTSTR pszFullPath,
	INT nStartLBA,
	INT nLastLBA
) {
	FILE* fpRev = CreateOrOpenFile(
		pszFullPath, _T("_reverse"), NULL, NULL, NULL, _T(".iso"), _T("wb"), 0, 0);
	if (!fpRev) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	try {
		if (NULL == (pBuf = (LPBYTE)calloc(
			DISC_MAIN_DATA_SIZE + pDevice->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);

		CDB::_READ12 cdb = {};
		cdb.OperationCode = SCSIOP_READ12;
		FOUR_BYTE transferLen;
		transferLen.AsULong = 1;
		REVERSE_BYTES(&cdb.TransferLength, &transferLen);
		if (pExtArg->byFua) {
			cdb.ForceUnitAccess = TRUE;
		}
#ifdef _WIN32
		INT direction = SCSI_IOCTL_DATA_IN;
#else
		INT direction = SG_DXFER_FROM_DEV;
#endif
		BYTE byScsiStatus = 0;
		FOUR_BYTE LBA;
		for (LBA.AsULong = (ULONG)nLastLBA; (ULONG)nStartLBA <= LBA.AsULong; LBA.AsULong--) {
			REVERSE_BYTES(&cdb.LogicalBlock, &LBA);
			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				direction, DISC_MAIN_DATA_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			fwrite(lpBuf, sizeof(BYTE), (size_t)DISC_MAIN_DATA_SIZE, fpRev);
			OutputString("\rCreating iso(LBA) %8lu/%8d", LBA.AsULong, nLastLBA);
		}
		OutputString("\n");
		FcloseAndNull(fpRev);

		FILE* fp = CreateOrOpenFile(
			pszFullPath, NULL, NULL, NULL, NULL, _T(".iso"), _T("wb"), 0, 0);
		if (!fp) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		fpRev = CreateOrOpenFile(
			pszFullPath, _T("_reverse"), NULL, NULL, NULL, _T(".iso"), _T("rb"), 0, 0);
		if (!fpRev) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		for (INT i = 1; i <= nLastLBA - nStartLBA + 1; i++) {
			fseek(fpRev, -DISC_MAIN_DATA_SIZE * i, SEEK_END);
			if (fread(lpBuf, sizeof(BYTE), DISC_MAIN_DATA_SIZE, fpRev) < DISC_MAIN_DATA_SIZE) {
				OutputErrorString("Failed to read [F:%s][L:%d]\n", _T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			fwrite(lpBuf, sizeof(BYTE), DISC_MAIN_DATA_SIZE, fp);
		}
		FcloseAndNull(fp);
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FreeAndNull(pBuf);
	FcloseAndNull(fpRev);
	return bRet;
}

BOOL ReadDVD(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath,
	PHASH pHash
) {
	_TCHAR szFnameAndExt[_MAX_FNAME + _MAX_EXT] = {};
	FILE* fp = CreateOrOpenFile(
		pszFullPath, NULL, NULL, szFnameAndExt, NULL, _T(".iso"), _T("wb"), 0, 0);
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (IsXbox(pExecType) && pExtArg->byNoSkipSS) {
		pDisc->PROTECT.byExist = physicalErr;
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

		CDB::_READ12 cdb = {};
		cdb.OperationCode = SCSIOP_READ12;
		if (pExtArg->byFua) {
			cdb.ForceUnitAccess = TRUE;
		}

		DWORD dwLayer0MiddleZone =
			pDisc->DVD.dwXboxStartPsn - pDisc->DVD.dwDVDStartPsn - pDisc->DVD.dwLayer0SectorLength;
		DWORD dwLayer1MiddleZone = dwLayer0MiddleZone;
		INT nAllLength = pDisc->SCSI.nAllLength;
		if (*pExecType == xbox) {
			OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("TotalLength")
				"\t    L0 + L1 data: %7d (%#x)\n"
				"\t+      L1 Middle: %7lu (%#lx)\n"
				"\t+       L1 Video: %7lu (%#lx)\n"
				"\t------------------------------------\n"
				, nAllLength, (UINT)nAllLength
				, dwLayer1MiddleZone, dwLayer1MiddleZone
				, pDisc->DVD.dwLayer1SectorLength, pDisc->DVD.dwLayer1SectorLength
			);
			nAllLength += (INT)(dwLayer1MiddleZone + pDisc->DVD.dwLayer1SectorLength);
			OutputDiscLog(
				"\t                  %7d (%#x)\n", nAllLength, (UINT)nAllLength);
#if 1
			if (nAllLength > 4000000) {
				INT nAdditional = 4096;
				nAllLength += nAdditional;
				OutputDiscLog(
						"\t+     Additional: %7d (%#x)\n"
						"\t------------------------------------\n"
						"\t                  %7d (%#x)\n"
						, nAdditional, (UINT)nAdditional, nAllLength, (UINT)nAllLength
				);
			}
#endif
			OutputDiscLog("\tLayerBreak: %7lu (L0 Video: %lu, L0 Middle: %lu, L0 Game: %lu)\n"
				, pDisc->DVD.dwLayer0SectorLength + dwLayer0MiddleZone + pDisc->DVD.dwXboxLayer0SectorLength
				, pDisc->DVD.dwLayer0SectorLength, dwLayer0MiddleZone, pDisc->DVD.dwXboxLayer0SectorLength
			);
		}
		else if (*pExecType == xboxswap) {
			pDisc->SCSI.nAllLength = XBOX_SIZE + (INT)pDisc->DVD.dwXboxSwapOfs;
			nAllLength = pDisc->SCSI.nAllLength;
		}
		else if (*pExecType == xgd2swap || *pExecType == xgd3swap) {
			pDisc->SCSI.nAllLength = pExtArg->nAllSectors + (INT)pDisc->DVD.dwXboxSwapOfs;
			nAllLength = pDisc->SCSI.nAllLength;
		}

		if (!ReadDVDForFileSystem(pExecType, pExtArg, pDevice, pDisc, &cdb, lpBuf)) {
			throw FALSE;
		}

		if (pDisc->DVD.protect != noProtect) {
			if (ExecReadingKey(pDevice, pDisc->DVD.protect, pszFullPath)) {
				OutputDiscLog("Outputted to _xxxKey.txt\n\n");
			}
			else {
				OutputDiscLog("Failed to get _xxxKey.txt\n\n");
			}
		}

		if (pExtArg->byAnchorVolumeDescriptorPointer) {
			nAllLength = pDisc->SCSI.nAllLength;
		}

		if (*pExecType == xbox) {
			if (!ReadXBOXFileSystem(pExtArg, pDevice, pDisc->DVD.dwXboxStartPsn - pDisc->DVD.dwDVDStartPsn)) {
				throw FALSE;
			}
		}
		else if (*pExecType == bd) {
			for (INT i = 0; i < MAX_PARAMSFO_NUM; i++) {
				if (pDisc->BD.nLBAForParamSfo[i] != 0) {
					if (!ReadBDForParamSfo(pExtArg, pDevice, pDisc, &cdb, lpBuf, i)) {
						throw FALSE;
					}
					else {
						break;
					}
				}
			}
		}
		FlushLog();
		CalcInit(&pHash->pHashChunk[pHash->uiIndex].md5, &pHash->pHashChunk[pHash->uiIndex].sha);
		if (pExtArg->byDatExpand) {
			CalcInitExpand(&pHash->pHashChunk[pHash->uiIndex].sha224, &pHash->pHashChunk[pHash->uiIndex].sha256
				, &pHash->pHashChunk[pHash->uiIndex].sha384, &pHash->pHashChunk[pHash->uiIndex].sha512);
		}

		FOUR_BYTE transferLen;
		transferLen.AsULong = pDevice->dwMaxTransferLength / DISC_MAIN_DATA_SIZE;
		REVERSE_BYTES(&cdb.TransferLength, &transferLen);
		BYTE byScsiStatus = 0;
		INT i = 0;
		UINT uiRetryCnt = 0;
		if (pDisc->PROTECT.byExist == arccos || pDisc->PROTECT.byExist == ripGuard) {
			transferLen.AsULong = 1;
			REVERSE_BYTES(&cdb.TransferLength, &transferLen);
		}
		DWORD dwTransferLenOrg = transferLen.AsULong;
		INT nFirstErrorLBA = 0;
		INT nLastErrorLBA = 0;
		BOOL bErrorForward = FALSE;
		BOOL bErrorBack = FALSE;
		UINT uiErrorForwardTimes = 0;
		UINT uiErrorBackTimes = 0;
		BOOL bSetErrorSectorRange = FALSE;
#ifdef _WIN32
		INT direction = SCSI_IOCTL_DATA_IN;
#else
		INT direction = SG_DXFER_FROM_DEV;
#endif
		FOUR_BYTE LBA;
		for (INT nLBA = 0; nLBA < pDisc->SCSI.nAllLength; nLBA += (INT)transferLen.AsULong) {
			if (IsXbox(pExecType)) {
				if ((nLBA == XBOX_LAYER_BREAK && *pExecType == xboxswap) ||
					(nLBA == XGD2_LAYER_BREAK && *pExecType == xgd2swap) || 
					(nLBA == XGD3_LAYER_BREAK && *pExecType == xgd3swap)) {
					nLBA += (INT)pDisc->DVD.dwXboxSwapOfs;
					transferLen.AsULong = dwTransferLenOrg;
					REVERSE_BYTES(&cdb.TransferLength, &transferLen);
				}
				if (pDisc->DVD.securitySectorRange[i][0] <= (DWORD)nLBA &&
					(DWORD)nLBA <= pDisc->DVD.securitySectorRange[i][1] + 1) {
					if ((DWORD)nLBA == pDisc->DVD.securitySectorRange[i][1] + 1) {
						i++;
						if (pExtArg->byNoSkipSS) {
							bSetErrorSectorRange = FALSE;
							transferLen.AsULong = dwTransferLenOrg;
							REVERSE_BYTES(&cdb.TransferLength, &transferLen);
						}
					}
					else {
						if (pExtArg->byNoSkipSS) {
							transferLen.AsULong = 1;
							REVERSE_BYTES(&cdb.TransferLength, &transferLen);
						}
						else {
							if (transferLen.AsULong != dwTransferLenOrg) {
								transferLen.AsULong = dwTransferLenOrg;
								REVERSE_BYTES(&cdb.TransferLength, &transferLen);
							}
							ZeroMemory(lpBuf, DISC_MAIN_DATA_SIZE * transferLen.AsULong);
							WriteBufWithCalc(pExtArg, lpBuf, DISC_MAIN_DATA_SIZE, transferLen.AsULong, fp, pHash);
							continue;
						}
					}
				}
				else if (transferLen.AsULong > (DWORD)(pDisc->DVD.securitySectorRange[i][0] - nLBA)) {
					transferLen.AsULong = (DWORD)(pDisc->DVD.securitySectorRange[i][0] - nLBA);
					REVERSE_BYTES(&cdb.TransferLength, &transferLen);
				}
			}

			if (*pExecType == xboxswap && transferLen.AsULong > (DWORD)(XBOX_LAYER_BREAK - nLBA) && XBOX_LAYER_BREAK - nLBA != 0) {
				transferLen.AsULong = (DWORD)(XBOX_LAYER_BREAK - nLBA);
				REVERSE_BYTES(&cdb.TransferLength, &transferLen);
			}
			else if (*pExecType == xgd2swap && transferLen.AsULong > (DWORD)(XGD2_LAYER_BREAK - nLBA) && XGD2_LAYER_BREAK - nLBA != 0) {
				transferLen.AsULong = (DWORD)(XGD2_LAYER_BREAK - nLBA);
				REVERSE_BYTES(&cdb.TransferLength, &transferLen);
			}
			else if (*pExecType == xgd3swap && transferLen.AsULong > (DWORD)(XGD3_LAYER_BREAK - nLBA) && XGD3_LAYER_BREAK - nLBA != 0) {
				transferLen.AsULong = (DWORD)(XGD3_LAYER_BREAK - nLBA);
				REVERSE_BYTES(&cdb.TransferLength, &transferLen);
			}
			else if (transferLen.AsULong != 1 && pDisc->PROTECT.byExist == physicalErr &&
				IsValidProtectedSector(pDisc, nLBA - 1, GetReadErrorFileIdx(pExtArg, pDisc, nLBA))) {
				transferLen.AsULong = 1;
				REVERSE_BYTES(&cdb.TransferLength, &transferLen);
			}
			else if (transferLen.AsULong > (DWORD)(pDisc->SCSI.nAllLength - nLBA)) {
				transferLen.AsULong = (DWORD)(pDisc->SCSI.nAllLength - nLBA);
				REVERSE_BYTES(&cdb.TransferLength, &transferLen);
			}

			if ((pDisc->PROTECT.byExist == physicalErr || pDisc->PROTECT.byExist == arccos || pDisc->PROTECT.byExist == ripGuard)
				&& nFirstErrorLBA != 0 && nFirstErrorLBA <= nLBA && nLBA <= nLastErrorLBA) {
				FillMemory(lpBuf, DISC_MAIN_DATA_SIZE * transferLen.AsULong, 0x00);
				WriteBufWithCalc(pExtArg, lpBuf, DISC_MAIN_DATA_SIZE, transferLen.AsULong, fp, pHash);
				if (nLBA == nLastErrorLBA) {
					nFirstErrorLBA = 0;
					OutputLog(standardOut | fileMainError, "Reset 1st error LBA\n");
				}
				continue;
			}
			LBA.AsULong = (ULONG)nLBA;
			REVERSE_BYTES(&cdb.LogicalBlock, &LBA);
			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				direction, DISC_MAIN_DATA_SIZE * transferLen.AsULong, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				if (IsXbox(pExecType) && !(pDisc->DVD.securitySectorRange[i][0] <= (DWORD)nLBA &&
					(DWORD)nLBA <= pDisc->DVD.securitySectorRange[i][1] + 1)) {
					if (++uiRetryCnt <= pExtArg->uiMaxRereadNum) {
						nLBA -= (INT)transferLen.AsULong;
						OutputLog(standardOut | fileMainError,
							"This sector is out of the ss ranges. Read retry (Pass %u/%u)\n", uiRetryCnt, pExtArg->uiMaxRereadNum);
						continue;
					}
					else {
						OutputLog(standardOut | fileMainError, "Retry NG\n");
						throw FALSE;
					}
				}
				
				if (pDisc->PROTECT.byExist == physicalErr || pDisc->PROTECT.byExist == arccos || pDisc->PROTECT.byExist == ripGuard) {
					if (IsXbox(pExecType) && bSetErrorSectorRange &&
						nLastErrorLBA <= nLBA && nLBA <= (INT)pDisc->DVD.securitySectorRange[i][1]) {
						if (++uiErrorForwardTimes <= pExtArg->uiMaxRereadNum) {
							OutputLog(standardOut | fileMainError
								, "Reread this sector: %u/%u\n"
								, uiErrorForwardTimes, pExtArg->uiMaxRereadNum);
							nLBA--;
							continue;
						}
						else {
							uiErrorForwardTimes = 0;
							bSetErrorSectorRange = FALSE;
							OutputLog(standardOut | fileMainError, "Reread NG -> Filled with 0x00\n");
							FillMemory(lpBuf, DISC_MAIN_DATA_SIZE * transferLen.AsULong, 0x00);
							WriteBufWithCalc(pExtArg, lpBuf, DISC_MAIN_DATA_SIZE, transferLen.AsULong, fp, pHash);
							continue;
						}
					}
					if (!bErrorBack) {
						if (nFirstErrorLBA == 0) {
							if (++uiErrorForwardTimes <= pExtArg->uiMaxRereadNum) {
								OutputLog(standardOut | fileMainError
									, "Reread this 1st error sector: %u/%u\n"
									, uiErrorForwardTimes, pExtArg->uiMaxRereadNum);
								nLBA--;
								continue;
							}
							uiErrorForwardTimes = 0;
							OutputLog(standardOut | fileMainError, "Set 1st error LBA\n");
							nFirstErrorLBA = nLBA;
							bErrorForward = TRUE;
						}
						if (pExtArg->uiSkipSectors) {
							nLBA += pExtArg->uiSkipSectors - 1;
						}
						continue;
					}
					else {
						if (++uiErrorBackTimes <= pExtArg->uiMaxRereadNum) {
							nLBA--;
							continue;
						}
						nLastErrorLBA = nLBA;
						nLBA = nFirstErrorLBA - 1;
						OutputLog(standardOut | fileDisc, "Error sectors range: LBA %d to %d = %d -> Filled with 0x00\n"
							, nFirstErrorLBA, nLastErrorLBA, nLastErrorLBA - nFirstErrorLBA + 1);
						FlushLog();
						bErrorBack = FALSE;
						bSetErrorSectorRange = TRUE;
						continue;
					}
				}
				else if (++uiRetryCnt <= pExtArg->uiMaxRereadNum) {
					OutputLog(standardOut | fileMainError, "Read retry from %d (Pass %u/%u)\n", nLBA, uiRetryCnt, pExtArg->uiMaxRereadNum);
					if (pExtArg->byPadSector) {
						if (transferLen.AsULong != 1) {
							OutputLog(standardOut | fileMainError,
								"Change the transfer length to 1\n");
							transferLen.AsULong = 1;
							REVERSE_BYTES(&cdb.TransferLength, &transferLen);
						}
					}
					nLBA -= (INT)transferLen.AsULong;
					continue;
				}
				else {
					if (pExtArg->byPadSector) {
						if (pExtArg->uiPadNum == 0) {
							OutputLog(standardOut | fileMainError, "Padded by 0x00\n");
							ZeroMemory(lpBuf, DISC_MAIN_DATA_SIZE);
						}
						else if (pExtArg->uiPadNum == 1) {
							OutputLog(standardOut | fileMainError, "Padded by 0xAA\n");
							FillMemory(lpBuf, DISC_MAIN_DATA_SIZE, 0xaa);
						}
						uiRetryCnt = 0;
					}
					else {
						OutputString("Retry NG\n");
						throw FALSE;
					}
				}
			}

			if (bErrorForward) {
				bErrorForward = FALSE;
				bErrorBack = TRUE;
				if (uiErrorForwardTimes) {
					OutputLog(standardOut | fileMainError, "Reread OK\n");
				}
				uiErrorForwardTimes = 0;
			}
			if (bErrorBack) {
				OutputLog(standardOut | fileMainError, STR_LBA "Read back a sector\n", nLBA, (UINT)nLBA);
				nLBA -= 2;
				uiErrorBackTimes = 0;
				continue;
			}
			if (uiRetryCnt) {
				OutputLog(standardOut | fileMainError, "LBA %d is retry OK\n", nLBA);
				uiRetryCnt = 0;
			}
			WriteBufWithCalc(pExtArg, lpBuf, DISC_MAIN_DATA_SIZE, transferLen.AsULong, fp, pHash);
			OutputString("\rCreating iso(LBA) %8lu/%8d", nLBA + transferLen.AsULong, nAllLength);
		}

		if (*pExecType == xbox) {
			if (!SetLockState(pExtArg, pDevice, 0)) {
				throw FALSE;
			}
			transferLen.AsULong = dwTransferLenOrg;
			ZeroMemory(lpBuf, DISC_MAIN_DATA_SIZE * transferLen.AsULong);
			DWORD dwEndOfMiddle = pDisc->SCSI.nAllLength + dwLayer1MiddleZone;
#if 1
			if (nAllLength > 4000000) {
				dwEndOfMiddle += 4096;
			}
#endif
			for (DWORD j = (DWORD)pDisc->SCSI.nAllLength; j < dwEndOfMiddle; j += transferLen.AsULong) {
				if (transferLen.AsULong > dwEndOfMiddle - j) {
					transferLen.AsULong = dwEndOfMiddle - j;
				}
				WriteBufWithCalc(pExtArg, lpBuf, DISC_MAIN_DATA_SIZE, transferLen.AsULong, fp, pHash);
				OutputString("\rCreating iso(LBA) %8lu/%8d", j + transferLen.AsULong, nAllLength);
			}

			transferLen.AsULong = dwTransferLenOrg;
			REVERSE_BYTES(&cdb.TransferLength, &transferLen);
			DWORD dwEndOfDvd = pDisc->DVD.dwLayer0SectorLength + pDisc->DVD.dwLayer1SectorLength;

			FOUR_BYTE tmpLen;
			for (tmpLen.AsULong = pDisc->DVD.dwLayer0SectorLength; tmpLen.AsULong < dwEndOfDvd; tmpLen.AsULong += transferLen.AsULong) {
				if (transferLen.AsULong > dwEndOfDvd - tmpLen.AsULong) {
					transferLen.AsULong = dwEndOfDvd - tmpLen.AsULong;
					REVERSE_BYTES(&cdb.TransferLength, &transferLen);
				}
				REVERSE_BYTES(&cdb.LogicalBlock, &tmpLen);
				if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
					direction, DISC_MAIN_DATA_SIZE * transferLen.AsULong, &byScsiStatus, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					throw FALSE;
				}
				WriteBufWithCalc(pExtArg, lpBuf, DISC_MAIN_DATA_SIZE, transferLen.AsULong, fp, pHash);
				OutputString("\rCreating iso(LBA) %8lu/%8d", dwEndOfMiddle + pDisc->DVD.dwLayer1SectorLength, nAllLength);
			}
		}
		_tcsncpy(pHash->pHashChunk[pHash->uiIndex].szFnameAndExt, szFnameAndExt, sizeof(szFnameAndExt));
		pHash->pHashChunk[pHash->uiIndex].ui64FileSize = DISC_MAIN_DATA_SIZE * (UINT64)nAllLength;
		OutputString("\n");
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
		!strncmp(pDevice->szProductId, "RW/DVD GCC-4240N", DRIVE_PRODUCT_ID_SIZE)
		) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsSupported0xE7Type2_1(
	PDEVICE pDevice
) {
	if (!strncmp(pDevice->szProductId, "RW/DVD GCC-4241N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD_GCC-4241N", DRIVE_PRODUCT_ID_SIZE)
		) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsSupported0xE7Type2_2(
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
	if (!strncmp(pDevice->szProductId, "CDRW/DVD GCC4244", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "CDRW/DVD GCC4247", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8083N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8084N", DRIVE_PRODUCT_ID_SIZE) ||
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
		IsSupported0xE7Type2_1(pDevice) ||
		IsSupported0xE7Type2_2(pDevice) ||
		IsSupported0xE7Type3(pDevice) ||
		IsSupported0xE7Type4(pDevice)) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsLiteOn(
	PDEVICE pDevice
) {
	if (!strncmp(pDevice->szVendorId, "LITE-ON ", DRIVE_VENDOR_ID_SIZE)) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsNintendoDisc(
	PDISC pDisc
) {
	if (pDisc->SCSI.nAllLength == GAMECUBE_SIZE ||
		pDisc->SCSI.nAllLength == WII_SL_SIZE ||
		pDisc->SCSI.nAllLength == WII_DL_SIZE) {
		return TRUE;
	}
	return FALSE;
}

BOOL ReadDVDRaw(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath,
	PHASH pHash
) {
//#define TEST_WII
#ifdef TEST_WII
	ReadWiiPartition(pDevice, pszFullPath);
	return TRUE;
#endif
	FILE* fp = NULL;
	_TCHAR szMode[4] = _T("wb");
	if (pExtArg->byResume || pExtArg->byFix) {
		memcpy(szMode, _T("rb+"), 3);
	}

	_TCHAR szFnameAndExt[_MAX_FNAME + _MAX_EXT] = {};
	_TCHAR pszOutPath[_MAX_PATH] = {};
	if (NULL == (fp = CreateOrOpenFile(pszFullPath, NULL, pszOutPath, szFnameAndExt, NULL, _T(".raw"), szMode, 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE pBuf = NULL;
	BOOL bRet = TRUE;
	try {
		FOUR_BYTE transferLen;
		transferLen.AsULong = 16;
		DWORD memBlkSize = 1;
		if (NULL == (pBuf = (LPBYTE)calloc(
			(size_t)DVD_RAW_SECTOR_SIZE * transferLen.AsULong * 5 + pDevice->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);
		INT nCmdType = 0;
		DWORD baseAddr = 0x80000000;
		DWORD dwSectorSize = DVD_RAW_SECTOR_SIZE;
		BYTE cdblen = CDB10GENERIC_LENGTH;
		// for dumping from memory
		BYTE CacheCmd[CDB12GENERIC_LENGTH] = {};
		INT nDriveSampleOffset = 0;
		if (!GetDriveOffsetAuto(pDevice, &nDriveSampleOffset)) {
			GetDriveOffsetManually(&nDriveSampleOffset);
		}
		// Panasonic MN103S chip
		if (nDriveSampleOffset == 102) {
			if (IsSupported0xE7(pDevice)) {
				if (IsSupported0xE7Type1(pDevice)) {
					// address which data frame is cached
					// a08000 - a0ffff (8000), a13000 - a1ffff (d000), a23000 - a2ffff (d000)
					// a48000 - a4ffff (8000), a53000 - a5ffff (d000), a63000 - a6ffff (d000)
					// a88000 - a8ffff (8000), a93000 - a9ffff (d000), aa3000 - aaffff (d000)
					// ac8000 - acffff (8000), ad3000 - adffff (d000), ae3000 - aeffff (d000)
					// b08000 - b0ffff (8000), b13000 - b1ffff (d000), b23000 - b2ffff (d000)
					//  :
					baseAddr = 0xa13000;
					nCmdType = 1;
				}
				else if (IsSupported0xE7Type2_1(pDevice)) {
					transferLen.AsULong = 4;
					baseAddr -= 0x810 * transferLen.AsULong;
					nCmdType = 2;
				}
				else if (IsSupported0xE7Type2_2(pDevice)) {
					baseAddr -= 0x810 * transferLen.AsULong;
					nCmdType = 2;
				}
				else if (IsSupported0xE7Type3(pDevice) || IsSupported0xE7Type4(pDevice)) {
					memBlkSize = 5;
					nCmdType = 1;
				}
				// https://web.archive.org/web/20080629151440/http://www.xboxhacker.net/index.php?option=com_smf&Itemid=33&topic=76.msg1656;topicseen#msg1656
				CacheCmd[0] = 0xE7; // vendor specific command
				CacheCmd[1] = 0x48; // H
				CacheCmd[2] = 0x49; // I
				CacheCmd[3] = 0x54; // T
				CacheCmd[4] = 0x01; // read MCU memory sub-command
				CacheCmd[10] = HIBYTE(LOWORD(dwSectorSize * transferLen.AsULong));
				CacheCmd[11] = LOBYTE(LOWORD(dwSectorSize * transferLen.AsULong));
				cdblen = CDB12GENERIC_LENGTH;
			}
			else {
				CacheCmd[0] = SCSIOP_READ_DATA_BUFF;
				CacheCmd[1] = 0x01;
				CacheCmd[6] = LOBYTE(HIWORD(dwSectorSize * transferLen.AsULong));
				CacheCmd[7] = HIBYTE(LOWORD(dwSectorSize * transferLen.AsULong));
				CacheCmd[8] = LOBYTE(LOWORD(dwSectorSize * transferLen.AsULong));
				nCmdType = 3;
			}
		}
		// Plextor
		else if (IsValidPlextorDrive(pDevice)) {
			CacheCmd[0] = SCSIOP_READ_DATA_BUFF;
			CacheCmd[1] = 0x02;
			CacheCmd[6] = LOBYTE(HIWORD(dwSectorSize * transferLen.AsULong));
			CacheCmd[7] = HIBYTE(LOWORD(dwSectorSize * transferLen.AsULong));
			CacheCmd[8] = LOBYTE(LOWORD(dwSectorSize * transferLen.AsULong));
		}
		// Mediatek MT chip
		// LITE-ON - LH-18A1P supports
		//  0x3c 0x01 0x01(or 0x02) -> rawdata
		//   (172bytes[raw] + 10bytes[garbage?]) * 12 + 200bytes[garbage?] = 2384bytes
		//  0x3c 0x01 0xe2 and 0x3c 0x01 0xf1 -> eeprom?
		else if (IsLiteOn(pDevice)) {
			dwSectorSize = DVD_RAW_SECTOR2_SIZE;
			CacheCmd[0] = SCSIOP_READ_DATA_BUFF;
			CacheCmd[1] = 0x01;
			CacheCmd[2] = 0x01;
			CacheCmd[6] = LOBYTE(HIWORD(dwSectorSize * transferLen.AsULong));
			CacheCmd[7] = HIBYTE(LOWORD(dwSectorSize * transferLen.AsULong));
			CacheCmd[8] = LOBYTE(LOWORD(dwSectorSize * transferLen.AsULong));
		}
#if 1
		// Renesas chip
		// LG Electronics - GSA-4163B supports (lsb 3 bits is used)
		//  0x3c 0x00, 0x01 -> perhaps same as spc3r23
		//  0x3c 0x02 -> perhaps same as spc3r23, but rawdata is imcomplete
		//  0x3c 0x03 -> perhaps same as spc3r23
		//  0x3c 0x05 -> eeprom?
		else if (nDriveSampleOffset == 667) {
			CacheCmd[0] = SCSIOP_READ_DATA_BUFF;
			CacheCmd[1] = 0x02;
			CacheCmd[6] = LOBYTE(HIWORD(dwSectorSize * transferLen.AsULong));
			CacheCmd[7] = HIBYTE(LOWORD(dwSectorSize * transferLen.AsULong));
			CacheCmd[8] = LOBYTE(LOWORD(dwSectorSize * transferLen.AsULong));
		}
		// Other
		// TOSHIBA - SD-H802A doesn't support 0x3c
		// Tsstcorp - TS-H352C doesn't support 0x3c
		// Optiarc - AD-7280S supports
		//  0x3c 0x01 0x01 and 0x3c 0x01 0x02 -> eeprom?
		// ASUS - BC-12D2HT supports
		//  0x3c 0x05 -> eeprom?
		//  0x3c 0x07 -> perhaps same as 0x00 of spc3r23
		//  0x3c 0x0c -> many sectors are zero, then same the main channel?
		//  0x3c 0x0d -> same the main channel?
		else {
			CacheCmd[0] = SCSIOP_READ_DATA_BUFF;
			CacheCmd[1] = 0x02;
			CacheCmd[6] = LOBYTE(HIWORD(dwSectorSize * transferLen.AsULong));
			CacheCmd[7] = HIBYTE(LOWORD(dwSectorSize * transferLen.AsULong));
			CacheCmd[8] = LOBYTE(LOWORD(dwSectorSize * transferLen.AsULong));
		}
#endif
		OutputString("Rawdump command [0]:%#04x [1]:%#04x [2]:%#04x\n", CacheCmd[0], CacheCmd[1], CacheCmd[2]);

		// for dumping from the disc
		CDB::_READ12 ReadCdb = {};
		ReadCdb.OperationCode = SCSIOP_READ12;
		REVERSE_BYTES(&ReadCdb.TransferLength, &transferLen);

		if (IsNintendoDisc(pDisc) && IsSupported0xE7(pDevice)) {
			if ((pDisc->SCSI.nAllLength == WII_SL_SIZE || pDisc->SCSI.nAllLength == WII_DL_SIZE)) {
				OutputString(
					"[INFO] If you want to decrypt the dumped iso file, you need to put key.bin in the same place as DiscImageCreator.exe\n"
					"key.bin is 16 bytes, crc32 is fc8bf576\n"
				);
			}
			ReadCdb.Streaming = TRUE;
		}
		else {
			ReadCdb.ForceUnitAccess = TRUE;
		}
		FlushLog();

		BYTE byScsiStatus = 0;
		DWORD readAddrForHG[5] = { 
			baseAddr + 0 * transferLen.AsULong * dwSectorSize,
			baseAddr + 1 * transferLen.AsULong * dwSectorSize,
			baseAddr + 2 * transferLen.AsULong * dwSectorSize,
			baseAddr + 3 * transferLen.AsULong * dwSectorSize,
			baseAddr + 4 * transferLen.AsULong * dwSectorSize
		};
		DWORD transferAndMemSize = transferLen.AsULong * memBlkSize;
		DWORD dwReadSize = (DWORD)DISC_MAIN_DATA_SIZE * transferLen.AsULong;
		DWORD dwRawReadSize = dwSectorSize * transferLen.AsULong;
//		size_t rawWriteSize = (size_t)dwSectorSize * transferLen.AsULong * memBlkSize;
		DWORD dwOfs[16] = {
			0, dwSectorSize, dwSectorSize * 2, dwSectorSize * 3, dwSectorSize * 4,
			dwSectorSize * 5, dwSectorSize * 6, dwSectorSize * 7, dwSectorSize * 8,
			dwSectorSize * 9, dwSectorSize * 10, dwSectorSize * 11, dwSectorSize * 12,
			dwSectorSize * 13, dwSectorSize * 14, dwSectorSize * 15
		};
		INT nRereadNum = 0;

		INT nLBA = 0;
		DWORD sectorNum = 0x30000;
		if (pExtArg->byFix) {
			UINT pos = pDisc->DVD.fixNum * 16;
			nLBA = (INT)pos;
			sectorNum += pos;
		}
		if (pExtArg->byResume) {
			INT64 size = (INT64)GetFileSize64(0, fp);
			_fseeki64(fp, size - dwSectorSize, SEEK_SET);
			if (fread(lpBuf, sizeof(BYTE), dwSectorSize, fp) < dwSectorSize) {
				OutputErrorString("Failed to read [F:%s][L:%d]\n", _T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			_fseeki64(fp, size, SEEK_SET);
			sectorNum = MAKEDWORD(MAKEWORD(lpBuf[3], lpBuf[2]), MAKEWORD(lpBuf[1], 0)) + 1;
			OutputString("Start resuming -> Last sector num: %6lx\n", sectorNum);
			nLBA = (INT)(size / dwSectorSize);
#if 1
			_fseeki64(fp, 0, SEEK_SET);
			for (DWORD n = 0x30000; n < sectorNum; n += transferAndMemSize) {
				if (fread(lpBuf, sizeof(BYTE), DVD_RAW_SECTOR_SIZE * transferAndMemSize, fp) < DVD_RAW_SECTOR_SIZE * transferAndMemSize) {
					OutputErrorString("Failed to read [F:%s][L:%d]\n", _T(__FUNCTION__), __LINE__);
					return FALSE;
				}
				DWORD num = MAKEDWORD(MAKEWORD(lpBuf[3], lpBuf[2]), MAKEWORD(lpBuf[1], 0));
				if (n != num) {
					OutputString("\nDetected bad sector. Expected sector num: %6lx, Got sector num: %6lx\n", n, num);
					sectorNum = n;
					_fseeki64(fp, (sectorNum - 0x30000) * dwSectorSize, SEEK_SET);
					UINT64 tmpPos = (UINT64)_ftelli64(fp);
					nLBA = (INT)(tmpPos / dwSectorSize);
					break;
				}
				OutputString("\rChecking sector num: %6lx", n);
			}
			OutputString("\n");
#endif
		}
		CalcInit(&pHash->pHashChunk[pHash->uiIndex].md5, &pHash->pHashChunk[pHash->uiIndex].sha);
		if (pExtArg->byDatExpand) {
			CalcInitExpand(&pHash->pHashChunk[pHash->uiIndex].sha224, &pHash->pHashChunk[pHash->uiIndex].sha256
				, &pHash->pHashChunk[pHash->uiIndex].sha384, &pHash->pHashChunk[pHash->uiIndex].sha512);
		}

#ifdef _WIN32
		INT direction = SCSI_IOCTL_DATA_IN;
#else
		INT direction = SG_DXFER_FROM_DEV;
#endif
		BOOL bRetry = FALSE;
		BOOL bReadErr = FALSE;
		BYTE prevId = 0;
		FOUR_BYTE LBA;
		for (; nLBA < pDisc->SCSI.nAllLength; nLBA += (INT)transferAndMemSize) {
			if (pExtArg->byFix) {
				_fseeki64(fp, dwSectorSize * nLBA, SEEK_SET);
			}
			if ((INT)sectorNum == pDisc->SCSI.nAllLength + 0x30000) {
				break;
			}
			if ((INT)transferAndMemSize > pDisc->SCSI.nAllLength - nLBA) {
				memBlkSize = (DWORD)(pDisc->SCSI.nAllLength - nLBA) / transferLen.AsULong;
//				rawWriteSize = dwSectorSize * transferLen.AsULong * memBlkSize;
				transferAndMemSize = transferLen.AsULong * memBlkSize;
			}
			LBA.AsULong = (ULONG)nLBA;
			REVERSE_BYTES(&ReadCdb.LogicalBlock, &LBA);
			// store the data frame to the drive cache memory
			if (!ScsiPassThroughDirect(pExtArg, pDevice, &ReadCdb, CDB12GENERIC_LENGTH, lpBuf,
				direction, dwReadSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				FillMemory(lpBuf, DISC_MAIN_DATA_SIZE * transferAndMemSize, 0x00);
				fwrite(lpBuf, sizeof(BYTE), (size_t)DISC_MAIN_DATA_SIZE * transferLen.AsULong, fp);
				sectorNum += transferAndMemSize;
				bReadErr = TRUE;
				continue;
//				throw FALSE;
			}
			BOOL bCheckSectorNum = TRUE;
			UINT gotSectorNum = 0;
			BYTE id = 0;
			for (UINT i = 0; i < memBlkSize; i++) {
				if (nCmdType == 1) {
					CacheCmd[6] = HIBYTE(HIWORD(readAddrForHG[i]));
					CacheCmd[7] = LOBYTE(HIWORD(readAddrForHG[i]));
					CacheCmd[8] = HIBYTE(LOWORD(readAddrForHG[i]));
					CacheCmd[9] = LOBYTE(LOWORD(readAddrForHG[i]));
				}
				else if (nCmdType == 2) {
					if (nRereadNum == 0) {
						baseAddr += 0x810 * transferLen.AsULong;
						if (baseAddr == 0x80008100) {
							baseAddr = 0x80000000;
						}
					}
					CacheCmd[6] = HIBYTE(HIWORD(baseAddr));
					CacheCmd[7] = LOBYTE(HIWORD(baseAddr));
					CacheCmd[8] = HIBYTE(LOWORD(baseAddr));
					CacheCmd[9] = LOBYTE(LOWORD(baseAddr));
				}
				else if (nCmdType == 3) {
					INT n = nLBA % 688;
					CacheCmd[3] = LOBYTE(HIWORD(n * DVD_RAW_SECTOR_SIZE));
					CacheCmd[4] = HIBYTE(LOWORD(n * DVD_RAW_SECTOR_SIZE));
					CacheCmd[5] = LOBYTE(LOWORD(n * DVD_RAW_SECTOR_SIZE));
				}

				DWORD dwOfs2 = dwRawReadSize * i;
				// read the drive cache memory
				if (!ScsiPassThroughDirect(pExtArg, pDevice, CacheCmd, cdblen, lpBuf + dwOfs2,
					direction, dwRawReadSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
#if 1
					throw FALSE;
#else
					CacheCmd[2] += 1;
					OutputString("Rawdump command [0]:%#04x [1]:%#04x [2]:%#04x\n", CacheCmd[0], CacheCmd[1], CacheCmd[2]);
					continue;
#endif
				}

				for (UINT j = 0; j < transferLen.AsULong; j++) {
					DWORD dwOfs3 = dwOfs2 + dwOfs[j];
					id = lpBuf[dwOfs3];
					gotSectorNum = MAKEUINT(MAKEWORD(lpBuf[3 + dwOfs3]
						, lpBuf[2 + dwOfs3]), MAKEWORD(lpBuf[1 + dwOfs3], 0));

					if ((id & 0x11) == 1 && (prevId & 0x11) == 0) {
						OutputString("\nLayer is changed: %02x -> %02x\n", prevId, id);
						sectorNum = gotSectorNum - j - i * transferLen.AsULong;
					}
					else if (sectorNum != gotSectorNum - j - i * transferLen.AsULong) {
						OutputLog(standardError | fileMainError
							, " Expected sector num: %6lx, Got sector num: %6lx "
							, sectorNum, gotSectorNum - j - i * transferLen.AsULong);
						OutputMainErrorLog("\n");
						bCheckSectorNum = FALSE;
						break;
					}
					prevId = id;
//					OutputMainChannel(fileMainInfo, lpBuf + dwOfs3, NULL, nLBA + j, DVD_RAW_SECTOR_SIZE);
					OutputDVDHeader(lpBuf + dwOfs3, dwSectorSize, nLBA + (INT)(j + i * transferLen.AsULong));
				}

				if (!bCheckSectorNum) {
					break;
				}
			}
			if (bCheckSectorNum) {
				nRereadNum = 0;
				WriteBufWithCalc(pExtArg, lpBuf, dwSectorSize * memBlkSize, transferLen.AsULong, fp, pHash);
				sectorNum += transferAndMemSize;
			}
			else {
				if (++nRereadNum == 40) {
					OutputString("Max Reread %d. LBA: %7d\n", nRereadNum, nLBA);
					if (!bRetry && (IsSupported0xE7Type2_1(pDevice) || IsSupported0xE7Type2_2(pDevice))) {
						transferLen.AsULong = 1;
						transferAndMemSize = transferLen.AsULong * memBlkSize;
						dwReadSize = (DWORD)DISC_MAIN_DATA_SIZE * transferLen.AsULong;
						dwRawReadSize = (DWORD)dwSectorSize * transferLen.AsULong;
//						rawWriteSize = (size_t)dwSectorSize * transferLen.AsULong * memBlkSize;
						baseAddr = 0x7FFFF7F0;
						nLBA = (INT)sectorNum - 0x30000 - 2;
						nRereadNum = 0;
						CacheCmd[10] = HIBYTE(LOWORD(dwSectorSize * transferLen.AsULong));
						CacheCmd[11] = LOBYTE(LOWORD(dwSectorSize * transferLen.AsULong));
						REVERSE_BYTES(&ReadCdb.TransferLength, &transferLen);
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
			if (nRereadNum || IsNintendoDisc(pDisc)) {
				if (IsSupported0xE7(pDevice)) {
					INT tmp = nLBA - (INT)transferAndMemSize;
					if (bReadErr) {
						tmp = nLBA;
							bReadErr = FALSE;
					}
					if (tmp < 0) {
						if (IsSupported0xE7Type1(pDevice)) {
							tmp = 16;
						}
						else if (IsSupported0xE7Type2_1(pDevice) ||
							IsSupported0xE7Type2_2(pDevice) ||
							IsSupported0xE7Type3(pDevice) ||
							IsSupported0xE7Type4(pDevice)
							) {
							tmp = 0;
						}
					}
					if (nRereadNum) {
						if (nCmdType == 1) {
							nLBA = tmp;
						}
#if 1
						else if (!bRetry && nCmdType == 2) {
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
						if (bRetry && nCmdType == 2) {
							if ((INT)(gotSectorNum - sectorNum) > 0) {
								nLBA -= 2;
							}
							else {
								nLBA = tmp + 1;
							}
						}
#endif
						else if (nCmdType == 3) {
							if ((INT)(gotSectorNum - sectorNum) > 0) {
								nLBA -= 32;
							}
						}
					}
					FOUR_BYTE tmpLBA;
					tmpLBA.AsULong = (ULONG)tmp;
					REVERSE_BYTES(&ReadCdb.LogicalBlock, &tmpLBA);
					// delete cache memory
					if (!ScsiPassThroughDirect(pExtArg, pDevice, &ReadCdb, CDB12GENERIC_LENGTH, lpBuf,
						direction, dwReadSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
						|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
						throw FALSE;
					}
				}
			}
			OutputString("\rCreating raw(LBA) %7d/%7d"
				, nLBA + (INT)transferAndMemSize, pDisc->SCSI.nAllLength);
			if (pExtArg->byFix) {
				if (nLBA == (INT)pDisc->DVD.fixNum * 16 + (INT)transferAndMemSize) {
					break;
				}
			}
		}
		OutputString("\n");
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pBuf);
	FcloseAndNull(fp);

	_tcsncpy(pHash->pHashChunk[pHash->uiIndex].szFnameAndExt, szFnameAndExt, sizeof(szFnameAndExt));
	pHash->pHashChunk[pHash->uiIndex].ui64FileSize = DVD_RAW_SECTOR_SIZE * (UINT64)pDisc->SCSI.nAllLength;

	if (bRet && IsNintendoDisc(pDisc) && IsSupported0xE7(pDevice)) {
		_TCHAR str[_MAX_PATH * 3] = {};
		if (GetUnscCmd(str, pszOutPath)) {
			bRet = _tsystem(str);
			// unscrambler error code
			// 0 == no error
			// 1 == failed open .raw
			// 2 == failed open .iso
			// 3 == no enough cache space for this seed
			// 6 == can't write to .iso
			// frame num == no seed found for recording frame xx
			// frame num == error unscrambling recording frame xx
			OutputString("ret = %d\n", bRet);
			if (bRet == 0) {
				if (pDisc->DVD.discType == gamecube) {
					ReadNintendoFileSystem(pDevice, pszFullPath, gamecube);
				}
				else if (pDisc->DVD.discType == wii) {
					ReadWiiPartition(pDevice, pszFullPath);
				}
			}
		}
	}
	return bRet;
}

#define CMI_SIZE (sizeof(DVD_DESCRIPTOR_HEADER) + sizeof(DVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR))
BOOL ReadDVDForCMI(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
#ifdef _WIN32
	_declspec(align(4)) BYTE pBuf[CMI_SIZE] = {};
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	__attribute__((aligned(4))) BYTE pBuf[CMI_SIZE] = {};
	INT direction = SG_DXFER_FROM_DEV;
#endif

	CDB::_READ_DVD_STRUCTURE cdb = {};
	cdb.OperationCode = SCSIOP_READ_DVD_STRUCTURE;
	cdb.Format = DvdMaxDescriptor;
	TWO_BYTE size;
	size.AsUShort = CMI_SIZE;
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("CopyrightManagementInformation"));

	BOOL bRet = TRUE;
	BYTE byScsiStatus = 0;
	FOUR_BYTE LBA;
	for (LBA.AsULong = 0; LBA.AsULong < (ULONG)pDisc->SCSI.nAllLength; LBA.AsULong++) {
		REVERSE_BYTES(&cdb.RMDBlockNumber, &LBA);
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, pBuf, 
			direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputDiscWithLBALog("Read error\n", (INT)LBA.AsULong);
		}
		else {
			OutputDVDCopyrightManagementInformation(
				(PDVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR)(pBuf + sizeof(DVD_DESCRIPTOR_HEADER)), (INT)LBA.AsULong);
		}
		OutputString(
			"\rWriting CMI log(LBA) %8lu/%8d", LBA.AsULong, pDisc->SCSI.nAllLength - 1);
	}
	OutputString("\n");
	return bRet;
}

BOOL ExecReadingKey(
	PDEVICE pDevice,
	_PROTECT_TYPE_DVD protect,
	LPCTSTR pszPath
) {
	if (!CloseHandle(pDevice->hDevice)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	try {
		_TCHAR str[_MAX_PATH + 10] = {};
		INT ret = 0;
		if (GetDVDProtectionCmd(pDevice, str, protect, pszPath)) {
			ret = _tsystem(str);
			if (ret == 1) {
				throw FALSE;
			}
		}
		else {
			throw FALSE;
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	if (!GetHandle(pDevice)) {
		return FALSE;
	}
	return bRet;
}

#define DVD_STRUCTURE_SIZE (sizeof(DVD_DESCRIPTOR_HEADER) + (sizeof(DVD_STRUCTURE_LIST_ENTRY) * 0xff))
BOOL ReadDiscStructure(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath,
	PHASH pHash
) {
#ifdef _WIN32
	_declspec(align(4)) BYTE pBuf[DVD_STRUCTURE_SIZE] = {};
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	__attribute__((aligned(4))) BYTE pBuf[DVD_STRUCTURE_SIZE] = {};
	INT direction = SG_DXFER_TO_FROM_DEV;
#endif
	CDB::_READ_DVD_STRUCTURE cdb = {};
	cdb.OperationCode = SCSIOP_READ_DVD_STRUCTURE;
	if (*pExecType == bd) {
		cdb.Reserved1 = 1; // media type
	}
	cdb.Format = 0xff;
	TWO_BYTE Size;
	Size.AsUShort = DVD_STRUCTURE_SIZE;
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &Size);

	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH,
		pBuf, direction, Size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}

	PDVD_DESCRIPTOR_HEADER pDescHeader = ((PDVD_DESCRIPTOR_HEADER)pBuf);
	TWO_BYTE descLen;
	descLen.AsUShort = pDescHeader->Length;
	REVERSE_SHORT(&descLen);
	WORD wDataSize = (WORD)(descLen.AsUShort - sizeof(descLen.AsUShort));
	WORD wEntrySize = (WORD)(wDataSize / sizeof(DVD_STRUCTURE_LIST_ENTRY));

	_TCHAR szPath[_MAX_PATH] = {};
	_TCHAR szOutPathPFI[_MAX_PATH] = {};
	_TCHAR szOutPathDMI[_MAX_PATH] = {};
	_TCHAR szOutPathPIC[_MAX_PATH] = {};
	_TCHAR szFnameAndExtPFI[_MAX_FNAME + _MAX_EXT] = {};
	_TCHAR szFnameAndExtDMI[_MAX_FNAME + _MAX_EXT] = {};
	_TCHAR szFnameAndExtPIC[_MAX_FNAME + _MAX_EXT] = {};
	FILE* fpPfi = NULL;
	FILE* fpDmi = NULL;
	FILE* fpPic = NULL;

	if (*pExecType == dvd || *pExecType == xbox) {
		_tcsncpy(szPath, pszFullPath, sizeof(szPath) / sizeof(szPath[0]) - 1);
		fpPfi = CreateOrOpenFile(szPath, _T("_PFI"), szOutPathPFI, szFnameAndExtPFI, NULL, _T(".bin"), _T("wb"), 0, 0);
		if (!fpPfi) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		_tcsncpy(szPath, pszFullPath, sizeof(szPath) / sizeof(szPath[0]) - 1);
		fpDmi = CreateOrOpenFile(szPath, _T("_DMI"), szOutPathDMI, szFnameAndExtDMI, NULL, _T(".bin"), _T("wb"), 0, 0);
		if (!fpDmi) {
			FcloseAndNull(fpPfi);
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	else if (*pExecType == bd) {
		_tcsncpy(szPath, pszFullPath, sizeof(szPath) / sizeof(szPath[0]) - 1);
		fpPic = CreateOrOpenFile(szPath, _T("_PIC"), szOutPathPIC, szFnameAndExtPIC, NULL, _T(".bin"), _T("wb"), 0, 0);
		if (!fpPic) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}

	INT nPacCnt = 0; // BD, format 0x30
	INT nPacNum = 0; // BD, format 0x30
	BYTE pacIdList[256][4] = {}; // BD, format 0x30

	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("DiscStructure"));
	for (WORD i = 0; i < wEntrySize; i++) {
		CalcInit(&pHash->pHashChunk[pHash->uiIndex].md5, &pHash->pHashChunk[pHash->uiIndex].sha);
		if (pExtArg->byDatExpand) {
			CalcInitExpand(&pHash->pHashChunk[pHash->uiIndex].sha224, &pHash->pHashChunk[pHash->uiIndex].sha256
				, &pHash->pHashChunk[pHash->uiIndex].sha384, &pHash->pHashChunk[pHash->uiIndex].sha512);
		}
		PDVD_STRUCTURE_LIST_ENTRY pEntry =
			((PDVD_STRUCTURE_LIST_ENTRY)pDescHeader->Data + i);
		// FormatLength is obsolete for late drive
//		WORD wFormatLen = MAKEWORD(pEntry->FormatLength[1], pEntry->FormatLength[0]);
		TWO_BYTE formatLen;
		formatLen.AsUShort = sizeof(DVD_DESCRIPTOR_HEADER);
		OutputDiscLog(
			"FormatCode: %02x, Sendable: %3s, Readable: %3s\n", 
			pEntry->FormatCode,	BOOLEAN_TO_STRING_YES_NO(pEntry->Sendable),
			BOOLEAN_TO_STRING_YES_NO(pEntry->Readable));
#if 0
		if (wFormatLen == 0) {
			OutputDiscLog("Skipped because length is 0\n\n");
			continue;
		}
#endif
		if (*pExecType == dvd || IsXbox(pExecType)) {
			if (pEntry->FormatCode == 0xff) {
				OutputDiscLog("Skipped\n\n");
				break;
			}
			else if (pEntry->FormatCode == 0x02) {
				continue; // skip output because disc key is random data
			}
			else if (pEntry->FormatCode == 0x03 && !pDisc->DVD.ucBca) {
				OutputDiscLog("Skipped because of no BCA data\n\n");
				continue;
			}
			else if (pEntry->FormatCode == 0x05) {
				OutputDiscLog("Skipped. If you see the detailed, use /c option\n\n");
				continue;
			}
			else if (pEntry->FormatCode == 0x06 || pEntry->FormatCode == 0x07) {
				if (pDisc->DVD.protect == cprm) {
					continue;
				}
				else {
					OutputDiscLog("Skipped because of DVD with CPRM only\n\n");
					continue;
				}
			}
			else if ((0x08 <= pEntry->FormatCode && pEntry->FormatCode <= 0x0b) &&
				pDisc->SCSI.wCurrentMedia != ProfileDvdRam) {
				OutputDiscLog("Skipped because of DVD-RAM only\n\n");
				continue;
			}
			else if ((0x0c <= pEntry->FormatCode && pEntry->FormatCode <= 0x10) &&
				(pDisc->SCSI.wCurrentMedia != ProfileDvdRecordable) &&
				(pDisc->SCSI.wCurrentMedia != ProfileDvdRewritable) &&
				(pDisc->SCSI.wCurrentMedia != ProfileDvdRWSequential)) {
				OutputDiscLog("Skipped because of DVD-R, RW only\n\n");
				continue;
			}
			else if ((pEntry->FormatCode == 0x11 || pEntry->FormatCode == 0x30) &&
				(pDisc->SCSI.wCurrentMedia != ProfileDvdPlusRW) &&
				(pDisc->SCSI.wCurrentMedia != ProfileDvdPlusR)) {
				OutputDiscLog("Skipped because of DVD+R, RW only\n\n");
				continue;
			}
			else if ((pEntry->FormatCode == 0x12 || pEntry->FormatCode == 0x15) &&
				(pDisc->SCSI.wCurrentMedia != ProfileHDDVDRom)) {
				OutputDiscLog("Skipped because of HD DVD only\n\n");
				continue;
			}
			else if ((0x20 <= pEntry->FormatCode && pEntry->FormatCode <= 0x24) &&
				pDisc->SCSI.wCurrentMedia != ProfileDvdDashRDualLayer) {
				OutputDiscLog("Skipped because of DVD-R DL only\n\n");
				continue;
			}
			else if ((pEntry->FormatCode == 0xc0) &&
				pDisc->SCSI.wCurrentMedia != ProfileDvdRewritable) {
				OutputDiscLog("Skipped because of DVD-R Rewritable only\n\n");
				continue;
			}
		}
		else if (*pExecType == bd) {
			if (pEntry->FormatCode == 0x03 && !pDisc->DVD.ucBca) {
				OutputDiscLog("Skipped because of no BCA data\n\n");
				continue;
			}
			else if ((pEntry->FormatCode == 0x08 || pEntry->FormatCode == 0x0a ||
				pEntry->FormatCode == 0x12) &&
				pDisc->SCSI.wCurrentMedia != ProfileBDRSequentialWritable &&
				pDisc->SCSI.wCurrentMedia != ProfileBDRRandomWritable &&
				pDisc->SCSI.wCurrentMedia != ProfileBDRewritable) {
				OutputDiscLog("Skipped because of BD-R, RW only\n\n");
				continue;
			}
		}
		if ((0x80 <= pEntry->FormatCode && pEntry->FormatCode <= 0x86)) {
			OutputDiscLog("AACS is not supported yet\n\n");
#if 0
			if (pDisc->DVD.protect != aacs) {
				OutputDiscLog("Skipped because of AACS disc only\n\n");
			}
#endif
			continue;
		}

		LPBYTE lpFormat = (LPBYTE)calloc(pDevice->dwMaxTransferLength, sizeof(BYTE));
		if (!lpFormat) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		cdb.LayerNumber = 0;
		cdb.Format = pEntry->FormatCode;
		REVERSE_BYTES_SHORT(&cdb.AllocationLength, &formatLen);

		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, 
			lpFormat, direction, formatLen.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
			byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputLog(standardError | fileDisc
				, "FormatCode: %02x failed\n"
				"cdb: %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x\n"
				, pEntry->FormatCode, cdb.OperationCode, (UCHAR)(cdb.Reserved1 << 5 | cdb.Lun)
				, cdb.RMDBlockNumber[0], cdb.RMDBlockNumber[1], cdb.RMDBlockNumber[2], cdb.RMDBlockNumber[3]
				, cdb.LayerNumber, cdb.Format, cdb.AllocationLength[0], cdb.AllocationLength[1]
				, (UCHAR)(cdb.Reserved3 << 6 | cdb.AGID), cdb.Control);
			continue;
		}
		formatLen.AsUShort = (WORD)(MAKEWORD(lpFormat[1], lpFormat[0]) + 2); // 2 is size of "DVD_DESCRIPTOR_HEADER::Length" itself
		REVERSE_BYTES_SHORT(&cdb.AllocationLength, &formatLen);
		OutputDiscLog("FormatLength: %u\n", formatLen.AsUShort);

		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH,
			lpFormat, direction, formatLen.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
			byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputLog(standardError | fileDisc, "FormatCode: %02x failed\n", pEntry->FormatCode);
		}
		else {
			if (*pExecType == dvd || IsXbox(pExecType)) {
				if (*pExecType == dvd || *pExecType == xbox) {
					if (pEntry->FormatCode == DvdPhysicalDescriptor) {
						// PFI doesn't include the header
						fwrite(lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), sizeof(BYTE), DISC_MAIN_DATA_SIZE, fpPfi);
						CalcHash(&pHash->pHashChunk[pHash->uiIndex].crc32, &pHash->pHashChunk[pHash->uiIndex].md5
							, &pHash->pHashChunk[pHash->uiIndex].sha, lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), DISC_MAIN_DATA_SIZE);
						if (pExtArg->byDatExpand) {
							CalcHashExpand(&pHash->pHashChunk[pHash->uiIndex].sha224, &pHash->pHashChunk[pHash->uiIndex].sha256
								, &pHash->pHashChunk[pHash->uiIndex].sha384, &pHash->pHashChunk[pHash->uiIndex].sha512, lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), DISC_MAIN_DATA_SIZE);
						}
						_tcsncpy(pHash->pHashChunk[pHash->uiIndex].szFnameAndExt, szFnameAndExtPFI, _MAX_FNAME + _MAX_EXT);
						pHash->pHashChunk[pHash->uiIndex].ui64FileSize = DISC_MAIN_DATA_SIZE;
						pHash->uiIndex++;
						FcloseAndNull(fpPfi);
					}
					else if (pEntry->FormatCode == DvdManufacturerDescriptor) {
						// DMI doesn't include the header
						fwrite(lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), sizeof(BYTE), DISC_MAIN_DATA_SIZE, fpDmi);
						CalcHash(&pHash->pHashChunk[pHash->uiIndex].crc32, &pHash->pHashChunk[pHash->uiIndex].md5
							, &pHash->pHashChunk[pHash->uiIndex].sha, lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), DISC_MAIN_DATA_SIZE);
						if (pExtArg->byDatExpand) {
							CalcHashExpand(&pHash->pHashChunk[pHash->uiIndex].sha224, &pHash->pHashChunk[pHash->uiIndex].sha256
								, &pHash->pHashChunk[pHash->uiIndex].sha384, &pHash->pHashChunk[pHash->uiIndex].sha512, lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), DISC_MAIN_DATA_SIZE);
						}
						_tcsncpy(pHash->pHashChunk[pHash->uiIndex].szFnameAndExt, szFnameAndExtDMI, _MAX_FNAME + _MAX_EXT);
						pHash->pHashChunk[pHash->uiIndex].ui64FileSize = DISC_MAIN_DATA_SIZE;
						pHash->uiIndex++;
						FcloseAndNull(fpDmi);
					}
				}
				DWORD dwSectorLen = 0;
				OutputDVDStructureFormat(pDisc, pEntry->FormatCode, (WORD)(formatLen.AsUShort - sizeof(DVD_DESCRIPTOR_HEADER))
					, lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), &dwSectorLen, cdb.LayerNumber);

				if (pEntry->FormatCode == DvdPhysicalDescriptor) {
					PDVD_FULL_LAYER_DESCRIPTOR dvdpd = (PDVD_FULL_LAYER_DESCRIPTOR)(lpFormat + sizeof(DVD_DESCRIPTOR_HEADER));
					// Parallel Track Path and Dual Layer
					if (dvdpd->commonHeader.TrackPath == 0 && dvdpd->commonHeader.NumberOfLayers == 1) {
						cdb.LayerNumber = 1;

						if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH,
							lpFormat, direction, formatLen.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
							byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
							OutputLog(standardError | fileDisc, "FormatCode: %02x failed\n", pEntry->FormatCode);
							if (pEntry->FormatCode == DvdPhysicalDescriptor) {
								ReadTOC(pExtArg, pExecType, pDevice, pDisc);
							}
						}
						else {
							DWORD dwSectorLen2 = 0;
							OutputDVDStructureFormat(pDisc, pEntry->FormatCode, (WORD)(formatLen.AsUShort - sizeof(DVD_DESCRIPTOR_HEADER))
								, lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), &dwSectorLen2, cdb.LayerNumber);
							OutputDiscLog("\tLayerAllSector: %7lu (%#lx)\n", dwSectorLen + dwSectorLen2, dwSectorLen + dwSectorLen2);
							dwSectorLen += dwSectorLen2;
						}
					}
					if (pDisc->SCSI.wCurrentMedia != ProfileDvdRam &&
						pDisc->SCSI.wCurrentMedia != ProfileDvdPlusR &&
						pDisc->SCSI.wCurrentMedia != ProfileHDDVDRam) {
						pDisc->SCSI.nAllLength = (INT)dwSectorLen;
					}
				}
				else if (pEntry->FormatCode == DvdManufacturerDescriptor && *pExecType == xbox) {
					OutputXboxManufacturingInfo(lpFormat + sizeof(DVD_DESCRIPTOR_HEADER));
				}
			}
			else if (*pExecType == bd) {
				if (pEntry->FormatCode == 0) {
					// PIC includes the header
					fwrite(lpFormat, sizeof(BYTE), formatLen.AsUShort, fpPic);
					CalcHash(&pHash->pHashChunk[pHash->uiIndex].crc32, &pHash->pHashChunk[pHash->uiIndex].md5
						, &pHash->pHashChunk[pHash->uiIndex].sha, lpFormat, formatLen.AsUShort);
					if (pExtArg->byDatExpand) {
						CalcHashExpand(&pHash->pHashChunk[pHash->uiIndex].sha224, &pHash->pHashChunk[pHash->uiIndex].sha256
							, &pHash->pHashChunk[pHash->uiIndex].sha384, &pHash->pHashChunk[pHash->uiIndex].sha512, lpFormat, formatLen.AsUShort);
					}
					_tcsncpy(pHash->pHashChunk[pHash->uiIndex].szFnameAndExt, szFnameAndExtPIC, _MAX_FNAME + _MAX_EXT);
					pHash->pHashChunk[pHash->uiIndex].ui64FileSize = formatLen.AsUShort;
					pHash->uiIndex++;
					FcloseAndNull(fpPic);
				}
				else if (pEntry->FormatCode == 0x30) {
					if (nPacCnt == 0) {
						nPacNum = (MAKEWORD(lpFormat[1], lpFormat[0]) - 2) / 384;
					}
				}
				OutputBDStructureFormat(pDisc, pEntry->FormatCode,
					MAKEWORD(lpFormat[1], lpFormat[0]), lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), nPacCnt);
				if (pEntry->FormatCode == 0x30) {
					if (nPacCnt == 0) {
						pacIdList[1][0] = 0xff;
						pacIdList[1][1] = 0xff;
						pacIdList[1][2] = 0xff;
						pacIdList[1][3] = 0xff;
						cdb.RMDBlockNumber[0] = pacIdList[1][0];
						cdb.RMDBlockNumber[1] = pacIdList[1][1];
						cdb.RMDBlockNumber[2] = pacIdList[1][2];
						cdb.RMDBlockNumber[3] = pacIdList[1][3];
						nPacCnt++;
						i--;
					}
					else if (nPacCnt == 1) {
						for (INT k = 2, m = 1; m <= nPacNum; k++, m++) {
							pacIdList[k][0] = lpFormat[4 * m];
							pacIdList[k][1] = lpFormat[5 * m];
							pacIdList[k][2] = lpFormat[6 * m];
							pacIdList[k][3] = lpFormat[7 * m];
						}
						cdb.RMDBlockNumber[0] = pacIdList[2][0];
						cdb.RMDBlockNumber[1] = pacIdList[2][1];
						cdb.RMDBlockNumber[2] = pacIdList[2][2];
						cdb.RMDBlockNumber[3] = pacIdList[2][3];
						nPacCnt++;
						i--;
					}
					else if (nPacCnt < nPacNum + 1) {
						cdb.RMDBlockNumber[0] = pacIdList[nPacCnt + 1][0];
						cdb.RMDBlockNumber[1] = pacIdList[nPacCnt + 1][1];
						cdb.RMDBlockNumber[2] = pacIdList[nPacCnt + 1][2];
						cdb.RMDBlockNumber[3] = pacIdList[nPacCnt + 1][3];
						nPacCnt++;
						i--;
					}
				}
			}
		}
		OutputDiscLog("\n");
		FreeAndNull(lpFormat);
	}
	// FormatCode: 00 failed (for gamecube. wii disc does success)
	if (pDisc->SCSI.nAllLength == 0) {
		if (pDisc->DVD.discType == DISC_TYPE_DVD::gamecube) {
			pDisc->SCSI.nAllLength = GAMECUBE_SIZE;
		}
		else {
			pDisc->SCSI.nAllLength = WII_SL_SIZE;
		}
	}
#if 0
	if (pDisc->DVD.discType == DISC_TYPE_DVD::gamecube || pDisc->DVD.discType == DISC_TYPE_DVD::wii) {
		BYTE bca[0xc0] = {};
		cdb.Reserved1 = 0;
		cdb.Lun = 0;
		cdb.RMDBlockNumber[0] = 0;
		cdb.RMDBlockNumber[1] = 0;
		cdb.RMDBlockNumber[2] = 0;
		cdb.RMDBlockNumber[3] = 0;
		cdb.LayerNumber = 0;
		cdb.Format = 0x03;
		cdb.AllocationLength[0] = 0;
		cdb.AllocationLength[1] = 0xc0;
		cdb.Reserved3 = 0;
		cdb.AGID = 0;
		cdb.Control = 0;
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH,
			bca, direction, 0x0c0, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
			byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputLog(standardError | fileDisc
				, "Failed to read BCA. cdb: %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x\n"
				, cdb.OperationCode, (UCHAR)(cdb.Reserved1 << 5 | cdb.Lun)
				, cdb.RMDBlockNumber[0], cdb.RMDBlockNumber[1], cdb.RMDBlockNumber[2], cdb.RMDBlockNumber[3]
				, cdb.LayerNumber, cdb.Format, cdb.AllocationLength[0], cdb.AllocationLength[1]
				, (UCHAR)(cdb.Reserved3 << 6 | cdb.AGID), cdb.Control);
		}
		else {
			OutputMainChannel(fileDisc, bca, _T("BCA"), 0, 0xc0);
		}
	}
#endif
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
	CDB::_CDB10 cdb = {};
	cdb.OperationCode = SCSIOP_READ_CAPACITY;

	BYTE buf[8] = {};
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE scsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH,
		&buf, direction, sizeof(buf), &scsiStatus, _T(__FUNCTION__), __LINE__)
		|| scsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	UINT len = MAKEUINT(MAKEWORD(buf[3], buf[2]), MAKEWORD(buf[1], buf[0]));
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("ReadCapacity")
		"\tMax LBA + 1: %u (0x%x)\n", len + 1, len + 1);
	return TRUE;
}

/*
AD 00 FF 02 FD FF FE 00 08 00 xx C0		, This is the well known SS extract commands from the xtreme FW.
*/
BOOL ExtractSecuritySector(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath,
	PHASH pHash
) {
	_TCHAR szPath[_MAX_PATH] = {};
	_tcsncpy(szPath, pszFullPath, sizeof(szPath) / sizeof(szPath[0]) - 1);
	if (!PathRemoveFileSpec(szPath)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	_TCHAR szOutPathSS[_MAX_PATH] = {};
	_TCHAR szFnameAndExtSS[_MAX_FNAME + _MAX_EXT] = {};

	FILE* fp = NULL;
	if (NULL == (fp = CreateOrOpenFile(
		szPath, _T("_SS"), szOutPathSS, szFnameAndExtSS, NULL, _T(".bin"), _T("wb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
	lpCmd[0] = 0xad;
	lpCmd[2] = 0xff;
	lpCmd[3] = 0x02;
	lpCmd[4] = 0xfd;
	lpCmd[5] = 0xff;
	lpCmd[6] = 0xfe;
	lpCmd[8] = 0x08;
	lpCmd[11] = 0xc0;
	BYTE cmd[5] = {};
	if (pDisc->SCSI.nAllLength == 3697696 || pDisc->SCSI.nAllLength == 4246304) {
		// http://beta.ivc.no/wiki/index.php/Xbox_360_Hacks#Save_security-sector
		// https://team-xecuter.com/forums/threads/42585-Xtreme-firmware-2-0-for-TS-H943-Xbox-360
		cmd[0] = 0x01;
		cmd[1] = 0x03;
		cmd[2] = 0x05;
		cmd[3] = 0x07;
	}
	BYTE buf[DISC_MAIN_DATA_SIZE] = {};
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	INT i = 0;
	do {
		lpCmd[10] = cmd[i];
		if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB12GENERIC_LENGTH,
			buf, direction, DISC_MAIN_DATA_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
	} while (cmd[++i] != 0);

	BOOL filled = FALSE;
	if (pDisc->SCSI.nAllLength == 3697696) {
		OutputString("Output SSv1 to SS.bin\n");
		// http://redump.org/download/ss_sector_range_1.0e.rar
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
	else if (pDisc->SCSI.nAllLength == 4246304) {
		// http://redump.org/download/ss_sector_range_1.0e.rar
		for (INT j = 32; j < 104; j++) {
			if (buf[j] != 0) {
				filled = TRUE;
				break;
			}
		}
		if (filled == FALSE) {
			OutputString("Not Output XGD3 SSv1 to SS.bin\n");
			//Fix XGD3 SSv1 ss.bin
			buf[552] = 0x01;
			buf[553] = 0x00;

			buf[561] = 0x5B;
			buf[562] = 0x00;

			buf[570] = 0xB5;
			buf[571] = 0x00;

			buf[579] = 0x0F;
			buf[580] = 0x01;
		}
		else {
			OutputString("Output XGD3 AP25 to SS.bin\n");
			//Fix XGD3 AP25 ss.bin
			buf[72] = 0x01;
			buf[73] = 0x00;
			buf[75] = 0x01;
			buf[76] = 0x00;

			buf[81] = 0x5B;
			buf[82] = 0x00;
			buf[84] = 0x5B;
			buf[85] = 0x00;

			buf[90] = 0xB5;
			buf[91] = 0x00;
			buf[93] = 0xB5;
			buf[94] = 0x00;

			buf[99] = 0x0F;
			buf[100] = 0x01;
			buf[102] = 0x0F;
			buf[103] = 0x01;
		}
	}
	if (pDisc->SCSI.nAllLength < 4246304 || filled) {
		fwrite(buf, sizeof(BYTE), (size_t)DISC_MAIN_DATA_SIZE, fp);
	}
	CalcHash(&pHash->pHashChunk[pHash->uiIndex].crc32, &pHash->pHashChunk[pHash->uiIndex].md5
		, &pHash->pHashChunk[pHash->uiIndex].sha, buf, DISC_MAIN_DATA_SIZE);
	if (pExtArg->byDatExpand) {
		CalcHashExpand(&pHash->pHashChunk[pHash->uiIndex].sha224, &pHash->pHashChunk[pHash->uiIndex].sha256
			, &pHash->pHashChunk[pHash->uiIndex].sha384, &pHash->pHashChunk[pHash->uiIndex].sha512, buf, DISC_MAIN_DATA_SIZE);
	}
	_tcsncpy(pHash->pHashChunk[pHash->uiIndex].szFnameAndExt, szFnameAndExtSS, _MAX_FNAME + _MAX_EXT);
	pHash->pHashChunk[pHash->uiIndex].ui64FileSize = DISC_MAIN_DATA_SIZE;
	pHash->uiIndex++;
	FcloseAndNull(fp);

//	if (pDisc->SCSI.nAllLength == 3697696 || pDisc->SCSI.nAllLength == 4246304) {
	if (buf[768] == 0x01) {
		OutputXboxSecuritySector(pDisc, buf);
	}
	else if (0x02 <= buf[768]) {
		OutputXbox360SecuritySector(pDisc, buf);
	}
	return TRUE;
}

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
	BYTE lpCmd[CDB6GENERIC_LENGTH] = {};
	lpCmd[0] = 0xff;
	lpCmd[1] = 0x08;
	lpCmd[2] = 0x01;
	lpCmd[3] = 0x10;

#ifdef _WIN32
	_declspec(align(4)) BYTE buf[26] = {};
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	__attribute__((aligned(4))) BYTE buf[26] = {};
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE scsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB6GENERIC_LENGTH,
		buf, direction, 26, &scsiStatus, _T(__FUNCTION__), __LINE__)
		|| scsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputDriveLog(OUTPUT_DHYPHEN_PLUS_STR("FeatureList"));
	for (INT i = 0; i < 26; i += 2) {
		WORD list = MAKEWORD(buf[i + 1], buf[i]);
		switch (list) {
		case 0:
			break;
		case 0x100:
			OutputDriveLog("0x%04x : The drive supports the unlock 1 state (xtreme) for Xbox 360\n", list);
			break;
		case 0x101:
			OutputDriveLog("0x%04x : The drive supports the unlock 2 state (wxripper) for Xbox 360\n", list);
			break;
		case 0x120:
			OutputDriveLog("0x%04x : The drive supports the unlock 1 state (xtreme) for Xbox 360\n", list);
			break;
		case 0x121:
			OutputDriveLog("0x%04x : The drive has full challenge response functionality for Xbox 360\n", list);
			break;
		case 0x200:
			OutputDriveLog("0x%04x : The drive supports the unlock 1 state (xtreme) for Xbox\n", list);
			break;
		case 0x201:
			OutputDriveLog("0x%04x : The drive supports the unlock 2 state (wxripper) for Xbox\n", list);
			break;
		case 0x220:
			OutputDriveLog("0x%04x : The drive supports the unlock 1 state (xtreme) for Xbox\n", list);
			break;
		case 0x221:
			OutputDriveLog("0x%04x : The drive has full challenge response functionality for Xbox\n", list);
			break;
		case 0xF000:
			OutputDriveLog("0x%04x : The drive supports the lock (cancel any unlock state) command\n", list);
			break;
		case 0xF001:
			OutputDriveLog("0x%04x : The drive supports error skipping\n", list);
			break;
		default:
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
	BYTE lpCmd[CDB6GENERIC_LENGTH] = {};
	lpCmd[0] = 0xff;
	lpCmd[1] = 0x08;
	lpCmd[2] = 0x01;
	lpCmd[3] = 0x11;
	lpCmd[4] = byState;

#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_NONE;
#endif
	BYTE scsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB6GENERIC_LENGTH,
		NULL, direction, 0, &scsiStatus, _T(__FUNCTION__), __LINE__)
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
	BYTE lpCmd[CDB6GENERIC_LENGTH] = {};
	lpCmd[0] = 0xff;
	lpCmd[1] = 0x08;
	lpCmd[2] = 0x01;
	lpCmd[3] = 0x15;
	lpCmd[4] = byState;

#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_NONE;
#endif
	BYTE scsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB6GENERIC_LENGTH,
		NULL, direction, 0, &scsiStatus, _T(__FUNCTION__), __LINE__)
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
	LPCTSTR pszFullPath,
	PHASH pHash
) {
	if (!GetFeatureListForXBox(pExtArg, pDevice)) {
		return FALSE;
	}

	if (!SetLockState(pExtArg, pDevice, 0)) {
		return FALSE;
	}
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("Lock state"));
	if (!ReadCapacity(pExtArg, pDevice)) {
		return FALSE;
	}
	if (!ReadTOC(pExtArg, pExecType, pDevice, pDisc)) {
		return FALSE;
	}
	if (!ReadDiscStructure(pExecType, pExtArg, pDevice, pDisc, pszFullPath, pHash)) {
		return FALSE;
	}

	if (!SetLockState(pExtArg, pDevice, 2)) {
		return FALSE;
	}
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("Unlock state 2 (wxripper)"));
	if (!ReadCapacity(pExtArg, pDevice)) {
		return FALSE;
	}
	if (!ReadTOC(pExtArg, pExecType, pDevice, pDisc)) {
		return FALSE;
	}

	if (!ExtractSecuritySector(pExtArg, pDevice, pDisc, pszFullPath, pHash)) {
		return FALSE;
	}

	if (!SetErrorSkipState(pExtArg, pDevice, 0)) {
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
	if (!ReadDVD(pExecType, pExtArg, pDevice, pDisc, pszFullPath, pHash)) {
		return FALSE;
	}
	return TRUE;
}

BOOL ReadXboxDVDBySwap(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath,
	PHASH pHash
) {
	if (!ReadDiscStructure(pExecType, pExtArg, pDevice, pDisc, pszFullPath, pHash)) {
		return FALSE;
	}
	
	DWORD dwDvdAllLen = pDisc->DVD.dwLayer0SectorLength + pDisc->DVD.dwLayer1SectorLength;
	if (*pExecType == xboxswap) {
		if (pDisc->DVD.dwLayer0SectorLength < XBOX_LAYER_BREAK) {
			OutputErrorString(
				"Short of length of DVD\n"
				"\tYour DVD length: %lu\n"
				"\tXbox LayerBreak: %d\n"
				, dwDvdAllLen, XBOX_LAYER_BREAK);
			return FALSE;
		}
		pDisc->DVD.dwXboxSwapOfs = (pDisc->DVD.dwLayer0SectorLength - XBOX_LAYER_BREAK) * 2;
		DWORD dwXboxAllLen = XBOX_SIZE + pDisc->DVD.dwXboxSwapOfs;
		if (dwXboxAllLen > dwDvdAllLen) {
			OutputErrorString(
				"Short of length of DVD\n"
				"\t  Your DVD length: %lu\n"
				"\tNeeded DVD length: (DVD L0[%lu] - Xbox LayerBreak[%d]) * 2 + Xbox Length[%d] = %lu\n"
				, dwDvdAllLen, pDisc->DVD.dwLayer0SectorLength, XBOX_LAYER_BREAK, XBOX_SIZE, dwXboxAllLen);
			return FALSE;
		}
	}
	else if (*pExecType == xgd2swap) {
		if (pDisc->DVD.dwLayer0SectorLength < XGD2_LAYER_BREAK) {
			OutputErrorString(
				"Short of length of DVD\n"
				"\tYour DVD length: %lu\n"
				"\tXGD2 LayerBreak: %d\n"
				, dwDvdAllLen, XGD2_LAYER_BREAK);
			return FALSE;
		}
		pDisc->DVD.dwXboxSwapOfs = (pDisc->DVD.dwLayer0SectorLength - XGD2_LAYER_BREAK) * 2;
		DWORD dwXboxAllLen = pExtArg->nAllSectors + pDisc->DVD.dwXboxSwapOfs;
		if (dwXboxAllLen > dwDvdAllLen) {
			OutputErrorString(
				"Short of length of DVD\n"
				"\t  Your DVD length: %lu\n"
				"\tNeeded DVD length: (DVD L0[%lu] - XGD2 LayerBreak[%d]) * 2 + XGD2 Length[%d] = %lu\n"
				, dwDvdAllLen, pDisc->DVD.dwLayer0SectorLength, XGD2_LAYER_BREAK, pExtArg->nAllSectors, dwXboxAllLen);
			return FALSE;
		}
	}
	else if (*pExecType == xgd3swap) {
		if (pDisc->DVD.dwLayer0SectorLength < XGD3_LAYER_BREAK) {
			OutputErrorString(
				"Short of length of DVD\n"
				"\tYour DVD length: %lu\n"
				"\tXGD3 LayerBreak: %d\n"
				, dwDvdAllLen, XGD3_LAYER_BREAK);
			return FALSE;
		}
		pDisc->DVD.dwXboxSwapOfs = (pDisc->DVD.dwLayer0SectorLength - XGD3_LAYER_BREAK) * 2;
		DWORD dwXboxAllLen = pExtArg->nAllSectors + pDisc->DVD.dwXboxSwapOfs;
		if (dwXboxAllLen > dwDvdAllLen) {
			OutputErrorString(
				"Short of length of DVD\n"
				"\t  Your DVD length: %lu\n"
				"\tNeeded DVD length: (DVD L0[%lu] - XGD3 LayerBreak[%d]) * 2 + XGD3 Length[%d] = %lu\n"
				, dwDvdAllLen, pDisc->DVD.dwLayer0SectorLength, XGD3_LAYER_BREAK, pExtArg->nAllSectors, dwXboxAllLen);
			return FALSE;
		}
	}

	for (INT i = 0; pExtArg->uiSecuritySector[i] != 0; i++) {
		if (i < 8) {
			if (pExtArg->uiSecuritySector[i] < 3000000) {
				pDisc->DVD.securitySectorRange[i][0] = pExtArg->uiSecuritySector[i];
				pDisc->DVD.securitySectorRange[i][1] = pExtArg->uiSecuritySector[i] + 4095;
			}
			else {
				pDisc->DVD.securitySectorRange[i][0] = pExtArg->uiSecuritySector[i] + pDisc->DVD.dwXboxSwapOfs;
				pDisc->DVD.securitySectorRange[i][1] = pExtArg->uiSecuritySector[i] + 4095 + pDisc->DVD.dwXboxSwapOfs;
			}
		}
		else {
			pDisc->DVD.securitySectorRange[i][0] = pExtArg->uiSecuritySector[i] + pDisc->DVD.dwXboxSwapOfs;
			pDisc->DVD.securitySectorRange[i][1] = pExtArg->uiSecuritySector[i] + 4095 + pDisc->DVD.dwXboxSwapOfs;
		}
	}
	if (!ReadDVD(pExecType, pExtArg, pDevice, pDisc, pszFullPath, pHash)) {
		return FALSE;
	}
	return TRUE;
}

BOOL ReadSACD(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath,
	PHASH pHash
) {
	ReadSACDFileSystem(pExtArg, pDevice);

	_TCHAR szFnameAndExt[_MAX_FNAME + _MAX_EXT] = {};
	FILE* fp = CreateOrOpenFile(
		pszFullPath, NULL, NULL, szFnameAndExt, NULL, _T(".iso"), _T("wb"), 0, 0);
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

		CDB::_READ12 cdb = {};
		cdb.OperationCode = SCSIOP_READ12;
		if (pExtArg->byFua) {
			cdb.ForceUnitAccess = TRUE;
		}
		CalcInit(&pHash->pHashChunk[pHash->uiIndex].md5, &pHash->pHashChunk[pHash->uiIndex].sha);
		if (pExtArg->byDatExpand) {
			CalcInitExpand(&pHash->pHashChunk[pHash->uiIndex].sha224, &pHash->pHashChunk[pHash->uiIndex].sha256
				, &pHash->pHashChunk[pHash->uiIndex].sha384, &pHash->pHashChunk[pHash->uiIndex].sha512);
		}
		FOUR_BYTE transferLen;
		transferLen.AsULong = pDevice->dwMaxTransferLength / DISC_MAIN_DATA_SIZE;
		REVERSE_BYTES(&cdb.TransferLength, &transferLen);
#ifdef _WIN32
		INT direction = SCSI_IOCTL_DATA_IN;
#else
		INT direction = SG_DXFER_FROM_DEV;
#endif
		BYTE byScsiStatus = 0;
		FOUR_BYTE LBA;
		for (LBA.AsULong = 0; LBA.AsULong < (DWORD)pDisc->SCSI.nAllLength; LBA.AsULong += transferLen.AsULong) {
			if (transferLen.AsULong > (DWORD)(pDisc->SCSI.nAllLength - LBA.AsULong)) {
				transferLen.AsULong = (DWORD)(pDisc->SCSI.nAllLength - LBA.AsULong);
				REVERSE_BYTES(&cdb.TransferLength, &transferLen);
			}

			REVERSE_BYTES(&cdb.LogicalBlock, &LBA);
			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				direction, DISC_MAIN_DATA_SIZE * transferLen.AsULong, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					throw FALSE;
			}

			fwrite(lpBuf, sizeof(BYTE), (size_t)DISC_MAIN_DATA_SIZE * transferLen.AsULong, fp);
			CalcHash(&pHash->pHashChunk[pHash->uiIndex].crc32, &pHash->pHashChunk[pHash->uiIndex].md5
				, &pHash->pHashChunk[pHash->uiIndex].sha, lpBuf, DISC_MAIN_DATA_SIZE * (UINT)transferLen.AsULong);
			if (pExtArg->byDatExpand) {
				CalcHashExpand(&pHash->pHashChunk[pHash->uiIndex].sha224, &pHash->pHashChunk[pHash->uiIndex].sha256
					, &pHash->pHashChunk[pHash->uiIndex].sha384, &pHash->pHashChunk[pHash->uiIndex].sha512, lpBuf, DISC_MAIN_DATA_SIZE * (UINT)transferLen.AsULong);
			}
			_tcsncpy(pHash->pHashChunk[pHash->uiIndex].szFnameAndExt, szFnameAndExt, sizeof(szFnameAndExt));
			pHash->pHashChunk[pHash->uiIndex].ui64FileSize = DISC_MAIN_DATA_SIZE * (UINT64)pDisc->SCSI.nAllLength;
			OutputString("\rCreating iso(LBA) %8lu/%8d", LBA.AsULong + transferLen.AsULong, pDisc->SCSI.nAllLength);
		}
		OutputString("\n");
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FreeAndNull(pBuf);
	FcloseAndNull(fp);
	return bRet;
}
