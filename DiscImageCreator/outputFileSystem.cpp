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
#include "get.h"
#include "output.h"
#include "_external/NonStandardFunction.h"

VOID OutputFsBootRecord(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\t                       Boot System Identifier: %.32" CHARWIDTH "s\n"
		"\t                              Boot Identifier: %.32" CHARWIDTH "s\n"
		"\t                              Boot System Use: "
		, &lpBuf[7], &lpBuf[39]);
	for (INT i = 71; i < 2048; i++) {
		OutputVolDescLog("%x", lpBuf[i]);
	}
	OutputVolDescLog("\n");
}

VOID OutputFsVolumeDescriptorFirst(
	PDISC pDisc,
	LPBYTE lpBuf,
	CHAR str32[][32],
	PVOLUME_DESCRIPTOR pVolDesc
) {
	UINT vss = GetSizeOrUintForVolDesc(lpBuf + 80, UINT(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE));
	OutputVolDescLog(
		"\t                            System Identifier: %.32" CHARWIDTH "s\n"
		"\t                            Volume Identifier: %.32" CHARWIDTH "s\n"
		"\t                            Volume Space Size: %u\n"
		, str32[0], str32[1], vss);
	pVolDesc->uiVolumeSpaceSize = vss;
}

VOID OutputFsDirectoryRecord(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	UINT uiExtentPos,
	UINT uiDataLen,
	LPSTR fname,
	PPATH_TABLE_RECORD pPathTblRec,
	UINT uiPathTblIdx
) {
	CHAR str[128]{};
	INT nFileFlag = lpBuf[25];
	if (nFileFlag & 0x01) {
		strncat(str, "Invisible, ", 11);
	}
	else {
		strncat(str, "Visible, ", 9);
	}
	if (nFileFlag & 0x02) {
		strncat(str, "Directory, ", 11);
	}
	else {
		strncat(str, "File, ", 6);
	}
	if (nFileFlag & 0x04) {
		strncat(str, "Associated, ", 12);
	}
	else {
		strncat(str, "No Associated, ", 15);
	}
	if (nFileFlag & 0x08) {
		strncat(str, "Record Format, ", 15);
	}
	else {
		strncat(str, "No Record Format, ", 18);
	}
	if (nFileFlag & 0x10) {
		strncat(str, "Owner/Group ID, ", 16);
	}
	else {
		strncat(str, "No Owner/Group ID, ", 19);
	}
	if (nFileFlag & 0x80) {
		strncat(str, "No Final Directory Record", 25);
	}
	else {
		strncat(str, "Final Directory Record", 22);
	}
	WORD vsn = GetSizeOrWordForVolDesc(lpBuf + 28);
	OutputVolDescLog(
		"\t\t      Length of Directory Record: %u\n"
		"\t\tExtended Attribute Record Length: %u\n"
		"\t\t              Location of Extent: %u\n"
		"\t\t                     Data Length: %u\n"
		"\t\t         Recording Date and Time: %u-%02u-%02u %02u:%02u:%02u %+03d:%02u\n"
		"\t\t                      File Flags: %u (%" CHARWIDTH "s)\n"
		"\t\t                  File Unit Size: %u\n"
		"\t\t             Interleave Gap Size: %u\n"
		"\t\t          Volume Sequence Number: %u\n"
		"\t\t       Length of File Identifier: %u\n"
		"\t\t                 File Identifier: "
		, lpBuf[0], lpBuf[1], uiExtentPos, uiDataLen, lpBuf[18] + 1900, lpBuf[19], lpBuf[20]
		, lpBuf[21], lpBuf[22], lpBuf[23], (CHAR)lpBuf[24] / 4, (CHAR)lpBuf[24] % 4 * 15
		, lpBuf[25], str, lpBuf[26], lpBuf[27], vsn, lpBuf[32]);
	BOOL bSkip = FALSE;
	for (INT n = 0; n < lpBuf[32]; n++) {
#ifndef _WIN32
		if (lpBuf[33 + n] == 0) continue;
#endif
		OutputVolDescLog("%c", lpBuf[33 + n]);
		if (lpBuf[33 + n] == ';' && n == lpBuf[32] - 2) {
			// skip "file revision number"
			bSkip = TRUE;
		}
		if (!bSkip) {
			fname[n] = (CHAR)lpBuf[33 + n];
		}
	}
	OutputVolDescLog("\n");

	CHAR strTmpFull[_MAX_PATH] = {};
	// not upper and current directory
	if (pPathTblRec &&
		!(lpBuf[32] == 1 && fname[0] == 0) &&
		!(lpBuf[32] == 1 && fname[0] == 1)) {
		LPCH pName[8] = {};
		INT fullIdx = 0;
		pName[fullIdx++] = fname;

		for (UINT idx = uiPathTblIdx; idx != 0;) {
			pName[fullIdx++] = &pPathTblRec[idx].szDirName[0];
			idx = pPathTblRec[idx].uiNumOfUpperDir - 1;
		}
		OutputVolDescLog("FullPath: ");
		CHAR strTmp[_MAX_PATH] = {};
		for (INT i = fullIdx; 0 < i; i--) {
			if (pName[i - 1] != 0) {
#ifdef _WIN32
				_snprintf(strTmp, sizeof(strTmp), "\\%s", pName[i - 1]);
#else
				_snprintf(strTmp, sizeof(strTmp), "/%s", pName[i - 1]);
#endif
				OutputVolDescLog("%" CHARWIDTH "s", strTmp);
				strncat(strTmpFull, strTmp, strlen(strTmp));
			}
		}
		OutputVolDescLog("\n\n");
	}
	if (!strncmp(fname, "PARAM.SFO", 9)) {
		pDisc->BD.nLBAForParamSfo = (INT)uiExtentPos;
	}

	CHAR fnameForProtect[MAX_FNAME_FOR_VOLUME] = {};
	if (lpBuf[32] != 1 && lpBuf[33] == 0) {
		// for Joliet
		for (INT n = 0; n < lpBuf[32] / 2; n++) {
			fnameForProtect[n] = fname[n * 2 + 1];
		}
	}
	else {
		strncpy(fnameForProtect, fname, MAX_FNAME_FOR_VOLUME);
	}
	fnameForProtect[MAX_FNAME_FOR_VOLUME - 1] = 0;

	if (pExtArg->byScanProtectViaFile || pExtArg->byIntentionalSub) {
		if (pExtArg->byScanProtectViaFile) {
			if ((nFileFlag & 0x02) == 0 && pDisc->PROTECT.byExist) {
				if (pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] < (INT)uiExtentPos) {
					if (pDisc->PROTECT.byTmpForSafeDisc) {
						pDisc->PROTECT.ERROR_SECTOR.nNextExtentPos = (INT)uiExtentPos;
						pDisc->PROTECT.byTmpForSafeDisc = FALSE;
					}
					if ((INT)uiExtentPos <= pDisc->PROTECT.ERROR_SECTOR.nNextExtentPos) {
						// because the size of 00000001.TMP is 2048
						pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] = (INT)uiExtentPos - pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] - 1;
						pDisc->PROTECT.ERROR_SECTOR.nNextExtentPos = (INT)uiExtentPos;
					}
				}
			}
			if (!strncmp(fnameForProtect, "__CDS.exe", 9)) {
				pDisc->PROTECT.byExist = cds300;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, 9);
			}
			else if (!strncmp(fnameForProtect, "BIG.DAT", 7)) {
				pDisc->PROTECT.byExist = datel;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, 7);
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = (INT)uiExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] = (INT)(uiDataLen / DISC_RAW_READ_SIZE);
			}
			else if (pDisc->PROTECT.byExist == datel && !strncmp(fnameForProtect, "DATA.DAT", 8)) {
				// for "DVD Region X"
				pDisc->PROTECT.byExist = datelAlt;
				strncpy(pDisc->PROTECT.name2, fnameForProtect, 8);
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd = (INT)uiExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize2nd = (INT)(uiDataLen / DISC_RAW_READ_SIZE);
			}
			else if (!strncmp(fnameForProtect, "CD.IDX", 6)) {
				pDisc->PROTECT.byExist = cdidx;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, 6);
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = (INT)uiExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] = (INT)(uiDataLen / DISC_RAW_READ_SIZE);
			}
			else if (!strncmp(fnameForProtect, "LASERLOK.IN", 11)) {
				pDisc->PROTECT.byExist = laserlock;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, 11);
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = (INT)uiExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] = (INT)(uiDataLen / DISC_RAW_READ_SIZE - 1);
			}
			else if (!_strnicmp(fnameForProtect, "PROTECT.PRO", 11)) {
				pDisc->PROTECT.byExist = proring;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, 11);
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = (INT)uiExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] = (INT)(uiDataLen / DISC_RAW_READ_SIZE - 1);
			}
			else if (!strncmp(fnameForProtect, "00000001.LT1", 12)) {
				pDisc->PROTECT.byExist = safeDiscLite;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, 12);
				pDisc->PROTECT.byTmpForSafeDisc = TRUE;
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = (INT)(uiExtentPos + uiDataLen / DISC_RAW_READ_SIZE);
			}
			else if (!strncmp(fnameForProtect, "00000001.TMP", 12) && pDisc->PROTECT.byExist != safeDisc) {
				pDisc->PROTECT.byExist = safeDisc;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, 12);
				pDisc->PROTECT.byTmpForSafeDisc = TRUE;
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = (INT)(uiExtentPos + uiDataLen / DISC_RAW_READ_SIZE);
			}
			else if (!_strnicmp(fnameForProtect, "00002.TMP", 9)) {
				pDisc->PROTECT.byExist = smartE;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, 9);
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = (INT)uiExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] = (INT)(uiDataLen / DISC_RAW_READ_SIZE - 1);
			}
			else if (GetReadErrorFileName(pExtArg, fnameForProtect)) {
				pDisc->PROTECT.byExist = physicalErr;
				strncpy(pDisc->PROTECT.name[pExtArg->FILE.readErrCnt], fnameForProtect, strlen(fnameForProtect));
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[pExtArg->FILE.readErrCnt] = (INT)uiExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize[pExtArg->FILE.readErrCnt] = (INT)(uiDataLen / DISC_RAW_READ_SIZE - 1);
				pExtArg->FILE.readErrCnt++;
			}
			else if (GetC2ErrorFileName(pExtArg, fnameForProtect)) {
				pDisc->PROTECT.byExist = c2Err;
				strncpy(pDisc->PROTECT.name[pExtArg->FILE.c2ErrCnt], fnameForProtect, strlen(fnameForProtect));
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[pExtArg->FILE.c2ErrCnt] = (INT)uiExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize[pExtArg->FILE.c2ErrCnt] = (INT)(uiDataLen / DISC_RAW_READ_SIZE - 1);
				pExtArg->FILE.c2ErrCnt++;
			}
		}

		if (pExtArg->byIntentionalSub) {
			if (!strncmp(fnameForProtect, "CMS16.DLL", 9) && pDisc->PROTECT.byExist == no) {
				pDisc->PROTECT.byExist = securomV1;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, 9);
			}
			else if ((!strncmp(fnameForProtect, "cms32_95.dll", 12) || !strncmp(fnameForProtect, "CMS32_NT.DLL", 12))
				&& pDisc->PROTECT.byExist == no) {
				pDisc->PROTECT.byExist = securomV1;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, 12);
			}
		}

		if (pDisc->PROTECT.byExist == no) {
			// for CodeLock, ProtectCD-VOB, a part of SecuROM
			CHAR szSearchStr[][5] = { ".EXE", ".DLL", ".DAT", ".HDR", ".CAB" };
			for (size_t i = 0; i < sizeof(szSearchStr[0]); i++) {
				LPCH p = strcasestr(fnameForProtect, szSearchStr[i]);
				if (p) {
					if (pDisc->PROTECT.nCntForExe == EXELBA_STORE_SIZE) {
						OutputLog(standardError | fileMainError, "Reached MAX .exe num\n");
						return;
					}
					size_t len = (size_t)(p - fnameForProtect + 4);
					pDisc->PROTECT.pExtentPosForExe[pDisc->PROTECT.nCntForExe] = (INT)uiExtentPos;
					pDisc->PROTECT.pDataLenForExe[pDisc->PROTECT.nCntForExe] = (INT)uiDataLen;
					pDisc->PROTECT.pSectorSizeForExe[pDisc->PROTECT.nCntForExe] = (INT)(uiDataLen / DISC_RAW_READ_SIZE);
					if (uiDataLen % DISC_RAW_READ_SIZE > 0) {
						pDisc->PROTECT.pSectorSizeForExe[pDisc->PROTECT.nCntForExe] += 1;
					}
					strncpy(pDisc->PROTECT.pNameForExe[pDisc->PROTECT.nCntForExe], fnameForProtect, len);
					strncpy(pDisc->PROTECT.pFullNameForExe[pDisc->PROTECT.nCntForExe], strTmpFull, strlen(strTmpFull));
					pDisc->PROTECT.nCntForExe++;
					break;
				}
			}
		}
	}
}

