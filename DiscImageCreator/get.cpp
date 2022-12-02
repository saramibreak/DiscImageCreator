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
#include "get.h"
#include "output.h"

BOOL GetAlignedCallocatedBuffer(
	PDEVICE pDevice,
	LPBYTE* ppSrcBuf,
	UINT uiSize,
	LPBYTE* ppOutBuf,
	LPCTSTR pszFuncName,
	LONG lLineNum
) {
	*ppSrcBuf = (LPBYTE)calloc(uiSize + pDevice->AlignmentMask, sizeof(BYTE));
	if (!*ppSrcBuf) {
		OutputLastErrorNumAndString(pszFuncName, lLineNum);
		return FALSE;
	}
	*ppOutBuf = (LPBYTE)ConvParagraphBoundary(pDevice, *ppSrcBuf);
	return TRUE;
}

BOOL GetHandle(
	PDEVICE pDevice
) {
#ifdef _WIN32
	CONST size_t bufSize = 8;
	_TCHAR szBuf[bufSize] = {};
	_sntprintf(szBuf, bufSize, _T("\\\\.\\%c:"), pDevice->byDriveLetter);
	szBuf[7] = 0;
	pDevice->hDevice = CreateFile(szBuf, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
#else
	pDevice->hDevice = open(pDevice->drivepath, O_RDONLY | O_NONBLOCK, 0777);
#endif
	if (pDevice->hDevice == INVALID_HANDLE_VALUE) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	return TRUE;
}

BOOL GetDriveOffsetManually(
	LPINT lpDriveOffset
) {
	_TCHAR aBuf[6] = {};
	OutputString(
		"This drive doesn't define in driveOffset.txt\n"
		"Please input drive offset(Samples): ");
	if (!_tscanf(_T("%6[^\n]%*[^\n]"), aBuf)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (_gettchar() == EOF) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	*lpDriveOffset = _ttoi(aBuf);
	return TRUE;
}

FILE* GetAppUsedFilePointer(
	LPCTSTR filename,
	size_t filelen
) {
	FILE * fp = NULL;
#ifdef _WIN32
	UNREFERENCED_PARAMETER(filelen);
	fp = OpenProgrammabledFile(filename, _T("r"));
#else
	char pathLocalShare[128] = { "/usr/local/share/DiscImageCreator/" };
	if (strlen(pathLocalShare) + filelen + 1 > 128) {
		return NULL;
	}
	strncat(pathLocalShare, filename, filelen);
	if (PathFileExists(pathLocalShare)) {
		fp = fopen(pathLocalShare, "r");
	}
	else {
		char pathShare[128] = { "/usr/share/DiscImageCreator/" };
		if (strlen(pathShare) + filelen + 1 > 128) {
			return NULL;
		}
		strncat(pathShare, filename, filelen);
		if (PathFileExists(pathShare)) {
			fp = fopen(pathShare, "r");
		}
		else {
			fp = OpenProgrammabledFile(filename, _T("r"));
		}
	}
#endif
	return fp;
}

BOOL GetDriveOffsetAuto(
	PDEVICE pDevice,
	LPINT lpDriveOffset
) {
	BOOL bGetOffset = FALSE;
	FILE* fpDrive = GetAppUsedFilePointer(_T("driveOffset.txt"), _tcslen(_T("driveOffset.txt")));
	if (!fpDrive) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	CHAR szTmpProduct[DRIVE_PRODUCT_ID_SIZE + 1] = {};
	for (size_t src = 0, dst = 0; dst < DRIVE_PRODUCT_ID_SIZE; dst++) {
		// remove multiple or last space
		if (pDevice->szProductId[dst] == ' ') {
			if (dst < DRIVE_PRODUCT_ID_SIZE - 1 &&
				(pDevice->szProductId[dst + 1] == ' ' || pDevice->szProductId[dst + 1] == '\0')) {
				continue;
			}
			else if (dst == DRIVE_PRODUCT_ID_SIZE - 1) {
				continue;
			}
		}
		szTmpProduct[src++] = pDevice->szProductId[dst];
	}
	LPCH pTrimBuf[4] = {};
	CHAR lpBuf[1024] = {};

	while ((fgets(lpBuf, sizeof(lpBuf), fpDrive))) {
		pTrimBuf[0] = strtok(lpBuf, "	"); // tab
		for (INT nRoop = 1; nRoop < 4; nRoop++) {
			pTrimBuf[nRoop] = strtok(NULL, "	"); // tab
		}
		if (pTrimBuf[0] == NULL || pTrimBuf[1] == NULL || pTrimBuf[2] == NULL || *pTrimBuf[0] == '\n') {
			continue;
		}
		if (strstr(pTrimBuf[0], szTmpProduct) != NULL) {
			*lpDriveOffset = atoi(pTrimBuf[1]);
			bGetOffset = TRUE;
			break;
		}
	}
	fclose(fpDrive);
	return bGetOffset;
}

BOOL GetReadErrorFileName(
	PEXT_ARG pExtArg,
	CHAR protectFname[MAX_FNAME_FOR_VOLUME]
) {
	size_t size1 = strlen(protectFname);
	for (INT i = 0; i < MAX_READ_ERROR_FILE_COUNT; i++) {
		// -1 is to skip the null character
		size_t size2 = strlen(pExtArg->FILE.readError[i]) - 1;
		if (size1 == size2 &&
			!strncmp(protectFname, pExtArg->FILE.readError[i], size2)) {
			return TRUE;
		}
	}
	return FALSE;
}

INT GetReadErrorFileIdx(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nLBA
) {
	INT idx = 0;
	if (pDisc->PROTECT.byExist == physicalErr) {
		for (INT i = 0; i < pExtArg->FILE.readErrCnt; i++) {
			if (pDisc->PROTECT.ERROR_SECTOR.nExtentPos[i] <= nLBA &&
				nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos[i] + pDisc->PROTECT.ERROR_SECTOR.nSectorSize[i]) {
				idx = i;
				break;
			}
		}
	}
	return idx;
}

BOOL GetFilenameToSkipError(
	CHAR szFilename[][MAX_FNAME_FOR_VOLUME]
) {
	FILE* fp = GetAppUsedFilePointer(_T("ReadErrorProtect.txt"), _tcslen(_T("ReadErrorProtect.txt")));
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(" => ReadErrorProtect.txt");
		return FALSE;
	}
	CHAR comment[MAX_FNAME_FOR_VOLUME] = {};
	if (fgets(comment, MAX_FNAME_FOR_VOLUME, fp)) {
		// 2nd, 3rd ... line is filename
		for (INT i = 0; i < MAX_READ_ERROR_FILE_COUNT; i++) {
			if (!fgets(szFilename[i], MAX_FNAME_FOR_VOLUME, fp)) {
				break;
			}
		}
	}
	else {
		return FALSE;
	}
	return TRUE;
}

BOOL GetC2ErrorFileName(
	PEXT_ARG pExtArg,
	CHAR protectFname[MAX_FNAME_FOR_VOLUME]
) {
	size_t size1 = strlen(protectFname);
	for (INT i = 0; i < MAX_READ_ERROR_FILE_COUNT; i++) {
		// -1 is to skip the null character
		size_t size2 = strlen(pExtArg->FILE.c2Error[i]) - 1;
		if (size1 == size2 &&
			!strncmp(protectFname, pExtArg->FILE.c2Error[i], size2)) {
			return TRUE;
		}
	}
	return FALSE;
}

INT GetC2ErrorFileIdx(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nLBA
) {
	INT idx = 0;
	if (pDisc->PROTECT.byExist == c2Err) {
		while (idx < pExtArg->FILE.c2ErrCnt) {
			if (pDisc->PROTECT.ERROR_SECTOR.nExtentPos[idx] <= nLBA &&
				nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos[idx] + pDisc->PROTECT.ERROR_SECTOR.nSectorSize[idx]) {
				return idx;
			}
			idx++;
		}
	}
	return idx;
}

BOOL GetFilenameToFixError(
	CHAR szFilename[][MAX_FNAME_FOR_VOLUME]
) {
	FILE* fp = GetAppUsedFilePointer(_T("C2ErrorProtect.txt"), _tcslen(_T("C2ErrorProtect.txt")));
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	CHAR comment[MAX_FNAME_FOR_VOLUME] = {};
	if (fgets(comment, MAX_FNAME_FOR_VOLUME, fp)) {
		// 2nd, 3rd ... line is filename
		for (INT i = 0; i < MAX_READ_ERROR_FILE_COUNT; i++) {
			if (!fgets(szFilename[i], MAX_FNAME_FOR_VOLUME, fp)) {
				break;
			}
		}
	}
	else {
		return FALSE;
	}
	return TRUE;
}

BOOL GetTimeStamp(
	_TCHAR* pszTime,
	size_t stBufsize,
	DWORD dwTime
) {
	time_t timeDateStamp = dwTime;
	tm* ctime = gmtime(&timeDateStamp);
	_tcsftime(pszTime, stBufsize / sizeof(_TCHAR), _T("%FT%T"), ctime);
	return TRUE;
}

DWORD GetFileSize(
	LONG lOffset,
	FILE *fp
) {
	DWORD dwFileSize = 0;
	if (fp != NULL) {
		fseek(fp, 0, SEEK_END);
		dwFileSize = (DWORD)ftell(fp);
		fseek(fp, lOffset, SEEK_SET);
	}
	return dwFileSize;
}

UINT64 GetFileSize64(
	INT64 n64Offset,
	FILE *fp
) {
	UINT64 ui64FileSize = 0;
	if (fp != NULL) {
		_fseeki64(fp, 0, SEEK_END);
		ui64FileSize = (UINT64)_ftelli64(fp);
		_fseeki64(fp, n64Offset, SEEK_SET);
	}
	return ui64FileSize;
}

BOOL GetDiscSize(
	LPTSTR path,
	PUINT64 lpSize
) {
	BOOL bRet = TRUE;
#ifdef _WIN32
	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile(path, &FindData);
	size_t len = _tcslen(path) - 1;
	path[len] = 0;
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			UINT64 size = 0;
			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (!_tcscmp(_T("."), FindData.cFileName) || !_tcscmp(_T(".."), FindData.cFileName)) {
					continue;
				}
				_TCHAR buf[MAX_PATH] = {};
				_stprintf(buf, _T("%s%s\\*"), path, FindData.cFileName);
				if (GetDiscSize(buf, &size) == FALSE) {
					bRet = FALSE;
					break;
				}
				*lpSize += size;
			}
			else {
				*lpSize += MAKEDWORD64(FindData.nFileSizeLow, FindData.nFileSizeHigh);
			}
		} while (FindNextFile(hFind, &FindData));
	}
	else {
		bRet = FALSE;
	}
	FindClose(hFind);
