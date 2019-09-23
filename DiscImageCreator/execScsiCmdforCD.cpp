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
			memcpy(lpBuf + DISC_RAW_READ_SIZE * i, bufDec + CD_RAW_SECTOR_SIZE * i + 16, DISC_RAW_READ_SIZE);
//			OutputCDMain(fileMainInfo, lpBuf + DISC_RAW_READ_SIZE * i, nLBA, DISC_RAW_READ_SIZE);
		}
	}
	else {
		if (!ExecReadCD(pExtArg, pDevice, pCdb, nLBA, lpBuf,
			(DWORD)(DISC_RAW_READ_SIZE * byTransferLen), pszFuncName, lLineNum)) {
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
	REVERSE_BYTES(&lpCmd[2], &nLBA);
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB12GENERIC_LENGTH
		, lpBuf, direction, dwBufSize, &byScsiStatus, pszFuncName, lLineNum)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputLogA(standardError | fileMainError,
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
			OutputErrorString(_T("Retry %d/10 after %d milliseconds\n"), n, milliseconds);
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
	OutputCDMain(fileMainInfo, lpInBuf + idx, nLBA, CD_RAW_SECTOR_SIZE);
#endif
	for (BYTE i = 0; i < byTransferLen; i++) {
		for (INT j = 0; j < CD_RAW_SECTOR_SIZE; j++) {
			lpOutBuf[j + CD_RAW_SECTOR_SIZE * i] =
				(BYTE)(lpInBuf[(nOfs + j) + CD_RAW_SECTOR_SIZE * i] ^ scrambled_table[j]);
		}
	}
#if 0
	OutputCDMain(fileMainInfo, lpOutBuf, nLBA, CD_RAW_SECTOR_SIZE);
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
	REVERSE_BYTES(&lpCmd[2], &nLBA);
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
		direction, pDevice->TRANSFER.uiBufLen * byTransferLen, &byScsiStatus, pszFuncName, lLineNum)) {
		if (pExtArg->byScanProtectViaFile ||
			pExtArg->byMultiSession/* && pDisc->MAIN.nFixFirstLBAofLeadout <= nLBA && nLBA < pDisc->MAIN.nFixFirstLBAofLeadout + 11400*/) {
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
		INT NextLBAAddress = nLBA + 1;
		REVERSE_BYTES(&cdb.LogicalBlock, &NextLBAAddress);
#ifdef _WIN32
		INT direction = SCSI_IOCTL_DATA_IN;
#else
		INT direction = SG_DXFER_FROM_DEV;
#endif
		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pExtArg, pDevice, (LPBYTE)&cdb, CDB12GENERIC_LENGTH,
			NULL, direction, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__)
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
	if (*pExecType != gd && !pExtArg->byMultiSession && pDevice->bySuccessReadTocFull) {
		if (pDisc->SCSI.nFirstLBAof2ndSession != -1) {
			if (pExtArg->byReverse) {
				if (pDisc->SCSI.nFirstLBAof2ndSession == nLBA + 1) {
					OutputMainInfoLogA(
						"Skip from Leadout of Session 1 [%d, %#x] to Leadin of Session 2 [%d, %#x]\n",
						pDisc->SCSI.nFirstLBAofLeadout, pDisc->SCSI.nFirstLBAofLeadout,
						pDisc->SCSI.nFirstLBAof2ndSession - 1, pDisc->SCSI.nFirstLBAof2ndSession - 1);
					pDiscPerSector->subQ.prev.nAbsoluteTime = nLBA - SESSION_TO_SESSION_SKIP_LBA - 150;
					return RETURNED_SKIP_LBA;
				}
			}
			else {
				if (pDisc->MAIN.nFixFirstLBAofLeadout == nLBA) {
					OutputMainInfoLogA(
						"Skip from Leadout of Session 1 [%d, %#x] to Leadin of Session 2 [%d, %#x]\n",
						pDisc->SCSI.nFirstLBAofLeadout, pDisc->SCSI.nFirstLBAofLeadout,
						pDisc->SCSI.nFirstLBAof2ndSession - 1, pDisc->SCSI.nFirstLBAof2ndSession - 1);
					if (pDisc->MAIN.nCombinedOffset > 0) {
						pDiscPerSector->subQ.prev.nAbsoluteTime =
							nLBA + SESSION_TO_SESSION_SKIP_LBA + 150 - pDisc->MAIN.nAdjustSectorNum - 1;
					}
					else if (pDisc->MAIN.nCombinedOffset < 0) {
						pDiscPerSector->subQ.prev.nAbsoluteTime =
							nLBA + SESSION_TO_SESSION_SKIP_LBA + 150 + pDisc->MAIN.nAdjustSectorNum;
					}
					return RETURNED_SKIP_LBA;
				}
			}
		}
	}
	else if (pExtArg->byMultiSession) {
		if (pDisc->MAIN.nFixFirstLBAofLeadout + LEADOUT_SIZE_OF_MULTISESSION + 4 == nLBA) {
			return RETURNED_CONTINUE;
		}
	}

	if (pExtArg->byFua || pDisc->SUB.nCorruptCrcH == 1 || pDisc->SUB.nCorruptCrcL == 1) {
		if (!IsValidProtectedSector(pDisc, nLBA) &&
			!IsValidIntentionalC2error(pDisc, pDiscPerSector) &&
			!pDiscPerSector->bLibCrypt && !pDiscPerSector->bSecuRom) {
			FlushDriveCache(pExtArg, pDevice, nLBA);
//			SynchronizeCache(pExtArg, pDevice);
			pDisc->SUB.nCorruptCrcH = 0;
			pDisc->SUB.nCorruptCrcL = 0;
		}
	}

	memcpy(pDiscPerSector->subcode.prev, pDiscPerSector->subcode.current, CD_RAW_READ_SUBCODE_SIZE);
	bRet = ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd, nLBA,
		pDiscPerSector->data.current, _T(__FUNCTION__), __LINE__);

	if (pDevice->byPlxtrDrive) {
		if (bRet == RETURNED_NO_C2_ERROR_1ST) {
			AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
			if (pDiscPerSector->data.next != NULL && 1 <= pExtArg->uiSubAddionalNum) {
				memcpy(pDiscPerSector->data.next, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufLen, pDevice->TRANSFER.uiBufLen);
				AlignRowSubcode(pDiscPerSector->subcode.next, pDiscPerSector->data.next + pDevice->TRANSFER.uiBufSubOffset);
					
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					bRet = ContainsC2Error(pDevice, pDiscPerSector->data.next, &pDiscPerSector->uiC2errorNum, TRUE);
				}
				if (pDiscPerSector->data.nextNext != NULL && 2 <= pExtArg->uiSubAddionalNum) {
					ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd,
						nLBA + 2, pDiscPerSector->data.nextNext, _T(__FUNCTION__), __LINE__);
					AlignRowSubcode(pDiscPerSector->subcode.nextNext, pDiscPerSector->data.nextNext + pDevice->TRANSFER.uiBufSubOffset);
				}
			}
		}
	}
	else {
		if (bRet == RETURNED_NO_C2_ERROR_1ST) {
			AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
			if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
				bRet = ContainsC2Error(pDevice, pDiscPerSector->data.current, &pDiscPerSector->uiC2errorNum, TRUE);
			}
			if (!IsValidProtectedSector(pDisc, nLBA)) {
				if (pDiscPerSector->data.next != NULL && 1 <= pExtArg->uiSubAddionalNum) {
					if (!(pDevice->byAsusDrive && pDisc->SCSI.nAllLength - 1 <= nLBA)) {
						ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd,
							nLBA + 1, pDiscPerSector->data.next, _T(__FUNCTION__), __LINE__);
						AlignRowSubcode(pDiscPerSector->subcode.next, pDiscPerSector->data.next + pDevice->TRANSFER.uiBufSubOffset);

						if (pDiscPerSector->data.nextNext != NULL && 2 <= pExtArg->uiSubAddionalNum) {
							ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd,
								nLBA + 2, pDiscPerSector->data.nextNext, _T(__FUNCTION__), __LINE__);
							AlignRowSubcode(pDiscPerSector->subcode.nextNext, pDiscPerSector->data.nextNext + pDevice->TRANSFER.uiBufSubOffset);
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
	LPBYTE lpCmd,
	FILE* fpImg,
	FILE* fpC2,
	INT nStart
) {
	LPBYTE lpBuf = NULL;
	if (NULL == (lpBuf = (LPBYTE)calloc(CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * 2, sizeof(BYTE)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	try {
		SetReadDiscCommand(pExecType, pExtArg, pDevice, 2
			, CDFLAG::_READ_CD::byte294, CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION::Raw, lpCmd, FALSE);

		for (INT m = 0; m < pDisc->MAIN.nC2ErrorCnt; m++) {
			INT nLBA = pDisc->MAIN.lpAllLBAOfC2Error[m];
			for (UINT i = 0; i < pExtArg->uiMaxRereadNum; i++) {
				OutputString(_T("\rNeed to reread sector: %6d rereading times: %4d/%4d")
					, nLBA, i + 1, pExtArg->uiMaxRereadNum);
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * 2, _T(__FUNCTION__), __LINE__)) {
					continue;
				}
				DWORD dwTmpCrc32 = 0;
				GetCrc32(&dwTmpCrc32, lpBuf, CD_RAW_SECTOR_SIZE);
//				OutputC2ErrorWithLBALogA("to [%06d] crc32[%03ld]: 0x%08lx "
//					, nLBA - pDisc->MAIN.nOffsetStart - 1, nLBA - pDisc->MAIN.nOffsetStart, i, dwTmpCrc32);
				OutputC2ErrorWithLBALogA("crc32[%03d]: 0x%08lx ", nLBA, i, dwTmpCrc32);

				LPBYTE lpNextBuf = lpBuf + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
				if (ContainsC2Error(pDevice, lpNextBuf, &pDiscPerSector->uiC2errorNum, FALSE) == RETURNED_NO_C2_ERROR_1ST) {
					LONG lSeekMain = CD_RAW_SECTOR_SIZE * (LONG)nLBA - nStart - pDisc->MAIN.nCombinedOffset;
					fseek(fpImg, lSeekMain, SEEK_SET);
					// Write track to scrambled again
					WriteMainChannel(pExecType, pExtArg, pDisc, lpBuf, nLBA, fpImg);

					LONG lSeekC2 = CD_RAW_READ_C2_294_SIZE * (LONG)nLBA - nStart - (pDisc->MAIN.nCombinedOffset / 8);
					fseek(fpC2, lSeekC2, SEEK_SET);
					WriteC2(pExtArg, pDisc, lpNextBuf + pDevice->TRANSFER.uiBufC2Offset, nLBA, fpC2);
					OutputC2ErrorLogA("good. Rewrote .scm[%ld-%ld(%lx-%lx)] .c2[%ld-%ld(%lx-%lx)]\n"
						, lSeekMain, lSeekMain + 2351, lSeekMain, lSeekMain + 2351
						, lSeekC2, lSeekC2 + 293, lSeekC2, lSeekC2 + 293);
					break;
				}
				else {
					if (i == pExtArg->uiMaxRereadNum - 1 && !pDisc->PROTECT.byExist) {
						OutputLogA(standardError | fileC2Error, "\nbad all. need to reread more\n");
						if (IsCDRDrive(pDisc) && pExtArg->uiMaxRereadNum >= 10000) {
							throw TRUE;
						}
						else {
							throw FALSE;
						}
					}
					else {
						OutputC2ErrorLogA("bad\n");
					}
				}
				if (!FlushDriveCache(pExtArg, pDevice, nLBA)) {
					throw FALSE;
				}
			}
		}
		OutputString(_T("\nDone. See _c2Error.txt\n"));
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(lpBuf);
	return bRet;
}

BOOL ReadCDForRereadingSectorType2(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
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
		if (NULL == (lpRereadSector = (LPBYTE*)calloc(dwTransferLen, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpCrc32RereadSector = (LPDWORD*)calloc(dwTransferLen, sizeof(DWORD_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpRepeatedNum = (LPUINT*)calloc(dwTransferLen, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpContainsC2 = (LPUINT*)calloc(dwTransferLen, sizeof(UINT_PTR)))) {
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
		SetReadDiscCommand(pExecType, pExtArg, pDevice, (BYTE)dwTransferLen
			, CDFLAG::_READ_CD::byte294, CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION::Raw, lpCmd, FALSE);

		INT nLBA = nStartLBA;
		INT nLastLBA = nEndLBA;

		while (nLBA < nLastLBA) {
			OutputString(_T("\rRewrited img (LBA) %6d/%6d"), nLBA, pDisc->SCSI.nAllLength - 1);
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
					OutputC2ErrorWithLBALogA("crc32", nLBA - pDisc->MAIN.nOffsetStart + (INT)k);
					DWORD dwTmpCrc32 = 0;
					GetCrc32(&dwTmpCrc32, lpBufMain + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * k, CD_RAW_SECTOR_SIZE);
#ifdef __DEBUG
					OutputCDMain(fileC2Error, lpBufMain + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * k, nLBA, CD_RAW_SECTOR_SIZE);
					OutputCDC2Error296(fileC2Error, lpBufMain + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * k + CD_RAW_SECTOR_SIZE, 294);
					OutputCDMain(fileC2Error, lpBufC2 + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * k, nLBA, CD_RAW_SECTOR_SIZE);
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
					for (UINT j = 0; j <= i; j++) {
						if (dwTmpCrc32 == lpCrc32RereadSector[k][j]) {
							OutputC2ErrorLogA("[%03d]:0x%08lx, %d ", i, dwTmpCrc32, bC2);
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
						OutputC2ErrorLogA("[%03d]:0x%08lx, %d ", idx, dwTmpCrc32, bC2);
#if 0
						if (idx > 0) {
							OutputCDMain(fileC2Error, &lpRereadSector[k][CD_RAW_SECTOR_SIZE * idx]
								, nLBA - pDisc->MAIN.nOffsetStart + (INT)k, CD_RAW_SECTOR_SIZE);
						}
#endif
					}
				}
				OutputC2ErrorLogA("\n");
				idx++;
				if (!FlushDriveCache(pExtArg, pDevice, nLBA)) {
					throw FALSE;
				}
			}

			for (DWORD q = 0; q < dwTransferLen; q++) {
				UINT uiMaxNum = lpRepeatedNum[q][0];
				UINT uiMaxC2 = lpContainsC2[q][0];
				for (INT k = 0; k < idx - 1; k++) {
					uiMaxNum = max(uiMaxNum, lpRepeatedNum[q][k + 1]);
					uiMaxC2 = max(uiMaxC2, lpContainsC2[q][k + 1]);
				}

				INT nTmpLBA = nLBA - pDisc->MAIN.nOffsetStart + (INT)q;
				if (uiMaxC2 == 0) {
					OutputC2ErrorWithLBALogA(
						"to[%06d] All crc32 is probably bad. No rewrite\n", nTmpLBA - 1, nTmpLBA);
				}
				else if (uiMaxNum + 1 == pExtArg->uiMaxRereadNum &&
					lpCrc32RereadSector[q][0] == pDisc->MAIN.lpAllSectorCrc32[nTmpLBA]) {
					OutputC2ErrorWithLBALogA(
						"to[%06d] All same crc32. No rewrite\n", nTmpLBA - 1, nTmpLBA);
				}
				else {
					for (INT l = 0; l < idx; l++) {
						if (uiMaxC2 == lpContainsC2[q][l]) {
							OutputC2ErrorWithLBALogA(
								"to[%06d] crc32[%d]:0x%08lx, no c2 %u times. Rewrite\n"
								, nTmpLBA - 1, nTmpLBA, l, lpCrc32RereadSector[q][l], uiMaxC2 + 1);
							fseek(fpImg, CD_RAW_SECTOR_SIZE * (LONG)(nLBA + q) - pDisc->MAIN.nCombinedOffset, SEEK_SET);
							// Write track to scrambled again
							WriteMainChannel(pExecType, pExtArg, pDisc, &lpRereadSector[q][CD_RAW_SECTOR_SIZE * l], nLBA, fpImg);
							fseek(fpC2, CD_RAW_READ_C2_294_SIZE * (LONG)nLBA - (pDisc->MAIN.nCombinedOffset / 8), SEEK_SET);
							if (q + 1 < dwTransferLen) {
								WriteC2(pExtArg, pDisc, &lpRereadSector[q + 1][CD_RAW_SECTOR_SIZE * l] + pDevice->TRANSFER.uiBufC2Offset, nLBA, fpC2);
							}
#if 0
							OutputC2ErrorLogA("Seek to %ld (0x%08lx)\n"
								, CD_RAW_SECTOR_SIZE * (LONG)(nLBA + q) - pDisc->MAIN.nCombinedOffset
								, CD_RAW_SECTOR_SIZE * (LONG)(nLBA + q) - pDisc->MAIN.nCombinedOffset);
							OutputCDMain(fileC2Error, &lpRereadSector[q][CD_RAW_SECTOR_SIZE * l], nTmpLBA, CD_RAW_SECTOR_SIZE);
							OutputC2ErrorLogA("\n");
#endif
							break;
						}
					}
				}
			}
			nLBA += (INT)dwTransferLen;
			for (DWORD r = 0; r < dwTransferLen; r++) {
				for (INT p = 0; p < idx; p++) {
					lpRereadSector[r][p] = 0;
					lpCrc32RereadSector[r][p] = 0;
					lpRepeatedNum[r][p] = 0;
					lpContainsC2[r][p] = 0;
				}
			}
		}
		OutputLogA(standardOut | fileC2Error, "\n");
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
	BYTE byScanProtectViaFile,
	_DISC::_PROTECT protect,
	LPCTSTR pszImgPath,
	_DISC::_PROTECT::_ERROR_SECTOR errorSector
) {
	CONST INT nCmdSize = 6;
	CONST INT nStrSize = _MAX_PATH * 2 + nCmdSize;
	_TCHAR str[nStrSize] = {};
	_TCHAR cmd[nCmdSize] = { _T("check") };
	INT nStartLBA = errorSector.nExtentPos;
	INT nEndLBA = errorSector.nExtentPos + errorSector.nSectorSize;
	if (byScanProtectViaFile) {
		if (protect.byExist == safeDisc || protect.byExist == safeDiscLite ||
			protect.byExist == codelock || protect.byExist == datel ||
			protect.byExist == datelAlt) {
			_tcsncpy(cmd, _T("fix"), sizeof(cmd) / sizeof(cmd[0]));
		}
	}
	INT ret = 0;
	if (GetEccEdcCmd(str, nStrSize, cmd, pszImgPath, nStartLBA, nEndLBA)) {
		OutputString(_T("Exec %s\n"), str);
		ret = _tsystem(str);
	}
	if (protect.byExist == microids || protect.byExist == datelAlt) {
		nStartLBA = errorSector.nExtentPos2nd;
		nEndLBA = errorSector.nExtentPos2nd + errorSector.nSectorSize2nd;
		if (GetEccEdcCmd(str, nStrSize, cmd, pszImgPath, nStartLBA, nEndLBA)) {
			OutputString(_T("Exec %s\n"), str);
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
	INT nMainDataType,
	INT nPadType,
	FILE* fpImg,
	FILE* fpSub,
	FILE* fpC2
) {
#if 0
	if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		OutputCDMain(fileMainError,
			pDiscPerSector->mainHeader.current, nLBA, MAINHEADER_MODE1_SIZE);
	}
#endif
	WriteErrorBuffer(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector,
		scrambled_table, nLBA, nMainDataType, nPadType, fpImg, fpSub, fpC2);
	UpdateTmpMainHeader(pDiscPerSector, nMainDataType);
	UpdateTmpSubQData(pDiscPerSector);
#if 1
	if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		LPBYTE pBuf = NULL;
		if (pExtArg->byBe) {
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
		OutputCDMain(fileMainError, pBuf, nLBA, MAINHEADER_MODE1_SIZE);
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
	_tcsncpy(pszNewPath, pszOutScmFile, sizeof(pszNewPath) / sizeof(pszNewPath[0]));
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
	if (pExtArg->byBe || (pDisc->SCSI.trackType != TRACK_TYPE::dataExist && pDisc->SCSI.trackType != TRACK_TYPE::pregapDataIn1stTrack)) {
		OutputString(_T("Moving .scm to .img\n"));
		if (!MoveFileEx(pszOutScmFile, pszNewPath, MOVEFILE_REPLACE_EXISTING)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (pExtArg->byBe) {
			ExecEccEdc(pExtArg->byScanProtectViaFile, pDisc->PROTECT, pszNewPath, pDisc->PROTECT.ERROR_SECTOR);
		}
	}
	else {
		OutputString(_T("Copying .scm to .img\n"));
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
		DescrambleMainChannelAll(pExtArg, pDisc, scrambled_table, fpImg);
		FcloseAndNull(fpImg);
		ExecEccEdc(pExtArg->byScanProtectViaFile, pDisc->PROTECT, pszImgPath, pDisc->PROTECT.ERROR_SECTOR);
	}
	return TRUE;
}

BOOL ProcessCreateBin(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszPath,
	FILE* fpCue,
	FILE* fpCueForImg,
	FILE* fpCcd
) {
	FILE* fpImg = NULL;
	_TCHAR pszImgName[_MAX_FNAME] = {};
	if (NULL == (fpImg = CreateOrOpenFile(
		pszPath, NULL, NULL, pszImgName, NULL, _T(".img"), _T("rb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (!CreateBinCueCcd(pExtArg, pDisc, pszPath, pszImgName,
		pDevice->FEATURE.byCanCDText, fpImg, fpCue, fpCueForImg, fpCcd)) {
		FcloseAndNull(fpImg);
		return FALSE;
	}
	FcloseAndNull(fpImg);
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
	FILE* fpImg = NULL;
	_TCHAR pszOutScmFile[_MAX_PATH] = {};
	if (NULL == (fpImg = CreateOrOpenFile(pszPath, NULL,
		pszOutScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	FILE* fpCue = NULL;
	FILE* fpCueForImg = NULL;
	FILE* fpSub = NULL;
	LPBYTE pBuf = NULL;
	LPBYTE pNextBuf = NULL;
	LPBYTE pNextNextBuf = NULL;
	INT nMainDataType = scrambled;
	if (pExtArg->byBe) {
		nMainDataType = unscrambled;
	}

	try {
		// init start
		if (NULL == (fpCue = CreateOrOpenFile(
			pszPath, NULL, NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpCueForImg = CreateOrOpenFile(
			pszPath, _T("_img"), NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpSub = CreateOrOpenFile(
			pszPath, NULL, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		BYTE byTransferLen = 1;
		if (pDevice->byPlxtrDrive) {
			byTransferLen = 2;
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
		SetReadDiscCommand(pExecType, pExtArg, pDevice, byTransferLen, c2, CDFLAG::_READ_CD::Raw, lpCmd, TRUE);

		BYTE lpPrevSubcode[CD_RAW_READ_SUBCODE_SIZE] = {};
		// to get prevSubQ
		if (pDisc->SUB.nSubChannelOffset) {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, -2, pDiscPerSector->data.current,
				pDevice->TRANSFER.uiBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
			SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.prev, pDiscPerSector->subcode.current);

			if (!ExecReadCD(pExtArg, pDevice, lpCmd, -1, pDiscPerSector->data.current,
				pDevice->TRANSFER.uiBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
			SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.current, pDiscPerSector->subcode.current);
			memcpy(lpPrevSubcode, pDiscPerSector->subcode.current, CD_RAW_READ_SUBCODE_SIZE);
		}
		else {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, -1, pDiscPerSector->data.current,
				pDevice->TRANSFER.uiBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
			SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.prev, pDiscPerSector->subcode.current);
		}
		OutputString(_T(STR_LBA "Q[%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]\n")
			, -1, -1, pDiscPerSector->subcode.current[12]
			, pDiscPerSector->subcode.current[13], pDiscPerSector->subcode.current[14]
			, pDiscPerSector->subcode.current[15], pDiscPerSector->subcode.current[16]
			, pDiscPerSector->subcode.current[17], pDiscPerSector->subcode.current[18]
			, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
			, pDiscPerSector->subcode.current[21], pDiscPerSector->subcode.current[22]
			, pDiscPerSector->subcode.current[23]);
		// special fix begin
//		if (pDiscPerSector->subQ.prev.byAdr != ADR_ENCODES_CURRENT_POSITION) {
			// [PCE] 1552 Tenka Tairan
			pDiscPerSector->subQ.prev.byAdr = ADR_ENCODES_CURRENT_POSITION;
			pDiscPerSector->subQ.prev.byTrackNum = 1;
			pDiscPerSector->subQ.prev.byIndex = 0;
			pDiscPerSector->subQ.prev.nAbsoluteTime = 149;
//		}
		if (!ReadCDForCheckingSecuROM(pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd)) {
			throw FALSE;
		}
		// special fix end

		for (UINT p = 0; p < pDisc->SCSI.toc.LastTrack; p++) {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, pDisc->SCSI.lpFirstLBAListOnToc[p]
				, pDiscPerSector->data.current, pDevice->TRANSFER.uiBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
			pDisc->SUB.lpEndCtlList[p] = (BYTE)((pDiscPerSector->subcode.current[12] >> 4) & 0x0f);
			OutputString(_T("\rChecking SubQ ctl (Track) %2u/%2u"), p + 1, pDisc->SCSI.toc.LastTrack);
		}
		OutputString(_T("\n"));
		SetCDOffset(pExecType, pExtArg->byBe, pDevice->byPlxtrDrive, pDisc, 0, pDisc->SCSI.nAllLength);

		pDiscPerSector->byTrackNum = pDisc->SCSI.toc.FirstTrack;
		INT nFirstLBAForSub = 0;
		INT nFirstLBA = pDisc->MAIN.nOffsetStart;
		INT nLastLBA =  pDisc->SCSI.nAllLength + pDisc->MAIN.nOffsetEnd;
		INT nLBA = nFirstLBA;

		if (pExtArg->byPre) {
			nFirstLBAForSub = PREGAP_START_LBA;
			nFirstLBA = PREGAP_START_LBA;
			nLBA = nFirstLBA;
			pDisc->MAIN.nOffsetStart = PREGAP_START_LBA;
		}
		// init end
		FlushLog();

		BOOL bReadOK = pExtArg->byPre ? FALSE : TRUE;
		BOOL bC2Error = FALSE;
		BOOL bReread = FALSE;
		INT nFirstErrLBA[12] = {};
		INT nSecondSessionLBA = pDisc->MAIN.nFixFirstLBAof2ndSession;
		INT nRetryCnt = 0;

		while (nFirstLBA < nLastLBA) {
			if (pExtArg->byMultiSession) {
				if (lpCmd[0] == 0xbe && pDisc->MAIN.nFixFirstLBAof2ndSession <= nLBA) {
					nMainDataType = scrambled;
					pDisc->MAIN.lpModeList[pDisc->SCSI.byFirstMultiSessionTrackNum - 1] = pDiscPerSector->mainHeader.current[15];
					nSecondSessionLBA = nLBA;
					SetReadDiscCommand(pExecType, pExtArg, pDevice, byTransferLen, c2, CDFLAG::_READ_CD::Raw, lpCmd, FALSE);
				}
			}
			else if (pDisc->PROTECT.byExist == laserlock || pDisc->PROTECT.byExist == proring ||
				pDisc->PROTECT.byExist == physicalErr || pDisc->PROTECT.byExist == microids) {
				if (IsValidProtectedSector(pDisc, nLBA - 1)) {
					ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
						, pDiscPerSector, nLBA, nMainDataType, padByUsr55, fpImg, fpSub, fpC2);
					nLBA++;
					nFirstLBA++;
					continue;
				}
			}

			pDiscPerSector->bReturnCode = FALSE;
			if (pDevice->byAsusDrive && pDisc->SCSI.nAllLength <= nLBA) {
				memcpy(pDiscPerSector->data.current, pDisc->lpCachedBuf, 2352);
				memcpy(pDiscPerSector->data.current + 2352, pDisc->lpCachedBuf + 0x9A4, 294);
				memcpy(pDiscPerSector->data.current + 2352 + 294, pDisc->lpCachedBuf + 2352, 96);
				AlignRowSubcode(pDiscPerSector->subcode.current, pDisc->lpCachedBuf + 2352);
#ifdef _DEBUG
				OutputCDMain(fileMainInfo, pDiscPerSector->data.current, nLBA, CD_RAW_SECTOR_SIZE);
				OutputCDSub96Align(fileMainInfo, pDiscPerSector->subcode.current, nLBA);
				OutputCDC2Error296(fileC2Error, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufC2Offset, nLBA);
#endif
				pDiscPerSector->bReturnCode = TRUE;
			}
			else {
				pDiscPerSector->bReturnCode = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd, nLBA);
			}
			if (pExtArg->byMultiSession && pDiscPerSector->subcode.current[14] == 0xa1 &&
				nFirstErrLBA[0] == 0 && pDiscPerSector->bReturnCode != RETURNED_CONTINUE) {
				if (!memcmp(&pDiscPerSector->subcode.prev[12], &pDiscPerSector->subcode.current[12], 12)) {
					pDiscPerSector->bReturnCode = RETURNED_CONTINUE;
				}
			}

			if (pDiscPerSector->bReturnCode == RETURNED_EXIST_C2_ERROR) {
				bC2Error = TRUE;
#ifdef _DEBUG
				OutputCDC2Error296(fileC2Error, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufC2Offset, nLBA);
				OutputCDC2Error296(fileC2Error, pDiscPerSector->data.next + pDevice->TRANSFER.uiBufC2Offset, nLBA + 1);
#endif
				// C2 error points the current LBA - 1 (offset?)
				OutputLogA(standardError | fileC2Error,
					" LBA[%06d, %#07x] Detected C2 error %d bit\n", nLBA, nLBA, pDiscPerSector->uiC2errorNum);
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					if (!(IsValidProtectedSector(pDisc, nLBA) && IsValidIntentionalC2error(pDisc, pDiscPerSector))) {
						pDisc->MAIN.lpAllLBAOfC2Error[pDisc->MAIN.nC2ErrorCnt++] = nLBA;
					}
				}
#ifdef _DEBUG
				FILE* fpErrbin = NULL;
				CHAR tmp[16] = {};
				_snprintf(tmp, sizeof(tmp) / sizeof(tmp[0]),"_%06d", nLBA);
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
				nLBA = pDisc->MAIN.nFixFirstLBAof2ndSession - 1;
				nFirstLBA = nLBA;
			}
			else if (pDiscPerSector->bReturnCode == RETURNED_CONTINUE) {
				if (!(pExtArg->byPre && -1149 <= nLBA && nLBA <= -76)) {
					if (pExtArg->byMultiSession && nFirstErrLBA[0] == 0) {
						OutputString(" End of readable sector\n");
#ifdef _DEBUG
						OutputCDMain(fileMainError, pDiscPerSector->data.current, nLBA, 2352);
#endif
						nFirstErrLBA[0] = nLBA;
						BYTE ctl = pDiscPerSector->subQ.current.byCtl;
						for (INT i = nLBA; i <= pDisc->MAIN.nFixFirstLBAof2ndSession - 150; i++) {
							ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
								, pDiscPerSector, nLBA, nMainDataType, padByPrevSector, fpImg, fpSub, fpC2);
							if (i != pDisc->MAIN.nFixFirstLBAof2ndSession - 150) {
								nLBA++;
								nFirstLBA++;
							}
						}
						INT idx = pDisc->SCSI.byFirstMultiSessionTrackNum - 1;
						if ((ctl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
							pDisc->SUB.lpLastLBAListOfDataTrackOnSub[idx - 1] = nLBA - 1;
							pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[idx] = pDisc->SCSI.nFirstLBAof2ndSession;
						}
						pDisc->SUB.lpFirstLBAListOnSub[idx][0] = pDisc->SCSI.nFirstLBAof2ndSession - 150;
						pDisc->SUB.lpFirstLBAListOnSub[idx][1] = pDisc->SCSI.nFirstLBAof2ndSession;

						pDiscPerSector->subQ.prev.byIndex = 0;
						pDiscPerSector->byTrackNum = pDisc->SCSI.byFirstMultiSessionTrackNum;

						// because unless using 0xbe, it can't get the pregap perfectly
						CDB::_READ_CD cdb = {};
						SetReadCDCommand(pDevice, &cdb, CDFLAG::_READ_CD::All, byTransferLen, c2, CDFLAG::_READ_CD::Raw);
						memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
						nMainDataType = unscrambled;

						OutputString("Sleep 20000 msec\n");
						Sleep(20000);
						continue;
					}
					else {
						INT nPadType = padByUsr55;
						if (pExtArg->byMultiSession) {
							nPadType = padByPrevSector;
						}
						ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
							, pDiscPerSector, nLBA, nMainDataType, nPadType, fpImg, fpSub, fpC2);
						FlushLog();
					}
				}
			}
			else if (pDiscPerSector->bReturnCode == RETURNED_FALSE) {
				throw FALSE;
			}

			if (pDiscPerSector->bReturnCode != RETURNED_CONTINUE &&
				pDiscPerSector->bReturnCode != RETURNED_SKIP_LBA) {
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData
					&& pExtArg->nC2RereadingType == 1 && nFirstLBA - pDisc->MAIN.nOffsetStart >= 0) {
					GetCrc32(&pDisc->MAIN.lpAllSectorCrc32[nFirstLBA - pDisc->MAIN.nOffsetStart]
						, pDiscPerSector->data.current, CD_RAW_SECTOR_SIZE);
				}
				if (pDisc->SUB.nSubChannelOffset) {
					if (!IsValidProtectedSector(pDisc, nLBA)) {
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
					SetBufferFromTmpSubQData(pDiscPerSector->subcode.current, pDiscPerSector->subQ.current, FALSE, TRUE);
				}
#endif
				SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.current, pDiscPerSector->subcode.current);

				if (pExtArg->byPre && PREGAP_START_LBA <= nLBA && nLBA <= -76) {
					if (pDiscPerSector->subQ.current.byTrackNum == 1 &&
						pDiscPerSector->subQ.current.nAbsoluteTime == 0) {
						pDiscPerSector->subQ.prev.nRelativeTime = pDiscPerSector->subQ.current.nRelativeTime + 1;
						pDiscPerSector->subQ.prev.nAbsoluteTime = -1;
						pDisc->MAIN.nFixStartLBA = nLBA;
						bReadOK = TRUE;
#if 0
						if (pDisc->MAIN.nAdjustSectorNum < 0 ||
							1 < pDisc->MAIN.nAdjustSectorNum) {
							for (INT i = 0; i < abs(pDisc->MAIN.nAdjustSectorNum) * CD_RAW_SECTOR_SIZE; i++) {
								fputc(0, fpImg);
							}
						}
#endif
					}
					if (bReadOK) {
						if ((pDiscPerSector->subQ.current.byAdr == ADR_ENCODES_CURRENT_POSITION &&
							pDiscPerSector->subQ.current.byTrackNum == 1 &&
							pDiscPerSector->subQ.current.nAbsoluteTime == 74) ||
							((pDiscPerSector->subQ.current.byAdr == ADR_ENCODES_MEDIA_CATALOG ||
							pDiscPerSector->subQ.current.byAdr == ADR_ENCODES_ISRC) &&
							pDiscPerSector->subQ.prev.byTrackNum == 1 &&
							pDiscPerSector->subQ.prev.nAbsoluteTime == 73)
							) {
							nFirstLBA = -76;
						}
					}
				}
#if 0
				if (pExtArg->byPre && PREGAP_START_LBA <= nLBA && nLBA <= -76) {
					OutputCDSub96Align(pDiscPerSector->subcode.current, nLBA);
					OutputCDMain(fileDisc, pDiscPerSector->data.current, nLBA, CD_RAW_SECTOR_SIZE);
				}
#endif
				if (bReadOK) {
					if (nFirstLBAForSub <= nLBA && nLBA < pDisc->SCSI.nAllLength) {
						if (IsCheckingSubChannel(pExtArg, pDisc, nLBA)) {
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
								OutputCDMain(fileMainError, pDiscPerSector->data.current, nLBA, 2352);
								OutputCDC2Error296(fileMainError, pDiscPerSector->data.current + 2352, nLBA);
								OutputCDSub96Raw(fileMainError, pDiscPerSector->data.current + 2352 + 294, nLBA);
								FlushLog();
								if (nRetryCnt++ == 0) {
									OutputErrorString(
										"\n" STR_LBA "Failed to reread because crc16 of subQ is 0. Read back 100 sector\n", nLBA, nLBA);
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
										, nLBA, nLBA);
									throw FALSE;
								}
							}
#if 0
							OutputCDSub96Align(pDiscPerSector->subcode.current, nLBA);
							OutputCDSub96Raw(standardOut, lpSubcodeRaw, nLBA);
#endif
							WriteSubChannel(pDisc, pDiscPerSector, lpSubcodeRaw, nLBA, fpSub);
							FixMainHeader(pExtArg, pDisc, pDiscPerSector, nLBA, nMainDataType);
							SetTrackAttribution(pExecType, pExtArg, pDisc, pDiscPerSector, nLBA);
							UpdateTmpSubQData(pDiscPerSector);
						}
					}
					// Write track to scrambled
					if (pExtArg->byMultiSession && nSecondSessionLBA == nLBA) {
						fwrite(pDiscPerSector->data.current + pDisc->MAIN.uiMainDataSlideSize
							, sizeof(BYTE), CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize, fpImg);
					}
					else {
						WriteMainChannel(pExecType, pExtArg, pDisc, pDiscPerSector->data.current, nLBA, fpImg);
					}
#if 0
					OutputCDMain(standardOut, pDiscPerSector->data.current, nLBA, 2352);
#endif

					if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
						if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
							WriteC2(pExtArg, pDisc, pDiscPerSector->data.next + pDevice->TRANSFER.uiBufC2Offset, nLBA, fpC2);
						}
						else {
							WriteC2(pExtArg, pDisc, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufC2Offset, nLBA, fpC2);
						}
					}
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

			OutputString(_T("\rCreating .scm (LBA) %6d/%6d"), nLBA, nLastLBA - 1);
			if (nFirstLBA == -76) {
				nLBA = nFirstLBA;
				if (!bReadOK) {
					bReadOK = TRUE;
				}
			}
			nLBA++;
			nFirstLBA++;
		}
		OutputString(_T("\n"));
		FcloseAndNull(fpSub);
		FlushLog();

		for (INT i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
			if (pDisc->PROTECT.byExist == cds300 && i == pDisc->SCSI.toc.LastTrack - 1) {
				break;
			}
#if 0
			OutputString("lpFirstLBAListOnSub[%d][0]: %d, [%d][1]: %d\n"
				, i, pDisc->SUB.lpFirstLBAListOnSub[i][0], i, pDisc->SUB.lpFirstLBAListOnSub[i][1]);
#endif
			BOOL bErr = FALSE;
			LONG lLine = 0;
			if (pDisc->SUB.lpFirstLBAListOnSub[i][1] == -1) {
				bErr = TRUE;
				lLine = __LINE__;
			}
			else if ((pDisc->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				if (pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[i] == -1) {
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
					_T("[L:%ld] Internal error. Failed to analyze the subchannel. Track[%02u]/[%02u]\n"),
					lLine, i + 1, pDisc->SCSI.toc.LastTrack);
				throw FALSE;
			}
		}
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (bC2Error && pDisc->MAIN.nC2ErrorCnt > 0) {
				if (pExtArg->nC2RereadingType == 0) {
					if (!ReadCDForRereadingSectorType1(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd, fpImg, fpC2, 0)) {
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
					if (!ReadCDForRereadingSectorType2(pExecType, pExtArg, pDevice, pDisc, lpCmd, fpImg, fpC2, nStartLBA, nEndLBA)) {
						throw FALSE;
					}
				}
			}
			else {
				if (pDisc->PROTECT.byExist == PROTECT_TYPE_CD::no) {
					OutputLogA(standardOut | fileC2Error, "No C2 errors\n");
				}
				else {
					OutputLogA(standardOut | fileC2Error, "No unintentional C2 errors\n");
				}
			}
		}
		FcloseAndNull(fpImg);
		OutputTocWithPregap(pDisc);
		FlushLog();

		if (!ProcessDescramble(pExtArg, pDisc, pszPath, pszOutScmFile)) {
			throw FALSE;
		}
		if (!ProcessCreateBin(pExtArg, pDevice, pDisc, pszPath, fpCue, fpCueForImg, fpCcd)) {
			throw FALSE;
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpImg);
	FcloseAndNull(fpCueForImg);
	FcloseAndNull(fpCue);
	FcloseAndNull(fpSub);
	if (pDevice->byAsusDrive) {
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
	FILE* fpCue = NULL;
	FILE* fpCueForImg = NULL;
	FILE* fpSub = NULL;
	FILE* fpLeadout = NULL;
	LPBYTE pBuf = NULL;
	INT nMainDataType = scrambled;
	if (*pExecType == data || pExtArg->byBe) {
		nMainDataType = unscrambled;
	}

	try {
		// init start
		if (NULL == (fpCue = CreateOrOpenFile(
			pszPath, NULL, NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpCueForImg = CreateOrOpenFile(
			pszPath, _T("_img"), NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
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
		SetReadDiscCommand(pExecType, pExtArg, pDevice, byTransferLen, c2, CDFLAG::_READ_CD::Raw, lpCmd, TRUE);
		SetCDOffset(pExecType, pExtArg->byBe, pDevice->byPlxtrDrive, pDisc, nStart, nEnd);
		pDiscPerSector->byTrackNum = 1;

		INT nFirstLBA = nStart + pDisc->MAIN.nOffsetStart;
		INT nLastLBA = nEnd + pDisc->MAIN.nOffsetEnd;
		INT nLBA = nFirstLBA;
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

		while (nFirstLBA < nLastLBA) {
			pDiscPerSector->bReturnCode = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd, nLBA);
			if (pDiscPerSector->bReturnCode == RETURNED_EXIST_C2_ERROR) {
				bC2Error = TRUE;
				// C2 error points the current LBA - 1 (offset?)
				OutputLogA(standardError | fileC2Error,
					"\rLBA[%06d, %#07x] Detected C2 error %d bit\n", nLBA, nLBA, pDiscPerSector->uiC2errorNum);
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					if (!(IsValidProtectedSector(pDisc, nLBA) && (pDisc->PROTECT.byExist == codelock
						|| IsValidSafeDiscSector(pDisc, pDiscPerSector)))) {
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
						, pDiscPerSector, nLBA, nMainDataType, padByAll0, fpScm, fpSub, fpC2);
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
								, pDiscPerSector, nLBA, nMainDataType, padByAll0, fpScm, fpSub, fpC2);
							nLBA++;
							nFirstLBA++;
						}
						continue;
					}
					else {
						ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
							, pDiscPerSector, nLBA, nMainDataType, padByAll0, fpScm, fpSub, fpC2);
					}
				}
				else {
					ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
						, pDiscPerSector, nLBA, nMainDataType, padByAll0, fpScm, fpSub, fpC2);
				}
			}
			else if (pDiscPerSector->bReturnCode == RETURNED_FALSE) {
				throw FALSE;
			}

			if (pDiscPerSector->bReturnCode != RETURNED_CONTINUE) {
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData
					&& pExtArg->nC2RereadingType == 1 && nFirstLBA - pDisc->MAIN.nOffsetStart >= 0) {
					GetCrc32(&pDisc->MAIN.lpAllSectorCrc32[nFirstLBA - pDisc->MAIN.nOffsetStart]
						, pDiscPerSector->data.current, CD_RAW_SECTOR_SIZE);
				}
				SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.current, pDiscPerSector->subcode.current);
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
						if (IsCheckingSubChannel(pExtArg, pDisc, nLBA)) {
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
									, nLBA, nLBA);
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
						UpdateTmpSubQData(pDiscPerSector);
					}
				}
				// Write track to scrambled
				WriteMainChannel(pExecType, pExtArg, pDisc, pDiscPerSector->data.current, nLBA, fpScm);
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
						WriteC2(pExtArg, pDisc, pDiscPerSector->data.next + pDevice->TRANSFER.uiBufC2Offset, nLBA, fpC2);
					}
					else {
						WriteC2(pExtArg, pDisc, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufC2Offset, nLBA, fpC2);
					}
				}
			}
			OutputString(_T("\rCreating .scm from %d to %d (LBA) %6d")
				, nStart + pDisc->MAIN.nOffsetStart, nEnd + pDisc->MAIN.nOffsetEnd, nLBA);
			nLBA++;
			nFirstLBA++;
		}
		OutputString(_T("\n"));
		FcloseAndNull(fpSub);
		FlushLog();
		nLBA = 0;

		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (bC2Error && pDisc->MAIN.nC2ErrorCnt > 0) {
				if (pExtArg->nC2RereadingType == 0) {
					if (!ReadCDForRereadingSectorType1(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd, fpScm, fpC2, 0)) {
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
					if (!ReadCDForRereadingSectorType2(pExecType, pExtArg, pDevice, pDisc, lpCmd, fpScm, fpC2, nStartLBA, nEndLBA)) {
						throw FALSE;
					}
				}
			}
			else {
				if (pDisc->PROTECT.byExist == PROTECT_TYPE_CD::no) {
					OutputLogA(standardOut | fileC2Error, "No C2 errors\n");
				}
				else {
					OutputLogA(standardOut | fileC2Error, "No unintentional C2 errors\n");
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
		OutputString(_T("Close the tray automatically after 3000 msec\n"));
		Sleep(3000);
		if (!GetHandle(pDevice)) {
			throw FALSE;
		}
		// close
		bRet = StartStopUnit(pExtArg, pDevice, START_UNIT_CODE, START_UNIT_CODE);
		OutputString(_T("Wait 15000 msec until your drive recognizes the disc\n"));
		Sleep(15000);

		OutputString(_T("Read TOC\n"));
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
		if (!ReadTOC(pExtArg, pExecType, pDevice, pDisc)) {
			throw FALSE;
		}
		pExtArg->byBe = TRUE;
		WriteCcdFirst(pExtArg, pDevice, pDisc, pDiscPerSector, &fullToc, pTocData, wTocEntries, fpCcd);
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
				OutputErrorString(_T("Failed to read [F:%s][L:%d]\n"), _T(__FUNCTION__), __LINE__);
				break;
			}
			SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.current, pDiscPerSector->subcode.current);
			if (nLBA < nFirstLeadErrLBA) {
				FixSubChannel(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, nLBA, &bReread);
			}
			if (pDiscPerSector->subQ.current.nAbsoluteTime == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->byTrackNum - 1] + 150) {
				if (fread(pDiscPerSector->mainHeader.current, sizeof(BYTE), MAINHEADER_MODE1_SIZE, fpScm) < MAINHEADER_MODE1_SIZE) {
					OutputErrorString(_T("Failed to read [F:%s][L:%d]\n"), _T(__FUNCTION__), __LINE__);
					break;
				}
				fseek(fpScm, CD_RAW_SECTOR_SIZE * pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->byTrackNum], SEEK_SET);
#if 0
				OutputCDMain(fileDisc, pDiscPerSector->mainHeader.current
					, pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->byTrackNum - 1], MAINHEADER_MODE1_SIZE);
#endif
			}
			SetTrackAttribution(pExecType, pExtArg, pDisc, pDiscPerSector, nLBA);
			UpdateTmpSubQData(pDiscPerSector);
			nLBA++;
		}
		FcloseAndNull(fpScm);

		if (!ProcessDescramble(pExtArg, pDisc, pszPath, pszScmPath)) {
			throw FALSE;
		}
		if (!ProcessCreateBin(pExtArg, pDevice, pDisc, pszPath, fpCue, fpCueForImg, fpCcd)) {
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
					OutputErrorString(_T("Failed to read [F:%s][L:%d]\n"), _T(__FUNCTION__), __LINE__);
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
					_T("\rDescrambling lead-out of img (LBA) %6d/%6d"), i, nMaxSize - 1);
			}
			OutputString(_T("\n"));
			FcloseAndNull(fpImg);

			if (NULL == (fpImg = CreateOrOpenFile(
				pszPath, NULL, NULL, NULL, NULL, _T(".img"), _T("rb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			fseek(fpImg, CD_RAW_SECTOR_SIZE * pDisc->SCSI.nAllLength, SEEK_SET);

			for (INT i = pDisc->SCSI.nAllLength; i < nMaxSize; i++) {
				if (fread(pDiscPerSector->data.current, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg) < CD_RAW_SECTOR_SIZE) {
					OutputErrorString(_T("Failed to read [F:%s][L:%d]\n"), _T(__FUNCTION__), __LINE__);
					break;
				}
				fwrite(pDiscPerSector->data.current, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpLeadout);
				OutputString(
					_T("\rCopying lead-out of img to bin (LBA) %6d/%6d"), i, nMaxSize - 1);
			}
			OutputString(_T("\n"));
			FcloseAndNull(fpImg);
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpLeadout);
	FcloseAndNull(fpScm);
	FcloseAndNull(fpCueForImg);
	FcloseAndNull(fpCue);
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
	if (*pExecType == data || pExtArg->byBe || pDisc->SCSI.trackType == TRACK_TYPE::audioOnly) {
		nMainDataType = unscrambled;
	}
	INT nPadType = padByUsr55;
	if (pDisc->SCSI.trackType == TRACK_TYPE::audioOnly) {
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
			byTransferLen = 2;
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
		SetReadDiscCommand(pExecType, pExtArg, pDevice, byTransferLen, c2, CDFLAG::_READ_CD::Raw, lpCmd, TRUE);

		BYTE lpPrevSubcode[CD_RAW_READ_SUBCODE_SIZE] = {};
		if (pDisc->SUB.nSubChannelOffset) { // confirmed PXS88T, TS-H353A
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, nStart - 2, pDiscPerSector->data.current,
				pDevice->TRANSFER.uiBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				if (nStart == 0) {
					pDiscPerSector->subQ.current.nRelativeTime = 0;
					pDiscPerSector->subQ.current.nAbsoluteTime = 149;
					SetBufferFromTmpSubQData(pDiscPerSector->subcode.current, pDiscPerSector->subQ.current, TRUE, TRUE);
					SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.prev, pDiscPerSector->subcode.current);
				}
				else {
					throw FALSE;
				}
			}
			else {
				AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
				SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.prev, pDiscPerSector->subcode.current);
			}
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, nStart - 1, pDiscPerSector->data.current,
				pDevice->TRANSFER.uiBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				if (nStart == 0) {
					pDiscPerSector->subQ.current.nRelativeTime = 0;
					pDiscPerSector->subQ.current.nAbsoluteTime = 150;
					SetBufferFromTmpSubQData(pDiscPerSector->subcode.current, pDiscPerSector->subQ.current, TRUE, TRUE);
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
				SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.current, pDiscPerSector->subcode.current);
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
					SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.prev, pDiscPerSector->subcode.current);
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
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, pDisc->SCSI.lpFirstLBAListOnToc[p]
					, pDiscPerSector->data.current, pDevice->TRANSFER.uiBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
				pDisc->SUB.lpEndCtlList[p] = (BYTE)((pDiscPerSector->subcode.current[12] >> 4) & 0x0f);
				OutputString(_T("\rChecking SubQ ctl (Track) %2u/%2u"), p + 1, pDisc->SCSI.toc.LastTrack);
			}
			OutputString(_T("\n"));
		}
		SetCDOffset(pExecType, pExtArg->byBe, pDevice->byPlxtrDrive, pDisc, nStart, nEnd);
#ifdef _DEBUG
		OutputString(
			_T("byBe: %d, nCombinedOffset: %d, uiMainDataSlideSize: %u, nOffsetStart: %d, nOffsetEnd: %d, nFixStartLBA: %d, nFixEndLBA: %d\n")
			, pExtArg->byBe, pDisc->MAIN.nCombinedOffset, pDisc->MAIN.uiMainDataSlideSize
			, pDisc->MAIN.nOffsetStart, pDisc->MAIN.nOffsetEnd, pDisc->MAIN.nFixStartLBA, pDisc->MAIN.nFixEndLBA);
#endif
		pDiscPerSector->byTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
		if (pDiscPerSector->byTrackNum < pDisc->SCSI.toc.FirstTrack ||
			pDisc->SCSI.toc.LastTrack < pDiscPerSector->byTrackNum) {
			pDiscPerSector->byTrackNum = pDisc->SCSI.toc.FirstTrack;
		}
#ifdef _DEBUG
		OutputString(
			_T("byBe: %d, nCombinedOffset: %d, uiMainDataSlideSize: %u, nOffsetStart: %d, nOffsetEnd: %d, nFixStartLBA: %d, nFixEndLBA: %d\n")
			, pExtArg->byBe, pDisc->MAIN.nCombinedOffset, pDisc->MAIN.uiMainDataSlideSize
			, pDisc->MAIN.nOffsetStart, pDisc->MAIN.nOffsetEnd, pDisc->MAIN.nFixStartLBA, pDisc->MAIN.nFixEndLBA);
#endif
		INT nFirstLBA = nStart + pDisc->MAIN.nOffsetStart - 1;
		if (pDisc->SCSI.trackType == audioOnly) {
			nFirstLBA = nStart;
		}
//		if (*pExecType == data) {
//			nFirstLBA++;
//		}
		INT nLastLBA = nEnd + pDisc->MAIN.nOffsetEnd;
		if (*pExecType == gd) {
			if (nFirstLBA < 0) {
				nFirstLBA = 0;
			}
			else if (nFirstLBA == 44849) {
				nFirstLBA = 44850;
			}
		}
		INT nLBA = nFirstLBA;
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
		
		while (nFirstLBA < nLastLBA) {
			pDiscPerSector->bReturnCode = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd, nLBA);
			if (pDiscPerSector->bReturnCode == RETURNED_EXIST_C2_ERROR) {
				bC2Error = TRUE;
				// C2 error points the current LBA - 1 (offset?)
				OutputLogA(standardError | fileC2Error,
					" LBA[%06d, %#07x] Detected C2 error %d bit\n", nLBA, nLBA, pDiscPerSector->uiC2errorNum);
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					if (!(IsValidProtectedSector(pDisc, nLBA) && (pDisc->PROTECT.byExist == codelock
						|| IsValidSafeDiscSector(pDisc, pDiscPerSector)))) {
						pDisc->MAIN.lpAllLBAOfC2Error[pDisc->MAIN.nC2ErrorCnt++] = nLBA;
					}
				}
			}
			else if (pDiscPerSector->bReturnCode == RETURNED_SKIP_LBA) {
				if (pExtArg->byReverse) {
					nLBA = pDisc->SCSI.nFirstLBAof2ndSession - SESSION_TO_SESSION_SKIP_LBA;
				}
				else {
					nLBA = pDisc->MAIN.nFixFirstLBAof2ndSession - 1;
				}
				nFirstLBA = nLBA;
			}
			else if (pDiscPerSector->bReturnCode == RETURNED_CONTINUE) {
				if (pDisc->PROTECT.byExist != physicalErr && (!bForceSkip || !bForceSkip2)) {
					if (pDisc->PROTECT.byExist == proring || pDisc->PROTECT.byExist == laserlock ||
						pDisc->PROTECT.byExist == microids) {
						if (!bForceSkip) {
							for (UINT i = 0; i < pExtArg->uiSkipSectors; i++) {
								ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector
									, nLBA, nMainDataType, nPadType, fpBin, fpSub, fpC2);
								nLBA++;
								nFirstLBA++;
							}
							bForceSkip = TRUE;
						}
						else if (!bForceSkip2) {
							for (UINT i = 0; i < pExtArg->uiSkipSectors2; i++) {
								ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector
									, nLBA, nMainDataType, nPadType, fpBin, fpSub, fpC2);
								nLBA++;
								nFirstLBA++;
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
						OutputLog(standardError | fileMainError, _T("Retry %d/10\n"), nRetryCnt);
						nRetryCnt++;
						continue;
					}
					ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector
						, nLBA, nMainDataType, nPadType, fpBin, fpSub, fpC2);
					nRetryCnt = 1;
					OutputLogA(standardOut | fileMainError, "LBA[%06d, %#07x] Reread NG\n", nLBA, nLBA);
				}
			}
			else if (pDiscPerSector->bReturnCode == RETURNED_FALSE) {
				if (*pExecType == gd && nRetryCnt <= 10 && nLBA > 65000) {
					OutputLog(standardError | fileMainError, _T("Retry %d/10\n"), nRetryCnt);
					INT nTmpLBA = 0;
					for (nTmpLBA = nLBA - 20000; 449849 <= nTmpLBA; nTmpLBA -= 20000) {
						OutputString(_T("Reread %d sector\n"), nTmpLBA);
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
					OutputString(_T("\n"));
				}
				else {
					OutputString(_T("\rReread %d sector"), nLBA);
					nLBA++;
					continue;
				}
			}
			if (pDiscPerSector->bReturnCode != RETURNED_CONTINUE &&
				pDiscPerSector->bReturnCode != RETURNED_SKIP_LBA) {
				if (nRetryCnt > 1) {
					OutputLogA(standardOut | fileMainError, "LBA[%06d, %#07x] Reread OK\n", nLBA, nLBA);
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
					&& pExtArg->nC2RereadingType == 1 && nFirstLBA - pDisc->MAIN.nOffsetStart >= 0) {
					GetCrc32(&pDisc->MAIN.lpAllSectorCrc32[nFirstLBA - pDisc->MAIN.nOffsetStart]
						, pDiscPerSector->data.current, CD_RAW_SECTOR_SIZE);
				}
				if (pDisc->SUB.nSubChannelOffset) {
					if (!IsValidProtectedSector(pDisc, nLBA)) {
						if (2 <= pExtArg->uiSubAddionalNum) {
							memcpy(pDiscPerSector->subcode.nextNext, pDiscPerSector->subcode.next, CD_RAW_READ_SUBCODE_SIZE);
						}
						if (1 <= pExtArg->uiSubAddionalNum) {
							memcpy(pDiscPerSector->subcode.next, pDiscPerSector->subcode.current, CD_RAW_READ_SUBCODE_SIZE);
						}
						memcpy(pDiscPerSector->subcode.current, lpPrevSubcode, CD_RAW_READ_SUBCODE_SIZE);
					}
				}
				SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.current, pDiscPerSector->subcode.current);

				if (nStart <= nLBA && nLBA < nEnd) {
					BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = {};
					if (!pExtArg->byReverse) {
						if (IsCheckingSubChannel(pExtArg, pDisc, nLBA)) {
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
									, nLBA, nLBA);
								throw FALSE;
							}
							// fix raw subchannel
							AlignColumnSubcode(lpSubcodeRaw, pDiscPerSector->subcode.current);
#if 0
							OutputCDSub96Align(pDiscPerSector->subcode.current, nLBA);
#endif
							WriteSubChannel(pDisc, pDiscPerSector, lpSubcodeRaw, nLBA, fpSub);
//							if (!(*pExecType == audio || *pExecType == data)) {
//							if (nStart < nLBA) {
								FixMainHeader(pExtArg, pDisc, pDiscPerSector, nLBA, nMainDataType);
//							}
						}
						if (!(*pExecType == audio || *pExecType == data)) {
//						if (nStart < nLBA) {
							SetTrackAttribution(pExecType, pExtArg, pDisc, pDiscPerSector, nLBA);
							UpdateTmpSubQData(pDiscPerSector);
						}
					}
				}
				// Write track to scrambled
				WriteMainChannel(pExecType, pExtArg, pDisc, pDiscPerSector->data.current, nLBA, fpBin);
#if 0
				OutputCDMain(standardOut, pDiscPerSector->data.current, nLBA, 2352);
#endif
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
						WriteC2(pExtArg, pDisc, pDiscPerSector->data.next + pDevice->TRANSFER.uiBufC2Offset, nLBA, fpC2);
					}
					else {
						WriteC2(pExtArg, pDisc, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufC2Offset, nLBA, fpC2);
					}
				}
				if (pDisc->SUB.nSubChannelOffset) {
					memcpy(lpPrevSubcode, pDiscPerSector->subcode.next, CD_RAW_READ_SUBCODE_SIZE);
				}
			}
			if (pExtArg->byReverse) {
				OutputString(_T("\rCreating %s from %d to %d (LBA) %6d"), szExt
					, nEnd + pDisc->MAIN.nOffsetEnd, nStart + pDisc->MAIN.nOffsetStart - 1, nLBA);
				nLBA--;
			}
			else {
				OutputString(_T("\rCreating %s from %d to %d (LBA) %6d"), szExt
					, nStart + pDisc->MAIN.nOffsetStart, nEnd + pDisc->MAIN.nOffsetEnd, nLBA);
				nLBA++;
			}
			nFirstLBA++;
		}
		OutputString(_T("\n"));
		FcloseAndNull(fpSub);
		FlushLog();

		if (*pExecType == gd) {
			for (INT i = pDisc->SCSI.toc.FirstTrack - 1; i < pDisc->SCSI.toc.LastTrack; i++) {
				BOOL bErr = FALSE;
				LONG lLine = 0;
				if (pDisc->SUB.lpFirstLBAListOnSub[i][1] == -1) {
					bErr = TRUE;
					lLine = __LINE__;
				}
				else if ((pDisc->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					if (pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[i] == -1) {
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
						_T("[L:%ld] Internal error. Failed to analyze the subchannel. Track[%02u]/[%02u]\n"),
						lLine, i + 1, pDisc->SCSI.toc.LastTrack);
					throw FALSE;
				}
			}
		}
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (bC2Error && pDisc->MAIN.nC2ErrorCnt > 0) {
				if (pExtArg->nC2RereadingType == 0) {
					if (!ReadCDForRereadingSectorType1(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd, fpBin, fpC2, nStart)) {
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
					if (!ReadCDForRereadingSectorType2(pExecType, pExtArg, pDevice, pDisc, lpCmd, fpBin, fpC2, nStartLBA, nEndLBA)) {
						throw FALSE;
					}
				}
			}
			else {
				if (pDisc->PROTECT.byExist == PROTECT_TYPE_CD::no) {
					OutputLogA(standardOut | fileC2Error, "No C2 errors\n");
				}
				else {
					OutputLogA(standardOut | fileC2Error, "No unintentional C2 errors\n");
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
			OutputString(_T("Reversing _reverse%s to %s\n"), szExt, szExt);
			BYTE rBuf[CD_RAW_SECTOR_SIZE] = {};
			DWORD dwRoop = GetFileSize(0, fpBin_r) - CD_RAW_SECTOR_SIZE/* * 3*/;
			LONG lSeek = CD_RAW_SECTOR_SIZE - (LONG)pDisc->MAIN.uiMainDataSlideSize;
			fseek(fpBin_r, -lSeek, SEEK_END);

			if (fread(rBuf, sizeof(BYTE), (size_t)lSeek, fpBin_r) < (size_t)lSeek) {
				OutputErrorString(_T("Failed to read [F:%s][L:%d]\n"), _T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			fwrite(rBuf, sizeof(BYTE), (size_t)lSeek, fpBin);
			fseek(fpBin_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);

			for (DWORD i = 0; i < dwRoop; i += CD_RAW_SECTOR_SIZE) {
				fseek(fpBin_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
				if (fread(rBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpBin_r) < CD_RAW_SECTOR_SIZE) {
					OutputErrorString(_T("Failed to read [F:%s][L:%d]\n"), _T(__FUNCTION__), __LINE__);
					break;
				}
				fwrite(rBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpBin);
				fseek(fpBin_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
			}
			fseek(fpBin_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
			if (fread(rBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpBin_r) < pDisc->MAIN.uiMainDataSlideSize) {
				OutputErrorString(_T("Failed to read [F:%s][L:%d]\n"), _T(__FUNCTION__), __LINE__);
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
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos = nStart;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize = nEnd - nStart;
			}
			ExecEccEdc(pExtArg->byScanProtectViaFile, pDisc->PROTECT, pszPath, pDisc->PROTECT.ERROR_SECTOR);
		}
		else if (*pExecType == gd) {
			_TCHAR pszImgPath[_MAX_PATH] = {};
			if (!DescrambleMainChannelForGD(pszPath, pszImgPath)) {
				throw FALSE;
			}
			ExecEccEdc(pExtArg->byScanProtectViaFile, pDisc->PROTECT, pszImgPath, pDisc->PROTECT.ERROR_SECTOR);
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