VOID OutputFsVolumeDescriptorSecond(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	CHAR str128[][128],
	CHAR str37[][37],
	BOOL bTCHAR
) {
	WORD vss = GetSizeOrWordForVolDesc(lpBuf + 120);
	WORD vsn = GetSizeOrWordForVolDesc(lpBuf + 124);
	WORD lbs = GetSizeOrWordForVolDesc(lpBuf + 128);
	UINT pts = GetSizeOrUintForVolDesc(lpBuf + 132, UINT(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE));
	UINT lopt = MAKEUINT(MAKEWORD(lpBuf[140], lpBuf[141]),
		MAKEWORD(lpBuf[142], lpBuf[143]));
	if (lopt == 0) {
		lopt = MAKEUINT(MAKEWORD(lpBuf[151], lpBuf[150]),
			MAKEWORD(lpBuf[149], lpBuf[148]));
	}
	// for Codelock
	if (pExtArg->byScanProtectViaFile) {
		if (pDisc->PROTECT.nPrevLBAOfPathTablePos == 0) {
			pDisc->PROTECT.nPrevLBAOfPathTablePos = (INT)lopt - 1;
		}
	}
	UINT loopt = MAKEUINT(MAKEWORD(lpBuf[144], lpBuf[145]),
		MAKEWORD(lpBuf[146], lpBuf[147]));
	if (loopt == 0) {
		loopt = MAKEUINT(MAKEWORD(lpBuf[155], lpBuf[154]),
			MAKEWORD(lpBuf[153], lpBuf[152]));
	}
	OutputVolDescLog(
		"\t                              Volume Set Size: %u\n"
		"\t                       Volume Sequence Number: %u\n"
		"\t                           Logical Block Size: %u\n"
		"\t                              Path Table Size: %u\n"
		"\t         Location of Occurrence of Path Table: %u\n"
		"\tLocation of Optional Occurrence of Path Table: %u\n"
		, vss, vsn, lbs, pts, lopt, loopt);

	UINT uiExtentPos = GetSizeOrUintForVolDesc(lpBuf + 158, UINT(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE));
	UINT uiDataLen = GetSizeOrUintForVolDesc(lpBuf + 166, UINT(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE));
	CHAR fname[64] = {};
	OutputFsDirectoryRecord(pExtArg, pDisc, lpBuf + 156, uiExtentPos, uiDataLen, fname, NULL, 0);
	if (bTCHAR) {
		OutputVolDescLog(
			"\t                        Volume Set Identifier: %.64" CHARWIDTH "s\n"
			"\t                         Publisher Identifier: %.64" CHARWIDTH "s\n"
			"\t                     Data Preparer Identifier: %.64" CHARWIDTH "s\n"
			"\t                       Application Identifier: %.64" CHARWIDTH "s\n"
			"\t                    Copyright File Identifier: %.18" CHARWIDTH "s\n"
			"\t                     Abstract File Identifier: %.18" CHARWIDTH "s\n"
			"\t                Bibliographic File Identifier: %.18" CHARWIDTH "s\n"
			, str128[0], str128[1], str128[2], str128[3], str37[0], str37[1], str37[2]);
	}
	else {
		OutputVolDescLog(
			"\t                        Volume Set Identifier: %.128" CHARWIDTH "s\n"
			"\t                         Publisher Identifier: %.128" CHARWIDTH "s\n"
			"\t                     Data Preparer Identifier: %.128" CHARWIDTH "s\n"
			"\t                       Application Identifier: %.128" CHARWIDTH "s\n"
			"\t                    Copyright File Identifier: %.37" CHARWIDTH "s\n"
			"\t                     Abstract File Identifier: %.37" CHARWIDTH "s\n"
			"\t                Bibliographic File Identifier: %.37" CHARWIDTH "s\n"
			, str128[0], str128[1], str128[2], str128[3], str37[0], str37[1], str37[2]);
	}
	OutputVolDescLog(
		"\t                Volume Creation Date and Time: %.4" CHARWIDTH "s-%.2" CHARWIDTH "s-%.2" CHARWIDTH "s %.2" CHARWIDTH "s:%.2" CHARWIDTH "s:%.2" CHARWIDTH "s.%.2" CHARWIDTH "s %+03d:%02d\n"
		"\t            Volume Modification Date and Time: %.4" CHARWIDTH "s-%.2" CHARWIDTH "s-%.2" CHARWIDTH "s %.2" CHARWIDTH "s:%.2" CHARWIDTH "s:%.2" CHARWIDTH "s.%.2" CHARWIDTH "s %+03d:%02d\n"
		"\t              Volume Expiration Date and Time: %.4" CHARWIDTH "s-%.2" CHARWIDTH "s-%.2" CHARWIDTH "s %.2" CHARWIDTH "s:%.2" CHARWIDTH "s:%.2" CHARWIDTH "s.%.2" CHARWIDTH "s %+03d:%02d\n"
		"\t               Volume Effective Date and Time: %.4" CHARWIDTH "s-%.2" CHARWIDTH "s-%.2" CHARWIDTH "s %.2" CHARWIDTH "s:%.2" CHARWIDTH "s:%.2" CHARWIDTH "s.%.2" CHARWIDTH "s %+03d:%02d\n"
		"\t                       File Structure Version: %u\n"
		"\t                              Application Use: ",
		&lpBuf[813], &lpBuf[817], &lpBuf[819], &lpBuf[821], &lpBuf[823]
		, &lpBuf[825], &lpBuf[827], (CHAR)lpBuf[829] / 4, abs((CHAR)lpBuf[829] % 4 * 15),
		&lpBuf[830], &lpBuf[834], &lpBuf[836], &lpBuf[838], &lpBuf[840]
		, &lpBuf[842], &lpBuf[844], (CHAR)lpBuf[846] / 4, abs((CHAR)lpBuf[846] % 4 * 15),
		&lpBuf[847], &lpBuf[851], &lpBuf[853], &lpBuf[855], &lpBuf[857]
		, &lpBuf[859], &lpBuf[861], (CHAR)lpBuf[863] / 4, abs((CHAR)lpBuf[863] % 4 * 15),
		&lpBuf[864], &lpBuf[868], &lpBuf[870], &lpBuf[872], &lpBuf[874]
		, &lpBuf[876], &lpBuf[878], (CHAR)lpBuf[880] / 4, abs((CHAR)lpBuf[880] % 4 * 15),
		lpBuf[881]);
	if (!strncmp((LPCCH)&lpBuf[883], "MVSNRGFP", 8)) {
		strncpy(pDisc->PROTECT.name[0], (LPCCH)&lpBuf[883], 8);
		pDisc->PROTECT.byExist = ripGuard;
	}
	for (INT i = 883; i <= 1394; i++) {
		OutputVolDescLog("%x", lpBuf[i]);
	}
	OutputVolDescLog("\n");
}

VOID OutputFsVolumeDescriptorForISO9660(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	PVOLUME_DESCRIPTOR pVolDesc
) {
	CHAR str32[2][32] = { {} };
	CHAR str128[4][128] = { {} };
	CHAR str37[3][37] = { {} };
	strncpy(str32[0], (LPCCH)&lpBuf[8], sizeof(str32[0]));
	strncpy(str32[1], (LPCCH)&lpBuf[40], sizeof(str32[1]));
	strncpy(str128[0], (LPCCH)&lpBuf[190], sizeof(str128[0]));
	strncpy(str128[1], (LPCCH)&lpBuf[318], sizeof(str128[1]));
	strncpy(str128[2], (LPCCH)&lpBuf[446], sizeof(str128[2]));
	strncpy(str128[3], (LPCCH)&lpBuf[574], sizeof(str128[3]));
	strncpy(str37[0], (LPCCH)&lpBuf[702], sizeof(str37[0]));
	strncpy(str37[1], (LPCCH)&lpBuf[739], sizeof(str37[1]));
	strncpy(str37[2], (LPCCH)&lpBuf[776], sizeof(str37[2]));
	OutputFsVolumeDescriptorFirst(pDisc, lpBuf, str32, pVolDesc);
	if (lpBuf[0] == 2) {
		OutputVolDescLog(
			"\t                             Escape Sequences: %.32" CHARWIDTH "s\n", &lpBuf[88]);
	}
	OutputFsVolumeDescriptorSecond(pExtArg, pDisc, lpBuf, str128, str37, FALSE);
}