#else
	// TODO
	UNREFERENCED_PARAMETER(path);
	UNREFERENCED_PARAMETER(lpSize);
#endif
	return bRet;
}

WORD GetSizeOrWordForVolDesc(
	LPBYTE lpBuf
) {
	WORD val = MAKEWORD(lpBuf[0], lpBuf[1]);
	if (val == 0) {
		val = MAKEWORD(lpBuf[3], lpBuf[2]);
	}
	return val;
}

UINT GetSizeOrUintForVolDesc(
	LPBYTE lpBuf,
	UINT uiMax
) {
	UINT val = MAKEUINT(MAKEWORD(lpBuf[0], lpBuf[1]),
		MAKEWORD(lpBuf[2], lpBuf[3]));
	if (val == 0 || val >= uiMax) {
		val = MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]),
			MAKEWORD(lpBuf[5], lpBuf[4]));
	}
	return val;
}

BYTE GetMode(
	PDISC_PER_SECTOR pDiscPerSector,
	INT nType
) {
	BYTE byMode = 0;
	if ((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		if (IsValidMainDataHeader(pDiscPerSector->mainHeader.current)) {
			if ((pDiscPerSector->mainHeader.current[15] & 0x60) == 0x60 && nType == unscrambled) {
				byMode = BcdToDec((BYTE)(pDiscPerSector->mainHeader.current[15] ^ 0x60));
			}
			else {
				byMode = pDiscPerSector->mainHeader.current[15];
			}
		}
		else {
			if ((pDiscPerSector->mainHeader.prev[15] & 0x60) == 0x60 && nType == unscrambled) {
				byMode = BcdToDec(pDiscPerSector->mainHeader.prev[15]);
			}
			else {
				byMode = pDiscPerSector->mainHeader.prev[15];
			}
		}
	}
	else if ((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == 0) {
		byMode = DATA_BLOCK_MODE0;
	}
	else {
		if ((pDiscPerSector->mainHeader.prev[15] & 0x60) == 0x60 && nType == unscrambled) {
			byMode = BcdToDec(pDiscPerSector->mainHeader.prev[15]);
		}
		else {
			byMode = pDiscPerSector->mainHeader.prev[15];
		}
	}
	return byMode;
}

BOOL GetWriteOffset(
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nSectorNum,
	INT nLBA
) {
	BOOL bRet = FALSE;
	for (INT i = 0; i < CD_RAW_SECTOR_SIZE * nSectorNum; i++) {
		if (IsValidMainDataHeader(lpBuf + i)) {
			BYTE sm = BcdToDec((BYTE)(lpBuf[i + 12] ^ 0x01));
			BYTE ss = BcdToDec((BYTE)(lpBuf[i + 13] ^ 0x80));
			BYTE sf = BcdToDec((BYTE)(lpBuf[i + 14]));
			if ((sm & 0x01) == 1 && (ss & 0x50) == 80) {
				break;
			}
			INT tmpLBA = MSFtoLBA(sm, ss, sf) - 150;
			pDisc->MAIN.nCombinedOffset = CD_RAW_SECTOR_SIZE * -(tmpLBA - nLBA) + i;
			bRet = TRUE;
			break;
		}
	}
	return bRet;
}

BOOL GetCmd(
	LPTSTR szPath,
	LPCTSTR szFname,
	LPCTSTR szExt
) {
	if (!::GetModuleFileName(NULL, szPath, _MAX_PATH)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	_TCHAR szDrive[_MAX_DRIVE] = {};
	_TCHAR szDir[_MAX_DIR] = {};
	_tsplitpath(szPath, szDrive, szDir, NULL, NULL);
	_tmakepath(szPath, szDrive, szDir, szFname, szExt);
	return TRUE;
}

BOOL GetEccEdcCmd(
	LPTSTR pszStr,
	size_t cmdSize,
	LPCTSTR pszCmd,
	LPCTSTR pszImgPath,
	INT nStartLBA,
	INT nEndLBA
) {
	_TCHAR szPathForEcc[_MAX_PATH] = {};
#ifdef _WIN32
	BOOL bRet = GetCmd(szPathForEcc, _T("EccEdc"), _T("exe"));
#else
	BOOL bRet = GetCmd(szPathForEcc, _T("./EccEdc_linux"), _T(".out"));
#endif
	if (bRet && PathFileExists(szPathForEcc)) {
		if (!_tcscmp(pszCmd, _T("check"))) {
#ifdef _WIN32
			_sntprintf(pszStr, cmdSize,
				_T("\"\"%s\" %s \"%s\"\""), szPathForEcc, pszCmd, pszImgPath);
#else
			_sntprintf(pszStr, cmdSize,
				_T("%s %s \"%s\""), szPathForEcc, pszCmd, pszImgPath);
#endif
		}
		else if (!_tcscmp(pszCmd, _T("fix"))) {
#ifdef _WIN32
			_sntprintf(pszStr, cmdSize,
				_T("\"\"%s\" %s \"%s\"\" %d %d"),
				szPathForEcc, pszCmd, pszImgPath, nStartLBA, nEndLBA);
#else
			_sntprintf(pszStr, cmdSize,
				_T("%s %s \"%s\" %d %d"),
				szPathForEcc, pszCmd, pszImgPath, nStartLBA, nEndLBA);
#endif
		}
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(" => %s\n", szPathForEcc);
		bRet = FALSE;
	}
	return bRet;
}

BOOL GetUnscCmd(
	LPTSTR pszStr,
	LPCTSTR pszPath
) {
	_TCHAR szDrive[_MAX_DRIVE] = {};
	_TCHAR szDir[_MAX_DIR] = {};
	_TCHAR szFname[_MAX_FNAME] = {};
	_TCHAR szPathForIso[_MAX_PATH] = {};
	_tsplitpath(pszPath, szDrive, szDir, szFname, NULL);
	_tmakepath(szPathForIso, szDrive, szDir, szFname, _T("iso"));

	_TCHAR szPathForUnsc[_MAX_PATH] = {};
#ifdef _WIN32
	BOOL bRet = GetCmd(szPathForUnsc, _T("unscrambler"), _T("exe"));
#else
	BOOL bRet = GetCmd(szPathForUnsc, _T("./unscrambler_linux"), _T(".out"));
#endif
	if (bRet && PathFileExists(szPathForUnsc)) {
		size_t size = _tcslen(szPathForUnsc) + _tcslen(pszPath) + _tcslen(szPathForIso) + 9;
#ifdef _WIN32
		_sntprintf(pszStr, size,
			_T("\"\"%s\" \"%s\" \"%s\"\""), szPathForUnsc, pszPath, szPathForIso);
#else
		_sntprintf(pszStr, size,
			_T("%s %s %s"), szPathForUnsc, pszPath, szPathForIso);
#endif
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(" => %s\n", szPathForUnsc);
		bRet = FALSE;
	}
	return bRet;
}

BOOL GetDVDProtectionCmd(
	PDEVICE pDevice,
	LPTSTR pszStr,
	_PROTECT_TYPE_DVD protect,
	LPCTSTR pszPath
) {
	_TCHAR szDrive[_MAX_DRIVE] = {};
	_TCHAR szDir[_MAX_DIR] = {};
	_TCHAR szFname[_MAX_FNAME] = {};
	_TCHAR szPathForKey[_MAX_PATH] = {};
	_tsplitpath(pszPath, szDrive, szDir, szFname, NULL);

	_TCHAR keyPath[_MAX_FNAME] = {};
	_TCHAR keyFile[10] = {}; 
	if (protect == css) {
		_tcsncpy(keyFile, _T("_CSSKey"), 8);
	}
	else if (protect == cppm) {
		_tcsncpy(keyFile, _T("_CPPMKey"), 9);
	}
	else if (protect == cprm) {
		_tcsncpy(keyFile, _T("_CPRMKey"), 9);
	}

	if (_tcslen(szFname) + _tcslen(keyFile) > _MAX_FNAME) {
		OutputErrorString("Path too long\n");
		return FALSE;
	}
	_tcsncpy(keyPath, szFname, sizeof(keyPath) / sizeof(keyPath[0]));
	_tcsncat(keyPath, keyFile, sizeof(keyPath) / sizeof(keyPath[0]) - _tcslen(keyPath) - 1);

	_TCHAR szPathForCss[_MAX_PATH] = {};
#ifdef _WIN32
	_tmakepath(szPathForKey, szDrive, szDir, keyPath, _T("txt"));
	BOOL bRet = GetCmd(szPathForCss, _T("DVDAuth"), _T("exe"));
#else
	_tmakepath(szPathForKey, szDrive, szDir, keyPath, _T(".txt"));
	BOOL bRet = GetCmd(szPathForCss, _T("./DVDAuth_linux"), _T(".out"));
#endif
	if (bRet && PathFileExists(szPathForCss)) {
#ifdef _WIN32
		size_t size = _tcslen(szPathForCss) + _tcslen(szDrive) + _tcslen(szPathForKey) + 9 + 4;
		if (protect == css) {
			_sntprintf(pszStr, size,
				_T("\"\"%s\" %c css \"%s\"\""), szPathForCss, pDevice->byDriveLetter, szPathForKey);
		}
		else if (protect == cppm) {
			_sntprintf(pszStr, size,
				_T("\"\"%s\" %c cppm \"%s\"\""), szPathForCss, pDevice->byDriveLetter, szPathForKey);
}
		else if (protect == cprm) {
			_sntprintf(pszStr, size,
				_T("\"\"%s\" %c cprm \"%s\"\""), szPathForCss, pDevice->byDriveLetter, szPathForKey);
		}
		OutputString("%s\n", pszStr);
#else
		size_t size = _tcslen(szPathForCss) + _tcslen(pDevice->drivepath) + _tcslen(szPathForKey) + 9 + 4;
		if (protect == css) {
			_sntprintf(pszStr, size,
				_T("%s %s css %s"), szPathForCss, pDevice->drivepath, szPathForKey);
		}
		else if (protect == cppm) {
			_sntprintf(pszStr, size,
				_T("%s %s cppm %s"), szPathForCss, pDevice->drivepath, szPathForKey);
		}
		else if (protect == cprm) {
			_sntprintf(pszStr, size,
				_T("%s %s cprm %s"), szPathForCss, pDevice->drivepath, szPathForKey);
		}
		OutputString("%s\n", pszStr);
#endif
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(" => %s\n", szPathForCss);
		bRet = FALSE;
	}
	return bRet;
}

UINT GetLBAfromClusterNumber(
	PEXFAT pExFat,
	UINT uiClusterNum
) {
	return pExFat->ClusterHeapOffset + pExFat->SectorsPerClusterShift * (uiClusterNum - 2);
}
