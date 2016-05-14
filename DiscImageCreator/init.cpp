/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "init.h"
#include "output.h"

// These global variable is set at DiscImageCreator.cpp
extern BYTE g_aSyncHeader[SYNC_SIZE];

BOOL InitC2ErrorData(
	PEXT_ARG pExtArg,
	PC2_ERROR_PER_SECTOR* pC2ErrorPerSector,
	DWORD dwAllBufLen
	)
{
	BOOL bRet = TRUE;
	if (NULL == (*pC2ErrorPerSector = (PC2_ERROR_PER_SECTOR)
		calloc(pExtArg->dwMaxC2ErrorNum, sizeof(C2_ERROR_PER_SECTOR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	try {
		for (DWORD n = 0; n < pExtArg->dwMaxC2ErrorNum; n++) {
			OutputString(_T("\rAllocating memory for C2 errors: %lu/%lu"), 
				n + 1, pExtArg->dwMaxC2ErrorNum);
			if (NULL == ((*pC2ErrorPerSector)[n].lpErrorBytePos = 
				(PSHORT)calloc(CD_RAW_SECTOR_SIZE, sizeof(WORD)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			if (NULL == ((*pC2ErrorPerSector)[n].lpErrorBytePosBackup = 
				(PSHORT)calloc(CD_RAW_SECTOR_SIZE, sizeof(WORD)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			if (NULL == ((*pC2ErrorPerSector)[n].lpBufNoC2Sector = 
				(LPBYTE)calloc(dwAllBufLen, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			if (NULL == ((*pC2ErrorPerSector)[n].lpBufNoC2SectorBackup = 
				(LPBYTE)calloc(dwAllBufLen, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			(*pC2ErrorPerSector)[n].byErrorFlag = RETURNED_NO_C2_ERROR_1ST;
		}
		OutputString(_T("\n"));
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	return bRet;
}

BOOL InitLBAPerTrack(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
	)
{
	size_t dwTrackAllocSize = 
		*pExecType == gd ? 100 : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
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
	)
{
	size_t dwTrackAllocSize =
		*pExecType == gd ? 100 : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	if (NULL == ((*pDisc)->SCSI.lpSessionNumList =
		(LPBYTE)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	return TRUE;
}

BOOL InitTocTextData(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC* pDisc
	)
{
	BOOL bRet = TRUE;
	size_t dwTrackAllocSize =
		*pExecType == gd ? 100 : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	try {
		if (pDevice->FEATURE.byCanCDText || *pExecType == gd) {
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
	PMAIN_HEADER pMain
	)
{
	memcpy(pMain->present, g_aSyncHeader, sizeof(g_aSyncHeader));
	if (!pExtArg->byBe && *pExecType != data) {
		pMain->present[12] = 0x01;
		pMain->present[13] = 0x82;
	}
	else {
		pMain->present[13] = 2;
	}
	pMain->present[14] = (BYTE)-1;
}

BOOL InitProtectData(
	PDISC* pDisc
	)
{
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
	)
{
	BOOL bRet = TRUE;
	size_t dwTrackAllocSize =
		*pExecType == gd ? 100 : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
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
	return bRet;
}

#ifndef _DEBUG
LOG_FILE g_LogFile;

BOOL InitLogFile(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	_TCHAR* szFullPath
	)
{
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

	if (*pExecType == fd) {
		strncpy(szDiscLogtxt, "_disc_fd", size);
	}
	else if (*pExecType == dvd) {
		strncpy(szDiscLogtxt, "_disc_dvd", size);
		strncpy(szDriveLogtxt, "_drive_dvd", size);
		strncpy(szVolDescLogtxt, "_volDesc_dvd", size);
		strncpy(szMainErrorLogtxt, "_mainError", size);
	}
	else if (*pExecType == gd) {
		strncpy(szDiscLogtxt, "_disc_gd", size);
		strncpy(szDriveLogtxt, "_drive_gd", size);
		strncpy(szVolDescLogtxt, "_volDesc_gd", size);
		strncpy(szMainInfoLogtxt, "_mainInfo_gd", size);
		strncpy(szMainErrorLogtxt, "_mainError_gd", size);
		strncpy(szSubErrorLogtxt, "_subError_gd", size);
		strncpy(szC2ErrorLogtxt, "_c2error_gd", size);
	}
	else {
		strncpy(szDiscLogtxt, "_disc", size);
		strncpy(szDriveLogtxt, "_drive", size);
		strncpy(szVolDescLogtxt, "_volDesc", size);
		strncpy(szMainInfoLogtxt, "_mainInfo", size);
		strncpy(szMainErrorLogtxt, "_mainError", size);
		strncpy(szSubInfoLogtxt, "_subInfo", size);
		strncpy(szSubErrorLogtxt, "_subError", size);
		strncpy(szC2ErrorLogtxt, "_c2error", size);
	}

	g_LogFile.fpDisc = CreateOrOpenFileA(
		path, szDiscLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0);
	if (!g_LogFile.fpDisc) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	try {
		if (*pExecType != fd) {
			g_LogFile.fpDrive = CreateOrOpenFileA(
				path, szDriveLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0);
			if (!g_LogFile.fpDrive) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			g_LogFile.fpVolDesc = CreateOrOpenFileA(
				path, szVolDescLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0);
			if (!g_LogFile.fpVolDesc) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			g_LogFile.fpMainError = CreateOrOpenFileA(
				path, szMainErrorLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0);
			if (!g_LogFile.fpMainError) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (*pExecType != dvd) {
				g_LogFile.fpMainInfo = CreateOrOpenFileA(
					path, szMainInfoLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0);
				if (!g_LogFile.fpMainInfo) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				g_LogFile.fpSubInfo = CreateOrOpenFileA(
					path, szSubInfoLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0);
				if (!g_LogFile.fpSubInfo) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				g_LogFile.fpSubError = CreateOrOpenFileA(
					path, szSubErrorLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0);
				if (!g_LogFile.fpSubError) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (pExtArg->byC2) {
					g_LogFile.fpC2Error = CreateOrOpenFileA(
						path, szC2ErrorLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0);
					if (!g_LogFile.fpC2Error) {
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

VOID TerminateC2ErrorData(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PC2_ERROR_PER_SECTOR* pC2ErrorPerSector
	)
{
	if (*pC2ErrorPerSector && pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		for (DWORD i = 0; i < pExtArg->dwMaxC2ErrorNum; i++) {
			OutputString(_T("\rFreeing allocated memory for C2 errors: %lu/%lu"), 
				i + 1, pExtArg->dwMaxC2ErrorNum);
			FreeAndNull((*pC2ErrorPerSector)[i].lpErrorBytePos);
			FreeAndNull((*pC2ErrorPerSector)[i].lpErrorBytePosBackup);
			FreeAndNull((*pC2ErrorPerSector)[i].lpBufNoC2Sector);
			FreeAndNull((*pC2ErrorPerSector)[i].lpBufNoC2SectorBackup);
		}
		OutputString(_T("\n"));
		FreeAndNull(*pC2ErrorPerSector);
	}
}

VOID TerminateLBAPerTrack(
	PDISC* pDisc
	)
{
	FreeAndNull((*pDisc)->SCSI.lpFirstLBAListOnToc);
	FreeAndNull((*pDisc)->SCSI.lpLastLBAListOnToc);
}

VOID TerminateTocFullData(
	PDISC* pDisc
	)
{
	FreeAndNull((*pDisc)->SCSI.lpSessionNumList);
}

VOID TerminateTocTextData(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC* pDisc
	)
{
	size_t dwTrackAllocSize =
		*pExecType == gd ? 100 : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
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
	)
{
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
	)
{
	size_t dwTrackAllocSize =
		*pExecType == gd ? 100 : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
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
	)
{
	FcloseAndNull(g_LogFile.fpDisc);
	if (*pExecType != fd) {
		FcloseAndNull(g_LogFile.fpDrive);
		FcloseAndNull(g_LogFile.fpVolDesc);
		if (*pExecType != dvd) {
			FcloseAndNull(g_LogFile.fpMainInfo);
			FcloseAndNull(g_LogFile.fpMainError);
			FcloseAndNull(g_LogFile.fpSubInfo);
			FcloseAndNull(g_LogFile.fpSubError);
			if (pExtArg->byC2) {
				FcloseAndNull(g_LogFile.fpC2Error);
			}
		}
	}
}
#endif