VOID OutputFsVolumeDescriptorForJoliet(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	PVOLUME_DESCRIPTOR pVolDesc
) {
	CHAR str32[2][32] = {};
	CHAR str128[4][128] = {};
	CHAR str37[3][37] = {};
	WCHAR tmp16[3][16] = {};
	WCHAR tmp64[4][64] = {};
	WCHAR tmp18[3][18] = {};
	BOOL bTCHAR = FALSE;
	if (lpBuf[8] == 0 && lpBuf[9] >= 0x20) {
		LittleToBig(tmp16[0], (LPWCH)&lpBuf[8], 16);
		bTCHAR = TRUE;
	}
	else if (lpBuf[8] >= 0x20 && lpBuf[9] == 0) {
		wcsncpy(tmp16[0], (LPWCH)&lpBuf[8], sizeof(tmp16[0]) / sizeof(tmp16[0][0]));
		bTCHAR = TRUE;
	}
	if (bTCHAR) {
		if (!WideCharToMultiByte(CP_ACP, 0,
			(LPCWSTR)&tmp16[0], 16, str32[0], sizeof(str32[0]), NULL, NULL)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		}
	}
	else {
		strncpy(str32[0], (LPCCH)&lpBuf[8], sizeof(str32[0]));
	}

	if (lpBuf[40] == 0 && lpBuf[41] >= 0x20) {
		LittleToBig(tmp16[1], (LPWCH)&lpBuf[40], 16);
	}
	else if (lpBuf[40] >= 0x20 && lpBuf[41] == 0) {
		wcsncpy(tmp16[1], (LPWCH)&lpBuf[40], sizeof(tmp16[1]) / sizeof(tmp16[1][0]));
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp16[1], 16, str32[1], sizeof(str32[1]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	if (lpBuf[190] == 0 && lpBuf[191] >= 0x20) {
		LittleToBig(tmp64[0], (LPWCH)&lpBuf[190], 64);
	}
	else if (lpBuf[190] >= 0x20 && lpBuf[191] == 0) {
		wcsncpy(tmp64[0], (LPWCH)&lpBuf[190], sizeof(tmp64[0]) / sizeof(tmp64[0][0]));
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp64[0], 64, str128[0], sizeof(str128[0]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	if (lpBuf[318] == 0 && lpBuf[319] >= 0x20) {
		LittleToBig(tmp64[1], (LPWCH)&lpBuf[318], 64);
	}
	else if (lpBuf[318] >= 0x20 && lpBuf[319] == 0) {
		wcsncpy(tmp64[1], (LPWCH)&lpBuf[318], sizeof(tmp64[1]) / sizeof(tmp64[1][0]));
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp64[1], 64, str128[1], sizeof(str128[1]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	if (lpBuf[446] == 0 && lpBuf[447] >= 0x20) {
		LittleToBig(tmp64[2], (LPWCH)&lpBuf[446], 64);
	}
	else if (lpBuf[446] >= 0x20 && lpBuf[447] == 0) {
		wcsncpy(tmp64[2], (LPWCH)&lpBuf[446], sizeof(tmp64[2]) / sizeof(tmp64[2][0]));
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp64[2], 64, str128[2], sizeof(str128[2]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	if (lpBuf[574] == 0 && lpBuf[575] >= 0x20) {
		LittleToBig(tmp64[3], (LPWCH)&lpBuf[574], 64);
	}
	else if (lpBuf[574] >= 0x20 && lpBuf[575] == 0) {
		wcsncpy(tmp64[3], (LPWCH)&lpBuf[574], sizeof(tmp64[3]) / sizeof(tmp64[3][0]));
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp64[3], 64, str128[3], sizeof(str128[3]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	if (lpBuf[702] == 0 && lpBuf[703] >= 0x20) {
		LittleToBig(tmp18[0], (LPWCH)&lpBuf[702], 18);
	}
	else if (lpBuf[702] >= 0x20 && lpBuf[703] == 0) {
		wcsncpy(tmp18[0], (LPWCH)&lpBuf[702], sizeof(tmp18[0]) / sizeof(tmp18[0][0]));
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp18[0], 18, str37[0], sizeof(str37[0]) - 1, NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	if (lpBuf[739] == 0 && lpBuf[740] >= 0x20) {
		LittleToBig(tmp18[1], (LPWCH)&lpBuf[739], 18);
	}
	else if (lpBuf[739] >= 0x20 && lpBuf[740] == 0) {
		wcsncpy(tmp18[1], (LPWCH)&lpBuf[739], sizeof(tmp18[1]) / sizeof(tmp18[1][0]));
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp18[1], 18, str37[1], sizeof(str37[1]) - 1, NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	if (lpBuf[776] == 0 && lpBuf[777] >= 0x20) {
		LittleToBig(tmp18[2], (LPWCH)&lpBuf[776], 18);
	}
	else if (lpBuf[776] >= 0x20 && lpBuf[777] == 0) {
		wcsncpy(tmp18[2], (LPWCH)&lpBuf[776], sizeof(tmp18[2]) / sizeof(tmp18[2][0]));
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp18[2], 18, str37[2], sizeof(str37[2]) - 1, NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	OutputFsVolumeDescriptorFirst(pDisc, lpBuf, str32, pVolDesc);

	OutputVolDescLog(
		"\t                             Escape Sequences: %.32" CHARWIDTH "s\n", &lpBuf[88]);
	OutputFsVolumeDescriptorSecond(pExtArg, pDisc, lpBuf, str128, str37, TRUE);
}

VOID OutputFsVolumePartitionDescriptor(
	LPBYTE lpBuf
) {
	CHAR str[2][32] = { {} };
	strncpy(str[0], (LPCCH)&lpBuf[8], sizeof(str[0]));
	strncpy(str[1], (LPCCH)&lpBuf[40], sizeof(str[1]));
	OutputVolDescLog(
		"\t          System Identifier: %.32" CHARWIDTH "s\n"
		"\tVolume Partition Identifier: %.32" CHARWIDTH "s\n"
		"\t  Volume Partition Location: %u\n"
		"\t      Volume Partition Size: %u\n"
		"\t                 System Use: ",
		str[0],
		str[1],
		MAKEUINT(MAKEWORD(lpBuf[76], lpBuf[77]), MAKEWORD(lpBuf[78], lpBuf[79])),
		MAKEUINT(MAKEWORD(lpBuf[84], lpBuf[85]), MAKEWORD(lpBuf[86], lpBuf[87])));
	for (INT i = 88; i < 2048; i++) {
		OutputVolDescLog("%x", lpBuf[i]);
	}
	OutputVolDescLog("\n");
}

VOID OutputFsVolumeDescriptor(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	PVOLUME_DESCRIPTOR pVolDesc,
	INT nLBA
) {
	// 0 is Boot Record.
	// 1 is Primary Volume Descriptor.
	// 2 is Supplementary Volume Descriptor.
	// 3 is Volume Partition Descriptor.
	// 4-254 is reserved.
	// 255 is Volume Descriptor Set Terminator.
	OutputVolDescWithLBALog2("Volume Descriptor",
		"\t                       Volume Descriptor Type: %u\n"
		"\t                          Standard Identifier: %.5" CHARWIDTH "s\n"
		"\t                    Volume Descriptor Version: %u\n"
		, nLBA, lpBuf[0], &lpBuf[1], lpBuf[6]);

	if (lpBuf[0] == 0) {
		OutputFsBootRecord(lpBuf);
	}
	else if (lpBuf[0] == 1) {
		OutputFsVolumeDescriptorForISO9660(pExtArg, pDisc, lpBuf, pVolDesc);
	}
	else if (lpBuf[0] == 2) {
		OutputFsVolumeDescriptorForJoliet(pExtArg, pDisc, lpBuf, pVolDesc);
	}
	else if (lpBuf[0] == 3) {
		OutputFsVolumePartitionDescriptor(lpBuf);
	}
	else if (lpBuf[0] == 255) {
		if (pExtArg->byScanProtectViaFile) {
			pDisc->PROTECT.nNextLBAOfLastVolDesc = nLBA + 1;
		}
	}
}

BOOL OutputFsPathTableRecord(
	LPBYTE lpBuf,
	UINT uiLogicalBlkCoef,
	UINT uiPathTblPos,
	UINT uiPathTblSize,
	BOOL bPathType,
	PPATH_TABLE_RECORD pPathTblRec,
	LPUINT uiDirPosNum
) {
	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Path Table Record"), (INT)uiPathTblPos, (INT)uiPathTblPos);
	for (UINT i = 0; i < uiPathTblSize;) {
		if (*uiDirPosNum > PATH_TABLE_RECORD_SIZE) {
			OutputErrorString("Directory Record is over %d\n", PATH_TABLE_RECORD_SIZE);
			FlushLog();
			return FALSE;
		}
		pPathTblRec[*uiDirPosNum].uiDirNameLen = lpBuf[i];
		if (bPathType == lType) {
			pPathTblRec[*uiDirPosNum].uiPosOfDir = MAKEUINT(MAKEWORD(lpBuf[2 + i], lpBuf[3 + i]),
				MAKEWORD(lpBuf[4 + i], lpBuf[5 + i])) / uiLogicalBlkCoef;
		}
		else {
			pPathTblRec[*uiDirPosNum].uiPosOfDir = MAKEUINT(MAKEWORD(lpBuf[5 + i], lpBuf[4 + i]),
				MAKEWORD(lpBuf[3 + i], lpBuf[2 + i])) / uiLogicalBlkCoef;
		}
		if (pPathTblRec[*uiDirPosNum].uiDirNameLen > 0) {
			if (bPathType == lType) {
				pPathTblRec[*uiDirPosNum].uiNumOfUpperDir = MAKEWORD(lpBuf[6 + i], lpBuf[7 + i]);
			}
			else {
				pPathTblRec[*uiDirPosNum].uiNumOfUpperDir = MAKEWORD(lpBuf[7 + i], lpBuf[6 + i]);
			}
			OutputVolDescLog(
				"\t     Length of Directory Identifier: %u\n"
				"\tLength of Extended Attribute Record: %u\n"
				"\t                 Position of Extent: %u\n"
				"\t          Number of Upper Directory: %u\n"
				"\t               Directory Identifier: "
				, pPathTblRec[*uiDirPosNum].uiDirNameLen, lpBuf[1 + i]
				, pPathTblRec[*uiDirPosNum].uiPosOfDir, pPathTblRec[*uiDirPosNum].uiNumOfUpperDir);
			for (size_t n = 0; n < pPathTblRec[*uiDirPosNum].uiDirNameLen; n++) {
#ifndef _WIN32
				if (lpBuf[8 + i + n] == 0) continue;
#endif
				OutputVolDescLog("%c", lpBuf[8 + i + n]);
				pPathTblRec[*uiDirPosNum].szDirName[n] = (CHAR)lpBuf[8 + i + n];
			}
			OutputVolDescLog("\n\n");

			i += 8 + pPathTblRec[*uiDirPosNum].uiDirNameLen;
			if ((i % 2) != 0) {
				i++;
			}
			*uiDirPosNum = *uiDirPosNum + 1;
		}
		else {
			OutputVolDescLog(
				"\t     Length of Directory Identifier: %u\n", pPathTblRec[*uiDirPosNum].uiDirNameLen);
			break;
		}
	}
	return TRUE;
}

VOID OutputFileAllocationTable(
	LPBYTE lpBuf,
	PFAT fat
) {
	WORD BytsPerSec = MAKEWORD(lpBuf[11], lpBuf[12]);
	fat->SecPerClus = lpBuf[13];
	WORD RsvdSecCnt = MAKEWORD(lpBuf[14], lpBuf[15]);
	BYTE NumFATs = lpBuf[16];
	fat->RootEntCnt = MAKEWORD(lpBuf[17], lpBuf[18]);
	UINT FATSz = MAKEWORD(lpBuf[22], lpBuf[23]);
	UINT TotSec16 = MAKEWORD(lpBuf[19], lpBuf[20]);
	UINT TotSec32 = MAKEUINT(MAKEWORD(lpBuf[32], lpBuf[33]), MAKEWORD(lpBuf[34], lpBuf[35]));

	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR("FileAllocationTable")
		"\t        BS_JmpBoot: %#02x %#02x %#02x\n"
		"\t        BS_OEMName: %.8" CHARWIDTH "s\n"
		"\t    BPB_BytsPerSec: %d\n"
		"\t    BPB_SecPerClus: %d\n"
		"\t    BPB_RsvdSecCnt: %d\n"
		"\t       BPB_NumFATs: %d\n"
		"\t    BPB_RootEntCnt: %d\n"
		"\t      BPB_TotSec16: %d\n"
		"\t         BPB_Media: %#x\n"
		"\t       BPB_FATSz16: %d\n"
		"\t     BPB_SecPerTrk: %d\n"
		"\t      BPB_NumHeads: %d\n"
		"\t       BPB_HiddSec: %d\n"
		"\t      BPB_TotSec32: %d\n"
		, lpBuf[0], lpBuf[1], lpBuf[2]
		, &lpBuf[3]
		, BytsPerSec
		, fat->SecPerClus
		, RsvdSecCnt
		, NumFATs
		, fat->RootEntCnt
		, TotSec16
		, lpBuf[21]
		, FATSz
		, MAKEWORD(lpBuf[24], lpBuf[25])
		, MAKEWORD(lpBuf[26], lpBuf[27])
		, MAKEUINT(MAKEWORD(lpBuf[28], lpBuf[29]), MAKEWORD(lpBuf[30], lpBuf[31]))
		, TotSec32
	);
	INT i = 36;
	if (fat->RootEntCnt == 0 && FATSz == 0) {
		FATSz = MAKEUINT(MAKEWORD(lpBuf[36], lpBuf[37]), MAKEWORD(lpBuf[38], lpBuf[39]));
		OutputVolDescLog(
			"\t       BPB_FATSz32: %d\n"
			"\t      BPB_ExtFlags: %#04x\n"
			"\t         BPB_FSVer: %#04x\n"
			"\t      BPB_RootClus: %d\n"
			"\t        BPB_FSInfo: %d\n"
			"\t     BPB_BkBootSec: %d\n"
			, FATSz
			, MAKEWORD(lpBuf[40], lpBuf[41])
			, MAKEWORD(lpBuf[42], lpBuf[43])
			, MAKEUINT(MAKEWORD(lpBuf[44], lpBuf[45]), MAKEWORD(lpBuf[46], lpBuf[47]))
			, MAKEWORD(lpBuf[48], lpBuf[49])
			, MAKEWORD(lpBuf[50], lpBuf[51])
		);
		i = 64;
	}
	OutputVolDescLog(
		"\t         BS_DrvNum: %#x\n"
		"\t      BS_Reserved1: %d\n"
		"\t        BS_BootSig: %#x\n"
		"\t          BS_VolID: %#08x\n"
		"\t         BS_VolLab: %.11" CHARWIDTH "s\n"
		"\t     BS_FilSysType: %.8" CHARWIDTH "s\n"
		"\t       BS_BootCode: "
		, lpBuf[i]
		, lpBuf[i + 1]
		, lpBuf[i + 2]
		, MAKEUINT(MAKEWORD(lpBuf[i + 3], lpBuf[i + 4]), MAKEWORD(lpBuf[i + 5], lpBuf[i + 6]))
		, &lpBuf[i + 7]
		, &lpBuf[i + 18]
	);
	for (INT j = i + 26; j < 510; j++) {
		OutputVolDescLog("%02x ", lpBuf[j]);
	}
	OutputVolDescLog(
		"\n"
		"\t       BS_BootSign: %#x\n"
		, MAKEWORD(lpBuf[510], lpBuf[511])
	);

	WORD FatStartSector = RsvdSecCnt;
	UINT FatSectorSize = FATSz * NumFATs;
	fat->RootDirStartSector = FatStartSector + FatSectorSize;
	UINT RootDirSectorSize = (UINT)(32 * fat->RootEntCnt + BytsPerSec - 1) / BytsPerSec;
	fat->DataStartSector = fat->RootDirStartSector + RootDirSectorSize;

	UINT TotSec = TotSec16 != 0 ? TotSec16 : TotSec32;
	UINT DataSectorSize = TotSec - fat->DataStartSector;
	UINT CountofClusters = DataSectorSize / fat->SecPerClus;
	OutputVolDescLog(
		"\t    FatStartSector: %d\n"
		"\t     FatSectorSize: %d\n"
		"\tRootDirStartSector: %d\n"
		"\t RootDirSectorSize: %d\n"
		"\t   DataStartSector: %d\n"
		"\t    DataSectorSize: %d\n"
		"\t   CountofClusters: %d "
		, FatStartSector, FatSectorSize
		, fat->RootDirStartSector, RootDirSectorSize
		, fat->DataStartSector, DataSectorSize
		, CountofClusters
	);
	if (CountofClusters <= 4085) {
		OutputVolDescLog(" => FAT12\n");
	}
	else if (4086 <= CountofClusters && CountofClusters <= 65525) {
		OutputVolDescLog(" => FAT16\n");
	}
	else {
		OutputVolDescLog(" => FAT32\n");
	}
}

// https://web.archive.org/web/20090307042249/http://developer.apple.com/documentation/mac/Devices/Devices-121.html
VOID OutputDriveDescriptorRecord(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR("DriverDescriptorRecord")
		"\t                   device signature: %.2" CHARWIDTH "s\n"
		"\t           block size of the device: %d\n"
		"\t     number of blocks on the device: %d\n"
		"\tnumber of driver descriptor entries: %d\n"
		, &lpBuf[0]
		, MAKEWORD(lpBuf[3], lpBuf[2])
		, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], lpBuf[4]))
		, MAKEWORD(lpBuf[17], lpBuf[16])
	);
}

VOID OutputPartitionMap(
	LPBYTE lpBuf,
	LPBOOL bHfs
) {
	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR("PartitionMap")
		"\t              partition signature: %.2" CHARWIDTH "s\n"
		"\tnumber of blocks in partition map: %d\n"
		"\tfirst physical block of partition: %d\n"
		"\t    number of blocks in partition: %d\n"
		"\t                   partition name: %.32" CHARWIDTH "s\n"
		"\t                   partition type: %.32" CHARWIDTH "s\n"
		"\t first logical block of data area: %d\n"
		"\t    number of blocks in data area: %d\n"
		"\t     partition status information: 0x%08x\n"
		"\t first logical block of boot code: %d\n"
		"\t      size of boot code, in bytes: %d\n"
		"\t           boot code load address: %d\n"
		"\t            boot code entry point: %d\n"
		"\t               boot code checksum: %d\n"
		"\t                   processor type: %.16" CHARWIDTH "s\n"
		, &lpBuf[0]
		, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], lpBuf[4]))
		, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], lpBuf[8]))
		, MAKEUINT(MAKEWORD(lpBuf[15], lpBuf[14]), MAKEWORD(lpBuf[13], lpBuf[12]))
		, &lpBuf[16]
		, &lpBuf[48]
		, MAKEUINT(MAKEWORD(lpBuf[83], lpBuf[82]), MAKEWORD(lpBuf[81], lpBuf[80]))
		, MAKEUINT(MAKEWORD(lpBuf[87], lpBuf[86]), MAKEWORD(lpBuf[85], lpBuf[84]))
		, MAKEUINT(MAKEWORD(lpBuf[91], lpBuf[90]), MAKEWORD(lpBuf[89], lpBuf[88]))
		, MAKEUINT(MAKEWORD(lpBuf[95], lpBuf[94]), MAKEWORD(lpBuf[93], lpBuf[92]))
		, MAKEUINT(MAKEWORD(lpBuf[99], lpBuf[98]), MAKEWORD(lpBuf[97], lpBuf[96]))
		, MAKEUINT(MAKEWORD(lpBuf[103], lpBuf[102]), MAKEWORD(lpBuf[101], lpBuf[100]))
		, MAKEUINT(MAKEWORD(lpBuf[111], lpBuf[110]), MAKEWORD(lpBuf[109], lpBuf[108]))
		, MAKEUINT(MAKEWORD(lpBuf[119], lpBuf[118]), MAKEWORD(lpBuf[117], lpBuf[116]))
		, &lpBuf[120]
	);
	if (!strncmp((LPCCH)&lpBuf[48], "Apple_HFS", 9)) {
		*bHfs = TRUE;
	}
}

// https://developer.apple.com/library/archive/documentation/mac/Files/Files-102.html
VOID OutputFsMasterDirectoryBlocks(
	LPBYTE lpBuf,
	INT nLBA
) {
	time_t creationTime = MAKEUINT(MAKEWORD(lpBuf[5], lpBuf[4]), MAKEWORD(lpBuf[3], lpBuf[2]));
	time_t modificationTime = MAKEUINT(MAKEWORD(lpBuf[9], lpBuf[8]), MAKEWORD(lpBuf[7], lpBuf[6]));
	time_t lastTime = MAKELONG(MAKEWORD(lpBuf[66], lpBuf[65]), MAKEWORD(lpBuf[64], lpBuf[63]));

	tm* ctime = gmtime(&creationTime);
	ctime->tm_year -= 66; // HFS starts from 1904, while UNIX starts from 1970
	_TCHAR szBufc[128] = {};
	_tcsftime(szBufc, sizeof(szBufc) / sizeof(szBufc[0]), _T("%Y-%m-%d %H:%M:%S"), ctime);

	tm* mtime = gmtime(&modificationTime);
	mtime->tm_year -= 66;
	_TCHAR szBufm[128] = {};
	_tcsftime(szBufm, sizeof(szBufm) / sizeof(szBufm[0]), _T("%Y-%m-%d %H:%M:%S"), mtime);

	tm* ltime = gmtime(&lastTime);
	ltime->tm_year -= 66;
	_TCHAR szBufl[128] = {};
	if (lastTime) {
		_tcsftime(szBufl, sizeof(szBufl) / sizeof(szBufl[0]), _T("%Y-%m-%d %H:%M:%S"), mtime);
	}

	OutputVolDescWithLBALog2("Master Directory Blocks",
		"\t                       volume signature: %04x\n"
		"\t       date and time of volume creation: %lld (%s)\n"
		"\t     date and time of last modification: %lld (%s)\n"
		"\t                      volume attributes: %04x\n"
		"\t      number of files in root directory: %d\n"
		"\t           first block of volume bitmap: %d\n"
		"\t        start of next allocation search: %d\n"
		"\t  number of allocation blocks in volume: %d\n"
		"\t   size (in bytes) of allocation blocks: %d\n"
		"\t                     default clump size: %d\n"
		"\t       first allocation block in volume: %d\n"
		"\t            next unused catalog node ID: %d\n"
		"\t     number of unused allocation blocks: %d\n"
		"\t                            volume name: %.27" CHARWIDTH "s\n"
		"\t           date and time of last backup: %lld (%s)\n"
		"\t          volume backup sequence number: %d\n"
		"\t                     volume write count: %d\n"
		"\t   clump size for extents overflow file: %d\n"
		"\t            clump size for catalog file: %d\n"
		"\tnumber of directories in root directory: %d\n"
		"\t              number of files in volume: %d\n"
		"\t        number of directories in volume: %d\n"
		, nLBA, MAKEWORD(lpBuf[1], lpBuf[0])
		, creationTime, szBufc
		, modificationTime, szBufm
		, MAKEWORD(lpBuf[11], lpBuf[10])
		, MAKEWORD(lpBuf[13], lpBuf[12])
		, MAKEWORD(lpBuf[15], lpBuf[14])
		, MAKEWORD(lpBuf[17], lpBuf[16])
		, MAKEWORD(lpBuf[19], lpBuf[18])
		, MAKEUINT(MAKEWORD(lpBuf[23], lpBuf[22]), MAKEWORD(lpBuf[21], lpBuf[20]))
		, MAKEUINT(MAKEWORD(lpBuf[27], lpBuf[26]), MAKEWORD(lpBuf[25], lpBuf[24]))
		, MAKEWORD(lpBuf[29], lpBuf[28])
		, MAKEUINT(MAKEWORD(lpBuf[33], lpBuf[32]), MAKEWORD(lpBuf[31], lpBuf[30]))
		, MAKEWORD(lpBuf[35], lpBuf[34])
		, &lpBuf[37]
		, lastTime, szBufl
		, MAKEWORD(lpBuf[69], lpBuf[68])
		, MAKEUINT(MAKEWORD(lpBuf[73], lpBuf[72]), MAKEWORD(lpBuf[71], lpBuf[70]))
		, MAKEUINT(MAKEWORD(lpBuf[77], lpBuf[76]), MAKEWORD(lpBuf[75], lpBuf[74]))
		, MAKEUINT(MAKEWORD(lpBuf[81], lpBuf[80]), MAKEWORD(lpBuf[79], lpBuf[78]))
		, MAKEWORD(lpBuf[83], lpBuf[82])
		, MAKEUINT(MAKEWORD(lpBuf[87], lpBuf[86]), MAKEWORD(lpBuf[85], lpBuf[84]))
		, MAKEUINT(MAKEWORD(lpBuf[91], lpBuf[90]), MAKEWORD(lpBuf[89], lpBuf[88]))
	);
	OutputVolDescLog(
		"\t         information used by the Finder: %d\n"
		"\t         information used by the Finder: %d\n"
		"\t         information used by the Finder: %d\n"
		"\t         information used by the Finder: %d\n"
		"\t         information used by the Finder: %d\n"
		"\t         information used by the Finder: %d\n"
		"\t         information used by the Finder: %d\n"
		"\t         information used by the Finder: %d\n"
		"\t       size (in blocks) of volume cache: %d\n"
		"\tsize (in blocks) of volume bitmap cache: %d\n"
		"\tsize (in blocks) of common volume cache: %d\n"
		"\t          size of extents overflow file: %d\n"
		"\textent record for extents overflow file\n"
		"\t\t               first allocation block: %d\n"
		"\t\t           number of allocation block: %d\n"
		"\t\t               first allocation block: %d\n"
		"\t\t           number of allocation block: %d\n"
		"\t\t               first allocation block: %d\n"
		"\t\t           number of allocation block: %d\n"
		"\t                   size of catalog file: %d\n"
		"\t         extent record for catalog file:\n"
		"\t\t               first allocation block: %d\n"
		"\t\t           number of allocation block: %d\n"
		"\t\t               first allocation block: %d\n"
		"\t\t           number of allocation block: %d\n"
		"\t\t               first allocation block: %d\n"
		"\t\t           number of allocation block: %d\n"
		, MAKEUINT(MAKEWORD(lpBuf[95], lpBuf[94]), MAKEWORD(lpBuf[93], lpBuf[92]))
		, MAKEUINT(MAKEWORD(lpBuf[99], lpBuf[98]), MAKEWORD(lpBuf[97], lpBuf[96]))
		, MAKEUINT(MAKEWORD(lpBuf[103], lpBuf[102]), MAKEWORD(lpBuf[101], lpBuf[100]))
		, MAKEUINT(MAKEWORD(lpBuf[107], lpBuf[106]), MAKEWORD(lpBuf[105], lpBuf[104]))
		, MAKEUINT(MAKEWORD(lpBuf[111], lpBuf[110]), MAKEWORD(lpBuf[109], lpBuf[108]))
		, MAKEUINT(MAKEWORD(lpBuf[115], lpBuf[114]), MAKEWORD(lpBuf[113], lpBuf[112]))
		, MAKEUINT(MAKEWORD(lpBuf[119], lpBuf[118]), MAKEWORD(lpBuf[117], lpBuf[116]))
		, MAKEUINT(MAKEWORD(lpBuf[121], lpBuf[120]), MAKEWORD(lpBuf[121], lpBuf[120]))
		, MAKEWORD(lpBuf[123], lpBuf[122])
		, MAKEWORD(lpBuf[125], lpBuf[124])
		, MAKEWORD(lpBuf[127], lpBuf[126])
		, MAKEUINT(MAKEWORD(lpBuf[131], lpBuf[130]), MAKEWORD(lpBuf[129], lpBuf[128]))
		, MAKEWORD(lpBuf[133], lpBuf[132]), MAKEWORD(lpBuf[135], lpBuf[134])
		, MAKEWORD(lpBuf[137], lpBuf[136]), MAKEWORD(lpBuf[139], lpBuf[138])
		, MAKEWORD(lpBuf[141], lpBuf[140]), MAKEWORD(lpBuf[143], lpBuf[142])
		, MAKEUINT(MAKEWORD(lpBuf[147], lpBuf[146]), MAKEWORD(lpBuf[145], lpBuf[144]))
		, MAKEWORD(lpBuf[149], lpBuf[148]), MAKEWORD(lpBuf[151], lpBuf[150])
		, MAKEWORD(lpBuf[153], lpBuf[152]), MAKEWORD(lpBuf[155], lpBuf[154])
		, MAKEWORD(lpBuf[157], lpBuf[156]), MAKEWORD(lpBuf[159], lpBuf[158])
	);
}

VOID OutputFs3doHeader(
	LPBYTE lpBuf,
	INT nLBA
) {
	OutputVolDescWithLBALog2("3DO Header",
		"\t                Record Type: %#04x\n"
		"\t                 Sync Bytes: %#04x %#04x %#04x %#04x %#04x\n"
		"\t             Record Version: %#04x\n"
		"\t               Volume Flags: %#04x\n"
		"\t             Volume Comment: "
		, nLBA, lpBuf[0], lpBuf[1], lpBuf[2], lpBuf[3]
		, lpBuf[4], lpBuf[5], lpBuf[6], lpBuf[7]);
	for (INT i = 0; i < 32; i++) {
		if (lpBuf[8 + i] != 0) {
			OutputVolDescLog("%c", lpBuf[8 + i]);
		}
	}
	OutputVolDescLog(
		"\n"
		"\t               Volume Label: ");
	for (INT i = 0; i < 32; i++) {
		if (lpBuf[40 + i] != 0) {
			OutputVolDescLog("%c", lpBuf[40 + i]);
		}
	}
	UINT uiNumOfCopy =
		MAKEUINT(MAKEWORD(lpBuf[99], lpBuf[98]), MAKEWORD(lpBuf[97], lpBuf[96]));
	OutputVolDescLog(
		"\n"
		"\t                  Volume ID: %#10x\n"
		"\t         Logical Block Size: %u\n"
		"\t          Volume Space Size: %u + 152\n"
		"\t                Root Dir ID: %#10x\n"
		"\t            Root Dir Blocks: %u\n"
		"\t        Root Dir Block Size: %u\n"
		"\tNum of Pos of Root Dir Copy: %u\n"
		"\t            Pos of Root Dir: %u\n",
		MAKEUINT(MAKEWORD(lpBuf[75], lpBuf[74]), MAKEWORD(lpBuf[73], lpBuf[72])),
		MAKEUINT(MAKEWORD(lpBuf[79], lpBuf[78]), MAKEWORD(lpBuf[77], lpBuf[76])),
		MAKEUINT(MAKEWORD(lpBuf[83], lpBuf[82]), MAKEWORD(lpBuf[81], lpBuf[80])),
		MAKEUINT(MAKEWORD(lpBuf[87], lpBuf[86]), MAKEWORD(lpBuf[85], lpBuf[84])),
		MAKEUINT(MAKEWORD(lpBuf[91], lpBuf[90]), MAKEWORD(lpBuf[89], lpBuf[88])),
		MAKEUINT(MAKEWORD(lpBuf[95], lpBuf[94]), MAKEWORD(lpBuf[93], lpBuf[92])),
		uiNumOfCopy,
		MAKEUINT(MAKEWORD(lpBuf[103], lpBuf[102]), MAKEWORD(lpBuf[101], lpBuf[100])));

	for (UINT i = 0; i < uiNumOfCopy; i++) {
		OutputVolDescLog(
			"\t       Pos of Root Dir Copy: %u\n",
			MAKEUINT(MAKEWORD(lpBuf[107 + i * 4], lpBuf[106 + i * 4]),
				MAKEWORD(lpBuf[105 + i * 4], lpBuf[104 + i * 4])));
	}
}

VOID OutputFs3doDirectoryRecord(
	LPBYTE lpBuf,
	INT nLBA,
	LPCCH pPath,
	UINT uiDirSize
) {
	OutputVolDescWithLBALog2("Directory Record",
		"\tcurrentDir: %" CHARWIDTH "s\n"
		"\t========== Directory Header ==========\n"
		"\t      nextBlock: %#08x\n"
		"\t      prevBlock: %#08x\n"
		"\t          flags: %u\n"
		"\t  directorySize: %u\n"
		"\tdirectoryOffset: %u\n"
		, nLBA, pPath
		, MAKEUINT(MAKEWORD(lpBuf[3], lpBuf[2]), MAKEWORD(lpBuf[1], lpBuf[0]))
		, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], lpBuf[4]))
		, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], lpBuf[8]))
		, uiDirSize
		, MAKEUINT(MAKEWORD(lpBuf[19], lpBuf[18]), MAKEWORD(lpBuf[17], lpBuf[16])));

	UINT cur = THREEDO_DIR_HEADER_SIZE;
	UINT lastCopy = 0;
	CHAR fname[32] = {};
	while (cur < uiDirSize) {
		LPBYTE dirEnt = lpBuf + cur;
		strncpy(fname, (LPCCH)&dirEnt[32], sizeof(fname));
		lastCopy = MAKEUINT(MAKEWORD(dirEnt[67], dirEnt[66]), MAKEWORD(dirEnt[65], dirEnt[64]));
		cur += THREEDO_DIR_ENTRY_SIZE;
		OutputVolDescLog(
			"\t========== Directory Entry ==========\n"
			"\t            flags: %#010x\n"
			"\t               id: %#08x\n"
			"\t              ext: %c%c%c%c\n"
			"\t        blockSize: %u\n"
			"\t entryLengthBytes: %u\n"
			"\tentryLengthBlocks: %u\n"
			"\t            burst: %u\n"
			"\t              gap: %u\n"
			"\t         fileName: %" CHARWIDTH "s\n"
			"\t         copy num: %u\n"
			"\t         data pos: %u\n"
			, MAKEUINT(MAKEWORD(dirEnt[3], dirEnt[2]), MAKEWORD(dirEnt[1], dirEnt[0]))
			, MAKEUINT(MAKEWORD(dirEnt[7], dirEnt[6]), MAKEWORD(dirEnt[5], dirEnt[4]))
			, dirEnt[8], dirEnt[9], dirEnt[10], dirEnt[11]
			, MAKEUINT(MAKEWORD(dirEnt[15], dirEnt[14]), MAKEWORD(dirEnt[13], dirEnt[12]))
			, MAKEUINT(MAKEWORD(dirEnt[19], dirEnt[18]), MAKEWORD(dirEnt[17], dirEnt[16]))
			, MAKEUINT(MAKEWORD(dirEnt[23], dirEnt[22]), MAKEWORD(dirEnt[21], dirEnt[20]))
			, MAKEUINT(MAKEWORD(dirEnt[27], dirEnt[26]), MAKEWORD(dirEnt[25], dirEnt[24]))
			, MAKEUINT(MAKEWORD(dirEnt[31], dirEnt[30]), MAKEWORD(dirEnt[29], dirEnt[28]))
			, fname, lastCopy
			, MAKEUINT(MAKEWORD(dirEnt[71], dirEnt[70]), MAKEWORD(dirEnt[69], dirEnt[68])));
		for (UINT i = 0; i < lastCopy; i++) {
			LPBYTE pCopyPos = lpBuf + cur + sizeof(LONG) * i;
			OutputVolDescLog("\t    data copy pos: %u\n"
				, MAKEUINT(MAKEWORD(pCopyPos[3], pCopyPos[2]), MAKEWORD(pCopyPos[1], pCopyPos[0])));
			cur += 4;
		}
	}
}

