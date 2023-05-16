/**
 * Copyright 2011-2023 sarami
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
#include "outputScsiCmdLogforDVD.h"
#include "set.h"

#ifdef _DEBUG
_TCHAR logBuffer[DISC_MAIN_DATA_SIZE];
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
		UINT pathSize = UINT(_tcslen(szDir) + _tcslen(szFname) + _tcslen(pszPlusFname) + _tcslen(pszExt));
		if (pathSize > _MAX_FNAME) {
			OutputErrorString("Path size is too long: %u\n", pathSize);
			return NULL;
		}
		_tcsncat(szFname, pszPlusFname, sizeof(szFname) - _tcslen(szFname) - 1);
	}
	if (byMaxTrackNum <= 1) {
		_sntprintf(szDstPath, SIZE_OF_ARRAY(szDstPath),
			_T("%s%s%s%s"), szDrive, szDir, szFname, pszExt);
	}
	else if (2 <= byMaxTrackNum && byMaxTrackNum <= 9) {
		_sntprintf(szDstPath, SIZE_OF_ARRAY(szDstPath),
			_T("%s%s%s (Track %u)%s"), szDrive, szDir, szFname, byTrackNum, pszExt);
	}
	else if (10 <= byMaxTrackNum) {
		_sntprintf(szDstPath, SIZE_OF_ARRAY(szDstPath),
			_T("%s%s%s (Track %02u)%s"), szDrive, szDir, szFname, byTrackNum, pszExt);
	}
	szDstPath[_MAX_PATH - 1] = 0;

	if (pszFnameAndExt) {
		// size of pszFnameAndExt must be _MAX_FNAME + _MAX_EXT.
		ZeroMemory(pszFnameAndExt, _MAX_FNAME + _MAX_EXT);
		_tsplitpath(szDstPath, NULL, NULL, szFname, szExt);
		_sntprintf(pszFnameAndExt, _MAX_FNAME + _MAX_EXT, _T("%s%s"), szFname, szExt);
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

FILE* OpenProgrammabledFile(
	LPCTSTR pszFname,
	LPCTSTR pszMode
) {
	_TCHAR szFullPath[_MAX_PATH] = {};
	if (!::GetModuleFileName(NULL, szFullPath, SIZE_OF_ARRAY(szFullPath))) {
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
		_TCHAR szFullPathName[_MAX_PATH + _MAX_FNAME] = {};
#ifdef _WIN32
		_sntprintf(szFullPathName
			, SIZE_OF_ARRAY(szFullPathName), _T("%s\\%s"), szFullPath, pszFname);
#else
		_sntprintf(szFullPathName
			, SIZE_OF_ARRAY(szFullPathName), _T("%s/%s"), szFullPath, pszFname);
#endif
		szFullPathName[_MAX_PATH + _MAX_FNAME - 1] = 0;
		fp = _tfopen(szFullPathName, pszMode);
	}
	return fp;
}

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
			_T("DataTracksScrambled=%d\n"),
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
		, pDisc->SUB.szCatalog, SIZE_OF_ARRAY(pDisc->SUB.szCatalog)
		, szCatalog, SIZE_OF_ARRAY(szCatalog));
#else
	strncpy(szCatalog, pDisc->SUB.szCatalog, SIZE_OF_ARRAY(szCatalog));
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
				(UINT)(pDesc[t].TrackNumber | (pDesc[t].ExtensionFlag << 7)),
				pDesc[t].SequenceNumber,
				(UINT)(pDesc[t].CharacterPosition | (pDesc[t].BlockNumber << 4) | (pDesc[t].Unicode << 7)),
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
			_T("PreGapSubC=%d\n"),
			SessionNumber,
			byMode,
			SessionNumber == 1 ? byMode : 0); // TODO
	}
}

BOOL WriteCcdFirst(
	PEXEC_TYPE pExecType,
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
		UINT uiBufLen = CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 2;
		if (!ReadCDForCheckingSubQAdrFirst(pExecType, pExtArg
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
				if (pTocData[i].Point != 0xa0) {
					pTocData[i].Msf[1] = BcdToDec(pTocData[i].Msf[1]);
				}
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
					"\rChecking SubQ adr (Track) %2u/%2u", pTocData[i].Point, pDisc->SCSI.toc.LastTrack);
			}
		}
		OutputString("\n");
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
			_T("ADR=0x%02hhx\n")
			_T("Control=0x%02hhx\n")
			_T("TrackNo=%u\n")
			_T("AMin=%u\n")
			_T("ASec=%u\n")
			_T("AFrame=%u\n")
			_T("ALBA=%d\n")
			_T("Zero=%u\n")
			_T("PMin=%u\n")
			_T("PSec=%u\n")
			_T("PFrame=%u\n")
			, a, toc[a].SessionNumber, toc[a].Point, toc[a].Adr, toc[a].Control
			, toc[a].Reserved1, toc[a].MsfExtra[0], toc[a].MsfExtra[1], toc[a].MsfExtra[2]
			, MSFtoLBA(toc[a].MsfExtra[0], toc[a].MsfExtra[1], toc[a].MsfExtra[2]) - 150
			, toc[a].Zero, toc[a].Msf[0], toc[a].Msf[1], toc[a].Msf[2]
		);
		INT LBA = MSFtoLBA(toc[a].Msf[0], toc[a].Msf[1], toc[a].Msf[2]);
		INT diff = 150;
		if (LBA >= 404850) {
			diff = 450150;
		}
		_ftprintf(fpCcd,
			_T("PLBA=%d\n"), LBA - diff);
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
				, szISRC, SIZE_OF_ARRAY(szISRC));
#else
			strncpy(szISRC, pDisc->SUB.pszISRC[byTrackNum - 1], SIZE_OF_ARRAY(szISRC) - 1);
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
	BYTE n,
	BYTE byIdx,
	FILE* fpCue
) {
	if (pDisc->SCSI.CDTEXT[n].pszSongWriter[byIdx][0] != 0) {
		_TCHAR szSongWriter[META_CDTEXT_SIZE] = {};
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0
			, pDisc->SCSI.CDTEXT[n].pszSongWriter[byIdx], META_CDTEXT_SIZE
			, szSongWriter, SIZE_OF_ARRAY(szSongWriter));
#else
		strncpy(szSongWriter, pDisc->SCSI.CDTEXT[n].pszSongWriter[byIdx], SIZE_OF_ARRAY(szSongWriter) - 1);
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
	BYTE n,
	BYTE byIdx,
	FILE* fpCue
) {
	if (pDisc->SCSI.CDTEXT[n].pszPerformer[byIdx][0] != 0) {
		_TCHAR szPerformer[META_CDTEXT_SIZE] = {};
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0
			, pDisc->SCSI.CDTEXT[n].pszPerformer[byIdx], META_CDTEXT_SIZE
			, szPerformer, SIZE_OF_ARRAY(szPerformer));
#else
		strncpy(szPerformer, pDisc->SCSI.CDTEXT[n].pszPerformer[byIdx], SIZE_OF_ARRAY(szPerformer) - 1);
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
	BYTE n,
	BYTE byIdx,
	FILE* fpCue
) {
	if (pDisc->SCSI.CDTEXT[n].pszTitle[byIdx][0] != 0) {
		_TCHAR szTitle[META_CDTEXT_SIZE] = {};
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0
			, pDisc->SCSI.CDTEXT[n].pszTitle[byIdx], META_CDTEXT_SIZE
			, szTitle, SIZE_OF_ARRAY(szTitle));
#else
		strncpy(szTitle, pDisc->SCSI.CDTEXT[n].pszTitle[byIdx], SIZE_OF_ARRAY(szTitle) - 1);
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
	BYTE n,
	FILE* fpCue
) {
	if (pDisc->SUB.byCatalog) {
		_TCHAR szCatalog[META_CATALOG_SIZE] = {};
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0 
			, pDisc->SUB.szCatalog, SIZE_OF_ARRAY(pDisc->SUB.szCatalog)
			, szCatalog, SIZE_OF_ARRAY(szCatalog));
#else
		strncpy(szCatalog, pDisc->SUB.szCatalog, SIZE_OF_ARRAY(szCatalog));
#endif
		_ftprintf(fpCue, _T("CATALOG %s\n"), szCatalog);
	}
	if (bCanCDText) {
		WriteCueForTitle(pDisc, n, 0, fpCue);
		WriteCueForPerformer(pDisc, n, 0, fpCue);
		WriteCueForSongWriter(pDisc, n, 0, fpCue);
	}
}

VOID WriteCueForMultiSessionMultiBin(
	PDISC pDisc,
	BYTE byTrackNum,
	FILE* fpCue
) {
	if (pDisc->SCSI.bMultiSession) {
		if (byTrackNum == pDisc->SCSI.by1stMultiSessionTrkNum) {
			_ftprintf(fpCue, _T("REM LEAD-OUT 01:30:00\n")); // always 01:30:00
		}
		if (byTrackNum == pDisc->SCSI.toc.FirstTrack || byTrackNum == pDisc->SCSI.by1stMultiSessionTrkNum) {
			_ftprintf(fpCue, _T("REM SESSION %02d\n"), pDisc->SCSI.lpSessionNumList[byTrackNum - 1]);
		}
		if (byTrackNum == pDisc->SCSI.by1stMultiSessionTrkNum) {
			_ftprintf(fpCue, _T("REM LEAD-IN 01:00:00\n")); // always 01:00:00
			_ftprintf(fpCue, _T("REM PREGAP 00:02:00\n")); // always 00:02:00
		}
	}
}

VOID WriteCueForMultiSessionWholeBin(
	PDISC pDisc,
	BYTE byTrackNum,
	FILE* fpCue
) {
	if (pDisc->SCSI.bMultiSession) {
		if (byTrackNum == pDisc->SCSI.by1stMultiSessionTrkNum) {
			BYTE m, s, f;
			LBAtoMSF(pDisc->SCSI.n1stLBAof2ndSession - SESSION_TO_SESSION_SKIP_LBA, &m, &s, &f);
			_ftprintf(fpCue, _T("  REM LEAD-OUT %02d:%02d:%02d\n"), m, s, f);
		}
		if (byTrackNum == pDisc->SCSI.toc.FirstTrack || byTrackNum == pDisc->SCSI.by1stMultiSessionTrkNum) {
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
			, szISRC, SIZE_OF_ARRAY(szISRC));
#else
		strncpy(szISRC, pDisc->SUB.pszISRC[nIdx], SIZE_OF_ARRAY(szISRC) - 1);
#endif
		_ftprintf(fpCue, _T("    ISRC %s\n"), szISRC);
	}
}

VOID WriteCueForUnderFileDirective(
	PDISC pDisc,
	BOOL bCanCDText,
	BYTE byIdx,
	BYTE byTrackNum,
	FILE* fpCue
) {
	if (pDisc->MAIN.lpModeList[byTrackNum - 1] == DATA_BLOCK_MODE0) {
		_ftprintf(fpCue, _T("  TRACK %02u AUDIO\n"), byTrackNum);
		if (bCanCDText) {
			WriteCueForTitle(pDisc, byIdx, byTrackNum, fpCue);
			WriteCueForPerformer(pDisc, byIdx, byTrackNum, fpCue);
			WriteCueForSongWriter(pDisc, byIdx, byTrackNum, fpCue);
		}
		WriteCueForISRC(pDisc, byTrackNum - 1, fpCue);
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
				if (pDisc->SUB.lp1stLBAListOnSub) {
					pDisc->SUB.lp1stLBAListOnSub[0][0] = -150;
				}
				if (pDisc->SUB.lp1stLBAListOnSubSync) {
					pDisc->SUB.lp1stLBAListOnSubSync[0][0] = -150;
				}
			}
			else {
				pDisc->SUB.lp1stLBAListOnSub[2][0] = FIRST_PREGAP_LBA_FOR_GD;
			}
		}
		// last sector in 1st session (when session 2 exists)
		else if (pDisc->SCSI.n1stLBAof2ndSession != -1 &&
			nLBA == pDisc->MAIN.nFix1stLBAofLeadout - 1) {
			fwrite(lpBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpImg);
		}
		// first sector in 2nd Session
		else if (pDisc->SCSI.n1stLBAof2ndSession != -1 &&
			nLBA == pDisc->MAIN.nFix1stLBAof2ndSession) {
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
	PDEVICE pDevice,
	PDISC_PER_SECTOR pDiscPerSector,
	FILE* fpC2
) {
	if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		if (0 < pExtArg->uiC2Offset && pExtArg->uiC2Offset < CD_RAW_READ_C2_294_SIZE) {
			fwrite(pDiscPerSector->data.current + pDevice->TRANSFER.uiBufC2Offset + pExtArg->uiC2Offset
				, sizeof(BYTE), (size_t)CD_RAW_READ_C2_294_SIZE - pExtArg->uiC2Offset, fpC2);
			fwrite(pDiscPerSector->data.next + pDevice->TRANSFER.uiBufC2Offset, sizeof(BYTE), (size_t)pExtArg->uiC2Offset, fpC2);
		}
		else if (pExtArg->uiC2Offset == CD_RAW_READ_C2_294_SIZE) {
			fwrite(pDiscPerSector->data.next + pDevice->TRANSFER.uiBufC2Offset, sizeof(BYTE), CD_RAW_READ_C2_294_SIZE, fpC2);
		}
		else if (CD_RAW_READ_C2_294_SIZE < pExtArg->uiC2Offset && pExtArg->uiC2Offset < CD_RAW_READ_C2_294_SIZE * 2) {
			UINT uiVal = pExtArg->uiC2Offset - CD_RAW_READ_C2_294_SIZE;
			fwrite(pDiscPerSector->data.next + pDevice->TRANSFER.uiBufC2Offset + uiVal, sizeof(BYTE), (size_t)CD_RAW_READ_C2_294_SIZE - uiVal, fpC2);
			fwrite(pDiscPerSector->data.nextNext + pDevice->TRANSFER.uiBufC2Offset, sizeof(BYTE), uiVal, fpC2);
		}
		else {
			fwrite(pDiscPerSector->data.current + pDevice->TRANSFER.uiBufC2Offset, sizeof(BYTE), CD_RAW_READ_C2_294_SIZE, fpC2);
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
	INT nLastErrLBA,
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
			if ((pDiscPerSector->subch.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				fwrite(pDiscPerSector->mainHeader.current, sizeof(BYTE), MAINHEADER_MODE1_SIZE, fpImg);
			}
			if (nPadType == padByUsr55) {
				for (UINT i = MAINHEADER_MODE1_SIZE; i < uiSize; i++) {
					pDiscPerSector->data.current[i] = 0x55;
				}
				if ((pDiscPerSector->subch.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
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
		else if (nLBA == pDisc->MAIN.nFixEndLBA - 1 || nLBA == pDisc->MAIN.nFix1stLBAof2ndSession - 150) {
			uiSize = pDisc->MAIN.uiMainDataSlideSize;
			if (nPadType == padByUsr55 || nPadType == padByUsr0 || nPadType == padByPrevSector) {
				if (nLBA == nLastErrLBA) {
					uiSize = CD_RAW_SECTOR_SIZE;
					if (pDisc->MAIN.nAdjustSectorNum > 1) {
						// Sonic The Hedgehog (Germany) (4 014548 004785)
						uiSize *= 2;
					}
				}
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
	OutputLog(fileMainError,
		STR_LBA "Read error. padding [%u bytes]\n", nLBA, (UINT)nLBA, uiSize);

	if (*pExecType != swap || (*pExecType == swap && nLBA < SECOND_ERROR_OF_LEADOUT)) {
		BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = {};
		if (pDiscPerSector->subch.current.byIndex == 0) {
			pDiscPerSector->subch.current.nRelativeTime--;
		}
		else {
			pDiscPerSector->subch.current.nRelativeTime++;
		}
		pDiscPerSector->subch.current.nAbsoluteTime++;
		SetBufferFromTmpSubch(pDiscPerSector->subcode.current, pDiscPerSector->subch.current, TRUE, TRUE);
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
	if (NULL == (g_LogFile.fpSubReadable = CreateOrOpenFile(
		pszSubfile, _T("_subReadable"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
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
		if (NULL == (g_LogFile.fpSubError = CreateOrOpenFile(
			pszSubfile, _T("_subError"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
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
			(LPSTR*)calloc(uiTrackAllocSize, sizeof(LPSTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		for (UINT h = 0; h < uiTrackAllocSize; h++) {
			if (NULL == (discData.SUB.pszISRC[h] =
				(LPSTR)calloc((META_ISRC_SIZE), sizeof(CHAR)))) {
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
				"\rParsing sub (Size) %8d/%8lu", i + CD_RAW_READ_SUBCODE_SIZE, dwFileSize);
		}
		OutputString("\n");
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
#ifndef DEBUG
	if (NULL == (g_LogFile.fpMdsReadable = CreateOrOpenFile(
		pszMdsfile, _T("_mdsReadable"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
#endif
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
			OutputErrorString("Failed to read [F:%s][L:%d]\n", _T(__FUNCTION__), __LINE__);
			FcloseAndNull(fpMds);
			throw FALSE;
		};
		FcloseAndNull(fpMds);

		MDS_HEADER h = {};
		memcpy(&h, data, sizeof(MDS_HEADER));
		size_t nOfs = sizeof(MDS_HEADER);

		MDS_FOR_DVD_BLK dvd[2] = {};
		BYTE layerNumber = 1;
		if (h.mediaType == 0x10) {
			if ((data[nOfs + 2054] >> 5 & 0x03) == 1) {
				layerNumber = 2;
			}
			for (BYTE i = 0; i < layerNumber; i++) {
				if (h.ofsToBca) {
					if (NULL == (dvd[i].bca = (PDVD_BCA_DESCRIPTOR)calloc(layerNumber, 2048))) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
					memcpy(dvd[i].bca, data + nOfs, h.lenOfBca);
					nOfs += h.lenOfBca;
				}
				memcpy(&dvd[i].copyright, data + nOfs, sizeof(dvd[i].copyright));
				nOfs += sizeof(dvd[i].copyright);

				memcpy(&dvd[i].dmi, data + nOfs, sizeof(dvd[i].dmi));
				nOfs += sizeof(dvd[i].dmi);

				memcpy(&dvd[i].layer, data + nOfs, sizeof(dvd[i].layer));
				nOfs += sizeof(dvd[i].layer);
			}
		}

		PMDS_SESSION_BLK psb = NULL;
		if (NULL == (psb = (PMDS_SESSION_BLK)calloc(h.sessionNum, sizeof(MDS_SESSION_BLK)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		size_t size = h.sessionNum * sizeof(MDS_SESSION_BLK);
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
			if (NULL == (pdb = (PMDS_DPM_HEADER)calloc(allocsize, sizeof(MDS_DPM_HEADER)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			size = allocsize;
			memcpy(pdb, data + nOfs, size);
			nOfs += size;

			if (NULL == (pddb = (PMDS_DPM_BLK*)calloc(pdb->dpmBlkTotalNum, sizeof(PMDS_DPM_BLK)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			for (UINT i = 0; i < pdb->dpmBlkTotalNum; i++) {
				UINT entry = MAKEUINT(MAKEWORD(data[nOfs + 12], data[nOfs + 13]), MAKEWORD(data[nOfs + 14], data[nOfs + 15]));
				allocsize = 16 + entry * sizeof(UINT);
				if (NULL == (pddb[i] = (PMDS_DPM_BLK)calloc(allocsize, sizeof(MDS_DPM_BLK)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				size = allocsize;
				memcpy(pddb[i], data + nOfs, size);
				nOfs += size;
			}
		}
		// http://problemkaputt.de/psx-spx.htm#cdromdiskimagesmdsmdfalcohol120
		OutputMdsReadableLog(
			OUTPUT_DHYPHEN_PLUS_STR("Header")
			"                      File ID: %.16" CHARWIDTH "s\n"
			"                      Unknown: %u\n"
			"                   Media Type: %u "
			, h.fileId, h.unknown1, h.mediaType
		);
		switch (h.mediaType) {
		case 0:
			OutputMdsReadableLog("(CD-DA or CD-ROM)\n");
			break;
		case 1:
			OutputMdsReadableLog("(CD-R)\n");
			break;
		case 2:
			OutputMdsReadableLog("(CD-RW)\n");
			break;
		case 16:
			OutputMdsReadableLog("(DVD-ROM)\n");
			break;
		default:
			OutputMdsReadableLog("(Other)\n");
			break;
		}
		OutputMdsReadableLog(
			"           Number of sessions: %u\n"
			"                      Unknown: %u\n"
			, h.sessionNum, h.unknown2
		);
		if (h.mediaType == 0x10) {
			OutputMdsReadableLog(
				"           Length of BCA data: %u\n"
				"                         Zero: %02u %02u %02u %02u %02u %02u %02u %02u\n"
				"           Offset to BCA data: %u\n"
				"                         Zero: %02u %02u %02u %02u %02u %02u %02u %02u\n"
				"                               %02u %02u %02u %02u %02u %02u %02u %02u\n"
				"                               %02u %02u %02u %02u %02u %02u %02u %02u\n"
				"    Offset to Disc Structures: %u\n"
				"                         Zero: %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u\n"
				"Offset to First Session-Block: %u\n"
				"    Offset to DPM data blocks: %u\n"
				, h.lenOfBca
				, h.zero1[0], h.zero1[1], h.zero1[2], h.zero1[3], h.zero1[4], h.zero1[5], h.zero1[6], h.zero1[7]
				, h.ofsToBca
				, h.zero2[0], h.zero2[1], h.zero2[2], h.zero2[3], h.zero2[4], h.zero2[5], h.zero2[6], h.zero2[7]
				, h.zero2[8], h.zero2[9], h.zero2[10], h.zero2[11], h.zero2[12], h.zero2[13], h.zero2[14], h.zero2[15]
				, h.zero2[16], h.zero2[17], h.zero2[18], h.zero2[19], h.zero2[20], h.zero2[21], h.zero2[22], h.zero2[23]
				, h.ofsToDiscStructures
				, h.zero3[0], h.zero3[1], h.zero3[2], h.zero3[3], h.zero3[4], h.zero3[5], h.zero3[6], h.zero3[7], h.zero3[8], h.zero3[9], h.zero3[10], h.zero3[11]
				, h.ofsTo1stSessionBlk, h.ofsToDpm
			);
			DISC disc;
			for (UINT i = 0; i < layerNumber; i++) {
				OutputDVDCopyrightDescriptor(&dvd[i].copyright, &(disc.DVD.protect), fileMds);
				if (h.lenOfBca) {
					OutputDiscBCADescriptor(&disc, dvd[i].bca, h.lenOfBca, fileMds);
				}
				OutputDVDManufacturerDescriptor(&dvd[i].dmi, &disc, fileMds);
				DWORD dwSectorLength = 0;
				OutputDVDLayerDescriptor(&disc, &dvd[i].layer, &dwSectorLength, layerNumber, fileMds);
			}
		}
		for (INT i = 0; i < h.sessionNum; i++) {
			OutputMdsReadableLog(
				OUTPUT_DHYPHEN_PLUS_STR("Session Block[%d]")
				"                 Session Start Sector: %d\n"
				"                   Session End Sector: %u\n"
				"                       Session number: %u\n"
				"          Total Number of Data Blocks: %u\n"
				"Number of Data Blocks with Point>=A0h: %u\n"
				"        First Track Number in Session: %u\n"
				"         Last Track Number in Session: %u\n"
				"                                 Zero: %02u %02u %02u %02u\n"
				"           Offset to First Data-Block: %u\n"
				, i + 1, psb[i].startSector, psb[i].endSector, psb[i].sessionNum
				, psb[i].totalDataBlkNum, psb[i].DataBlkNum, psb[i].firstTrackNum
				, psb[i].lastTrackNum
				, psb[i].zero[0], psb[i].zero[1], psb[i].zero[2], psb[i].zero[3]
				, psb[i].ofsTo1stDataBlk
			);
		}
		for (INT i = 0; i < tdb; i++) {
			OutputMdsReadableLog(
				OUTPUT_DHYPHEN_PLUS_STR("Full TOC Data Block[%d]")
				"                                 Track mode: %02X"
				, i + 1, db[i].trackMode
			);
			switch (db[i].trackMode) {
			case 0xa9:
				OutputMdsReadableLog(" (Audio)\n");
				break;
			case 0xaa:
				OutputMdsReadableLog(" (Mode 1)\n");
				break;
			case 0xab:
				OutputMdsReadableLog(" (Mode 2)\n");
				break;
			case 0xac:
				OutputMdsReadableLog(" (Mode 2 Form 1)\n");
				break;
			case 0xad:
				OutputMdsReadableLog(" (Mode 2 Form 2)\n");
				break;
			case 0xec:
				OutputMdsReadableLog(" (Mode 2 Form 1)\n");
				break;
			case 0xed:
				OutputMdsReadableLog(" (Mode 2 Form 2)\n");
				break;
			default:
				OutputMdsReadableLog("\n");
				break;
			}
			OutputMdsReadableLog(
				"         Number of subchannels in .MDF file: %u\n"
				"                                        ADR: %hhu\n"
				"                                    Control: %hhu\n"
				"                                      Point: %02u (%02X) "
				, db[i].numOfSubch, db[i].Adr, db[i].Control, db[i].Point, db[i].Point
			);
			INT nTmpLBA = 0;
			INT nTmpLBAExt = 0;
			switch (db[i].Point) {
			case 0xa0:
				OutputMdsReadableLog("FirstTrack %u, ", db[i].Msf[0]);
				switch (db[i].Msf[1]) {
				case DISK_TYPE_CDDA:
					OutputMdsReadableLog("Format: CD-DA or CD-ROM\n");
					break;
				case DISK_TYPE_CDI:
					OutputMdsReadableLog("Format: CD-I\n");
					break;
				case DISK_TYPE_XA:
					OutputMdsReadableLog("Format: CD-ROM-XA\n");
					break;
				default:
					OutputMdsReadableLog("Format: Other [0x%02x]\n", db[i].Msf[1]);
					break;
				}
				break;
			case 0xa1:
				OutputMdsReadableLog("LastTrack %u\n", db[i].Msf[0]);
				break;
			case 0xa2:
				nTmpLBA =
					MSFtoLBA(db[i].Msf[0], db[i].Msf[1], db[i].Msf[2]) - 150;
				OutputMdsReadableLog(
					"Lead-out, AMSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
					, db[i].Msf[0], db[i].Msf[1], db[i].Msf[2], nTmpLBA, (UINT)nTmpLBA);
				break;
			case 0xb0:
				nTmpLBAExt =
					MSFtoLBA(db[i].MsfExtra[0], db[i].MsfExtra[1], db[i].MsfExtra[2]) - 150;
				nTmpLBA =
					MSFtoLBA(db[i].Msf[0], db[i].Msf[1], db[i].Msf[2]) - 150;
				OutputMdsReadableLog(
					"NextSession, AMSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
					"\t                               Outermost Lead-out of the disc, AMSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
					"\t                                    Num of pointers in Mode 5, %02u\n"
					, db[i].MsfExtra[0], db[i].MsfExtra[1], db[i].MsfExtra[2]
					, nTmpLBAExt, (UINT)nTmpLBAExt
					, db[i].Msf[0], db[i].Msf[1], db[i].Msf[2]
					, nTmpLBA, (UINT)nTmpLBA, db[i].Zero);
				break;
			case 0xb1:
				OutputMdsReadableLog(
					"\tNum of skip interval pointers, %02u\n"
					"\t   Num of skip track pointers, %02u\n"
					, db[i].Msf[0], db[i].Msf[1]);
				break;
			case 0xb2:
			case 0xb3:
			case 0xb4:
				OutputMdsReadableLog(
					"Skip num, %02u %02u %02u %02u %02u %02u\n"
					, db[i].MsfExtra[0], db[i].MsfExtra[1], db[i].MsfExtra[2]
					, db[i].Msf[0], db[i].Msf[1], db[i].Msf[2]);
				break;
			case 0xc0:
				nTmpLBA =
					MSFtoLBA(db[i].Msf[0], db[i].Msf[1], db[i].Msf[2]) - 150;
				if (h.mediaType == 1 || h.mediaType == 2) {
					OutputMdsReadableLog(
						", MSF %02u:%02u:%02u\n"
						"\t                        First Lead-in of the disc, AMSF %02u:%02u:%02u\n"
						, db[i].MsfExtra[0], db[i].MsfExtra[1], db[i].MsfExtra[2]
						, db[i].Msf[0], db[i].Msf[1], db[i].Msf[2]);
					// Ecma 394
					// 4.4.1 Special Information 1 : M1,S1,F1 = 101
					db[i].MsfExtra[0] = (UCHAR)(db[i].MsfExtra[0] >> 1 | 0x80);
					db[i].MsfExtra[1] = (UCHAR)(db[i].MsfExtra[1] >> 1);
					db[i].MsfExtra[2] = (UCHAR)(db[i].MsfExtra[2] >> 1 | 0x80);
				}
				else {
					OutputMdsReadableLog(
						", MSF %02u:%02u:%02u\n"
						"\t                        First Lead-in of the disc, AMSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
						, db[i].MsfExtra[0], db[i].MsfExtra[1], db[i].MsfExtra[2]
						, db[i].Msf[0], db[i].Msf[1], db[i].Msf[2]
						, nTmpLBA - 450150, (UINT)nTmpLBA - 450150);
				}
				break;
			case 0xc1:
				OutputMdsReadableLog(
					"\nCopy of info from Additional Information1 in ATIP, MSF %02u:%02u:%02u, AMSF %02u:%02u:%02u\n"
					, db[i].MsfExtra[0], db[i].MsfExtra[1], db[i].MsfExtra[2]
					, db[i].Msf[0], db[i].Msf[1], db[i].Msf[2]);
				break;
			default:
				if (db[i].Adr == 1) {
					nTmpLBA =
						MSFtoLBA(db[i].Msf[0], db[i].Msf[1], db[i].Msf[2]) - 150;
					OutputMdsReadableLog(
						"Track %u, AMSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
						, db[i].Point, db[i].Msf[0], db[i].Msf[1], db[i].Msf[2], nTmpLBA, (UINT)nTmpLBA);
				}
				else if (db[i].Adr == 5) {
					nTmpLBAExt =
						MSFtoLBA(db[i].MsfExtra[0], db[i].MsfExtra[1], db[i].MsfExtra[2]) - 150;
					nTmpLBA =
						MSFtoLBA(db[i].Msf[0], db[i].Msf[1], db[i].Msf[2]) - 150;
					OutputMdsReadableLog(
						"\tSkipped interval end time, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
						"\tSkipped interval start time on playback, AMSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
						, db[i].MsfExtra[0], db[i].MsfExtra[1], db[i].MsfExtra[2], nTmpLBAExt, (UINT)nTmpLBAExt
						, db[i].Msf[0], db[i].Msf[1], db[i].Msf[2], nTmpLBA, (UINT)nTmpLBA);
				}
				OutputMdsReadableLog(
					"                      Offset to Index-block: %u\n"
					"                                Sector size: %u\n"
					"                                    Unknown: %u\n"
					"                                       Zero: %02u %02u %02u %02u %02u %02u %02u %02u\n"
					"                                             %02u %02u %02u %02u %02u %02u %02u %02u %02u\n"
					"                         Track start sector: %u\n"
					"      Track start offset from begin of .MDF: %llu\n"
					"                        Number of Filenames: %u\n"
					"Offset to Filename Block from begin of .MDS: %u\n"
					"                                       Zero: %02u %02u %02u %02u %02u %02u %02u %02u\n"
					"                                             %02u %02u %02u %02u %02u %02u %02u %02u\n"
					"                                             %02u %02u %02u %02u %02u %02u %02u %02u\n"
					, db[i].ofsToIndexBlk, db[i].sectorSize, db[i].unknown1
					, db[i].zero2[0], db[i].zero2[1], db[i].zero2[2], db[i].zero2[3], db[i].zero2[4], db[i].zero2[5]
					, db[i].zero2[6], db[i].zero2[7], db[i].zero2[8], db[i].zero2[9], db[i].zero2[10], db[i].zero2[11]
					, db[i].zero2[12], db[i].zero2[13], db[i].zero2[14], db[i].zero2[15], db[i].zero2[16]
					, db[i].trackStartSector
					, db[i].trackStartOffset, db[i].NumOfFname, db[i].OfsToFname
					, db[i].zero3[0], db[i].zero3[1], db[i].zero3[2], db[i].zero3[3], db[i].zero3[4], db[i].zero3[5]
					, db[i].zero3[6], db[i].zero3[7], db[i].zero3[8], db[i].zero3[9], db[i].zero3[10], db[i].zero3[11]
					, db[i].zero3[12], db[i].zero3[13], db[i].zero3[14], db[i].zero3[15], db[i].zero3[16], db[i].zero3[17]
					, db[i].zero3[18], db[i].zero3[19], db[i].zero3[20], db[i].zero3[21], db[i].zero3[22], db[i].zero3[23]
				);
				break;
			}
		}
		if (h.mediaType != 0x10) {
			for (INT i = 0; i < tdb; i++) {
				if (ib) {
					OutputMdsReadableLog(
						OUTPUT_DHYPHEN_PLUS_STR("Index Block[%d]")
						"Number of sectors with Index 0: %u\n"
						"Number of sectors with Index 1: %u\n"
						, i + 1, ib[i].NumOfIdx0, ib[i].NumOfIdx1
					);
				}
			}
		}
		char fname[12];
		if (!WideCharToMultiByte(CP_ACP, 0,
			fb.fnameString, 6, fname, sizeof(fname), NULL, NULL)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		}
		OutputMdsReadableLog(
			OUTPUT_DHYPHEN_PLUS_STR("Filename Block")
			"Offset to Filename from begin of .MDS: %u\n"
			"                      Filename format: %u\n"
			"                      Filename string: %" CHARWIDTH "s\n"
			, fb.ofsToFname, fb.fnameFmt, fname
		);
		if (pdb && h.ofsToDpm > 0) {
			OutputMdsReadableLog(
				OUTPUT_DHYPHEN_PLUS_STR("DPM Block")
				"      dpmBlkTotalNum: %u\n"
				, pdb->dpmBlkTotalNum);
			for (UINT i = 0; i < pdb->dpmBlkTotalNum; i++) {
				OutputMdsReadableLog(
					"        ofsToDpmInfo: %u\n", pdb->ofsToDpmBlk[i]);
			}
			for (UINT i = 0; i < pdb->dpmBlkTotalNum; i++) {
				LPUINT diff = NULL;
				if (NULL == (diff = (LPUINT)calloc(pddb[i]->entry, sizeof(UINT)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				OutputMdsReadableLog(
					"           dpmBlkNum: %u\n"
					"            unknown1: %u\n"
					"          resolution: %u\n"
					"               entry: %u\n"
					, pddb[i]->dpmBlkNum, pddb[i]->unknown1, pddb[i]->resolution, pddb[i]->entry
				);
				OutputMdsReadableLog(
					"       0    readTime:                       %5u ms\n", pddb[i]->readTime[0]);
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
							OutputMdsReadableLog(" end changing\n");
							bStart = FALSE;
							bEnd = TRUE;
						}
						else if (k > 1) {
							OutputMdsReadableLog("\n");
						}
					}
					OutputMdsReadableLog(
						"%8u    readTime: %8u - %8u = %5u ms [%d]"
						, pddb[i]->resolution * k, pddb[i]->readTime[k], pddb[i]->readTime[k - 1], diff[k], diffAndDiff);
//					prevDiffAndDiff = diffAndDiff;
					if (pddb[i]->resolution == 50 || pddb[i]->resolution == 256) {
						// SecuROM
						if (9 < diffAndDiff && diffAndDiff < 35 && !bStart && bEnd) {
							OutputMdsReadableLog(" start changing");
							bStart = TRUE;
							bEnd = FALSE;
						}
						else if (!bStart && bEnd) {
							prevDiff = (INT)diff[k];
						}
					}
					else if (pddb[i]->resolution == 500 || pddb[i]->resolution == 2048) {
						// Starforce
						OutputMdsReadableLog("\n");
					}
					else {
						OutputMdsReadableLog("\n");
					}
				}
				OutputMdsReadableLog("\n");
				FreeAndNull(diff);
			}
		}
		FreeAndNull(data);
		FreeAndNull(psb);
		FreeAndNull(ib);
		FreeAndNull(db);
		FreeAndNull(dvd[0].bca);
		FreeAndNull(dvd[1].bca);
		if (pdb) {
			for (UINT i = 0; i < pdb->dpmBlkTotalNum; i++) {
				FreeAndNull(pddb[i]);
			}
		}
		FreeAndNull(pddb);
		FreeAndNull(pdb);
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
#ifndef DEBUG
	FcloseAndNull(g_LogFile.fpMdsReadable);
#endif
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
			OutputErrorString("Failed to read\n");
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
		OutputString("\rDescrambling img (LBA) %6lu/%6lu", i, dwAllSectorVal);
	}
	OutputString("\n");
	FcloseAndNull(fpImg);
	FcloseAndNull(fpScm);
	return bRet;
}

BOOL CreateBinCueForGD(
	PDISC pDisc,
	LPCTSTR pszPath
) {
	BOOL bRet = TRUE;
	_TCHAR pszImgName[_MAX_FNAME + _MAX_EXT] = {};
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
	LPUINT lpToc = NULL;
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
			OutputErrorString("No GD-ROM data. Size: %lu\n", dwFileSize);
			throw FALSE;
		}
		fseek(fpImg, 0x110, SEEK_SET);
		// 0x110 - 0x31F is toc data
		BYTE aToc[512] = {};
		if (fread(aToc, sizeof(BYTE), sizeof(aToc), fpImg) < sizeof(aToc)) {
			OutputErrorString("Failed to read [F:%s][L:%d]\n", _T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (aToc[0] != 'T' || aToc[1] != 'O' || aToc[2] != 'C' || aToc[3] != '1') {
			OutputErrorString("No GD-ROM data. Header: %c%c%c%c\n",
				aToc[0], aToc[1], aToc[2], aToc[3]);
			throw FALSE;
		}

		UINT uiMaxToc = 98 * 4;
		BYTE byMaxTrackNum = aToc[uiMaxToc + 4 * 1 + 2];
		lpToc = (LPUINT)calloc(byMaxTrackNum, sizeof(UINT));
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
				_T("1 %5d 4 2352 \"%s (Track %d).bin\" 0\n")
				_T("2 [fix] 0 2352 \"%s (Track %d).bin\" 0\n"),
				0, pszFname, 1,	pszFname, 2);
		}
		else if (10 <= byMaxTrackNum && lMaxLBA <= 99999) {
			_ftprintf(fpGdi,
				_T(" 1 %5d 4 2352 \"%s (Track %02d).bin\" 0\n")
				_T(" 2 [fix] 0 2352 \"%s (Track %02d).bin\" 0\n"),
				0, pszFname, 1, pszFname, 2);
		}
		else if (byMaxTrackNum <= 9 && 100000 <= lMaxLBA) {
			_ftprintf(fpGdi,
				_T("1 %6d 4 2352 \"%s (Track %d).bin\" 0\n")
				_T("2  [fix] 0 2352 \"%s (Track %d).bin\" 0\n"),
				0, pszFname, 1,	pszFname, 2);
		}
		else if (10 <= byMaxTrackNum && 100000 <= lMaxLBA) {
			_ftprintf(fpGdi,
				_T(" 1 %6d 4 2352 \"%s (Track %02d).bin\" 0\n")
				_T(" 2  [fix] 0 2352 \"%s (Track %02d).bin\" 0\n"),
				0, pszFname, 1,	pszFname, 2);
		}

		BYTE byTrackNum = 3;
		for (UINT i = 0; i < uiMaxToc; i += 4, byTrackNum++) {
			if (aToc[7 + i] == 0xff) {
				break;
			}
			BYTE byCtl = (BYTE)((aToc[7 + i] >> 4) & 0x0f);
			UINT lToc =
				MAKEUINT(MAKEWORD(aToc[4 + i], aToc[5 + i]), MAKEWORD(aToc[6 + i], 0));
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
				_ftprintf(fpGdi, _T("%u %5u %u 2352 \"%s (Track %u).bin\" 0\n"),
					byTrackNum, lpToc[byTrackNum - 3], byCtl, pszFname, byTrackNum);
			}
			else if (10 <= byMaxTrackNum && lMaxLBA <= 99999) {
				_ftprintf(fpGdi, _T("%2u %5u %u 2352 \"%s (Track %02u).bin\" 0\n"),
					byTrackNum, lpToc[byTrackNum - 3], byCtl, pszFname, byTrackNum);
			}
			else if (byMaxTrackNum <= 9 && 100000 <= lMaxLBA) {
				_ftprintf(fpGdi, _T("%u %6u %u 2352 \"%s (Track %u).bin\" 0\n"),
					byTrackNum, lpToc[byTrackNum - 3], byCtl, pszFname, byTrackNum);
			}
			else if (10 <= byMaxTrackNum && 100000 <= lMaxLBA) {
				_ftprintf(fpGdi, _T("%2u %6u %u 2352 \"%s (Track %02u).bin\" 0\n"),
					byTrackNum, lpToc[byTrackNum - 3], byCtl, pszFname, byTrackNum);
			}
		}
		UINT lToc = 
			MAKEUINT(MAKEWORD(aToc[uiMaxToc + 4 * 2], aToc[uiMaxToc + 4 * 2 + 1]),
			MAKEWORD(aToc[uiMaxToc + 4 * 2 + 2], 0)) - 150;
		lpToc[byTrackNum - 3] = lToc;

		rewind(fpImg);
		WriteCueForFileDirective(pszImgName, fpCueForImg);
		_TCHAR pszBinFname[_MAX_FNAME + _MAX_EXT] = {};

		for (BYTE i = 3; i <= byMaxTrackNum; i++) {
			OutputString("\rCreating bin, cue (Track) %2u/%2u", i, byMaxTrackNum);
			if (NULL == (fpBin = CreateOrOpenFile(pszPath, NULL, NULL,
				pszBinFname, NULL, _T(".bin"), _T("wb"), i, byMaxTrackNum))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			WriteCueForUnderFileDirective(pDisc, FALSE, 0, i, fpCueForImg);
			WriteCueForFileDirective(pszBinFname, fpCue);
			WriteCueForUnderFileDirective(pDisc, FALSE, 0, i, fpCue);

			BYTE index = 0;
			INT nLBAofFirstIdx = pDisc->SUB.lp1stLBAListOnSub[i - 1][0] - FIRST_LBA_FOR_GD;
			// nothing or index 0 in track 1
			if (nLBAofFirstIdx == -1 || nLBAofFirstIdx == -150) {
				nLBAofFirstIdx = pDisc->SUB.lp1stLBAListOnSub[i - 1][1] - FIRST_LBA_FOR_GD;
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
				INT nLBAofNextIdx = pDisc->SUB.lp1stLBAListOnSub[i - 1][index] - FIRST_LBA_FOR_GD;
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
				OutputErrorString("Failed to read [L:%d]\n", __LINE__);
			}
			fwrite(lpBuf, sizeof(BYTE), size, fpBin);
			FcloseAndNull(fpBin);
			FreeAndNull(lpBuf);
		}
		OutputString("\n");
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
	LPINT lp1stLBAList,
	LPINT lpLastLBAList,
	LPCTSTR pszType,
	LPBYTE lpScrambledBuf,
	FILE* fpImg
) {
	BYTE aSrcBuf[CD_RAW_SECTOR_SIZE] = {};
	LONG lSeekPtr = 0;

	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("Data sector range (%s)"), pszType);
	for (INT k = pDisc->SCSI.by1stDataTrkNum - 1; k < pDisc->SCSI.byLastDataTrkNum; k++) {
		INT n1stLBA = lp1stLBAList[k];
		if (n1stLBA != -1) {
			INT nLastLBA = lpLastLBAList[k];
			OutputDiscLog("\tTrack %2d, %6d - %6d (%#07x - %#07x)\n",
				k + 1, n1stLBA, nLastLBA, (UINT)n1stLBA, (UINT)nLastLBA);
			if (pDisc->SCSI.lpSessionNumList[k] >= 2) {
				INT nSkipLBA = (SESSION_TO_SESSION_SKIP_LBA * (INT)(pDisc->SCSI.lpSessionNumList[k] - 1));
				n1stLBA -= nSkipLBA;
				nLastLBA -= nSkipLBA;
			}
			if (pExtArg->byPre) {
//				n1stLBA += 75;
				nLastLBA += 150;
			}
			if (!pExtArg->byReverse) {
				lSeekPtr = n1stLBA;
			}
			for (; n1stLBA <= nLastLBA; n1stLBA++, lSeekPtr++) {
				// ファイルを読み書き両用モードで開いている時は 注意が必要です。
				// 読み込みを行った後に書き込みを行う場合やその逆を行う場合は、 
				// 必ずfseekを呼ばなければなりません。もしこれを忘れると、
				// 場合によってはバッファー内と 実際にディスクに描き込まれた
				// データに矛盾が生じ、正確に書き込まれない場合や、
				// 嘘の データを読み込む場合があります。
				fseek(fpImg, lSeekPtr * CD_RAW_SECTOR_SIZE, SEEK_SET);
				if (fread(aSrcBuf, sizeof(BYTE), sizeof(aSrcBuf), fpImg) < sizeof(aSrcBuf)) {
					OutputErrorString("LBA[%06d, %#07x]: Failed to read [F:%s][L:%d]\n"
						, n1stLBA, (UINT)n1stLBA, _T(__FUNCTION__), __LINE__);
					break;
				}
				if (IsValidMainDataHeader(aSrcBuf)) {
					if (aSrcBuf[0x0f] == 0x60) {
						for (INT n = 0x10; n < CD_RAW_SECTOR_SIZE; n++) {
							if (aSrcBuf[n] != lpScrambledBuf[n]) {
								OutputMainErrorWithLBALog("Not all zero sector\n", n1stLBA, k + 1);
								OutputString(
									"\rDescrambling data sector of img: %6d/%6d", n1stLBA, nLastLBA);
								OutputMainChannel(fileMainError, aSrcBuf, NULL, n1stLBA, CD_RAW_SECTOR_SIZE);
								continue;
							}
						}
					}
					else if (aSrcBuf[0x0f] == 0x61) {
						if (IsValidReservedByte(aSrcBuf)) {
							OutputMainErrorWithLBALog("A part of reversed sector. (Not be scrambled)\n", n1stLBA, k + 1);
							OutputMainChannel(fileMainError, aSrcBuf, NULL, n1stLBA, CD_RAW_SECTOR_SIZE);
						}
					}
					else if (aSrcBuf[0x0f] == 0x00 || aSrcBuf[0x0f] == 0x01 || aSrcBuf[0x0f] == 0x02) {
						OutputMainErrorWithLBALog("Reversed sector. (Not be scrambled)\n", n1stLBA, k + 1);
						OutputMainChannel(fileMainError, aSrcBuf, NULL, n1stLBA, CD_RAW_SECTOR_SIZE);
					}
					else if (aSrcBuf[0x0f] != 0x60 && aSrcBuf[0x0f] != 0x61 && aSrcBuf[0x0f] != 0x62 &&
						aSrcBuf[0x0f] != 0x00 && aSrcBuf[0x0f] != 0x01 && aSrcBuf[0x0f] != 0x02) {
						OutputMainErrorWithLBALog("Invalid mode. ", n1stLBA, k + 1);
						BYTE m, s, f = 0;
						LBAtoMSF(n1stLBA + 150, &m, &s, &f);
						if (aSrcBuf[0x0c] == m && aSrcBuf[0x0d] == s && aSrcBuf[0x0e] == f) {
							OutputMainErrorLog("Reversed sector. (Not be scrambled)");
							if (!IsValidReservedByte(aSrcBuf)) {
								OutputMainErrorLog(" Invalid reserved byte");
							}
							OutputMainErrorLog("\n");
						}
						else if (IsValidReservedByte(aSrcBuf)) {
							OutputMainErrorLog("A part of reversed sector. (Not be scrambled)\n");
						}
						else if (aSrcBuf[0x814] != 0x48 || aSrcBuf[0x815] != 0x64 || aSrcBuf[0x816] != 0x36 ||
							aSrcBuf[0x817] != 0xab || aSrcBuf[0x818] != 0x56 || aSrcBuf[0x819] != 0xff ||
							aSrcBuf[0x81a] != 0x7e || aSrcBuf[0x81b] != 0xc0) {
							OutputMainErrorLog("Invalid reserved byte\n");
						}
						else {
							OutputMainErrorLog("\n");
						}
						OutputMainChannel(fileMainError, aSrcBuf, NULL, n1stLBA, CD_RAW_SECTOR_SIZE);
					}
					fseek(fpImg, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
					for (INT n = 0; n < CD_RAW_SECTOR_SIZE; n++) {
						aSrcBuf[n] ^= lpScrambledBuf[n];
					}
					fwrite(aSrcBuf, sizeof(BYTE), sizeof(aSrcBuf), fpImg);
				}
				else {
					if (pDisc->SCSI.trkType != TRACK_TYPE::pregapAudioIn1stTrack &&
						pDisc->SCSI.trkType != TRACK_TYPE::pregapDataIn1stTrack) {
						OutputMainErrorWithLBALog("Invalid sync. Skip descrambling\n", n1stLBA, k + 1);
						OutputMainChannel(fileMainError, aSrcBuf, NULL, n1stLBA, CD_RAW_SECTOR_SIZE);
					}
				}
				OutputString(
					"\rDescrambling data sector of img: %6d/%6d", n1stLBA, nLastLBA);
			}
			OutputString("\n");
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
			OutputErrorString("Failed to read [F:%s][L:%d]\n", _T(__FUNCTION__), __LINE__);
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
				OutputMainInfoWithLBALog("Invalid mode. Skip descrambling\n", nStartLBA, 0);
				OutputMainChannel(fileMainInfo, aSrcBuf, NULL, nStartLBA, CD_RAW_SECTOR_SIZE);
			}
		}
		else {
			OutputMainErrorWithLBALog("Invalid sync. Skip descrambling\n", nStartLBA, 0);
			OutputMainChannel(fileMainError, aSrcBuf, NULL, nStartLBA, CD_RAW_SECTOR_SIZE);
		}
		OutputString(
			"\rDescrambling data sector of img: %6d/%6d", nStartLBA, nEndLBA);
	}
	OutputString("\n");
}

BOOL CreateBin(
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
		// last 1st session track
		if (pDisc->SCSI.lpSessionNumList[byTrackNum - 1] != pDisc->SCSI.lpSessionNumList[byTrackNum]) {
			nLBA -= SESSION_TO_SESSION_SKIP_LBA;
		}
		stBufSize = (size_t)nLBA * CD_RAW_SECTOR_SIZE;
		nPrevLBA = 0;
	}
	else if (byTrackNum == pDisc->SCSI.toc.LastTrack) {
		INT nTmpLength = pDisc->SCSI.nAllLength;
		if (pDisc->SCSI.by1stMultiSessionTrkNum) {
			INT nSessionSize =
				SESSION_TO_SESSION_SKIP_LBA * (pDisc->SCSI.lpSessionNumList[byTrackNum - 1] - 1);
			nPrevLBA -= nSessionSize;
			nTmpLength -= nSessionSize;
			nLBA = nTmpLength;
		}
		stBufSize = (size_t)(nTmpLength - nPrevLBA) * CD_RAW_SECTOR_SIZE;
	}
	else {
		INT nSessionSize =
			SESSION_TO_SESSION_SKIP_LBA * (pDisc->SCSI.lpSessionNumList[byTrackNum] - 1);
		// last 1st session track
		if (byTrackNum >= 1 && pDisc->SCSI.lpSessionNumList[byTrackNum - 1] != pDisc->SCSI.lpSessionNumList[byTrackNum]) {
			nLBA -= nSessionSize;
		}
		// first 2nd session track
		else if (byTrackNum >= 2 && pDisc->SCSI.lpSessionNumList[byTrackNum - 1] >= 2) {
				nPrevLBA -= nSessionSize;
				nLBA -= nSessionSize;
		}
		stBufSize = (size_t)(nLBA - nPrevLBA) * CD_RAW_SECTOR_SIZE;
	}
#if 0
	OutputString(" nLBA - nPrevLBA (%d - %d) --> %d\n", nLBA, nPrevLBA, nLBA - nPrevLBA);
#endif
	fseek(fpImg, nPrevLBA * CD_RAW_SECTOR_SIZE, SEEK_SET);
	LPBYTE lpBuf = (LPBYTE)calloc(stBufSize, sizeof(BYTE));
	if (!lpBuf) {
		OutputString("\n");
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("bufSize: %zu", stBufSize);
		return FALSE;
	}
	if (fread(lpBuf, sizeof(BYTE), stBufSize, fpImg) < stBufSize) {
		OutputErrorString("Failed to read: read size %zu [F:%s][L:%d]\n", stBufSize, _T(__FUNCTION__), __LINE__);
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
	FILE* fpImgDesync,
	FILE* fpCcd
) {
	BOOL bRet = TRUE;
	FILE* fpCue = NULL;
	FILE* fpCueForImg = NULL;
	FILE* fpCueSyncForImg = NULL;
	FILE* fpCueSync = NULL;

	if (NULL == (fpCue = CreateOrOpenFile(
		pszPath, NULL, NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	try {
		if (NULL == (fpCueForImg = CreateOrOpenFile(
			pszPath, _T("_img"), NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		WriteCueForFirst(pDisc, bCanCDText, 0, fpCueForImg);
		WriteCueForFileDirective(pszImgName, fpCueForImg);
		WriteCueForFirst(pDisc, bCanCDText, 0, fpCue);

		if (pDisc->SUB.byIdxDesync) {
			if (NULL == (fpCueSyncForImg = CreateOrOpenFile(
				pszPath, _T(" (Subs indexes)_img"), NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (fpCueSync = CreateOrOpenFile(
				pszPath, _T(" (Subs indexes)"), NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				FcloseAndNull(fpCueSyncForImg);
				throw FALSE;
			}
			WriteCueForFirst(pDisc, bCanCDText, 0, fpCueSyncForImg);
			WriteCueForFileDirective(pszImgName, fpCueSyncForImg);
			WriteCueForMultiSessionWholeBin(pDisc, 1, fpCueSyncForImg);
			WriteCueForFirst(pDisc, bCanCDText, 0, fpCueSync);
		}

		_TCHAR pszFname[_MAX_FNAME + _MAX_EXT] = {};
		_TCHAR pszAltCueStr[][6] = {
			_T(""), _T("_alt"), _T("_alt2"), _T("_alt3"), _T("_alt4"), _T("_alt5"), _T("_alt6"), _T("_alt7")
		};
		_TCHAR pszImgAltCueStr[][9] = {
			_T("_img"), _T("_imgAlt"), _T("_imgAlt2"), _T("_imgAlt3"), _T("_imgAlt4"), _T("_imgAlt5"), _T("_imgAlt6"), _T("_imgAlt7")
		};
		FILE* fpBin = NULL;
		FILE* fpBinIdxDesync = NULL;
		FILE* fpBinCtlDesync = NULL;
		for (BYTE j = 0; j < MAX_CDTEXT_LANG; j++) {
			if (0 < j) {
				if (pDisc->SCSI.CDTEXT[j].bExist) {
					// This fpBin is dummy to get the AltCue filename
					_TCHAR out[_MAX_PATH] = {};
					if (NULL == (fpBin = CreateOrOpenFile(pszPath, pszAltCueStr[j], out, NULL,
						NULL, _T(".bin"), _T("wb"), 0, 0))) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						bRet = FALSE;
						break;
					}
					FcloseAndNull(fpBin);
					_tremove(out);
					if (NULL == (fpCue = CreateOrOpenFile(
						out, NULL, NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
					if (NULL == (fpCueForImg = CreateOrOpenFile(
						pszPath, pszImgAltCueStr[j], NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
					WriteCueForFirst(pDisc, bCanCDText, j, fpCueForImg);
					WriteCueForFileDirective(pszImgName, fpCueForImg);
					WriteCueForFirst(pDisc, bCanCDText, j, fpCue);
				}
				else {
					break;
				}
			}
			for (BYTE i = pDisc->SCSI.toc.FirstTrack; i <= pDisc->SCSI.toc.LastTrack; i++) {
				OutputString(
					"\rCreating cue and ccd (Track) %2u/%2u", i, pDisc->SCSI.toc.LastTrack);
				// This fpBin is dummy to get filename and ext written in cue
				_TCHAR out[_MAX_PATH] = {};
				if (NULL == (fpBin = CreateOrOpenFile(pszPath, NULL, out, pszFname,
					NULL, _T(".bin"), _T("wb"), i, pDisc->SCSI.toc.LastTrack))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					bRet = FALSE;
					break;
				}
				WriteCueForMultiSessionWholeBin(pDisc, i, fpCueForImg);
				WriteCueForUnderFileDirective(pDisc, bCanCDText, j, i, fpCueForImg);
				WriteCueForMultiSessionMultiBin(pDisc, i, fpCue);
				WriteCueForFileDirective(pszFname, fpCue);
				WriteCueForUnderFileDirective(pDisc, bCanCDText, j, i, fpCue);
				if (j == 0) {
					WriteCcdForTrack(pDisc, i, fpCcd);
				}

				_TCHAR pszFnameSync[_MAX_FNAME + _MAX_EXT] = {};
				if (pDisc->SUB.byIdxDesync) {
					// This fpBin is dummy
					if (NULL == (fpBinIdxDesync = CreateOrOpenFile(pszPath, _T(" (Subs indexes)"), out,
						pszFnameSync, NULL, _T(".bin"), _T("wb"), i, pDisc->SCSI.toc.LastTrack))) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						bRet = FALSE;
						break;
					}
					FcloseAndNull(fpBinIdxDesync);
					_tremove(out);
					if (j == 0) {
						WriteCueForUnderFileDirective(pDisc, bCanCDText, j, i, fpCueSyncForImg);
					}
					WriteCueForMultiSessionMultiBin(pDisc, i, fpCueSync);
					WriteCueForFileDirective(pszFnameSync, fpCueSync);
					WriteCueForUnderFileDirective(pDisc, bCanCDText, j, i, fpCueSync);
				}

				BYTE index = 0;
				INT nLBAofFirstIdx = pDisc->SUB.lp1stLBAListOnSub[i - 1][0];
				// nothing or index 0 in track 1
				if (nLBAofFirstIdx == -1 || nLBAofFirstIdx == -150 ||
					(pDisc->SCSI.bMultiSession && i == pDisc->SCSI.by1stMultiSessionTrkNum)) {
					nLBAofFirstIdx = pDisc->SUB.lp1stLBAListOnSub[i - 1][1];
					index++;
				}

				BYTE indexSync = 0;
				INT nLBAofFirstIdxSync = pDisc->SUB.lp1stLBAListOnSubSync[i - 1][0];
				if (nLBAofFirstIdxSync == -1 || nLBAofFirstIdxSync == -150) {
					nLBAofFirstIdxSync = pDisc->SUB.lp1stLBAListOnSubSync[i - 1][1];
					indexSync++;
				}

				BYTE byFrame = 0, bySecond = 0, byMinute = 0;
				if (i == pDisc->SCSI.toc.FirstTrack ||
					(pDisc->SCSI.bMultiSession && i == pDisc->SCSI.by1stMultiSessionTrkNum)) {
					if (0 == nLBAofFirstIdx || (i == pDisc->SCSI.by1stMultiSessionTrkNum &&
						pDisc->SCSI.trkType != TRACK_TYPE::pregapAudioIn1stTrack &&
						pDisc->SCSI.trkType != TRACK_TYPE::pregapDataIn1stTrack)) {

						WriteCueForIndexDirective(index, 0, 0, 0, fpCue);
						if (pDisc->SCSI.bMultiSession && i == pDisc->SCSI.by1stMultiSessionTrkNum) {
							LBAtoMSF(nLBAofFirstIdx, &byMinute, &bySecond, &byFrame);
							WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueForImg);
							if (j == 0) {
								WriteCcdForTrackIndex(index, nLBAofFirstIdx, fpCcd);
							}
						}
						else {
							WriteCueForIndexDirective(index, 0, 0, 0, fpCueForImg);
							if (j == 0) {
								WriteCcdForTrackIndex(index, 0, fpCcd);
							}
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
						if (j == 0) {
							WriteCcdForTrackIndex(0, 0, fpCcd);
						}

						if (pExtArg->byPre) {
							nLBAofFirstIdx += 150;
						}
						LBAtoMSF(nLBAofFirstIdx, &byMinute, &bySecond, &byFrame);
						WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueForImg);
						WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCue);
						if (j == 0) {
							WriteCcdForTrackIndex(index, nLBAofFirstIdx, fpCcd);
						}
					}
					index++;

					if (pDisc->SUB.byIdxDesync) {
						if (0 == nLBAofFirstIdxSync || (i == pDisc->SCSI.by1stMultiSessionTrkNum &&
							pDisc->SCSI.trkType != TRACK_TYPE::pregapAudioIn1stTrack &&
							pDisc->SCSI.trkType != TRACK_TYPE::pregapDataIn1stTrack)) {

							WriteCueForIndexDirective(indexSync, 0, 0, 0, fpCueSyncForImg);
							WriteCueForIndexDirective(indexSync, 0, 0, 0, fpCueSync);
						}
						else if (0 < nLBAofFirstIdxSync) {
							WriteCueForIndexDirective(0, 0, 0, 0, fpCueSyncForImg);
							WriteCueForIndexDirective(0, 0, 0, 0, fpCueSync);

							if (pExtArg->byPre) {
								nLBAofFirstIdxSync += 150;
							}
							LBAtoMSF(nLBAofFirstIdxSync, &byMinute, &bySecond, &byFrame);
							WriteCueForIndexDirective(indexSync, byMinute, bySecond, byFrame, fpCueSyncForImg);
							WriteCueForIndexDirective(indexSync, byMinute, bySecond, byFrame, fpCueSync);
						}
						indexSync++;
					}
				}
				else if (pExtArg->byPre) {
					nLBAofFirstIdx += 150;
					if (pDisc->SUB.byIdxDesync) {
						nLBAofFirstIdxSync += 150;
					}
				}

				for (; index < MAXIMUM_NUMBER_INDEXES; index++) {
					INT nLBAofNextIdx = pDisc->SUB.lp1stLBAListOnSub[i - 1][index];
					if (nLBAofNextIdx != -1) {
						if (pExtArg->byPre) {
							nLBAofNextIdx += 150;
						}
						LBAtoMSF(nLBAofNextIdx, &byMinute, &bySecond, &byFrame);
						WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueForImg);

						INT nTmpLBA = nLBAofNextIdx - nLBAofFirstIdx;
						if (2 <= index && i == pDisc->SCSI.toc.FirstTrack) {
							nTmpLBA += pDisc->SUB.lp1stLBAListOnSub[i - 1][1];
						}
						LBAtoMSF(nTmpLBA, &byMinute, &bySecond, &byFrame);
						WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCue);

						if (j == 0) {
							WriteCcdForTrackIndex(index, nLBAofNextIdx, fpCcd);
						}
					}
					else {
						if (index >= 2) {
							break;
						}
					}
				}
				if (pDisc->SUB.byIdxDesync) {
					for (; indexSync < MAXIMUM_NUMBER_INDEXES; indexSync++) {
						INT nLBAofNextIdxSync = pDisc->SUB.lp1stLBAListOnSubSync[i - 1][indexSync];
						if (nLBAofNextIdxSync != -1) {
							if (pExtArg->byPre) {
								nLBAofNextIdxSync += 150;
							}
							LBAtoMSF(nLBAofNextIdxSync, &byMinute, &bySecond, &byFrame);
							WriteCueForIndexDirective(indexSync, byMinute, bySecond, byFrame, fpCueSyncForImg);

							INT nTmpLBA = nLBAofNextIdxSync - nLBAofFirstIdxSync;
							if (2 <= index && i == pDisc->SCSI.toc.FirstTrack) {
								nTmpLBA += pDisc->SUB.lp1stLBAListOnSubSync[i - 1][1];
							}
							LBAtoMSF(nTmpLBA, &byMinute, &bySecond, &byFrame);
							WriteCueForIndexDirective(indexSync, byMinute, bySecond, byFrame, fpCueSync);
						}
						else {
							if (indexSync >= 2) {
								break;
							}
						}
					}
				}
			}
			FcloseAndNull(fpCueForImg);
			FcloseAndNull(fpCue);
			FcloseAndNull(fpCueSyncForImg);
			FcloseAndNull(fpCueSync);
		}
		OutputString("\n");
		for (BYTE i = pDisc->SCSI.toc.FirstTrack; i <= pDisc->SCSI.toc.LastTrack; i++) {
			OutputString(
				"\rCreating bin (Track) %2u/%2u", i, pDisc->SCSI.toc.LastTrack);

			if (NULL == (fpBin = CreateOrOpenFile(pszPath, NULL, NULL, NULL,
				NULL, _T(".bin"), _T("wb"), i, pDisc->SCSI.toc.LastTrack))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				bRet = FALSE;
				break;
			}
			INT nLBA = pDisc->SUB.lp1stLBAListOnSub[i - 1][0] == -1 ?
				pDisc->SUB.lp1stLBAListOnSub[i - 1][1] : pDisc->SUB.lp1stLBAListOnSub[i - 1][0];
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
			INT nNextLBA = pDisc->SUB.lp1stLBAListOnSub[i][0] == -1 ?
				pDisc->SUB.lp1stLBAListOnSub[i][1] : pDisc->SUB.lp1stLBAListOnSub[i][0];
			if (pExtArg->byPre) {
				nNextLBA += 150;
			}
#ifdef _DEBUG
			OutputDebugStringEx(" nNextLBA(%d) - nLBA(%d) = %d\n", nNextLBA, nLBA, nNextLBA - nLBA);
#endif
			bRet = CreateBin(pDisc, i, nNextLBA, nLBA, fpImg, fpBin);
			FcloseAndNull(fpBin);
			if (!bRet) {
				break;
			}
			if (pDisc->SUB.byCtlDesync) {
				if (NULL == (fpBinCtlDesync = CreateOrOpenFile(pszPath, _T(" (Subs control)"), NULL,
					NULL, NULL, _T(".bin"), _T("wb"), i, pDisc->SCSI.toc.LastTrack))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					bRet = FALSE;
					break;
				}
				bRet = CreateBin(pDisc, i, nNextLBA, nLBA, fpImgDesync, fpBinCtlDesync);
				FcloseAndNull(fpBinCtlDesync);
				if (!bRet) {
					break;
				}
			}
			if (pDisc->SUB.byIdxDesync) {
				if (NULL == (fpBinIdxDesync = CreateOrOpenFile(pszPath, _T(" (Subs indexes)"), NULL,
					NULL, NULL, _T(".bin"), _T("wb"), i, pDisc->SCSI.toc.LastTrack))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					bRet = FALSE;
					break;
				}
				nLBA = pDisc->SUB.lp1stLBAListOnSubSync[i - 1][0] == -1 ?
					pDisc->SUB.lp1stLBAListOnSubSync[i - 1][1] : pDisc->SUB.lp1stLBAListOnSubSync[i - 1][0];
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
				nNextLBA = pDisc->SUB.lp1stLBAListOnSubSync[i][0] == -1 ?
					pDisc->SUB.lp1stLBAListOnSubSync[i][1] : pDisc->SUB.lp1stLBAListOnSubSync[i][0];
				if (pExtArg->byPre) {
					nNextLBA += 150;
				}
				bRet = CreateBin(pDisc, i, nNextLBA, nLBA, fpImg, fpBinIdxDesync);
				FcloseAndNull(fpBinIdxDesync);
				if (!bRet) {
					break;
				}
			}
		}
		OutputString("\n");
		FcloseAndNull(fpBinCtlDesync);
		FcloseAndNull(fpBinIdxDesync);
		FcloseAndNull(fpBin);
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpCueForImg);
	FcloseAndNull(fpCue);
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
	OutputSubIntentionalLog(
		"MSF: %02d:%02d:%02d Q-Data: %02x%02x%02x %02x:%02x:%02x %02x %02x:%02x:%02x %02x%02x\n"
		, m, s, f, lpSubcode[0], lpSubcode[1], lpSubcode[2], lpSubcode[3], lpSubcode[4], lpSubcode[5]
		, lpSubcode[6], lpSubcode[7], lpSubcode[8], lpSubcode[9], lpSubcode[10], lpSubcode[11]
	);

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
	OutputErrorString("[F:%s][L:%lu] GetLastError: %lu, %s\n",
		pszFuncName, lLineNum, GetLastError(), (LPCTSTR)lpMsgBuf);

	LocalFree(lpMsgBuf);
#else
	OutputErrorString("[F:%s][L:%ld] GetLastError: %d, %s\n",
		pszFuncName, lLineNum, GetLastError(), strerror(GetLastError()));
#endif
}
#ifdef _WIN32
VOID OutputProductType(
	DWORD dwProductType
) {
	switch (dwProductType) {
	case PRODUCT_ULTIMATE:
		OutputString("Ultimate");
		break;
	case PRODUCT_HOME_BASIC:
		OutputString("Home Basic");
		break;
	case PRODUCT_HOME_PREMIUM:
		OutputString("Home Premium");
		break;
	case PRODUCT_ENTERPRISE:
		OutputString("Enterprise");
		break;
	case PRODUCT_HOME_BASIC_N:
		OutputString("Home Basic N");
		break;
	case PRODUCT_BUSINESS:
		OutputString("Business");
		break;
	case PRODUCT_STANDARD_SERVER:
		OutputString("Standard");
		break;
	case PRODUCT_DATACENTER_SERVER:
		OutputString("Datacenter");
		break;
	case PRODUCT_SMALLBUSINESS_SERVER:
		OutputString("Small Business");
		break;
	case PRODUCT_ENTERPRISE_SERVER:
		OutputString("Enterprise");
		break;
	case PRODUCT_STARTER:
		OutputString("Starter");
		break;
	case PRODUCT_DATACENTER_SERVER_CORE:
		OutputString("Datacenter (core installation)");
		break;
	case PRODUCT_STANDARD_SERVER_CORE:
		OutputString("Standard (core installation)");
		break;
	case PRODUCT_ENTERPRISE_SERVER_CORE:
		OutputString("Enterprise (core installation)");
		break;
	case PRODUCT_ENTERPRISE_SERVER_IA64:
		OutputString("Enterprise for Itanium-based Systems");
		break;
	case PRODUCT_BUSINESS_N:
		OutputString("Business N");
		break;
	case PRODUCT_WEB_SERVER:
		OutputString("Web");
		break;
	case PRODUCT_CLUSTER_SERVER:
		OutputString("Cluster");
		break;
	case PRODUCT_HOME_SERVER:
		OutputString("Home");
		break;
	case PRODUCT_STORAGE_EXPRESS_SERVER:
		OutputString("Storage Express");
		break;
	case PRODUCT_STORAGE_STANDARD_SERVER:
		OutputString("Strage Standard");
		break;
	case PRODUCT_STORAGE_WORKGROUP_SERVER:
		OutputString("Storage Workgroup");
		break;
	case PRODUCT_STORAGE_ENTERPRISE_SERVER:
		OutputString("Storage Enterprise");
		break;
	case PRODUCT_SERVER_FOR_SMALLBUSINESS:
		OutputString("for Small Business");
		break;
	case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
		OutputString("Small Business Premium");
		break;
	case PRODUCT_HOME_PREMIUM_N:
		OutputString("Home Premium N");
		break;
	case PRODUCT_ENTERPRISE_N:
		OutputString("Enterprise N");
		break;
	case PRODUCT_ULTIMATE_N:
		OutputString("Ultimate N");
		break;
	case PRODUCT_WEB_SERVER_CORE:
		OutputString("Web (core installation)");
		break;
	case PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:
		OutputString("Medium Business Server Management");
		break;
	case PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:
		OutputString("Medium Business Server Security");
		break;
	case PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:
		OutputString("Medium Business Server Messaging");
		break;
	case PRODUCT_SERVER_FOUNDATION:
		OutputString("Foundation");
		break;
	case PRODUCT_HOME_PREMIUM_SERVER:
		OutputString("Home Premium");
		break;
	case PRODUCT_SERVER_FOR_SMALLBUSINESS_V:
		OutputString("for Small Business without Hyper-V");
		break;
	case PRODUCT_STANDARD_SERVER_V:
		OutputString("Standard without Hyper-V");
		break;
	case PRODUCT_DATACENTER_SERVER_V:
		OutputString("Datacenter without Hyper-V (full installation)");
		break;
	case PRODUCT_ENTERPRISE_SERVER_V:
		OutputString("Enterprise without Hyper-V (full installation)");
		break;
	case PRODUCT_DATACENTER_SERVER_CORE_V:
		OutputString("Datacenter without Hyper-V (core installation)");
		break;
	case PRODUCT_STANDARD_SERVER_CORE_V:
		OutputString("Standard without Hyper-V (core installation)");
		break;
	case PRODUCT_ENTERPRISE_SERVER_CORE_V:
		OutputString("Enterprise without Hyper-V (core installation)");
		break;
	case PRODUCT_HYPERV:
		OutputString("without Hyper-V");
		break;
	case PRODUCT_STORAGE_EXPRESS_SERVER_CORE:
		OutputString("Storage Express (core installation)");
		break;
	case PRODUCT_STORAGE_STANDARD_SERVER_CORE:
		OutputString("Storage Strandard (core installation)");
		break;
	case PRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
		OutputString("Storage Workgroup (core installation)");
		break;
	case PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
		OutputString("Storage Enterprise (core installation)");
		break;
	case PRODUCT_STARTER_N:
		OutputString("Starter N");
		break;
	case PRODUCT_PROFESSIONAL:
		OutputString("Professional");
		break;
	case PRODUCT_PROFESSIONAL_N:
		OutputString("Professional N");
		break;
	case PRODUCT_SB_SOLUTION_SERVER:
		OutputString("SB Solution");
		break;
	case PRODUCT_SERVER_FOR_SB_SOLUTIONS:
		OutputString("for SB Solutions");
		break;
	case PRODUCT_STANDARD_SERVER_SOLUTIONS:
		OutputString("Standard Server Solutions");
		break;
	case PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE:
		OutputString("Standard Server Solutions (core installation)");
		break;
	case PRODUCT_SB_SOLUTION_SERVER_EM:
		OutputString("SB Solutions Server EM");
		break;
	case PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM:
		OutputString("for SB Solutions EM");
		break;
	case PRODUCT_SOLUTION_EMBEDDEDSERVER:
		OutputString("Solution Embedded Server");
		break;
	case PRODUCT_SOLUTION_EMBEDDEDSERVER_CORE:
		OutputString("Solution Embedded Server (core installation)");
		break;
	case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE:
		OutputString("Small Business Server Premium (core installation)");
		break;
	case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT:
		OutputString("Essential Business Server MGMT");
		break;
	case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL:
		OutputString("Essential Business Server ADDL");
		break;
	case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC:
		OutputString("Essential Business Server MGMTSVC");
		break;
	case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC:
		OutputString("Essential Business Server ADDLSVC");
		break;
	case PRODUCT_CLUSTER_SERVER_V:
		OutputString("Cluster without Hyper-V");
		break;
	case PRODUCT_EMBEDDED:
		OutputString("Embedded");
		break;
	case PRODUCT_STARTER_E:
		OutputString("Starter E");
		break;
	case PRODUCT_HOME_BASIC_E:
		OutputString("Home Basic E");
		break;
	case PRODUCT_HOME_PREMIUM_E:
		OutputString("Home Premium E");
		break;
	case PRODUCT_PROFESSIONAL_E:
		OutputString("Professional E");
		break;
	case PRODUCT_ENTERPRISE_E:
		OutputString("Enterprise E");
		break;
	case PRODUCT_ULTIMATE_E:
		OutputString("Ultimate E");
		break;
	default:
		OutputString("Other");
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
	OutputString("OS\n");
	switch (OSver.dwMajorVersion) {
	case 5:
		switch (OSver.dwMinorVersion) {
		case 0:
			OutputString("\tWindows 2000 ");
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				OutputString("Professional");
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				if (OSver.wSuiteMask & VER_SUITE_ENTERPRISE) {
					OutputString("Advanced Server");
				}
				else if (OSver.wSuiteMask & VER_SUITE_DATACENTER) {
					OutputString("Datacenter Server");
				}
				else {
					OutputString("Server");
				}
				break;
			}
			break;
		case 1:
			OutputString("\tWindows XP ");
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				if (OSver.wSuiteMask & VER_SUITE_PERSONAL) {
					OutputString("Home Edition");
				}
				else if (OSver.wSuiteMask & VER_SUITE_EMBEDDEDNT) {
					OutputString("Embedded");
				}
				else {
					OutputString("Professional");
				}
				break;
			}
			break;
		case 2:
			OutputString("\tWindows Server 2003 ");
			switch (OSver.wProductType) {
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				if (OSver.wSuiteMask & VER_SUITE_SMALLBUSINESS) {
					OutputString("Small Business Server");
				}
				else if (OSver.wSuiteMask & VER_SUITE_ENTERPRISE) {
					OutputString("Enterprise Edition");
				}
				else if (OSver.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED) {
					OutputString("Small Business Server with the restrictive client license");
				}
				else if (OSver.wSuiteMask & VER_SUITE_DATACENTER) {
					OutputString("Datacenter Edition");
				}
				else if (OSver.wSuiteMask & VER_SUITE_BLADE) {
					OutputString("Web Edition");
				}
				else if (OSver.wSuiteMask & VER_SUITE_STORAGE_SERVER) {
					OutputString("Storage Server Edition");
				}
				else if (OSver.wSuiteMask & VER_SUITE_COMPUTE_SERVER) {
					OutputString("Compute Cluster Edition");
				}
				else {
					OutputString("Other");
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
				OutputString("\tWindows Vista ");
				OutputProductType(dwProductType);
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				OutputString("\tWindows Server 2008 ");
				OutputProductType(dwProductType);
				break;
			}
			break;
		case 1:
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				OutputString("\tWindows 7 ");
				OutputProductType(dwProductType);
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				OutputString("\tWindows Server 2008 R2 ");
				OutputProductType(dwProductType);
				break;
			}
			break;
		case 2:
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				OutputString("\tWindows 8 ");
				OutputProductType(dwProductType);
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				OutputString("\tWindows Server 2012 ");
				OutputProductType(dwProductType);
				break;
			}
			break;
		case 3:
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				OutputString("\tWindows 8.1 ");
				OutputProductType(dwProductType);
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				OutputString("\tWindows Server 2012 R2 ");
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
				OutputString("\tWindows 10 ");
				OutputProductType(dwProductType);
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				OutputString("\tWindows Server 2016 ");
				OutputProductType(dwProductType);
				break;
			}
			break;
		}
	}
	OutputString(" %s ", OSver.szCSDVersion);
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
	OutputString("%dbit\n", bit);
#ifdef _DEBUG
	OutputString(
		"\tMajorVersion: %lu, MinorVersion: %lu, BuildNumber: %lu, PlatformId: %lu\n"
		"\tServicePackMajor: %u, ServicePackMinor: %u, SuiteMask: %u, ProductType: %u\n",
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
	if (NULL == (fpSrc1 = CreateOrOpenFile(
		pszFullPath, NULL, NULL, NULL, NULL, _T(".bin"), _T("rb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fpSrc2 = NULL;
	if (NULL == (fpSrc2 = CreateOrOpenFile(
		pszFullPath2, NULL, NULL, NULL, NULL, _T(".bin"), _T("rb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FcloseAndNull(fpSrc1);
		return FALSE;
	}
	FILE* fpDst = NULL;
	if (NULL == (fpDst = CreateOrOpenFile(
		pszFullPath, _T("_merge"), NULL, NULL, NULL, _T(".bin"), _T("wb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FcloseAndNull(fpSrc1);
		FcloseAndNull(fpSrc2);
		return FALSE;
	}
	BYTE buf[2352] = {};
	if (fread(buf, sizeof(BYTE), sizeof(buf), fpSrc2) < sizeof(buf)) {
		OutputErrorString("Failed to read: read size %zu [F:%s][L:%d]\n", sizeof(buf), _T(__FUNCTION__), __LINE__);
		FcloseAndNull(fpSrc1);
		FcloseAndNull(fpSrc2);
		FcloseAndNull(fpDst);
		return FALSE;
	};
	INT nLBA = MSFtoLBA(BcdToDec(buf[12]), BcdToDec(buf[13]), BcdToDec(buf[14])) - 150;
	rewind(fpSrc2);

	for (INT i = 0; i < nLBA; i++) {
		if (fread(buf, sizeof(BYTE), sizeof(buf), fpSrc1) < sizeof(buf)) {
			OutputErrorString("Failed to read: read size %zu [F:%s][L:%d]\n", sizeof(buf), _T(__FUNCTION__), __LINE__);
			FcloseAndNull(fpSrc1);
			FcloseAndNull(fpSrc2);
			FcloseAndNull(fpDst);
			return FALSE;
		}
		fwrite(buf, sizeof(BYTE), sizeof(buf), fpDst);
	}

	if (fread(buf, sizeof(BYTE), sizeof(buf), fpSrc2) < sizeof(buf)) {
		OutputErrorString("Failed to read: read size %zu [F:%s][L:%d]\n", sizeof(buf), _T(__FUNCTION__), __LINE__);
		FcloseAndNull(fpSrc1);
		FcloseAndNull(fpSrc2);
		FcloseAndNull(fpDst);
		return FALSE;
	};
	fseek(fpSrc1, sizeof(buf), SEEK_CUR);
	while (!feof(fpSrc2) && !ferror(fpSrc2)) {
		fwrite(buf, sizeof(BYTE), sizeof(buf), fpDst);
		if (fread(buf, sizeof(BYTE), sizeof(buf), fpSrc2) < sizeof(buf)) {
			OutputErrorString("Failed to read: read size %zu [F:%s][L:%d]\n", sizeof(buf), _T(__FUNCTION__), __LINE__);
			FcloseAndNull(fpSrc1);
			FcloseAndNull(fpSrc2);
			FcloseAndNull(fpDst);
			return FALSE;
		};
		fseek(fpSrc1, sizeof(buf), SEEK_CUR);
	}
	fseek(fpSrc1, -2352, SEEK_CUR);

	if (fread(buf, sizeof(BYTE), sizeof(buf), fpSrc1) < sizeof(buf)) {
		OutputErrorString("Failed to read: read size %zu [F:%s][L:%d]\n", sizeof(buf), _T(__FUNCTION__), __LINE__);
		FcloseAndNull(fpSrc1);
		FcloseAndNull(fpSrc2);
		FcloseAndNull(fpDst);
		return FALSE;
	}
	while (!feof(fpSrc1) && !ferror(fpSrc1)) {
		fwrite(buf, sizeof(BYTE), sizeof(buf), fpDst);
		if (fread(buf, sizeof(BYTE), sizeof(buf), fpSrc1) < sizeof(buf)) {
			OutputErrorString("Failed to read: read size %zu [F:%s][L:%d]\n", sizeof(buf), _T(__FUNCTION__), __LINE__);
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

size_t WriteBufWithCalc(
	PEXT_ARG pExtArg,
	LPBYTE lpBuf,
	size_t writeSize,
	ULONG ulTransferLen,
	FILE* fp,
	PHASH pHash
) {
	size_t stSize = fwrite(lpBuf, sizeof(BYTE), writeSize * ulTransferLen, fp);
	CalcHash(pExtArg, &pHash->pHashChunk[pHash->uiIndex], lpBuf, (UINT)(writeSize * ulTransferLen));

	return stSize;
}

VOID OutputMainChannel(
	LOG_TYPE type,
	LPBYTE lpBuf,
	LPCTSTR szLabel,
	INT nLBA,
	DWORD dwSize
) {
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(type);
#endif
	if (szLabel != NULL) {
		OutputLog(type, OUTPUT_DHYPHEN_PLUS_STR("%s"), szLabel);
	}
	OutputLog(type, OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Main Channel")
		"       +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F\n", nLBA, (UINT)nLBA);

	for (DWORD i = 0; i < dwSize; i += 16) {
		if (16 > dwSize - i) {
			OutputLog(type, "%04lX : ", i);
			for (DWORD j = 0; j < dwSize - i; j++) {
				if (j == 8) {
					OutputLog(type, " ");
				}
				OutputLog(type, "%02X ", lpBuf[i + j]);
			}
		}
		else {
			OutputLog(type,
				"%04lX : %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X   "
				, i, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3], lpBuf[i + 4], lpBuf[i + 5]
				, lpBuf[i + 6], lpBuf[i + 7], lpBuf[i + 8], lpBuf[i + 9], lpBuf[i + 10], lpBuf[i + 11]
				, lpBuf[i + 12], lpBuf[i + 13], lpBuf[i + 14], lpBuf[i + 15]);
			for (INT j = 0; j < 16; j++) {
				INT ch = isprint(lpBuf[i + j]) ? lpBuf[i + j] : '.';
				OutputLog(type, "%c", ch);
			}
		}
		OutputLog(type, "\n");
	}
}

BOOL ConcatenateFile(
	FILE* fpIn,
	FILE* fpImgFull,
	LPCTSTR func,
	LONG line
) {
	BYTE buf[CD_RAW_SECTOR_SIZE] = {};
	do {
		if ((fread(buf, sizeof(BYTE), sizeof(buf), fpIn)) < sizeof(buf)) {
			if (feof(fpIn)) {
				break;
			}
			if (ferror(fpIn)) {
				OutputErrorString("Failed to read: read size %zu [F:%s][L:%ld]\n", sizeof(buf), func, line);
				return FALSE;
			}
		};
		fwrite(buf, sizeof(BYTE), sizeof(buf), fpImgFull);
	} while (1);
	return TRUE;
}

BOOL ConcatenateFromPregapToLeadout(
	PDISC pDisc,
	LPCTSTR pszPath
) {
	FILE* fpImgFull = NULL;
	if (NULL == (fpImgFull = CreateOrOpenFile(pszPath, _T(" (Track all)"), NULL, NULL, NULL, _T(".img"), _T("wb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	_TCHAR appendName1[18] = {};
	if (pDisc->SCSI.toc.LastTrack > 9) {
		_tcsncpy(appendName1, _T(" (Track 01)(-LBA)"), SIZE_OF_ARRAY(appendName1));
	}
	else {
		_tcsncpy(appendName1, _T(" (Track 1)(-LBA)"), SIZE_OF_ARRAY(appendName1));
	}
	OutputString("Creating (Track all).img\n");

	FILE* fpIn = NULL;
	if (NULL == (fpIn = CreateOrOpenFile(pszPath, appendName1, NULL, NULL, NULL, _T(".bin"), _T("rb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (!ConcatenateFile(fpIn, fpImgFull, _T(__FUNCTION__), __LINE__)) {
		FcloseAndNull(fpIn);
		FcloseAndNull(fpImgFull);
		return FALSE;
	}
	FcloseAndNull(fpIn);

	FILE* fpImg = NULL;
	if (NULL == (fpImg = CreateOrOpenFile(pszPath, NULL, NULL, NULL, NULL, _T(".img"), _T("rb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (!ConcatenateFile(fpImg, fpImgFull, _T(__FUNCTION__), __LINE__)) {
		FcloseAndNull(fpImg);
		FcloseAndNull(fpImgFull);
		return FALSE;
	}
	FcloseAndNull(fpImg);

	FILE* fpOut = NULL;
	if (NULL == (fpOut = CreateOrOpenFile(pszPath, _T(" (Track AA)"), NULL, NULL, NULL, _T(".bin"), _T("rb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (!ConcatenateFile(fpOut, fpImgFull, _T(__FUNCTION__), __LINE__)) {
		FcloseAndNull(fpOut);
		FcloseAndNull(fpImgFull);
		return FALSE;
	}
	FcloseAndNull(fpOut);
	FcloseAndNull(fpImgFull);
	return TRUE;
}
