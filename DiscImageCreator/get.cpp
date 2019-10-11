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
		_T("This drive doesn't define in driveOffset.txt\n")
		_T("Please input drive offset(Samples): "));
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

BOOL GetDriveOffsetAuto(
	LPCSTR szProductId,
	LPINT lpDriveOffset
) {
	BOOL bGetOffset = FALSE;
	FILE* fpDrive = OpenProgrammabledFile(_T("driveOffset.txt"), _T("r"));
	if (!fpDrive) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	CHAR szProduct[DRIVE_PRODUCT_ID_SIZE + 1] = {};
	for (size_t src = 0, dst = 0; dst < sizeof(szProduct) - 1; dst++) {
		if (szProductId[dst] == ' ' && (szProductId[dst + 1] == ' ' ||
			szProductId[dst + 1] == '\0')) {
			continue;
		}
		szProduct[src++] = szProductId[dst];
	}

	LPCH pTrimId[5] = {};
	LPCH pId = NULL;
	pTrimId[0] = strtok(szProduct, " ");
	// get model string (ex. PX-755A)
	for (INT nRoop = 1; nRoop < 5; nRoop++) {
		pTrimId[nRoop] = strtok(NULL, " ");
		if (pTrimId[nRoop] != NULL) {
			pId = pTrimId[nRoop];
		}
		else {
			if (pTrimId[1] == NULL) {
				pId = pTrimId[0];
			}
			break;
		}
	}
	if (pId) {
		LPCH pTrimBuf[10] = {};
		CHAR lpBuf[1024] = {};

		while ((fgets(lpBuf, sizeof(lpBuf), fpDrive))) {
			pTrimBuf[0] = strtok(lpBuf, " 	"); // space & tab
			for (INT nRoop = 1; nRoop < 10; nRoop++) {
				pTrimBuf[nRoop] = strtok(NULL, " 	"); // space & tab
			}
			if (pTrimBuf[0] == NULL || pTrimBuf[1] == NULL || pTrimBuf[2] == NULL) {
				continue;
			}
			else if (*pTrimBuf[0] == '\n' || (*pTrimBuf[1] != '-' && *pTrimBuf[2] != '-')) {
				continue;
			}
			for (INT nRoop = 0; nRoop < 10 && pTrimBuf[nRoop] != NULL; nRoop++) {
				if (strstr(pTrimBuf[nRoop], pId) != NULL) {
					*lpDriveOffset = atoi(pTrimBuf[nRoop + 1]);
					bGetOffset = TRUE;
					break;
				}
			}
			if (bGetOffset) {
				break;
			}
		}
	}
	fclose(fpDrive);
	return bGetOffset;
}

BOOL GetFilenameToSkipError(
	LPSTR szFilename
) {
	FILE* fp = OpenProgrammabledFile(_T("ReadErrorProtect.txt"), _T("r"));
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T(" => ReadErrorProtect.txt"));
		return FALSE;
	}
	CHAR comment[MAX_FNAME_FOR_VOLUME] = {};
	if (fgets(comment, MAX_FNAME_FOR_VOLUME, fp)) {
		if (!fgets(szFilename, MAX_FNAME_FOR_VOLUME, fp)) { // 2nd line is filename
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
	return TRUE;
}

BOOL GetFilenameToFixError(
	LPSTR szFilename
) {
	FILE* fp = OpenProgrammabledFile(_T("EdcEccErrorProtect.txt"), _T("r"));
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	CHAR comment[MAX_FNAME_FOR_VOLUME] = {};
	if (fgets(comment, MAX_FNAME_FOR_VOLUME, fp)) {
		if (!fgets(szFilename, MAX_FNAME_FOR_VOLUME, fp)) { // 2nd line is filename
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
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
	if ((pDiscPerSector->subQ.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
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
	else if ((pDiscPerSector->subQ.current.byCtl & AUDIO_DATA_TRACK) == 0) {
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
	LPBYTE lpBuf
) {
	BOOL bRet = FALSE;
	for (INT i = 0; i < CD_RAW_SECTOR_SIZE * 2; i++) {
		if (IsValidMainDataHeader(lpBuf + i)) {
			BYTE sm = BcdToDec((BYTE)(lpBuf[i + 12] ^ 0x01));
			BYTE ss = BcdToDec((BYTE)(lpBuf[i + 13] ^ 0x80));
			BYTE sf = BcdToDec((BYTE)(lpBuf[i + 14]));
			if ((sm & 0x01) == 1 && (ss & 0x50) == 80) {
				break;
			}
			INT tmpLBA = MSFtoLBA(sm, ss, sf) - 150;
			pDisc->MAIN.nCombinedOffset =
				CD_RAW_SECTOR_SIZE * -(tmpLBA - pDisc->SCSI.nFirstLBAofDataTrack) + i;
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
		OutputErrorString(_T(" => %s\n"), szPathForEcc);
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
		OutputErrorString(_T(" => %s\n"), szPathForUnsc);
		bRet = FALSE;
	}
	return bRet;
}

BOOL GetCssCmd(
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
	else if (protect == cprm) {
		_tcsncpy(keyFile, _T("_CPRMKey"), 9);
	}

	if (_tcslen(szFname) + _tcslen(keyFile) > _MAX_FNAME) {
		OutputErrorString(_T("Path too long\n"));
		return FALSE;
	}
	_tcsncpy(keyPath, szFname, _tcslen(szFname));
	_tcsncat(keyPath, keyFile, _tcslen(keyFile));
	_tmakepath(szPathForKey, szDrive, szDir, keyPath, _T("txt"));

	_TCHAR szPathForCss[_MAX_PATH] = {};
#ifdef _WIN32
	BOOL bRet = GetCmd(szPathForCss, _T("DVDAuth"), _T("exe"));
#else
	UNREFERENCED_PARAMETER(protect);
	BOOL bRet = GetCmd(szPathForCss, _T("./css-auth"), _T(".out"));
#endif
	OutputString("%s\n", szPathForCss);
	if (bRet && PathFileExists(szPathForCss)) {
		size_t size = _tcslen(szPathForCss) + _tcslen(szPathForKey) + 9 + 4;
#ifdef _WIN32
		if (protect == css) {
			_sntprintf(pszStr, size,
				_T("\"\"%s\" %c css \"%s\"\""), szPathForCss, pDevice->byDriveLetter, szPathForKey);
		}
		else if (protect == cprm) {
			_sntprintf(pszStr, size,
				_T("\"\"%s\" %c cprm \"%s\"\""), szPathForCss, pDevice->byDriveLetter, szPathForKey);
		}
#else
		_sntprintf(pszStr, size,
			_T("%s %s"), szPathForCss, pDevice->drivepath/*, szPathForKey*/);
#endif
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T(" => %s\n"), szPathForCss);
		bRet = FALSE;
	}
	return bRet;
}
