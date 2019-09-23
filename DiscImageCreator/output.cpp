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
#include "execScsiCmd.h"
#include "execScsiCmdforCDCheck.h"
#include "get.h"
#include "output.h"
#include "outputScsiCmdLog.h"
#include "outputScsiCmdLogforCD.h"
#include "set.h"

#ifdef _DEBUG
WCHAR logBufferW[DISC_RAW_READ_SIZE];
CHAR logBufferA[DISC_RAW_READ_SIZE];
#endif
// These global variable is set at prngcd.cpp
extern unsigned char scrambled_table[2352];

FILE* CreateOrOpenFile(
	LPCTSTR pszPath,
	LPCTSTR pszPlusFname,
	LPTSTR pszOutPath,
	LPTSTR pszFnameAndExt,
	LPTSTR pszFname,
	LPCTSTR pszExt,
	LPCTSTR pszMode,
	BYTE byTrackNum,
	BYTE byMaxTrackNum
) {
	_TCHAR szDstPath[_MAX_PATH] = {};
	_TCHAR szDrive[_MAX_DRIVE] = {};
	_TCHAR szDir[_MAX_DIR] = {};
	_TCHAR szFname[_MAX_FNAME] = {};
	_TCHAR szExt[_MAX_EXT] = {};

	_tsplitpath(pszPath, szDrive, szDir, szFname, szExt);
	if (pszPlusFname) {
		size_t plusFnameLen = _tcslen(pszPlusFname);
		UINT pathSize = UINT(_tcslen(szDir) + _tcslen(szFname) + plusFnameLen + _tcslen(pszExt));
		if (pathSize > _MAX_FNAME) {
			OutputErrorString(_T("File Size is too long: %u\n"), pathSize);
			return NULL;
		}
		_tcsncat(szFname, pszPlusFname, plusFnameLen);
	}
	if (byMaxTrackNum <= 1) {
		_sntprintf(szDstPath, sizeof(szDstPath) / sizeof(szDstPath[0]),
			_T("%s%s%s%s"), szDrive, szDir, szFname, pszExt);
	}
	else if (2 <= byMaxTrackNum && byMaxTrackNum <= 9) {
		_sntprintf(szDstPath, sizeof(szDstPath) / sizeof(szDstPath[0]),
			_T("%s%s%s (Track %u)%s"), szDrive, szDir, szFname, byTrackNum, pszExt);
	}
	else if (10 <= byMaxTrackNum) {
		_sntprintf(szDstPath, sizeof(szDstPath) / sizeof(szDstPath[0]),
			_T("%s%s%s (Track %02u)%s"), szDrive, szDir, szFname, byTrackNum, pszExt);
	}
	szDstPath[_MAX_PATH - 1] = 0;

	if (pszFnameAndExt) {
		// size of pszFnameAndExt must be _MAX_FNAME.
		ZeroMemory(pszFnameAndExt, _MAX_FNAME);
		_tsplitpath(szDstPath, NULL, NULL, szFname, szExt);
		_sntprintf(pszFnameAndExt, _MAX_FNAME, _T("%s%s"), szFname, szExt);
	}
	if (pszFname) {
		// size of pszFname must be _MAX_FNAME.
		ZeroMemory(pszFname, _MAX_FNAME);
		_tsplitpath(szDstPath, NULL, NULL, szFname, NULL);
		_sntprintf(pszFname, _MAX_FNAME, _T("%s"), szFname);
	}
	if (pszOutPath) {
		// size of pszOutPath must be _MAX_PATH.
		_tcsncpy(pszOutPath, szDstPath, _MAX_PATH);
	}
	FILE* fp = _tfopen(szDstPath, pszMode);
#ifdef UNICODE
	// delete bom
	fseek(fp, 0, SEEK_SET);
#endif
	return fp;
}
#ifdef _WIN32
FILE* CreateOrOpenFileW(
	LPCWSTR pszPath,
	LPCWSTR pszPlusFname,
	LPWSTR pszOutPath,
	LPWSTR pszFnameAndExt,
	LPWSTR pszFname,
	LPCWSTR pszExt,
	LPCWSTR pszMode,
	BYTE byTrackNum,
	BYTE byMaxTrackNum
) {
	WCHAR szDstPath[_MAX_PATH] = {};
	WCHAR szDrive[_MAX_DRIVE] = {};
	WCHAR szDir[_MAX_DIR] = {};
	WCHAR szFname[_MAX_FNAME] = {};
	WCHAR szExt[_MAX_EXT] = {};

	_wsplitpath(pszPath, szDrive, szDir, szFname, szExt);
	if (pszPlusFname) {
		size_t plusFnameLen = wcslen(pszPlusFname);
		UINT pathSize = UINT(wcslen(szDir) + wcslen(szFname) + plusFnameLen + wcslen(pszExt));
		if (pathSize > _MAX_FNAME) {
			OutputErrorString(_T("File Size is too long: %u\n"), pathSize);
			return NULL;
		}
		wcsncat(szFname, pszPlusFname, plusFnameLen);
	}
	if (byMaxTrackNum <= 1) {
		_snwprintf(szDstPath, sizeof(szDstPath) / sizeof(szDstPath[0]),
			L"%s%s%s%s", szDrive, szDir, szFname, pszExt);
	}
	else if (2 <= byMaxTrackNum && byMaxTrackNum <= 9) {
		_snwprintf(szDstPath, sizeof(szDstPath) / sizeof(szDstPath[0]),
			L"%s%s%s (Track %u)%s", szDrive, szDir, szFname, byTrackNum, pszExt);
	}
	else if (10 <= byMaxTrackNum) {
		_snwprintf(szDstPath, sizeof(szDstPath) / sizeof(szDstPath[0]),
			L"%s%s%s (Track %02u)%s", szDrive, szDir, szFname, byTrackNum, pszExt);
	}
	szDstPath[_MAX_PATH - 1] = 0;

	if (pszFnameAndExt) {
		// size of pszFnameAndExt must be _MAX_PATH.
		ZeroMemory(pszFnameAndExt, _MAX_FNAME);
		_wsplitpath(szDstPath, NULL, NULL, szFname, szExt);
		_snwprintf(pszFnameAndExt, _MAX_FNAME, L"%s%s", szFname, szExt);
	}
	if (pszFname) {
		// size of pszFname must be _MAX_PATH.
		ZeroMemory(pszFname, _MAX_FNAME);
		_wsplitpath(szDstPath, NULL, NULL, szFname, NULL);
		_snwprintf(pszFname, _MAX_FNAME, L"%s", szFname);
	}
	if (pszOutPath) {
		// size of pszOutPath must be _MAX_PATH.
		wcsncpy(pszOutPath, szDstPath, _MAX_PATH);
	}
	FILE* fp = _wfopen(szDstPath, pszMode);
#ifndef UNICODE
	// delete bom
	fseek(fp, 0, SEEK_SET);
#endif
	return fp;
}
#endif
FILE* CreateOrOpenFileA(
	LPCSTR pszPath,
	LPCSTR pszPlusFname,
	LPSTR pszOutPath,
	LPSTR pszFnameAndExt,
	LPSTR pszFname,
	LPCSTR pszExt,
	LPCSTR pszMode,
	BYTE byTrackNum,
	BYTE byMaxTrackNum
) {
	CHAR szDstPath[_MAX_PATH] = {};
	CHAR szDrive[_MAX_DRIVE] = {};
	CHAR szDir[_MAX_DIR] = {};
	CHAR szFname[_MAX_FNAME] = {};
	CHAR szExt[_MAX_EXT] = {};

	_splitpath(pszPath, szDrive, szDir, szFname, szExt);
	if (pszPlusFname) {
		size_t plusFnameLen = strlen(pszPlusFname);
		UINT pathSize = UINT(strlen(szDir) + strlen(szFname) + plusFnameLen + strlen(pszExt));
		if (pathSize > _MAX_FNAME) {
			OutputErrorString(_T("File Size is too long: %u\n"), pathSize);
			return NULL;
		}
		strncat(szFname, pszPlusFname, plusFnameLen);
	}
	if (byMaxTrackNum <= 1) {
		_snprintf(szDstPath, sizeof(szDstPath) / sizeof(szDstPath[0]),
			"%s%s%s%s", szDrive, szDir, szFname, pszExt);
	}
	else if (2 <= byMaxTrackNum && byMaxTrackNum <= 9) {
		_snprintf(szDstPath, sizeof(szDstPath) / sizeof(szDstPath[0]),
			"%s%s%s (Track %u)%s", szDrive, szDir, szFname, byTrackNum, pszExt);
	}
	else if (10 <= byMaxTrackNum) {
		_snprintf(szDstPath, sizeof(szDstPath) / sizeof(szDstPath[0]),
			"%s%s%s (Track %02u)%s", szDrive, szDir, szFname, byTrackNum, pszExt);
	}
	szDstPath[_MAX_PATH - 1] = 0;

	if (pszFnameAndExt) {
		// size of pszFnameAndExt must be _MAX_FNAME.
		ZeroMemory(pszFnameAndExt, _MAX_FNAME);
		_splitpath(szDstPath, NULL, NULL, szFname, szExt);
		_snprintf(pszFnameAndExt, _MAX_FNAME, "%s%s", szFname, szExt);
	}
	if (pszFname) {
		// size of pszFname must be _MAX_FNAME.
		ZeroMemory(pszFname, _MAX_FNAME);
		_splitpath(szDstPath, NULL, NULL, szFname, NULL);
		_snprintf(pszFname, _MAX_FNAME, "%s", szFname);
	}
	if (pszOutPath) {
		// size of pszOutPath must be _MAX_PATH.
		strncpy(pszOutPath, szDstPath, _MAX_PATH);
	}
	FILE* fp = fopen(szDstPath, pszMode);
	return fp;
}

