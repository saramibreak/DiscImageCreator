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
	if (NULL == ((*pDisc)->SCSI.lpFirstLBAListOnToc = 
		(LPINT)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (NULL == ((*pDisc)->SCSI.lpLastLBAListOnToc = 
		(LPINT)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
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
		(LPBYTE)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	(*pDisc)->SCSI.nFirstLBAofLeadout = -1;
	(*pDisc)->SCSI.nFirstLBAof2ndSession = -1;
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
		if (pDevice->FEATURE.byCanCDText || *pExecType == gd || *pExecType == swap) {
			if (NULL == ((*pDisc)->SUB.pszISRC = 
				(LPSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDisc)->SCSI.pszTitle = 
				(LPSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDisc)->SCSI.pszPerformer = 
				(LPSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDisc)->SCSI.pszSongWriter = 
				(LPSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}

			size_t isrcSize = META_ISRC_SIZE;
			size_t textSize = META_CDTEXT_SIZE;
			for (size_t h = 0; h < dwTrackAllocSize; h++) {
				if (NULL == ((*pDisc)->SUB.pszISRC[h] = 
					(LPSTR)calloc(isrcSize, sizeof(CHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDisc)->SCSI.pszTitle[h] = 
					(LPSTR)calloc(textSize, sizeof(CHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDisc)->SCSI.pszPerformer[h] = 
					(LPSTR)calloc(textSize, sizeof(CHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDisc)->SCSI.pszSongWriter[h] = 
					(LPSTR)calloc(textSize, sizeof(CHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
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
		if (NULL == ((*pDisc)->PROTECT.pNameForExe =
			(LPCH*)calloc(EXELBA_STORE_SIZE, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		for (size_t h = 0; h < EXELBA_STORE_SIZE; h++) {
			if (NULL == ((*pDisc)->PROTECT.pNameForExe[h] =
				(LPCH)calloc(MAX_FNAME_FOR_VOLUME, sizeof(CHAR)))) {
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
		if (NULL == ((*pDisc)->SUB.lpFirstLBAListOnSub =
			(LPINT*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lpFirstLBAListOnSubSync = 
			(LPINT*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lpFirstLBAListOfDataTrackOnSub = 
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

		size_t dwIndexAllocSize = (size_t)MAXIMUM_NUMBER_INDEXES * sizeof(UINT_PTR);
		for (size_t h = 0; h < dwTrackAllocSize; h++) {
			if (NULL == ((*pDisc)->SUB.lpFirstLBAListOnSub[h] = (LPINT)malloc(dwIndexAllocSize))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FillMemory((*pDisc)->SUB.lpFirstLBAListOnSub[h], dwIndexAllocSize, -1);
			if (NULL == ((*pDisc)->SUB.lpFirstLBAListOnSubSync[h] = (LPINT)malloc(dwIndexAllocSize))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FillMemory((*pDisc)->SUB.lpFirstLBAListOnSubSync[h], dwIndexAllocSize, -1);
			(*pDisc)->SUB.lpFirstLBAListOfDataTrackOnSub[h] = -1;
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
	CHAR path[_MAX_PATH] = { 0 };
#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, szFullPath, _MAX_PATH, path, sizeof(path), NULL, NULL);
#else
	strncpy(path, szFullPath, sizeof(path));
#endif
	CONST INT size = 32;
	CHAR szDiscLogtxt[size] = { 0 };
	CHAR szDriveLogtxt[size] = { 0 };
	CHAR szVolDescLogtxt[size] = { 0 };
	CHAR szMainInfoLogtxt[size] = { 0 };
	CHAR szMainErrorLogtxt[size] = { 0 };
	CHAR szSubInfoLogtxt[size] = { 0 };
	CHAR szSubErrorLogtxt[size] = { 0 };
	CHAR szC2ErrorLogtxt[size] = { 0 };

	strncpy(szDiscLogtxt, "_disc", size);
	strncpy(szDriveLogtxt, "_drive", size);
	strncpy(szVolDescLogtxt, "_volDesc", size);
	strncpy(szMainInfoLogtxt, "_mainInfo", size);
	strncpy(szMainErrorLogtxt, "_mainError", size);
	strncpy(szSubInfoLogtxt, "_subInfo", size);
	strncpy(szSubErrorLogtxt, "_subError", size);
	strncpy(szC2ErrorLogtxt, "_c2Error", size);
		
	if (NULL == (g_LogFile.fpDisc = CreateOrOpenFileA(
		path, szDiscLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	try {
		if (*pExecType != fd) {
			if (NULL == (g_LogFile.fpDrive = CreateOrOpenFileA(
				path, szDriveLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (g_LogFile.fpVolDesc = CreateOrOpenFileA(
				path, szVolDescLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (g_LogFile.fpMainInfo = CreateOrOpenFileA(
				path, szMainInfoLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (g_LogFile.fpMainError = CreateOrOpenFileA(
				path, szMainErrorLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (*pExecType != dvd && *pExecType != bd) {
				if (NULL == (g_LogFile.fpSubInfo = CreateOrOpenFileA(
					path, szSubInfoLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == (g_LogFile.fpSubError = CreateOrOpenFileA(
					path, szSubErrorLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (pExtArg->byC2) {
					if (NULL == (g_LogFile.fpC2Error = CreateOrOpenFileA(
						path, szC2ErrorLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0))) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
				}
				if (pExtArg->byIntentionalSub || pExtArg->byLibCrypt) {
					if (NULL == (g_LogFile.fpSubIntention = CreateOrOpenFileA(
						path, "_subIntention", NULL, NULL, NULL, ".txt", "w", 0, 0))) {
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
	FreeAndNull((*pDisc)->SCSI.lpFirstLBAListOnToc);
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
	if (pDevice->FEATURE.byCanCDText) {
		for (size_t i = 0; i < dwTrackAllocSize; i++) {
			FreeAndNull((*pDisc)->SUB.pszISRC[i]);
			FreeAndNull((*pDisc)->SCSI.pszTitle[i]);
			FreeAndNull((*pDisc)->SCSI.pszPerformer[i]);
			FreeAndNull((*pDisc)->SCSI.pszSongWriter[i]);
		}
		FreeAndNull((*pDisc)->SUB.pszISRC);
		FreeAndNull((*pDisc)->SCSI.pszTitle);
		FreeAndNull((*pDisc)->SCSI.pszPerformer);
		FreeAndNull((*pDisc)->SCSI.pszSongWriter);
	}
}

VOID TerminateProtectData(
	PDISC* pDisc
) {
	for (size_t h = 0; h < EXELBA_STORE_SIZE; h++) {
		if ((*pDisc)->PROTECT.pNameForExe) {
			FreeAndNull((*pDisc)->PROTECT.pNameForExe[h]);
		}
	}
	FreeAndNull((*pDisc)->PROTECT.pNameForExe);
	FreeAndNull((*pDisc)->PROTECT.pExtentPosForExe);
}

VOID TerminateSubData(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
) {
	size_t dwTrackAllocSize =
		(*pExecType == gd || *pExecType == swap) ? MAXIMUM_NUMBER_TRACKS : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	for (size_t h = 0; h < dwTrackAllocSize; h++) {
		if ((*pDisc)->SUB.lpFirstLBAListOnSub) {
			FreeAndNull((*pDisc)->SUB.lpFirstLBAListOnSub[h]);
		}
		if ((*pDisc)->SUB.lpFirstLBAListOnSubSync) {
			FreeAndNull((*pDisc)->SUB.lpFirstLBAListOnSubSync[h]);
		}
	}
	FreeAndNull((*pDisc)->SUB.lpRtoWList);
	FreeAndNull((*pDisc)->SUB.lpFirstLBAListOnSub);
	FreeAndNull((*pDisc)->SUB.lpFirstLBAListOnSubSync);
	FreeAndNull((*pDisc)->SUB.lpFirstLBAListOfDataTrackOnSub);
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
	if (*pExecType != fd) {
		FcloseAndNull(g_LogFile.fpDrive);
		FcloseAndNull(g_LogFile.fpVolDesc);
		FcloseAndNull(g_LogFile.fpMainInfo);
		FcloseAndNull(g_LogFile.fpMainError);
		if (*pExecType != dvd && *pExecType != bd) {
			FcloseAndNull(g_LogFile.fpSubInfo);
			FcloseAndNull(g_LogFile.fpSubError);
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