VOID OutputFsPceStuff(
	LPBYTE lpBuf,
	INT nLBA
) {
	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("PCE Warning msg & all stuff"), nLBA, nLBA);
	CHAR str[39] = {};
	size_t len = 0;
	for (size_t idx = 0; idx < 805; idx += len) {
		LPCH ptr = (LPCH)&lpBuf[idx];
		if (ptr[0] == -1) {
			len = 1;
			continue;
		}
		len = strlen(ptr) + 1;
		if (len < sizeof(str)) {
			strncpy(str, ptr, len);
			OutputVolDescLog("\t%" CHARWIDTH "s\n", str);
		}
	}
}

VOID OutputFsPceBootSector(
	LPBYTE lpBuf,
	INT nLBA
) {
	CHAR str[24] = {};
	strncpy(str, (LPCCH)&lpBuf[32], sizeof(str));
	CHAR str2[50] = {};
	strncpy(str2, (LPCCH)&lpBuf[56], sizeof(str2));
	CHAR str3[17] = {};
	strncpy(str3, (LPCCH)&lpBuf[106], sizeof(str3) - 1);
	CHAR str4[7] = {};
	strncpy(str4, (LPCCH)&lpBuf[122], sizeof(str4) - 1);
	OutputVolDescWithLBALog2("PCE Boot Sector",
		"\t       load start record no.of CD: %02x:%02x:%02x\n"
		"\t          load block length of CD: %02x\n"
		"\t             program load address: %04x\n"
		"\t          program execute address: %04x\n"
		"\t     ipl set mpr2 (+ max mapping): %02x\n"
		"\t     ipl set mpr3 (+ max mapping): %02x\n"
		"\t     ipl set mpr4 (+ max mapping): %02x\n"
		"\t     ipl set mpr5 (+ max mapping): %02x\n"
		"\t     ipl set mpr6 (+ max mapping): %02x\n"
		"\t                     opening mode: %02x\n"
		"\t  opening graphic data record no.: %02x:%02x:%02x\n"
		"\t      opening graphic data length: %02x\n"
		"\topening graphic data read address: %04x\n"
		"\t    opening ADPCM data record no.: %02x:%02x:%02x\n"
		"\t        opening ADPCM data length: %02x\n"
		"\t      opening ADPCM sampling rate: %02x\n"
		"\t                         reserved: %02x, %02x, %02x, %02x, %02x, %02x, %02x\n"
		"\t                           system: %" CHARWIDTH "s\n"
		"\t                        copyright: %" CHARWIDTH "s\n"
		"\t                     program name: %" CHARWIDTH "s\n"
		"\t                         reserved: %" CHARWIDTH "s\n"
		, nLBA, lpBuf[0], lpBuf[1], lpBuf[2], lpBuf[3], MAKEWORD(lpBuf[4], lpBuf[5]),
		MAKEWORD(lpBuf[6], lpBuf[7]), lpBuf[8], lpBuf[9], lpBuf[10], lpBuf[11], lpBuf[12],
		lpBuf[13], lpBuf[14], lpBuf[15], lpBuf[16], lpBuf[17], MAKEWORD(lpBuf[18], lpBuf[19]),
		lpBuf[20], lpBuf[21], lpBuf[22], lpBuf[23], lpBuf[24],
		lpBuf[25], lpBuf[26], lpBuf[27], lpBuf[28], lpBuf[29], lpBuf[30], lpBuf[31],
		str, str2, str3, str4
	);
}

