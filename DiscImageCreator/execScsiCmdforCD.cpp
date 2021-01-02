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
//			OutputCDMain(fileMainInfo, lpBuf + DISC_MAIN_DATA_SIZE * i, nLBA, DISC_MAIN_DATA_SIZE);
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
		, lpBuf, direction, dwBufSize, &byScsiStatus, pszFuncName, lLineNum)
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
				if (pDisc->MAIN.nFixFirstLBAofLeadout == nLBA) {
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
	else if (pExtArg->byMultiSession) {
		if (pDisc->MAIN.nFixFirstLBAofLeadout + LEADOUT_SIZE_OF_MULTISESSION + 4 == nLBA) {
			return RETURNED_CONTINUE;
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
	bRet = ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd, nLBA,
		pDiscPerSector->data.current, _T(__FUNCTION__), __LINE__);

	if (pDevice->byPlxtrDrive) {
		if (bRet == RETURNED_NO_C2_ERROR_1ST) {
			AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
			if (pDiscPerSector->data.next != NULL && 1 <= pExtArg->uiSubAddionalNum) {
				memcpy(pDiscPerSector->data.next, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufLen, pDevice->TRANSFER.uiBufLen);
				AlignRowSubcode(pDiscPerSector->subcode.next, pDiscPerSector->data.next + pDevice->TRANSFER.uiBufSubOffset);
					
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					bRet = ContainsC2Error(pDevice, pDiscPerSector->data.next, &pDiscPerSector->uiC2errorNum, nLBA, TRUE);
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
				bRet = ContainsC2Error(pDevice, pDiscPerSector->data.current, &pDiscPerSector->uiC2errorNum, nLBA, TRUE);
			}
			if (!IsValidProtectedSector(pDisc, nLBA, GetReadErrorFileIdx(pExtArg, pDisc, nLBA))) {
				if (pDiscPerSector->data.next != NULL && 1 <= pExtArg->uiSubAddionalNum) {
					if (!(pDevice->by0xF1Drive && pDisc->SCSI.nAllLength - 1 <= nLBA)) {
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
			, CDFLAG::_READ_CD::byte294, CDFLAG::_READ_CD::Raw, lpCmd, FALSE);

		for (INT m = 0; m < pDisc->MAIN.nC2ErrorCnt; m++) {
			INT nLBA = pDisc->MAIN.lpAllLBAOfC2Error[m];
			for (UINT i = 0; i < pExtArg->uiMaxRereadNum; i++) {
				OutputString("\rNeed to reread sector: %6d rereading times: %4u/%4u"
					, nLBA, i + 1, pExtArg->uiMaxRereadNum);
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * 2, _T(__FUNCTION__), __LINE__)) {
					continue;
				}
				DWORD dwTmpCrc32 = 0;
				GetCrc32(&dwTmpCrc32, lpBuf, CD_RAW_SECTOR_SIZE);
//				OutputC2ErrorWithLBALogA("to [%06d] crc32[%03ld]: 0x%08lx "
//					, nLBA - pDisc->MAIN.nOffsetStart - 1, nLBA - pDisc->MAIN.nOffsetStart, i, dwTmpCrc32);
				OutputC2ErrorWithLBALog("crc32[%03u]: 0x%08lx ", nLBA, i, dwTmpCrc32);

				LPBYTE lpNextBuf = lpBuf + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
				if (ContainsC2Error(pDevice, lpNextBuf, &pDiscPerSector->uiC2errorNum, nLBA, FALSE) == RETURNED_NO_C2_ERROR_1ST) {
					LONG lSeekMain = CD_RAW_SECTOR_SIZE * (LONG)nLBA - nStart - pDisc->MAIN.nCombinedOffset;
					fseek(fpImg, lSeekMain, SEEK_SET);
					// Write track to scrambled again
					WriteMainChannel(pExecType, pExtArg, pDisc, lpBuf, nLBA, fpImg);

					LONG lSeekC2 = CD_RAW_READ_C2_294_SIZE * (LONG)nLBA - nStart - (pDisc->MAIN.nCombinedOffset / 8);
					fseek(fpC2, lSeekC2, SEEK_SET);
					WriteC2(pExtArg, pDisc, lpNextBuf + pDevice->TRANSFER.uiBufC2Offset, nLBA, fpC2);
					OutputC2ErrorLog("good. Rewrote .scm[%ld-%ld(%lx-%lx)] .c2[%ld-%ld(%lx-%lx)]\n"
						, lSeekMain, lSeekMain + 2351, (ULONG)lSeekMain, (ULONG)lSeekMain + 2351
						, lSeekC2, lSeekC2 + 293, (ULONG)lSeekC2, (ULONG)lSeekC2 + 293);
					break;
				}
				else {
					if (i == pExtArg->uiMaxRereadNum - 1 && !pDisc->PROTECT.byExist) {
						OutputLog(standardError | fileC2Error, "\nbad all. need to reread more\n");
						if (IsCDRDrive(pDisc) && pExtArg->uiMaxRereadNum >= 10000) {
							throw TRUE;
						}
						else {
							throw FALSE;
						}
					}
					else {
						OutputC2ErrorLog("bad\n");
					}
				}
				if (!FlushDriveCache(pExtArg, pDevice, nLBA)) {
					throw FALSE;
				}
			}
		}
		OutputString("\nDone. See _c2Error.txt\n");
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
		SetReadDiscCommand(pExecType, pExtArg, pDevice, (BYTE)dwTransferLen
			, CDFLAG::_READ_CD::byte294, CDFLAG::_READ_CD::Raw, lpCmd, FALSE);

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
								OutputCDMain(fileC2Error, &lpRereadSector[k][CD_RAW_SECTOR_SIZE * idx]
									, nLBA - pDisc->MAIN.nOffsetStart + (INT)k, CD_RAW_SECTOR_SIZE);
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
								fseek(fpC2, CD_RAW_READ_C2_294_SIZE * (LONG)nLBA - (pDisc->MAIN.nCombinedOffset / 8), SEEK_SET);
								if (q + 1 < dwTransferLen) {
									WriteC2(pExtArg, pDisc, &lpRereadSector[q + 1][CD_RAW_SECTOR_SIZE * l] + pDevice->TRANSFER.uiBufC2Offset, nLBA, fpC2);
								}
#if 0
								OutputC2ErrorLog("Seek to %ld (0x%08lx)\n"
									, CD_RAW_SECTOR_SIZE * (LONG)(nLBA + q) - pDisc->MAIN.nCombinedOffset
									, CD_RAW_SECTOR_SIZE * (LONG)(nLBA + q) - pDisc->MAIN.nCombinedOffset);
								OutputCDMain(fileC2Error, &lpRereadSector[q][CD_RAW_SECTOR_SIZE * l], nTmpLBA, CD_RAW_SECTOR_SIZE);
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
			protect.byExist == datelAlt || protect.byExist == c2Err) {
			_tcsncpy(cmd, _T("fix"), sizeof(cmd) / sizeof(cmd[0]));
		}
	}
	INT ret = 0;
	if (GetEccEdcCmd(str, nStrSize, cmd, pszImgPath, nStartLBA, nEndLBA)) {
		OutputString("Exec %s\n", str);
		ret = _tsystem(str);
	}
	if (protect.byExist == physicalErr) {
		for (INT i = 1; i < pExtArg->FILE.readErrCnt; i++) {
			nStartLBA = errorSector.nExtentPos[i];
			nEndLBA = errorSector.nExtentPos[i] + errorSector.nSectorSize[i];
			if (GetEccEdcCmd(str, nStrSize, cmd, pszImgPath, nStartLBA, nEndLBA)) {
				OutputString("Exec %s\n", str);
				ret = _tsystem(str);
			}
		}
	}
	else if (protect.byExist == c2Err) {
		for (INT i = 1; i < pExtArg->FILE.c2ErrCnt; i++) {
			nStartLBA = errorSector.nExtentPos[i];
			nEndLBA = errorSector.nExtentPos[i] + errorSector.nSectorSize[i];
			if (GetEccEdcCmd(str, nStrSize, cmd, pszImgPath, nStartLBA, nEndLBA)) {
				OutputString("Exec %s\n", str);
				ret = _tsystem(str);
			}
		}
	}
	else if (protect.byExist == datelAlt) {
		nStartLBA = errorSector.nExtentPos2nd;
		nEndLBA = errorSector.nExtentPos2nd + errorSector.nSectorSize2nd;
		if (GetEccEdcCmd(str, nStrSize, cmd, pszImgPath, nStartLBA, nEndLBA)) {
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
	INT nMainDataType,
	INT nPadType,
	FILE* fpImg,
	FILE* fpSub,
	FILE* fpC2
) {
#if 0
	if ((pDiscPerSector->subch.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		OutputCDMain(fileMainError,
			pDiscPerSector->mainHeader.current, nLBA, MAINHEADER_MODE1_SIZE);
	}
#endif
	WriteErrorBuffer(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector,
		scrambled_table, nLBA, nMainDataType, nPadType, fpImg, fpSub, fpC2);
	UpdateTmpMainHeader(pDiscPerSector, nMainDataType);
	UpdateTmpSubch(pDiscPerSector);
#if 1
	if ((pDiscPerSector->subch.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
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
	if (pExtArg->byBe || (pDisc->SCSI.trkType != TRACK_TYPE::dataExist && pDisc->SCSI.trkType != TRACK_TYPE::pregapDataIn1stTrack)) {
		OutputString("Moving .scm to .img\n");
		if (!MoveFileEx(pszOutScmFile, pszNewPath, MOVEFILE_REPLACE_EXISTING)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (pExtArg->byBe) {
			ExecEccEdc(pExtArg, pDisc->PROTECT, pszNewPath, pDisc->PROTECT.ERROR_SECTOR);
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
		DescrambleMainChannelAll(pExtArg, pDisc, scrambled_table, fpImg);
		FcloseAndNull(fpImg);
		ExecEccEdc(pExtArg, pDisc->PROTECT, pszImgPath, pDisc->PROTECT.ERROR_SECTOR);
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
	if (NULL == (fpImg = CreateOrOpenFile(
		pszPath, NULL, NULL, pszImgName, NULL, _T(".img"), _T("rb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (!CreateBinCueCcd(pExtArg, pDisc, pszPath, pszImgName,
		pDevice->FEATURE.byCanCDText, fpImg, fpCcd)) {
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
	FILE* fpScm = NULL;
	_TCHAR pszOutScmFile[_MAX_PATH] = {};
	if (pExtArg->byMultiSession) {
		if (NULL == (fpScm = CreateOrOpenFile(pszPath, NULL,
			pszOutScmFile, NULL, NULL, _T(".scmtmp"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	else {
		if (NULL == (fpScm = CreateOrOpenFile(pszPath, NULL,
			pszOutScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	BOOL bRet = TRUE;
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
		SetReadDiscCommand(pExecType, pExtArg, pDevice, byTransferLen, c2, pDevice->sub, lpCmd, TRUE);

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
				throw FALSE;
			}
			AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
			pDisc->SUB.lpEndCtlList[p] = (BYTE)((pDiscPerSector->subcode.current[12] >> 4) & 0x0f);
			OutputString("\rChecking SubQ ctl (Track) %2u/%2u", p + 1, pDisc->SCSI.toc.LastTrack);
		}
		OutputString("\n");
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
		BOOL bDupsRead = FALSE;
		INT nFirstErrLBA[12] = {};
		INT nSecondSessionLBA = pDisc->MAIN.nFixFirstLBAof2ndSession;
		if (pDisc->MAIN.nAdjustSectorNum < 0) {
			nSecondSessionLBA = pDisc->SCSI.n1stLBAof2ndSession;
		}
		INT nRetryCnt = 0;

		while (nFirstLBA < nLastLBA) {
			if (pExtArg->byMultiSession && lpCmd[0] == 0xbe) {
				if (!bDupsRead && pDisc->MAIN.nAdjustSectorNum < 0) {
					if (((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) && nLBA == pDisc->SCSI.n1stLBAof2ndSession) {
						// Multi-session: combined offset minus disc
						pDiscPerSector->bReturnCode = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd, nLBA);
						BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = {};
						AlignColumnSubcode(lpSubcodeRaw, pDiscPerSector->subcode.current);
						WriteSubChannel(pDisc, pDiscPerSector, lpSubcodeRaw, nLBA, fpSub);
						for (INT n = 0; n < CD_RAW_SECTOR_SIZE; n++) {
							pDiscPerSector->data.current[n] ^= scrambled_table[n];
						}
						WriteMainChannel(pExecType, pExtArg, pDisc, pDiscPerSector->data.current, nLBA, fpScm);
#ifdef _DEBUG
						OutputCDMain(fileMainInfo, pDiscPerSector->data.current, nLBA, CD_RAW_SECTOR_SIZE);
#endif
						bDupsRead = TRUE;
						continue;
					}
					else if ((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == 0 && nLBA == pDisc->MAIN.nFixFirstLBAof2ndSession) {
						// Multi-session: combined offset minus disc (audio only e.g. atari jaguar CD)
						BYTE zero[CD_RAW_SECTOR_SIZE] = {};
						fwrite(zero, sizeof(BYTE), CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize, fpScm);
						bDupsRead = TRUE;
					}
				}
				else if (pDisc->SCSI.n1stLBAof2ndSession <= nLBA) {
					nMainDataType = scrambled;
					pDisc->MAIN.lpModeList[pDisc->SCSI.by1stMultiSessionTrkNum - 1] = pDiscPerSector->mainHeader.current[15];
					nSecondSessionLBA = nLBA;
					SetReadDiscCommand(pExecType, pExtArg, pDevice, byTransferLen, c2, pDevice->sub, lpCmd, FALSE);
				}
			}
			else if (pDisc->PROTECT.byExist == laserlock || pDisc->PROTECT.byExist == proring ||
				pDisc->PROTECT.byExist == physicalErr) {
				if (IsValidProtectedSector(pDisc, nLBA - 1, GetReadErrorFileIdx(pExtArg, pDisc, nLBA))) {
					ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
						, pDiscPerSector, nLBA, nMainDataType, padByUsr55, fpScm, fpSub, fpC2);
					nLBA++;
					nFirstLBA++;
					continue;
				}
			}

			pDiscPerSector->bReturnCode = FALSE;
			if (pDevice->by0xF1Drive && nLBA >= pDisc->SCSI.nAllLength) {
				INT nOfs = F1_BUFFER_SIZE * (nLBA - pDisc->SCSI.nAllLength);
				// main
				memcpy(pDiscPerSector->data.current, pDisc->lpCachedBuf + nOfs, CD_RAW_SECTOR_SIZE);
				// c2
				memcpy(pDiscPerSector->data.current + CD_RAW_SECTOR_SIZE, pDisc->lpCachedBuf + nOfs + 0x9A4, CD_RAW_READ_C2_294_SIZE);
				// sub
				memcpy(pDiscPerSector->data.current + CD_RAW_SECTOR_WITH_C2_294_SIZE, pDisc->lpCachedBuf + nOfs + CD_RAW_SECTOR_SIZE, CD_RAW_READ_SUBCODE_SIZE);
				AlignRowSubcode(pDiscPerSector->subcode.current, pDisc->lpCachedBuf + nOfs + CD_RAW_SECTOR_SIZE);
#ifdef _DEBUG
				OutputCDMain(fileMainInfo, pDiscPerSector->data.current, nLBA, CD_RAW_SECTOR_SIZE);
				OutputCDSub96Align(fileMainInfo, pDiscPerSector->subcode.current, nLBA);
				OutputCDC2Error296(fileC2Error, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufC2Offset, nLBA);

				BYTE buf[CD_RAW_READ_SUBCODE_SIZE] = {};
				for (INT i = 0; i < 20; i++) {
					OutputCDMain(fileMainInfo, pDisc->lpCachedBuf + 0xb00 * i, nLBA + i, CD_RAW_SECTOR_SIZE);
					AlignRowSubcode(buf, pDisc->lpCachedBuf + 0xb00 * i + CD_RAW_SECTOR_SIZE);
					OutputCDSub96Align(fileMainInfo, buf, nLBA + i);
				}
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
				OutputLog(standardError | fileC2Error,
					" LBA[%06d, %#07x] Detected C2 error %u bit\n", nLBA, (UINT)nLBA, pDiscPerSector->uiC2errorNum);
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					if (!(IsValidIntentionalC2error(pDisc, pDiscPerSector, nLBA, GetC2ErrorFileIdx(pExtArg, pDisc, nLBA)))) {
						pDisc->MAIN.lpAllLBAOfC2Error[pDisc->MAIN.nC2ErrorCnt++] = nLBA;
					}
				}
#ifdef _DEBUG
				FILE* fpErrbin = NULL;
				_TCHAR tmp[16] = {};
				_sntprintf(tmp, sizeof(tmp) / sizeof(tmp[0]), _T("_%06d"), nLBA);
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
						for (INT i = nLBA; i <= pDisc->MAIN.nFixFirstLBAof2ndSession - 150; i++) {
							ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
								, pDiscPerSector, nLBA, nMainDataType, padByPrevSector, fpScm, fpSub, fpC2);
							if (i != pDisc->MAIN.nFixFirstLBAof2ndSession - 150) {
								nLBA++;
								nFirstLBA++;
							}
						}
						if (pDisc->MAIN.nAdjustSectorNum < 0) {
							nLBA = pDisc->SCSI.n1stLBAof2ndSession - 150;
							nFirstLBA = nLBA;
							nFirstLBAForSub = nLBA - pDisc->MAIN.nAdjustSectorNum;
						}
						INT idx = pDisc->SCSI.by1stMultiSessionTrkNum - 1;
						if ((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
							pDisc->SUB.lpLastLBAListOfDataTrackOnSub[idx - 1] = nLBA - 1;
							pDisc->SUB.lp1stLBAListOfDataTrackOnSub[idx] = pDisc->SCSI.n1stLBAof2ndSession;
						}
						pDisc->SUB.lp1stLBAListOnSub[idx][0] = pDisc->SCSI.n1stLBAof2ndSession - 150;
						pDisc->SUB.lp1stLBAListOnSub[idx][1] = pDisc->SCSI.n1stLBAof2ndSession;

						pDiscPerSector->subch.prev.byIndex = 0;
						pDiscPerSector->byTrackNum = pDisc->SCSI.by1stMultiSessionTrkNum;

						// because unless using 0xbe, it can't get the pregap perfectly
						CDB::_READ_CD cdb = {};
						SetReadCDCommand(pDevice, &cdb, CDFLAG::_READ_CD::All, byTransferLen, c2, CDFLAG::_READ_CD::Raw);
						memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
						nMainDataType = unscrambled;

//						OutputString("Sleep 20000 msec\n");
//						Sleep(20000);
						continue;
					}
					else {
						INT nPadType = padByUsr55;
						if (pExtArg->byMultiSession) {
							nPadType = padByPrevSector;
						}
						ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
							, pDiscPerSector, nLBA, nMainDataType, nPadType, fpScm, fpSub, fpC2);
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

				if (pExtArg->byPre && PREGAP_START_LBA <= nLBA && nLBA <= -76) {
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
							FixMainHeader(pExtArg, pDisc, pDiscPerSector, nLBA, nMainDataType);
							SetTrackAttribution(pExecType, pExtArg, pDisc, pDiscPerSector, nLBA);
							UpdateTmpSubch(pDiscPerSector);
						}
					}
					// Write track to scrambled
					if (pExtArg->byMultiSession && nSecondSessionLBA == nLBA &&
						(pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
						fwrite(pDiscPerSector->data.current + pDisc->MAIN.uiMainDataSlideSize
							, sizeof(BYTE), CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize, fpScm);
					}
					else {
						WriteMainChannel(pExecType, pExtArg, pDisc, pDiscPerSector->data.current, nLBA, fpScm);
					}
#ifdef _DEBUG
					if (nLBA == pDisc->MAIN.nFixFirstLBAof2ndSession || nSecondSessionLBA == nLBA) {
						OutputCDMain(fileMainInfo, pDiscPerSector->data.current, nLBA, CD_RAW_SECTOR_SIZE);
					}
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

			OutputString("\rCreating .scm (LBA) %6d/%6d", nLBA, nLastLBA - 1);
			if (nFirstLBA == -76) {
				nLBA = nFirstLBA;
				if (!bReadOK) {
					bReadOK = TRUE;
				}
			}
			nLBA++;
			nFirstLBA++;
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
					if (!ReadCDForRereadingSectorType1(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd, fpScm, fpC2, 0)) {
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
					if (!ReadCDForRereadingSectorType2(pExecType, pExtArg, pDevice, pDisc, lpCmd, fpScm, fpC2, nStartLBA, nEndLBA)) {
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
		if (pExtArg->byMultiSession) {
			OutputString("Trimming lead-out, lead-in, pregap of 1st track of 2nd session\n");
			_TCHAR pszOutImgFile[_MAX_PATH] = {};
			_tcsncpy(pszOutImgFile, pszPath, sizeof(pszOutImgFile) / sizeof(pszOutImgFile[0]) - 1);
			if (!PathRenameExtension(pszOutImgFile, _T(".img"))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			_TCHAR pszOutImgFileTmp[_MAX_PATH] = {};
			_tcsncpy(pszOutImgFileTmp, pszPath, sizeof(pszOutImgFileTmp) / sizeof(pszOutImgFileTmp[0]) - 1);
			if (!PathRenameExtension(pszOutImgFileTmp, _T(".imgtmp"))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			if (!MoveFileEx(pszOutImgFile, pszOutImgFileTmp, MOVEFILE_REPLACE_EXISTING)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			_tremove(pszOutImgFile);

			_TCHAR pszOutSubFile[_MAX_PATH] = {};
			_tcsncpy(pszOutSubFile, pszPath, sizeof(pszOutSubFile) / sizeof(pszOutSubFile[0]) - 1);
			if (!PathRenameExtension(pszOutSubFile, _T(".sub"))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			_TCHAR pszOutSubFileTmp[_MAX_PATH] = {};
			_tcsncpy(pszOutSubFileTmp, pszPath, sizeof(pszOutSubFileTmp) / sizeof(pszOutSubFileTmp[0]) - 1);
			if (!PathRenameExtension(pszOutSubFileTmp, _T(".subtmp"))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			if (!MoveFileEx(pszOutSubFile, pszOutSubFileTmp, MOVEFILE_REPLACE_EXISTING)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			_tremove(pszOutSubFile);

			FILE* fpImg = NULL;
			if (NULL == (fpImg = CreateOrOpenFile(pszPath, NULL
				, pszOutImgFileTmp, NULL, NULL, _T(".imgtmp"), _T("rb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			FILE* fpImg2 = NULL;
			if (NULL == (fpImg2 = CreateOrOpenFile(pszPath, NULL
				, NULL, NULL, NULL, _T(".img"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString("%s\n", pszOutScmFile);
				return FALSE;
			}
			if (NULL == (fpSub = CreateOrOpenFile(pszPath, NULL
				, pszOutSubFileTmp, NULL, NULL, _T(".subtmp"), _T("rb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FILE* fpSub2 = NULL;
			if (NULL == (fpSub2 = CreateOrOpenFile(pszPath, NULL
				, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			BYTE main[CD_RAW_SECTOR_SIZE] = {};
			BYTE sub[CD_RAW_READ_SUBCODE_SIZE] = {};
			size_t readMainSize = 0;
			size_t readSubSize = 0;
			for (INT i = 0; i < pDisc->SCSI.nAllLength; i++) {
				readMainSize = fread(main, sizeof(BYTE), sizeof(main), fpImg);
				readSubSize = fread(sub, sizeof(BYTE), sizeof(sub), fpSub);
				if (i < pDisc->SCSI.n1stLBAofLeadout || pDisc->SCSI.n1stLBAof2ndSession <= i) {
					fwrite(main, sizeof(BYTE), readMainSize, fpImg2);
					fwrite(sub, sizeof(BYTE), readSubSize, fpSub2);
				}
			}
			FcloseAndNull(fpImg);
			FcloseAndNull(fpSub);
			_tremove(pszOutImgFileTmp);
			_tremove(pszOutSubFileTmp);
			FcloseAndNull(fpImg2);
			FcloseAndNull(fpSub2);

			if (pDisc->SCSI.trkType != TRACK_TYPE::audioOnly) {
				_TCHAR pszOutScmFileTmp[_MAX_PATH] = {};
				if (NULL == (fpScm = CreateOrOpenFile(pszPath, NULL
					, pszOutScmFileTmp, NULL, NULL, _T(".scmtmp"), _T("rb"), 0, 0))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString("%s\n", pszOutScmFile);
					return FALSE;
				}
				FILE* fpScm2 = NULL;
				if (NULL == (fpScm2 = CreateOrOpenFile(pszPath, NULL
					, NULL, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					return FALSE;
				}
				BYTE mainScm[CD_RAW_SECTOR_SIZE] = {};
				size_t readMainScmSize = 0;
				for (INT i = 0; i < pDisc->SCSI.nAllLength; i++) {
					readMainScmSize = fread(mainScm, sizeof(BYTE), sizeof(mainScm), fpScm);
					if (i < pDisc->SCSI.n1stLBAofLeadout || pDisc->SCSI.n1stLBAof2ndSession <= i) {
						fwrite(mainScm, sizeof(BYTE), readMainScmSize, fpScm2);
					}
				}
				FcloseAndNull(fpScm);
				_tremove(pszOutScmFileTmp);
				FcloseAndNull(fpScm2);
			}
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpScm);
	FcloseAndNull(fpSub);
	if (pDevice->by0xF1Drive) {
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
	if (*pExecType == data || pExtArg->byBe) {
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
		SetReadDiscCommand(pExecType, pExtArg, pDevice, byTransferLen, c2, pDevice->sub, lpCmd, TRUE);
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
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
						WriteC2(pExtArg, pDisc, pDiscPerSector->data.next + pDevice->TRANSFER.uiBufC2Offset, nLBA, fpC2);
					}
					else {
						WriteC2(pExtArg, pDisc, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufC2Offset, nLBA, fpC2);
					}
				}
			}
			OutputString("\rCreating .scm from %d to %d (LBA) %6d"
				, nStart + pDisc->MAIN.nOffsetStart, nEnd + pDisc->MAIN.nOffsetEnd, nLBA);
			nLBA++;
			nFirstLBA++;
		}
		OutputString("\n");
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
				OutputCDMain(fileDisc, pDiscPerSector->mainHeader.current
					, pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->byTrackNum - 1], MAINHEADER_MODE1_SIZE);
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
	if (*pExecType == data || pExtArg->byBe || pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
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
		SetReadDiscCommand(pExecType, pExtArg, pDevice, byTransferLen, c2, pDevice->sub, lpCmd, TRUE);

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
					throw FALSE;
				}
				AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
				pDisc->SUB.lpEndCtlList[p] = (BYTE)((pDiscPerSector->subcode.current[12] >> 4) & 0x0f);
				OutputString("\rChecking SubQ ctl (Track) %2u/%2u", p + 1, pDisc->SCSI.toc.LastTrack);
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
		INT nFirstLBA = nStart + pDisc->MAIN.nOffsetStart - 1;
		if (pDisc->SCSI.trkType == audioOnly) {
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
				OutputLog(standardError | fileC2Error,
					" LBA[%06d, %#07x] Detected C2 error %u bit\n", nLBA, (UINT)nLBA, pDiscPerSector->uiC2errorNum);
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
					nLBA = pDisc->MAIN.nFixFirstLBAof2ndSession - 1;
				}
				nFirstLBA = nLBA;
			}
			else if (pDiscPerSector->bReturnCode == RETURNED_CONTINUE) {
				if (pDisc->PROTECT.byExist != physicalErr && (!bForceSkip || !bForceSkip2)) {
					if (pDisc->PROTECT.byExist == proring || pDisc->PROTECT.byExist == laserlock) {
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
						OutputLog(standardError | fileMainError, "Retry %d/10", nRetryCnt);
						nRetryCnt++;
						continue;
					}
					ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector
						, nLBA, nMainDataType, nPadType, fpBin, fpSub, fpC2);
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
					&& pExtArg->nC2RereadingType == 1 && nFirstLBA - pDisc->MAIN.nOffsetStart >= 0) {
					GetCrc32(&pDisc->MAIN.lpAllSectorCrc32[nFirstLBA - pDisc->MAIN.nOffsetStart]
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
									, nLBA, (UINT)nLBA);
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
							UpdateTmpSubch(pDiscPerSector);
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
				OutputString("\rCreating %s from %d to %d (LBA) %6d", szExt
					, nEnd + pDisc->MAIN.nOffsetEnd, nStart + pDisc->MAIN.nOffsetStart - 1, nLBA);
				nLBA--;
			}
			else {
				OutputString("\rCreating %s from %d to %d (LBA) %6d", szExt
					, nStart + pDisc->MAIN.nOffsetStart, nEnd + pDisc->MAIN.nOffsetEnd, nLBA);
				nLBA++;
			}
			nFirstLBA++;
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
			ExecEccEdc(pExtArg, pDisc->PROTECT, pszPath, pDisc->PROTECT.ERROR_SECTOR);
		}
		else if (*pExecType == gd) {
			_TCHAR pszImgPath[_MAX_PATH] = {};
			if (!DescrambleMainChannelForGD(pszPath, pszImgPath)) {
				throw FALSE;
			}
			ExecEccEdc(pExtArg, pDisc->PROTECT, pszImgPath, pDisc->PROTECT.ERROR_SECTOR);
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
