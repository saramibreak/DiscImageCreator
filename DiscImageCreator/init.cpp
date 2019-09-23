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
	if (NULL == (*pDisc)->SCSI.lpFirstLBAListOnToc) {
		if (NULL == ((*pDisc)->SCSI.lpFirstLBAListOnToc =
			(LPINT)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	if (NULL == (*pDisc)->SCSI.lpLastLBAListOnToc) {
		if (NULL == ((*pDisc)->SCSI.lpLastLBAListOnToc =
			(LPINT)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
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
		(LPBYTE)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	(*pDisc)->SCSI.nFirstLBAofLeadout = -1;
	// init this by ReadTOCFull
//	(*pDisc)->SCSI.nFirstLBAof2ndSession = -1;
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
				(LPSTR*)calloc(dwTrackAllocSize * 2, sizeof(INT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDisc)->SCSI.pszTitle = 
				(LPSTR*)calloc(dwTrackAllocSize * 2, sizeof(INT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDisc)->SCSI.pszPerformer = 
				(LPSTR*)calloc(dwTrackAllocSize * 2, sizeof(INT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDisc)->SCSI.pszSongWriter = 
				(LPSTR*)calloc(dwTrackAllocSize * 2, sizeof(INT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDisc)->SCSI.pszTitleW =
				(LPSTR*)calloc(dwTrackAllocSize * 2, sizeof(INT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDisc)->SCSI.pszPerformerW =
				(LPSTR*)calloc(dwTrackAllocSize * 2, sizeof(INT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDisc)->SCSI.pszSongWriterW =
				(LPSTR*)calloc(dwTrackAllocSize * 2, sizeof(INT_PTR)))) {
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
				if (NULL == ((*pDisc)->SCSI.pszTitleW[h] =
					(LPSTR)calloc(textSize, sizeof(CHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDisc)->SCSI.pszPerformerW[h] =
					(LPSTR)calloc(textSize, sizeof(CHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDisc)->SCSI.pszSongWriterW[h] =
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
			(LPCH*)calloc(EXELBA_STORE_SIZE, sizeof(INT_PTR)))) {
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
			(LPINT*)calloc(dwTrackAllocSize * 2, sizeof(INT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lpFirstLBAListOnSubSync = 
			(LPINT*)calloc(dwTrackAllocSize * 2, sizeof(INT_PTR)))) {
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

		size_t dwIndexAllocSize = (size_t)MAXIMUM_NUMBER_INDEXES * sizeof(INT);
		for (size_t h = 0; h < dwTrackAllocSize; h++) {
			if (NULL == ((*pDisc)->SUB.lpFirstLBAListOnSub[h] = (LPINT)malloc(dwIndexAllocSize))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FillMemory((*pDisc)->SUB.lpFirstLBAListOnSub[h], dwIndexAllocSize, -1);
#if 0
			for (INT j = 0; j < 2; j++) {
				OutputString(_T("lpFirstLBAListOnSub[%zd][%zd]: %d, %p\n")
					, h, j, (*pDisc)->SUB.lpFirstLBAListOnSub[h][j], &(*pDisc)->SUB.lpFirstLBAListOnSub[h][j]);
			}
#endif
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
	CHAR path[_MAX_PATH] = {};
#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, szFullPath, _MAX_PATH, path, sizeof(path), NULL, NULL);
#else
	strncpy(path, szFullPath, sizeof(path));
#endif
	if (NULL == (g_LogFile.fpDisc = CreateOrOpenFileA(
		path, "_disc", NULL, NULL, NULL, ".txt", "w", 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (setvbuf(g_LogFile.fpDisc, NULL, _IONBF, 0) != 0) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	BOOL bRet = TRUE;
	try {
		if (NULL == (g_LogFile.fpDrive = CreateOrOpenFileA(
			path, "_drive", NULL, NULL, NULL, ".txt", "w", 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (setvbuf(g_LogFile.fpDrive, NULL, _IONBF, 0) != 0) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		}
		if (*pExecType != fd && * pExecType != disk) {
			if (NULL == (g_LogFile.fpVolDesc = CreateOrOpenFileA(
				path, "_volDesc", NULL, NULL, NULL, ".txt", "w", 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (setvbuf(g_LogFile.fpVolDesc, NULL, _IONBF, 0) != 0) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			}
			if (NULL == (g_LogFile.fpMainInfo = CreateOrOpenFileA(
				path, "_mainInfo", NULL, NULL, NULL, ".txt", "w", 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (setvbuf(g_LogFile.fpMainInfo, NULL, _IONBF, 0) != 0) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			}
			if (NULL == (g_LogFile.fpMainError = CreateOrOpenFileA(
				path, "_mainError", NULL, NULL, NULL, ".txt", "w", 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (setvbuf(g_LogFile.fpMainError, NULL, _IONBF, 0) != 0) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			}
			if (*pExecType != dvd && *pExecType != bd && *pExecType != sacd &&
				*pExecType != xbox && *pExecType != xboxswap &&
				*pExecType != xgd2swap && *pExecType != xgd3swap) {
				if (NULL == (g_LogFile.fpSubInfo = CreateOrOpenFileA(
					path, "_subInfo", NULL, NULL, NULL, ".txt", "w", 0, 0))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (setvbuf(g_LogFile.fpSubInfo, NULL, _IONBF, 0) != 0) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				}
				if (NULL == (g_LogFile.fpSubError = CreateOrOpenFileA(
					path, "_subError", NULL, NULL, NULL, ".txt", "w", 0, 0))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == (g_LogFile.fpSubReadable = CreateOrOpenFileA(
					path, "_subReadable", NULL, NULL, NULL, ".txt", "w", 0, 0))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (pExtArg->byC2) {
					if (NULL == (g_LogFile.fpC2Error = CreateOrOpenFileA(
						path, "_c2Error", NULL, NULL, NULL, ".txt", "w", 0, 0))) {
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
			FreeAndNull((*pDisc)->SCSI.pszTitleW[i]);
			FreeAndNull((*pDisc)->SCSI.pszPerformerW[i]);
			FreeAndNull((*pDisc)->SCSI.pszSongWriterW[i]);
		}
		FreeAndNull((*pDisc)->SUB.pszISRC);
		FreeAndNull((*pDisc)->SCSI.pszTitle);
		FreeAndNull((*pDisc)->SCSI.pszPerformer);
		FreeAndNull((*pDisc)->SCSI.pszSongWriter);
		FreeAndNull((*pDisc)->SCSI.pszTitleW);
		FreeAndNull((*pDisc)->SCSI.pszPerformerW);
		FreeAndNull((*pDisc)->SCSI.pszSongWriterW);
	}
}

VOID TerminateProtectData(
	PDISC* pDisc
) {
	for (size_t h = 0; h < EXELBA_STORE_SIZE; h++) {
		FreeAndNull((*pDisc)->PROTECT.pNameForExe[h]);
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
	if ((*pDisc)->SUB.lpFirstLBAListOnSub && (*pDisc)->SUB.lpFirstLBAListOnSubSync) {
		for (size_t h = 0; h < dwTrackAllocSize; h++) {
			FreeAndNull((*pDisc)->SUB.lpFirstLBAListOnSub[h]);
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
	FcloseAndNull(g_LogFile.fpDrive);
	if (*pExecType != fd && *pExecType != disk) {
		FcloseAndNull(g_LogFile.fpVolDesc);
		FcloseAndNull(g_LogFile.fpMainInfo);
		FcloseAndNull(g_LogFile.fpMainError);
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