VOID OutputFsPcfxHeader(
	LPBYTE lpBuf,
	INT nLBA
) {
	OutputVolDescWithLBALog2("PCFX Warning msg", "\t%" CHARWIDTH "s\n", nLBA, &lpBuf[0]);
}

VOID OutputFsPcfxSector(
	LPBYTE lpBuf,
	INT nLBA
) {
	OutputVolDescWithLBALog2("PCFX Title Maker YMD",
		"\t       Title: %" CHARWIDTH "s\n"
		"\tMaker(short): %" CHARWIDTH "s\n"
		"\t Maker(long): %" CHARWIDTH "s\n"
		"\t         YMD: %" CHARWIDTH "s\n"
		, nLBA, &lpBuf[0], &lpBuf[48], &lpBuf[52], &lpBuf[120]
	);
}

// Start of ECMA-167
VOID OutputFsRecordingDateAndTime(
	LPBYTE lpBuf
) {
	WORD sTime = MAKEWORD(lpBuf[0], lpBuf[1]);
	CHAR cTimeZone = (CHAR)(sTime >> 12 & 0x0f);
	OutputVolDescLog("\tRecording Date and Time: ");
	if (cTimeZone == 0) {
		OutputVolDescLog("UTC ");
	}
	else if (cTimeZone == 1) {
		OutputVolDescLog("LocalTime ");
	}
	else if (cTimeZone == 2) {
		OutputVolDescLog("OriginalTime ");
	}
	else {
		OutputVolDescLog("Reserved ");
	}
	SHORT nTime = sTime & 0xfff;
	OutputVolDescLog(
		"%+03d%02d %u-%02u-%02u %02u:%02u:%02u.%02u.%02u.%02u\n",
		nTime / 60, nTime % 60, MAKEWORD(lpBuf[2], lpBuf[3]), lpBuf[4], lpBuf[5],
		lpBuf[6], lpBuf[7], lpBuf[8], lpBuf[9], lpBuf[10], lpBuf[11]);
}