FILE* OpenProgrammabledFile(
	LPCTSTR pszFname,
	LPCTSTR pszMode
) {
	_TCHAR szFullPath[_MAX_PATH] = {};
	if (!::GetModuleFileName(NULL, szFullPath, sizeof(szFullPath) / sizeof(szFullPath[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return NULL;
	}
	FILE* fp = NULL;
#ifdef _WIN32
	_TCHAR* p = _tcsrchr(szFullPath, '\\');
#else
	_TCHAR* p = _tcsrchr(szFullPath, '/');
#endif
	if (p) {
		p[0] = 0;
		_TCHAR szFullPathName[_MAX_PATH] = {};
#ifdef _WIN32
		_sntprintf(szFullPathName
			, sizeof(szFullPathName) / sizeof(szFullPathName[0]), _T("%s\\%s"), szFullPath, pszFname);
#else
		_sntprintf(szFullPathName
			, sizeof(szFullPathName) / sizeof(szFullPathName[0]), _T("%s/%s"), szFullPath, pszFname);
#endif
		szFullPathName[_MAX_PATH - 1] = 0;
		fp = _tfopen(szFullPathName, pszMode);
	}
	return fp;
}
#ifdef _WIN32
FILE* OpenProgrammabledFileW(
	LPCWSTR pszFname,
	LPCWSTR pszMode
) {
	WCHAR szFullPath[_MAX_PATH] = {};
	if (!::GetModuleFileNameW(NULL, szFullPath, sizeof(szFullPath) / sizeof(szFullPath[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return NULL;
	}
	FILE* fp = NULL;
	LPWCH p = wcsrchr(szFullPath, '\\');
	if (p) {
		p[0] = 0;
		WCHAR szFullPathName[_MAX_PATH] = {};
		_snwprintf(szFullPathName
			, sizeof(szFullPathName) / sizeof(szFullPathName[0]), L"%s\\%s", szFullPath, pszFname);
		szFullPathName[_MAX_PATH - 1] = 0;
		fp = _wfopen(szFullPathName, pszMode);
	}
	return fp;
}
#endif

VOID WriteCcdForDisc(
	WORD wTocEntries,
	BYTE LastCompleteSession,
	FILE* fpCcd
) {
	if (fpCcd) {
		_ftprintf(fpCcd,
			_T("[CloneCD]\n")
			_T("Version=3\n")
			_T("[Disc]\n")
			_T("TocEntries=%u\n")
			_T("Sessions=%u\n")
			_T("DataTracksScrambled=%u\n"),
			wTocEntries,
			LastCompleteSession,
			0); // TODO
	}
}

VOID WriteCcdForDiscCDTextLength(
	WORD wCDTextLength,
	FILE* fpCcd
) {
	if (fpCcd) {
		_ftprintf(fpCcd, _T("CDTextLength=%u\n"), wCDTextLength);
	}
}

VOID WriteCcdForDiscCatalog(
	PDISC pDisc,
	FILE* fpCcd
) {
	_TCHAR szCatalog[META_CATALOG_SIZE] = {};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0
		, pDisc->SUB.szCatalog, sizeof(pDisc->SUB.szCatalog) / sizeof(pDisc->SUB.szCatalog[0])
		, szCatalog, sizeof(szCatalog) / sizeof(szCatalog[0]));
#else
	strncpy(szCatalog, pDisc->SUB.szCatalog, sizeof(szCatalog) / sizeof(szCatalog[0]));
#endif
	if (fpCcd) {
		_ftprintf(fpCcd, _T("CATALOG=%s\n"), szCatalog);
	}
}

VOID WriteCcdForCDText(
	WORD wTocTextEntries,
	FILE* fpCcd
) {
	if (fpCcd) {
		_ftprintf(fpCcd,
			_T("[CDText]\n")
			_T("Entries=%u\n"),
			wTocTextEntries);
	}
}

VOID WriteCcdForCDTextEntry(
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	WORD wTocTextEntries,
	FILE* fpCcd
) {
	if (fpCcd) {
		for (WORD t = 0; t < wTocTextEntries; t++) {
			_ftprintf(fpCcd,
				_T("Entry %u=%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"),
				t,
				pDesc[t].PackType,
				pDesc[t].TrackNumber | (pDesc[t].ExtensionFlag << 7),
				pDesc[t].SequenceNumber,
				pDesc[t].CharacterPosition | (pDesc[t].BlockNumber << 4) | (pDesc[t].Unicode << 7),
				pDesc[t].Text[0], pDesc[t].Text[1], pDesc[t].Text[2], pDesc[t].Text[3],
				pDesc[t].Text[4], pDesc[t].Text[5], pDesc[t].Text[6], pDesc[t].Text[7],
				pDesc[t].Text[8], pDesc[t].Text[9], pDesc[t].Text[10], pDesc[t].Text[11]);
		}
	}
}

VOID WriteCcdForSession(
	BYTE SessionNumber,
	BYTE byMode,
	FILE* fpCcd
) {
	if (fpCcd) {
		_ftprintf(fpCcd,
			_T("[Session %u]\n")
			_T("PreGapMode=%u\n")
			_T("PreGapSubC=%u\n"),
			SessionNumber,
			byMode,
			SessionNumber == 1 ? byMode : 0); // TODO
	}
}

BOOL WriteCcdFirst(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	PCDROM_TOC_FULL_TOC_DATA fullToc,
	PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData,
	WORD wTocEntries,
	FILE* fpCcd
) {
	if (fpCcd) {
		WriteCcdForDisc(wTocEntries, fullToc->LastCompleteSession, fpCcd);
		if (pDevice->FEATURE.byCanCDText) {
			ReadTOCText(pExtArg, pDevice, pDisc, fpCcd);
		}
		LPBYTE pBuf = NULL;
		LPBYTE lpBuf = NULL;
		BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
		INT nOfs = 0;
		UINT uiBufLen = (CD_RAW_SECTOR_SIZE + CD_RAW_READ_SUBCODE_SIZE) * 2;
		if (!ReadCDForCheckingSubQAdrFirst(pExtArg
			, pDevice, pDisc, &pBuf, &lpBuf, lpCmd, &uiBufLen, &nOfs)) {
			FreeAndNull(pBuf);
			return FALSE;
		}
		BYTE byMode = DATA_BLOCK_MODE0;
		BYTE bySessionNum = 0;
		for (WORD i = 0; i < wTocEntries; i++) {
			if (pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX40TS) {
				// Somehow Ultraplex seems to get the fulltoc data as "hexadecimal"
				pTocData[i].Msf[0] = BcdToDec(pTocData[i].Msf[0]);
				pTocData[i].Msf[1] = BcdToDec(pTocData[i].Msf[1]);
				pTocData[i].Msf[2] = BcdToDec(pTocData[i].Msf[2]);
				pTocData[i].MsfExtra[0] = BcdToDec(pTocData[i].MsfExtra[0]);
				pTocData[i].MsfExtra[1] = BcdToDec(pTocData[i].MsfExtra[1]);
				pTocData[i].MsfExtra[2] = BcdToDec(pTocData[i].MsfExtra[2]);
				if (pTocData[i].Point < 0xa0) {
					pTocData[i].Point = BcdToDec(pTocData[i].Point);
				}
			}
			if (pTocData[i].Point < 100) {
				if (!ReadCDForCheckingSubQAdr(pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd, lpBuf
					, uiBufLen, nOfs, (BYTE)(pTocData[i].Point - 1), &byMode, pTocData[i].SessionNumber, fpCcd)) {
					FreeAndNull(pBuf);
					return FALSE;
				}
				if (bySessionNum < pTocData[i].SessionNumber) {
					WriteCcdForSession(pTocData[i].SessionNumber, byMode, fpCcd);
					bySessionNum++;
				}
				OutputString(
					_T("\rChecking SubQ adr (Track) %2u/%2u"), pTocData[i].Point, pDisc->SCSI.toc.LastTrack);
			}
		}
		OutputString(_T("\n"));
		FreeAndNull(pBuf);
	}
	return TRUE;
};

VOID WriteCcdForEntry(
	PCDROM_TOC_FULL_TOC_DATA_BLOCK toc,
	UINT a,
	FILE* fpCcd
) {
	if (fpCcd) {
		_ftprintf(fpCcd,
			_T("[Entry %u]\n")
			_T("Session=%u\n")
			_T("Point=0x%02x\n")
			_T("ADR=0x%02x\n")
			_T("Control=0x%02x\n")
			_T("TrackNo=%u\n")
			_T("AMin=%u\n")
			_T("ASec=%u\n")
			_T("AFrame=%u\n")
			_T("ALBA=%d\n")
			_T("Zero=%u\n")
			_T("PMin=%u\n")
			_T("PSec=%u\n")
			_T("PFrame=%u\n")
			_T("PLBA=%d\n"),
			a,
			toc[a].SessionNumber,
			toc[a].Point,
			toc[a].Adr,
			toc[a].Control,
			toc[a].Reserved1,
			toc[a].MsfExtra[0],
			toc[a].MsfExtra[1],
			toc[a].MsfExtra[2],
			MSFtoLBA(toc[a].MsfExtra[0], toc[a].MsfExtra[1], toc[a].MsfExtra[2]) - 150,
			toc[a].Zero,
			toc[a].Msf[0],
			toc[a].Msf[1],
			toc[a].Msf[2],
			MSFtoLBA(toc[a].Msf[0], toc[a].Msf[1], toc[a].Msf[2]) - 150);
	}
}

VOID WriteCcdForTrack(
	PDISC pDisc,
	BYTE byTrackNum,
	FILE* fpCcd
) {
	if (fpCcd) {
		_ftprintf(fpCcd,
			_T("[TRACK %u]\n")
			_T("MODE=%u\n"),
			byTrackNum,
			pDisc->MAIN.lpModeList[byTrackNum - 1]);
		if (pDisc->SUB.lpISRCList[byTrackNum - 1]) {
			_TCHAR szISRC[META_ISRC_SIZE] = {};
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, 0
				, pDisc->SUB.pszISRC[byTrackNum - 1], META_ISRC_SIZE
				, szISRC, sizeof(szISRC) / sizeof(szISRC[0]));
#else
			strncpy(szISRC, pDisc->SUB.pszISRC[byTrackNum - 1], sizeof(szISRC) / sizeof(szISRC[0]));
#endif
			_ftprintf(fpCcd, _T("ISRC=%s\n"), szISRC);
		}
		switch (pDisc->SUB.lpCtlList[byTrackNum - 1] & ~AUDIO_DATA_TRACK) {
		case AUDIO_WITH_PREEMPHASIS:
			_ftprintf(fpCcd, _T("FLAGS= PRE\n"));
			break;
		case DIGITAL_COPY_PERMITTED:
			_ftprintf(fpCcd, _T("FLAGS= DCP\n"));
			break;
		case DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
			_ftprintf(fpCcd, _T("FLAGS= DCP PRE\n"));
			break;
		case TWO_FOUR_CHANNEL_AUDIO:
			_ftprintf(fpCcd, _T("FLAGS= 4CH\n"));
			break;
		case TWO_FOUR_CHANNEL_AUDIO | AUDIO_WITH_PREEMPHASIS:
			_ftprintf(fpCcd, _T("FLAGS= 4CH PRE\n"));
			break;
		case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED:
			_ftprintf(fpCcd, _T("FLAGS= 4CH DCP\n"));
			break;
		case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
			_ftprintf(fpCcd, _T("FLAGS= 4CH DCP PRE\n"));
			break;
		default:
			break;
		}
	}
}

VOID WriteCcdForTrackIndex(
	BYTE byIndex,
	INT nLBA,
	FILE* fpCcd
) {
	if (fpCcd) {
		_ftprintf(fpCcd, _T("INDEX %u=%d\n"), byIndex, nLBA);
	}
}

VOID WriteCueForSongWriter(
	PDISC pDisc,
	BYTE byIdx,
	FILE* fpCue
) {
	if (pDisc->SCSI.pszSongWriter[byIdx][0] != 0) {
		_TCHAR szSongWriter[META_CDTEXT_SIZE] = {};
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0
			, pDisc->SCSI.pszSongWriter[byIdx], META_CDTEXT_SIZE
			, szSongWriter, sizeof(szSongWriter) / sizeof(szSongWriter[0]));
#else
		strncpy(szSongWriter, pDisc->SCSI.pszSongWriter[byIdx], sizeof(szSongWriter) / sizeof(szSongWriter[0]));
#endif
		if (byIdx == 0) {
			_ftprintf(fpCue, _T("SONGWRITER \"%s\"\n"), szSongWriter);
		}
		else {
			_ftprintf(fpCue, _T("    SONGWRITER \"%s\"\n"), szSongWriter);
		}
	}
}

VOID WriteCueForPerformer(
	PDISC pDisc,
	BYTE byIdx,
	FILE* fpCue
) {
	if (pDisc->SCSI.pszPerformer[byIdx][0] != 0) {
		_TCHAR szPerformer[META_CDTEXT_SIZE] = {};
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0
			, pDisc->SCSI.pszPerformer[byIdx], META_CDTEXT_SIZE
			, szPerformer, sizeof(szPerformer) / sizeof(szPerformer[0]));
#else
		strncpy(szPerformer, pDisc->SCSI.pszPerformer[byIdx], sizeof(szPerformer) / sizeof(szPerformer[0]));
#endif
		if (byIdx == 0) {
			_ftprintf(fpCue, _T("PERFORMER \"%s\"\n"), szPerformer);
		}
		else {
			_ftprintf(fpCue, _T("    PERFORMER \"%s\"\n"), szPerformer);
		}
	}
}

VOID WriteCueForTitle(
	PDISC pDisc,
	BYTE byIdx,
	FILE* fpCue
) {
	if (pDisc->SCSI.pszTitle[byIdx][0] != 0) {
		_TCHAR szTitle[META_CDTEXT_SIZE] = {};
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0
			, pDisc->SCSI.pszTitle[byIdx], META_CDTEXT_SIZE
			, szTitle, sizeof(szTitle) / sizeof(szTitle[0]));
#else
		strncpy(szTitle, pDisc->SCSI.pszTitle[byIdx], sizeof(szTitle) / sizeof(szTitle[0]));
#endif
		if(byIdx == 0) {
			_ftprintf(fpCue, _T("TITLE \"%s\"\n"), szTitle);
		}
		else {
			_ftprintf(fpCue, _T("    TITLE \"%s\"\n"), szTitle);
		}
	}
}

VOID WriteCueForFirst(
	PDISC pDisc,
	BOOL bCanCDText,
	FILE* fpCue
) {
	if (pDisc->SUB.byCatalog) {
		_TCHAR szCatalog[META_CATALOG_SIZE] = {};
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0 
			, pDisc->SUB.szCatalog, sizeof(pDisc->SUB.szCatalog) / sizeof(pDisc->SUB.szCatalog[0])
			, szCatalog, sizeof(szCatalog) / sizeof(szCatalog[0]));
#else
		strncpy(szCatalog, pDisc->SUB.szCatalog, sizeof(szCatalog) / sizeof(szCatalog[0]));
#endif
		_ftprintf(fpCue, _T("CATALOG %s\n"), szCatalog);
	}
	if (bCanCDText) {
		WriteCueForTitle(pDisc, 0, fpCue);
		WriteCueForPerformer(pDisc, 0, fpCue);
		WriteCueForSongWriter(pDisc, 0, fpCue);
	}
}

VOID WriteCueForMultiSessionMultiBin(
	PDISC pDisc,
	BYTE byTrackNum,
	FILE* fpCue
) {
	if (pDisc->SCSI.bMultiSession) {
		if (byTrackNum == pDisc->SCSI.byFirstMultiSessionTrackNum) {
			BYTE m, s, f;
			LBAtoMSF(pDisc->SCSI.nLeadoutLenOf1stSession, &m, &s, &f);
			_ftprintf(fpCue, _T("REM LEAD-OUT %02d:%02d:%02d\n"), m, s, f); // always 01:30:00
		}
		if (byTrackNum == pDisc->SCSI.toc.FirstTrack || byTrackNum == pDisc->SCSI.byFirstMultiSessionTrackNum) {
			_ftprintf(fpCue, _T("REM SESSION %02d\n"), pDisc->SCSI.lpSessionNumList[byTrackNum - 1]);
		}
		if (byTrackNum == pDisc->SCSI.byFirstMultiSessionTrackNum) {
			BYTE m, s, f;
			LBAtoMSF(pDisc->SCSI.nLeadinLenOf2ndSession, &m, &s, &f);
			_ftprintf(fpCue, _T("REM LEAD-IN %02d:%02d:%02d\n"), m, s, f); // always 01:00:00
			LBAtoMSF(pDisc->SCSI.nPregapLenOf1stTrkOf2ndSession, &m, &s, &f);
			_ftprintf(fpCue, _T("REM PREGAP %02d:%02d:%02d\n"), m, s, f); // always 00:02:00
		}
	}
}

VOID WriteCueForMultiSessionWholeBin(
	PDISC pDisc,
	BYTE byTrackNum,
	FILE* fpCue
) {
	if (pDisc->SCSI.bMultiSession) {
		if (byTrackNum == pDisc->SCSI.byFirstMultiSessionTrackNum) {
			BYTE m, s, f;
			LBAtoMSF(pDisc->SCSI.nFirstLBAof2ndSession - SESSION_TO_SESSION_SKIP_LBA, &m, &s, &f);
			_ftprintf(fpCue, _T("  REM LEAD-OUT %02d:%02d:%02d\n"), m, s, f);
		}
		if (byTrackNum == pDisc->SCSI.toc.FirstTrack || byTrackNum == pDisc->SCSI.byFirstMultiSessionTrackNum) {
			_ftprintf(fpCue, _T("  REM SESSION %02d\n"), pDisc->SCSI.lpSessionNumList[byTrackNum - 1]);
		}
	}
}

VOID WriteCueForFileDirective(
	LPCTSTR pszPath,
	FILE* fpCue
) {
	_ftprintf(fpCue, _T("FILE \"%s\" BINARY\n"), pszPath);
}

VOID WriteCueForISRC(
	PDISC pDisc,
	INT nIdx,
	FILE* fpCue
) {
	if (pDisc->SUB.lpISRCList[nIdx]) {
		_TCHAR szISRC[META_ISRC_SIZE] = {};
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0
			, pDisc->SUB.pszISRC[nIdx], META_ISRC_SIZE
			, szISRC, sizeof(szISRC) / sizeof(szISRC[0]));
#else
		strncpy(szISRC, pDisc->SUB.pszISRC[nIdx], sizeof(szISRC) / sizeof(szISRC[0]));
#endif
		_ftprintf(fpCue, _T("    ISRC %s\n"), szISRC);
	}
}

VOID WriteCueForUnderFileDirective(
	PDISC pDisc,
	BOOL bCanCDText,
	BYTE byTrackNum,
	FILE* fpCue
) {
	if (pDisc->MAIN.lpModeList[byTrackNum - 1] == DATA_BLOCK_MODE0) {
		_ftprintf(fpCue, _T("  TRACK %02u AUDIO\n"), byTrackNum);
		WriteCueForISRC(pDisc, byTrackNum - 1, fpCue);
		if (bCanCDText) {
			WriteCueForTitle(pDisc, byTrackNum, fpCue);
			WriteCueForPerformer(pDisc, byTrackNum, fpCue);
			WriteCueForSongWriter(pDisc, byTrackNum, fpCue);
		}
		switch (pDisc->SUB.lpCtlList[byTrackNum - 1] & ~AUDIO_DATA_TRACK) {
		case AUDIO_WITH_PREEMPHASIS:
			_ftprintf(fpCue, _T("    FLAGS PRE\n"));
			break;
		case DIGITAL_COPY_PERMITTED:
			_ftprintf(fpCue, _T("    FLAGS DCP\n"));
			break;
		case DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
			_ftprintf(fpCue, _T("    FLAGS DCP PRE\n"));
			break;
		case TWO_FOUR_CHANNEL_AUDIO:
			_ftprintf(fpCue, _T("    FLAGS 4CH\n"));
			break;
		case TWO_FOUR_CHANNEL_AUDIO | AUDIO_WITH_PREEMPHASIS:
			_ftprintf(fpCue, _T("    FLAGS 4CH PRE\n"));
			break;
		case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED:
			_ftprintf(fpCue, _T("    FLAGS 4CH DCP\n"));
			break;
		case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
			_ftprintf(fpCue, _T("    FLAGS 4CH DCP PRE\n"));
			break;
		default:
			break;
		}
	}
	else {
		if (pDisc->SCSI.byFormat == DISK_TYPE_CDI) {
			_ftprintf(fpCue, _T("  TRACK %02u CDI/2352\n"), byTrackNum);
		}
		else {
			_ftprintf(fpCue, _T("  TRACK %02u MODE%1u/2352\n"), 
				byTrackNum, pDisc->MAIN.lpModeList[byTrackNum - 1]);
		}
		WriteCueForISRC(pDisc, byTrackNum - 1, fpCue);
		if ((pDisc->SUB.lpCtlList[byTrackNum - 1] & DIGITAL_COPY_PERMITTED) == DIGITAL_COPY_PERMITTED) {
			_ftprintf(fpCue, _T("    FLAGS DCP\n"));
		}
	}
}

