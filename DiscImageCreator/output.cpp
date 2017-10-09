/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "check.h"
#include "convert.h"
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
	)
{
	_TCHAR szDstPath[_MAX_PATH] = { 0 };
	_TCHAR szTmpPath[_MAX_PATH + 1] = { 0 };
	_TCHAR szDrive[_MAX_DRIVE] = { 0 };
	_TCHAR szDir[_MAX_DIR] = { 0 };
	_TCHAR szFname[_MAX_FNAME] = { 0 };
	_TCHAR szExt[_MAX_EXT] = { 0 };

	_tsplitpath(pszPath, szDrive, szDir, szFname, szExt);
	if (pszPlusFname) {
		size_t plusFnameLen = _tcslen(pszPlusFname);
		DWORD pathSize = DWORD(_tcslen(szDir) + _tcslen(szFname) + plusFnameLen + _tcslen(pszExt));
		if (pathSize > _MAX_FNAME) {
			OutputErrorString(_T("File Size is too long: %lu\n"), pathSize);
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
	)
{
	WCHAR szDstPath[_MAX_PATH] = { 0 };
	WCHAR szTmpPath[_MAX_PATH + 1] = { 0 };
	WCHAR szDrive[_MAX_DRIVE] = { 0 };
	WCHAR szDir[_MAX_DIR] = { 0 };
	WCHAR szFname[_MAX_FNAME] = { 0 };
	WCHAR szExt[_MAX_EXT] = { 0 };

	_wsplitpath(pszPath, szDrive, szDir, szFname, szExt);
	if (pszPlusFname) {
		size_t plusFnameLen = wcslen(pszPlusFname);
		DWORD pathSize = DWORD(wcslen(szDir) + wcslen(szFname) + plusFnameLen + wcslen(pszExt));
		if (pathSize > _MAX_FNAME) {
			OutputErrorString(_T("File Size is too long: %lu\n"), pathSize);
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
	)
{
	CHAR szDstPath[_MAX_PATH] = { 0 };
	CHAR szTmpPath[_MAX_PATH + 1] = { 0 };
	CHAR szDrive[_MAX_DRIVE] = { 0 };
	CHAR szDir[_MAX_DIR] = { 0 };
	CHAR szFname[_MAX_FNAME] = { 0 };
	CHAR szExt[_MAX_EXT] = { 0 };

	_splitpath(pszPath, szDrive, szDir, szFname, szExt);
	if (pszPlusFname) {
		size_t plusFnameLen = strlen(pszPlusFname);
		DWORD pathSize = DWORD(strlen(szDir) + strlen(szFname) + plusFnameLen + strlen(pszExt));
		if (pathSize > _MAX_FNAME) {
			OutputErrorString(_T("File Size is too long: %lu\n"), pathSize);
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
	)
{
	_TCHAR szFullPath[_MAX_PATH] = { 0 };
	if (!::GetModuleFileName(NULL, szFullPath, sizeof(szFullPath) / sizeof(szFullPath[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return NULL;
	}
	FILE* fp = NULL;
	_TCHAR* p = _tcsrchr(szFullPath, '\\');
	if (p) {
		p[0] = NULL;
		_TCHAR szFullPathName[_MAX_PATH] = { 0 };
		_sntprintf(szFullPathName
			, sizeof(szFullPathName) / sizeof(szFullPathName[0]), _T("%s\\%s"), szFullPath, pszFname);
		szFullPathName[_MAX_PATH - 1] = 0;
		fp = _tfopen(szFullPathName, pszMode);
	}
	return fp;
}

FILE* OpenProgrammabledFileW(
	LPCWSTR pszFname,
	LPCWSTR pszMode
	)
{
	WCHAR szFullPath[_MAX_PATH] = { 0 };
	if (!::GetModuleFileNameW(NULL, szFullPath, sizeof(szFullPath) / sizeof(szFullPath[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return NULL;
	}
	FILE* fp = NULL;
	LPWCH p = wcsrchr(szFullPath, '\\');
	if (p) {
		p[0] = NULL;
		WCHAR szFullPathName[_MAX_PATH] = { 0 };
		_snwprintf(szFullPathName
			, sizeof(szFullPathName) / sizeof(szFullPathName[0]), L"%s\\%s", szFullPath, pszFname);
		szFullPathName[_MAX_PATH - 1] = 0;
		fp = _wfopen(szFullPathName, pszMode);
	}
	return fp;
}

VOID WriteCcdForDisc(
	WORD wTocEntries,
	BYTE LastCompleteSession,
	FILE* fpCcd
	)
{
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
	)
{
	if (fpCcd) {
		_ftprintf(fpCcd, _T("CDTextLength=%u\n"), wCDTextLength);
	}
}

VOID WriteCcdForDiscCatalog(
	PDISC pDisc,
	FILE* fpCcd
	)
{
	_TCHAR szCatalog[META_CATALOG_SIZE] = { 0 };
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
	)
{
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
	)
{
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
	)
{
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

VOID WriteCcdForEntry(
	PCDROM_TOC_FULL_TOC_DATA_BLOCK toc,
	UINT a,
	FILE* fpCcd
	)
{
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
	)
{
	if (fpCcd) {
		_ftprintf(fpCcd,
			_T("[TRACK %u]\n")
			_T("MODE=%u\n"),
			byTrackNum,
			pDisc->MAIN.lpModeList[byTrackNum - 1]);
		if (pDisc->SUB.lpISRCList[byTrackNum - 1]) {
			_TCHAR szISRC[META_ISRC_SIZE] = { 0 };
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
		}
	}
}

VOID WriteCcdForTrackIndex(
	BYTE byIndex,
	INT nLBA,
	FILE* fpCcd
	)
{
	if (fpCcd) {
		_ftprintf(fpCcd, _T("INDEX %u=%d\n"), byIndex, nLBA);
	}
}

VOID WriteCueForSongWriter(
	PDISC pDisc,
	BYTE byIdx,
	FILE* fpCue
	)
{
	if (pDisc->SCSI.pszSongWriter[byIdx][0] != 0) {
		_TCHAR szSongWriter[META_CDTEXT_SIZE] = { 0 };
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
	)
{
	if (pDisc->SCSI.pszPerformer[byIdx][0] != 0) {
		_TCHAR szPerformer[META_CDTEXT_SIZE] = { 0 };
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
	)
{
	if (pDisc->SCSI.pszTitle[byIdx][0] != 0) {
		_TCHAR szTitle[META_CDTEXT_SIZE] = { 0 };
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
	)
{
	if (pDisc->SUB.byCatalog) {
		_TCHAR szCatalog[META_CATALOG_SIZE] = { 0 };
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

VOID WriteCueForFileDirective(
	LPCTSTR pszPath,
	FILE* fpCue
	)
{
	_ftprintf(fpCue, _T("FILE \"%s\" BINARY\n"), pszPath);
}

VOID WriteCueForISRC(
	PDISC pDisc,
	INT nIdx,
	FILE* fpCue
	)
{
	if (pDisc->SUB.lpISRCList[nIdx]) {
		_TCHAR szISRC[META_ISRC_SIZE] = { 0 };
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
	)
{
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
		}
	}
	else {
		if (pDisc->SCSI.byCdi) {
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
	)
{
	_ftprintf(fpCue, _T("    INDEX %02u %02u:%02u:%02u\n"), 
		byIndex, byMinute, bySecond, byFrame);
}

VOID WriteMainChannel(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nLBA,
	FILE* fpImg
	)
{
	INT sLBA = pDisc->MAIN.nFixStartLBA;
	INT eLBA = pDisc->MAIN.nFixEndLBA;
	if (pExtArg->byReverse) {
		fwrite(lpBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
	}
	else if (sLBA <= nLBA && nLBA < eLBA) {
		// first sector
		if (nLBA == sLBA) {
			fwrite(lpBuf + pDisc->MAIN.uiMainDataSlideSize, sizeof(BYTE),
				CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize, fpImg);
			if (pDisc->SUB.lpFirstLBAListOnSub) {
				pDisc->SUB.lpFirstLBAListOnSub[0][0] = -150;
				pDisc->SUB.lpFirstLBAListOnSub[0][1] = nLBA - sLBA;
			}
			if (pDisc->SUB.lpFirstLBAListOnSubSync) {
				pDisc->SUB.lpFirstLBAListOnSubSync[0][0] = -150;
				pDisc->SUB.lpFirstLBAListOnSubSync[0][1] = nLBA - sLBA;
			}
		}
		// last sector in 1st session (when session 2 exists)
		else if (!pExtArg->byRawDump && pDisc->SCSI.nFirstLBAof2ndSession != -1 &&
			nLBA == pDisc->MAIN.nFixFirstLBAofLeadout - 1) {
			fwrite(lpBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpImg);
		}
		// first sector in 2nd Session
		else if (!pExtArg->byRawDump && pDisc->SCSI.nFirstLBAof2ndSession != -1 &&
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
	)
{
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
		else if (!pExtArg->byRawDump && pDisc->SCSI.nFirstLBAof2ndSession != -1 &&
			nLBA == pDisc->MAIN.nFixFirstLBAofLeadout - 1) {
			fwrite(lpBuf, sizeof(BYTE), nC2SlideSize, fpC2);
		}
		// first sector in 2nd Session
		else if (!pExtArg->byRawDump && pDisc->SCSI.nFirstLBAof2ndSession != -1 &&
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
	LPBYTE lpSubcodeRaw,
	LPBYTE lpSubcode,
	INT nLBA,
	BYTE byCurrentTrackNum,
	FILE* fpSub,
	FILE* fpParse
	)
{
	if (fpSub && fpParse) {
		fwrite(lpSubcode, sizeof(BYTE), CD_RAW_READ_SUBCODE_SIZE, fpSub);
		OutputCDSubToLog(pDisc, lpSubcode, lpSubcodeRaw, nLBA, byCurrentTrackNum, fpParse);
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
	BYTE byCurrentTrackNum,
	FILE* fpImg,
	FILE* fpSub,
	FILE* fpC2,
	FILE* fpParse
	)
{
	UINT uiSize = 0;
	BYTE zeroByte[CD_RAW_SECTOR_SIZE] = { 0 };
	if (*pExecType == data || pExtArg->byBe) {
		uiSize = CD_RAW_SECTOR_SIZE;
		if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			fwrite(pDiscPerSector->mainHeader.present, sizeof(BYTE), MAINHEADER_MODE1_SIZE, fpImg);
		}
		for (UINT i = MAINHEADER_MODE1_SIZE; i < CD_RAW_SECTOR_SIZE; i++) {
			pDiscPerSector->data.present[i] = 0x55;
		}
		if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			fwrite(pDiscPerSector->data.present + MAINHEADER_MODE1_SIZE,
				sizeof(BYTE), CD_RAW_SECTOR_SIZE - MAINHEADER_MODE1_SIZE, fpImg);
		}
		else {
			fwrite(zeroByte, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
		}
	}
	else {
		ZeroMemory(pDiscPerSector->data.present, CD_RAW_SECTOR_SIZE);
		for (UINT i = 0; i < pDisc->MAIN.uiMainDataSlideSize; i++) {
			pDiscPerSector->data.present[i] =
				(BYTE)(0x55 ^ lpScrambledBuf[CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize + i]);
		}
		for (UINT i = pDisc->MAIN.uiMainDataSlideSize; i < pDisc->MAIN.uiMainDataSlideSize + MAINHEADER_MODE1_SIZE; i++) {
			pDiscPerSector->data.present[i] =
				pDiscPerSector->mainHeader.present[i - pDisc->MAIN.uiMainDataSlideSize];
		}
		for (UINT i = pDisc->MAIN.uiMainDataSlideSize + MAINHEADER_MODE1_SIZE; i < CD_RAW_SECTOR_SIZE; i++) {
			pDiscPerSector->data.present[i] =
				(BYTE)(0x55 ^ lpScrambledBuf[i - pDisc->MAIN.uiMainDataSlideSize]);
		}
		if (nLBA == pDisc->MAIN.nFixStartLBA) {
			uiSize = CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize;
			if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				fwrite(pDiscPerSector->data.present + pDisc->MAIN.uiMainDataSlideSize,
					sizeof(BYTE), uiSize, fpImg);
			}
			else {
				fwrite(zeroByte, sizeof(BYTE), uiSize, fpImg);
			}
		}
		else if (nLBA == pDisc->MAIN.nFixEndLBA - 1) {
			uiSize = pDisc->MAIN.uiMainDataSlideSize;
			if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				fwrite(pDiscPerSector->data.present, sizeof(BYTE), uiSize, fpImg);
			}
			else {
				fwrite(zeroByte, sizeof(BYTE), uiSize, fpImg);
			}
		}
		else {
			uiSize = CD_RAW_SECTOR_SIZE;
			if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				fwrite(pDiscPerSector->data.present, sizeof(BYTE), uiSize, fpImg);
			}
			else {
				fwrite(zeroByte, sizeof(BYTE), uiSize, fpImg);
			}
		}
	}
	OutputLogA(standardError | fileMainError,
		"LBA[%06d, %#07x] Read error. padding [%ubyte]\n", nLBA, nLBA, uiSize);

	BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
	AlignColumnSubcode(pDiscPerSector->subcode.present, lpSubcodeRaw);
	WriteSubChannel(pDisc, lpSubcodeRaw
		, pDiscPerSector->subcode.present, nLBA, byCurrentTrackNum, fpSub, fpParse);

	if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		fwrite(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufC2Offset
			, sizeof(BYTE), CD_RAW_READ_C2_294_SIZE, fpC2);
	}
}

BOOL WriteParsingSubfile(
	LPCTSTR pszSubfile
	)
{
	BOOL bRet = TRUE;
	FILE* fpParse = CreateOrOpenFile(
		pszSubfile, _T("_sub"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0);
	if (!fpParse) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	LPBYTE data = NULL;
	DISC discData = { 0 };
	FILE* fpSub = NULL;
	DWORD dwTrackAllocSize = MAXIMUM_NUMBER_TRACKS + 10 + 1;
	try {
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
			(LPSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		for (DWORD h = 0; h < dwTrackAllocSize; h++) {
			if (NULL == (discData.SUB.pszISRC[h] =
				(LPSTR)calloc((META_ISRC_SIZE), sizeof(_TCHAR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}
		// TODO: doesn't use RtoW in present
		BYTE lpSubcodeRtoW[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		BYTE byTrackNum = 1;
		BYTE byPrevTrackNum = 1;
		INT nLBA = 0;

		for (INT i = 0; i < (INT)dwFileSize; i += CD_RAW_READ_SUBCODE_SIZE) {
			BYTE byAdr = (BYTE)(*(data + i + 12) & 0x0f);
			if (byAdr == ADR_ENCODES_CURRENT_POSITION) {
				byTrackNum = BcdToDec(*(data + i + 13));
				nLBA = MSFtoLBA(BcdToDec(*(data + i + 19)), 
					BcdToDec(*(data + i + 20)), BcdToDec(*(data + i + 21))) - 150;
				if (byTrackNum == 0) {
					byTrackNum = BcdToDec(*(data + i + 14));
					nLBA = MSFtoLBA(BcdToDec(*(data + i + 15)), 
						BcdToDec(*(data + i + 16)), BcdToDec(*(data + i + 17))) - 150;
				}
			}
			else if (byAdr == ADR_ENCODES_MEDIA_CATALOG) {
				SetMCNToString(&discData, &data[i],	discData.SUB.szCatalog, FALSE);
				nLBA++;
			}
			else if (byAdr == ADR_ENCODES_ISRC) {
				if (0 < byPrevTrackNum && byPrevTrackNum < dwTrackAllocSize) {
					SetISRCToString(&discData, &data[i], 
						discData.SUB.pszISRC[byPrevTrackNum - 1], (BYTE)(byPrevTrackNum - 1), FALSE);
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
			OutputCDSubToLog(&discData, &data[i], lpSubcodeRtoW, nLBA, byTrackNum, fpParse);
			OutputString(
				_T("\rParsing sub (Size) %8d/%8lu"), i + CD_RAW_READ_SUBCODE_SIZE, dwFileSize);
		}
		OutputString(_T("\n"));
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FcloseAndNull(fpParse);
	FcloseAndNull(fpSub);
	for (DWORD i = 0; i < dwTrackAllocSize; i++) {
		if (discData.SUB.pszISRC) {
			FreeAndNull(discData.SUB.pszISRC[i]);
		}
	}
	FreeAndNull(discData.SUB.pszISRC);
	FreeAndNull(data);
	return bRet;
}

BOOL DescrambleMainChannelForGD(
	LPCTSTR pszPath,
	LPTSTR pszOutPath
	)
{
	BOOL bRet = TRUE;
	FILE* fpScm = CreateOrOpenFile(
		pszPath, NULL, NULL, NULL, NULL, _T(".scm2"), _T("rb"), 0, 0);
	if (!fpScm) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fpImg = CreateOrOpenFile(
		pszPath, NULL, pszOutPath, NULL, NULL, _T(".img2"), _T("wb"), 0, 0);
	if (!fpImg) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FcloseAndNull(fpScm);
		return FALSE;
	}
	DWORD dwFileSize = GetFileSize(0, fpScm);
	DWORD dwAllSectorVal = dwFileSize / CD_RAW_SECTOR_SIZE;
	BYTE bufScm[CD_RAW_SECTOR_SIZE] = { 0 };
	BYTE bufImg[CD_RAW_SECTOR_SIZE] = { 0 };
	for (DWORD i = 0; i < dwAllSectorVal; i++) {
		fread(bufScm, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpScm);
		if (IsValidMainDataHeader(bufScm)) {
			if (bufScm[0x0C] == 0xC3 && bufScm[0x0D] == 0x84 && bufScm[0x0E] >= 0x00) {
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

BOOL SplitFileForGD(
	LPCTSTR pszPath
	)
{
	BOOL bRet = TRUE;
	_TCHAR pszFname[_MAX_PATH] = { 0 };
	FILE* fpImg = CreateOrOpenFile(pszPath, NULL,
		NULL, NULL, pszFname, _T(".img2"), _T("rb"), 0, 0);
	if (!fpImg) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fpBin = NULL;
	FILE* fpGdi = NULL;
	LPLONG lpToc = NULL;
	LPBYTE lpBuf = NULL;
	try {
		if (NULL == (fpGdi = CreateOrOpenFile(pszPath, NULL,
			NULL, NULL, NULL, _T(".gdi"), _T(WFLAG), 0, 0))) {
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
		BYTE aToc[512] = { 0 };
		fread(aToc, sizeof(BYTE), sizeof(aToc), fpImg);
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
		for (BYTE i = 3; i <= byMaxTrackNum; i++) {
			if (NULL == (fpBin = CreateOrOpenFile(pszPath, NULL, NULL,
				NULL, NULL, _T(".bin"), _T("wb"), i, byMaxTrackNum))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			size_t size = 
				(size_t)(lpToc[i - 2] - lpToc[i - 3]) * CD_RAW_SECTOR_SIZE;
			if (NULL == (lpBuf = (LPBYTE)calloc(size, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			fread(lpBuf, sizeof(BYTE), size, fpImg);
			fwrite(lpBuf, sizeof(BYTE), size, fpBin);
			FcloseAndNull(fpBin);
			FreeAndNull(lpBuf);
			OutputString(_T("\rSplitting img (Track) %2u/%2u"), i, byMaxTrackNum);
		}
		OutputString(_T("\n"));
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpImg);
	FcloseAndNull(fpBin);
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
	)
{
	BYTE aSrcBuf[CD_RAW_SECTOR_SIZE] = { 0 };
	LONG lSeekPtr = 0;

	for (INT k = pDisc->SCSI.byFirstDataTrackNum - 1; k < pDisc->SCSI.byLastDataTrackNum; k++) {
		INT nFirstLBA = pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[k];
		if (nFirstLBA != -1) {
			INT nLastLBA = pDisc->SUB.lpLastLBAListOfDataTrackOnSub[k];
			OutputDiscLogA("\tData Sector, LBA %6d-%6d (%#07x-%#07x)\n",
				nFirstLBA, nLastLBA, nFirstLBA, nLastLBA);
			if (!pExtArg->byRawDump && pDisc->SCSI.lpSessionNumList[k] >= 2) {
				INT nSkipLBA = (SESSION_TO_SESSION_SKIP_LBA * (INT)(pDisc->SCSI.lpSessionNumList[k] - 1));
				nFirstLBA -= nSkipLBA;
				nLastLBA -= nSkipLBA;
			}
			if (pExtArg->byPre) {
				nFirstLBA += 75;
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
				fread(aSrcBuf, sizeof(BYTE), sizeof(aSrcBuf), fpImg);
				if (IsValidMainDataHeader(aSrcBuf)) {
					if (aSrcBuf[0x0f] == 0x61 || aSrcBuf[0x0f] == 0x62) {
						if (IsValidReservedByte(aSrcBuf)) {
							OutputMainErrorWithLBALogA("A part of reverted sector. (Not be scrambled)\n", nFirstLBA, k + 1);
							OutputCDMain(fileMainError, aSrcBuf, nFirstLBA, CD_RAW_SECTOR_SIZE);
						}
					}
					else if (aSrcBuf[0x0f] == 0x01 || aSrcBuf[0x0f] == 0x02) {
						OutputMainErrorWithLBALogA("Reverted sector. (Not be scrambled)\n", nFirstLBA, k + 1);
						OutputCDMain(fileMainError, aSrcBuf, nFirstLBA, CD_RAW_SECTOR_SIZE);
					}
					else if (aSrcBuf[0x0f] != 0x61 && aSrcBuf[0x0f] != 0x62 &&
						aSrcBuf[0x0f] != 0x01 && aSrcBuf[0x0f] != 0x02) {
						OutputMainErrorWithLBALogA("Invalid mode. ", nFirstLBA, k + 1);
						BYTE m, s, f = 0;
						LBAtoMSF(nFirstLBA + 150, &m, &s, &f);
						if (aSrcBuf[0x0c] == m && aSrcBuf[0x0d] == s && aSrcBuf[0x0e] == f) {
							OutputMainErrorLogA("Reverted sector. (Not be scrambled)\n");
							if (!IsValidReservedByte(aSrcBuf)) {
								OutputMainErrorLogA("Invalid reserved byte. Skip descrambling\n");
								OutputString(
									_T("\rDescrambling data sector of img (LBA) %6d/%6d"), nFirstLBA, nLastLBA);
								OutputCDMain(fileMainError, aSrcBuf, nFirstLBA, CD_RAW_SECTOR_SIZE);
								continue;
							}
						}
						else if (IsValidReservedByte(aSrcBuf)) {
							OutputMainErrorLogA("A part of reverted sector. (Not be scrambled)\n");
						}
						else if (aSrcBuf[0x814] != 0x48 || aSrcBuf[0x815] != 0x64 || aSrcBuf[0x816] != 0x36 ||
							aSrcBuf[0x817] != 0xab || aSrcBuf[0x818] != 0x56 || aSrcBuf[0x819] != 0xff ||
							aSrcBuf[0x81a] != 0x7e || aSrcBuf[0x81b] != 0xc0) {
							OutputMainErrorLogA("Invalid reserved byte. Skip descrambling\n");
							OutputString(
								_T("\rDescrambling data sector of img (LBA) %6d/%6d"), nFirstLBA, nLastLBA);
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
					OutputMainErrorWithLBALogA("Invalid sync. Skip descrambling\n", nFirstLBA, k + 1);
					OutputCDMain(fileMainError, aSrcBuf, nFirstLBA, CD_RAW_SECTOR_SIZE);
				}
				OutputString(
					_T("\rDescrambling data sector of img (LBA) %6d/%6d"), nFirstLBA, nLastLBA);
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
)
{
	BYTE aSrcBuf[CD_RAW_SECTOR_SIZE] = { 0 };
	LONG lSeekPtr = 0;

	for (; nStartLBA <= nEndLBA; nStartLBA++, lSeekPtr++) {
		// ファイルを読み書き両用モードで開いている時は 注意が必要です。
		// 読み込みを行った後に書き込みを行う場合やその逆を行う場合は、 
		// 必ずfseekを呼ばなければなりません。もしこれを忘れると、
		// 場合によってはバッファー内と 実際にディスクに描き込まれた
		// データに矛盾が生じ、正確に書き込まれない場合や、
		// 嘘の データを読み込む場合があります。
		fseek(fpImg, lSeekPtr * CD_RAW_SECTOR_SIZE, SEEK_SET);
		fread(aSrcBuf, sizeof(BYTE), sizeof(aSrcBuf), fpImg);
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
			_T("\rDescrambling data sector of img (LBA) %6d/%6d"), nStartLBA, nEndLBA);
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
	)
{
	size_t stBufSize = 0;

	if (pDisc->SCSI.toc.LastTrack == pDisc->SCSI.toc.FirstTrack) {
		stBufSize = (size_t)pDisc->SCSI.nAllLength * CD_RAW_SECTOR_SIZE;
		nPrevLBA = 0;
	}
	else if (byTrackNum == pDisc->SCSI.toc.FirstTrack) {
		stBufSize = (size_t)nLBA * CD_RAW_SECTOR_SIZE;
		if (!pExtArg->byRawDump &&
			pDisc->SCSI.lpSessionNumList[byTrackNum - 1] != pDisc->SCSI.lpSessionNumList[byTrackNum]) {
			stBufSize -= SESSION_TO_SESSION_SKIP_LBA * CD_RAW_SECTOR_SIZE;
		}
		nPrevLBA = 0;
	}
	else if (byTrackNum == pDisc->SCSI.toc.LastTrack) {
		INT nTmpLength = pDisc->SCSI.nAllLength;
		if (!pExtArg->byRawDump && pDisc->SCSI.lpSessionNumList[byTrackNum - 1] >= 2) {
			INT nSessionSize =
				SESSION_TO_SESSION_SKIP_LBA * (pDisc->SCSI.lpSessionNumList[byTrackNum - 1] - 1);
			nPrevLBA -= nSessionSize;
			nTmpLength -= nSessionSize;
		}
		stBufSize = (size_t)(nTmpLength - nPrevLBA) * CD_RAW_SECTOR_SIZE;
	}
	else {
		if (!pExtArg->byRawDump) {
			INT nSessionSize =
				SESSION_TO_SESSION_SKIP_LBA * (pDisc->SCSI.lpSessionNumList[byTrackNum] - 1);
			// 1st
			if (pDisc->SCSI.lpSessionNumList[byTrackNum - 1] != pDisc->SCSI.lpSessionNumList[byTrackNum]) {
				nLBA -= nSessionSize;
			}
			// 2nd -
			else if (pDisc->SCSI.lpSessionNumList[byTrackNum - 1] >= 2) {
				nPrevLBA -= nSessionSize;
				nLBA -= nSessionSize;
			}
		}
		stBufSize = (size_t)(nLBA - nPrevLBA) * CD_RAW_SECTOR_SIZE;
	}
	fseek(fpImg, nPrevLBA * CD_RAW_SECTOR_SIZE, SEEK_SET);
	LPBYTE lpBuf = (LPBYTE)calloc(stBufSize, sizeof(BYTE));
	if (!lpBuf) {
		OutputString(_T("\n"));
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	fread(lpBuf, sizeof(BYTE), stBufSize, fpImg);
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
	)
{
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
		WriteCueForFirst(pDisc, bCanCDText, fpCueSync);
	}

	BOOL bRet = TRUE;
	_TCHAR pszFname[_MAX_FNAME] = { 0 };
	FILE* fpBin = NULL;
	FILE* fpBinSync = NULL;
	for (BYTE i = pDisc->SCSI.toc.FirstTrack; i <= pDisc->SCSI.toc.LastTrack; i++) {
		if (NULL == (fpBin = CreateOrOpenFile(pszPath, NULL, NULL, pszFname,
			NULL, _T(".bin"), _T("wb"), i, pDisc->SCSI.toc.LastTrack))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			bRet = FALSE;
			break;
		}
		WriteCueForUnderFileDirective(pDisc, bCanCDText, i, fpCueForImg);
		WriteCueForFileDirective(pszFname, fpCue);
		WriteCueForUnderFileDirective(pDisc, bCanCDText, i, fpCue);
		WriteCcdForTrack(pDisc, i, fpCcd);

		_TCHAR pszFnameSync[_MAX_FNAME] = { 0 };
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
		if (nLBAofFirstIdx == -1 || nLBAofFirstIdx == -150) {
			nLBAofFirstIdx = pDisc->SUB.lpFirstLBAListOnSub[i - 1][1];
			index++;
		}
		INT nLBAofFirstIdxSync = pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][0];
		if (nLBAofFirstIdxSync == -1 || nLBAofFirstIdxSync == -150) {
			nLBAofFirstIdxSync = pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][1];
		}

		BYTE byFrame = 0, bySecond = 0, byMinute = 0;
		if (i == pDisc->SCSI.toc.FirstTrack) { 
			if (0 == nLBAofFirstIdx ||
				i == pDisc->SCSI.toc.LastTrack) {
				WriteCueForIndexDirective(index, 0, 0, 0, fpCueForImg);
				WriteCueForIndexDirective(index, 0, 0, 0, fpCue);
				WriteCcdForTrackIndex(index, 0, fpCcd);
				if (pDisc->SUB.byDesync) {
					WriteCueForIndexDirective(index, 0, 0, 0, fpCueSyncForImg);
					WriteCueForIndexDirective(index, 0, 0, 0, fpCueSync);
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
				if (pDisc->SUB.byDesync) {
					WriteCueForIndexDirective(0, 0, 0, 0, fpCueSyncForImg);
					WriteCueForIndexDirective(0, 0, 0, 0, fpCueSync);
					WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueSyncForImg);
					WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueSync);
				}
			}
			index++;
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
			if (pDisc->SUB.byDesync) {
				INT nLBAofNextIdxSync = pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][index];
				if (nLBAofNextIdxSync != -1) {
					LBAtoMSF(nLBAofNextIdxSync,	&byMinute, &bySecond, &byFrame);
					WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueSyncForImg);

					LBAtoMSF(nLBAofNextIdxSync - nLBAofFirstIdxSync, &byMinute, &bySecond, &byFrame);
					WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueSync);
				}
			}
		}
		// write each track
		INT nLBA = pDisc->SUB.lpFirstLBAListOnSub[i - 1][0] == -1 ?
			pDisc->SUB.lpFirstLBAListOnSub[i - 1][1] : 
			pDisc->SUB.lpFirstLBAListOnSub[i - 1][0];
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
			pDisc->SUB.lpFirstLBAListOnSub[i][1] : 
			pDisc->SUB.lpFirstLBAListOnSub[i][0];
		if (pExtArg->byPre) {
			nNextLBA += 150;
		}
		bRet = CreateBin(pExtArg, pDisc, i, nNextLBA, nLBA, fpImg, fpBin);
		FcloseAndNull(fpBin);
		if (!bRet) {
			break;
		}
		if (pDisc->SUB.byDesync) {
			nLBA = pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][0] == -1 ?
				pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][1] : 
				pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][0];
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
				pDisc->SUB.lpFirstLBAListOnSubSync[i][1] :
				pDisc->SUB.lpFirstLBAListOnSubSync[i][0];
			if (pExtArg->byPre) {
				nNextLBA += 150;
			}
			bRet = CreateBin(pExtArg, pDisc, i, nNextLBA, nLBA, fpImg, fpBinSync);
			FcloseAndNull(fpBinSync);
			if (!bRet) {
				break;
			}
		}
		OutputString(
			_T("\rCreating bin, cue and ccd (Track) %2u/%2u"), i, pDisc->SCSI.toc.LastTrack);
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
)
{
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
	)
{
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
	)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	OutputErrorString(_T("[F:%s][L:%lu] GetLastError: %lu, %s\n"), 
		pszFuncName, lLineNum, GetLastError(), (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

VOID OutputProductType(
	DWORD dwProductType
	)
{
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
)
{
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
	)
{
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