VOID OutputFsRegid(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\t\tFlags: %u\n"
		"\t\tIdentifier: %.23" CHARWIDTH "s\n"
		"\t\tIdentifier Suffix: ",
		lpBuf[0],
		&lpBuf[1]);
	for (INT i = 24; i < 32; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog("\n");
}

VOID OutputFsNSRDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\tStructure Data: ");
	for (INT i = 8; i < 2048; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog("\n");
}

VOID OutputFsBootDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR("Architecture Type"));
	OutputFsRegid(lpBuf + 8);
	OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR("Boot Identifier"));
	OutputFsRegid(lpBuf + 8);

	OutputVolDescLog(
		"\tBoot Extent Location: %u\n"
		"\tBoot Extent Length: %u\n"
		"\tLoad Address: %u%u\n"
		"\tStart Address: %u%u\n",
		MAKEUINT(MAKEWORD(lpBuf[72], lpBuf[73]), MAKEWORD(lpBuf[74], lpBuf[75])),
		MAKEUINT(MAKEWORD(lpBuf[76], lpBuf[77]), MAKEWORD(lpBuf[78], lpBuf[79])),
		MAKEUINT(MAKEWORD(lpBuf[80], lpBuf[81]), MAKEWORD(lpBuf[82], lpBuf[83])),
		MAKEUINT(MAKEWORD(lpBuf[84], lpBuf[85]), MAKEWORD(lpBuf[86], lpBuf[87])),
		MAKEUINT(MAKEWORD(lpBuf[88], lpBuf[89]), MAKEWORD(lpBuf[90], lpBuf[91])),
		MAKEUINT(MAKEWORD(lpBuf[92], lpBuf[93]), MAKEWORD(lpBuf[94], lpBuf[95])));

	OutputFsRecordingDateAndTime(lpBuf + 96);
	OutputVolDescLog(
		"\tFlags: %u\n"
		"\tBoot Use: ",
		MAKEWORD(lpBuf[108], lpBuf[109]));
	for (INT i = 142; i < 2048; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog("\n");
}

VOID OutputFsTerminatingExtendedAreaDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\tStructure Data: ");
	for (INT i = 7; i < 2048; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog("\n");
}

VOID OutputFsBeginningExtendedAreaDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\tStructure Data: ");
	for (INT i = 7; i < 2048; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog("\n");
}

VOID OutputFsVolumeStructureDescriptorFormat(
	LPBYTE lpBuf,
	INT nLBA
) {
	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Volume Recognition Sequence")
		"\tStructure Type: %u\n"
		"\tStandard Identifier: %.5" CHARWIDTH "s\n"
		"\tStructure Version: %u\n"
		, nLBA, nLBA, lpBuf[0], &lpBuf[1], lpBuf[6]);
}

VOID OutputFsVolumeRecognitionSequence(
	LPBYTE lpBuf,
	INT nLBA,
	LPBOOL pUDF
) {
	if (lpBuf[0] == 0 && !strncmp((LPCCH)&lpBuf[1], "BOOT2", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nLBA);
		OutputFsBootDescriptor(lpBuf);
		*pUDF = TRUE;
	}
	else if (lpBuf[0] == 0 && !strncmp((LPCCH)&lpBuf[1], "BEA01", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nLBA);
		OutputFsBeginningExtendedAreaDescriptor(lpBuf);
		*pUDF = TRUE;
	}
	else if (lpBuf[0] == 0 && !strncmp((LPCCH)&lpBuf[1], "NSR02", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nLBA);
		OutputFsNSRDescriptor(lpBuf);
		*pUDF = TRUE;
	}
	else if (lpBuf[0] == 0 && !strncmp((LPCCH)&lpBuf[1], "NSR03", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nLBA);
		OutputFsNSRDescriptor(lpBuf);
		*pUDF = TRUE;
	}
	else if (lpBuf[0] == 0 && !strncmp((LPCCH)&lpBuf[1], "TEA01", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nLBA);
		OutputFsTerminatingExtendedAreaDescriptor(lpBuf);
		*pUDF = TRUE;
	}
}

VOID OutputFsCharspec(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\t\tCharacter Set Type: %u\n"
		"\t\tCharacter Set Information: %.63" CHARWIDTH "s\n",
		lpBuf[0], &lpBuf[1]);
}

VOID OutputFsExtentDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\t\tExtent Length: %u\n"
		"\t\tExtent Location: %u\n",
		MAKEUINT(MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3])),
		MAKEUINT(MAKEWORD(lpBuf[4], lpBuf[5]), MAKEWORD(lpBuf[6], lpBuf[7])));
}

VOID OutputFsRecordedAddress(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\t\tLogical Block Number: %u\n"
		"\t\tPartition Reference Number: %u\n",
		MAKEUINT(MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3])),
		MAKEWORD(lpBuf[4], lpBuf[5])
	);
}

VOID OutputFsShortAllocationDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\t\tExtent Length: %u\n"
		"\t\tFlags: %d\n"
		"\t\tExtent Position: %u\n"
		, MAKEUINT(MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3] & 0x3f))
		, lpBuf[3] & 0xc0
		, MAKEUINT(MAKEWORD(lpBuf[4], lpBuf[5]), MAKEWORD(lpBuf[6], lpBuf[3]))
	);
}

VOID OutputFsLongAllocationDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\t\tExtent Length: %u\n"
		"\t\tFlags: %d\n"
		"\t\tExtent Location\n"
		, MAKEUINT(MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3] & 0x7f))
		, lpBuf[3] & 0x80
	);
	OutputFsRecordedAddress(lpBuf + 4);

	OutputVolDescLog(
		"\t\tImplementation Use\n"
		"\t\t\tADImpUse\n"
		"\t\t\t\tFlags: %d\n"
		"\t\t\t\tUnique Id: %d\n"
		, MAKEWORD(lpBuf[10], lpBuf[11])
		, MAKEUINT(MAKEWORD(lpBuf[12], lpBuf[13]), MAKEWORD(lpBuf[14], lpBuf[15]))
	);
}

VOID OutputFsDescriptorTag(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\tDescriptor Tag\n"
		"\t\tTag Identifier: %u\n"
		"\t\tDescriptor Version: %u\n"
		"\t\tTag Checksum: %u\n"
		"\t\tTag Serial Number: %u\n"
		"\t\tDescriptor CRC: %04x\n"
		"\t\tDescriptor CRC Length: %u\n"
		"\t\tTag Location: %u\n",
		MAKEWORD(lpBuf[0], lpBuf[1]),
		MAKEWORD(lpBuf[2], lpBuf[3]),
		lpBuf[4],
		MAKEWORD(lpBuf[6], lpBuf[7]),
		MAKEWORD(lpBuf[8], lpBuf[9]),
		MAKEWORD(lpBuf[10], lpBuf[11]),
		MAKEUINT(MAKEWORD(lpBuf[12], lpBuf[13]), MAKEWORD(lpBuf[14], lpBuf[15]))
	);
}

VOID OutputFsFileEntry(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\tICB Tag\n"
		"\t\tPrior Recorded Number of Direct Entries: %u\n"
		"\t\tStrategy Type: %u\n"
		"\t\tStrategy Parameter: %u %u\n"
		"\t\tMaximum Number of Entries: %u\n"
		"\t\tFile Type: %u\n"
		"\t\tParent ICB Location\n"
		, MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19]))
		, MAKEWORD(lpBuf[20], lpBuf[21]), lpBuf[22], lpBuf[23]
		, MAKEWORD(lpBuf[24], lpBuf[25]), lpBuf[27]
	);
	OutputFsRecordedAddress(lpBuf + 28);

	OutputVolDescLog(
		"\t\tFlags: %#04x\n"
		"\tUid: %x\n"
		"\tGid: %x\n"
		"\tPermissions: %#04x\n"
		"\tFile Link Count: %u\n"
		"\tRecord Format: %u\n"
		"\tRecord Display Attributes: %u\n"
		"\tRecord Length: %u\n"
		"\tInformation Length: %llu\n"
		"\tLogical Blocks Recorded: %llu\n"
		, MAKEWORD(lpBuf[34], lpBuf[35])
		, MAKEUINT(MAKEWORD(lpBuf[36], lpBuf[37]), MAKEWORD(lpBuf[38], lpBuf[39]))
		, MAKEUINT(MAKEWORD(lpBuf[40], lpBuf[41]), MAKEWORD(lpBuf[42], lpBuf[43]))
		, MAKEUINT(MAKEWORD(lpBuf[44], lpBuf[45]), MAKEWORD(lpBuf[46], lpBuf[47]))
		, MAKEWORD(lpBuf[48], lpBuf[49]), lpBuf[50], lpBuf[51]
		, MAKEUINT(MAKEWORD(lpBuf[52], lpBuf[53]), MAKEWORD(lpBuf[54], lpBuf[55]))
		, MAKEUINT64(MAKEUINT(MAKEWORD(lpBuf[56], lpBuf[57]), MAKEWORD(lpBuf[58], lpBuf[59]))
			, MAKEUINT(MAKEWORD(lpBuf[60], lpBuf[61]), MAKEWORD(lpBuf[62], lpBuf[63])))
		, MAKEUINT64(MAKEUINT(MAKEWORD(lpBuf[64], lpBuf[65]), MAKEWORD(lpBuf[66], lpBuf[67]))
			, MAKEUINT(MAKEWORD(lpBuf[68], lpBuf[69]), MAKEWORD(lpBuf[70], lpBuf[71])))
	);
	OutputFsRecordingDateAndTime(lpBuf + 72);
	OutputFsRecordingDateAndTime(lpBuf + 84);
	OutputFsRecordingDateAndTime(lpBuf + 96);
	OutputVolDescLog(
		"\tCheckpoint: %u\n"
		, MAKEUINT(MAKEWORD(lpBuf[108], lpBuf[109]), MAKEWORD(lpBuf[110], lpBuf[111]))
	);
	OutputVolDescLog("\tExtended Attribute ICB\n");
	OutputFsLongAllocationDescriptor(lpBuf + 112);

	OutputVolDescLog("\tImplementation Identifier\n");
	OutputFsRegid(lpBuf + 128);

	OutputVolDescLog("\tUnique Id: %llu\n"
		, MAKEUINT64(MAKEUINT(MAKEWORD(lpBuf[160], lpBuf[161]), MAKEWORD(lpBuf[162], lpBuf[163]))
			, MAKEUINT(MAKEWORD(lpBuf[164], lpBuf[165]), MAKEWORD(lpBuf[166], lpBuf[167])))
	);
	UINT L_EA = MAKEUINT(MAKEWORD(lpBuf[168], lpBuf[169]), MAKEWORD(lpBuf[170], lpBuf[171]));
	//	UINT L_AD = MAKEUINT(MAKEWORD(lpBuf[172], lpBuf[173]), MAKEWORD(lpBuf[174], lpBuf[175]));

	OutputVolDescLog(
		"\tExtended Attributes\n"
		"\t\tExtended Attribute Header Descriptor\n"
	);
	INT nOfs = 176;
	OutputFsDescriptorTag(lpBuf + nOfs);

	UINT ImplAttrPos = MAKEUINT(MAKEWORD(lpBuf[nOfs + 16], lpBuf[nOfs + 17]), MAKEWORD(lpBuf[nOfs + 18], lpBuf[nOfs + 19]));
	UINT AppAttrPos = MAKEUINT(MAKEWORD(lpBuf[nOfs + 20], lpBuf[nOfs + 21]), MAKEWORD(lpBuf[nOfs + 22], lpBuf[nOfs + 23]));
	OutputVolDescLog(
		"\t\tImplementation Attributes Location: %u\n"
		"\t\tApplication Attributes Location: %u\n"
		, ImplAttrPos, AppAttrPos
	);
	nOfs += 24;

	UINT aType = MAKEUINT(MAKEWORD(lpBuf[nOfs], lpBuf[nOfs + 1])
		, MAKEWORD(lpBuf[nOfs + 2], lpBuf[nOfs + 3]));
	do {
		if (aType == 2048) {
			UINT A_L = MAKEUINT(MAKEWORD(lpBuf[nOfs + 8], lpBuf[nOfs + 9])
				, MAKEWORD(lpBuf[nOfs + 10], lpBuf[nOfs + 11]));
			UINT IU_L = MAKEUINT(MAKEWORD(lpBuf[nOfs + 12], lpBuf[nOfs + 13])
				, MAKEWORD(lpBuf[nOfs + 14], lpBuf[nOfs + 15]));
			OutputVolDescLog(
				"\t\tImplementation Use Extended Attribute\n"
				"\t\t\tAttribute Type: %u\n"
				"\t\t\tAttribute Subtype: %u\n"
				"\t\t\tAttribute Length: %u\n"
				"\t\t\tImplementation Use Length: %u\n"
				"\t\t\tImplementation Identifier\n"
				, aType, lpBuf[nOfs + 4], A_L, IU_L
			);
			OutputFsRegid(lpBuf + nOfs + 16);

			OutputVolDescLog("\t\t\tImplementation Use:");
			for (INT i = nOfs + 48; i < (INT)(nOfs + 48 + IU_L); i++) {
				OutputVolDescLog(" %02x", lpBuf[i]);
			}
			OutputVolDescLog("\n");
			nOfs += 48 + IU_L;
			aType = MAKEUINT(MAKEWORD(lpBuf[nOfs], lpBuf[nOfs + 1])
				, MAKEWORD(lpBuf[nOfs + 2], lpBuf[nOfs + 3]));
		}
	} while (aType == 2048);

	OutputVolDescLog("\tAllocation descriptors\n");
	OutputFsShortAllocationDescriptor(lpBuf + 176 + L_EA);
	//	*lpNextFileLen = MAKEUINT(MAKEWORD(lpBuf[176 + L_EA], lpBuf[176 + L_EA + 1])
	//		, MAKEWORD(lpBuf[176 + L_EA + 2], lpBuf[176 + L_EA + 3] & 0x3f));
	//	*lpNextFilePos = MAKEUINT(MAKEWORD(lpBuf[176 + L_EA + 4] & 0x3f, lpBuf[176 + L_EA + 5])
	//		, MAKEWORD(lpBuf[176 + L_EA + 6], lpBuf[176 + L_EA + 7]));
}

