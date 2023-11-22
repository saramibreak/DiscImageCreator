/**
 * Copyright 2011-2023 sarami
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
#include "calcHash.h"
#include "check.h"
#include "convert.h"
#include "execScsiCmd.h"
#include "execScsiCmdforCD.h"
#include "execScsiCmdforCDCheck.h"
#include "execIoctl.h"
#include "fix.h"
#include "get.h"
#include "init.h"
#include "output.h"
#include "outputScsiCmdLog.h"
#include "outputScsiCmdLogforCD.h"
#include "set.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
// These global variable is set at prngcd.cpp
extern unsigned char scrambled_table[2352];
// This global function is defined at DiscImageCreator.cpp
extern int stopMessage(void);

BOOL ExecReadDisc(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE pCdb,
	INT nLBA,
	LPBYTE lpBuf,
	LPBYTE bufDec,
	BYTE byTransferLen,
	LPCTSTR pszFuncName,
	LONG lLineNum
) {
	if (*pExecType == gd) {
		if (!ExecReadGD(pExtArg, pDevice, pDisc, pCdb, nLBA, byTransferLen, lpBuf, bufDec)) {
			return FALSE;
		}
		for (BYTE i = 0; i < byTransferLen; i++) {
			memcpy(lpBuf + DISC_MAIN_DATA_SIZE * i, bufDec + CD_RAW_SECTOR_SIZE * i + 16, DISC_MAIN_DATA_SIZE);
//			OutputMainChannel(fileMainInfo, lpBuf + DISC_MAIN_DATA_SIZE * i, NULL, nLBA, DISC_MAIN_DATA_SIZE);
		}
	}
	else {
		if (!ExecReadCD(pExtArg, pDevice, pCdb, nLBA, lpBuf,
			(DWORD)(DISC_MAIN_DATA_SIZE * byTransferLen), pszFuncName, lLineNum)) {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL ExecReadCD(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPBYTE lpCmd,
	INT nLBA,
	LPBYTE lpBuf,
	DWORD dwBufSize,
	LPCTSTR pszFuncName,
	LONG lLineNum
) {
	FOUR_BYTE LBA;
	LBA.AsULong = (ULONG)nLBA;
	REVERSE_BYTES(&lpCmd[2], &LBA);
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB12GENERIC_LENGTH
		, lpBuf, direction, dwBufSize, &byScsiStatus, pszFuncName, lLineNum, TRUE)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputLog(standardError | fileMainError,
			"lpCmd: %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x\n"
			"dwBufSize: %lu\n"
			, lpCmd[0], lpCmd[1], lpCmd[2], lpCmd[3], lpCmd[4], lpCmd[5]
			, lpCmd[6], lpCmd[7], lpCmd[8], lpCmd[9], lpCmd[10], lpCmd[11]
			, dwBufSize
		);
		return FALSE;
	}
	return TRUE;
}

BOOL ExecReadGD(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE pCdb,
	INT nLBA,
	BYTE byTransferLen,
	LPBYTE lpInBuf,
	LPBYTE lpOutBuf
) {
	for (INT n = 1; n <= 10; n++) {
		BOOL bRet = TRUE;
		if (!ExecReadCD(pExtArg, pDevice, pCdb, nLBA, lpInBuf,
			CD_RAW_SECTOR_SIZE * (DWORD)byTransferLen, _T(__FUNCTION__), __LINE__)) {
			if (n == 10) {
				return FALSE;
			}
			StartStopUnit(pExtArg, pDevice, STOP_UNIT_CODE, STOP_UNIT_CODE);
			UINT milliseconds = 10000;
			OutputErrorString("Retry %d/10 after %u milliseconds\n", n, milliseconds);
			Sleep(milliseconds);
			bRet = FALSE;
		}
		if (!bRet) {
			continue;
		}
		else {
			break;
		}
	}
	INT nOfs = pDisc->MAIN.nCombinedOffset;
	if (pDisc->MAIN.nCombinedOffset < 0) {
		nOfs = CD_RAW_SECTOR_SIZE + pDisc->MAIN.nCombinedOffset;
	}
#if 0
	OutputMainChannel(fileMainInfo, lpInBuf + idx, NULL, nLBA, CD_RAW_SECTOR_SIZE);
#endif
	for (BYTE i = 0; i < byTransferLen; i++) {
		for (INT j = 0; j < CD_RAW_SECTOR_SIZE; j++) {
			lpOutBuf[j + CD_RAW_SECTOR_SIZE * i] =
				(BYTE)(lpInBuf[(nOfs + j) + CD_RAW_SECTOR_SIZE * i] ^ scrambled_table[j]);
		}
	}
#if 0
	OutputMainChannel(fileMainInfo, lpOutBuf, NULL, nLBA, CD_RAW_SECTOR_SIZE);
#endif
	return TRUE;
}

BOOL ExecReadCDForC2(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPBYTE lpCmd,
	INT nLBA,
	LPBYTE lpBuf,
	LPCTSTR pszFuncName,
	LONG lLineNum
) {
	FOUR_BYTE LBA;
	LBA.AsULong = (ULONG)nLBA;
	REVERSE_BYTES(&lpCmd[2], &LBA);

	BYTE byTransferLen = lpCmd[9];
	if (lpCmd[0] != 0xd8) {
		byTransferLen = lpCmd[8];
	}
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB12GENERIC_LENGTH, lpBuf,
		direction, pDevice->TRANSFER.uiBufLen * byTransferLen, &byScsiStatus, pszFuncName, lLineNum, TRUE)) {
		if (pExtArg->byScanProtectViaFile) {
			return RETURNED_CONTINUE;
		}
		else {
			return RETURNED_FALSE;
		}
	}
	if (byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		if (*pExecType != gd) {
			return RETURNED_CONTINUE;
		}
		else {
			return RETURNED_FALSE;
		}
	}
	return RETURNED_NO_C2_ERROR_1ST;
}

// http://tmkk.undo.jp/xld/secure_ripping.html
// https://forum.dbpoweramp.com/showthread.php?33676
BOOL FlushDriveCache(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	INT nLBA
) {
	if (nLBA < MAX_LBA_OF_CD && nLBA % pExtArg->uiCacheDelNum == 0) {
		CDB::_READ12 cdb = {};
		cdb.OperationCode = SCSIOP_READ12;
		cdb.ForceUnitAccess = TRUE;
		FOUR_BYTE NextLBAAddress;
		NextLBAAddress.AsULong = (ULONG)(nLBA + 1);
		REVERSE_BYTES(&cdb.LogicalBlock, &NextLBAAddress);
#ifdef _WIN32
		INT direction = SCSI_IOCTL_DATA_IN;
#else
		INT direction = SG_DXFER_FROM_DEV;
#endif
		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pExtArg, pDevice, (LPBYTE)&cdb, CDB12GENERIC_LENGTH,
			NULL, direction, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL ProcessReadCD(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	LPBYTE lpCmd,
	INT nLBA
) {
	BOOL bRet = RETURNED_NO_C2_ERROR_1ST;
	if (*pExecType != gd && pDevice->bySuccessReadTocFull) {
		if (pDisc->SCSI.n1stLBAof2ndSession != -1) {
			if (pExtArg->byReverse) {
				if (pDisc->SCSI.n1stLBAof2ndSession == nLBA + 1) {
					OutputMainInfoLog(
						"Skip from Leadout of Session 1 [%d, %#x] to Leadin of Session 2 [%d, %#x]\n",
						pDisc->SCSI.n1stLBAofLeadout, (UINT)pDisc->SCSI.n1stLBAofLeadout,
						pDisc->SCSI.n1stLBAof2ndSession - 1, (UINT)pDisc->SCSI.n1stLBAof2ndSession - 1);
					pDiscPerSector->subch.prev.nAbsoluteTime = nLBA - SESSION_TO_SESSION_SKIP_LBA - 150;
					return RETURNED_SKIP_LBA;
				}
			}
			else {
				if (pDisc->MAIN.nFix1stLBAofLeadout == nLBA) {
					OutputMainInfoLog(
						"Skip from Leadout of Session 1 [%d, %#x] to Leadin of Session 2 [%d, %#x]\n",
						pDisc->SCSI.n1stLBAofLeadout, (UINT)pDisc->SCSI.n1stLBAofLeadout,
						pDisc->SCSI.n1stLBAof2ndSession - 1, (UINT)pDisc->SCSI.n1stLBAof2ndSession - 1);
					if (pDisc->MAIN.nCombinedOffset > 0) {
						pDiscPerSector->subch.prev.nAbsoluteTime =
							nLBA + SESSION_TO_SESSION_SKIP_LBA + 150 - pDisc->MAIN.nAdjustSectorNum - 1;
					}
					else if (pDisc->MAIN.nCombinedOffset < 0) {
						pDiscPerSector->subch.prev.nAbsoluteTime =
							nLBA + SESSION_TO_SESSION_SKIP_LBA + 150 + pDisc->MAIN.nAdjustSectorNum;
					}
					return RETURNED_SKIP_LBA;
				}
			}
		}
	}

	if (pExtArg->byFua || pDisc->SUB.nCorruptCrcH == 1 || pDisc->SUB.nCorruptCrcL == 1) {
		if (!IsValidProtectedSector(pDisc, nLBA, GetReadErrorFileIdx(pExtArg, pDisc, nLBA)) &&
			!IsValidIntentionalC2error(pDisc, pDiscPerSector, nLBA, GetC2ErrorFileIdx(pExtArg, pDisc, nLBA)) &&
			!pDiscPerSector->bLibCrypt && !pDiscPerSector->bSecuRom) {
			FlushDriveCache(pExtArg, pDevice, nLBA);
//			SynchronizeCache(pExtArg, pDevice);
			pDisc->SUB.nCorruptCrcH = 0;
			pDisc->SUB.nCorruptCrcL = 0;
		}
	}

	memcpy(pDiscPerSector->subcode.prev, pDiscPerSector->subcode.current, CD_RAW_READ_SUBCODE_SIZE);
	bRet = ExecReadCDForC2(pExecType, pExtArg, pDevice,
		lpCmd, nLBA, pDiscPerSector->data.current, _T(__FUNCTION__), __LINE__);

	if (pDevice->byPlxtrDrive) {
		if (bRet == RETURNED_NO_C2_ERROR_1ST) {
			AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
			if (pDiscPerSector->data.next != NULL && 1 <= pExtArg->uiSubAddionalNum) {
				memcpy(pDiscPerSector->data.next, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufLen, pDevice->TRANSFER.uiBufLen);
				AlignRowSubcode(pDiscPerSector->subcode.next, pDiscPerSector->data.next + pDevice->TRANSFER.uiBufSubOffset);
					
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					if (IsPlextor712OrNewer(pDevice)) {
						bRet = ContainsC2Error(pDevice, pDisc, 1, CD_RAW_READ_C2_294_SIZE, pDiscPerSector->data.next, &pDiscPerSector->uiC2errorNum, nLBA, TRUE);
					}
					else {
						bRet = ContainsC2Error(pDevice, pDisc, 0, CD_RAW_READ_C2_294_SIZE, pDiscPerSector->data.next, &pDiscPerSector->uiC2errorNum, nLBA, TRUE);
					}
				}
				if (pDiscPerSector->data.nextNext != NULL && 2 <= pExtArg->uiSubAddionalNum) {
//					ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd,
//						nLBA + 2, pDiscPerSector->data.nextNext, _T(__FUNCTION__), __LINE__);
					memcpy(pDiscPerSector->data.nextNext, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufLen * 2, pDevice->TRANSFER.uiBufLen);
					AlignRowSubcode(pDiscPerSector->subcode.nextNext, pDiscPerSector->data.nextNext + pDevice->TRANSFER.uiBufSubOffset);

					if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
						BOOL bRet2 = RETURNED_NO_C2_ERROR_1ST;
						if (IsPlextor712OrNewer(pDevice)) {
							UINT uiErrorBak = pDiscPerSector->uiC2errorNum;
							bRet2 = ContainsC2Error(pDevice, pDisc, 0, 1, pDiscPerSector->data.nextNext, &pDiscPerSector->uiC2errorNum, nLBA, TRUE);
							if (bRet2 == RETURNED_EXIST_C2_ERROR) {
								bRet = bRet2;
							}
							pDiscPerSector->uiC2errorNum += uiErrorBak;
						}
					}
				}
			}
		}
	}
	else {
		if (bRet == RETURNED_NO_C2_ERROR_1ST) {
			AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
			if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
				if (0 == pExtArg->uiC2Offset) {
					bRet = ContainsC2Error(pDevice, pDisc, 0, CD_RAW_READ_C2_294_SIZE, pDiscPerSector->data.current, &pDiscPerSector->uiC2errorNum, nLBA, TRUE);
				}
				else if (0 < pExtArg->uiC2Offset && pExtArg->uiC2Offset < CD_RAW_READ_C2_294_SIZE) {
					bRet = ContainsC2Error(pDevice, pDisc, pExtArg->uiC2Offset, CD_RAW_READ_C2_294_SIZE, pDiscPerSector->data.current, &pDiscPerSector->uiC2errorNum, nLBA, TRUE);
				}
			}
			if (!IsValidProtectedSector(pDisc, nLBA, GetReadErrorFileIdx(pExtArg, pDisc, nLBA))) {
				if (pDiscPerSector->data.next != NULL && 1 <= pExtArg->uiSubAddionalNum) {
					if (!(IsValid0xF1SupportedDrive(pDevice) && pDisc->SCSI.nAllLength - 1 <= nLBA)) {
						ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd,
							nLBA + 1, pDiscPerSector->data.next, _T(__FUNCTION__), __LINE__);
						AlignRowSubcode(pDiscPerSector->subcode.next, pDiscPerSector->data.next + pDevice->TRANSFER.uiBufSubOffset);

						if (0 < pExtArg->uiC2Offset && pExtArg->uiC2Offset < CD_RAW_READ_C2_294_SIZE) {
							bRet = ContainsC2Error(pDevice, pDisc, 0, pExtArg->uiC2Offset, pDiscPerSector->data.next, &pDiscPerSector->uiC2errorNum, nLBA, TRUE);
						}
						else if (CD_RAW_READ_C2_294_SIZE == pExtArg->uiC2Offset) {
							bRet = ContainsC2Error(pDevice, pDisc, 0, CD_RAW_READ_C2_294_SIZE, pDiscPerSector->data.next, &pDiscPerSector->uiC2errorNum, nLBA, TRUE);
						}
						else if (CD_RAW_READ_C2_294_SIZE < pExtArg->uiC2Offset && pExtArg->uiC2Offset < CD_RAW_READ_C2_294_SIZE * 2) {
							UINT uiVal = pExtArg->uiC2Offset - CD_RAW_READ_C2_294_SIZE;
							bRet = ContainsC2Error(pDevice, pDisc, uiVal, CD_RAW_READ_C2_294_SIZE, pDiscPerSector->data.next, &pDiscPerSector->uiC2errorNum, nLBA, TRUE);
						}

						if (pDiscPerSector->data.nextNext != NULL && 2 <= pExtArg->uiSubAddionalNum) {
							ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd,
								nLBA + 2, pDiscPerSector->data.nextNext, _T(__FUNCTION__), __LINE__);
							AlignRowSubcode(pDiscPerSector->subcode.nextNext, pDiscPerSector->data.nextNext + pDevice->TRANSFER.uiBufSubOffset);

							if (CD_RAW_READ_C2_294_SIZE < pExtArg->uiC2Offset && pExtArg->uiC2Offset < CD_RAW_READ_C2_294_SIZE * 2) {
								UINT uiVal = pExtArg->uiC2Offset - CD_RAW_READ_C2_294_SIZE;
								bRet = ContainsC2Error(pDevice, pDisc, 0, uiVal, pDiscPerSector->data.nextNext, &pDiscPerSector->uiC2errorNum, nLBA, TRUE);
							}
						}
					}
				}
			}
		}
	}
	return bRet;
}

BOOL ReadCDForRereadingSectorType1(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	CDFLAG::_READ_CD::_EXPECTED_SECTOR_TYPE type,
	LPBYTE lpCmd,
	FILE* fpImg,
	FILE* fpC2,
	INT nStart
) {
	LPBYTE lpBuf = NULL;
	BYTE byTransferLength = 3;
	if (IsValid0xF1SupportedDrive(pDevice) || IsValidAsusDriveWith310(pDevice)) {
		byTransferLength = 1;
	}
	if (NULL == (lpBuf = (LPBYTE)calloc(CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * (size_t)byTransferLength, sizeof(BYTE)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	LPINT lpAllLBAOfC2ErrorRemain = NULL;
	try {
		INT nC2ErrorCntRemain = 0;
		if (NULL == (lpAllLBAOfC2ErrorRemain = (LPINT)calloc((size_t)pDisc->MAIN.nC2ErrorCnt, sizeof(INT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		SetDiscSpeed(pExecType, pExtArg, pDevice, pExtArg->uiReadingSpeedForC2);
		SetReadDiscCommand(pExtArg, pDevice, byTransferLength
			, type, CDFLAG::_READ_CD::byte294, CDFLAG::_READ_CD::Raw, lpCmd, FALSE);

		for (UINT i = 0; i < pExtArg->uiMaxRereadNum; i++) {
			for (INT m = 0; m < pDisc->MAIN.nC2ErrorCnt; m++) {
				INT nLBA = pDisc->MAIN.lpAllLBAOfC2Error[m];
				OutputString("\rNeed to reread sector: %6d", nLBA);
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * (DWORD)byTransferLength, _T(__FUNCTION__), __LINE__)) {
					continue;
				}
				DWORD dwTmpCrc32 = 0;
				GetCrc32(&dwTmpCrc32, lpBuf, CD_RAW_SECTOR_SIZE);
//				OutputC2ErrorWithLBALogA("to [%06d] crc32[%03ld]: 0x%08lx "
//					, nLBA - pDisc->MAIN.nOffsetStart - 1, nLBA - pDisc->MAIN.nOffsetStart, i, dwTmpCrc32);
				OutputC2ErrorWithLBALog("crc32[%03u]: 0x%08lx ", nLBA, i, dwTmpCrc32);

				memcpy(pDiscPerSector->data.current, lpBuf, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE);
				if (IsValid0xF1SupportedDrive(pDevice) || IsValidAsusDriveWith310(pDevice)) {
					bRet = ContainsC2Error(pDevice, pDisc, 0, CD_RAW_READ_C2_294_SIZE
						, pDiscPerSector->data.current, &pDiscPerSector->uiC2errorNum, nLBA, FALSE);
				}
				else {
					if (1 <= pExtArg->uiSubAddionalNum) {
						memcpy(pDiscPerSector->data.next
							, lpBuf + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE);
					}
					if (2 <= pExtArg->uiSubAddionalNum) {
						memcpy(pDiscPerSector->data.nextNext
							, lpBuf + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * 2, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE);
					}
					if (1 <= pExtArg->uiSubAddionalNum && 0 < pExtArg->uiC2Offset && pExtArg->uiC2Offset < CD_RAW_READ_C2_294_SIZE) {
						BOOL bRetA = ContainsC2Error(pDevice, pDisc, pExtArg->uiC2Offset
							, CD_RAW_READ_C2_294_SIZE, pDiscPerSector->data.current, &pDiscPerSector->uiC2errorNum, nLBA, FALSE);
						UINT c2ErrorBak = pDiscPerSector->uiC2errorNum;

						BOOL bRetB = ContainsC2Error(pDevice, pDisc, 0, pExtArg->uiC2Offset
							, pDiscPerSector->data.next, &pDiscPerSector->uiC2errorNum, nLBA, FALSE);
						pDiscPerSector->uiC2errorNum += c2ErrorBak;

						bRet = bRetA == RETURNED_NO_C2_ERROR_1ST ? bRetB : bRetA;
					}
					else if (1 <= pExtArg->uiSubAddionalNum && pExtArg->uiC2Offset == CD_RAW_READ_C2_294_SIZE) {
						// Plextor older than PX-712 => +294
						bRet = ContainsC2Error(pDevice, pDisc, 0, CD_RAW_READ_C2_294_SIZE
							, pDiscPerSector->data.next, &pDiscPerSector->uiC2errorNum, nLBA, FALSE);
					}
					else if (2 <= pExtArg->uiSubAddionalNum && CD_RAW_READ_C2_294_SIZE < pExtArg->uiC2Offset && pExtArg->uiC2Offset < CD_RAW_READ_C2_294_SIZE * 2) {
						// Plextor PX-712 or newer => +295
						UINT uiVal = pExtArg->uiC2Offset - CD_RAW_READ_C2_294_SIZE;
						BOOL bRetA = ContainsC2Error(pDevice, pDisc, uiVal, CD_RAW_READ_C2_294_SIZE
							, pDiscPerSector->data.next, &pDiscPerSector->uiC2errorNum, nLBA, FALSE);
						UINT c2ErrorBak = pDiscPerSector->uiC2errorNum;

						BOOL bRetB = ContainsC2Error(pDevice, pDisc, 0, uiVal
							, pDiscPerSector->data.nextNext, &pDiscPerSector->uiC2errorNum, nLBA, FALSE);
						pDiscPerSector->uiC2errorNum += c2ErrorBak;

						bRet = bRetA == RETURNED_NO_C2_ERROR_1ST ? bRetB : bRetA;
					}
					else {
						bRet = ContainsC2Error(pDevice, pDisc, 0, CD_RAW_READ_C2_294_SIZE
							, pDiscPerSector->data.current, &pDiscPerSector->uiC2errorNum, nLBA, FALSE);
					}
				}
				if (bRet == RETURNED_NO_C2_ERROR_1ST) {
					LONG lSeekMain = CD_RAW_SECTOR_SIZE * (LONG)nLBA - nStart - pDisc->MAIN.nCombinedOffset;
					fseek(fpImg, lSeekMain, SEEK_SET);
					// Write track to scrambled again
					WriteMainChannel(pExecType, pExtArg, pDisc, pDiscPerSector->data.current, nLBA, fpImg);

					LONG lSeekC2 = CD_RAW_READ_C2_294_SIZE * (LONG)nLBA - nStart/* - (pDisc->MAIN.nCombinedOffset / 8)*/;
					fseek(fpC2, lSeekC2, SEEK_SET);
					WriteC2(pExtArg, pDevice, pDiscPerSector, fpC2);

					OutputC2ErrorLog("good. Rewrote .scm[%ld-%ld(%lx-%lx)] .c2[%ld-%ld(%lx-%lx)]\n"
						, lSeekMain, lSeekMain + 2351, (ULONG)lSeekMain, (ULONG)lSeekMain + 2351
						, lSeekC2, lSeekC2 + 293, (ULONG)lSeekC2, (ULONG)lSeekC2 + 293);
					if (pDisc->MAIN.nC2ErrorCnt == 1) {
						nC2ErrorCntRemain = 0;
						break;
					}
				}
				else {
					lpAllLBAOfC2ErrorRemain[nC2ErrorCntRemain++] = nLBA;
					OutputC2ErrorLog("bad\n");
				}

				if (nC2ErrorCntRemain == 1) {
					// Dummy reading
					// LG/ASUS drive (0xf1 is used) can't reread to infinity. Minimum is 500 or so, Maximum is 2000 or so.
					// This value varies every time. If rereadings are beyond this value, the drive always returns same hash.
					if (!ExecReadCD(pExtArg, pDevice, lpCmd, 0, lpBuf
						, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * (DWORD)byTransferLength, _T(__FUNCTION__), __LINE__)) {
						continue;
					}
				}
				else {
					if (!FlushDriveCache(pExtArg, pDevice, nLBA)) {
						throw FALSE;
					}
				}
			}

			if (nC2ErrorCntRemain) {
				OutputLog(standardOut | fileC2Error, "\nRemains %d c2 error. Rereading times: %4u/%4u\n", nC2ErrorCntRemain, i + 1, pExtArg->uiMaxRereadNum);
				if (!(i == pExtArg->uiMaxRereadNum - 1 && !pDisc->PROTECT.byExist)) {
					ZeroMemory(pDisc->MAIN.lpAllLBAOfC2Error, pDisc->MAIN.nC2ErrorCnt * sizeof(INT));
					pDisc->MAIN.nC2ErrorCnt = nC2ErrorCntRemain;
					memcpy(pDisc->MAIN.lpAllLBAOfC2Error, lpAllLBAOfC2ErrorRemain, nC2ErrorCntRemain * sizeof(INT));

					ZeroMemory(lpAllLBAOfC2ErrorRemain, nC2ErrorCntRemain * sizeof(INT));
					nC2ErrorCntRemain = 0;
				}
			}
			else {
				break;
			}
		}
		OutputString("\nDone. See _c2Error.txt\n");
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(lpBuf);
	FreeAndNull(lpAllLBAOfC2ErrorRemain);
	return bRet;
}

