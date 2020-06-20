/**
 * Copyright 2011-2020 sarami
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
#include "init.h"
#include "output.h"

// These global variable is set at DiscImageCreator.cpp
extern BYTE g_aSyncHeader[SYNC_SIZE];

BOOL InitC2(
	PDISC* pDisc
) {
	size_t dwAllocSize = (size_t)((*pDisc)->SCSI.nAllLength + FIRST_TRACK_PREGAP_SIZE + LAST_TRACK_LEADOUT_SIZE);
	if (NULL == ((*pDisc)->MAIN.lpAllSectorCrc32 = (LPDWORD)calloc(dwAllocSize, sizeof(DWORD)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (NULL == ((*pDisc)->MAIN.lpAllLBAOfC2Error = (LPINT)calloc(dwAllocSize, sizeof(INT)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	return TRUE;
}

BOOL InitLBAPerTrack(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
) {
	size_t dwTrackAllocSize =
		(*pExecType == gd || *pExecType == swap) ? MAXIMUM_NUMBER_TRACKS : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	if (NULL == (*pDisc)->SCSI.lp1stLBAListOnToc) {
		if (NULL == ((*pDisc)->SCSI.lp1stLBAListOnToc =
			(LPINT)calloc(dwTrackAllocSize, sizeof(INT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	if (NULL == (*pDisc)->SCSI.lpLastLBAListOnToc) {
		if (NULL == ((*pDisc)->SCSI.lpLastLBAListOnToc =
			(LPINT)calloc(dwTrackAllocSize, sizeof(INT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL InitTocFullData(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
) {
	size_t dwTrackAllocSize =
		(*pExecType == gd || *pExecType == swap) ? MAXIMUM_NUMBER_TRACKS : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	if (NULL == ((*pDisc)->SCSI.lpSessionNumList =
		(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	(*pDisc)->SCSI.n1stLBAofLeadout = -1;
	// init this by ReadTOCFull
//	(*pDisc)->SCSI.n1stLBAof2ndSession = -1;
	return TRUE;
}

BOOL InitTocTextData(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC* pDisc
) {
	BOOL bRet = TRUE;
	size_t dwTrackAllocSize =
		(*pExecType == gd || *pExecType == swap) ? MAXIMUM_NUMBER_TRACKS : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	try {
		if (NULL == ((*pDisc)->SUB.pszISRC =
			(LPSTR*)calloc(dwTrackAllocSize * 2, sizeof(LPSTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		for (size_t h = 0; h < dwTrackAllocSize; h++) {
			if (NULL == ((*pDisc)->SUB.pszISRC[h] =
				(LPSTR)calloc(META_ISRC_SIZE, sizeof(CHAR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}
		if (pDevice->FEATURE.byCanCDText || *pExecType == gd || *pExecType == swap) {
			for (INT i = 0; i < MAX_CDTEXT_LANG; i++) {
				if (NULL == ((*pDisc)->SCSI.CDTEXT[i].pszTitle =
					(LPSTR*)calloc(dwTrackAllocSize * 2, sizeof(LPSTR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDisc)->SCSI.CDTEXT[i].pszPerformer =
					(LPSTR*)calloc(dwTrackAllocSize * 2, sizeof(LPSTR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDisc)->SCSI.CDTEXT[i].pszSongWriter =
					(LPSTR*)calloc(dwTrackAllocSize * 2, sizeof(LPSTR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
			}
			for (INT i = 0; i < MAX_CDTEXT_LANG; i++) {
				for (size_t h = 0; h < dwTrackAllocSize; h++) {
					if (NULL == ((*pDisc)->SCSI.CDTEXT[i].pszTitle[h] =
						(LPSTR)calloc(META_CDTEXT_SIZE, sizeof(CHAR)))) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
					if (NULL == ((*pDisc)->SCSI.CDTEXT[i].pszPerformer[h] =
						(LPSTR)calloc(META_CDTEXT_SIZE, sizeof(CHAR)))) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
					if (NULL == ((*pDisc)->SCSI.CDTEXT[i].pszSongWriter[h] =
						(LPSTR)calloc(META_CDTEXT_SIZE, sizeof(CHAR)))) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
				}
			}
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	return bRet;
}

VOID InitMainDataHeader(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PMAIN_HEADER pMain,
	INT nLBA
) {
	memcpy(pMain->current, g_aSyncHeader, sizeof(g_aSyncHeader));
	BYTE m, s, f;
	LBAtoMSF(nLBA + 150, &m, &s, &f);
	if (!pExtArg->byBe && *pExecType != data) {
		pMain->current[12] = (BYTE)(DecToBcd(m) ^ 0x01);
		pMain->current[13] = (BYTE)(DecToBcd(s) ^ 0x80);
	}
	else {
		pMain->current[12] = (BYTE)(DecToBcd(m));
		pMain->current[13] = (BYTE)(DecToBcd(s));
	}
	pMain->current[14] = (BYTE)(DecToBcd(f) - 1);
}

BOOL InitProtectData(
	PDISC* pDisc
) {
	BOOL bRet = TRUE;
	try {
		if (NULL == ((*pDisc)->PROTECT.pExtentPosForExe =
			(LPINT)calloc(EXELBA_STORE_SIZE, sizeof(INT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->PROTECT.pDataLenForExe =
			(LPINT)calloc(EXELBA_STORE_SIZE, sizeof(INT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->PROTECT.pSectorSizeForExe =
			(LPINT)calloc(EXELBA_STORE_SIZE, sizeof(INT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->PROTECT.pNameForExe =
			(LPCH*)calloc(EXELBA_STORE_SIZE, sizeof(LPCH)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->PROTECT.pFullNameForExe =
			(LPCH*)calloc(EXELBA_STORE_SIZE, sizeof(LPCH)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		for (size_t h = 0; h < EXELBA_STORE_SIZE; h++) {
			if (NULL == ((*pDisc)->PROTECT.pNameForExe[h] =
				(LPCH)calloc(MAX_FNAME_FOR_VOLUME, sizeof(CHAR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDisc)->PROTECT.pFullNameForExe[h] =
				(LPCH)calloc(_MAX_PATH, sizeof(CHAR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	return bRet;
}

BOOL InitSubData(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
) {
	BOOL bRet = TRUE;
	size_t dwTrackAllocSize =
		(*pExecType == gd || *pExecType == swap) ? MAXIMUM_NUMBER_TRACKS : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	try {
		if (NULL == ((*pDisc)->SUB.lpRtoWList =
			(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lp1stLBAListOnSub =
			(LPINT*)calloc(dwTrackAllocSize * 2, sizeof(LPINT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lp1stLBAListOnSubSync = 
			(LPINT*)calloc(dwTrackAllocSize * 2, sizeof(LPINT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lp1stLBAListOfDataTrackOnSub = 
			(LPINT)calloc(dwTrackAllocSize, sizeof(INT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lpLastLBAListOfDataTrackOnSub = 
			(LPINT)calloc(dwTrackAllocSize, sizeof(INT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lpISRCList = 
			(LPBOOL)calloc(dwTrackAllocSize, sizeof(BOOL)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lpCtlList = 
			(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->MAIN.lpModeList = 
			(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lpEndCtlList = 
			(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		size_t dwIndexAllocSize = (size_t)MAXIMUM_NUMBER_INDEXES * sizeof(INT);
		for (size_t h = 0; h < dwTrackAllocSize; h++) {
			if (NULL == ((*pDisc)->SUB.lp1stLBAListOnSub[h] = (LPINT)malloc(dwIndexAllocSize))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FillMemory((*pDisc)->SUB.lp1stLBAListOnSub[h], dwIndexAllocSize, -1);
#if 0
			for (INT j = 0; j < 2; j++) {
				OutputString("lp1stLBAListOnSub[%zd][%zd]: %d, %p\n")
					, h, j, (*pDisc)->SUB.lp1stLBAListOnSub[h][j], &(*pDisc)->SUB.lp1stLBAListOnSub[h][j]);
			}
#endif
			if (NULL == ((*pDisc)->SUB.lp1stLBAListOnSubSync[h] = (LPINT)malloc(dwIndexAllocSize))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FillMemory((*pDisc)->SUB.lp1stLBAListOnSubSync[h], dwIndexAllocSize, -1);
			(*pDisc)->SUB.lp1stLBAListOfDataTrackOnSub[h] = -1;
			(*pDisc)->SUB.lpLastLBAListOfDataTrackOnSub[h] = -1;
		}
		(*pDisc)->SUB.byCatalog = FALSE;
		for (INT i = 0; i < 3; i++) {
			for (INT j = 0; j < 2; j++) {
				(*pDisc)->SUB.nFirstLBAForMCN[i][j] = -1;
				(*pDisc)->SUB.nRangeLBAForMCN[i][j] = -1;
				(*pDisc)->SUB.nFirstLBAForISRC[i][j] = -1;
				(*pDisc)->SUB.nRangeLBAForISRC[i][j] = -1;
			}
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	(*pDisc)->SUB.nSubChannelOffset = 0xff;
	return bRet;
}

#ifndef _DEBUG
LOG_FILE g_LogFile;

BOOL InitLogFile(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	_TCHAR* szFullPath
) {
	if (NULL == (g_LogFile.fpDisc = CreateOrOpenFile(
		szFullPath, _T("_disc"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (setvbuf(g_LogFile.fpDisc, NULL, _IONBF, 0) != 0) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	BOOL bRet = TRUE;
	try {
		if (NULL == (g_LogFile.fpDrive = CreateOrOpenFile(
			szFullPath, _T("_drive"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (setvbuf(g_LogFile.fpDrive, NULL, _IONBF, 0) != 0) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		}
		if (NULL == (g_LogFile.fpMainError = CreateOrOpenFile(
			szFullPath, _T("_mainError"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (setvbuf(g_LogFile.fpMainError, NULL, _IONBF, 0) != 0) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		}
		if (NULL == (g_LogFile.fpVolDesc = CreateOrOpenFile(
			szFullPath, _T("_volDesc"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (setvbuf(g_LogFile.fpVolDesc, NULL, _IONBF, 0) != 0) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		}
		if (*pExecType != fd && * pExecType != disk) {
			if (NULL == (g_LogFile.fpMainInfo = CreateOrOpenFile(
				szFullPath, _T("_mainInfo"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (setvbuf(g_LogFile.fpMainInfo, NULL, _IONBF, 0) != 0) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			}
			if (pExtArg->byRawDump) {
				if (NULL == (g_LogFile.fpRawReadable = CreateOrOpenFile(
					szFullPath, _T("_rawReadable"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (setvbuf(g_LogFile.fpRawReadable, NULL, _IONBF, 0) != 0) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				}
			}
			if (*pExecType != dvd && *pExecType != bd && *pExecType != sacd &&
				*pExecType != xbox && *pExecType != xboxswap &&
				*pExecType != xgd2swap && *pExecType != xgd3swap) {
				if (NULL == (g_LogFile.fpSubInfo = CreateOrOpenFile(
					szFullPath, _T("_subInfo"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (setvbuf(g_LogFile.fpSubInfo, NULL, _IONBF, 0) != 0) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				}
				if (NULL == (g_LogFile.fpSubError = CreateOrOpenFile(
					szFullPath, _T("_subError"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == (g_LogFile.fpSubReadable = CreateOrOpenFile(
					szFullPath, _T("_subReadable"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (pExtArg->byC2) {
					if (NULL == (g_LogFile.fpC2Error = CreateOrOpenFile(
						szFullPath, _T("_c2Error"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
				}
				if (pExtArg->byIntentionalSub || pExtArg->byLibCrypt) {
					if (NULL == (g_LogFile.fpSubIntention = CreateOrOpenFile(
						szFullPath, _T("_subIntention"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
					if (setvbuf(g_LogFile.fpSubIntention, NULL, _IONBF, 0) != 0) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					}
				}
			}
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	return bRet;
}
#endif

VOID TerminateC2(
	PDISC* pDisc
) {
	FreeAndNull((*pDisc)->MAIN.lpAllSectorCrc32);
	FreeAndNull((*pDisc)->MAIN.lpAllLBAOfC2Error);
}

VOID TerminateLBAPerTrack(
	PDISC* pDisc
) {
	FreeAndNull((*pDisc)->SCSI.lp1stLBAListOnToc);
	FreeAndNull((*pDisc)->SCSI.lpLastLBAListOnToc);
}

VOID TerminateTocFullData(
	PDISC* pDisc
) {
	FreeAndNull((*pDisc)->SCSI.lpSessionNumList);
}

VOID TerminateTocTextData(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC* pDisc
) {
	size_t dwTrackAllocSize =
		(*pExecType == gd || *pExecType == swap) ? MAXIMUM_NUMBER_TRACKS : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	if ((*pDisc)->SUB.pszISRC) {
		for (size_t i = 0; i < dwTrackAllocSize; i++) {
			FreeAndNull((*pDisc)->SUB.pszISRC[i]);
		}
	}
	FreeAndNull((*pDisc)->SUB.pszISRC);
	if (pDevice->FEATURE.byCanCDText || *pExecType == gd || *pExecType == swap) {
		for (INT j = 0; j < MAX_CDTEXT_LANG; j++) {
			if ((*pDisc)->SCSI.CDTEXT[j].pszTitle &&
				(*pDisc)->SCSI.CDTEXT[j].pszPerformer &&
				(*pDisc)->SCSI.CDTEXT[j].pszSongWriter) {
				for (size_t i = 0; i < dwTrackAllocSize; i++) {
					FreeAndNull((*pDisc)->SCSI.CDTEXT[j].pszTitle[i]);
					FreeAndNull((*pDisc)->SCSI.CDTEXT[j].pszPerformer[i]);
					FreeAndNull((*pDisc)->SCSI.CDTEXT[j].pszSongWriter[i]);
				}
			}
		}
		for (INT j = 0; j < MAX_CDTEXT_LANG; j++) {
			FreeAndNull((*pDisc)->SCSI.CDTEXT[j].pszTitle);
			FreeAndNull((*pDisc)->SCSI.CDTEXT[j].pszPerformer);
			FreeAndNull((*pDisc)->SCSI.CDTEXT[j].pszSongWriter);
		}
	}
}

VOID TerminateProtectData(
	PDISC* pDisc
) {
	if ((*pDisc)->PROTECT.pNameForExe && (*pDisc)->PROTECT.pFullNameForExe) {
		for (size_t h = 0; h < EXELBA_STORE_SIZE; h++) {
			FreeAndNull((*pDisc)->PROTECT.pNameForExe[h]);
			FreeAndNull((*pDisc)->PROTECT.pFullNameForExe[h]);
		}
	}
	FreeAndNull((*pDisc)->PROTECT.pNameForExe);
	FreeAndNull((*pDisc)->PROTECT.pFullNameForExe);
	FreeAndNull((*pDisc)->PROTECT.pExtentPosForExe);
	FreeAndNull((*pDisc)->PROTECT.pDataLenForExe);
	FreeAndNull((*pDisc)->PROTECT.pSectorSizeForExe);
}

VOID TerminateSubData(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
) {
	size_t dwTrackAllocSize =
		(*pExecType == gd || *pExecType == swap) ? MAXIMUM_NUMBER_TRACKS : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	if ((*pDisc)->SUB.lp1stLBAListOnSub && (*pDisc)->SUB.lp1stLBAListOnSubSync) {
		for (size_t h = 0; h < dwTrackAllocSize; h++) {
			FreeAndNull((*pDisc)->SUB.lp1stLBAListOnSub[h]);
			FreeAndNull((*pDisc)->SUB.lp1stLBAListOnSubSync[h]);
		}
	}
	FreeAndNull((*pDisc)->SUB.lpRtoWList);
	FreeAndNull((*pDisc)->SUB.lp1stLBAListOnSub);
	FreeAndNull((*pDisc)->SUB.lp1stLBAListOnSubSync);
	FreeAndNull((*pDisc)->SUB.lp1stLBAListOfDataTrackOnSub);
	FreeAndNull((*pDisc)->SUB.lpLastLBAListOfDataTrackOnSub);
	FreeAndNull((*pDisc)->SUB.lpCtlList);
	FreeAndNull((*pDisc)->SUB.lpEndCtlList);
	FreeAndNull((*pDisc)->SUB.lpISRCList);
	FreeAndNull((*pDisc)->MAIN.lpModeList);
}

#ifndef _DEBUG
VOID TerminateLogFile(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg
) {
	FcloseAndNull(g_LogFile.fpDisc);
	FcloseAndNull(g_LogFile.fpDrive);
	FcloseAndNull(g_LogFile.fpMainError);
	FcloseAndNull(g_LogFile.fpVolDesc);
	if (*pExecType != fd && *pExecType != disk) {
		FcloseAndNull(g_LogFile.fpMainInfo);
		if (pExtArg->byRawDump) {
			FcloseAndNull(g_LogFile.fpRawReadable);
		}
		if (*pExecType != dvd && *pExecType != bd &&
			*pExecType != xbox && *pExecType != xboxswap &&
			*pExecType != xgd2swap && *pExecType != xgd3swap) {
			FcloseAndNull(g_LogFile.fpSubInfo);
			FcloseAndNull(g_LogFile.fpSubError);
			FcloseAndNull(g_LogFile.fpSubReadable);
			if (pExtArg->byC2) {
				FcloseAndNull(g_LogFile.fpC2Error);
			}
			if (pExtArg->byIntentionalSub || pExtArg->byLibCrypt) {
				FcloseAndNull(g_LogFile.fpSubIntention);
			}
		}
	}
}
#endif