VOID WriteCueForIndexDirective(
	BYTE byIndex,
	BYTE byMinute,
	BYTE bySecond,
	BYTE byFrame,
	FILE* fpCue
) {
	_ftprintf(fpCue, _T("    INDEX %02u %02u:%02u:%02u\n"),
		byIndex, byMinute, bySecond, byFrame);
}

VOID WriteMainChannel(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nLBA,
	FILE* fpImg
) {
	INT sLBA = pDisc->MAIN.nFixStartLBA;
	INT eLBA = pDisc->MAIN.nFixEndLBA;
	if (pExtArg->byReverse) {
		fwrite(lpBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
	}
	else if (sLBA <= nLBA && nLBA < eLBA) {
		// first sector
		if (nLBA == sLBA /*|| (nLBA == pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize + 1 &&
			(pDisc->PROTECT.byExist == laserlock || pDisc->PROTECT.byExist == proring))*/) {
			fwrite(lpBuf + pDisc->MAIN.uiMainDataSlideSize, sizeof(BYTE),
				CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize, fpImg);
			if (*pExecType != gd) {
				if (pDisc->SUB.lpFirstLBAListOnSub) {
					pDisc->SUB.lpFirstLBAListOnSub[0][0] = -150;
				}
				if (pDisc->SUB.lpFirstLBAListOnSubSync) {
					pDisc->SUB.lpFirstLBAListOnSubSync[0][0] = -150;
				}
			}
			else {
				pDisc->SUB.lpFirstLBAListOnSub[2][0] = 44850;
			}
		}
		// last sector in 1st session (when session 2 exists)
		else if (!pExtArg->byMultiSession && pDisc->SCSI.nFirstLBAof2ndSession != -1 &&
			nLBA == pDisc->MAIN.nFixFirstLBAofLeadout - 1) {
			fwrite(lpBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpImg);
		}
		// first sector in 2nd Session
		else if (!pExtArg->byMultiSession && pDisc->SCSI.nFirstLBAof2ndSession != -1 &&
			nLBA == pDisc->MAIN.nFixFirstLBAof2ndSession) {
			fwrite(lpBuf + pDisc->MAIN.uiMainDataSlideSize, sizeof(BYTE),
				CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize, fpImg);
		}
		// last sector
		else if (nLBA == eLBA - 1) {
			if (pDisc->MAIN.uiMainDataSlideSize != 0) {
				fwrite(lpBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpImg);
			}
			else {
				fwrite(lpBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
			}
		}
		else {
			fwrite(lpBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
		}
	}
}

VOID WriteC2(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nLBA,
	FILE* fpC2
) {
	INT sLBA = pDisc->MAIN.nFixStartLBA;
	INT eLBA = pDisc->MAIN.nFixEndLBA;
	UINT nC2SlideSize = pDisc->MAIN.uiMainDataSlideSize / 8;
	if (sLBA <= nLBA && nLBA < eLBA) {
		// first sector
		if (nLBA == sLBA) {
			fwrite(lpBuf + nC2SlideSize, sizeof(BYTE),
				CD_RAW_READ_C2_294_SIZE - nC2SlideSize, fpC2);
		}
		// last sector in 1st session (when exists session 2)
		else if (!pExtArg->byMultiSession && pDisc->SCSI.nFirstLBAof2ndSession != -1 &&
			nLBA == pDisc->MAIN.nFixFirstLBAofLeadout - 1) {
			fwrite(lpBuf, sizeof(BYTE), nC2SlideSize, fpC2);
		}
		// first sector in 2nd Session
		else if (!pExtArg->byMultiSession && pDisc->SCSI.nFirstLBAof2ndSession != -1 &&
			nLBA == pDisc->MAIN.nFixFirstLBAof2ndSession) {
			fwrite(lpBuf + nC2SlideSize, sizeof(BYTE),
				CD_RAW_READ_C2_294_SIZE - nC2SlideSize, fpC2);
		}
		// last sector
		else if (nLBA == eLBA - 1) {
			if (pDisc->MAIN.uiMainDataSlideSize != 0) {
				fwrite(lpBuf, sizeof(BYTE), nC2SlideSize, fpC2);
			}
			else {
				fwrite(lpBuf, sizeof(BYTE), CD_RAW_READ_C2_294_SIZE, fpC2);
			}
		}
		else {
			fwrite(lpBuf, sizeof(BYTE), CD_RAW_READ_C2_294_SIZE, fpC2);
		}
	}
}

VOID WriteSubChannel(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	LPBYTE lpSubcodeRaw,
	INT nLBA,
	FILE* fpSub
) {
	if (fpSub) {
		fwrite(pDiscPerSector->subcode.current, sizeof(BYTE), CD_RAW_READ_SUBCODE_SIZE, fpSub);
		OutputCDSubToLog(pDisc, pDiscPerSector, lpSubcodeRaw, nLBA);
	}
}

VOID WriteErrorBuffer(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	LPBYTE lpScrambledBuf,
	INT nLBA,
	INT nMainDataType,
	INT nPadType,
	FILE* fpImg,
	FILE* fpSub,
	FILE* fpC2
) {
	UINT uiSize = 0;
	BYTE zeroByte[CD_RAW_SECTOR_SIZE] = {};
	if (*pExecType == data || pExtArg->byBe) {
		uiSize = CD_RAW_SECTOR_SIZE;
		if (nPadType == padByUsr55 || nPadType == padByUsr0) {
			if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				fwrite(pDiscPerSector->mainHeader.current, sizeof(BYTE), MAINHEADER_MODE1_SIZE, fpImg);
			}
			if (nPadType == padByUsr55) {
				for (UINT i = MAINHEADER_MODE1_SIZE; i < uiSize; i++) {
					pDiscPerSector->data.current[i] = 0x55;
				}
				if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					fwrite(pDiscPerSector->data.current + MAINHEADER_MODE1_SIZE,
						sizeof(BYTE), uiSize - MAINHEADER_MODE1_SIZE, fpImg);
				}
			}
			else {
				fwrite(zeroByte, sizeof(BYTE), uiSize - MAINHEADER_MODE1_SIZE, fpImg);
			}
		}
		else {
			fwrite(zeroByte, sizeof(BYTE), uiSize, fpImg);
		}
	}
	else {
		if (nPadType == padByUsr55 || nPadType == padByUsr0) {
			ZeroMemory(pDiscPerSector->data.current, CD_RAW_SECTOR_SIZE);
		}
		if (nPadType == padByUsr55 || nPadType == padByUsr0 || nPadType == padByPrevSector) {
			BYTE tmp = 0;
			BYTE pad = 0x55;
			if (nPadType == padByUsr0) {
				pad = 0;
			}
			if (nPadType == padByUsr55 || nPadType == padByUsr0) {
				for (UINT i = 0; i < pDisc->MAIN.uiMainDataSlideSize; i++) {
					if (nMainDataType == scrambled) {
						tmp = (BYTE)(pad ^ lpScrambledBuf[CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize + i]);
					}
					else if (nMainDataType == unscrambled) {
						tmp = pad;
					}
					pDiscPerSector->data.current[i] = tmp;
				}
			}
			for (UINT i = pDisc->MAIN.uiMainDataSlideSize; i < pDisc->MAIN.uiMainDataSlideSize + MAINHEADER_MODE1_SIZE; i++) {
				pDiscPerSector->data.current[i] =
					pDiscPerSector->mainHeader.current[i - pDisc->MAIN.uiMainDataSlideSize];
			}
			if (nPadType == padByUsr55 || nPadType == padByUsr0) {
				for (UINT i = pDisc->MAIN.uiMainDataSlideSize + MAINHEADER_MODE1_SIZE; i < CD_RAW_SECTOR_SIZE; i++) {
					if (nMainDataType == scrambled) {
						tmp = (BYTE)(pad ^ lpScrambledBuf[i - pDisc->MAIN.uiMainDataSlideSize]);
					}
					else if (nMainDataType == unscrambled) {
						tmp = pad;
					}
					pDiscPerSector->data.current[i] = tmp;
				}
			}
		}
		if (nLBA == pDisc->MAIN.nFixStartLBA) {
			uiSize = CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize;
			if (nPadType == padByUsr55 || nPadType == padByUsr0 || nPadType == padByPrevSector) {
				fwrite(pDiscPerSector->data.current + pDisc->MAIN.uiMainDataSlideSize,
					sizeof(BYTE), uiSize, fpImg);
			}
			else {
				fwrite(zeroByte, sizeof(BYTE), uiSize, fpImg);
			}
		}
		else if (nLBA == pDisc->MAIN.nFixEndLBA - 1 || nLBA == pDisc->MAIN.nFixFirstLBAof2ndSession - 150) {
			uiSize = pDisc->MAIN.uiMainDataSlideSize;
			if (nPadType == padByUsr55 || nPadType == padByUsr0 || nPadType == padByPrevSector) {
				fwrite(pDiscPerSector->data.current, sizeof(BYTE), uiSize, fpImg);
			}
			else {
				fwrite(zeroByte, sizeof(BYTE), uiSize, fpImg);
			}
		}
		else {
			uiSize = CD_RAW_SECTOR_SIZE;
			if (nPadType == padByUsr55 || nPadType == padByUsr0 || nPadType == padByPrevSector) {
				fwrite(pDiscPerSector->data.current, sizeof(BYTE), uiSize, fpImg);
			}
			else {
				fwrite(zeroByte, sizeof(BYTE), uiSize, fpImg);
			}
		}
	}
	OutputLogA(fileMainError,
		"LBA[%06d, %#07x] Read error. padding [%ubyte]\n", nLBA, nLBA, uiSize);

	if (*pExecType != swap || (*pExecType == swap && nLBA < SECOND_ERROR_OF_LEADOUT)) {
		BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = {};
		if (pDiscPerSector->subQ.current.byIndex == 0) {
			pDiscPerSector->subQ.current.nRelativeTime--;
		}
		else {
			pDiscPerSector->subQ.current.nRelativeTime++;
		}
		pDiscPerSector->subQ.current.nAbsoluteTime++;
		SetBufferFromTmpSubQData(pDiscPerSector->subcode.current, pDiscPerSector->subQ.current, TRUE, TRUE);
		AlignColumnSubcode(lpSubcodeRaw, pDiscPerSector->subcode.current);
		WriteSubChannel(pDisc, pDiscPerSector, lpSubcodeRaw, nLBA, fpSub);

		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			fwrite(pDiscPerSector->data.current + pDevice->TRANSFER.uiBufC2Offset
				, sizeof(BYTE), CD_RAW_READ_C2_294_SIZE, fpC2);
		}
	}
}