VOID OutputFsFileIdentifierDescriptor(
	LPBYTE lpBuf
) {
	UINT uiOfs = 0;
	//	do {
	BYTE L_FI = lpBuf[19 + uiOfs];
	OutputVolDescLog(
		"\tFile Version Number: %u\n"
		"\tFile Characteristics: %u\n"
		"\tLength of File Identifier: %u\n"
		"\tICB\n"
		, MAKEWORD(lpBuf[16 + uiOfs], lpBuf[17 + uiOfs]), lpBuf[18 + uiOfs], L_FI
	);
	OutputFsLongAllocationDescriptor(lpBuf + 20 + uiOfs);

	WORD L_IU = MAKEWORD(lpBuf[36 + uiOfs], lpBuf[37 + uiOfs]);
	OutputVolDescLog(
		"\tLength of Implementation Use: %u\n"
		"\tImplementation Use:\n"
		, L_IU
	);
	for (UINT i = 38 + uiOfs; i < 38 + L_IU + uiOfs; i++) {
		OutputVolDescLog(" %02x", lpBuf[i]);
	}
	OutputVolDescLog("\tFile Identifier: ");
	for (UINT i = 38 + L_IU + uiOfs; i < 38 + L_IU + L_FI + uiOfs; i++) {
		OutputVolDescLog("%c", lpBuf[i]);
	}
	OutputVolDescLog("\n\n");
	uiOfs += 38 + L_IU + L_FI;
	uiOfs += 4 - (uiOfs % 4);
	// todo: call SPTI for nest
//	} while (uiOfs < uiExtLen);
}

VOID OutputFsFileSetDescriptor(
	LPBYTE lpBuf,
	PUDF pUdf
) {
	OutputFsRecordingDateAndTime(lpBuf + 16);
	OutputVolDescLog(
		"\tInterchange Level: %u\n"
		"\tMaximum Interchange Level: %u\n"
		"\tCharacter Set List: %u\n"
		"\tMaximum Character Set List: %u\n"
		"\tFile Set Number: %u\n"
		"\tFile Set Descriptor Number: %u\n"
		"\tLogical Volume Identifier Character Set\n"
		, MAKEWORD(lpBuf[28], lpBuf[29])
		, MAKEWORD(lpBuf[30], lpBuf[31])
		, MAKEUINT(MAKEWORD(lpBuf[32], lpBuf[33]), MAKEWORD(lpBuf[34], lpBuf[35]))
		, MAKEUINT(MAKEWORD(lpBuf[36], lpBuf[37]), MAKEWORD(lpBuf[38], lpBuf[39]))
		, MAKEUINT(MAKEWORD(lpBuf[40], lpBuf[41]), MAKEWORD(lpBuf[42], lpBuf[43]))
		, MAKEUINT(MAKEWORD(lpBuf[44], lpBuf[45]), MAKEWORD(lpBuf[46], lpBuf[47]))
	);
	OutputFsCharspec(lpBuf + 48);

	OutputVolDescLog(
		"\tLogical Volume Identifier: %.128" CHARWIDTH "s\n"
		"\tFile Set Character Set\n"
		, &lpBuf[112]);
	OutputFsCharspec(lpBuf + 240);

	OutputVolDescLog(
		"\tFile Set Identifier: %.32" CHARWIDTH "s\n"
		"\tCopyright File Identifier: %.32" CHARWIDTH "s\n"
		"\tAbstract File Identifier: %.32" CHARWIDTH "s\n"
		, &lpBuf[304]
		, &lpBuf[336]
		, &lpBuf[368]);

	OutputVolDescLog("\tRoot Directory ICB\n");
	OutputFsLongAllocationDescriptor(lpBuf + 400);
	pUdf->uiFileEntryLen = MAKEUINT(MAKEWORD(lpBuf[400], lpBuf[401]), MAKEWORD(lpBuf[402], lpBuf[403] & 0x7f));
	pUdf->uiFileEntryPos = MAKEUINT(MAKEWORD(lpBuf[404], lpBuf[405]), MAKEWORD(lpBuf[406], lpBuf[407]));

	OutputVolDescLog("\tDomain Identifier\n");
	OutputFsRegid(lpBuf + 416);

	OutputVolDescLog("\tNext Extent\n");
	OutputFsLongAllocationDescriptor(lpBuf + 448);

	OutputVolDescLog("\tSystem Stream Directory ICB\n");
	OutputFsLongAllocationDescriptor(lpBuf + 464);
}

VOID OutputFsLogicalVolumeIntegrityDescriptor(
	LPBYTE lpBuf
) {
	OutputFsRecordingDateAndTime(lpBuf + 16);
	OutputVolDescLog(
		"\tIntegrity Type: %u\n"
		"\tNext Integrity Extent\n"
		, MAKEUINT(MAKEWORD(lpBuf[28], lpBuf[29]), MAKEWORD(lpBuf[30], lpBuf[31])));
	OutputFsExtentDescriptor(lpBuf + 32);

	OutputVolDescLog(
		"\tLogical Volume Header Descriptor\n"
		"\t\tUnique ID: %#lld\n"
		, MAKEUINT64(MAKEUINT(MAKEWORD(lpBuf[40], lpBuf[41]), MAKEWORD(lpBuf[42], lpBuf[43]))
			, MAKEUINT(MAKEWORD(lpBuf[44], lpBuf[45]), MAKEWORD(lpBuf[46], lpBuf[47])))
	);

	UINT N_P =
		MAKEUINT(MAKEWORD(lpBuf[72], lpBuf[73]), MAKEWORD(lpBuf[74], lpBuf[75]));
	UINT L_IU =
		MAKEUINT(MAKEWORD(lpBuf[76], lpBuf[77]), MAKEWORD(lpBuf[78], lpBuf[79]));
	OutputVolDescLog(
		"\tNumber of Partitions: %u\n"
		"\tLength of Implementation Use: %u\n"
		, N_P, L_IU);
	UINT nOfs = N_P * 4;
	if (0 < N_P) {
		OutputVolDescLog("\tFree Space Table: ");
		for (UINT i = 0; i < nOfs; i += 4) {
			OutputVolDescLog("%u, "
				, MAKEUINT(MAKEWORD(lpBuf[80 + i], lpBuf[81 + i]), MAKEWORD(lpBuf[82 + i], lpBuf[83 + i])));
		}
		OutputVolDescLog("\n\tSize Table: ");
		for (UINT i = 80 + nOfs, j = 0; j < nOfs; j += 4) {
			OutputVolDescLog("%u, "
				, MAKEUINT(MAKEWORD(lpBuf[i + j], lpBuf[i + 1 + j]), MAKEWORD(lpBuf[i + 2 + j], lpBuf[i + 3 + j])));
		}
		OutputVolDescLog("\n");
	}
	if (0 < L_IU) {
		nOfs = 80 + N_P * 8;
		OutputVolDescLog(
			"\tImplementation Use\n"
			"\t\tImplementation ID\n"
			"\t\t\tflags: %d\n"
			"\t\t\tid: %.23" CHARWIDTH "s\n"
			"\t\t\tsuffix: %.8" CHARWIDTH "s\n"
			"\t\tNumber of Files: %d\n"
			"\t\tNumber of Directories: %d\n"
			"\t\tMinimum UDF Read Revision: %#04x\n"
			"\t\tMinimum UDF Write Revision: %#04x\n"
			"\t\tMaximum UDF Write Revision: %#04x\n"
			, lpBuf[nOfs], &lpBuf[nOfs + 1], &lpBuf[nOfs + 25]
			, MAKEUINT(MAKEWORD(lpBuf[nOfs + 32], lpBuf[nOfs + 33]), MAKEWORD(lpBuf[nOfs + 34], lpBuf[nOfs + 35]))
			, MAKEUINT(MAKEWORD(lpBuf[nOfs + 36], lpBuf[nOfs + 37]), MAKEWORD(lpBuf[nOfs + 38], lpBuf[nOfs + 39]))
			, MAKEWORD(lpBuf[nOfs + 40], lpBuf[nOfs + 41])
			, MAKEWORD(lpBuf[nOfs + 42], lpBuf[nOfs + 43])
			, MAKEWORD(lpBuf[nOfs + 44], lpBuf[nOfs + 45])
		);
	}
}

VOID OutputFsTerminatingDescriptor(
	LPBYTE lpBuf
) {
	// all reserved byte
	UNREFERENCED_PARAMETER(lpBuf);
}

VOID OutputFsUnallocatedSpaceDescriptor(
	LPBYTE lpBuf
) {
	UINT N_AD =
		MAKEUINT(MAKEWORD(lpBuf[20], lpBuf[21]), MAKEWORD(lpBuf[22], lpBuf[23]));
	OutputVolDescLog(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\tNumber of Allocation Descriptors: %u\n"
		, MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])),
		N_AD);
	if (0 < N_AD) {
		OutputVolDescLog("\tAllocation Descriptors\n");
		for (UINT i = 0; i < N_AD * 8; i += 8) {
			OutputFsExtentDescriptor(lpBuf + 24 + i);
		}
	}
}