BOOL ReadCDForRereadingSectorType2(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDFLAG::_READ_CD::_EXPECTED_SECTOR_TYPE type,
	LPBYTE lpCmd,
	FILE* fpImg,
	FILE* fpC2,
	INT nStartLBA,
	INT nEndLBA
) {
	BOOL bRet = TRUE;
	DWORD dwTransferLen = pDevice->dwMaxTransferLength / CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
	DWORD dwTransferLenBak = dwTransferLen;
	LPBYTE lpBufMain = NULL;
	if (NULL == (lpBufMain = (LPBYTE)calloc(
		dwTransferLen * CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * pExtArg->uiMaxRereadNum, sizeof(BYTE)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE lpBufC2 = NULL;
	LPBYTE* lpRereadSector = NULL;
	LPDWORD* lpCrc32RereadSector = NULL;
	LPUINT* lpRepeatedNum = NULL;
	LPUINT* lpContainsC2 = NULL;
	try {
		if (NULL == (lpBufC2 = (LPBYTE)calloc(
			dwTransferLen * CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * pExtArg->uiMaxRereadNum, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpRereadSector = (LPBYTE*)calloc(dwTransferLen, sizeof(LPBYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpCrc32RereadSector = (LPDWORD*)calloc(dwTransferLen, sizeof(LPDWORD)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpRepeatedNum = (LPUINT*)calloc(dwTransferLen, sizeof(LPUINT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpContainsC2 = (LPUINT*)calloc(dwTransferLen, sizeof(LPUINT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		for (DWORD a = 0; a < dwTransferLen; a++) {
			if (NULL == (lpRereadSector[a] = (LPBYTE)calloc(CD_RAW_SECTOR_SIZE * pExtArg->uiMaxRereadNum, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (lpCrc32RereadSector[a] = (LPDWORD)calloc(pExtArg->uiMaxRereadNum, sizeof(DWORD)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (lpRepeatedNum[a] = (LPUINT)calloc(pExtArg->uiMaxRereadNum, sizeof(UINT)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (lpContainsC2[a] = (LPUINT)calloc(pExtArg->uiMaxRereadNum, sizeof(UINT)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}
		SetReadDiscCommand(pExtArg, pDevice, (BYTE)dwTransferLen
			, type, CDFLAG::_READ_CD::byte294, CDFLAG::_READ_CD::Raw, lpCmd, FALSE);

		INT nLBA = nStartLBA;
		INT nLastLBA = nEndLBA;

		while (nLBA < nLastLBA) {
			OutputString("\rRewrited img (LBA) %6d/%6d", nLBA, pDisc->SCSI.nAllLength - 1);
			INT idx = 0;
			if (nLastLBA - nLBA < (INT)dwTransferLen) {
				dwTransferLen = (DWORD)(nLastLBA - nLBA);
				lpCmd[9] = (BYTE)dwTransferLen;
			}
			for (UINT i = 0; i < pExtArg->uiMaxRereadNum; i++) {
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, lpBufMain
					, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * dwTransferLen, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA + 1, lpBufC2
					, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * dwTransferLen, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				for (DWORD k = 0; k < dwTransferLen; k++) {
					OutputC2ErrorWithLBALog("crc32", nLBA - pDisc->MAIN.nOffsetStart + (INT)k);
					DWORD dwTmpCrc32 = 0;
					GetCrc32(&dwTmpCrc32, lpBufMain + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * k, CD_RAW_SECTOR_SIZE);
#ifdef __DEBUG
					OutputMainChannel(fileC2Error, lpBufMain + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * k, NULL, nLBA, CD_RAW_SECTOR_SIZE);
					OutputCDC2Error296(fileC2Error, lpBufMain + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * k + CD_RAW_SECTOR_SIZE, 294);
					OutputMainChannel(fileC2Error, lpBufC2 + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * k, NULL, nLBA, CD_RAW_SECTOR_SIZE);
					OutputCDC2Error296(fileC2Error, lpBufC2 + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * k + CD_RAW_SECTOR_SIZE, 294);
#endif
					BOOL bMatch = FALSE;
					BOOL bC2 = RETURNED_NO_C2_ERROR_1ST;
					for (WORD wC2ErrorPos = 0; wC2ErrorPos < CD_RAW_READ_C2_294_SIZE; wC2ErrorPos++) {
						INT nBit = 0x80;
						DWORD dwPos = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * k + pDevice->TRANSFER.uiBufC2Offset + wC2ErrorPos;
						for (INT n = 0; n < CHAR_BIT; n++) {
							// exist C2 error
							if (lpBufC2[dwPos] & nBit) {
								bC2 = RETURNED_EXIST_C2_ERROR;
								break;
							}
							nBit >>= 1;
						}
						if (bC2 == RETURNED_EXIST_C2_ERROR) {
							break;
						}
					}
					if (lpCrc32RereadSector[k]) {
						for (UINT j = 0; j <= i; j++) {
							if (dwTmpCrc32 == lpCrc32RereadSector[k][j]) {
								OutputC2ErrorLog("[%03u]:0x%08lx, %d ", i, dwTmpCrc32, bC2);
								lpRepeatedNum[k][j] += 1;
								lpContainsC2[k][j] += bC2;
								bMatch = TRUE;
								break;
							}
						}
						if (!bMatch) {
							lpCrc32RereadSector[k][idx] = dwTmpCrc32;
							memcpy(&lpRereadSector[k][CD_RAW_SECTOR_SIZE * idx]
								, lpBufMain + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * k, CD_RAW_SECTOR_SIZE);
							OutputC2ErrorLog("[%03d]:0x%08lx, %d ", idx, dwTmpCrc32, bC2);
#if 0
							if (idx > 0) {
								OutputMainChannel(fileC2Error, &lpRereadSector[k][CD_RAW_SECTOR_SIZE * idx]
									, NULL, nLBA - pDisc->MAIN.nOffsetStart + (INT)k, CD_RAW_SECTOR_SIZE);
							}
#endif
						}
					}
				}
				OutputC2ErrorLog("\n");
				idx++;
				if (!FlushDriveCache(pExtArg, pDevice, nLBA)) {
					throw FALSE;
				}
			}

			for (DWORD q = 0; q < dwTransferLen; q++) {
				if (lpRepeatedNum[q]) {
					UINT uiMaxNum = lpRepeatedNum[q][0];
					UINT uiMaxC2 = lpContainsC2[q][0];
					for (INT k = 0; k < idx - 1; k++) {
						uiMaxNum = max(uiMaxNum, lpRepeatedNum[q][k + 1]);
						uiMaxC2 = max(uiMaxC2, lpContainsC2[q][k + 1]);
					}

					INT nTmpLBA = nLBA - pDisc->MAIN.nOffsetStart + (INT)q;
					if (uiMaxC2 == 0) {
						OutputC2ErrorWithLBALog(
							"to[%06d] All crc32 is probably bad. No rewrite\n", nTmpLBA - 1, nTmpLBA);
					}
					else if (uiMaxNum + 1 == pExtArg->uiMaxRereadNum &&
						lpCrc32RereadSector[q][0] == pDisc->MAIN.lpAllSectorCrc32[nTmpLBA]) {
						OutputC2ErrorWithLBALog(
							"to[%06d] All same crc32. No rewrite\n", nTmpLBA - 1, nTmpLBA);
					}
					else {
						for (INT l = 0; l < idx; l++) {
							if (uiMaxC2 == lpContainsC2[q][l]) {
								OutputC2ErrorWithLBALog(
									"to[%06d] crc32[%d]:0x%08lx, no c2 %u times. Rewrite\n"
									, nTmpLBA - 1, nTmpLBA, l, lpCrc32RereadSector[q][l], uiMaxC2 + 1);
								fseek(fpImg, CD_RAW_SECTOR_SIZE * (LONG)(nLBA + q) - pDisc->MAIN.nCombinedOffset, SEEK_SET);
								// Write track to scrambled again
								WriteMainChannel(pExecType, pExtArg, pDisc, &lpRereadSector[q][CD_RAW_SECTOR_SIZE * l], nLBA, fpImg);
								fseek(fpC2, CD_RAW_READ_C2_294_SIZE * (LONG)nLBA/* - (pDisc->MAIN.nCombinedOffset / 8)*/, SEEK_SET);
								if (q + 1 < dwTransferLen) {
									fwrite(&lpRereadSector[q + 1][CD_RAW_SECTOR_SIZE * l] + pDevice->TRANSFER.uiBufC2Offset, sizeof(BYTE), CD_RAW_READ_C2_294_SIZE, fpC2);
								}
#if 0
								OutputC2ErrorLog("Seek to %ld (0x%08lx)\n"
									, CD_RAW_SECTOR_SIZE * (LONG)(nLBA + q) - pDisc->MAIN.nCombinedOffset
									, CD_RAW_SECTOR_SIZE * (LONG)(nLBA + q) - pDisc->MAIN.nCombinedOffset);
								OutputMainChannel(fileC2Error, &lpRereadSector[q][CD_RAW_SECTOR_SIZE * l], NULL, nTmpLBA, CD_RAW_SECTOR_SIZE);
								OutputC2ErrorLog("\n");
#endif
								break;
							}
						}
					}
				}
			}
			nLBA += (INT)dwTransferLen;
			for (DWORD r = 0; r < dwTransferLen; r++) {
				for (INT p = 0; p < idx; p++) {
					if (lpRereadSector[r]) {
						lpRereadSector[r][p] = 0;
					}
					if (lpCrc32RereadSector[r]) {
						lpCrc32RereadSector[r][p] = 0;
					}
					if (lpRepeatedNum[r]) {
						lpRepeatedNum[r][p] = 0;
					}
					if (lpContainsC2[r]) {
						lpContainsC2[r][p] = 0;
					}
				}
			}
		}
		OutputLog(standardOut | fileC2Error, "\n");
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(lpBufMain);
	FreeAndNull(lpBufC2);
	for (DWORD r = 0; r < dwTransferLenBak; r++) {
		FreeAndNull(lpRereadSector[r]);
		FreeAndNull(lpCrc32RereadSector[r]);
		FreeAndNull(lpRepeatedNum[r]);
		FreeAndNull(lpContainsC2[r]);
	}
	FreeAndNull(lpRereadSector);
	FreeAndNull(lpCrc32RereadSector);
	FreeAndNull(lpRepeatedNum);
	FreeAndNull(lpContainsC2);

	return bRet;
}

INT ExecEccEdc(
	PEXT_ARG pExtArg,
	_DISC::_PROTECT protect,
	LPCTSTR pszType,
	LPCTSTR pszImgPath,
	_DISC::_PROTECT::_ERROR_SECTOR errorSector
) {
	CONST INT nCmdSize = 6;
	CONST INT nStrSize = _MAX_PATH * 2 + nCmdSize;
	_TCHAR str[nStrSize] = {};
	_TCHAR cmd[nCmdSize] = { _T("check") };
	INT nStartLBA = errorSector.nExtentPos[0];
	INT nEndLBA = errorSector.nExtentPos[0] + errorSector.nSectorSize[0];
	if (pExtArg->byScanProtectViaFile) {
		if (protect.byExist == safeDisc || protect.byExist == safeDiscLite ||
			protect.byExist == codelock || protect.byExist == datel ||
			protect.byExist == datelAlt || protect.byExist == discguard ||
			protect.byExist == physicalErr || protect.byExist == c2Err) {
			_tcsncpy(cmd, _T("fix"), SIZE_OF_ARRAY(cmd));
		}
	}
	INT ret = 0;
	if (GetEccEdcCmd(str, nStrSize, cmd, pszType, pszImgPath, nStartLBA, nEndLBA)) {
		OutputString("Exec %s\n", str);
		ret = _tsystem(str);
	}
	if (protect.byExist == physicalErr) {
		for (INT i = 1; i < pExtArg->FILE.readErrCnt; i++) {
			nStartLBA = errorSector.nExtentPos[i];
			nEndLBA = errorSector.nExtentPos[i] + errorSector.nSectorSize[i];
			if (GetEccEdcCmd(str, nStrSize, cmd, pszType, pszImgPath, nStartLBA, nEndLBA)) {
				OutputString("Exec %s\n", str);
				ret = _tsystem(str);
			}
		}
	}
	else if (protect.byExist == c2Err) {
		for (INT i = 1; i < pExtArg->FILE.c2ErrCnt; i++) {
			nStartLBA = errorSector.nExtentPos[i];
			nEndLBA = errorSector.nExtentPos[i] + errorSector.nSectorSize[i];
			if (GetEccEdcCmd(str, nStrSize, cmd, pszType, pszImgPath, nStartLBA, nEndLBA)) {
				OutputString("Exec %s\n", str);
				ret = _tsystem(str);
			}
		}
	}
	else if (protect.byExist == datelAlt) {
		nStartLBA = errorSector.nExtentPos2nd;
		nEndLBA = errorSector.nExtentPos2nd + errorSector.nSectorSize2nd;
		if (GetEccEdcCmd(str, nStrSize, cmd, pszType, pszImgPath, nStartLBA, nEndLBA)) {
			OutputString("Exec %s\n", str);
			ret = _tsystem(str);
		}
	}
	return ret;
}

VOID ProcessReturnedContinue(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA,
	INT nLastErrLBA,
	INT nMainDataType,
	INT nPadType,
	FILE* fpImg,
	FILE* fpSub,
	FILE* fpC2
) {
#if 0
	if ((pDiscPerSector->subch.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		OutputMainChannel(fileMainError,
			pDiscPerSector->mainHeader.current, NULL, nLBA, MAINHEADER_MODE1_SIZE);
	}
#endif
	WriteErrorBuffer(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector,
		scrambled_table, nLBA, nLastErrLBA, nMainDataType, nPadType, fpImg, fpSub, fpC2);
	UpdateTmpMainHeader(pDiscPerSector, nMainDataType);

	BYTE trk = (BYTE)(pDiscPerSector->subch.current.byTrackNum + 1);
	if (trk <= pDisc->SCSI.toc.LastTrack &&
		pDisc->SCSI.lp1stLBAListOnToc[trk - 1] == nLBA &&
		pDisc->SUB.lp1stLBAListOnSub[trk - 1][1] == -1) {
		pDisc->SUB.lp1stLBAListOnSub[trk - 1][1] = nLBA;
		pDiscPerSector->subch.current.byTrackNum++;
	}
	UpdateTmpSubch(pDiscPerSector);
#if 1
	if ((pDiscPerSector->subch.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		LPBYTE pBuf = NULL;
		if (*pExecType == data) {
//		if (pExtArg->byBe) {
			pBuf = pDiscPerSector->mainHeader.current;
		}
		else {
			if (nMainDataType == scrambled) {
				pBuf = pDiscPerSector->data.current + pDisc->MAIN.uiMainDataSlideSize;
			}
			else if (nMainDataType == unscrambled) {
				pBuf = pDiscPerSector->mainHeader.prev;
			}
		}
		OutputMainChannel(fileMainError, pBuf, NULL, nLBA, MAINHEADER_MODE1_SIZE);
	}
#endif
}

BOOL ProcessDescramble(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPCTSTR pszPath,
	_TCHAR* pszOutScmFile
) {
	_TCHAR pszNewPath[_MAX_PATH] = {};
	_tcsncpy(pszNewPath, pszOutScmFile, SIZE_OF_ARRAY(pszNewPath));
	pszNewPath[_MAX_PATH - 1] = 0;
	// "PathRenameExtension" fails to rename if space is included in extension.
	// e.g.
	//  no label. 2017-02-14_ 9-41-31 => no label. 2017-02-14_ 9-41-31.img
	//  no label.2017-02-14_9-41-31   => no label.img
	if (!PathRenameExtension(pszNewPath, _T(".img"))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	// audio only -> from .scm to .img. other descramble img.
	if (/*pExtArg->byBe || */(pDisc->SCSI.trkType != TRACK_TYPE::dataExist && pDisc->SCSI.trkType != TRACK_TYPE::pregapDataIn1stTrack)) {
		OutputString("Moving .scm to .img\n");
		if (!MoveFileEx(pszOutScmFile, pszNewPath, MOVEFILE_REPLACE_EXISTING)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (pExtArg->byBe) {
			ExecEccEdc(pExtArg, pDisc->PROTECT, _T("TOC"), pszNewPath, pDisc->PROTECT.ERROR_SECTOR);
		}
	}
	else {
		OutputString("Copying .scm to .img\n");
		if (!CopyFile(pszOutScmFile, pszNewPath, FALSE)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		FILE* fpImg = NULL;
		_TCHAR pszImgPath[_MAX_PATH] = {};
		if (NULL == (fpImg = CreateOrOpenFile(
			pszPath, NULL, pszImgPath, NULL, NULL, _T(".img"), _T("rb+"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		DescrambleMainChannelAll(pExtArg, pDisc, pDisc->SCSI.lp1stLBAListOfDataTrkOnToc
			, pDisc->SCSI.lpLastLBAListOfDataTrkOnToc, _T("TOC based"), scrambled_table, fpImg);
		FcloseAndNull(fpImg);
		ExecEccEdc(pExtArg, pDisc->PROTECT, _T("TOC"), pszImgPath, pDisc->PROTECT.ERROR_SECTOR);

		for (INT k = pDisc->SCSI.by1stDataTrkNum - 1; k < pDisc->SCSI.byLastDataTrkNum; k++) {
			if (pDisc->SCSI.lp1stLBAListOfDataTrkOnToc[k] != pDisc->SUB.lp1stLBAListOfDataTrackOnSub[k]) {
				pDisc->SUB.byCtlDesync = TRUE;
			}
		}
		if (pDisc->SUB.byCtlDesync) {
			ZeroMemory(pszNewPath, _MAX_PATH);
			_tcsncpy(pszNewPath, pszOutScmFile, SIZE_OF_ARRAY(pszNewPath));
			pszNewPath[_MAX_PATH - 1] = 0;
			PathRemoveExtension(pszNewPath);
			_tcsncat(pszNewPath, _T(" (Subs control).img"), sizeof(pszNewPath) - _tcslen(pszNewPath) - 1);

			OutputString("Copying .scm to .img (Subs control)\n");
			if (!CopyFile(pszOutScmFile, pszNewPath, FALSE)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			if (NULL == (fpImg = CreateOrOpenFile(
				pszPath, _T(" (Subs control)"), pszImgPath, NULL, NULL, _T(".img"), _T("rb+"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			DescrambleMainChannelAll(pExtArg, pDisc, pDisc->SUB.lp1stLBAListOfDataTrackOnSub
				, pDisc->SUB.lpLastLBAListOfDataTrackOnSub, _T("Sub based"), scrambled_table, fpImg);
			FcloseAndNull(fpImg);
			ExecEccEdc(pExtArg, pDisc->PROTECT, _T("Sub"), pszImgPath, pDisc->PROTECT.ERROR_SECTOR);
		}
	}
	return TRUE;
}

BOOL ProcessCreateBin(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszPath,
	FILE* fpCcd
) {
	FILE* fpImg = NULL;
	_TCHAR pszImgName[_MAX_FNAME + _MAX_EXT] = {};
	FILE* fpImgDesync = NULL;

	if (NULL == (fpImg = CreateOrOpenFile(
		pszPath, NULL, NULL, pszImgName, NULL, _T(".img"), _T("rb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	else {
		if (pDisc->SUB.byCtlDesync) {
			if (NULL == (fpImgDesync = CreateOrOpenFile(
				pszPath, _T(" (Subs control)"), NULL, NULL, NULL, _T(".img"), _T("rb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
		}
		if (!CreateBinCueCcd(pExtArg, pDisc, pszPath, pszImgName,
			pDevice->FEATURE.byCanCDText, fpImg, fpImgDesync, fpCcd)) {
		}
	}
	FcloseAndNull(fpImg);
	FcloseAndNull(fpImgDesync);
	return TRUE;
}

BOOL ReadCDAll(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	CDFLAG::_READ_CD::_ERROR_FLAGS c2,
	LPCTSTR pszPath,
	FILE* fpCcd,
	FILE* fpC2
) {
	FILE* fpScm = NULL;
	_TCHAR pszOutScmFile[_MAX_PATH] = {};
	if (NULL == (fpScm = CreateOrOpenFile(pszPath, NULL,
		pszOutScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	BOOL bRet = TRUE;
	FILE* fpSub = NULL;
	LPBYTE pBuf = NULL;
	LPBYTE pNextBuf = NULL;
	LPBYTE pNextNextBuf = NULL;
	INT nMainDataType = scrambled;

	try {
		// init start
		if (NULL == (fpSub = CreateOrOpenFile(
			pszPath, NULL, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		BYTE byTransferLen = 1;
		if (pDevice->byPlxtrDrive) {
			byTransferLen = (BYTE)(pExtArg->uiSubAddionalNum + 1);
		}
		// store main + (c2) + sub data all
		if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
			pDevice->TRANSFER.uiBufLen * byTransferLen, &pDiscPerSector->data.current, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		if (1 <= pExtArg->uiSubAddionalNum) {
			if (!GetAlignedCallocatedBuffer(pDevice, &pNextBuf,
				pDevice->TRANSFER.uiBufLen * byTransferLen, &pDiscPerSector->data.next, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			if (2 <= pExtArg->uiSubAddionalNum) {
				if (!GetAlignedCallocatedBuffer(pDevice, &pNextNextBuf,
					pDevice->TRANSFER.uiBufLen * byTransferLen, &pDiscPerSector->data.nextNext, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
			}
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
		SetReadDiscCommand(pExtArg, pDevice
			, byTransferLen, CDFLAG::_READ_CD::CDDA, c2, pDevice->sub, lpCmd, TRUE);

		BYTE lpPrevSubcode[CD_RAW_READ_SUBCODE_SIZE] = {};
		// to get prevSubQ
		if (pDisc->SUB.nSubChannelOffset) {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, -2, pDiscPerSector->data.current,
				pDevice->TRANSFER.uiBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
			SetTmpSubchFromBuffer(&pDiscPerSector->subch.prev, pDiscPerSector->subcode.current);

			if (!ExecReadCD(pExtArg, pDevice, lpCmd, -1, pDiscPerSector->data.current,
				pDevice->TRANSFER.uiBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
			SetTmpSubchFromBuffer(&pDiscPerSector->subch.current, pDiscPerSector->subcode.current);
			memcpy(lpPrevSubcode, pDiscPerSector->subcode.current, CD_RAW_READ_SUBCODE_SIZE);
		}
		else {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, -1, pDiscPerSector->data.current,
				pDevice->TRANSFER.uiBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
			SetTmpSubchFromBuffer(&pDiscPerSector->subch.prev, pDiscPerSector->subcode.current);
		}
		OutputSubInfoWithLBALog("Q[%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]\n"
			, -1, 0, pDiscPerSector->subcode.current[12]
			, pDiscPerSector->subcode.current[13], pDiscPerSector->subcode.current[14]
			, pDiscPerSector->subcode.current[15], pDiscPerSector->subcode.current[16]
			, pDiscPerSector->subcode.current[17], pDiscPerSector->subcode.current[18]
			, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
			, pDiscPerSector->subcode.current[21], pDiscPerSector->subcode.current[22]
			, pDiscPerSector->subcode.current[23]);
		// special fix begin
//		if (pDiscPerSector->subch.prev.byAdr != ADR_ENCODES_CURRENT_POSITION) {
			// [PCE] 1552 Tenka Tairan
			pDiscPerSector->subch.prev.byAdr = ADR_ENCODES_CURRENT_POSITION;
			pDiscPerSector->subch.prev.byTrackNum = 1;
			pDiscPerSector->subch.prev.byIndex = 0;
			pDiscPerSector->subch.prev.nAbsoluteTime = 149;
//		}
		if (!ReadCDForCheckingSecuROM(pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd)) {
			throw FALSE;
		}
		// special fix end

		for (UINT p = 0; p < pDisc->SCSI.toc.LastTrack; p++) {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, pDisc->SCSI.lp1stLBAListOnToc[p]
				, pDiscPerSector->data.current, pDevice->TRANSFER.uiBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				pDisc->SUB.lpEndCtlList[p] = pDisc->SCSI.toc.TrackData[p + 1].Control;
			}
			else {
				AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
				pDisc->SUB.lpEndCtlList[p] = (BYTE)((pDiscPerSector->subcode.current[12] >> 4) & 0x0f);
			}
			OutputString("\rChecking SubQ ctl (Track) %2u/%2u", p + 1, pDisc->SCSI.toc.LastTrack);
		}
		OutputString("\n");
		SetCDOffset(pExecType, pExtArg->byBe, pDevice->byPlxtrDrive, pDisc, 0, pDisc->SCSI.nAllLength);

		pDiscPerSector->byTrackNum = pDisc->SCSI.toc.FirstTrack;
		INT n1stLBAForSub = 0;
		INT n1stLBA = pDisc->MAIN.nOffsetStart;
		INT nLastLBA =  pDisc->SCSI.nAllLength + pDisc->MAIN.nOffsetEnd;
		INT nLBA = n1stLBA;

		if (pExtArg->byPre) {
			n1stLBAForSub = LEADIN_START_LBA_SESSION_1;
			n1stLBA = LEADIN_START_LBA_SESSION_1;
			nLBA = n1stLBA;
			pDisc->MAIN.nOffsetStart = LEADIN_START_LBA_SESSION_1;
		}
		// init end
		FlushLog();

		BOOL bReadOK = pExtArg->byPre ? FALSE : TRUE;
		BOOL bC2Error = FALSE;
		BOOL bReread = FALSE;
		INT nLastErrLBA = 0;
#ifdef _DEBUG
		INT n2ndSessionLBA = pDisc->MAIN.nFix1stLBAof2ndSession;
		if (pDisc->MAIN.nAdjustSectorNum < 0) {
			n2ndSessionLBA = pDisc->SCSI.n1stLBAof2ndSession;
		}
#endif
		INT nRetryCnt = 0;

		while (n1stLBA < nLastLBA) {
			if (pDisc->PROTECT.byExist == laserlock || pDisc->PROTECT.byExist == proring ||
				pDisc->PROTECT.byExist == physicalErr) {
				INT tmpLBA = nLBA;
				INT idx = GetReadErrorFileIdx(pExtArg, pDisc, nLBA);
				if (pDisc->PROTECT.ERROR_SECTOR.nExtentPos[idx] == nLBA) {
					tmpLBA = nLBA - pDisc->MAIN.nAdjustSectorNum;
				}
				if (IsValidProtectedSector(pDisc, tmpLBA, idx)) {
					ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
						, pDiscPerSector, nLBA, nLastErrLBA, nMainDataType, padByUsr55, fpScm, fpSub, fpC2);
					nLBA++;
					n1stLBA++;
					continue;
				}
			}

			pDiscPerSector->bReturnCode = FALSE;
			if (IsValid0xF1SupportedDrive(pDevice) && nLBA >= pDisc->SCSI.nAllLength) {
				INT nOfs = F1_BUFFER_SIZE * (nLBA - pDisc->SCSI.nAllLength);
				// main
				memcpy(pDiscPerSector->data.current, pDisc->lpCachedBuf + nOfs, CD_RAW_SECTOR_SIZE);
				// c2
				memcpy(pDiscPerSector->data.current + CD_RAW_SECTOR_SIZE, pDisc->lpCachedBuf + nOfs + 0x9A4, CD_RAW_READ_C2_294_SIZE);
				// sub
				memcpy(pDiscPerSector->data.current + CD_RAW_SECTOR_WITH_C2_294_SIZE, pDisc->lpCachedBuf + nOfs + CD_RAW_SECTOR_SIZE, CD_RAW_READ_SUBCODE_SIZE);
				AlignRowSubcode(pDiscPerSector->subcode.current, pDisc->lpCachedBuf + nOfs + CD_RAW_SECTOR_SIZE);
#ifdef _DEBUG
				OutputMainChannel(fileMainInfo, pDiscPerSector->data.current, NULL, nLBA, CD_RAW_SECTOR_SIZE);
				OutputCDSub96Align(fileMainInfo, pDiscPerSector->subcode.current, nLBA);
				OutputCDC2Error296(fileC2Error, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufC2Offset, nLBA);

				BYTE buf[CD_RAW_READ_SUBCODE_SIZE] = {};
				for (INT i = 0; i < 20; i++) {
					OutputMainChannel(fileMainInfo, pDisc->lpCachedBuf + F1_BUFFER_SIZE * i, NULL, nLBA + i, CD_RAW_SECTOR_SIZE);
					AlignRowSubcode(buf, pDisc->lpCachedBuf + F1_BUFFER_SIZE * i + CD_RAW_SECTOR_SIZE);
					OutputCDSub96Align(fileMainInfo, buf, nLBA + i);
				}
#endif
				pDiscPerSector->bReturnCode = TRUE;
			}
			else {
				pDiscPerSector->bReturnCode = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd, nLBA);
			}

			if (pDiscPerSector->bReturnCode == RETURNED_EXIST_C2_ERROR) {
				bC2Error = TRUE;
#ifdef _DEBUG
				OutputCDC2Error296(fileC2Error, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufC2Offset, nLBA);
				OutputCDC2Error296(fileC2Error, pDiscPerSector->data.next + pDevice->TRANSFER.uiBufC2Offset, nLBA + 1);
#endif
				OutputLog(standardError | fileC2Error,
					" LBA[%06d], LBA translation to SCM address in hex[%#x], LBA in C2 file[%#x] Detected C2 error %u bit\n"
					, nLBA, (UINT)(nLBA * CD_RAW_SECTOR_SIZE), (UINT)(nLBA * CD_RAW_READ_C2_294_SIZE), pDiscPerSector->uiC2errorNum);
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					if (!(IsValidIntentionalC2error(pDisc, pDiscPerSector, nLBA, GetC2ErrorFileIdx(pExtArg, pDisc, nLBA)))) {
						pDisc->MAIN.lpAllLBAOfC2Error[pDisc->MAIN.nC2ErrorCnt++] = nLBA;
					}
				}
#ifdef _DEBUG
				FILE* fpErrbin = NULL;
				_TCHAR tmp[16] = {};
				_sntprintf(tmp, SIZE_OF_ARRAY(tmp), _T("_%06d"), nLBA);
				if (NULL == (fpErrbin = CreateOrOpenFile(pszPath, tmp,
					NULL, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					return FALSE;
				}
				fwrite(pDiscPerSector->data.current, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpErrbin);
				FcloseAndNull(fpErrbin);
#endif
			}
			else if (pDiscPerSector->bReturnCode == RETURNED_SKIP_LBA) {
				nLBA = pDisc->MAIN.nFix1stLBAof2ndSession - 1;
				n1stLBA = nLBA;
			}
			else if (pDiscPerSector->bReturnCode == RETURNED_CONTINUE) {
				if (!(pExtArg->byPre && -1149 <= nLBA && nLBA <= -76)) {
					INT nPadType = padByUsr55;
					if ((pDisc->SCSI.toc.TrackData[pDiscPerSector->byTrackNum - 1].Control & AUDIO_DATA_TRACK) == 0) {
						nPadType = padByAll0;
					}
					ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
						, pDiscPerSector, nLBA, nLastErrLBA, nMainDataType, nPadType, fpScm, fpSub, fpC2);
					FlushLog();
				}
			}
			else if (pDiscPerSector->bReturnCode == RETURNED_FALSE) {
				throw FALSE;
			}

			if (pDiscPerSector->bReturnCode != RETURNED_CONTINUE &&
				pDiscPerSector->bReturnCode != RETURNED_SKIP_LBA) {
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData
					&& pExtArg->nC2RereadingType == 1 && n1stLBA - pDisc->MAIN.nOffsetStart >= 0) {
					GetCrc32(&pDisc->MAIN.lpAllSectorCrc32[n1stLBA - pDisc->MAIN.nOffsetStart]
						, pDiscPerSector->data.current, CD_RAW_SECTOR_SIZE);
				}
				if (pDisc->SUB.nSubChannelOffset) {
					if (!IsValidProtectedSector(pDisc, nLBA, GetReadErrorFileIdx(pExtArg, pDisc, nLBA))) {
						if (2 <= pExtArg->uiSubAddionalNum) {
							memcpy(pDiscPerSector->subcode.nextNext
								, pDiscPerSector->subcode.next, CD_RAW_READ_SUBCODE_SIZE);
						}
						if (1 <= pExtArg->uiSubAddionalNum) {
							memcpy(pDiscPerSector->subcode.next
								, pDiscPerSector->subcode.current, CD_RAW_READ_SUBCODE_SIZE);
						}
						memcpy(pDiscPerSector->subcode.current, lpPrevSubcode, CD_RAW_READ_SUBCODE_SIZE);
					}
				}
#if 0
				if ((pDisc->PROTECT.byExist == laserlock || pDisc->PROTECT.byExist == proring) &&
					pDiscPerSector->bReturnCode == RETURNED_EXIST_C2_ERROR) {
					SetBufferFromTmpSubch(pDiscPerSector->subcode.current, pDiscPerSector->subch.current, FALSE, TRUE);
				}
#endif
				SetTmpSubchFromBuffer(&pDiscPerSector->subch.current, pDiscPerSector->subcode.current);
				if (2 <= pExtArg->uiSubAddionalNum) {
					SetTmpSubchFromBuffer(&pDiscPerSector->subch.nextNext, pDiscPerSector->subcode.nextNext);
				}
				if (1 <= pExtArg->uiSubAddionalNum) {
					SetTmpSubchFromBuffer(&pDiscPerSector->subch.next, pDiscPerSector->subcode.next);
				}

				if (pExtArg->byPre && LEADIN_START_LBA_SESSION_1 <= nLBA && nLBA <= -76) {
					if (pDiscPerSector->subch.current.byTrackNum == 1 &&
						pDiscPerSector->subch.current.nAbsoluteTime == 0) {
						pDiscPerSector->subch.prev.nRelativeTime = pDiscPerSector->subch.current.nRelativeTime + 1;
						pDiscPerSector->subch.prev.nAbsoluteTime = -1;
						pDisc->MAIN.nFixStartLBA = nLBA;
						bReadOK = TRUE;
#if 0
						if (pDisc->MAIN.nAdjustSectorNum < 0 ||
							1 < pDisc->MAIN.nAdjustSectorNum) {
							for (INT i = 0; i < abs(pDisc->MAIN.nAdjustSectorNum) * CD_RAW_SECTOR_SIZE; i++) {
								fputc(0, fpScm);
							}
						}
#endif
					}
					if (bReadOK) {
						if ((pDiscPerSector->subch.current.byAdr == ADR_ENCODES_CURRENT_POSITION &&
							pDiscPerSector->subch.current.byTrackNum == 1 &&
							pDiscPerSector->subch.current.nAbsoluteTime == 74) ||
							((pDiscPerSector->subch.current.byAdr == ADR_ENCODES_MEDIA_CATALOG ||
							pDiscPerSector->subch.current.byAdr == ADR_ENCODES_ISRC) &&
							pDiscPerSector->subch.prev.byTrackNum == 1 &&
							pDiscPerSector->subch.prev.nAbsoluteTime == 73)
							) {
							n1stLBA = -76;
						}
					}
				}
#if 0
				if (pExtArg->byPre && LEADIN_START_LBA_SESSION_1 <= nLBA && nLBA <= -76) {
					OutputCDSub96Align(pDiscPerSector->subcode.current, nLBA);
					OutputMainChannel(fileDisc, pDiscPerSector->data.current, NULL, nLBA, CD_RAW_SECTOR_SIZE);
				}
#endif
				if (bReadOK) {
					if (n1stLBAForSub <= nLBA && nLBA < pDisc->SCSI.nAllLength) {
						if (IsCheckingSubChannel(pDisc, nLBA)) {
							pDiscPerSector->bLibCrypt = IsValidLibCryptSector(pExtArg->byLibCrypt, nLBA);
							pDiscPerSector->bSecuRom = IsValidSecuRomSector(pExtArg->byIntentionalSub, pDisc, nLBA);
							BOOL bReturn = FixSubChannel(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, nLBA, &bReread);

							BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = {};
							// fix raw subchannel
							AlignColumnSubcode(lpSubcodeRaw, pDiscPerSector->subcode.current);

							if (bReread && pDiscPerSector->bReturnCode != RETURNED_EXIST_C2_ERROR) {
								continue;
							}
							else if (!bReturn) {
								OutputMainChannel(fileMainError, pDiscPerSector->data.current, NULL, nLBA, 2352);
								OutputCDC2Error296(fileMainError, pDiscPerSector->data.current + 2352, nLBA);
								OutputCDSub96Raw(fileMainError, pDiscPerSector->data.current + 2352 + 294, nLBA);
								FlushLog();
								if (nRetryCnt++ == 0) {
									OutputErrorString(
										"\n" STR_LBA "Failed to reread because crc16 of subQ is 0. Read back 100 sector\n", nLBA, (UINT)nLBA);
									FlushDriveCache(pExtArg, pDevice, nLBA);
									SetDiscSpeed(pExecType, pExtArg, pDevice, 24);
									continue;
								}
								else if (nRetryCnt++ == 1) {
									DISC_PER_SECTOR tmp = {};
									memcpy(&tmp, pDiscPerSector,sizeof(DISC_PER_SECTOR));
									ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, &tmp, lpCmd, nLBA - 100);
									continue;
								}
								else {
									OutputErrorString(
										"\n" STR_LBA "Failed to reread because crc16 of subQ is 0\n"
										"1. If your disc has scratches, it needs to be polished\n"
										"2. Try to dump with different drive speed\n"
										"3. Try to dump with different drive\n"
										, nLBA, (UINT)nLBA);
									throw FALSE;
								}
							}
#if 0
							OutputCDSub96Align(pDiscPerSector->subcode.current, nLBA);
							OutputCDSub96Raw(standardOut, lpSubcodeRaw, nLBA);
#endif
							WriteSubChannel(pDisc, pDiscPerSector, lpSubcodeRaw, nLBA, fpSub);
							SetTrackAttribution(pExecType, pExtArg, pDisc, pDiscPerSector, nLBA);
							FixMainHeader(pExtArg, pDisc, pDiscPerSector, nLBA, nMainDataType);
							UpdateTmpSubch(pDiscPerSector);
						}
					}
					// Write track to scrambled
					WriteMainChannel(pExecType, pExtArg, pDisc, pDiscPerSector->data.current, nLBA, fpScm);
#ifdef _DEBUG
					if (nLBA == pDisc->MAIN.nFix1stLBAof2ndSession || n2ndSessionLBA == nLBA) {
						OutputMainChannel(fileMainInfo, pDiscPerSector->data.current, NULL, nLBA, CD_RAW_SECTOR_SIZE);
					}
#endif
					WriteC2(pExtArg, pDevice, pDiscPerSector, fpC2);
				}
#if 0
				else {
					BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = {};
					AlignColumnSubcode(lpSubcodeRaw, pDiscPerSector->subcode.current);
					OutputCDSubToLog(pDisc, pDiscPerSector->subcode.current, lpSubcodeRaw, nLBA, byCurrentTrackNum);
				}
#endif
				if (pDisc->SUB.nSubChannelOffset) {
					memcpy(lpPrevSubcode, pDiscPerSector->subcode.next, CD_RAW_READ_SUBCODE_SIZE);
				}
			}

			OutputString("\rCreating .scm (LBA) %6d/%6d", nLBA, nLastLBA - 1);
			if (n1stLBA == -76) {
				nLBA = n1stLBA;
				if (!bReadOK) {
					bReadOK = TRUE;
				}
			}
			nLBA++;
			n1stLBA++;
		}
		OutputString("\n");
		FcloseAndNull(fpSub);
		FlushLog();

		for (INT i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
			INT i2 = i;
			if (pDisc->SCSI.byFormat == DISK_TYPE_CDI && pDisc->SCSI.toc.LastTrack > 1) {
				i2 -= 1;
				if (i2 < 0) {
					continue;
				}
			}
			if (pDisc->PROTECT.byExist == cds300 && i == pDisc->SCSI.toc.LastTrack - 1) {
				break;
			}
#if 0
			OutputString("lp1stLBAListOnSub[%d][0]: %d, [%d][1]: %d\n"
				, i, pDisc->SUB.lp1stLBAListOnSub[i][0], i, pDisc->SUB.lp1stLBAListOnSub[i][1]);
#endif
			BOOL bErr = FALSE;
			LONG lLine = 0;
			if (pDisc->SUB.lp1stLBAListOnSub[i][1] == -1) {
				bErr = TRUE;
				lLine = __LINE__;
			}
			else if ((pDisc->SCSI.toc.TrackData[i2].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				if (pDisc->SUB.lp1stLBAListOfDataTrackOnSub[i] == -1) {
					bErr = TRUE;
					lLine = __LINE__;
				}
				else if (pDisc->SUB.lpLastLBAListOfDataTrackOnSub[i] == -1) {
					bErr = TRUE;
					lLine = __LINE__;
				}
			}
			if (bErr) {
				OutputLog(standardError | fileSubError,
					"[L:%ld] Internal error. Failed to analyze the subchannel. Track[%02d]/[%02u]\n",
					lLine, i + 1, pDisc->SCSI.toc.LastTrack);
				throw FALSE;
			}
		}
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (bC2Error && pDisc->MAIN.nC2ErrorCnt > 0) {
				if (pExtArg->nC2RereadingType == 0) {
					if (!ReadCDForRereadingSectorType1(pExecType, pExtArg, pDevice
						, pDisc, pDiscPerSector, CDFLAG::_READ_CD::CDDA, lpCmd, fpScm, fpC2, 0)) {
						throw FALSE;
					}
				}
				else {
					INT nStartLBA = (INT)pExtArg->nStartLBAForC2;
					INT nEndLBA = (INT)pExtArg->nEndLBAForC2;
					if (nStartLBA == 0 && nEndLBA == 0) {
						nStartLBA = pDisc->MAIN.nOffsetStart;
						nEndLBA = pDisc->SCSI.nAllLength + pDisc->MAIN.nOffsetEnd;
					}
					if (!ReadCDForRereadingSectorType2(pExecType, pExtArg, pDevice, pDisc
						, CDFLAG::_READ_CD::CDDA, lpCmd, fpScm, fpC2, nStartLBA, nEndLBA)) {
						throw FALSE;
					}
				}
			}
			else {
				if (pDisc->PROTECT.byExist == PROTECT_TYPE_CD::no) {
					OutputLog(standardOut | fileC2Error, "No C2 errors\n");
				}
				else {
					OutputLog(standardOut | fileC2Error, "No unintentional C2 errors\n");
				}
			}
		}
		FcloseAndNull(fpScm);
		OutputTocWithPregap(pDisc);
		FlushLog();

		if (!ProcessDescramble(pExtArg, pDisc, pszPath, pszOutScmFile)) {
			throw FALSE;
		}
		if (!ProcessCreateBin(pExtArg, pDevice, pDisc, pszPath, fpCcd)) {
			throw FALSE;
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpScm);
	FcloseAndNull(fpSub);
	if (IsValid0xF1SupportedDrive(pDevice)) {
		FreeAndNull(pDisc->lpCachedBuf);
	}
	FreeAndNull(pBuf);
	if (1 <= pExtArg->uiSubAddionalNum) {
		FreeAndNull(pNextBuf);
		if (2 <= pExtArg->uiSubAddionalNum) {
			FreeAndNull(pNextNextBuf);
		}
	}

	return bRet;
}

BOOL ReadCDForSwap(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	CDFLAG::_READ_CD::_ERROR_FLAGS c2,
	LPCTSTR pszPath,
	INT nStart,
	INT nEnd,
	FILE* fpCcd,
	FILE* fpC2
) {
	_TCHAR pszScmPath[_MAX_PATH] = {};
#ifndef __DEBUG
	FILE* fpScm = CreateOrOpenFile(pszPath, NULL, pszScmPath, NULL, NULL, _T(".scm"), _T("wb"), 0, 0);
#else
	FILE* fpScm = CreateOrOpenFile(pszPath, NULL, pszScmPath, NULL, NULL, _T(".scm"), _T("rb"), 0, 0);
#endif
	if (!fpScm) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	FILE* fpSub = NULL;
	FILE* fpLeadout = NULL;
	LPBYTE pBuf = NULL;
	INT nMainDataType = scrambled;
	if (pExtArg->byBe) {
		nMainDataType = unscrambled;
	}

	try {
		// init start
#ifndef __DEBUG
		if (NULL == (fpSub = CreateOrOpenFile(
			pszPath, NULL, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (pExtArg->by74Min) {
			if (NULL == (fpLeadout = CreateOrOpenFile(
				pszPath, _T("_leadout"), NULL, NULL, NULL, _T(".bin"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}

		BYTE byTransferLen = 1;
		// store main+(c2)+sub data
		if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
			pDevice->TRANSFER.uiBufLen * byTransferLen, &pDiscPerSector->data.current, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
		CDFLAG::_READ_CD::_EXPECTED_SECTOR_TYPE type = CDFLAG::_READ_CD::CDDA;
		if (pExtArg->byBe) {
			type = CDFLAG::_READ_CD::All;
		}
		SetReadDiscCommand(pExtArg, pDevice, byTransferLen, type, c2, pDevice->sub, lpCmd, TRUE);
		SetCDOffset(pExecType, pExtArg->byBe, pDevice->byPlxtrDrive, pDisc, nStart, nEnd);
		pDiscPerSector->byTrackNum = 1;

		INT n1stLBA = nStart + pDisc->MAIN.nOffsetStart;
		INT nLastLBA = nEnd + pDisc->MAIN.nOffsetEnd;
		INT nLBA = n1stLBA;
		// init end
		FlushLog();

		BOOL bC2Error = FALSE;
		BOOL bReread = FALSE;
		BOOL bSetLastLBA = FALSE;
		INT nFirstLeadoutLBA = 0;
		INT nFirstLeadErrLBA = 0;
		INT nSecondLeadErrLBA = 0;

		if (pExtArg->by74Min) {
			nLastLBA = 333000 - pDisc->MAIN.nOffsetEnd;
			pDisc->MAIN.nFixEndLBA = nLastLBA;
			nEnd = nLastLBA;
		}

		while (n1stLBA < nLastLBA) {
			pDiscPerSector->bReturnCode = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd, nLBA);
			if (pDiscPerSector->bReturnCode == RETURNED_EXIST_C2_ERROR) {
				bC2Error = TRUE;
				// C2 error points the current LBA - 1 (offset?)
				OutputLog(standardError | fileC2Error,
					"\rLBA[%06d, %#07x] Detected C2 error %u bit\n", nLBA, (UINT)nLBA, pDiscPerSector->uiC2errorNum);
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					if (!(IsValidIntentionalC2error(pDisc, pDiscPerSector, nLBA, GetC2ErrorFileIdx(pExtArg, pDisc, nLBA)))) {
						pDisc->MAIN.lpAllLBAOfC2Error[pDisc->MAIN.nC2ErrorCnt++] = nLBA;
					}
				}
			}
			else if (pDiscPerSector->bReturnCode == RETURNED_CONTINUE) {
				if (SECOND_ERROR_OF_LEADOUT < nLBA) {
					if (nSecondLeadErrLBA == 0) {
						nSecondLeadErrLBA = nLBA;
						pDisc->MAIN.nFixEndLBA = nLBA + pDisc->MAIN.nOffsetEnd;
					}
					ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
						, pDiscPerSector, nLBA, 0, nMainDataType, padByAll0, fpScm, fpSub, fpC2);
					if (pDisc->MAIN.nFixEndLBA == nLBA + 1) {
						break;
					}
				}
				else if (FIRST_ERROR_OF_LEADOUT < nLBA) {
					if (nFirstLeadErrLBA == 0) {
						nFirstLeadErrLBA = nLBA;
					}
					if (nLBA < nFirstLeadErrLBA + 670) {
						for (INT i = 0; i < 670; i++) {
							ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
								, pDiscPerSector, nLBA, 0, nMainDataType, padByAll0, fpScm, fpSub, fpC2);
							nLBA++;
							n1stLBA++;
						}
						continue;
					}
					else {
						ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
							, pDiscPerSector, nLBA, 0, nMainDataType, padByAll0, fpScm, fpSub, fpC2);
					}
				}
				else {
					ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
						, pDiscPerSector, nLBA, 0, nMainDataType, padByAll0, fpScm, fpSub, fpC2);
				}
			}
			else if (pDiscPerSector->bReturnCode == RETURNED_FALSE) {
				throw FALSE;
			}

			if (pDiscPerSector->bReturnCode != RETURNED_CONTINUE) {
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData
					&& pExtArg->nC2RereadingType == 1 && n1stLBA - pDisc->MAIN.nOffsetStart >= 0) {
					GetCrc32(&pDisc->MAIN.lpAllSectorCrc32[n1stLBA - pDisc->MAIN.nOffsetStart]
						, pDiscPerSector->data.current, CD_RAW_SECTOR_SIZE);
				}
				SetTmpSubchFromBuffer(&pDiscPerSector->subch.current, pDiscPerSector->subcode.current);
				if (2 <= pExtArg->uiSubAddionalNum) {
					SetTmpSubchFromBuffer(&pDiscPerSector->subch.nextNext, pDiscPerSector->subcode.nextNext);
				}
				if (1 <= pExtArg->uiSubAddionalNum) {
					SetTmpSubchFromBuffer(&pDiscPerSector->subch.next, pDiscPerSector->subcode.next);
				}
				BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = {};
				if (!bSetLastLBA) {
					if (nFirstLeadoutLBA == 0 && pDiscPerSector->subcode.current[13] == 0xaa) {
						nFirstLeadoutLBA = nLBA;
						if (!pExtArg->by74Min) {
							pDisc->MAIN.nFixEndLBA = nLBA + pDisc->MAIN.nAdjustSectorNum;
							nLastLBA = nLBA + pDisc->MAIN.nOffsetEnd;
							bSetLastLBA = TRUE;
						}
						else {
							AlignColumnSubcode(lpSubcodeRaw, pDiscPerSector->subcode.current);
							WriteSubChannel(pDisc, pDiscPerSector, lpSubcodeRaw, nLBA, fpSub);
						}
						FlushLog();
					}
					else {
						if (IsCheckingSubChannel(pDisc, nLBA)) {
							BOOL bReturn = FixSubChannel(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, nLBA, &bReread);
							if (bReread) {
								continue;
							}
							else if (!bReturn) {
								OutputErrorString(
									"\n" STR_LBA "Failed to reread because crc16 of subQ is 0\n"
									"1. If your disc has scratches, it needs to be polished\n"
									"2. Try to dump with different drive speed\n"
									"3. Try to dump with different drive\n"
									, nLBA, (UINT)nLBA);
								throw FALSE;
							}
							// fix raw subchannel
							AlignColumnSubcode(lpSubcodeRaw, pDiscPerSector->subcode.current);
#if 0
							OutputCDSub96Align(pDiscPerSector->subcode.current, nLBA);
#endif
							WriteSubChannel(pDisc, pDiscPerSector, lpSubcodeRaw, nLBA, fpSub);
							FixMainHeader(pExtArg, pDisc, pDiscPerSector, nLBA, nMainDataType);
						}
						UpdateTmpSubch(pDiscPerSector);
					}
				}
				// Write track to scrambled
				WriteMainChannel(pExecType, pExtArg, pDisc, pDiscPerSector->data.current, nLBA, fpScm);
				WriteC2(pExtArg, pDevice, pDiscPerSector, fpC2);
			}
			OutputString("\rCreating .scm from %d to %d (LBA) %6d"
				, nStart + pDisc->MAIN.nOffsetStart, nEnd + pDisc->MAIN.nOffsetEnd, nLBA);
			nLBA++;
			n1stLBA++;
		}
		OutputString("\n");
		FcloseAndNull(fpSub);
		FlushLog();
		nLBA = 0;

		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (bC2Error && pDisc->MAIN.nC2ErrorCnt > 0) {
				if (pExtArg->nC2RereadingType == 0) {
					if (!ReadCDForRereadingSectorType1(pExecType, pExtArg
						, pDevice, pDisc, pDiscPerSector, type, lpCmd, fpScm, fpC2, 0)) {
						throw FALSE;
					}
				}
				else {
					INT nStartLBA = (INT)pExtArg->nStartLBAForC2;
					INT nEndLBA = (INT)pExtArg->nEndLBAForC2;
					if (nStartLBA == 0 && nEndLBA == 0) {
						nStartLBA = nStart;
						nEndLBA = nEnd;
					}
					if (!ReadCDForRereadingSectorType2(pExecType, pExtArg
						, pDevice, pDisc, type, lpCmd, fpScm, fpC2, nStartLBA, nEndLBA)) {
						throw FALSE;
					}
				}
			}
			else {
				if (pDisc->PROTECT.byExist == PROTECT_TYPE_CD::no) {
					OutputLog(standardOut | fileC2Error, "No C2 errors\n");
				}
				else {
					OutputLog(standardOut | fileC2Error, "No unintentional C2 errors\n");
				}
			}
		}
		FcloseAndNull(fpScm);
		FlushLog();

		// eject
		bRet = StartStopUnit(pExtArg, pDevice, STOP_UNIT_CODE, START_UNIT_CODE);
		if (!CloseHandle(pDevice->hDevice)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (pDevice->byLoadingMechanism == 1) {
			OutputString("Close the tray automatically after 3000 msec\n");
		}
		else {
			OutputString("Wait 3000 msec\n");
		}
		Sleep(3000);
		if (!GetHandle(pDevice)) {
			throw FALSE;
		}
		if (pDevice->byLoadingMechanism == 1) {
			// close
			bRet = StartStopUnit(pExtArg, pDevice, START_UNIT_CODE, START_UNIT_CODE);
		}
		else {
			OutputString("Insert the disc again\n");
			stopMessage();
		}
		OutputString("Wait 15000 msec until your drive recognizes the disc\n");
		Sleep(15000);

		OutputString("Read TOC\n");
#endif
#ifdef _WIN32
		_declspec(align(4)) CDROM_TOC_FULL_TOC_DATA fullToc = { 0 };
#else
		__attribute__((aligned(4))) CDROM_TOC_FULL_TOC_DATA fullToc = {};
#endif
		PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData = NULL;
		WORD wTocEntries = 0;
		LPBYTE pPFullToc = NULL;
		if (!ReadTOCFull(pExtArg, pDevice, pDisc, &fullToc, &pTocData, &wTocEntries, &pPFullToc)) {
			throw FALSE;
		}
		if (!ReadTOC(pExtArg, pExecType, pDevice, pDisc, pszPath)) {
			throw FALSE;
		}
		pExtArg->byBe = TRUE;
		WriteCcdFirst(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, &fullToc, pTocData, wTocEntries, fpCcd);
		pExtArg->byBe = FALSE;
		SetAndOutputTocFull(pDisc, &fullToc, pTocData, wTocEntries, fpCcd);
		FreeAndNull(pPFullToc);

		if (!ReadCDCheck(pExecType, pExtArg, pDevice, pDisc)) {
			throw FALSE;
		}

		if (NULL == (fpScm = CreateOrOpenFile(
			pszPath, NULL, NULL, NULL, NULL, _T(".scm"), _T("rb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpSub = CreateOrOpenFile(
			pszPath, NULL, NULL, NULL, NULL, _T(".sub"), _T("rb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		DWORD size = GetFileSize(0, fpSub);
#ifdef __DEBUG
		INT nLBA = 0;
#endif
		pDiscPerSector->byTrackNum = 1;
		for (DWORD i = 0; i < size; i += CD_RAW_READ_SUBCODE_SIZE) {
			if (fread(pDiscPerSector->subcode.current, sizeof(BYTE), CD_RAW_READ_SUBCODE_SIZE, fpSub) < CD_RAW_READ_SUBCODE_SIZE) {
				OutputErrorString("Failed to read [F:%s][L:%d]\n", _T(__FUNCTION__), __LINE__);
				break;
			}
			SetTmpSubchFromBuffer(&pDiscPerSector->subch.current, pDiscPerSector->subcode.current);
			if (2 <= pExtArg->uiSubAddionalNum) {
				SetTmpSubchFromBuffer(&pDiscPerSector->subch.nextNext, pDiscPerSector->subcode.nextNext);
			}
			if (1 <= pExtArg->uiSubAddionalNum) {
				SetTmpSubchFromBuffer(&pDiscPerSector->subch.next, pDiscPerSector->subcode.next);
			}
			if (nLBA < nFirstLeadErrLBA) {
				FixSubChannel(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, nLBA, &bReread);
			}
			if (pDiscPerSector->subch.current.nAbsoluteTime == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->byTrackNum - 1] + 150) {
				if (fread(pDiscPerSector->mainHeader.current, sizeof(BYTE), MAINHEADER_MODE1_SIZE, fpScm) < MAINHEADER_MODE1_SIZE) {
					OutputErrorString("Failed to read [F:%s][L:%d]\n", _T(__FUNCTION__), __LINE__);
					break;
				}
				fseek(fpScm, CD_RAW_SECTOR_SIZE * pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->byTrackNum], SEEK_SET);
#if 0
				OutputMainChannel(fileDisc, pDiscPerSector->mainHeader.current
					, NULL, pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->byTrackNum - 1], MAINHEADER_MODE1_SIZE);
#endif
			}
			SetTrackAttribution(pExecType, pExtArg, pDisc, pDiscPerSector, nLBA);
			UpdateTmpSubch(pDiscPerSector);
			nLBA++;
		}
		FcloseAndNull(fpScm);

		if (!ProcessDescramble(pExtArg, pDisc, pszPath, pszScmPath)) {
			throw FALSE;
		}
		if (!ProcessCreateBin(pExtArg, pDevice, pDisc, pszPath, fpCcd)) {
			throw FALSE;
		}
		if (pExtArg->by74Min) {
			FILE* fpImg = NULL;
			if (NULL == (fpImg = CreateOrOpenFile(
				pszPath, NULL, NULL, NULL, NULL, _T(".img"), _T("rb+"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			fseek(fpImg, CD_RAW_SECTOR_SIZE * pDisc->SCSI.nAllLength, SEEK_SET);
			INT nMaxSize = pDisc->MAIN.nFixEndLBA - pDisc->MAIN.nOffsetEnd;

			for (INT i = pDisc->SCSI.nAllLength; i < nMaxSize; i++) {
				if (fread(pDiscPerSector->data.current, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg) < CD_RAW_SECTOR_SIZE) {
					OutputErrorString("Failed to read [F:%s][L:%d]\n", _T(__FUNCTION__), __LINE__);
					break;
				}
				if (IsValidMainDataHeader(pDiscPerSector->data.current)) {
					fseek(fpImg, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
					for (INT n = 0; n < CD_RAW_SECTOR_SIZE; n++) {
						pDiscPerSector->data.current[n] ^= scrambled_table[n];
					}
					fwrite(pDiscPerSector->data.current, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
					fseek(fpImg, 0, SEEK_CUR);
				}
				OutputString(
					"\rDescrambling lead-out of img (LBA) %6d/%6d", i, nMaxSize - 1);
			}
			OutputString("\n");
			FcloseAndNull(fpImg);

			if (NULL == (fpImg = CreateOrOpenFile(
				pszPath, NULL, NULL, NULL, NULL, _T(".img"), _T("rb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			fseek(fpImg, CD_RAW_SECTOR_SIZE * pDisc->SCSI.nAllLength, SEEK_SET);

			for (INT i = pDisc->SCSI.nAllLength; i < nMaxSize; i++) {
				if (fread(pDiscPerSector->data.current, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg) < CD_RAW_SECTOR_SIZE) {
					OutputErrorString("Failed to read [F:%s][L:%d]\n", _T(__FUNCTION__), __LINE__);
					break;
				}
				fwrite(pDiscPerSector->data.current, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpLeadout);
				OutputString(
					"\rCopying lead-out of img to bin (LBA) %6d/%6d", i, nMaxSize - 1);
			}
			OutputString("\n");
			FcloseAndNull(fpImg);
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpLeadout);
	FcloseAndNull(fpScm);
	FcloseAndNull(fpSub);
	FreeAndNull(pBuf);

	return bRet;
}

BOOL ReadCDPartial(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	CDFLAG::_READ_CD::_ERROR_FLAGS c2,
	LPCTSTR pszPath,
	INT nStart,
	INT nEnd,
	FILE* fpC2
) {
	CONST INT size = 5;
	_TCHAR szExt[size] = {};

	if (*pExecType == gd) {
		_tcsncpy(szExt, _T(".scm"), size);
	}
	else {
		_tcsncpy(szExt, _T(".bin"), size);
	}
	_TCHAR pszBinPath[_MAX_PATH] = {};
	FILE* fpBin = NULL;
	if (pExtArg->byReverse) {
		fpBin = CreateOrOpenFile(pszPath, _T("_reverse"), pszBinPath, NULL, NULL, szExt, _T("wb"), 0, 0);
	}
	else {
		fpBin = CreateOrOpenFile(pszPath, NULL, pszBinPath, NULL, NULL, szExt, _T("wb"), 0, 0);
	}
	if (!fpBin) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	FILE* fpSub = NULL;
	LPBYTE pBuf = NULL;
	LPBYTE pNextBuf = NULL;
	LPBYTE pNextNextBuf = NULL;
	INT nMainDataType = scrambled;
	if (*pExecType == data || /*pExtArg->byBe ||*/ pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
		nMainDataType = unscrambled;
	}
	INT nPadType = padByUsr55;
	if (pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
		nPadType = padByAll0;
	}

	try {
		// init start
		if (!pExtArg->byReverse) {
			if (NULL == (fpSub = CreateOrOpenFile(
				pszPath, NULL, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}
		BYTE byTransferLen = 1;
		if (pDevice->byPlxtrDrive) {
			byTransferLen = (BYTE)(pExtArg->uiSubAddionalNum + 1);
		}
		// store main+(c2)+sub data
		if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
			pDevice->TRANSFER.uiBufLen * byTransferLen, &pDiscPerSector->data.current, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		if (1 <= pExtArg->uiSubAddionalNum) {
			if (!GetAlignedCallocatedBuffer(pDevice, &pNextBuf,
				pDevice->TRANSFER.uiBufLen * byTransferLen, &pDiscPerSector->data.next, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			if (2 <= pExtArg->uiSubAddionalNum) {
				if (!GetAlignedCallocatedBuffer(pDevice, &pNextNextBuf,
					pDevice->TRANSFER.uiBufLen * byTransferLen, &pDiscPerSector->data.nextNext, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
			}
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
		CDFLAG::_READ_CD::_EXPECTED_SECTOR_TYPE type = CDFLAG::_READ_CD::CDDA;
		if (*pExecType == data || !pDevice->bySupportedScrambled) {
			type = CDFLAG::_READ_CD::All;
		}
		SetReadDiscCommand(pExtArg, pDevice, byTransferLen, type, c2, pDevice->sub, lpCmd, TRUE);

		BYTE lpPrevSubcode[CD_RAW_READ_SUBCODE_SIZE] = {};
		if (pDisc->SUB.nSubChannelOffset) { // confirmed PXS88T, TS-H353A
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, nStart - 2, pDiscPerSector->data.current,
				pDevice->TRANSFER.uiBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				if (nStart == 0) {
					pDiscPerSector->subch.current.nRelativeTime = 0;
					pDiscPerSector->subch.current.nAbsoluteTime = 149;
					SetBufferFromTmpSubch(pDiscPerSector->subcode.current, pDiscPerSector->subch.current, TRUE, TRUE);
					SetTmpSubchFromBuffer(&pDiscPerSector->subch.prev, pDiscPerSector->subcode.current);
				}
				else {
					throw FALSE;
				}
			}
			else {
				AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
				SetTmpSubchFromBuffer(&pDiscPerSector->subch.prev, pDiscPerSector->subcode.current);
			}
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, nStart - 1, pDiscPerSector->data.current,
				pDevice->TRANSFER.uiBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				if (nStart == 0) {
					pDiscPerSector->subch.current.nRelativeTime = 0;
					pDiscPerSector->subch.current.nAbsoluteTime = 150;
					SetBufferFromTmpSubch(pDiscPerSector->subcode.current, pDiscPerSector->subch.current, TRUE, TRUE);
					for (INT i = 0; i < 12; i++) {
						pDiscPerSector->subcode.current[i] = 0xff;
					}
				}
				else {
					throw FALSE;
				}
			}
			else {
				AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
				SetTmpSubchFromBuffer(&pDiscPerSector->subch.current, pDiscPerSector->subcode.current);
			}
			memcpy(lpPrevSubcode, pDiscPerSector->subcode.current, CD_RAW_READ_SUBCODE_SIZE);
		}
		else {
			if (*pExecType != gd && *pExecType != data) {
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, nStart - 1, pDiscPerSector->data.current,
					pDevice->TRANSFER.uiBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				}
				else {
					AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
					SetTmpSubchFromBuffer(&pDiscPerSector->subch.prev, pDiscPerSector->subcode.current);
				}
			}
		}
		if (*pExecType == gd) {
			for (INT p = pDisc->SCSI.toc.FirstTrack - 1; p < pDisc->SCSI.toc.LastTrack; p++) {
				pDisc->SUB.lpEndCtlList[p] = pDisc->SCSI.toc.TrackData[p].Control;
			}
		}
		else {
			for (UINT p = 0; p < pDisc->SCSI.toc.LastTrack; p++) {
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, pDisc->SCSI.lp1stLBAListOnToc[p]
					, pDiscPerSector->data.current, pDevice->TRANSFER.uiBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
					pDisc->SUB.lpEndCtlList[p] = pDisc->SCSI.toc.TrackData[p + 1].Control;
				}
				else {
					AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
					pDisc->SUB.lpEndCtlList[p] = (BYTE)((pDiscPerSector->subcode.current[12] >> 4) & 0x0f);
					OutputString("\rChecking SubQ ctl (Track) %2u/%2u", p + 1, pDisc->SCSI.toc.LastTrack);
				}
			}
			OutputString("\n");
		}
		SetCDOffset(pExecType, pExtArg->byBe, pDevice->byPlxtrDrive, pDisc, nStart, nEnd);
#ifdef _DEBUG
		OutputString(
			"byBe: %d, nCombinedOffset: %d, uiMainDataSlideSize: %u, nOffsetStart: %d, nOffsetEnd: %d, nFixStartLBA: %d, nFixEndLBA: %d\n"
			, pExtArg->byBe, pDisc->MAIN.nCombinedOffset, pDisc->MAIN.uiMainDataSlideSize
			, pDisc->MAIN.nOffsetStart, pDisc->MAIN.nOffsetEnd, pDisc->MAIN.nFixStartLBA, pDisc->MAIN.nFixEndLBA);
#endif
		pDiscPerSector->byTrackNum = pDiscPerSector->subch.prev.byTrackNum;
		if (pDiscPerSector->byTrackNum < pDisc->SCSI.toc.FirstTrack ||
			pDisc->SCSI.toc.LastTrack < pDiscPerSector->byTrackNum) {
			pDiscPerSector->byTrackNum = pDisc->SCSI.toc.FirstTrack;
		}
#ifdef _DEBUG
		OutputString(
			"byBe: %d, nCombinedOffset: %d, uiMainDataSlideSize: %u, nOffsetStart: %d, nOffsetEnd: %d, nFixStartLBA: %d, nFixEndLBA: %d\n"
			, pExtArg->byBe, pDisc->MAIN.nCombinedOffset, pDisc->MAIN.uiMainDataSlideSize
			, pDisc->MAIN.nOffsetStart, pDisc->MAIN.nOffsetEnd, pDisc->MAIN.nFixStartLBA, pDisc->MAIN.nFixEndLBA);
#endif
		INT n1stLBA = nStart + pDisc->MAIN.nOffsetStart - 1;
		if (pDisc->SCSI.trkType == audioOnly) {
			n1stLBA = nStart;
		}
//		if (*pExecType == data) {
//			n1stLBA++;
//		}
		INT nLastLBA = nEnd + pDisc->MAIN.nOffsetEnd;
		if (*pExecType == gd) {
			if (n1stLBA < 0) {
				n1stLBA = 0;
			}
			else if (n1stLBA == 44849) {
				n1stLBA = FIRST_PREGAP_LBA_FOR_GD;
			}
		}
		INT nLBA = n1stLBA;
		if (pExtArg->byReverse) {
			nLBA = nLastLBA;
		}
		// init end
		FlushLog();

		INT nStoreLBA = 0;
		INT nRetryCnt = 1;
		BOOL bC2Error = FALSE;
		INT bReread = FALSE;
		BOOL bForceSkip = TRUE;
		BOOL bForceSkip2 = TRUE;

		if (pExtArg->uiSkipSectors != 0) {
			bForceSkip = FALSE;
		}
		if (pDisc->PROTECT.byExist == laserlock && pExtArg->uiSkipSectors2 != 0) {
			bForceSkip2 = FALSE;
		}
		
		while (n1stLBA < nLastLBA) {
			pDiscPerSector->bReturnCode = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd, nLBA);
			if (pDiscPerSector->bReturnCode == RETURNED_EXIST_C2_ERROR) {
				bC2Error = TRUE;
				// C2 error points the current LBA - 1 (offset?)
				OutputLog(standardError | fileC2Error,
					" LBA[%06d], LBA translation to SCM address in hex[%#x], LBA in C2 file[%#x] Detected C2 error %u bit\n"
					, nLBA, (UINT)(nLBA * CD_RAW_SECTOR_SIZE), (UINT)(nLBA* CD_RAW_READ_C2_294_SIZE), pDiscPerSector->uiC2errorNum);
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					if (!(IsValidIntentionalC2error(pDisc, pDiscPerSector, nLBA, GetC2ErrorFileIdx(pExtArg, pDisc, nLBA)))) {
						pDisc->MAIN.lpAllLBAOfC2Error[pDisc->MAIN.nC2ErrorCnt++] = nLBA;
					}
				}
			}
			else if (pDiscPerSector->bReturnCode == RETURNED_SKIP_LBA) {
				if (pExtArg->byReverse) {
					nLBA = pDisc->SCSI.n1stLBAof2ndSession - SESSION_TO_SESSION_SKIP_LBA;
				}
				else {
					nLBA = pDisc->MAIN.nFix1stLBAof2ndSession - 1;
				}
				n1stLBA = nLBA;
			}
			else if (pDiscPerSector->bReturnCode == RETURNED_CONTINUE) {
				if (pDisc->PROTECT.byExist != physicalErr && (!bForceSkip || !bForceSkip2)) {
					if (pDisc->PROTECT.byExist == proring || pDisc->PROTECT.byExist == laserlock) {
						if (!bForceSkip) {
							for (UINT i = 0; i < pExtArg->uiSkipSectors; i++) {
								ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector
									, nLBA, 0, nMainDataType, nPadType, fpBin, fpSub, fpC2);
								nLBA++;
								n1stLBA++;
							}
							bForceSkip = TRUE;
						}
						else if (!bForceSkip2) {
							for (UINT i = 0; i < pExtArg->uiSkipSectors2; i++) {
								ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector
									, nLBA, 0, nMainDataType, nPadType, fpBin, fpSub, fpC2);
								nLBA++;
								n1stLBA++;
							}
							bForceSkip2 = TRUE;
						}
						continue;
					}
				}
				else {
					if ((nRetryCnt <= 2 && pDisc->PROTECT.byExist == proring) ||
						(nRetryCnt <= 10 && (pDisc->PROTECT.byExist == laserlock || pDisc->PROTECT.byExist == physicalErr))) {
						FlushDriveCache(pExtArg, pDevice, nLBA - 1);
						Sleep(1000);
						OutputLog(standardError | fileMainError, "Retry %d/10", nRetryCnt);
						nRetryCnt++;
						continue;
					}
					ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector
						, nLBA, 0, nMainDataType, nPadType, fpBin, fpSub, fpC2);
					nRetryCnt = 1;
					OutputLog(standardOut | fileMainError, "LBA[%06d, %#07x] Reread NG\n", nLBA, (UINT)nLBA);
				}
			}
			else if (pDiscPerSector->bReturnCode == RETURNED_FALSE) {
				if (*pExecType == gd && nRetryCnt <= 10 && nLBA > 65000) {
					OutputLog(standardError | fileMainError, "Retry %d/10\n", nRetryCnt);
					INT nTmpLBA = 0;
					for (nTmpLBA = nLBA - 20000; 449849 <= nTmpLBA; nTmpLBA -= 20000) {
						OutputString("Reread %d sector\n", nTmpLBA);
						if (RETURNED_FALSE == ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd, nTmpLBA,
							pDiscPerSector->data.current, _T(__FUNCTION__), __LINE__)) {
							if (nTmpLBA < 20000) {
								break;
							}
						}
						else {
							break;
						}
					}
					if (nStoreLBA == 0 && nRetryCnt == 1) {
						nStoreLBA = nLBA;
					}
					nLBA = nTmpLBA;
					nRetryCnt++;
					continue;
				}
				else {
					throw FALSE;
				}
			}
			if (*pExecType == gd && nRetryCnt > 1) {
				if (nStoreLBA == nLBA) {
					// init
					nStoreLBA = 0;
					nRetryCnt = 1;
					OutputString("\n");
				}
				else {
					OutputString("\rReread %d sector", nLBA);
					nLBA++;
					continue;
				}
			}
			if (pDiscPerSector->bReturnCode != RETURNED_CONTINUE &&
				pDiscPerSector->bReturnCode != RETURNED_SKIP_LBA) {
				if (nRetryCnt > 1) {
					OutputLog(standardOut | fileMainError, "LBA[%06d, %#07x] Reread OK\n", nLBA, (UINT)nLBA);
					nRetryCnt = 1;
				}
				if (pDisc->PROTECT.byExist == laserlock) {
					if (pExtArg->uiSkipSectors != 0) {
						bForceSkip = FALSE;
					}
					if (pExtArg->uiSkipSectors2 != 0) {
						bForceSkip2 = FALSE;
					}
				}
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData
					&& pExtArg->nC2RereadingType == 1 && n1stLBA - pDisc->MAIN.nOffsetStart >= 0) {
					GetCrc32(&pDisc->MAIN.lpAllSectorCrc32[n1stLBA - pDisc->MAIN.nOffsetStart]
						, pDiscPerSector->data.current, CD_RAW_SECTOR_SIZE);
				}
				if (pDisc->SUB.nSubChannelOffset) {
					if (!IsValidProtectedSector(pDisc, nLBA, GetReadErrorFileIdx(pExtArg, pDisc, nLBA))) {
						if (2 <= pExtArg->uiSubAddionalNum) {
							memcpy(pDiscPerSector->subcode.nextNext, pDiscPerSector->subcode.next, CD_RAW_READ_SUBCODE_SIZE);
						}
						if (1 <= pExtArg->uiSubAddionalNum) {
							memcpy(pDiscPerSector->subcode.next, pDiscPerSector->subcode.current, CD_RAW_READ_SUBCODE_SIZE);
						}
						memcpy(pDiscPerSector->subcode.current, lpPrevSubcode, CD_RAW_READ_SUBCODE_SIZE);
					}
				}
				SetTmpSubchFromBuffer(&pDiscPerSector->subch.current, pDiscPerSector->subcode.current);
				if (2 <= pExtArg->uiSubAddionalNum) {
					SetTmpSubchFromBuffer(&pDiscPerSector->subch.nextNext, pDiscPerSector->subcode.nextNext);
				}
				if (1 <= pExtArg->uiSubAddionalNum) {
					SetTmpSubchFromBuffer(&pDiscPerSector->subch.next, pDiscPerSector->subcode.next);
				}

				if (nStart <= nLBA && nLBA < nEnd) {
					BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = {};
					if (!pExtArg->byReverse) {
						if (IsCheckingSubChannel(pDisc, nLBA)) {
							BOOL bReturn = TRUE;
							if (!(*pExecType == audio || *pExecType == data)) {
//							if (nStart < nLBA) {
								bReturn = FixSubChannel(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, nLBA, &bReread);
							}
							if (bReread) {
								continue;
							}
							else if (!bReturn) {
								OutputErrorString(
									"\n" STR_LBA "Failed to reread because crc16 of subQ is 0\n"
									"1. If your disc has scratches, it needs to be polished\n"
									"2. Try to dump with different drive speed\n"
									"3. Try to dump with different drive\n"
									, nLBA, (UINT)nLBA);
								throw FALSE;
							}
							// fix raw subchannel
							AlignColumnSubcode(lpSubcodeRaw, pDiscPerSector->subcode.current);
#if 0
							OutputCDSub96Align(pDiscPerSector->subcode.current, nLBA);
#endif
							WriteSubChannel(pDisc, pDiscPerSector, lpSubcodeRaw, nLBA, fpSub);
							if (!(*pExecType == audio || *pExecType == data)) {
								SetTrackAttribution(pExecType, pExtArg, pDisc, pDiscPerSector, nLBA);
							}
							FixMainHeader(pExtArg, pDisc, pDiscPerSector, nLBA, nMainDataType);
						}
						if (!(*pExecType == audio || *pExecType == data)) {
							UpdateTmpSubch(pDiscPerSector);
						}
					}
				}
				// Write track to scrambled
				WriteMainChannel(pExecType, pExtArg, pDisc, pDiscPerSector->data.current, nLBA, fpBin);
#if 0
				OutputMainChannel(standardOut, pDiscPerSector->data.current, NULL, nLBA, 2352);
#endif
				WriteC2(pExtArg, pDevice, pDiscPerSector, fpC2);
				if (pDisc->SUB.nSubChannelOffset) {
					memcpy(lpPrevSubcode, pDiscPerSector->subcode.next, CD_RAW_READ_SUBCODE_SIZE);
				}
			}
			if (pExtArg->byReverse) {
				OutputString("\rCreating %s from %d to %d (LBA) %6d", szExt
					, nEnd + pDisc->MAIN.nOffsetEnd, nStart + pDisc->MAIN.nOffsetStart - 1, nLBA);
				nLBA--;
			}
			else {
				OutputString("\rCreating %s from %d to %d (LBA) %6d", szExt
					, nStart + pDisc->MAIN.nOffsetStart, nEnd + pDisc->MAIN.nOffsetEnd, nLBA);
				nLBA++;
			}
			n1stLBA++;
		}
		OutputString("\n");
		FcloseAndNull(fpSub);
		FlushLog();

		if (*pExecType == gd) {
			for (INT i = pDisc->SCSI.toc.FirstTrack - 1; i < pDisc->SCSI.toc.LastTrack; i++) {
				BOOL bErr = FALSE;
				LONG lLine = 0;
				if (pDisc->SUB.lp1stLBAListOnSub[i][1] == -1) {
					bErr = TRUE;
					lLine = __LINE__;
				}
				else if ((pDisc->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					if (pDisc->SUB.lp1stLBAListOfDataTrackOnSub[i] == -1) {
						bErr = TRUE;
						lLine = __LINE__;
					}
					else if (pDisc->SUB.lpLastLBAListOfDataTrackOnSub[i] == -1) {
						bErr = TRUE;
						lLine = __LINE__;
					}
				}
				if (bErr) {
					OutputErrorString(
						"[L:%ld] Internal error. Failed to analyze the subchannel. Track[%02d]/[%02u]\n",
						lLine, i + 1, pDisc->SCSI.toc.LastTrack);
					throw FALSE;
				}
			}
		}
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (bC2Error && pDisc->MAIN.nC2ErrorCnt > 0) {
				if (pExtArg->nC2RereadingType == 0) {
					if (!ReadCDForRereadingSectorType1(pExecType, pExtArg, pDevice
						, pDisc, pDiscPerSector, type, lpCmd, fpBin, fpC2, nStart)) {
						throw FALSE;
					}
				}
				else {
					INT nStartLBA = (INT)pExtArg->nStartLBAForC2;
					INT nEndLBA = (INT)pExtArg->nEndLBAForC2;
					if (nStartLBA == 0 && nEndLBA == 0) {
						nStartLBA = nStart;
						nEndLBA = nEnd;
					}
					if (!ReadCDForRereadingSectorType2(pExecType, pExtArg
						, pDevice, pDisc, type, lpCmd, fpBin, fpC2, nStartLBA, nEndLBA)) {
						throw FALSE;
					}
				}
			}
			else {
				if (pDisc->PROTECT.byExist == PROTECT_TYPE_CD::no) {
					OutputLog(standardOut | fileC2Error, "No C2 errors\n");
				}
				else {
					OutputLog(standardOut | fileC2Error, "No unintentional C2 errors\n");
				}
			}
		}
		FcloseAndNull(fpBin);
		if (*pExecType == gd) {
			OutputTocWithPregap(pDisc);
		}

		if (pExtArg->byReverse) {
			FILE* fpBin_r = NULL;
			if (NULL == (fpBin_r = CreateOrOpenFile(
				pszPath, _T("_reverse"), NULL, NULL, NULL, szExt, _T("rb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (fpBin = CreateOrOpenFile(
				pszPath, NULL, NULL, NULL, NULL, szExt, _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			OutputString("Reversing _reverse%s to %s\n", szExt, szExt);
			BYTE rBuf[CD_RAW_SECTOR_SIZE] = {};
			DWORD dwRoop = GetFileSize(0, fpBin_r) - CD_RAW_SECTOR_SIZE/* * 3*/;
			LONG lSeek = CD_RAW_SECTOR_SIZE - (LONG)pDisc->MAIN.uiMainDataSlideSize;
			fseek(fpBin_r, -lSeek, SEEK_END);

			if (fread(rBuf, sizeof(BYTE), (size_t)lSeek, fpBin_r) < (size_t)lSeek) {
				OutputErrorString("Failed to read [F:%s][L:%d]\n", _T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			fwrite(rBuf, sizeof(BYTE), (size_t)lSeek, fpBin);
			fseek(fpBin_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);

			for (DWORD i = 0; i < dwRoop; i += CD_RAW_SECTOR_SIZE) {
				fseek(fpBin_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
				if (fread(rBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpBin_r) < CD_RAW_SECTOR_SIZE) {
					OutputErrorString("Failed to read [F:%s][L:%d]\n", _T(__FUNCTION__), __LINE__);
					break;
				}
				fwrite(rBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpBin);
				fseek(fpBin_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
			}
			fseek(fpBin_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
			if (fread(rBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpBin_r) < pDisc->MAIN.uiMainDataSlideSize) {
				OutputErrorString("Failed to read [F:%s][L:%d]\n", _T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			fwrite(rBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpBin);

			FcloseAndNull(fpBin);
			FcloseAndNull(fpBin_r);
		}

		if (*pExecType == data) {
			if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
				if (NULL == (fpBin = CreateOrOpenFile(
					pszPath, NULL, NULL, NULL, NULL, _T(".bin"), _T("rb+"), 0, 0))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				DescrambleMainChannelPartial(nStart, nEnd - 1, scrambled_table, fpBin);
				FcloseAndNull(fpBin);
			}
			if (pExtArg->byScanProtectViaFile) {
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = nStart;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] = nEnd - nStart;
			}
			ExecEccEdc(pExtArg, pDisc->PROTECT, _T("TOC"), pszPath, pDisc->PROTECT.ERROR_SECTOR);
		}
		else if (*pExecType == gd) {
			_TCHAR pszImgPath[_MAX_PATH] = {};
			if (!DescrambleMainChannelForGD(pszPath, pszImgPath)) {
				throw FALSE;
			}
			ExecEccEdc(pExtArg, pDisc->PROTECT, _T("Sub"), pszImgPath, pDisc->PROTECT.ERROR_SECTOR);
			if (!CreateBinCueForGD(pDisc, pszPath)) {
				throw FALSE;
			}
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpBin);
	FcloseAndNull(fpSub);
	FreeAndNull(pBuf);
	if (1 <= pExtArg->uiSubAddionalNum) {
		FreeAndNull(pNextBuf);
		if (2 <= pExtArg->uiSubAddionalNum) {
			FreeAndNull(pNextNextBuf);
		}
	}
	return bRet;
}

#define POSITIVE -1
#define NEGATIVE 1
BOOL HasNonZeroByte(
	LPBYTE buf,
	UINT uiSize,
	INT nIdx,
	LPUINT lpByteOffset,
	BOOL bOrder
) {
	if (bOrder == POSITIVE) {
		for (UINT k = 0; k < uiSize; k++) {
			if (buf[k] != 0) {
				*lpByteOffset = k + CD_RAW_SECTOR_SIZE * nIdx;
				return TRUE;
			}
		}
	}
	else if (bOrder == NEGATIVE) {
		for (INT k = (INT)uiSize - 1; 0 <= k; k--) {
			if (buf[k] != 0) {
				*lpByteOffset = (UINT)CD_RAW_SECTOR_SIZE * nIdx - k;
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL ConvertScmToBin(
	LPCTSTR pszPath,
	LPCTSTR appendName0,
	INT nLine
) {
	BYTE aBuf[CD_RAW_SECTOR_SIZE] = {};
	FILE* fpBin = NULL;
	FILE* fpScm = NULL;
	if (NULL == (fpScm = CreateOrOpenFile(pszPath, appendName0, NULL, NULL, NULL, _T(".scm"), _T("rb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	try {
		DWORD fsize = GetFileSize(0, fpScm);

		if (NULL == (fpBin = CreateOrOpenFile(pszPath, appendName0, NULL, NULL, NULL, _T(".bin"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		size_t readsize = 0;
		for (DWORD i = 0; i < fsize; i += CD_RAW_SECTOR_SIZE) {
			readsize = fread(aBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpScm);
			if (readsize < CD_RAW_SECTOR_SIZE) {
				OutputLog(standardError | fileMainError, "Failed to read. readsize=%zu, total_read=%lu/%lu [F:%s][L:%d]\n"
					, readsize, i, fsize, _T(__FUNCTION__), nLine);
				throw FALSE;
			}
			for (INT n = 0; n < CD_RAW_SECTOR_SIZE; n++) {
				aBuf[n] ^= scrambled_table[n];
			}
			fwrite(aBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpBin);
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpScm);
	FcloseAndNull(fpBin);
	return bRet;
}

BOOL ReadCDOutOfRange(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszPath
) {
	BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
	_TCHAR ext[_MAX_EXT] = {};
	FILE* fpMain = NULL;
	FILE* fpSub = NULL;
	_TCHAR appendName0[32] = {};
	_TCHAR appendName1[32] = {};

	UINT uiByteOffsetOut = 0;
	BOOL bNonZeroByteExistOut = FALSE;
	BYTE byScsiStatus = 0;
	BYTE aBuf[CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 4] = {};
	BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = {};

	DISC_PER_SECTOR discPerSector = {};
	PDISC_PER_SECTOR pDiscPerSector = &discPerSector;
	BOOL bReread = FALSE;
	BYTE bakSkipSubP = pExtArg->bySkipSubP;
	pExtArg->bySkipSubP = TRUE;

	INT nOverreadSize = pDisc->MAIN.nCombinedOffset / CD_RAW_SECTOR_SIZE;
	if (pDisc->MAIN.nCombinedOffset < 0) {
		nOverreadSize = pDisc->MAIN.nAdjustSectorNum;
	}
	SetCDOffset(pExecType, pExtArg->byBe, pDevice->byPlxtrDrive, pDisc, 0, pDisc->SCSI.nAllLength);

	if (pDisc->SCSI.by1stMultiSessionTrkNum) {
		SetReadDiscCommand(pExtArg, pDevice, 2
			, CDFLAG::_READ_CD::All, CDFLAG::_READ_CD::NoC2, CDFLAG::_READ_CD::Pack, lpCmd, FALSE);
		BOOL bPregap = FALSE;
		if ((pDisc->SCSI.toc.TrackData[pDisc->SCSI.by1stMultiSessionTrkNum - 1].Control & AUDIO_DATA_TRACK) == 0) {
			_tcsncpy(ext, _T(".bin"), SIZE_OF_ARRAY(ext));
		}
		else {
			_tcsncpy(ext, _T(".scm"), SIZE_OF_ARRAY(ext));
		}
		// 2nd session lead-in
		if (pDisc->SCSI.toc.LastTrack > 9) {
			_tcsncpy(appendName0, _T(" (Track 00)(Session 2)"), SIZE_OF_ARRAY(appendName0));
			_sntprintf(appendName1, SIZE_OF_ARRAY(appendName1), _T(" (Track %02d)(Pregap)"), pDisc->SCSI.by1stMultiSessionTrkNum);
		}
		else {
			_tcsncpy(appendName0, _T(" (Track 0)(Session 2)"), SIZE_OF_ARRAY(appendName0));
			_sntprintf(appendName1, SIZE_OF_ARRAY(appendName1), _T(" (Track %d)(Pregap)"), pDisc->SCSI.by1stMultiSessionTrkNum);
		}

		BOOL bOK2ndSessionLeadin = TRUE;
		try {
			if (NULL == (fpMain = CreateOrOpenFile(pszPath, appendName0, NULL, NULL, NULL, ext, _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (fpSub = CreateOrOpenFile(pszPath, appendName0, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			INT n1stLeadinLBA = pDisc->SCSI.n1stLBAof2ndSession - 150;
			for (INT i = LEADIN_START_LBA_SESSION_2; i < -1150; i++) {
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, i, aBuf,
					CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 2, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					OutputLog(standardOut | fileDisc, "can't read\n");
					throw FALSE;
				}
				if (bPregap) {
					AlignRowSubcode(pDiscPerSector->subcode.current, &aBuf[CD_RAW_SECTOR_SIZE]);
					AlignRowSubcode(pDiscPerSector->subcode.next, &aBuf[CD_RAW_SECTOR_WITH_SUBCODE_SIZE + CD_RAW_SECTOR_SIZE]);

					FixSubChannel(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, ++n1stLeadinLBA, &bReread);

					memcpy(lpSubcode, pDiscPerSector->subcode.current, CD_RAW_READ_SUBCODE_SIZE);
//					memcpy(lpSubcodeNext, pDiscPerSector->subcode.next, CD_RAW_READ_SUBCODE_SIZE);
				}
				else {
					AlignRowSubcode(lpSubcode, &aBuf[CD_RAW_SECTOR_SIZE]);
				}
#if 0
				OutputCDSub96Align(standardOut, lpSubcode, i);
#endif
				BOOL bLastPregapSector = FALSE;
				INT adr = lpSubcode[12] & 0x0f;
				INT nTmpLBA = MSFtoLBA(BcdToDec(lpSubcode[19]), BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21]));
				if (BcdToDec(lpSubcode[13]) == pDisc->SCSI.by1stMultiSessionTrkNum &&
					(pDisc->SCSI.n1stLBAof2ndSession + 150 + nOverreadSize == nTmpLBA)
					) {
					bLastPregapSector = TRUE;
				}

				if (i == LEADIN_START_LBA_SESSION_2) {
					fwrite(aBuf + pDisc->MAIN.uiMainDataSlideSize, sizeof(BYTE), CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize, fpMain);
				}
				else if ((nOverreadSize >= 0 && pDisc->SCSI.n1stLBAof2ndSession + nOverreadSize == nTmpLBA) ||
					(nOverreadSize < 0 && pDisc->SCSI.n1stLBAof2ndSession + nOverreadSize == MSFtoLBA(BcdToDec(lpSubcode[15]), BcdToDec(lpSubcode[16]), BcdToDec(lpSubcode[17])))
					) {
					fwrite(aBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpMain);
					FcloseAndNull(fpMain);
					if (NULL == (fpMain = CreateOrOpenFile(pszPath, appendName1, NULL, NULL, NULL, ext, _T("wb"), 0, 0))) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
					UINT uiTmpSize = pDisc->MAIN.uiMainDataSlideSize;
					fwrite(aBuf + uiTmpSize, sizeof(BYTE), CD_RAW_SECTOR_SIZE - uiTmpSize, fpMain);
					bPregap = TRUE;
				}
				else if (bLastPregapSector) {
					fwrite(aBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpMain);
				}
				else {
					fwrite(aBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpMain);
				}

				if (pDisc->SCSI.n1stLBAof2ndSession == nTmpLBA) {
					FcloseAndNull(fpSub);
					if (NULL == (fpSub = CreateOrOpenFile(pszPath, appendName1, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
				}
				BOOL bWriteSub = FALSE;
				if (!bPregap || adr != ADR_ENCODES_CURRENT_POSITION) {
					bWriteSub = TRUE;
				}
				else {
					if (nOverreadSize >= 0 && pDisc->SCSI.n1stLBAof2ndSession + 150 + nOverreadSize > nTmpLBA) {
						bWriteSub = TRUE;
					}
					else if (nOverreadSize < 0 && pDisc->SCSI.n1stLBAof2ndSession + 150 > nTmpLBA) {
						bWriteSub = TRUE;
					}
				}
				if (bWriteSub) {
					fwrite(lpSubcode, sizeof(BYTE), CD_RAW_READ_SUBCODE_SIZE, fpSub);
				}
				OutputString("\rCreating%s%s & .sub,%s%s & .sub (LBA) %8d/%8d", appendName0, ext, appendName1, ext, i, LEADIN_START_LBA_SESSION_2);

				if (bLastPregapSector) {
					break;
				}
			}
			OutputString("\n");
		}
		catch (BOOL ret) {
			bOK2ndSessionLeadin = ret;
		}
		FcloseAndNull(fpMain);
		FcloseAndNull(fpSub);
		ZeroMemory(aBuf, CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 4);
		ZeroMemory(lpSubcode, CD_RAW_READ_SUBCODE_SIZE);

		if (!bPregap || !bOK2ndSessionLeadin) {
			pExtArg->bySkipSubP = bakSkipSubP;
			return FALSE;
		}

		if (!_tcsncmp(ext, _T(".scm"), SIZE_OF_ARRAY(ext))) {
			ConvertScmToBin(pszPath, appendName0, __LINE__);
			ConvertScmToBin(pszPath, appendName1, __LINE__);
		}

		// 2nd session lead-out
		SetReadDiscCommand(pExtArg, pDevice, 1
			, CDFLAG::_READ_CD::All, CDFLAG::_READ_CD::NoC2, CDFLAG::_READ_CD::Pack, lpCmd, FALSE);
		if ((pDisc->SCSI.toc.TrackData[pDisc->SCSI.toc.LastTrack - 1].Control & AUDIO_DATA_TRACK) == 0) {
			_tcsncpy(ext, _T(".bin"), SIZE_OF_ARRAY(ext));
		}
		else {
			_tcsncpy(ext, _T(".scm"), SIZE_OF_ARRAY(ext));
		}

		if (NULL == (fpMain = CreateOrOpenFile(pszPath, _T(" (Track AA)(Session 2)"), NULL, NULL, NULL, ext, _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (NULL == (fpSub = CreateOrOpenFile(pszPath, _T(" (Track AA)(Session 2)"), NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}

		for (INT i = pDisc->SCSI.nAllLength + nOverreadSize; i < pDisc->SCSI.nAllLength + LAST_TRACK_LEADOUT_SIZE; i++) {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, i, aBuf,
				CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				OutputLog(standardOut | fileDisc, "can't read\n");
				return FALSE;
			}
			AlignRowSubcode(lpSubcode, &aBuf[CD_RAW_SECTOR_SIZE]);

			if (i == pDisc->SCSI.nAllLength + nOverreadSize) {
				fwrite(aBuf + pDisc->MAIN.uiMainDataSlideSize, sizeof(BYTE), CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize, fpMain);
			}
			else if (i == pDisc->SCSI.nAllLength + LAST_TRACK_LEADOUT_SIZE - 1) {
				// 101th sector can't read by Plextor
				fwrite(aBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpMain);
			}
			else {
				fwrite(aBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpMain);
			}

			if (pDisc->SCSI.nAllLength <= i && i < pDisc->SCSI.nAllLength + LAST_TRACK_LEADOUT_SIZE - 1) {
				fwrite(lpSubcode, sizeof(BYTE), CD_RAW_READ_SUBCODE_SIZE, fpSub);
			}
			OutputString("\rCreating (Track AA)(Session 2)%s & .sub (LBA) %8d/%8d", ext, i, pDisc->SCSI.nAllLength + 99);
		}
		OutputString("\n");
		FcloseAndNull(fpMain);
		FcloseAndNull(fpSub);
		ZeroMemory(aBuf, CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 4);
		ZeroMemory(lpSubcode, CD_RAW_READ_SUBCODE_SIZE);

		if (!_tcsncmp(ext, _T(".scm"), SIZE_OF_ARRAY(ext))) {
			ConvertScmToBin(pszPath, _T(" (Track AA)(Session 2)"), __LINE__);
		}

		if ((pDisc->SCSI.toc.TrackData[pDisc->SCSI.toc.LastTrack - 1].Control & AUDIO_DATA_TRACK) == 0) {
			if (NULL == (fpMain = CreateOrOpenFile(pszPath, _T(" (Track AA)(Session 2)"), NULL, NULL, NULL, ext, _T("rb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			BYTE buf = 0;
			// 101th sector can't read by Plextor
			UINT fsize = CD_RAW_SECTOR_SIZE * LAST_TRACK_LEADOUT_SIZE - pDisc->MAIN.uiMainDataSlideSize;

			for (UINT i = 1; i < fsize; i++) {
				fseek(fpMain, (LONG)(fsize - i), SEEK_SET);
				if ((fread(&buf, sizeof(BYTE), sizeof(buf), fpMain)) < sizeof(buf)) {
					if (feof(fpMain)) {
						break;
					}
					if (ferror(fpMain)) {
						OutputErrorString("Failed to read: read size %zu [F:%s][L:%d]\n", sizeof(buf), __FUNCTION__, __LINE__);
						return FALSE;
					}
				}
				if (buf != 0) {
					bNonZeroByteExistOut = TRUE;
					uiByteOffsetOut = i;
					break;
				}
				OutputString("\rSearching last non-zero byte (size) %6u/%6u", fsize - i - 1, fsize);
			}
			OutputString("\n");

			OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("Check non-zero byte of the sector"));
			if (bNonZeroByteExistOut) {
				UINT ofs = fsize - uiByteOffsetOut + 1;
				UINT sample = (ofs + (ofs % 4)) / 4;
				OutputLog(standardOut | fileDisc, "\tThere is non-zero byte in the (Track AA)(Session 2): %u byte => %u sample\n", ofs, sample);
			}
			else {
				OutputLog(standardOut | fileDisc, "\tThere is not non-zero byte in the (Track AA)(Session 2)\n");
			}
			FcloseAndNull(fpMain);
		}
	}

	// 1st session lead-out
	_TCHAR appendNameA[23] = {};
	UINT uiLastTrk = 0;
	INT nStartLBA = 0;
	INT nLastLBA = 0;
	if (pDisc->SCSI.by1stMultiSessionTrkNum) {
		uiLastTrk = (UINT)(pDisc->SCSI.by1stMultiSessionTrkNum - 1);
		nStartLBA = pDisc->SCSI.n1stLBAofLeadout;
		nLastLBA = 6750;
		_tcsncpy(appendNameA, _T(" (Track AA)(Session 1)"), SIZE_OF_ARRAY(appendNameA));
	}
	else {
		uiLastTrk = pDisc->SCSI.toc.LastTrack;
		nStartLBA = pDisc->SCSI.nAllLength;
		nLastLBA = LAST_TRACK_LEADOUT_SIZE;
		_tcsncpy(appendNameA, _T(" (Track AA)"), SIZE_OF_ARRAY(appendNameA));
	}

	if ((pDisc->SCSI.toc.TrackData[uiLastTrk - 1].Control & AUDIO_DATA_TRACK) == 0) {
		_tcsncpy(ext, _T(".bin"), SIZE_OF_ARRAY(ext));
	}
	else {
		_tcsncpy(ext, _T(".scm"), SIZE_OF_ARRAY(ext));
	}

	if (NULL == (fpMain = CreateOrOpenFile(pszPath, appendNameA, NULL, NULL, NULL, ext, _T("wb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (NULL == (fpSub = CreateOrOpenFile(pszPath, appendNameA, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	SetReadDiscCommand(pExtArg, pDevice, 1
		, CDFLAG::_READ_CD::All, CDFLAG::_READ_CD::NoC2, CDFLAG::_READ_CD::Pack, lpCmd, FALSE);

	for (INT i = nStartLBA + nOverreadSize; i < nStartLBA + nLastLBA; i++) {
		if (!ExecReadCD(pExtArg, pDevice, lpCmd, i, aBuf,
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputLog(standardOut | fileDisc, "can't read\n");
			return FALSE;
		}
		AlignRowSubcode(lpSubcode, &aBuf[CD_RAW_SECTOR_SIZE]);

		if (i == nStartLBA + nOverreadSize) {
			fwrite(aBuf + pDisc->MAIN.uiMainDataSlideSize, sizeof(BYTE), CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize, fpMain);
		}
		else if (i == nStartLBA + nLastLBA - 1) {
			// 101th sector can't read by Plextor
			fwrite(aBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpMain);
		}
		else {
			fwrite(aBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpMain);
		}

		if (nStartLBA <= i && i < nStartLBA + nLastLBA - 1) {
			fwrite(lpSubcode, sizeof(BYTE), CD_RAW_READ_SUBCODE_SIZE, fpSub);
		}
		OutputString("\rCreating%s%s & .sub (LBA) %8d/%8d", appendNameA, ext, i, nStartLBA + nLastLBA - 1);
	}
	OutputString("\n");
	FcloseAndNull(fpMain);
	FcloseAndNull(fpSub);
	ZeroMemory(aBuf, CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 4);
	ZeroMemory(lpSubcode, CD_RAW_READ_SUBCODE_SIZE);

	if (!_tcsncmp(ext, _T(".scm"), SIZE_OF_ARRAY(ext))) {
		ConvertScmToBin(pszPath, appendNameA, __LINE__);
	}

	if ((pDisc->SCSI.toc.TrackData[pDisc->SCSI.toc.LastTrack - 1].Control & AUDIO_DATA_TRACK) == 0) {
		if (NULL == (fpMain = CreateOrOpenFile(pszPath, appendNameA, NULL, NULL, NULL, ext, _T("rb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		BYTE buf = 0;
		// 101th sector can't read by Plextor
		UINT fsize = CD_RAW_SECTOR_SIZE * LAST_TRACK_LEADOUT_SIZE - pDisc->MAIN.uiMainDataSlideSize;

		for (UINT i = 1; i < fsize; i++) {
			fseek(fpMain, (LONG)(fsize - i), SEEK_SET);
			if ((fread(&buf, sizeof(BYTE), sizeof(buf), fpMain)) < sizeof(buf)) {
				if (feof(fpMain)) {
					break;
				}
				if (ferror(fpMain)) {
					OutputErrorString("Failed to read: read size %zu [F:%s][L:%d]\n", sizeof(buf), __FUNCTION__, __LINE__);
					return FALSE;
				}
			}
			if (buf != 0) {
				bNonZeroByteExistOut = TRUE;
				uiByteOffsetOut = i;
				break;
			}
			OutputString("\rSearching last non-zero byte (size) %6u/%6u", fsize - i - 1, fsize);
		}
		OutputString("\n");

		OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("Check non-zero byte of the sector"));
		if (bNonZeroByteExistOut) {
			UINT ofs = fsize - uiByteOffsetOut + 1;
			UINT sample = (ofs + (ofs % 4)) / 4;
			OutputLog(standardOut | fileDisc, "\tThere is non-zero byte in the%s: %u byte => %u sample\n", appendNameA, ofs, sample);
		}
		else {
			OutputLog(standardOut | fileDisc, "\tThere is not non-zero byte in the%s\n", appendNameA);
		}
		FcloseAndNull(fpMain);
	}

	// 1st session lead-in
	if ((pDisc->SCSI.toc.TrackData[0].Control & AUDIO_DATA_TRACK) == 0) {
		_tcsncpy(ext, _T(".bin"), SIZE_OF_ARRAY(ext));
	}
	else {
		_tcsncpy(ext, _T(".scm"), SIZE_OF_ARRAY(ext));
	}

	if (pDisc->SCSI.toc.LastTrack > 9) {
		if (pDisc->SCSI.by1stMultiSessionTrkNum) {
			_tcsncpy(appendName0, _T(" (Track 00)(Session 1)"), SIZE_OF_ARRAY(appendName0));
		}
		else {
			_tcsncpy(appendName0, _T(" (Track 00)"), SIZE_OF_ARRAY(appendName0));
		}
		_tcsncpy(appendName1, _T(" (Track 01)(-LBA)"), SIZE_OF_ARRAY(appendName1));
	}
	else {
		if (pDisc->SCSI.by1stMultiSessionTrkNum) {
			_tcsncpy(appendName0, _T(" (Track 0)(Session 1)"), SIZE_OF_ARRAY(appendName0));
		}
		else {
			_tcsncpy(appendName0, _T(" (Track 0)"), SIZE_OF_ARRAY(appendName0));
		}
		_tcsncpy(appendName1, _T(" (Track 1)(-LBA)"), SIZE_OF_ARRAY(appendName1));
	}

	if (NULL == (fpMain = CreateOrOpenFile(pszPath, appendName0, NULL, NULL, NULL, ext, _T("wb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (NULL == (fpSub = CreateOrOpenFile(pszPath, appendName0, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	UINT uiByteOffsetIn = 0;
	BOOL bNonZeroByteExistIn = FALSE;
	BOOL b1stPregapSector = FALSE;
	BOOL bTrack1 = FALSE;
	INT n1stLeadinLBA = -150;
	BYTE aBufBak[CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 4] = {};
	BYTE lpSubcodeNext[CD_RAW_READ_SUBCODE_SIZE] = {};
	SetReadDiscCommand(pExtArg, pDevice, 4
		, CDFLAG::_READ_CD::All, CDFLAG::_READ_CD::NoC2, CDFLAG::_READ_CD::Pack, lpCmd, FALSE);

	for (INT i = LEADIN_START_LBA_SESSION_1; i < -1153; i++) {
		if (!ExecReadCD(pExtArg, pDevice, lpCmd, i, aBuf,
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 4, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputLog(standardOut | fileDisc, "can't read\n");
			return FALSE;
		}
		if (b1stPregapSector) {
			AlignRowSubcode(pDiscPerSector->subcode.current, &aBuf[CD_RAW_SECTOR_SIZE]);
			AlignRowSubcode(pDiscPerSector->subcode.next, &aBuf[CD_RAW_SECTOR_WITH_SUBCODE_SIZE + CD_RAW_SECTOR_SIZE]);

			FixSubChannel(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, ++n1stLeadinLBA, &bReread);
	
			memcpy(lpSubcode, pDiscPerSector->subcode.current, CD_RAW_READ_SUBCODE_SIZE);
			memcpy(lpSubcodeNext, pDiscPerSector->subcode.next, CD_RAW_READ_SUBCODE_SIZE);
		}
		else {
			AlignRowSubcode(lpSubcode, &aBuf[CD_RAW_SECTOR_SIZE]);
			AlignRowSubcode(lpSubcodeNext, &aBuf[CD_RAW_SECTOR_WITH_SUBCODE_SIZE + CD_RAW_SECTOR_SIZE]);
		}
#if 0
		OutputCDSub96Align(standardOut, lpSubcode, i);
#endif
		BOOL bLastPregapSector = FALSE;
		if (lpSubcode[13] == 1 && (lpSubcode[14] == 0 || lpSubcode[14] == 1)) {
			if (lpSubcode[19] == 0 && lpSubcode[20] == 0 && lpSubcode[21] == 0) {
				b1stPregapSector = TRUE;
			}
			else if ((lpSubcode[19] == 0 && lpSubcode[20] == 1 && BcdToDec(lpSubcode[21]) == 71 && nOverreadSize == -4) ||
				(lpSubcode[19] == 0 && lpSubcode[20] == 1 && BcdToDec(lpSubcode[21]) == 72 && nOverreadSize == -3) ||
				(lpSubcode[19] == 0 && lpSubcode[20] == 1 && BcdToDec(lpSubcode[21]) == 73 && nOverreadSize == -2) ||
				(lpSubcode[19] == 0 && lpSubcode[20] == 1 && BcdToDec(lpSubcode[21]) == 74 && nOverreadSize == -1) ||
				(lpSubcode[19] == 0 && lpSubcode[20] == 2 && lpSubcode[21] == 0 && nOverreadSize == 0) ||
				(lpSubcode[19] == 0 && lpSubcode[20] == 2 && lpSubcode[21] == 1 && nOverreadSize == 1) ||
				(lpSubcode[19] == 0 && lpSubcode[20] == 2 && lpSubcode[21] == 2 && nOverreadSize == 2) ||
				(lpSubcode[19] == 0 && lpSubcode[20] == 2 && lpSubcode[21] == 3 && nOverreadSize == 3)
				) {
				bLastPregapSector = TRUE;
			}
		}

		if (((lpSubcode[12] & 0x0f) == ADR_ENCODES_CURRENT_POSITION && lpSubcode[13] == 1 &&
				lpSubcode[14] == 0 && lpSubcode[19] == 0 && lpSubcode[20] == 0 && lpSubcode[21] == 0) ||
			(((lpSubcode[12] & 0x0f) == ADR_ENCODES_MEDIA_CATALOG && lpSubcode[21] == 0) &&
				(lpSubcodeNext[12] & 0x0f) == ADR_ENCODES_CURRENT_POSITION && lpSubcodeNext[13] == 1 &&
				lpSubcodeNext[14] == 0 && lpSubcodeNext[19] == 0 && lpSubcodeNext[20] == 0 && lpSubcodeNext[21] == 1)) {
			if (nOverreadSize >= 0) {
				fwrite(aBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpMain);
				if (nOverreadSize >= 1) {
					fwrite(aBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpMain);
					if (nOverreadSize >= 2) {
						fwrite(aBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 2, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpMain);
						if (nOverreadSize >= 3) {
							fwrite(aBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 3, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpMain);
						}
					}
				}
			}
			FcloseAndNull(fpMain);
			bTrack1 = TRUE;

			if (NULL == (fpMain = CreateOrOpenFile(pszPath, appendName1, NULL, NULL, NULL, ext, _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			LPBYTE addr = aBufBak + pDisc->MAIN.uiMainDataSlideSize;
			LPBYTE addr2 = aBufBak + CD_RAW_SECTOR_WITH_SUBCODE_SIZE + pDisc->MAIN.uiMainDataSlideSize;
			LPBYTE addr3 = aBufBak + CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 2 + pDisc->MAIN.uiMainDataSlideSize;
			LPBYTE addr4 = aBufBak + CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 3 + pDisc->MAIN.uiMainDataSlideSize;
			size_t size = CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize;
			if (nOverreadSize == -4) {
				fwrite(addr, sizeof(BYTE), size, fpMain);
				fwrite(addr2, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpMain);
				fwrite(addr3, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpMain);
				fwrite(addr4, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpMain);
				fwrite(aBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpMain);
				if ((pDisc->SCSI.toc.TrackData[0].Control & AUDIO_DATA_TRACK) == 0) {
					bNonZeroByteExistIn = HasNonZeroByte(aBuf, CD_RAW_SECTOR_SIZE, 0, &uiByteOffsetIn, POSITIVE);
					if (!bNonZeroByteExistIn) {
						bNonZeroByteExistIn = HasNonZeroByte(addr4, CD_RAW_SECTOR_SIZE, 0, &uiByteOffsetIn, POSITIVE);
						if (!bNonZeroByteExistIn) {
							bNonZeroByteExistIn = HasNonZeroByte(addr3, CD_RAW_SECTOR_SIZE, 0, &uiByteOffsetIn, POSITIVE);
							if (!bNonZeroByteExistIn) {
								bNonZeroByteExistIn = HasNonZeroByte(addr2, CD_RAW_SECTOR_SIZE, 0, &uiByteOffsetIn, POSITIVE);
								if (!bNonZeroByteExistIn) {
									bNonZeroByteExistIn = HasNonZeroByte(addr, (UINT)size, 0, &uiByteOffsetIn, POSITIVE);
								}
							}
						}
					}
				}
			}
			else if (nOverreadSize == -3) {
				fwrite(addr2, sizeof(BYTE), size, fpMain);
				fwrite(addr3, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpMain);
				fwrite(addr4, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpMain);
				fwrite(aBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpMain);
				if ((pDisc->SCSI.toc.TrackData[0].Control & AUDIO_DATA_TRACK) == 0) {
					bNonZeroByteExistIn = HasNonZeroByte(aBuf, CD_RAW_SECTOR_SIZE, 0, &uiByteOffsetIn, POSITIVE);
					if (!bNonZeroByteExistIn) {
						bNonZeroByteExistIn = HasNonZeroByte(addr4, CD_RAW_SECTOR_SIZE, 0, &uiByteOffsetIn, POSITIVE);
						if (!bNonZeroByteExistIn) {
							bNonZeroByteExistIn = HasNonZeroByte(addr3, CD_RAW_SECTOR_SIZE, 0, &uiByteOffsetIn, POSITIVE);
							if (!bNonZeroByteExistIn) {
								bNonZeroByteExistIn = HasNonZeroByte(addr2, (UINT)size, 0, &uiByteOffsetIn, POSITIVE);
							}
						}
					}
				}
			}
			else if (nOverreadSize == -2) {
				fwrite(addr3, sizeof(BYTE), size, fpMain);
				fwrite(addr4, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpMain);
				fwrite(aBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpMain);
				if ((pDisc->SCSI.toc.TrackData[0].Control & AUDIO_DATA_TRACK) == 0) {
					bNonZeroByteExistIn = HasNonZeroByte(aBuf, CD_RAW_SECTOR_SIZE, 0, &uiByteOffsetIn, POSITIVE);
					if (!bNonZeroByteExistIn) {
						bNonZeroByteExistIn = HasNonZeroByte(addr4, CD_RAW_SECTOR_SIZE, 0, &uiByteOffsetIn, POSITIVE);
						if (!bNonZeroByteExistIn) {
							bNonZeroByteExistIn = HasNonZeroByte(addr3, (UINT)size, 0, &uiByteOffsetIn, POSITIVE);
						}
					}
				}
			}
			else if (nOverreadSize == -1) {
				fwrite(addr4, sizeof(BYTE), size, fpMain);
				fwrite(aBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpMain);
				if ((pDisc->SCSI.toc.TrackData[0].Control & AUDIO_DATA_TRACK) == 0) {
					bNonZeroByteExistIn = HasNonZeroByte(addr4, (UINT)size, 0, &uiByteOffsetIn, POSITIVE);
					if (!bNonZeroByteExistIn) {
						bNonZeroByteExistIn = HasNonZeroByte(aBuf, CD_RAW_SECTOR_SIZE, 0, &uiByteOffsetIn, POSITIVE);
					}
				}
			}
			else {
				fwrite(aBuf + pDisc->MAIN.uiMainDataSlideSize, sizeof(BYTE), size, fpMain);
				if ((pDisc->SCSI.toc.TrackData[0].Control & AUDIO_DATA_TRACK) == 0) {
					bNonZeroByteExistIn = HasNonZeroByte(aBuf + pDisc->MAIN.uiMainDataSlideSize, (UINT)size, 0, &uiByteOffsetIn, POSITIVE);
				}
			}
		}
		else if (i == LEADIN_START_LBA_SESSION_1 && nOverreadSize >= 0) {
			fwrite(aBuf + pDisc->MAIN.uiMainDataSlideSize, sizeof(BYTE), CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize, fpMain);
		}
		else if (bLastPregapSector) {
			fwrite(aBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpMain);
			if (!bNonZeroByteExistIn && (pDisc->SCSI.toc.TrackData[0].Control & AUDIO_DATA_TRACK) == 0) {
				bNonZeroByteExistIn = HasNonZeroByte(aBuf, pDisc->MAIN.uiMainDataSlideSize, 150, &uiByteOffsetIn, POSITIVE);
			}
		}
		else {
			fwrite(aBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpMain);
			if (bTrack1 && !bNonZeroByteExistIn && (pDisc->SCSI.toc.TrackData[0].Control & AUDIO_DATA_TRACK) == 0) {
				INT nIdx = MSFtoLBA(BcdToDec(lpSubcode[19]), BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21]));
				bNonZeroByteExistIn = HasNonZeroByte(aBuf, CD_RAW_SECTOR_SIZE, nIdx, &uiByteOffsetIn, POSITIVE);
			}
		}

		if (((lpSubcode[12] & 0x0f) == ADR_ENCODES_CURRENT_POSITION && lpSubcode[13] == 1 &&
				lpSubcode[14] == 0 && lpSubcode[19] == 0 && lpSubcode[20] == 0 && lpSubcode[21] == 0) ||
			(((lpSubcode[12] & 0x0f) == ADR_ENCODES_MEDIA_CATALOG && lpSubcode[21] == 0) &&
				(lpSubcodeNext[12] & 0x0f) == ADR_ENCODES_CURRENT_POSITION && lpSubcodeNext[13] == 1 &&
				lpSubcodeNext[14] == 0 && lpSubcodeNext[19] == 0 && lpSubcodeNext[20] == 0 && lpSubcodeNext[21] == 1)) {
			FcloseAndNull(fpSub);
			if (NULL == (fpSub = CreateOrOpenFile(pszPath, appendName1, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
		}
		OutputString("\rCreating%s%s & .sub,%s%s & .sub (LBA) %8d/%8d", appendName0, ext, appendName1, ext, i, LEADIN_START_LBA_SESSION_1);

		if (bLastPregapSector) {
			if (nOverreadSize < 0) {
				fwrite(lpSubcode, sizeof(BYTE), CD_RAW_READ_SUBCODE_SIZE, fpSub);
			}
			break;
		}
		else {
			fwrite(lpSubcode, sizeof(BYTE), CD_RAW_READ_SUBCODE_SIZE, fpSub);
			memcpy(aBufBak, aBuf, CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 4);
		}
	}
	OutputString("\n");
	FcloseAndNull(fpMain);
	FcloseAndNull(fpSub);
	ZeroMemory(aBuf, CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 4);

	if (!_tcsncmp(ext, _T(".scm"), SIZE_OF_ARRAY(ext))) {
		ConvertScmToBin(pszPath, appendName0, __LINE__);
		ConvertScmToBin(pszPath, appendName1, __LINE__);
	}

	if (!bTrack1) {
		OutputErrorString("Failed to get%s\n", appendName1);
		return FALSE;
	}

	if ((pDisc->SCSI.toc.TrackData[0].Control & AUDIO_DATA_TRACK) == 0) {
		if (bNonZeroByteExistIn) {
			INT ofs = (INT)(uiByteOffsetIn - pDisc->MAIN.uiMainDataSlideSize) - 352800;
			INT sample = (ofs + (ofs % 4)) / 4;
			OutputLog(standardOut | fileDisc, "\tThere is non-zero byte in the%s: %d byte => %d sample\n", appendName1, ofs, sample);
		}
		else {
			OutputLog(standardOut | fileDisc, "\tThere is not non-zero byte in the%s\n", appendName1);
		}
	}

	if (pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
		SetReadDiscCommand(pExtArg, pDevice, 1
			, CDFLAG::_READ_CD::All, CDFLAG::_READ_CD::NoC2, CDFLAG::_READ_CD::Pack, lpCmd, FALSE);
		if (bNonZeroByteExistIn && !bNonZeroByteExistOut) {
			INT nTmpLBA = pDisc->SCSI.nAllLength;
			for (INT i = nTmpLBA, j = 0; nTmpLBA - FIRST_TRACK_PREGAP_SIZE < i; i--, j++) {
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, i, aBuf,
					CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					OutputLog(standardOut | fileDisc, "can't read\n");
					return FALSE;
				}
				if (i == pDisc->SCSI.nAllLength) {
					bNonZeroByteExistOut = HasNonZeroByte(aBuf, pDisc->MAIN.uiMainDataSlideSize, j, &uiByteOffsetOut, NEGATIVE);
				}
				else {
					bNonZeroByteExistOut = HasNonZeroByte(aBuf, CD_RAW_SECTOR_SIZE, j, &uiByteOffsetOut, NEGATIVE);
				}
				if (bNonZeroByteExistOut) {
					INT ofs = (INT)(uiByteOffsetOut + pDisc->MAIN.uiMainDataSlideSize - 1) * -1;
					INT sample = (ofs + (ofs % 4)) / 4;
					OutputLog(standardOut | fileDisc, "\tLast non-zero byte before (Track AA): %d byte => %d sample\n", ofs, sample);
					break;
				}
			}
		}
		if (!bNonZeroByteExistIn && bNonZeroByteExistOut) {
			for (INT i = 0, j = 0; i < LAST_TRACK_LEADOUT_SIZE; i++, j++) {
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, i, aBuf,
					CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					OutputLog(standardOut | fileDisc, "can't read\n");
					return FALSE;
				}
				if (i == 0) {
					bNonZeroByteExistIn = HasNonZeroByte(aBuf + pDisc->MAIN.uiMainDataSlideSize
						, CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize, j, &uiByteOffsetOut, POSITIVE);
				}
				else {
					bNonZeroByteExistIn = HasNonZeroByte(aBuf, CD_RAW_SECTOR_SIZE, j, &uiByteOffsetOut, POSITIVE);
				}
				if (bNonZeroByteExistIn) {
					INT ofs = (INT)(uiByteOffsetOut - pDisc->MAIN.uiMainDataSlideSize);
					INT sample = (ofs - (ofs % 4)) / 4;
					OutputLog(standardOut | fileDisc, "\t1st non-zero byte after%s: %d byte => %d sample\n", appendName1, ofs, sample);
					break;
				}
			}
		}
	}
	pExtArg->bySkipSubP = bakSkipSubP;
	return TRUE;
}