BOOL WriteParsingSubfile(
	LPCTSTR pszSubfile
) {
	BOOL bRet = TRUE;
#ifndef _DEBUG
	if (NULL == (g_LogFile.fpSubReadable = CreateOrOpenFileA(
		pszSubfile, "_subReadable", NULL, NULL, NULL, ".txt", "w", 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
#endif
	LPBYTE data = NULL;
	DISC discData = {};
	DISC_PER_SECTOR discPerSector = {};
	FILE* fpSub = NULL;
	UINT uiTrackAllocSize = MAXIMUM_NUMBER_TRACKS + 10 + 1;

	try {
#ifndef _DEBUG
		if (NULL == (g_LogFile.fpSubError = CreateOrOpenFileA(
			pszSubfile, "_subError", NULL, NULL, NULL, ".txt", "w", 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
#endif
		if (NULL == (fpSub = CreateOrOpenFile(
			pszSubfile, NULL, NULL, NULL, NULL, _T(".sub"), _T("rb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		DWORD dwFileSize = GetFileSize(0, fpSub);
		if (NULL == (data = (LPBYTE)calloc(dwFileSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		size_t uiReadSize = fread(data, sizeof(BYTE), dwFileSize, fpSub);
		FcloseAndNull(fpSub);

		if (uiReadSize < CD_RAW_READ_SUBCODE_SIZE) {
			throw FALSE;
		}
		if (NULL == (discData.SUB.pszISRC =
			(LPSTR*)calloc(uiTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		for (UINT h = 0; h < uiTrackAllocSize; h++) {
			if (NULL == (discData.SUB.pszISRC[h] =
				(LPSTR)calloc((META_ISRC_SIZE), sizeof(_TCHAR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}
		// TODO: doesn't use RtoW in current
		BYTE lpSubcodeRtoW[CD_RAW_READ_SUBCODE_SIZE] = {};
		BYTE byTrackNum = 1;
		BYTE byPrevTrackNum = 1;
		INT nLBA = 0;

		for (INT i = 0; i < (INT)dwFileSize; i += CD_RAW_READ_SUBCODE_SIZE) {
			memcpy(discPerSector.subcode.current, data + i, CD_RAW_READ_SUBCODE_SIZE);
			BYTE byAdr = (BYTE)(discPerSector.subcode.current[12] & 0x0f);
			if (byAdr == ADR_ENCODES_CURRENT_POSITION) {
				byTrackNum = BcdToDec(discPerSector.subcode.current[13]);
				nLBA = MSFtoLBA(BcdToDec(discPerSector.subcode.current[19]),
					BcdToDec(discPerSector.subcode.current[20]), BcdToDec(discPerSector.subcode.current[21])) - 150;
				if (byTrackNum == 0) {
					byTrackNum = BcdToDec(discPerSector.subcode.current[14]);
					nLBA = MSFtoLBA(BcdToDec(discPerSector.subcode.current[15]),
						BcdToDec(discPerSector.subcode.current[16]), BcdToDec(discPerSector.subcode.current[17])) - 150;
				}
			}
			else if (byAdr == ADR_ENCODES_MEDIA_CATALOG) {
				SetMCNToString(&discData, &data[i],	discData.SUB.szCatalog, FALSE);
				nLBA++;
			}
			else if (byAdr == ADR_ENCODES_ISRC) {
				if (0 < byPrevTrackNum && byPrevTrackNum < uiTrackAllocSize) {
					SetISRCToString(&discData, &discPerSector, 
						discData.SUB.pszISRC[byPrevTrackNum - 1], FALSE);
				}
				nLBA++;
			}
			else if (byAdr == 5) {
				byTrackNum = BcdToDec(*(data + i + 13));
				if (byTrackNum == 0) {
					byTrackNum = BcdToDec(*(data + i + 14));
					nLBA = -151; // TMP because can't get lba
				}
			}
			byPrevTrackNum = byTrackNum;
			OutputCDSubToLog(&discData, &discPerSector, lpSubcodeRtoW, nLBA);
			OutputString(
				_T("\rParsing sub (Size) %8d/%8lu"), i + CD_RAW_READ_SUBCODE_SIZE, dwFileSize);
		}
		OutputString(_T("\n"));
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FcloseAndNull(fpSub);
#ifndef _DEBUG
	FcloseAndNull(g_LogFile.fpSubReadable);
	FcloseAndNull(g_LogFile.fpSubError);
#endif
	for (UINT i = 0; i < uiTrackAllocSize; i++) {
		if (discData.SUB.pszISRC) {
			FreeAndNull(discData.SUB.pszISRC[i]);
		}
	}
	FreeAndNull(discData.SUB.pszISRC);
	FreeAndNull(data);
	return bRet;
}

BOOL WriteParsingMdsfile(
	LPCTSTR pszMdsfile
) {
	BOOL bRet = TRUE;
	FILE* fpParse = CreateOrOpenFile(
		pszMdsfile, _T("_mdsReadable"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0);
	if (!fpParse) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	FILE* fpMds = NULL;
	LPBYTE data = NULL;
	try {
		if (NULL == (fpMds = CreateOrOpenFile(
			pszMdsfile, NULL, NULL, NULL, NULL, _T(".mds"), _T("rb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		DWORD dwFileSize = GetFileSize(0, fpMds);
		if (NULL == (data = (LPBYTE)calloc(dwFileSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (fread(data, sizeof(BYTE), dwFileSize, fpMds) < dwFileSize) {
			OutputErrorString(_T("Failed to read [F:%s][L:%d]\n"), _T(__FUNCTION__), __LINE__);
			FcloseAndNull(fpMds);
			throw FALSE;
		};
		FcloseAndNull(fpMds);

		MDS_HEADER h = {};
		size_t nOfs = 0;
		size_t size = sizeof(MDS_HEADER);
		memcpy(&h, data, size);
		nOfs += size;

		PMDS_FOR_DVD_BLK dvd = {};
		BYTE layer = 1;
		if (h.mediaType == 0x10) {
			if ((data[nOfs + 2054] >> 5 & 0x03) == 1) {
				layer = 2;
			}
			if (NULL == (dvd = (PMDS_FOR_DVD_BLK)calloc(layer, sizeof(MDS_FOR_DVD_BLK)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			size = layer * sizeof(MDS_FOR_DVD_BLK);
			memcpy(dvd, data + nOfs, size);
			nOfs += size;
		}

		PMDS_SESSION_BLK psb = NULL;
		if (NULL == (psb = (PMDS_SESSION_BLK)calloc(h.sessionNum, sizeof(MDS_SESSION_BLK)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		size = h.sessionNum * sizeof(MDS_SESSION_BLK);
		memcpy(psb, data + nOfs, size);
		nOfs += size;

		WORD tdb = psb->totalDataBlkNum;
		if (h.sessionNum > 1) {
			tdb = (WORD)(tdb + psb[1].totalDataBlkNum);
		}
		PMDS_DATA_BLK db = NULL;
		if (NULL == (db = (PMDS_DATA_BLK)calloc(tdb, sizeof(MDS_DATA_BLK)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		size = tdb * sizeof(MDS_DATA_BLK);
		memcpy(db, data + nOfs, size);
		nOfs += size;

		PMDS_IDX_BLK ib = NULL;
		if (h.mediaType != 0x10) {
			if (NULL == (ib = (PMDS_IDX_BLK)calloc(tdb, sizeof(MDS_IDX_BLK)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			size = tdb * sizeof(MDS_IDX_BLK);
			memcpy(ib, data + nOfs, size);
			nOfs += size;
		}

		MDS_FNAME_BLK fb = {};
		size = sizeof(MDS_FNAME_BLK);
		memcpy(&fb, data + nOfs, size);
		nOfs += size;

		PMDS_DPM_HEADER pdb = NULL;
		PMDS_DPM_BLK* pddb = NULL;
		
		if (h.ofsToDpm > 0) {
			size_t allocsize = 4 + data[h.ofsToDpm] * sizeof(UINT);
			if (NULL == (pdb = (PMDS_DPM_HEADER)calloc(allocsize, sizeof(UCHAR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			size = allocsize;
			memcpy(pdb, data + nOfs, size);
			nOfs += size;

			if (NULL == (pddb = (PMDS_DPM_BLK*)calloc(pdb->dpmBlkTotalNum, sizeof(PMDS_DPM_BLK*)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			for (UINT i = 0; i < pdb->dpmBlkTotalNum; i++) {
				UINT entry = MAKEUINT(MAKEWORD(data[nOfs + 12], data[nOfs + 13]), MAKEWORD(data[nOfs + 14], data[nOfs + 15]));
				allocsize = 16 + entry * sizeof(UINT);
				if (NULL == (pddb[i] = (PMDS_DPM_BLK)calloc(allocsize, sizeof(UCHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				size = allocsize;
				memcpy(pddb[i], data + nOfs, size);
				nOfs += size;
			}
		}
		_ftprintf(fpParse,
			_T(OUTPUT_DHYPHEN_PLUS_STR(Header))
			_T("                  id: %.16s\n")
			_T("             unknown: %d\n")
			_T("           mediaType: %d\n")
			_T("          sessionNum: %d\n")
			_T("             unknown: %d\n")
			_T("            LenOfBca: %d\n")
			_T("            ofsToBca: %d\n")
			_T("  ofsTo1stSessionBlk: %d\n")
			_T("            ofsToDpm: %d\n")
			, h.fileId, h.unknown1, h.mediaType, h.sessionNum
			, h.unknown2, h.lenOfBca, h.ofsToBca
			, h.ofsTo1stSessionBlk, h.ofsToDpm
		);
		if (h.mediaType == 0x10) {
			LPCSTR lpBookType[] = {
				"DVD-ROM", "DVD-RAM", "DVD-R", "DVD-RW",
				"HD DVD-ROM", "HD DVD-RAM", "HD DVD-R", "Reserved",
				"Reserved", "DVD+RW", "DVD+R", "Reserved",
				"Reserved", "DVD+RW DL", "DVD+R DL", "Reserved"
			};

			LPCSTR lpMaximumRate[] = {
				"2.52 Mbps", "5.04 Mbps", "10.08 Mbps", "20.16 Mbps",
				"30.24 Mbps", "Reserved", "Reserved", "Reserved",
				"Reserved", "Reserved", "Reserved", "Reserved",
				"Reserved", "Reserved", "Reserved", "Not Specified"
			};

			LPCSTR lpLayerType[] = {
				"Unknown", "Layer contains embossed data", "Layer contains recordable data", "Unknown",
				"Layer contains rewritable data", "Unknown", "Unknown", "Unknown",
				"Reserved", "Unknown", "Unknown", "Unknown",
				"Unknown", "Unknown", "Unknown", "Unknown"
			};

			LPCSTR lpTrackDensity[] = {
				"0.74um/track", "0.80um/track", "0.615um/track", "0.40um/track",
				"0.34um/track", "Reserved", "Reserved", "Reserved",
				"Reserved", "Reserved", "Reserved", "Reserved",
				"Reserved", "Reserved", "Reserved", "Reserved"
			};

			LPCSTR lpLinearDensity[] = {
				"0.267um/bit", "0.293um/bit", "0.409 to 0.435um/bit", "Reserved",
				"0.280 to 0.291um/bit", "0.153um/bit", "0.130 to 0.140um/bit", "Reserved",
				"0.353um/bit", "Reserved", "Reserved", "Reserved",
				"Reserved", "Reserved", "Reserved", "Reserved"
			};
			for (INT i = 0; i < layer; i++) {
				_ftprintf(fpParse, _T(OUTPUT_DHYPHEN_PLUS_STR(BCA)));
				for (size_t k = 0; k < sizeof(dvd[i].bca); k += 16) {
					_ftprintf(fpParse,
						_T("%04zX : %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X\n")
						, k, dvd[i].bca[k], dvd[i].bca[k + 1], dvd[i].bca[k + 2], dvd[i].bca[k + 3], dvd[i].bca[k + 4], dvd[i].bca[k + 5]
						, dvd[i].bca[k + 6], dvd[i].bca[k + 7], dvd[i].bca[k + 8], dvd[i].bca[k + 9], dvd[i].bca[k + 10], dvd[i].bca[k + 11]
						, dvd[i].bca[k + 12], dvd[i].bca[k + 13], dvd[i].bca[k + 14], dvd[i].bca[k + 15]);
				}
				DWORD dwStartingDataSector = dvd[i].layer.commonHeader.StartingDataSector;
				DWORD dwEndDataSector = dvd[i].layer.commonHeader.EndDataSector;
				DWORD dwEndLayerZeroSector = dvd[i].layer.commonHeader.EndLayerZeroSector;
				REVERSE_LONG(&dwStartingDataSector);
				REVERSE_LONG(&dwEndDataSector);
				REVERSE_LONG(&dwEndLayerZeroSector);
				_ftprintf(fpParse,
					_T(OUTPUT_DHYPHEN_PLUS_STR(DVD Structure))
					"\t       BookVersion: %u\n"
					"\t          BookType: %s\n"
					"\t       MinimumRate: %s\n"
					"\t          DiskSize: %s\n"
					"\t         LayerType: %s\n"
					"\t         TrackPath: %s\n"
					"\t    NumberOfLayers: %s\n"
					"\t      TrackDensity: %s\n"
					"\t     LinearDensity: %s\n"
					"\tStartingDataSector: %7lu (%#lx)\n"
					"\t     EndDataSector: %7lu (%#lx)\n"
					"\tEndLayerZeroSector: %7lu (%#lx)\n"
					"\t           BCAFlag: %s\n"
					"\t     MediaSpecific: \n",
					dvd[i].layer.commonHeader.BookVersion,
					lpBookType[dvd[i].layer.commonHeader.BookType],
					lpMaximumRate[dvd[i].layer.commonHeader.MinimumRate],
					dvd[i].layer.commonHeader.DiskSize == 0 ? _T("120mm") : _T("80mm"),
					lpLayerType[dvd[i].layer.commonHeader.LayerType],
					dvd[i].layer.commonHeader.TrackPath == 0 ? _T("Parallel Track Path") : _T("Opposite Track Path"),
					dvd[i].layer.commonHeader.NumberOfLayers == 0 ? _T("Single Layer") : _T("Double Layer"),
					lpTrackDensity[dvd[i].layer.commonHeader.TrackDensity],
					lpLinearDensity[dvd[i].layer.commonHeader.LinearDensity],
					dwStartingDataSector, dwStartingDataSector,
					dwEndDataSector, dwEndDataSector,
					dwEndLayerZeroSector, dwEndLayerZeroSector,
					dvd[i].layer.commonHeader.BCAFlag == 0 ? "No" : "Exist");
				for (size_t j = 0; j < sizeof(dvd[i].layer.MediaSpecific); j += 16) {
					_ftprintf(fpParse,
						_T("%04X : %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X\n")
						, i, dvd[i].layer.MediaSpecific[j], dvd[i].layer.MediaSpecific[j + 1], dvd[i].layer.MediaSpecific[j + 2]
						, dvd[i].layer.MediaSpecific[j + 3], dvd[i].layer.MediaSpecific[j + 4], dvd[i].layer.MediaSpecific[j + 5]
						, dvd[i].layer.MediaSpecific[j + 6], dvd[i].layer.MediaSpecific[j + 7], dvd[i].layer.MediaSpecific[j + 8]
						, dvd[i].layer.MediaSpecific[j + 9], dvd[i].layer.MediaSpecific[j + 10], dvd[i].layer.MediaSpecific[j + 11]
						, dvd[i].layer.MediaSpecific[j + 12], dvd[i].layer.MediaSpecific[j + 13], dvd[i].layer.MediaSpecific[j + 14]
						, dvd[i].layer.MediaSpecific[j + 15]);
				}
			}
		}
		for (INT i = 0; i < h.sessionNum; i++) {
			_ftprintf(fpParse,
				_T(OUTPUT_DHYPHEN_PLUS_STR(SessionBlock))
				_T("         startSector: %d\n")
				_T("           endSector: %d\n")
				_T("          sessionNum: %d\n")
				_T("     totalDataBlkNum: %d\n")
				_T("          DataBlkNum: %d\n")
				_T("       firstTrackNum: %d\n")
				_T("        lastTrackNum: %d\n")
				_T("     ofsTo1stDataBlk: %d\n")
				, psb[i].startSector, psb[i].endSector, psb[i].sessionNum
				, psb[i].totalDataBlkNum, psb[i].DataBlkNum, psb[i].firstTrackNum
				, psb[i].lastTrackNum, psb[i].ofsTo1stDataBlk
			);
		}
		for (INT i = 0; i < tdb; i++) {
			_ftprintf(fpParse,
				OUTPUT_DHYPHEN_PLUS_STR(DataBlock)
				_T("           trackMode: %d\n")
				_T("          numOfSubch: %d\n")
				_T("              adrCtl: %d\n")
				_T("            trackNum: %d\n")
				_T("               point: %d\n")
				_T("                 msf: %02d:%02d:%02d\n")
				_T("       ofsToIndexBlk: %d\n")
				_T("          sectorSize: %d\n")
				_T("             unknown: %d\n")
				_T("    trackStartSector: %d\n")
				_T("   ofsFromHeadToIdx1: %d\n")
				_T("             unknown: %d\n")
				_T("          NumOfFname: %d\n")
				_T("          OfsToFname: %d\n")
				, db[i].trackMode, db[i].numOfSubch, db[i].adrCtl, db[i].trackNum
				, db[i].point, db[i].m, db[i].s, db[i].f, db[i].ofsToIndexBlk
				, db[i].sectorSize, db[i].unknown1, db[i].trackStartSector
				, db[i].ofsFromHeadToIdx1, db[i].unknown2, db[i].NumOfFname, db[i].OfsToFname
			);
		}
		if (h.mediaType != 0x10) {
			for (INT i = 0; i < tdb; i++) {
				_ftprintf(fpParse,
					OUTPUT_DHYPHEN_PLUS_STR(IndexBlock)
					_T("           NumOfIdx0: %d\n")
					_T("           NumOfIdx1: %d\n")
					, ib[i].NumOfIdx0, ib[i].NumOfIdx1
				);
			}
		}
		char fname[12];
		if (!WideCharToMultiByte(CP_ACP, 0,
			fb.fnameString, 6, fname, sizeof(fname), NULL, NULL)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		}
		_ftprintf(fpParse,
			_T(OUTPUT_DHYPHEN_PLUS_STR(Fname))
			_T("          ofsToFname: %d\n")
			_T("            fnameFmt: %d\n")
			_T("         fnameString: %s\n")
			, fb.ofsToFname, fb.fnameFmt, fname
		);
		if (h.ofsToDpm > 0) {
			_ftprintf(fpParse,
				OUTPUT_DHYPHEN_PLUS_STR(DPM)
				_T("      dpmBlkTotalNum: %d\n")
				, pdb->dpmBlkTotalNum);
			for (UINT i = 0; i < pdb->dpmBlkTotalNum; i++) {
				_ftprintf(fpParse,
					_T("        ofsToDpmInfo: %d\n"), pdb->ofsToDpmBlk[i]);
			}
			for (UINT i = 0; i < pdb->dpmBlkTotalNum; i++) {
				LPUINT diff = NULL;
				if (NULL == (diff = (LPUINT)calloc(pddb[i]->entry, sizeof(UINT)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				_ftprintf(fpParse,
					_T("           dpmBlkNum: %d\n")
					_T("            unknown1: %d\n")
					_T("          resolution: %d\n")
					_T("               entry: %d\n")
					, pddb[i]->dpmBlkNum, pddb[i]->unknown1, pddb[i]->resolution, pddb[i]->entry
				);
				_ftprintf(fpParse,
					_T("       0    readTime:                       %5d ms\n"), pddb[i]->readTime[0]);
				diff[0] = pddb[i]->readTime[0];

				BOOL bStart = FALSE;
				BOOL bEnd = TRUE;
				INT prevDiff = 0;
//				INT prevDiffAndDiff = 0;
				for (UINT k = 1; k < pddb[i]->entry; k++) {
					diff[k] = pddb[i]->readTime[k] - pddb[i]->readTime[k - 1];
					INT diffAndDiff = (INT)(diff[k] - diff[k - 1]);
					if (pddb[i]->resolution == 50 || pddb[i]->resolution == 256) {
						INT orgDiff = prevDiff - (INT)diff[k];
						if (((pddb[i]->resolution == 50 && abs(orgDiff) < 10) ||
							(pddb[i]->resolution == 256 && abs(orgDiff) < 15)) && bStart && !bEnd) {
							_ftprintf(fpParse, _T(" end changing\n"));
							bStart = FALSE;
							bEnd = TRUE;
						}
						else if (k > 1) {
							_ftprintf(fpParse, _T("\n"));
						}
					}
					_ftprintf(fpParse,
						_T("%8d    readTime: %8d - %8d = %5d ms [%d]")
						, pddb[i]->resolution * k, pddb[i]->readTime[k], pddb[i]->readTime[k - 1], diff[k], diffAndDiff);
//					prevDiffAndDiff = diffAndDiff;
					if (pddb[i]->resolution == 50 || pddb[i]->resolution == 256) {
						// SecuROM
						if (9 < diffAndDiff && diffAndDiff < 35 && !bStart && bEnd) {
							_ftprintf(fpParse, _T(" start changing"));
							bStart = TRUE;
							bEnd = FALSE;
						}
						else if (!bStart && bEnd) {
							prevDiff = (INT)diff[k];
						}
					}
					else if (pddb[i]->resolution == 500 || pddb[i]->resolution == 2048) {
						// Starforce
						_ftprintf(fpParse, _T("\n"));
					}
					else {
						_ftprintf(fpParse, _T("\n"));
					}
				}
				_ftprintf(fpParse, _T("\n"));
				FreeAndNull(diff);
			}
		}
		FreeAndNull(data);
		FreeAndNull(psb);
		FreeAndNull(ib);
		FreeAndNull(db);
		FreeAndNull(dvd);
		for (UINT i = 0; i < pdb->dpmBlkTotalNum; i++) {
			FreeAndNull(pddb[i]);
		}
		FreeAndNull(pddb);
		FreeAndNull(pdb);
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FcloseAndNull(fpParse);
	FcloseAndNull(fpMds);
	return bRet;
}

BOOL DescrambleMainChannelForGD(
	LPCTSTR pszPath,
	LPTSTR pszOutPath
) {
	BOOL bRet = TRUE;
	FILE* fpScm = CreateOrOpenFile(
		pszPath, NULL, NULL, NULL, NULL, _T(".scm"), _T("rb"), 0, 0);
	if (!fpScm) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fpImg = CreateOrOpenFile(
		pszPath, NULL, pszOutPath, NULL, NULL, _T(".img"), _T("wb"), 0, 0);
	if (!fpImg) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FcloseAndNull(fpScm);
		return FALSE;
	}
	DWORD dwFileSize = GetFileSize(0, fpScm);
	DWORD dwAllSectorVal = dwFileSize / CD_RAW_SECTOR_SIZE;
	BYTE bufScm[CD_RAW_SECTOR_SIZE] = {};
	BYTE bufImg[CD_RAW_SECTOR_SIZE] = {};
	for (DWORD i = 0; i < dwAllSectorVal; i++) {
		if (fread(bufScm, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpScm) < CD_RAW_SECTOR_SIZE) {
			OutputErrorString(_T("Failed to read\n"));
			break;
		}
		if (IsValidMainDataHeader(bufScm)) {
			if (bufScm[0x0C] == 0xC3 && bufScm[0x0D] == 0x84/* && bufScm[0x0E] >= 0x00*/) {
				break;
			}
			for (INT j = 0; j < CD_RAW_SECTOR_SIZE; j++) {
				bufImg[j] = (BYTE)(bufScm[j] ^ scrambled_table[j]);
			}
			fwrite(bufImg, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
		}
		else {
			// copy audio data
			fwrite(bufScm, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
		}
		OutputString(_T("\rDescrambling img (LBA) %6lu/%6lu"), i, dwAllSectorVal);
	}
	OutputString(_T("\n"));
	FcloseAndNull(fpImg);
	FcloseAndNull(fpScm);
	return bRet;
}

BOOL CreateBinCueForGD(
	PDISC pDisc,
	LPCTSTR pszPath
) {
	BOOL bRet = TRUE;
	_TCHAR pszImgName[_MAX_FNAME] = {};
	_TCHAR pszFname[_MAX_PATH] = {};
	FILE* fpImg = CreateOrOpenFile(pszPath, NULL,
		NULL, pszImgName, pszFname, _T(".img"), _T("rb"), 0, 0);
	if (!fpImg) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fpBin = NULL;
	FILE* fpGdi = NULL;
	FILE* fpCue = NULL;
	FILE* fpCueForImg = NULL;
	LPLONG lpToc = NULL;
	LPBYTE lpBuf = NULL;
	try {
		if (NULL == (fpGdi = CreateOrOpenFile(pszPath, NULL,
			NULL, NULL, NULL, _T(".gdi"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpCue = CreateOrOpenFile(pszPath, NULL,
			NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpCueForImg = CreateOrOpenFile(pszPath, _T("_img"),
			NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		DWORD dwFileSize = GetFileSize(0, fpImg);
		if (dwFileSize < 0x110 + 512) {
			OutputErrorString(_T("No GD-ROM data. Size: %lu\n"), dwFileSize);
			throw FALSE;
		}
		fseek(fpImg, 0x110, SEEK_SET);
		// 0x110 - 0x31F is toc data
		BYTE aToc[512] = {};
		if (fread(aToc, sizeof(BYTE), sizeof(aToc), fpImg) < sizeof(aToc)) {
			OutputErrorString(_T("Failed to read [F:%s][L:%d]\n"), _T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (aToc[0] != 'T' || aToc[1] != 'O' || aToc[2] != 'C' || aToc[3] != '1') {
			OutputErrorString(_T("No GD-ROM data. Header: %c%c%c%c\n"),
				aToc[0], aToc[1], aToc[2], aToc[3]);
			throw FALSE;
		}

		UINT uiMaxToc = 98 * 4;
		BYTE byMaxTrackNum = aToc[uiMaxToc + 4 * 1 + 2];
		lpToc = (PLONG)calloc(byMaxTrackNum, sizeof(UINT));
		if (!lpToc) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LONG lMaxLBA = 0;
		for (UINT i = 0; i < uiMaxToc; i += 4) {
			if (aToc[7 + i] == 0xff) {
				lMaxLBA = MAKELONG(
					MAKEWORD(aToc[4 + i - 4], aToc[5 + i - 4]), MAKEWORD(aToc[6 + i - 4], 0));
				break;
			}
		}
		_ftprintf(fpGdi, _T("%u\n"), byMaxTrackNum);
		if (byMaxTrackNum <= 9 && lMaxLBA <= 99999) {
			_ftprintf(fpGdi,
				_T("1 %5d 4 2352 \"%s (Track %u).bin\" 0\n")
				_T("2 [fix] 0 2352 \"%s (Track %u).bin\" 0\n"),
				0, pszFname, 1,	pszFname, 2);
		}
		else if (10 <= byMaxTrackNum && lMaxLBA <= 99999) {
			_ftprintf(fpGdi,
				_T(" 1 %5d 4 2352 \"%s (Track %02u).bin\" 0\n")
				_T(" 2 [fix] 0 2352 \"%s (Track %02u).bin\" 0\n"),
				0, pszFname, 1, pszFname, 2);
		}
		else if (byMaxTrackNum <= 9 && 100000 <= lMaxLBA) {
			_ftprintf(fpGdi,
				_T("1 %6d 4 2352 \"%s (Track %u).bin\" 0\n")
				_T("2  [fix] 0 2352 \"%s (Track %u).bin\" 0\n"),
				0, pszFname, 1,	pszFname, 2);
		}
		else if (10 <= byMaxTrackNum && 100000 <= lMaxLBA) {
			_ftprintf(fpGdi,
				_T(" 1 %6d 4 2352 \"%s (Track %02u).bin\" 0\n")
				_T(" 2  [fix] 0 2352 \"%s (Track %02u).bin\" 0\n"),
				0, pszFname, 1,	pszFname, 2);
		}

		BYTE byTrackNum = 3;
		for (UINT i = 0; i < uiMaxToc; i += 4, byTrackNum++) {
			if (aToc[7 + i] == 0xff) {
				break;
			}
			BYTE byCtl = (BYTE)((aToc[7 + i] >> 4) & 0x0f);
			LONG lToc =
				MAKELONG(MAKEWORD(aToc[4 + i], aToc[5 + i]), MAKEWORD(aToc[6 + i], 0));
			lpToc[byTrackNum - 3] = lToc - 300;
			if (byTrackNum == 3) {
				lpToc[byTrackNum - 3] += 150;
			}
			else {
				if ((byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					lpToc[byTrackNum - 3] -= 75;
				}
			}
			if (byMaxTrackNum <= 9 && lMaxLBA <= 99999) {
				_ftprintf(fpGdi, _T("%u %5ld %u 2352 \"%s (Track %u).bin\" 0\n"),
					byTrackNum, lpToc[byTrackNum - 3], byCtl, pszFname, byTrackNum);
			}
			else if (10 <= byMaxTrackNum && lMaxLBA <= 99999) {
				_ftprintf(fpGdi, _T("%2u %5ld %u 2352 \"%s (Track %02u).bin\" 0\n"),
					byTrackNum, lpToc[byTrackNum - 3], byCtl, pszFname, byTrackNum);
			}
			else if (byMaxTrackNum <= 9 && 100000 <= lMaxLBA) {
				_ftprintf(fpGdi, _T("%u %6ld %u 2352 \"%s (Track %u).bin\" 0\n"),
					byTrackNum, lpToc[byTrackNum - 3], byCtl, pszFname, byTrackNum);
			}
			else if (10 <= byMaxTrackNum && 100000 <= lMaxLBA) {
				_ftprintf(fpGdi, _T("%2u %6ld %u 2352 \"%s (Track %02u).bin\" 0\n"),
					byTrackNum, lpToc[byTrackNum - 3], byCtl, pszFname, byTrackNum);
			}
		}
		LONG lToc = 
			MAKELONG(MAKEWORD(aToc[uiMaxToc + 4 * 2], aToc[uiMaxToc + 4 * 2 + 1]),
			MAKEWORD(aToc[uiMaxToc + 4 * 2 + 2], 0)) - 150;
		lpToc[byTrackNum - 3] = lToc;

		rewind(fpImg);
		WriteCueForFileDirective(pszImgName, fpCueForImg);
		_TCHAR pszBinFname[_MAX_PATH] = {};

		for (BYTE i = 3; i <= byMaxTrackNum; i++) {
			OutputString(_T("\rCreating bin, cue (Track) %2u/%2u"), i, byMaxTrackNum);
			if (NULL == (fpBin = CreateOrOpenFile(pszPath, NULL, NULL,
				pszBinFname, NULL, _T(".bin"), _T("wb"), i, byMaxTrackNum))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			WriteCueForUnderFileDirective(pDisc, FALSE, i, fpCueForImg);
			WriteCueForFileDirective(pszBinFname, fpCue);
			WriteCueForUnderFileDirective(pDisc, FALSE, i, fpCue);

			BYTE index = 0;
			INT nLBAofFirstIdx = pDisc->SUB.lpFirstLBAListOnSub[i - 1][0] - FIRST_LBA_FOR_GD;
			// nothing or index 0 in track 1
			if (nLBAofFirstIdx == -1 || nLBAofFirstIdx == -150) {
				nLBAofFirstIdx = pDisc->SUB.lpFirstLBAListOnSub[i - 1][1] - FIRST_LBA_FOR_GD;
				index++;
			}

			BYTE byFrame = 0, bySecond = 0, byMinute = 0;
			if (i == 3) {
				if (0 == nLBAofFirstIdx || i == byMaxTrackNum) {
					WriteCueForIndexDirective(index, 0, 0, 0, fpCueForImg);
					WriteCueForIndexDirective(index, 0, 0, 0, fpCue);
				}
				else if (0 < nLBAofFirstIdx) {
					// index 0 in track 1
					//  Crow, The - Original Motion Picture Soundtrack (82519-2)
					//  Now on Never (Nick Carter) (ZJCI-10118)
					//  SaGa Frontier Original Sound Track (Disc 3)
					//  etc..
					WriteCueForIndexDirective(0, 0, 0, 0, fpCueForImg);
					WriteCueForIndexDirective(0, 0, 0, 0, fpCue);

					LBAtoMSF(nLBAofFirstIdx, &byMinute, &bySecond, &byFrame);
					WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueForImg);
					WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCue);
				}
				index++;
			}

			for (; index < MAXIMUM_NUMBER_INDEXES; index++) {
				INT nLBAofNextIdx = pDisc->SUB.lpFirstLBAListOnSub[i - 1][index] - FIRST_LBA_FOR_GD;
				if (nLBAofNextIdx != -1 - FIRST_LBA_FOR_GD) {
					LBAtoMSF(nLBAofNextIdx, &byMinute, &bySecond, &byFrame);
					WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueForImg);

					LBAtoMSF(nLBAofNextIdx - nLBAofFirstIdx, &byMinute, &bySecond, &byFrame);
					WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCue);
				}
				else {
					if (index >= 2) {
						break;
					}
				}
			}
			size_t size = (size_t)(lpToc[i - 2] - lpToc[i - 3]) * CD_RAW_SECTOR_SIZE;
			if (NULL == (lpBuf = (LPBYTE)calloc(size, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (fread(lpBuf, sizeof(BYTE), size, fpImg) < size) {
				OutputErrorString(_T("Failed to read [L:%d]\n"), __LINE__);
			}
			fwrite(lpBuf, sizeof(BYTE), size, fpBin);
			FcloseAndNull(fpBin);
			FreeAndNull(lpBuf);
		}
		OutputString(_T("\n"));
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpImg);
	FcloseAndNull(fpBin);
	FcloseAndNull(fpCue);
	FcloseAndNull(fpCueForImg);
	FcloseAndNull(fpGdi);
	FreeAndNull(lpToc);
	return bRet;
}

BOOL IsValidReservedByte(
	LPBYTE aSrcBuf
) {
	BOOL bRet = FALSE;
	if (aSrcBuf[0x814] == 0x00 && aSrcBuf[0x815] == 0x00 && aSrcBuf[0x816] == 0x00 &&
		aSrcBuf[0x817] == 0x00 && aSrcBuf[0x818] == 0x00 && aSrcBuf[0x819] == 0x00 &&
		aSrcBuf[0x81a] == 0x00 && aSrcBuf[0x81b] == 0x00) {
		bRet = TRUE;
	}
	return bRet;
}

VOID DescrambleMainChannelAll(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpScrambledBuf,
	FILE* fpImg
) {
	BYTE aSrcBuf[CD_RAW_SECTOR_SIZE] = {};
	LONG lSeekPtr = 0;

	for (INT k = pDisc->SCSI.byFirstDataTrackNum - 1; k < pDisc->SCSI.byLastDataTrackNum; k++) {
		INT nFirstLBA = pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[k];
		if (nFirstLBA != -1) {
			INT nLastLBA = pDisc->SUB.lpLastLBAListOfDataTrackOnSub[k];
			OutputDiscLogA("\tTrack %2u Data Sector: %6d - %6d (%#07x - %#07x)\n",
				k + 1, nFirstLBA, nLastLBA, nFirstLBA, nLastLBA);
			if (!pExtArg->byMultiSession && pDisc->SCSI.lpSessionNumList[k] >= 2) {
				INT nSkipLBA = (SESSION_TO_SESSION_SKIP_LBA * (INT)(pDisc->SCSI.lpSessionNumList[k] - 1));
				nFirstLBA -= nSkipLBA;
				nLastLBA -= nSkipLBA;
			}
			if (pExtArg->byPre) {
//				nFirstLBA += 75;
				nLastLBA += 150;
			}
			if (!pExtArg->byReverse) {
				lSeekPtr = nFirstLBA;
			}
			for (; nFirstLBA <= nLastLBA; nFirstLBA++, lSeekPtr++) {
				// ファイルを読み書き両用モードで開いている時は 注意が必要です。
				// 読み込みを行った後に書き込みを行う場合やその逆を行う場合は、 
				// 必ずfseekを呼ばなければなりません。もしこれを忘れると、
				// 場合によってはバッファー内と 実際にディスクに描き込まれた
				// データに矛盾が生じ、正確に書き込まれない場合や、
				// 嘘の データを読み込む場合があります。
				fseek(fpImg, lSeekPtr * CD_RAW_SECTOR_SIZE, SEEK_SET);
				if (fread(aSrcBuf, sizeof(BYTE), sizeof(aSrcBuf), fpImg) < sizeof(aSrcBuf)) {
					OutputErrorString(_T("LBA[%06d, %#07x]: Failed to read [F:%s][L:%d]\n")
						,nFirstLBA, nFirstLBA, _T(__FUNCTION__), __LINE__);
					break;
				}
				if (IsValidMainDataHeader(aSrcBuf)) {
					if (aSrcBuf[0x0f] == 0x60) {
						for (INT n = 0x10; n < CD_RAW_SECTOR_SIZE; n++) {
							if (aSrcBuf[n] != lpScrambledBuf[n]) {
								OutputMainErrorWithLBALogA("Not all zero sector\n", nFirstLBA, k + 1);
								OutputString(
									_T("\rDescrambling data sector of img: %6d/%6d"), nFirstLBA, nLastLBA);
								OutputCDMain(fileMainError, aSrcBuf, nFirstLBA, CD_RAW_SECTOR_SIZE);
								continue;
							}
						}
					}
					else if (aSrcBuf[0x0f] == 0x61) {
						if (IsValidReservedByte(aSrcBuf)) {
							OutputMainErrorWithLBALogA("A part of reversed sector. (Not be scrambled)\n", nFirstLBA, k + 1);
							OutputCDMain(fileMainError, aSrcBuf, nFirstLBA, CD_RAW_SECTOR_SIZE);
						}
					}
					else if (aSrcBuf[0x0f] == 0x00 || aSrcBuf[0x0f] == 0x01 || aSrcBuf[0x0f] == 0x02) {
						OutputMainErrorWithLBALogA("Reversed sector. (Not be scrambled)\n", nFirstLBA, k + 1);
						OutputCDMain(fileMainError, aSrcBuf, nFirstLBA, CD_RAW_SECTOR_SIZE);
					}
					else if (aSrcBuf[0x0f] != 0x60 && aSrcBuf[0x0f] != 0x61 && aSrcBuf[0x0f] != 0x62 &&
						aSrcBuf[0x0f] != 0x00 && aSrcBuf[0x0f] != 0x01 && aSrcBuf[0x0f] != 0x02) {
						OutputMainErrorWithLBALogA("Invalid mode. ", nFirstLBA, k + 1);
						BYTE m, s, f = 0;
						LBAtoMSF(nFirstLBA + 150, &m, &s, &f);
						if (aSrcBuf[0x0c] == m && aSrcBuf[0x0d] == s && aSrcBuf[0x0e] == f) {
							OutputMainErrorLogA("Reversed sector. (Not be scrambled)\n");
							if (!IsValidReservedByte(aSrcBuf)) {
								OutputMainErrorLogA("Invalid reserved byte. Skip descrambling\n");
								OutputString(
									_T("\rDescrambling data sector of img: %6d/%6d"), nFirstLBA, nLastLBA);
								OutputCDMain(fileMainError, aSrcBuf, nFirstLBA, CD_RAW_SECTOR_SIZE);
								continue;
							}
						}
						else if (IsValidReservedByte(aSrcBuf)) {
							OutputMainErrorLogA("A part of reversed sector. (Not be scrambled)\n");
						}
						else if (aSrcBuf[0x814] != 0x48 || aSrcBuf[0x815] != 0x64 || aSrcBuf[0x816] != 0x36 ||
							aSrcBuf[0x817] != 0xab || aSrcBuf[0x818] != 0x56 || aSrcBuf[0x819] != 0xff ||
							aSrcBuf[0x81a] != 0x7e || aSrcBuf[0x81b] != 0xc0) {
							OutputMainErrorLogA("Invalid reserved byte. Skip descrambling\n");
							OutputString(
								_T("\rDescrambling data sector of img: %6d/%6d"), nFirstLBA, nLastLBA);
							OutputCDMain(fileMainError, aSrcBuf, nFirstLBA, CD_RAW_SECTOR_SIZE);
							continue;
						}
						else {
							OutputMainErrorLogA("\n");
						}
						OutputCDMain(fileMainError, aSrcBuf, nFirstLBA, CD_RAW_SECTOR_SIZE);
					}
					fseek(fpImg, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
					for (INT n = 0; n < CD_RAW_SECTOR_SIZE; n++) {
						aSrcBuf[n] ^= lpScrambledBuf[n];
					}
					fwrite(aSrcBuf, sizeof(BYTE), sizeof(aSrcBuf), fpImg);
				}
				else {
					if (pDisc->SCSI.trackType != TRACK_TYPE::pregapAudioIn1stTrack &&
						pDisc->SCSI.trackType != TRACK_TYPE::pregapDataIn1stTrack) {
						OutputMainErrorWithLBALogA("Invalid sync. Skip descrambling\n", nFirstLBA, k + 1);
						OutputCDMain(fileMainError, aSrcBuf, nFirstLBA, CD_RAW_SECTOR_SIZE);
					}
				}
				OutputString(
					_T("\rDescrambling data sector of img: %6d/%6d"), nFirstLBA, nLastLBA);
			}
			OutputString(_T("\n"));
		}
	}
}

VOID DescrambleMainChannelPartial(
	INT nStartLBA,
	INT nEndLBA,
	LPBYTE lpScrambledBuf,
	FILE* fpImg
) {
	BYTE aSrcBuf[CD_RAW_SECTOR_SIZE] = {};
	LONG lSeekPtr = 0;

	for (; nStartLBA <= nEndLBA; nStartLBA++, lSeekPtr++) {
		// ファイルを読み書き両用モードで開いている時は 注意が必要です。
		// 読み込みを行った後に書き込みを行う場合やその逆を行う場合は、 
		// 必ずfseekを呼ばなければなりません。もしこれを忘れると、
		// 場合によってはバッファー内と 実際にディスクに描き込まれた
		// データに矛盾が生じ、正確に書き込まれない場合や、
		// 嘘の データを読み込む場合があります。
		fseek(fpImg, lSeekPtr * CD_RAW_SECTOR_SIZE, SEEK_SET);
		if (fread(aSrcBuf, sizeof(BYTE), sizeof(aSrcBuf), fpImg) < sizeof(aSrcBuf)) {
			OutputErrorString(_T("Failed to read [F:%s][L:%d]\n"), _T(__FUNCTION__), __LINE__);
			break;
		}
		if (IsValidMainDataHeader(aSrcBuf)) {
			if (aSrcBuf[0x0f] == 0x61 || aSrcBuf[0x0f] == 0x62) {
				fseek(fpImg, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
				for (INT n = 0; n < CD_RAW_SECTOR_SIZE; n++) {
					aSrcBuf[n] ^= lpScrambledBuf[n];
				}
				fwrite(aSrcBuf, sizeof(BYTE), sizeof(aSrcBuf), fpImg);
			}
			else {
				OutputMainInfoWithLBALogA("Invalid mode. Skip descrambling\n", nStartLBA, 0);
				OutputCDMain(fileMainInfo, aSrcBuf, nStartLBA, CD_RAW_SECTOR_SIZE);
			}
		}
		else {
			OutputMainErrorWithLBALogA("Invalid sync. Skip descrambling\n", nStartLBA, 0);
			OutputCDMain(fileMainError, aSrcBuf, nStartLBA, CD_RAW_SECTOR_SIZE);
		}
		OutputString(
			_T("\rDescrambling data sector of img: %6d/%6d"), nStartLBA, nEndLBA);
	}
	OutputString(_T("\n"));
}

BOOL CreateBin(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	BYTE byTrackNum,
	INT nLBA,
	INT nPrevLBA,
	FILE* fpImg,
	FILE* fpBin
) {
	size_t stBufSize = 0;

	if (pDisc->SCSI.toc.LastTrack == pDisc->SCSI.toc.FirstTrack) {
		stBufSize = (size_t)pDisc->SCSI.nAllLength * CD_RAW_SECTOR_SIZE;
		nPrevLBA = 0;
	}
	else if (byTrackNum == pDisc->SCSI.toc.FirstTrack) {
		stBufSize = (size_t)nLBA * CD_RAW_SECTOR_SIZE;
		// last 1st session track
		if (pDisc->SCSI.lpSessionNumList[byTrackNum - 1] != pDisc->SCSI.lpSessionNumList[byTrackNum]) {
			stBufSize -= (SESSION_TO_SESSION_SKIP_LBA - 150) * CD_RAW_SECTOR_SIZE;
		}
		nPrevLBA = 0;
	}
	else if (byTrackNum == pDisc->SCSI.toc.LastTrack) {
		INT nTmpLength = pDisc->SCSI.nAllLength;
		// first 2nd session track
		if (pDisc->SCSI.lpSessionNumList[byTrackNum - 1] >= 2 &&
			pDisc->SCSI.lpSessionNumList[byTrackNum - 2] == 1) {
			INT nSessionSize =
				SESSION_TO_SESSION_SKIP_LBA * (pDisc->SCSI.lpSessionNumList[byTrackNum - 1] - 1);
			if (!pExtArg->byMultiSession) {
				nPrevLBA -= nSessionSize;
				nTmpLength -= nSessionSize;
			}
			else {
				// ignore index 0
				nPrevLBA += 150;
			}
		}
		stBufSize = (size_t)(nTmpLength - nPrevLBA) * CD_RAW_SECTOR_SIZE;
	}
	else {
		INT nSessionSize =
			SESSION_TO_SESSION_SKIP_LBA * (pDisc->SCSI.lpSessionNumList[byTrackNum] - 1);
		// last 1st session track
		if (pDisc->SCSI.lpSessionNumList[byTrackNum - 1] != pDisc->SCSI.lpSessionNumList[byTrackNum]) {
			nLBA -= nSessionSize;
			if (pExtArg->byMultiSession) {
				// ignore index 0
				nLBA += 150;
			}
		}
		// first 2nd session track
		else if (pDisc->SCSI.lpSessionNumList[byTrackNum - 1] >= 2 &&
			pDisc->SCSI.lpSessionNumList[byTrackNum - 2] == 1) {
			if (!pExtArg->byMultiSession) {
				nPrevLBA -= nSessionSize;
				nLBA -= nSessionSize;
			}
			else {
				// ignore index 0
				nPrevLBA += 150;
			}
		}
		stBufSize = (size_t)(nLBA - nPrevLBA) * CD_RAW_SECTOR_SIZE;
	}
	fseek(fpImg, nPrevLBA * CD_RAW_SECTOR_SIZE, SEEK_SET);
	LPBYTE lpBuf = (LPBYTE)calloc(stBufSize, sizeof(BYTE));
	if (!lpBuf) {
		OutputString(_T("\n"));
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("bufSize: %zd", stBufSize);
		return FALSE;
	}
	if (fread(lpBuf, sizeof(BYTE), stBufSize, fpImg) < stBufSize) {
		OutputErrorString(_T("Failed to read: read size %zd [F:%s][L:%d]\n"), stBufSize, _T(__FUNCTION__), __LINE__);
		FreeAndNull(lpBuf);
		return FALSE;
	}
	fwrite(lpBuf, sizeof(BYTE), stBufSize, fpBin);
	FreeAndNull(lpBuf);

	return TRUE;
}

BOOL CreateBinCueCcd(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPCTSTR pszPath,
	LPCTSTR pszImgName,
	BOOL bCanCDText,
	FILE* fpImg,
	FILE* fpCue,
	FILE* fpCueForImg,
	FILE* fpCcd
) {
	WriteCueForFirst(pDisc, bCanCDText, fpCueForImg);
	WriteCueForFileDirective(pszImgName, fpCueForImg);
	WriteCueForFirst(pDisc, bCanCDText, fpCue);

	FILE* fpCueSyncForImg = NULL;
	FILE* fpCueSync = NULL;
	if (pDisc->SUB.byDesync) {
		if (NULL == (fpCueSyncForImg = CreateOrOpenFile(
			pszPath, _T(" (Subs indexes)_img"), NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (NULL == (fpCueSync = CreateOrOpenFile(
			pszPath, _T(" (Subs indexes)"), NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			FcloseAndNull(fpCueSyncForImg);
			return FALSE;
		}
		WriteCueForFirst(pDisc, bCanCDText, fpCueSyncForImg);
		WriteCueForFileDirective(pszImgName, fpCueSyncForImg);
		WriteCueForMultiSessionWholeBin(pDisc, 1, fpCueSyncForImg);
		WriteCueForFirst(pDisc, bCanCDText, fpCueSync);
	}

	BOOL bRet = TRUE;
	_TCHAR pszFname[_MAX_FNAME] = {};
	FILE* fpBin = NULL;
	FILE* fpBinSync = NULL;
	for (BYTE i = pDisc->SCSI.toc.FirstTrack; i <= pDisc->SCSI.toc.LastTrack; i++) {
		OutputString(
			_T("\rCreating bin, cue and ccd (Track) %2u/%2u"), i, pDisc->SCSI.toc.LastTrack);
		if (NULL == (fpBin = CreateOrOpenFile(pszPath, NULL, NULL, pszFname,
			NULL, _T(".bin"), _T("wb"), i, pDisc->SCSI.toc.LastTrack))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			bRet = FALSE;
			break;
		}
		WriteCueForMultiSessionWholeBin(pDisc, i, fpCueForImg);
		WriteCueForUnderFileDirective(pDisc, bCanCDText, i, fpCueForImg);
		WriteCueForMultiSessionMultiBin(pDisc, i, fpCue);
		WriteCueForFileDirective(pszFname, fpCue);
		WriteCueForUnderFileDirective(pDisc, bCanCDText, i, fpCue);
		WriteCcdForTrack(pDisc, i, fpCcd);

		_TCHAR pszFnameSync[_MAX_FNAME] = {};
		if (pDisc->SUB.byDesync) {
			if (NULL == (fpBinSync = CreateOrOpenFile(pszPath, _T(" (Subs indexes)"), NULL,
				pszFnameSync, NULL, _T(".bin"), _T("wb"), i, pDisc->SCSI.toc.LastTrack))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				bRet = FALSE;
				break;
			}
			WriteCueForUnderFileDirective(pDisc, bCanCDText, i, fpCueSyncForImg);
			WriteCueForFileDirective(pszFnameSync, fpCueSync);
			WriteCueForUnderFileDirective(pDisc, bCanCDText, i, fpCueSync);
		}

		BYTE index = 0;
		INT nLBAofFirstIdx = pDisc->SUB.lpFirstLBAListOnSub[i - 1][0];
		// nothing or index 0 in track 1
		if (nLBAofFirstIdx == -1 || nLBAofFirstIdx == -150 ||
			(pDisc->SCSI.bMultiSession && i == pDisc->SCSI.byFirstMultiSessionTrackNum)) {
			nLBAofFirstIdx = pDisc->SUB.lpFirstLBAListOnSub[i - 1][1];
			index++;
		}

		BYTE indexSync = 0;
		INT nLBAofFirstIdxSync = pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][0];
		if (nLBAofFirstIdxSync == -1 || nLBAofFirstIdxSync == -150) {
			nLBAofFirstIdxSync = pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][1];
			indexSync++;
		}

		BYTE byFrame = 0, bySecond = 0, byMinute = 0;
		if (i == pDisc->SCSI.toc.FirstTrack ||
			(pDisc->SCSI.bMultiSession && i == pDisc->SCSI.byFirstMultiSessionTrackNum)) {
			if (0 == nLBAofFirstIdx || (i == pDisc->SCSI.byFirstMultiSessionTrackNum &&
				pDisc->SCSI.trackType != TRACK_TYPE::pregapAudioIn1stTrack &&
				pDisc->SCSI.trackType != TRACK_TYPE::pregapDataIn1stTrack)) {

				WriteCueForIndexDirective(index, 0, 0, 0, fpCue);
				if (pDisc->SCSI.bMultiSession && i == pDisc->SCSI.byFirstMultiSessionTrackNum) {
					LBAtoMSF(nLBAofFirstIdx, &byMinute, &bySecond, &byFrame);
					WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueForImg);
					WriteCcdForTrackIndex(index, nLBAofFirstIdx, fpCcd);
				}
				else {
					WriteCueForIndexDirective(index, 0, 0, 0, fpCueForImg);
					WriteCcdForTrackIndex(index, 0, fpCcd);
				}
			}
			else if (0 < nLBAofFirstIdx) {
				// index 0 in track 1
				//  Crow, The - Original Motion Picture Soundtrack (82519-2)
				//  Now on Never (Nick Carter) (ZJCI-10118)
				//  SaGa Frontier Original Sound Track (Disc 3)
				//  etc..
				WriteCueForIndexDirective(0, 0, 0, 0, fpCueForImg);
				WriteCueForIndexDirective(0, 0, 0, 0, fpCue);
				WriteCcdForTrackIndex(0, 0, fpCcd);

				LBAtoMSF(nLBAofFirstIdx, &byMinute, &bySecond, &byFrame);
				WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueForImg);
				WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCue);
				WriteCcdForTrackIndex(index, nLBAofFirstIdx, fpCcd);
			}
			index++;

			if (pDisc->SUB.byDesync) {
				if (0 == nLBAofFirstIdxSync || (i == pDisc->SCSI.byFirstMultiSessionTrackNum &&
					pDisc->SCSI.trackType != TRACK_TYPE::pregapAudioIn1stTrack &&
					pDisc->SCSI.trackType != TRACK_TYPE::pregapDataIn1stTrack)) {

					WriteCueForIndexDirective(indexSync, 0, 0, 0, fpCueSyncForImg);
					WriteCueForIndexDirective(indexSync, 0, 0, 0, fpCueSync);
				}
				else if (0 < nLBAofFirstIdxSync) {
					WriteCueForIndexDirective(0, 0, 0, 0, fpCueSyncForImg);
					WriteCueForIndexDirective(0, 0, 0, 0, fpCueSync);

					LBAtoMSF(nLBAofFirstIdxSync, &byMinute, &bySecond, &byFrame);
					WriteCueForIndexDirective(indexSync, byMinute, bySecond, byFrame, fpCueSyncForImg);
					WriteCueForIndexDirective(indexSync, byMinute, bySecond, byFrame, fpCueSync);
				}
				indexSync++;
			}
		}

		for (; index < MAXIMUM_NUMBER_INDEXES; index++) {
			INT nLBAofNextIdx = pDisc->SUB.lpFirstLBAListOnSub[i - 1][index];
			if (nLBAofNextIdx != -1) {
				LBAtoMSF(nLBAofNextIdx,	&byMinute, &bySecond, &byFrame);
				WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueForImg);

				LBAtoMSF(nLBAofNextIdx - nLBAofFirstIdx, &byMinute, &bySecond, &byFrame);
				WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCue);

				WriteCcdForTrackIndex(index, nLBAofNextIdx, fpCcd);
			}
			else {
				if (index >= 2) {
					break;
				}
			}
		}
		if (pDisc->SUB.byDesync) {
			for (; indexSync < MAXIMUM_NUMBER_INDEXES; indexSync++) {
				INT nLBAofNextIdxSync = pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][indexSync];
				if (nLBAofNextIdxSync != -1) {
					LBAtoMSF(nLBAofNextIdxSync, &byMinute, &bySecond, &byFrame);
					WriteCueForIndexDirective(indexSync, byMinute, bySecond, byFrame, fpCueSyncForImg);

					LBAtoMSF(nLBAofNextIdxSync - nLBAofFirstIdxSync, &byMinute, &bySecond, &byFrame);
					WriteCueForIndexDirective(indexSync, byMinute, bySecond, byFrame, fpCueSync);
				}
				else {
					if (indexSync >= 2) {
						break;
					}
				}
			}
		}
		// write each track
		INT nLBA = pDisc->SUB.lpFirstLBAListOnSub[i - 1][0] == -1 ?
			pDisc->SUB.lpFirstLBAListOnSub[i - 1][1] : pDisc->SUB.lpFirstLBAListOnSub[i - 1][0];
		if (pExtArg->byPre) {
			if (i == pDisc->SCSI.toc.FirstTrack) {
				nLBA += 150 - abs(pDisc->MAIN.nAdjustSectorNum);
			}
			else {
				nLBA += 150;
			}
			if (i == pDisc->SCSI.toc.LastTrack) {
				pDisc->SCSI.nAllLength += 150;
			}
		}
		INT nNextLBA = pDisc->SUB.lpFirstLBAListOnSub[i][0] == -1 ?
			pDisc->SUB.lpFirstLBAListOnSub[i][1] : pDisc->SUB.lpFirstLBAListOnSub[i][0];
		if (pExtArg->byPre) {
			nNextLBA += 150;
		}
#ifdef _DEBUG
		OutputDebugStringExA(" nNextLBA(%d) - nLBA(%d) = %d\n", nNextLBA, nLBA, nNextLBA - nLBA);
#endif
		bRet = CreateBin(pExtArg, pDisc, i, nNextLBA, nLBA, fpImg, fpBin);
		FcloseAndNull(fpBin);
		if (!bRet) {
			break;
		}
		if (pDisc->SUB.byDesync) {
			nLBA = pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][0] == -1 ?
				pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][1] : pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][0];
			if (pExtArg->byPre) {
				if (i == pDisc->SCSI.toc.FirstTrack) {
					nLBA += 150 - abs(pDisc->MAIN.nAdjustSectorNum);
				}
				else {
					nLBA += 150;
				}
				if (i == pDisc->SCSI.toc.LastTrack) {
					pDisc->SCSI.nAllLength += 150;
				}
			}
			nNextLBA = pDisc->SUB.lpFirstLBAListOnSubSync[i][0] == -1 ?
				pDisc->SUB.lpFirstLBAListOnSubSync[i][1] : pDisc->SUB.lpFirstLBAListOnSubSync[i][0];
			if (pExtArg->byPre) {
				nNextLBA += 150;
			}
			bRet = CreateBin(pExtArg, pDisc, i, nNextLBA, nLBA, fpImg, fpBinSync);
			FcloseAndNull(fpBinSync);
			if (!bRet) {
				break;
			}
		}
	}
	OutputString(_T("\n"));
	FcloseAndNull(fpBinSync);
	FcloseAndNull(fpBin);
	FcloseAndNull(fpCueSyncForImg);
	FcloseAndNull(fpCueSync);
	return bRet;
}

VOID OutputIntentionalSubchannel(
	INT nLBA,
	LPBYTE lpSubcode
) {
	BYTE m, s, f;
	LBAtoMSF(nLBA + 150, &m, &s, &f);
	OutputSubIntentionalLogA(
		"MSF: %02d:%02d:%02d Q-Data: %02x%02x%02x %02x:%02x:%02x %02x %02x:%02x:%02x %02x%02x\n"
		, m, s, f, lpSubcode[0], lpSubcode[1], lpSubcode[2], lpSubcode[3], lpSubcode[4], lpSubcode[5]
		, lpSubcode[6], lpSubcode[7], lpSubcode[8], lpSubcode[9], lpSubcode[10], lpSubcode[11]
	);

}

VOID OutputHashData(
	FILE* fpHash,
	LPCTSTR filename,
	UINT64 ui64FileSize,
	DWORD crc32,
	LPBYTE digest,
	LPBYTE Message_Digest
) {
	_ftprintf(fpHash,
		_T("\t\t<rom name=\"%s\" size=\"%llu\" crc=\"%08lx\" md5=\""),
		filename, ui64FileSize, crc32);
	for (INT i = 0; i < 16; i++) {
		_ftprintf(fpHash, _T("%02x"), digest[i]);
	}
	_ftprintf(fpHash, _T("\" sha1=\""));
	for (INT i = 0; i < 20; i++) {
		_ftprintf(fpHash, _T("%02x"), Message_Digest[i]);
	}
	_ftprintf(fpHash, _T("\"/>\n"));
}

VOID OutputLastErrorNumAndString(
	LPCTSTR pszFuncName,
	LONG lLineNum
) {
#ifdef _WIN32
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	// http://blog.livedoor.jp/afsoft/archives/52230222.html
	OutputErrorString(_T("[F:%s][L:%lu] GetLastError: %lu, %s\n"),
		pszFuncName, lLineNum, GetLastError(), (LPCTSTR)lpMsgBuf);

	LocalFree(lpMsgBuf);
#else
	OutputErrorString(_T("[F:%s][L:%lu] GetLastError: %lu, %s\n"),
		pszFuncName, lLineNum, GetLastError(), strerror(GetLastError()));
#endif
}
#ifdef _WIN32
VOID OutputProductType(
	DWORD dwProductType
) {
	switch (dwProductType) {
	case PRODUCT_ULTIMATE:
		OutputString(_T("Ultimate"));
		break;
	case PRODUCT_HOME_BASIC:
		OutputString(_T("Home Basic"));
		break;
	case PRODUCT_HOME_PREMIUM:
		OutputString(_T("Home Premium"));
		break;
	case PRODUCT_ENTERPRISE:
		OutputString(_T("Enterprise"));
		break;
	case PRODUCT_HOME_BASIC_N:
		OutputString(_T("Home Basic N"));
		break;
	case PRODUCT_BUSINESS:
		OutputString(_T("Business"));
		break;
	case PRODUCT_STANDARD_SERVER:
		OutputString(_T("Standard"));
		break;
	case PRODUCT_DATACENTER_SERVER:
		OutputString(_T("Datacenter"));
		break;
	case PRODUCT_SMALLBUSINESS_SERVER:
		OutputString(_T("Small Business"));
		break;
	case PRODUCT_ENTERPRISE_SERVER:
		OutputString(_T("Enterprise"));
		break;
	case PRODUCT_STARTER:
		OutputString(_T("Starter"));
		break;
	case PRODUCT_DATACENTER_SERVER_CORE:
		OutputString(_T("Datacenter (core installation)"));
		break;
	case PRODUCT_STANDARD_SERVER_CORE:
		OutputString(_T("Standard (core installation)"));
		break;
	case PRODUCT_ENTERPRISE_SERVER_CORE:
		OutputString(_T("Enterprise (core installation)"));
		break;
	case PRODUCT_ENTERPRISE_SERVER_IA64:
		OutputString(_T("Enterprise for Itanium-based Systems"));
		break;
	case PRODUCT_BUSINESS_N:
		OutputString(_T("Business N"));
		break;
	case PRODUCT_WEB_SERVER:
		OutputString(_T("Web"));
		break;
	case PRODUCT_CLUSTER_SERVER:
		OutputString(_T("Cluster"));
		break;
	case PRODUCT_HOME_SERVER:
		OutputString(_T("Home"));
		break;
	case PRODUCT_STORAGE_EXPRESS_SERVER:
		OutputString(_T("Storage Express"));
		break;
	case PRODUCT_STORAGE_STANDARD_SERVER:
		OutputString(_T("Strage Standard"));
		break;
	case PRODUCT_STORAGE_WORKGROUP_SERVER:
		OutputString(_T("Storage Workgroup"));
		break;
	case PRODUCT_STORAGE_ENTERPRISE_SERVER:
		OutputString(_T("Storage Enterprise"));
		break;
	case PRODUCT_SERVER_FOR_SMALLBUSINESS:
		OutputString(_T("for Small Business"));
		break;
	case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
		OutputString(_T("Small Business Premium"));
		break;
	case PRODUCT_HOME_PREMIUM_N:
		OutputString(_T("Home Premium N"));
		break;
	case PRODUCT_ENTERPRISE_N:
		OutputString(_T("Enterprise N"));
		break;
	case PRODUCT_ULTIMATE_N:
		OutputString(_T("Ultimate N"));
		break;
	case PRODUCT_WEB_SERVER_CORE:
		OutputString(_T("Web (core installation)"));
		break;
	case PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:
		OutputString(_T("Medium Business Server Management"));
		break;
	case PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:
		OutputString(_T("Medium Business Server Security"));
		break;
	case PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:
		OutputString(_T("Medium Business Server Messaging"));
		break;
	case PRODUCT_SERVER_FOUNDATION:
		OutputString(_T("Foundation"));
		break;
	case PRODUCT_HOME_PREMIUM_SERVER:
		OutputString(_T("Home Premium"));
		break;
	case PRODUCT_SERVER_FOR_SMALLBUSINESS_V:
		OutputString(_T("for Small Business without Hyper-V"));
		break;
	case PRODUCT_STANDARD_SERVER_V:
		OutputString(_T("Standard without Hyper-V"));
		break;
	case PRODUCT_DATACENTER_SERVER_V:
		OutputString(_T("Datacenter without Hyper-V (full installation)"));
		break;
	case PRODUCT_ENTERPRISE_SERVER_V:
		OutputString(_T("Enterprise without Hyper-V (full installation)"));
		break;
	case PRODUCT_DATACENTER_SERVER_CORE_V:
		OutputString(_T("Datacenter without Hyper-V (core installation)"));
		break;
	case PRODUCT_STANDARD_SERVER_CORE_V:
		OutputString(_T("Standard without Hyper-V (core installation)"));
		break;
	case PRODUCT_ENTERPRISE_SERVER_CORE_V:
		OutputString(_T("Enterprise without Hyper-V (core installation)"));
		break;
	case PRODUCT_HYPERV:
		OutputString(_T("without Hyper-V"));
		break;
	case PRODUCT_STORAGE_EXPRESS_SERVER_CORE:
		OutputString(_T("Storage Express (core installation)"));
		break;
	case PRODUCT_STORAGE_STANDARD_SERVER_CORE:
		OutputString(_T("Storage Strandard (core installation)"));
		break;
	case PRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
		OutputString(_T("Storage Workgroup (core installation)"));
		break;
	case PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
		OutputString(_T("Storage Enterprise (core installation)"));
		break;
	case PRODUCT_STARTER_N:
		OutputString(_T("Starter N"));
		break;
	case PRODUCT_PROFESSIONAL:
		OutputString(_T("Professional"));
		break;
	case PRODUCT_PROFESSIONAL_N:
		OutputString(_T("Professional N"));
		break;
	case PRODUCT_SB_SOLUTION_SERVER:
		OutputString(_T("SB Solution"));
		break;
	case PRODUCT_SERVER_FOR_SB_SOLUTIONS:
		OutputString(_T("for SB Solutions"));
		break;
	case PRODUCT_STANDARD_SERVER_SOLUTIONS:
		OutputString(_T("Standard Server Solutions"));
		break;
	case PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE:
		OutputString(_T("Standard Server Solutions (core installation)"));
		break;
	case PRODUCT_SB_SOLUTION_SERVER_EM:
		OutputString(_T("SB Solutions Server EM"));
		break;
	case PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM:
		OutputString(_T("for SB Solutions EM"));
		break;
	case PRODUCT_SOLUTION_EMBEDDEDSERVER:
		OutputString(_T("Solution Embedded Server"));
		break;
	case PRODUCT_SOLUTION_EMBEDDEDSERVER_CORE:
		OutputString(_T("Solution Embedded Server (core installation)"));
		break;
	case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE:
		OutputString(_T("Small Business Server Premium (core installation)"));
		break;
	case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT:
		OutputString(_T("Essential Business Server MGMT"));
		break;
	case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL:
		OutputString(_T("Essential Business Server ADDL"));
		break;
	case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC:
		OutputString(_T("Essential Business Server MGMTSVC"));
		break;
	case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC:
		OutputString(_T("Essential Business Server ADDLSVC"));
		break;
	case PRODUCT_CLUSTER_SERVER_V:
		OutputString(_T("Cluster without Hyper-V"));
		break;
	case PRODUCT_EMBEDDED:
		OutputString(_T("Embedded"));
		break;
	case PRODUCT_STARTER_E:
		OutputString(_T("Starter E"));
		break;
	case PRODUCT_HOME_BASIC_E:
		OutputString(_T("Home Basic E"));
		break;
	case PRODUCT_HOME_PREMIUM_E:
		OutputString(_T("Home Premium E"));
		break;
	case PRODUCT_PROFESSIONAL_E:
		OutputString(_T("Professional E"));
		break;
	case PRODUCT_ENTERPRISE_E:
		OutputString(_T("Enterprise E"));
		break;
	case PRODUCT_ULTIMATE_E:
		OutputString(_T("Ultimate E"));
		break;
	default:
		OutputString(_T("Other"));
		break;
	}
}

BOOL GetOSVersion(
	LPOSVERSIONINFOEX lpOSver,
	LPDWORD lpdwProductType
) {
	BOOL(CALLBACK* pfnGetProductInfo)
		(DWORD dwOSMajorVersion, DWORD dwOSMinorVersion,
			DWORD dwSpMajorVersion, DWORD dwSpMinorVersion, PDWORD pdwReturnedProductType);
	HMODULE	hModule = ::LoadLibrary(_T("kernel32.dll"));
	if (!hModule) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	(*(FARPROC*)&pfnGetProductInfo) = ::GetProcAddress(hModule, "GetProductInfo");
	if (!pfnGetProductInfo) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	pfnGetProductInfo(lpOSver->dwMajorVersion, lpOSver->dwMinorVersion,
		lpOSver->wServicePackMajor, lpOSver->wServicePackMinor, lpdwProductType);
	if (!::FreeLibrary(hModule)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	return TRUE;
}

BOOL OutputWindowsVersion(
	VOID
) {
	DWORD dwProductType = PRODUCT_UNDEFINED;
	OSVERSIONINFOEX OSver;
	OSver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (!GetVersionEx((LPOSVERSIONINFO)&OSver)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	OutputString(_T("OS\n"));
	switch (OSver.dwMajorVersion) {
	case 5:
		switch (OSver.dwMinorVersion) {
		case 0:
			OutputString(_T("\tWindows 2000 "));
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				OutputString(_T("Professional"));
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				if (OSver.wSuiteMask & VER_SUITE_ENTERPRISE) {
					OutputString(_T("Advanced Server"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_DATACENTER) {
					OutputString(_T("Datacenter Server"));
				}
				else {
					OutputString(_T("Server"));
				}
				break;
			}
			break;
		case 1:
			OutputString(_T("\tWindows XP "));
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				if (OSver.wSuiteMask & VER_SUITE_PERSONAL) {
					OutputString(_T("Home Edition"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_EMBEDDEDNT) {
					OutputString(_T("Embedded"));
				}
				else {
					OutputString(_T("Professional"));
				}
				break;
			}
			break;
		case 2:
			OutputString(_T("\tWindows Server 2003 "));
			switch (OSver.wProductType) {
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				if (OSver.wSuiteMask & VER_SUITE_SMALLBUSINESS) {
					OutputString(_T("Small Business Server"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_ENTERPRISE) {
					OutputString(_T("Enterprise Edition"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED) {
					OutputString(_T("Small Business Server with the restrictive client license"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_DATACENTER) {
					OutputString(_T("Datacenter Edition"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_BLADE) {
					OutputString(_T("Web Edition"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_STORAGE_SERVER) {
					OutputString(_T("Storage Server Edition"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_COMPUTE_SERVER) {
					OutputString(_T("Compute Cluster Edition"));
				}
				else {
					OutputString(_T("Other"));
				}
				break;
			}
			break;
		}
		break;
	case 6:
		if (!GetOSVersion(&OSver, &dwProductType)) {
			return FALSE;
		}
		switch (OSver.dwMinorVersion) {
		case 0:
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				OutputString(_T("\tWindows Vista "));
				OutputProductType(dwProductType);
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				OutputString(_T("\tWindows Server 2008 "));
				OutputProductType(dwProductType);
				break;
			}
			break;
		case 1:
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				OutputString(_T("\tWindows 7 "));
				OutputProductType(dwProductType);
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				OutputString(_T("\tWindows Server 2008 R2 "));
				OutputProductType(dwProductType);
				break;
			}
			break;
		case 2:
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				OutputString(_T("\tWindows 8 "));
				OutputProductType(dwProductType);
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				OutputString(_T("\tWindows Server 2012 "));
				OutputProductType(dwProductType);
				break;
			}
			break;
		case 3:
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				OutputString(_T("\tWindows 8.1 "));
				OutputProductType(dwProductType);
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				OutputString(_T("\tWindows Server 2012 R2 "));
				OutputProductType(dwProductType);
				break;
			}
			break;
		}
		break;
	case 10:
		if (!GetOSVersion(&OSver, &dwProductType)) {
			return FALSE;
		}
		switch (OSver.dwMinorVersion) {
		case 0:
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				OutputString(_T("\tWindows 10 "));
				OutputProductType(dwProductType);
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				OutputString(_T("\tWindows Server 2016 "));
				OutputProductType(dwProductType);
				break;
			}
			break;
		}
	}
	OutputString(_T(" %s "), OSver.szCSDVersion);
	BOOL b64BitOS = TRUE;
#ifndef _WIN64
	if (!::IsWow64Process(GetCurrentProcess(), &b64BitOS)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
#endif
	INT bit = 32;
	if (b64BitOS) {
		bit = 64;
	}
	OutputString(_T("%dbit\n"), bit);
#ifdef _DEBUG
	OutputString(
		_T("\tMajorVersion: %lu, MinorVersion: %lu, BuildNumber: %lu, PlatformId: %lu\n")
		_T("\tServicePackMajor: %u, ServicePackMinor: %u, SuiteMask: %u, ProductType: %u\n"),
		OSver.dwMajorVersion, OSver.dwMinorVersion, OSver.dwBuildNumber, OSver.dwPlatformId,
		OSver.wServicePackMajor, OSver.wServicePackMinor, OSver.wSuiteMask, OSver.wProductType);
#endif
	return TRUE;
}
#endif

BOOL OutputMergedFile(
	LPCTSTR pszFullPath,
	LPCTSTR pszFullPath2
) {
	FILE* fpSrc1 = NULL;
	if (NULL == (fpSrc1 = CreateOrOpenFileA(
		pszFullPath, NULL, NULL, NULL, NULL, ".bin", "rb", 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fpSrc2 = NULL;
	if (NULL == (fpSrc2 = CreateOrOpenFileA(
		pszFullPath2, NULL, NULL, NULL, NULL, ".bin", "rb", 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FcloseAndNull(fpSrc1);
		return FALSE;
	}
	FILE* fpDst = NULL;
	if (NULL == (fpDst = CreateOrOpenFileA(
		pszFullPath, "_merge", NULL, NULL, NULL, ".bin", "wb", 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FcloseAndNull(fpSrc1);
		FcloseAndNull(fpSrc2);
		return FALSE;
	}
	BYTE buf[2352] = {};
	if (fread(buf, sizeof(BYTE), sizeof(buf), fpSrc2) < sizeof(buf)) {
		OutputErrorString(_T("Failed to read: read size %zd [F:%s][L:%d]\n"), sizeof(buf), _T(__FUNCTION__), __LINE__);
		FcloseAndNull(fpSrc1);
		FcloseAndNull(fpSrc2);
		FcloseAndNull(fpDst);
		return FALSE;
	};
	INT nLBA = MSFtoLBA(BcdToDec(buf[12]), BcdToDec(buf[13]), BcdToDec(buf[14])) - 150;
	rewind(fpSrc2);

	for (INT i = 0; i < nLBA; i++) {
		if (fread(buf, sizeof(BYTE), sizeof(buf), fpSrc1) < sizeof(buf)) {
			OutputErrorString(_T("Failed to read: read size %zd [F:%s][L:%d]\n"), sizeof(buf), _T(__FUNCTION__), __LINE__);
			FcloseAndNull(fpSrc1);
			FcloseAndNull(fpSrc2);
			FcloseAndNull(fpDst);
			return FALSE;
		}
		fwrite(buf, sizeof(BYTE), sizeof(buf), fpDst);
	}

	if (fread(buf, sizeof(BYTE), sizeof(buf), fpSrc2) < sizeof(buf)) {
		OutputErrorString(_T("Failed to read: read size %zd [F:%s][L:%d]\n"), sizeof(buf), _T(__FUNCTION__), __LINE__);
		FcloseAndNull(fpSrc1);
		FcloseAndNull(fpSrc2);
		FcloseAndNull(fpDst);
		return FALSE;
	};
	fseek(fpSrc1, sizeof(buf), SEEK_CUR);
	while (!feof(fpSrc2) && !ferror(fpSrc2)) {
		fwrite(buf, sizeof(BYTE), sizeof(buf), fpDst);
		if (fread(buf, sizeof(BYTE), sizeof(buf), fpSrc2) < sizeof(buf)) {
			OutputErrorString(_T("Failed to read: read size %zd [F:%s][L:%d]\n"), sizeof(buf), _T(__FUNCTION__), __LINE__);
			FcloseAndNull(fpSrc1);
			FcloseAndNull(fpSrc2);
			FcloseAndNull(fpDst);
			return FALSE;
		};
		fseek(fpSrc1, sizeof(buf), SEEK_CUR);
	}
	fseek(fpSrc1, -2352, SEEK_CUR);

	if (fread(buf, sizeof(BYTE), sizeof(buf), fpSrc1) < sizeof(buf)) {
		OutputErrorString(_T("Failed to read: read size %zd [F:%s][L:%d]\n"), sizeof(buf), _T(__FUNCTION__), __LINE__);
		FcloseAndNull(fpSrc1);
		FcloseAndNull(fpSrc2);
		FcloseAndNull(fpDst);
		return FALSE;
	}
	while (!feof(fpSrc1) && !ferror(fpSrc1)) {
		fwrite(buf, sizeof(BYTE), sizeof(buf), fpDst);
		if (fread(buf, sizeof(BYTE), sizeof(buf), fpSrc1) < sizeof(buf)) {
			OutputErrorString(_T("Failed to read: read size %zd [F:%s][L:%d]\n"), sizeof(buf), _T(__FUNCTION__), __LINE__);
			FcloseAndNull(fpSrc1);
			FcloseAndNull(fpSrc2);
			FcloseAndNull(fpDst);
			return FALSE;
		}
	}
	FcloseAndNull(fpSrc1);
	FcloseAndNull(fpSrc2);
	FcloseAndNull(fpDst);
	return TRUE;
}