VOID OutputFsLogicalVolumeDescriptor(
	LPBYTE lpBuf,
	PUDF pUdf
) {
	OutputVolDescLog(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\tDescriptor Character Set\n",
		MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])));
	OutputFsCharspec(lpBuf + 20);

	OutputVolDescLog(
		"\tLogical Volume Identifier: %.128" CHARWIDTH "s\n"
		"\tLogical Block Size : %u\n"
		"\tDomain Identifier\n",
		&lpBuf[84],
		MAKEUINT(MAKEWORD(lpBuf[212], lpBuf[213]), MAKEWORD(lpBuf[214], lpBuf[215])));
	OutputFsRegid(lpBuf + 216);

	OutputVolDescLog("\tLogical Volume Contents Use\n");
	OutputFsLongAllocationDescriptor(lpBuf + 248);
	pUdf->uiFSDLen = MAKEUINT(MAKEWORD(lpBuf[248], lpBuf[249]), MAKEWORD(lpBuf[250], lpBuf[251] & 0x7f));
	pUdf->uiFSDPos = MAKEUINT(MAKEWORD(lpBuf[252], lpBuf[253]), MAKEWORD(lpBuf[254], lpBuf[255]));

	UINT MT_L = MAKEUINT(MAKEWORD(lpBuf[264], lpBuf[265]), MAKEWORD(lpBuf[266], lpBuf[267]));
	OutputVolDescLog(
		"\tMap Table Length: %u\n"
		"\tNumber of Partition Maps: %u\n"
		"\tImplementation Identifier\n",
		MT_L,
		MAKEUINT(MAKEWORD(lpBuf[268], lpBuf[269]), MAKEWORD(lpBuf[270], lpBuf[271])));
	OutputFsRegid(lpBuf + 272);

	OutputVolDescLog("\tImplementation Use: ");
	for (INT i = 304; i < 432; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog(
		"\n"
		"\tIntegrity Sequence Extent\n");
	OutputFsExtentDescriptor(lpBuf + 432);
	pUdf->uiLogicalVolumeIntegrityLen = MAKEUINT(MAKEWORD(lpBuf[432], lpBuf[433]), MAKEWORD(lpBuf[434], lpBuf[435]));
	pUdf->uiLogicalVolumeIntegrityPos = MAKEUINT(MAKEWORD(lpBuf[436], lpBuf[437]), MAKEWORD(lpBuf[438], lpBuf[439]));

	if (0 < MT_L) {
		if (lpBuf[440] == 1) {
			OutputVolDescLog(
				"\tPartition Maps\n"
				"\t\tPartition Map Type: %d\n"
				"\t\tPartition Map Length: %d\n"
				"\t\tVolume Sequence Number: %d\n"
				"\t\tPartition Number: %d\n"
				, lpBuf[440], lpBuf[441]
				, MAKEWORD(lpBuf[442], lpBuf[443])
				, MAKEWORD(lpBuf[444], lpBuf[445])
			);
			if (6 < MT_L) {
				OutputVolDescLog(
					"\tPartition Maps\n"
					"\t\tPartition Map Type: %d\n"
					"\t\tPartition Map Length: %d\n"
					"\t\tPartition Identifier: "
					, lpBuf[446], lpBuf[447]
				);
				for (INT i = 448; i < 510; i++) {
					OutputVolDescLog("%02x", lpBuf[i]);
				}
				OutputVolDescLog("\n");
			}
		}
		else if (lpBuf[440] == 2) {
			OutputVolDescLog(
				"\tPartition Maps\n"
				"\t\tPartition Map Type: %d\n"
				"\t\tPartition Map Length: %d\n"
				"\t\tPartition Identifier: "
				, lpBuf[440], lpBuf[441]
			);
			for (INT i = 442; i < 504; i++) {
				OutputVolDescLog("%02x", lpBuf[i]);
			}
			OutputVolDescLog("\n");
		}
	}
}

VOID OutputFsPartitionDescriptor(
	LPBYTE lpBuf,
	PUDF pUdf
) {
	OutputVolDescLog(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\tPartition Flags: %u\n"
		"\tPartition Number: %u\n"
		"\tPartition Contents\n",
		MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])),
		MAKEWORD(lpBuf[20], lpBuf[21]),
		MAKEWORD(lpBuf[22], lpBuf[23]));

	OutputFsRegid(lpBuf + 24);

	OutputVolDescLog(
		"\tPartition Contents Use\n"
		"\tPartition Header Descriptor\n"
		"\t\tUnallocated Space Table\n");
	OutputFsShortAllocationDescriptor(lpBuf + 56);

	OutputVolDescLog(
		"\t\tUnallocated Space Bitmap\n");
	OutputFsShortAllocationDescriptor(lpBuf + 64);

	OutputVolDescLog(
		"\t\tPartition Integrity Table\n");
	OutputFsShortAllocationDescriptor(lpBuf + 72);

	OutputVolDescLog(
		"\t\tFreed Space Table\n");
	OutputFsShortAllocationDescriptor(lpBuf + 80);

	OutputVolDescLog(
		"\t\tFreed Space Bitmap\n");
	OutputFsShortAllocationDescriptor(lpBuf + 88);

	pUdf->uiPartitionPos = MAKEUINT(MAKEWORD(lpBuf[188], lpBuf[189]), MAKEWORD(lpBuf[190], lpBuf[191]));
	pUdf->uiPartitionLen = MAKEUINT(MAKEWORD(lpBuf[192], lpBuf[193]), MAKEWORD(lpBuf[194], lpBuf[195]));
	UINT accessType = MAKEUINT(MAKEWORD(lpBuf[184], lpBuf[185]), MAKEWORD(lpBuf[186], lpBuf[187]));
	OutputVolDescLog("\tAccess Type: %u ", accessType);

	switch (accessType) {
	case 0:
		OutputVolDescLog("(Not Specified)\n");
		break;
	case 1:
		OutputVolDescLog("(Read Only)\n");
		break;
	case 2:
		OutputVolDescLog("(Write Once)\n");
		break;
	case 3:
		OutputVolDescLog("(Rewritable)\n");
		break;
	case 4:
		OutputVolDescLog("(Overwritable)\n");
		break;
	default:
		OutputVolDescLog("(Reserved)\n");
		break;
	}
	OutputVolDescLog(
		"\tPartition Starting Location: %u\n"
		"\tPartition Length: %u\n"
		"\tImplementation Identifier\n"
		, pUdf->uiPartitionPos, pUdf->uiPartitionLen
	);

	OutputFsRegid(lpBuf + 196);

	OutputVolDescLog("\tImplementation Use: ");
	for (INT i = 228; i < 356; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog("\n");
}

VOID OutputFsImplementationUseVolumeDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\tImplementation Identifier\n",
		MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])));
	OutputFsRegid(lpBuf + 20);

	INT nOfs = 52;
	OutputVolDescLog(
		"\tLogical Volume Information\n"
		"\t\tLVI Charset\n"
	);
	OutputFsCharspec(lpBuf + nOfs);

	OutputVolDescLog(
		"\t\tLogical Volume Identifier: %.128" CHARWIDTH "s\n"
		"\t\tLV Info 1: %.36" CHARWIDTH "s\n"
		"\t\tLV Info 2: %.36" CHARWIDTH "s\n"
		"\t\tLV Info 3: %.36" CHARWIDTH "s\n"
		"\t\tImplemention ID\n"
		, &lpBuf[nOfs + 64]
		, &lpBuf[nOfs + 192]
		, &lpBuf[nOfs + 228]
		, &lpBuf[nOfs + 264]
	);
	OutputFsRegid(lpBuf + 352);

	OutputVolDescLog("\t\tImplementation Use: ");
	for (INT i = nOfs + 332; i < nOfs + 460; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog("\n");
}

VOID OutputFsVolumeDescriptorPointer(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\tNext Volume Descriptor Sequence Extent\n",
		MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19]))
	);
	OutputFsExtentDescriptor(lpBuf + 20);
}

VOID OutputFsAnchorVolumeDescriptorPointer(
	LPBYTE lpBuf,
	PUDF pUdf
) {
	OutputVolDescLog("\tMain Volume Descriptor Sequence Extent\n");
	OutputFsExtentDescriptor(lpBuf + 16);
	pUdf->uiPVDLen = MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19]));
	pUdf->uiPVDPos = MAKEUINT(MAKEWORD(lpBuf[20], lpBuf[21]), MAKEWORD(lpBuf[22], lpBuf[23]));
	OutputVolDescLog("\tReserve Volume Descriptor Sequence Extent\n");
	OutputFsExtentDescriptor(lpBuf + 24);
}

VOID OutputFsPrimaryVolumeDescriptorForUDF(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\tPrimary Volume Descriptor Number: %u\n"
		"\tVolume Identifier: %.32" CHARWIDTH "s\n"
		"\tVolume Sequence Number: %u\n"
		"\tMaximum Volume Sequence Number: %u\n"
		"\tInterchange Level: %u\n"
		"\tMaximum Interchange Level: %u\n"
		"\tCharacter Set List: %u\n"
		"\tMaximum Character Set List: %u\n"
		"\tVolume Set Identifier: %.128" CHARWIDTH "s\n"
		"\tDescriptor Character Set\n",
		MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])),
		MAKEUINT(MAKEWORD(lpBuf[20], lpBuf[21]), MAKEWORD(lpBuf[22], lpBuf[23])),
		&lpBuf[24],
		MAKEWORD(lpBuf[56], lpBuf[57]),
		MAKEWORD(lpBuf[58], lpBuf[59]),
		MAKEWORD(lpBuf[60], lpBuf[61]),
		MAKEWORD(lpBuf[62], lpBuf[63]),
		MAKEUINT(MAKEWORD(lpBuf[64], lpBuf[65]), MAKEWORD(lpBuf[66], lpBuf[67])),
		MAKEUINT(MAKEWORD(lpBuf[68], lpBuf[69]), MAKEWORD(lpBuf[70], lpBuf[71])),
		&lpBuf[72]);

	OutputFsCharspec(lpBuf + 200);

	OutputVolDescLog("\tExplanatory Character Set\n");
	OutputFsCharspec(lpBuf + 264);

	OutputVolDescLog("\tVolume Abstract\n");
	OutputFsExtentDescriptor(lpBuf + 328);

	OutputVolDescLog("\tVolume Copyright Notice\n");
	OutputFsExtentDescriptor(lpBuf + 336);

	OutputVolDescLog("\tApplication Identifier\n");
	OutputFsRegid(lpBuf + 344);

	OutputFsRecordingDateAndTime(lpBuf + 376);

	OutputVolDescLog("\tImplementation Identifier\n");
	OutputFsRegid(lpBuf + 388);

	OutputVolDescLog("\tImplementation Use: ");
	for (INT i = 420; i < 484; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog(
		"\n"
		"\tPredecessor Volume Descriptor Sequence Location: %u\n"
		"\tFlags: %u\n"
		, MAKEUINT(MAKEWORD(lpBuf[484], lpBuf[485]), MAKEWORD(lpBuf[486], lpBuf[487]))
		, MAKEWORD(lpBuf[488], lpBuf[489]));
}

VOID OutputFsVolumeDescriptorSequence(
	LPBYTE lpBuf,
	INT nLBA,
	PUDF pUDF
) {
	WORD wTagId = MAKEWORD(lpBuf[0], lpBuf[1]);
	if (wTagId == 0 || (10 <= wTagId && wTagId <= 255) || 267 <= wTagId) {
		return;
	}
	switch (wTagId) {
	case 1:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Primary Volume Descriptor"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsPrimaryVolumeDescriptorForUDF(lpBuf);
		break;
	case 2:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Anchor Volume Descriptor Pointer"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsAnchorVolumeDescriptorPointer(lpBuf, pUDF);
		break;
	case 3:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Volume Descriptor Pointer"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsVolumeDescriptorPointer(lpBuf);
		break;
	case 4:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Implementation Use Volume Descriptor"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsImplementationUseVolumeDescriptor(lpBuf);
		break;
	case 5:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Partition Descriptor"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsPartitionDescriptor(lpBuf, pUDF);
		break;
	case 6:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Logical Volume Descriptor"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsLogicalVolumeDescriptor(lpBuf, pUDF);
		break;
	case 7:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Unallocated Space Descriptor"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsUnallocatedSpaceDescriptor(lpBuf);
		break;
	case 8:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Terminating Descriptor"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsTerminatingDescriptor(lpBuf);
		break;
	case 9:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Logical Volume Integrity Descriptor"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsLogicalVolumeIntegrityDescriptor(lpBuf);
		break;
	case 256:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("File Set Descriptor"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsFileSetDescriptor(lpBuf, pUDF);
		break;
	case 257:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("File Identifier Descriptor"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsFileIdentifierDescriptor(lpBuf);
		break;
	case 258:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Allocation Extent Descriptor"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		break;
	case 259:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Indirect Entry"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		break;
	case 260:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Terminal Entry"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		break;
	case 261:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("File Entry"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsFileEntry(lpBuf);
		break;
	case 262:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Extended Attribute Header Descriptor"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		break;
	case 263:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Unallocated Space Entry"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		break;
	case 264:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Space Bitmap Descriptor"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		break;
	case 265:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Partition Integrity Entry"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		break;
	case 266:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Extended File Entry"), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		break;
	default:
		break;
	}
	return;
}
// End of ECMA-167
