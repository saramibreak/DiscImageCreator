/**
 * Copyright 2011-2024 sarami
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
	if (!fname) {
		return;
	}
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
		"\t\t         Recording Date and Time: %d-%02u-%02uT%02u:%02u:%02u%+03d:%02d\n"
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
		LPCH pName[_MAX_FNAME] = {};
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
				if (strlen(strTmpFull) + strlen(strTmp) + 1 <= sizeof(strTmpFull)) {
					strcat(strTmpFull, strTmp);
				}
			}
		}
		OutputVolDescLog("\n\n");
	}
	if (!strncmp(fname, "PS3_DISC.SFB", 12)) {
		pDisc->BD.nLBAForPs3DiscSfb = (INT)uiExtentPos;
		pDisc->BD.nSectorSizeForPs3DiscSfb = 1;
	}
	else if (!strncmp(fname, "PS3UPDAT.PUP", 12)) {
		pDisc->BD.nLBAForPup = (INT)uiExtentPos;
		pDisc->BD.nSectorSizeForPup = 1;
	}
	else if (!strncmp(fname, "PARAM.SFO", 9)) {
		if (pDisc->BD.nParamSfoCnt < MAX_PARAMSFO_NUM) {
			pDisc->BD.nLBAForParamSfo[pDisc->BD.nParamSfoCnt] = (INT)uiExtentPos;
			INT quotient = (INT)(uiDataLen / DISC_MAIN_DATA_SIZE);
			INT mod = (INT)(uiDataLen % DISC_MAIN_DATA_SIZE);
			if (mod != 0) {
				quotient++;
			}
			pDisc->BD.nSectorSizeForParamSfo[pDisc->BD.nParamSfoCnt++] = quotient;
		}
		else {
			OutputVolDescLog("PARAM.SFO is over %d files\n", MAX_PARAMSFO_NUM);
		}
	}

	CHAR fnameForProtect[MAX_FNAME_FOR_VOLUME - 1] = {};
	if (lpBuf[32] != 1 && lpBuf[33] == 0) {
		// for Joliet
		for (INT n = 0; n < lpBuf[32] / 2; n++) {
			fnameForProtect[n] = fname[n * 2 + 1];
		}
	}
	else {
		strncpy(fnameForProtect, fname, sizeof(fnameForProtect) - 1);
	}

	if ((nFileFlag & 0x02) == 0 && (pExtArg->byScanProtectViaFile || pExtArg->byIntentionalSub)) {
		if (pExtArg->byScanProtectViaFile) {
			if (pDisc->PROTECT.byExist) {
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
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, sizeof(pDisc->PROTECT.name[0]));
			}
			else if (!strncmp(fnameForProtect, "BIG.DAT", 7)) {
				pDisc->PROTECT.byExist = datel;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, sizeof(pDisc->PROTECT.name[0]));
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = (INT)uiExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] = (INT)(uiDataLen / DISC_MAIN_DATA_SIZE);
			}
			else if (!strncmp(fnameForProtect, "DUMMY.ZIP", 9) && pDisc->PROTECT.byExist != datel) {
				pDisc->PROTECT.byExist = datel;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, sizeof(pDisc->PROTECT.name[0]));
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = (INT)uiExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] = (INT)(uiDataLen / DISC_MAIN_DATA_SIZE);
			}
			else if (pDisc->PROTECT.byExist == datel && !strncmp(fnameForProtect, "DATA.DAT", 8)) {
				// for "DVD Region X"
				pDisc->PROTECT.byExist = datelAlt;
				strncpy(pDisc->PROTECT.name2, fnameForProtect, sizeof(pDisc->PROTECT.name2));
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd = (INT)uiExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize2nd = (INT)(uiDataLen / DISC_MAIN_DATA_SIZE);
			}
			else if (!strncmp(fnameForProtect, "CD.IDX", 6)) {
				pDisc->PROTECT.byExist = cdidx;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, sizeof(pDisc->PROTECT.name[0]));
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = (INT)uiExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] = (INT)(uiDataLen / DISC_MAIN_DATA_SIZE);
			}
			else if (!_strnicmp(fnameForProtect, "IOSLINK.VXD", 11) || !_strnicmp(fnameForProtect, "IOSLINK.SYS", 11)) {
				pDisc->PROTECT.byExist = discguard;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, sizeof(pDisc->PROTECT.name[0]));
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = 302;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] = 28;
			}
			else if (!strncmp(fnameForProtect, "LASERLOK.IN", 11)) {
				pDisc->PROTECT.byExist = laserlock;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, sizeof(pDisc->PROTECT.name[0]));
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = (INT)uiExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] = (INT)(uiDataLen / DISC_MAIN_DATA_SIZE - 1);
			}
			else if (!_strnicmp(fnameForProtect, "PROTECT.PRO", 11)) {
				pDisc->PROTECT.byExist = proring;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, sizeof(pDisc->PROTECT.name[0]));
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = (INT)uiExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] = (INT)(uiDataLen / DISC_MAIN_DATA_SIZE - 1);
			}
			else if (!strncmp(fnameForProtect, "00000001.LT1", 12)) {
				pDisc->PROTECT.byExist = safeDiscLite;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, sizeof(pDisc->PROTECT.name[0]));
				pDisc->PROTECT.byTmpForSafeDisc = TRUE;
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = (INT)(uiExtentPos + uiDataLen / DISC_MAIN_DATA_SIZE);
			}
			else if (!strncmp(fnameForProtect, "00000001.TMP", 12) && pDisc->PROTECT.byExist != safeDisc) {
				pDisc->PROTECT.byExist = safeDisc;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, sizeof(pDisc->PROTECT.name[0]));
				pDisc->PROTECT.byTmpForSafeDisc = TRUE;
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = (INT)(uiExtentPos + uiDataLen / DISC_MAIN_DATA_SIZE);
			}
			else if (!_strnicmp(fnameForProtect, "00002.TMP", 9)) {
				pDisc->PROTECT.byExist = smartE;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, sizeof(pDisc->PROTECT.name[0]));
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = (INT)uiExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] = (INT)(uiDataLen / DISC_MAIN_DATA_SIZE - 1);
			}
			else if (GetReadErrorFileName(pExtArg, fnameForProtect)) {
				pDisc->PROTECT.byExist = physicalErr;
				if (pExtArg->FILE.readErrCnt < MAX_READ_ERROR_FILE_COUNT) {
					strncpy(pDisc->PROTECT.name[pExtArg->FILE.readErrCnt], fnameForProtect, sizeof(pDisc->PROTECT.name[pExtArg->FILE.readErrCnt]));
					pDisc->PROTECT.ERROR_SECTOR.nExtentPos[pExtArg->FILE.readErrCnt] = (INT)uiExtentPos;
					pDisc->PROTECT.ERROR_SECTOR.nSectorSize[pExtArg->FILE.readErrCnt] = (INT)(uiDataLen / DISC_MAIN_DATA_SIZE - 1);
					pExtArg->FILE.readErrCnt++;
				}
				else {
					OutputErrorString("[ERROR] Read error filename is over %d\n", MAX_READ_ERROR_FILE_COUNT);
				}
			}
			else if (GetC2ErrorFileName(pExtArg, fnameForProtect)) {
				pDisc->PROTECT.byExist = c2Err;
				if (pExtArg->FILE.c2ErrCnt < MAX_READ_ERROR_FILE_COUNT) {
					strncpy(pDisc->PROTECT.name[pExtArg->FILE.c2ErrCnt], fnameForProtect, sizeof(pDisc->PROTECT.name[pExtArg->FILE.c2ErrCnt]));
					pDisc->PROTECT.ERROR_SECTOR.nExtentPos[pExtArg->FILE.c2ErrCnt] = (INT)uiExtentPos;
					pDisc->PROTECT.ERROR_SECTOR.nSectorSize[pExtArg->FILE.c2ErrCnt] = (INT)(uiDataLen / DISC_MAIN_DATA_SIZE - 1);
					pExtArg->FILE.c2ErrCnt++;
				}
				else {
					OutputErrorString("[ERROR] C2 error filename is over %d\n", MAX_READ_ERROR_FILE_COUNT);
				}
			}
		}

		if (pExtArg->byIntentionalSub) {
			if (!strncmp(fnameForProtect, "CMS16.DLL", 9) && pDisc->PROTECT.byExist == no) {
				// Shadow Man (Italian) isn't securomV1
//				pDisc->PROTECT.byExist = securomV1;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, sizeof(pDisc->PROTECT.name[0]));
			}
			else if ((!strncmp(fnameForProtect, "cms32_95.dll", 12) || !strncmp(fnameForProtect, "CMS32_NT.DLL", 12))
				&& pDisc->PROTECT.byExist == no) {
//				pDisc->PROTECT.byExist = securomV1;
				strncpy(pDisc->PROTECT.name[0], fnameForProtect, sizeof(pDisc->PROTECT.name[0]));
			}
		}

		if (pDisc->PROTECT.byExist == no) {
			// for CodeLock, ProtectCD-VOB, a part of SecuROM
			CHAR szSearchStr[][5] = { ".EXE", ".DLL", ".VXD", ".DAT", ".HDR", ".CAB" };
			for (size_t i = 0; i < SIZE_OF_ARRAY(szSearchStr); i++) {
				LPCH p = strcasestr(fnameForProtect, szSearchStr[i]);
				if (p) {
					if (pDisc->PROTECT.nCntForExe == EXELBA_STORE_SIZE) {
						OutputLog(standardError | fileMainError, "Reached MAX .exe num\n");
						return;
					}
//					size_t len = (size_t)(p - fnameForProtect + 4);
					pDisc->PROTECT.pExtentPosForExe[pDisc->PROTECT.nCntForExe] = (INT)uiExtentPos;
					pDisc->PROTECT.pDataLenForExe[pDisc->PROTECT.nCntForExe] = (INT)uiDataLen;
					pDisc->PROTECT.pSectorSizeForExe[pDisc->PROTECT.nCntForExe] = (INT)(uiDataLen / DISC_MAIN_DATA_SIZE);
					if (uiDataLen % DISC_MAIN_DATA_SIZE > 0) {
						pDisc->PROTECT.pSectorSizeForExe[pDisc->PROTECT.nCntForExe] += 1;
					}
					strncpy(pDisc->PROTECT.pNameForExe[pDisc->PROTECT.nCntForExe], fnameForProtect, MAX_FNAME_FOR_VOLUME);
					strncpy(pDisc->PROTECT.pFullNameForExe[pDisc->PROTECT.nCntForExe], strTmpFull, _MAX_PATH);
					pDisc->PROTECT.nCntForExe++;
					break;
				}
			}
		}
	}
}

VOID OutputFsVolumeDescriptorDetail(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	PVOLUME_DESCRIPTOR pVolDesc
) {
	pVolDesc->uiVolumeSpaceSize = GetSizeOrUintForVolDesc(lpBuf + 80, UINT(pDisc->SCSI.nAllLength * DISC_MAIN_DATA_SIZE));

	if (lpBuf[0] == 2) {
		WCHAR lpWBuf[32] = {};
		for (INT i = 0, j = 8; i < 32; i++, j += 2) {
			lpWBuf[i] = (WCHAR)((lpBuf[j] << 8) | (lpBuf[j + 1]));
		}
		OutputVolDescLog(
			"\t                            System Identifier: %.16ls\n"
			"\t                            Volume Identifier: %.16ls\n"
			"\t                            Volume Space Size: %u\n"
			, &lpWBuf[0], &lpWBuf[16], pVolDesc->uiVolumeSpaceSize);
	}
	else {
		OutputVolDescLog(
			"\t                            System Identifier: %.32" CHARWIDTH "s\n"
			"\t                            Volume Identifier: %.32" CHARWIDTH "s\n"
			"\t                            Volume Space Size: %u\n"
			, &lpBuf[8], &lpBuf[40], pVolDesc->uiVolumeSpaceSize);
	}
	if (lpBuf[0] == 2) {
		OutputVolDescLog(
			"\t                             Escape Sequences: %.32" CHARWIDTH "s\n", &lpBuf[88]);
	}

	WORD vss = GetSizeOrWordForVolDesc(lpBuf + 120);
	WORD vsn = GetSizeOrWordForVolDesc(lpBuf + 124);
	WORD lbs = GetSizeOrWordForVolDesc(lpBuf + 128);
	UINT pts = GetSizeOrUintForVolDesc(lpBuf + 132, UINT(pDisc->SCSI.nAllLength * DISC_MAIN_DATA_SIZE));
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

	UINT uiExtentPos = GetSizeOrUintForVolDesc(lpBuf + 158, UINT(pDisc->SCSI.nAllLength * DISC_MAIN_DATA_SIZE));
	UINT uiDataLen = GetSizeOrUintForVolDesc(lpBuf + 166, UINT(pDisc->SCSI.nAllLength * DISC_MAIN_DATA_SIZE));
	CHAR fname[64] = {};
	OutputFsDirectoryRecord(pExtArg, pDisc, lpBuf + 156, uiExtentPos, uiDataLen, fname, NULL, 0);

	if (lpBuf[0] == 2) {
		WCHAR lpWBuf[256] = {};
		for (INT i = 0, j = 190; i < 256; i++, j += 2) {
			lpWBuf[i] = (WCHAR)((lpBuf[j] << 8) | (lpBuf[j + 1]));
		}
		WCHAR lpWBufCopyright[18] = {};
		for (INT i = 0, j = 702; i < 18; i++, j += 2) {
			lpWBufCopyright[i] = (WCHAR)((lpBuf[j] << 8) | (lpBuf[j + 1]));
		}
		WCHAR lpWBufAbstract[18] = {};
		for (INT i = 0, j = 739; i < 18; i++, j += 2) {
			lpWBufAbstract[i] = (WCHAR)((lpBuf[j] << 8) | (lpBuf[j + 1]));
		}
		WCHAR lpWBufBibliographic[18] = {};
		for (INT i = 0, j = 776; i < 18; i++, j += 2) {
			lpWBufBibliographic[i] = (WCHAR)((lpBuf[j] << 8) | (lpBuf[j + 1]));
		}
		OutputVolDescLog(
			"\t                        Volume Set Identifier: %.64ls\n"
			"\t                         Publisher Identifier: %.64ls\n"
			"\t                     Data Preparer Identifier: %.64ls\n"
			"\t                       Application Identifier: %.64ls\n"
			"\t                    Copyright File Identifier: %.18ls\n"
			"\t                     Abstract File Identifier: %.18ls\n"
			"\t                Bibliographic File Identifier: %.18ls\n"
			, &lpWBuf, &lpWBuf[64], &lpWBuf[128], &lpWBuf[192], &lpWBufCopyright, &lpWBufAbstract, &lpWBufBibliographic);
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
			, &lpBuf[190], &lpBuf[318], &lpBuf[446], &lpBuf[574], &lpBuf[702], &lpBuf[739], &lpBuf[776]);
	}
	OutputVolDescLog(
		"\t                Volume Creation Date and Time: %.4" CHARWIDTH "s-%.2" CHARWIDTH "s-%.2" CHARWIDTH "sT%.2" CHARWIDTH "s:%.2" CHARWIDTH "s:%.2" CHARWIDTH "s.%.2" CHARWIDTH "s%+03d:%02d\n"
		"\t            Volume Modification Date and Time: %.4" CHARWIDTH "s-%.2" CHARWIDTH "s-%.2" CHARWIDTH "sT%.2" CHARWIDTH "s:%.2" CHARWIDTH "s:%.2" CHARWIDTH "s.%.2" CHARWIDTH "s%+03d:%02d\n"
		"\t              Volume Expiration Date and Time: %.4" CHARWIDTH "s-%.2" CHARWIDTH "s-%.2" CHARWIDTH "sT%.2" CHARWIDTH "s:%.2" CHARWIDTH "s:%.2" CHARWIDTH "s.%.2" CHARWIDTH "s%+03d:%02d\n"
		"\t               Volume Effective Date and Time: %.4" CHARWIDTH "s-%.2" CHARWIDTH "s-%.2" CHARWIDTH "sT%.2" CHARWIDTH "s:%.2" CHARWIDTH "s:%.2" CHARWIDTH "s.%.2" CHARWIDTH "s%+03d:%02d\n"
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

VOID OutputFsVolumePartitionDescriptor(
	LPBYTE lpBuf
) {
	CHAR str[2][32 + 1] = { {} };
	strncpy(str[0], (LPCCH)&lpBuf[8], sizeof(str[0]) - 1);
	strncpy(str[1], (LPCCH)&lpBuf[40], sizeof(str[1]) - 1);
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
	else if (lpBuf[0] == 1 || lpBuf[0] == 2) {
		OutputFsVolumeDescriptorDetail(pExtArg, pDisc, lpBuf, pVolDesc);
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
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Path Table Record"), (INT)uiPathTblPos, uiPathTblPos);
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

// Microsoft Extensible Firmware Initiative FAT32 File System Specification
// https://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/fatgen103.doc
VOID OutputFsFileAllocationTable(
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
		"\t    BPB_BytsPerSec: %u\n"
		"\t    BPB_SecPerClus: %u\n"
		"\t    BPB_RsvdSecCnt: %u\n"
		"\t       BPB_NumFATs: %u\n"
		"\t    BPB_RootEntCnt: %u\n"
		"\t      BPB_TotSec16: %u\n"
		"\t         BPB_Media: %#x\n"
		"\t       BPB_FATSz16: %u\n"
		"\t     BPB_SecPerTrk: %u\n"
		"\t      BPB_NumHeads: %u\n"
		"\t       BPB_HiddSec: %u\n"
		"\t      BPB_TotSec32: %u\n"
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
			"\t       BPB_FATSz32: %u\n"
			"\t      BPB_ExtFlags: %#04x\n"
			"\t         BPB_FSVer: %#04x\n"
			"\t      BPB_RootClus: %u\n"
			"\t        BPB_FSInfo: %u\n"
			"\t     BPB_BkBootSec: %u\n"
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
		"\t      BS_Reserved1: %u\n"
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
		"\t    FatStartSector: %u\n"
		"\t     FatSectorSize: %u\n"
		"\tRootDirStartSector: %u\n"
		"\t RootDirSectorSize: %u\n"
		"\t   DataStartSector: %u\n"
		"\t    DataSectorSize: %u\n"
		"\t   CountofClusters: %u "
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

VOID OutputFsFATLDirEntry(
	LPBYTE lpBuf,
	LPUINT i,
	_TCHAR* pTab
) {
	WCHAR fnameW[_MAX_FNAME] = {};
	OutputVolDescLog("%s        LDIR_Ord: ", &pTab[0]);
	if ((lpBuf[*i] & 0x40) == 0x40) {
		INT nCnt = (lpBuf[*i] & 0x1f) - 1;
		for (INT h = 0, j = 32 * nCnt, k = 0; h <= nCnt; h++, j -= 32, k += 13) {
			memcpy(fnameW + k, (LPWCH)&lpBuf[1 + *i + j], 10);
			memcpy(fnameW + 5 + k, (LPWCH)&lpBuf[14 + *i + j], 12);
			memcpy(fnameW + 11 + k, (LPWCH)&lpBuf[28 + *i + j], 4);
			OutputVolDescLog("0x%02x ", lpBuf[*i + j]);
		}
		OutputVolDescLog("\n");
		*i += 32 * ((lpBuf[*i] & 0x0f) - 1);
	}
	else {
		OutputVolDescLog("%#02x\n", lpBuf[*i]);
	}
	_TCHAR fname[_MAX_FNAME] = {};
#ifndef UNICODE
	if (!WideCharToMultiByte(CP_ACP, 0,
		fnameW, (INT)wcslen(fnameW), fname, sizeof(fname), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	memcpy(fname, fnameW, sizeof(fname));
#endif
	OutputVolDescLog(
		"%s       LDIR_Name: %s\n"
		"%s       LDIR_Attr: 0x%02x\n"
		"%s       LDIR_Type: 0x%02x\n"
		"%s     LDIR_Chksum: 0x%02x\n"
		"%s  LDIR_FstClusLO: %u\n\n"
		, &pTab[0], fname
		, &pTab[0], lpBuf[11 + *i]
		, &pTab[0], lpBuf[12 + *i]
		, &pTab[0], lpBuf[13 + *i]
		, &pTab[0], lpBuf[26 + *i]
	);
}

VOID OutputFsFATDirEntry(
	LPBYTE lpBuf,
	UINT i,
	_TCHAR* pTab
) {
	_TCHAR fname[_MAX_FNAME] = {};
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0,
		(LPCSTR)&lpBuf[i], (INT)strlen((LPCSTR)&lpBuf[i]), fname, sizeof(fname))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	memcpy(fname, &lpBuf[i], strlen((LPCSTR)&lpBuf[i]));
#endif
	OutputVolDescLog(
		"%s        DIR_Name: %.11s\n"
		"%s        DIR_Attr: 0x%02x\n"
		"%s       DIR_NTRes: %u\n"
		"%sDIR_CrtTimeTenth: %u\n"
		"%s     DIR_CrtTime: %02d:%02d:%02d\n"
		"%s     DIR_CrtDate: %04d-%02d-%02d\n"
		"%s  DIR_LstAccDate: %04d-%02d-%02d\n"
		"%s   DIR_FstClusHI: %u\n"
		"%s     DIR_WrtTime: %02d:%02d:%02d\n"
		"%s     DIR_WrtDate: %04d-%02d-%02d\n"
		"%s   DIR_FstClusLO: %u\n"
		"%s    DIR_FileSize: %u\n\n"
		, &pTab[0], fname
		, &pTab[0], lpBuf[11 + i]
		, &pTab[0], lpBuf[12 + i]
		, &pTab[0], lpBuf[13 + i]
		, &pTab[0], ((lpBuf[15 + i] >> 3) & 0x1f), ((lpBuf[15 + i] << 3) & 0x38) | ((lpBuf[14 + i] >> 5) & 0x07), (lpBuf[14 + i] & 0x1f) / 2
		, &pTab[0], ((lpBuf[17 + i] >> 1) & 0x7f) + 1980, ((lpBuf[17 + i] << 3) & 0x08) | ((lpBuf[16 + i] >> 5) & 0x07), lpBuf[16 + i] & 0x1f
		, &pTab[0], ((lpBuf[19 + i] >> 1) & 0x7f) + 1980, ((lpBuf[19 + i] << 3) & 0x08) | ((lpBuf[18 + i] >> 5) & 0x07), lpBuf[18 + i] & 0x1f
		, &pTab[0], MAKEWORD(lpBuf[20 + i], lpBuf[21 + i])
		, &pTab[0], ((lpBuf[23 + i] >> 3) & 0x1f), ((lpBuf[23 + i] << 3) & 0x38) | ((lpBuf[22 + i] >> 5) & 0x07), (lpBuf[22 + i] & 0x1f) / 2
		, &pTab[0], ((lpBuf[25 + i] >> 1) & 0x7f) + 1980, ((lpBuf[25 + i] << 3) & 0x08) | ((lpBuf[24 + i] >> 5) & 0x07), lpBuf[24 + i] & 0x1f
		, &pTab[0], MAKEWORD(lpBuf[26 + i], lpBuf[27 + i])
		, &pTab[0], MAKEUINT(MAKEWORD(lpBuf[28 + i], lpBuf[29 + i]), MAKEWORD(lpBuf[30 + i], lpBuf[31 + i]))
	);
}

// exFAT
// https://docs.microsoft.com/en-US/windows/win32/fileio/exfat-specification
VOID OutputFsExFAT(
	LPBYTE lpBuf,
	PEXFAT pExFat
) {
	UINT64 PartitionOffset = MAKEUINT64(MAKEUINT(MAKEWORD(lpBuf[64], lpBuf[65]), MAKEWORD(lpBuf[66], lpBuf[67]))
		, MAKEUINT(MAKEWORD(lpBuf[68], lpBuf[69]), MAKEWORD(lpBuf[70], lpBuf[71])));
	pExFat->ClusterHeapOffset = MAKEUINT(MAKEWORD(lpBuf[88], lpBuf[89]), MAKEWORD(lpBuf[90], lpBuf[91]));
	UINT FirstClusterOfRootDirectory = MAKEUINT(MAKEWORD(lpBuf[96], lpBuf[97]), MAKEWORD(lpBuf[98], lpBuf[99]));
	BYTE SectorsPerClusterShift = lpBuf[109];

	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR("exFAT Volume Structure")
		"\t                   JumpBoot: %#02x %#02x %#02x\n"
		"\t             FileSystemName: %.8" CHARWIDTH "s\n"
		"\t            PartitionOffset: %llu\n"
		"\t               VolumeLength: %llu\n"
		"\t                  FatOffset: %u\n"
		"\t                  FatLength: %u\n"
		"\t          ClusterHeapOffset: %u\n"
		"\t               ClusterCount: %u\n"
		"\tFirstClusterOfRootDirectory: %u\n"
		"\t         VolumeSerialNumber: %u\n"
		"\t         FileSystemRevision: %02x\n"
		"\t                VolumeFlags: %02x\n"
		"\t        BytesPerSectorShift: %u\n"
		"\t     SectorsPerClusterShift: %u\n"
		"\t               NumberOfFats: %u\n"
		"\t                DriveSelect: %x\n"
		"\t               PercentInUse: %u\n"
		"\t                   BootCode: "
		, lpBuf[0], lpBuf[1], lpBuf[2]
		, &lpBuf[3]
		, PartitionOffset
		, MAKEUINT64(MAKEUINT(MAKEWORD(lpBuf[72], lpBuf[73]), MAKEWORD(lpBuf[74], lpBuf[75]))
			, MAKEUINT(MAKEWORD(lpBuf[76], lpBuf[77]), MAKEWORD(lpBuf[78], lpBuf[79])))
		, MAKEUINT(MAKEWORD(lpBuf[80], lpBuf[81]), MAKEWORD(lpBuf[82], lpBuf[83]))
		, MAKEUINT(MAKEWORD(lpBuf[84], lpBuf[85]), MAKEWORD(lpBuf[86], lpBuf[87]))
		, pExFat->ClusterHeapOffset
		, MAKEUINT(MAKEWORD(lpBuf[92], lpBuf[93]), MAKEWORD(lpBuf[94], lpBuf[95]))
		, FirstClusterOfRootDirectory
		, MAKEUINT(MAKEWORD(lpBuf[100], lpBuf[101]), MAKEWORD(lpBuf[102], lpBuf[103]))
		, MAKEWORD(lpBuf[104], lpBuf[105])
		, MAKEWORD(lpBuf[106], lpBuf[107])
		, lpBuf[108], SectorsPerClusterShift, lpBuf[110], lpBuf[111], lpBuf[112]
	);
	for (INT i = 120; i < 510; i++) {
		OutputVolDescLog("%02x ", lpBuf[i]);
	}
	OutputVolDescLog("\n\t              BootSignature: %x\n", MAKEWORD(lpBuf[510], lpBuf[511]));

	for (INT i = 0; i < SectorsPerClusterShift; i++) {
		pExFat->SectorsPerClusterShift *= 2;
	}
	pExFat->DirStartSector = GetLBAfromClusterNumber(pExFat, FirstClusterOfRootDirectory);
	OutputVolDescLog(
		"\t         RootDirStartSector: %u (%u + %u * %u)\n"
		"\t    RootDirStartSectorOfRaw: %llu (%u + %llu)\n"
		, pExFat->DirStartSector, pExFat->ClusterHeapOffset, pExFat->SectorsPerClusterShift, FirstClusterOfRootDirectory - 2
		, pExFat->DirStartSector + PartitionOffset, pExFat->DirStartSector, PartitionOffset);
}

VOID OutputFsExFATDirectoryEntry0x81(
	LPBYTE lpBuf,
	INT i,
	_TCHAR* pTab
) {
	OutputVolDescLog(
		"%sAllocation Bitmap Directory Entry\n"
		"%s\t                EntryType: %02x\n"
		"%s\t              BitMapFlags: %02x\n"
		"%s\t             FirstCluster: %u\n"
		"%s\t               DataLength: %llu\n"
		, &pTab[0], &pTab[0], lpBuf[i * 32]
		, &pTab[0], lpBuf[i * 32 + 1]
		, &pTab[0], MAKEUINT(MAKEWORD(lpBuf[i * 32 + 20], lpBuf[i * 32 + 21]), MAKEWORD(lpBuf[i * 32 + 22], lpBuf[i * 32 + 23]))
		, &pTab[0], MAKEUINT64(MAKEUINT(MAKEWORD(lpBuf[i * 32 + 24], lpBuf[i * 32 + 25]), MAKEWORD(lpBuf[i * 32 + 26], lpBuf[i * 32 + 27]))
			, MAKEUINT(MAKEWORD(lpBuf[i * 32 + 28], lpBuf[i * 32 + 29]), MAKEWORD(lpBuf[i * 32 + 30], lpBuf[i * 32 + 31])))
	);
}

VOID OutputFsExFATDirectoryEntry0x82(
	LPBYTE lpBuf,
	INT i,
	_TCHAR* pTab
) {
	OutputVolDescLog(
		"%sUp-case Table Directory Entry\n"
		"%s\t                EntryType: %02x\n"
		"%s\t            TableChecksum: %08x\n"
		"%s\t             FirstCluster: %u\n"
		"%s\t               DataLength: %llu\n"
		, &pTab[0], &pTab[0], lpBuf[i * 32]
		, &pTab[0], MAKEUINT(MAKEWORD(lpBuf[i * 32 + 4], lpBuf[i * 32 + 5]), MAKEWORD(lpBuf[i * 32 + 6], lpBuf[i * 32 + 7]))
		, &pTab[0], MAKEUINT(MAKEWORD(lpBuf[i * 32 + 20], lpBuf[i * 32 + 21]), MAKEWORD(lpBuf[i * 32 + 22], lpBuf[i * 32 + 23]))
		, &pTab[0], MAKEUINT64(MAKEUINT(MAKEWORD(lpBuf[i * 32 + 24], lpBuf[i * 32 + 25]), MAKEWORD(lpBuf[i * 32 + 26], lpBuf[i * 32 + 27]))
			, MAKEUINT(MAKEWORD(lpBuf[i * 32 + 28], lpBuf[i * 32 + 29]), MAKEWORD(lpBuf[i * 32 + 30], lpBuf[i * 32 + 31])))
	);
}

VOID OutputFsExFATDirectoryEntry0x83(
	LPBYTE lpBuf,
	INT i,
	_TCHAR* pTab
) {
	OutputVolDescLog(
		"%sVolume Label Directory Entry\n"
		"%s\t                EntryType: %02x\n"
		"%s\t           CharacterCount: %02x\n"
		"%s\t              VolumeLabel: %.22" CHARWIDTH "s\n"
		, &pTab[0], &pTab[0], lpBuf[i * 32]
		, &pTab[0], lpBuf[i * 32 + 1]
		, &pTab[0], &lpBuf[i * 32 + 2]
	);
}

VOID OutputFsExFATDirectoryEntry0x85(
	LPBYTE lpBuf,
	WORD attr,
	INT i,
	_TCHAR* pTab
) {
	OutputVolDescLog(
		"%sFile Directory Entry\n"
		"%s\t                EntryType: %02x\n"
		"%s\t           SecondaryCount: %02x\n"
		"%s\t              SetCheckSum: %04x\n"
		"%s\t            FileAttribute: %04x ("
		, &pTab[0], &pTab[0], lpBuf[i * 32]
		, &pTab[0], lpBuf[i * 32 + 1]
		, &pTab[0], MAKEWORD(lpBuf[i * 32 + 2], lpBuf[i * 32 + 3])
		, &pTab[0], attr
	);
	if ((attr & 0x01) == 0x01) {
		OutputVolDescLog("ReadOnly");
		if ((attr & 0xfe) != 0) {
			OutputVolDescLog(", ");
		}
	}
	if ((attr & 0x02) == 0x02) {
		OutputVolDescLog("Hidden");
		if ((attr & 0xfc) != 0) {
			OutputVolDescLog(", ");
		}
	}
	if ((attr & 0x04) == 0x04) {
		OutputVolDescLog("System");
		if ((attr & 0xf8) != 0) {
			OutputVolDescLog(", ");
		}
	}
	if ((attr & 0x10) == 0x10) {
		OutputVolDescLog("Directory");
		if ((attr & 0xe0) != 0) {
			OutputVolDescLog(", ");
		}
	}
	if ((attr & 0x20) == 0x20) {
		OutputVolDescLog("Archive");
	}
	OutputVolDescLog(")\n");

	UINT cTime = MAKEUINT(MAKEWORD(lpBuf[i * 32 + 8], lpBuf[i * 32 + 9]), MAKEWORD(lpBuf[i * 32 + 10], lpBuf[i * 32 + 11]));
	UINT lmTime = MAKEUINT(MAKEWORD(lpBuf[i * 32 + 12], lpBuf[i * 32 + 13]), MAKEWORD(lpBuf[i * 32 + 14], lpBuf[i * 32 + 15]));
	UINT laTime = MAKEUINT(MAKEWORD(lpBuf[i * 32 + 16], lpBuf[i * 32 + 17]), MAKEWORD(lpBuf[i * 32 + 18], lpBuf[i * 32 + 19]));
	OutputVolDescLog(
		"%s\t          CreateTimestamp: %4u-%02u-%02uT%02u:%02u:%02u\n"
		"%s\t    LastModifiedTimestamp: %4u-%02u-%02uT%02u:%02u:%02u\n"
		"%s\t    LastAccessedTimestamp: %4u-%02u-%02uT%02u:%02u:%02u\n"
		"%s\t      Create10msIncrement: %d\n"
		"%s\tLastModified10msIncrement: %d\n"
		, &pTab[0], ((cTime >> 25) & 0x7f) + 1980, (cTime >> 21) & 0x0f, (cTime >> 16) & 0x1f, (cTime >> 11) & 0x1f, (cTime >> 5) & 0x3f, (cTime & 0x1f) * 2
		, &pTab[0], ((lmTime >> 25) & 0x7f) + 1980, (lmTime >> 21) & 0x0f, (lmTime >> 16) & 0x1f, (lmTime >> 11) & 0x1f, (lmTime >> 5) & 0x3f, (lmTime & 0x1f) * 2
		, &pTab[0], ((laTime >> 25) & 0x7f) + 1980, (laTime >> 21) & 0x0f, (laTime >> 16) & 0x1f, (laTime >> 11) & 0x1f, (laTime >> 5) & 0x3f, (laTime & 0x1f) * 2
		, &pTab[0], lpBuf[i * 32 + 20] * 10
		, &pTab[0], lpBuf[i * 32 + 21] * 10);

	INT cUtcValid = lpBuf[i * 32 + 22] & 0x80;
	INT cUtc = lpBuf[i * 32 + 22] & 0x7f;
	INT lmUtcValid = lpBuf[i * 32 + 23] & 0x80;
	INT lmUtc = lpBuf[i * 32 + 23] & 0x7f;
	INT laUtcValid = lpBuf[i * 32 + 24] & 0x80;
	INT laUtc = lpBuf[i * 32 + 24] & 0x7f;
	if (cUtcValid == 0x80) {
		if ((cUtc & 0x40) == 0) {
			OutputVolDescLog("%s\t          CreateUtcOffset: +%02d:%02d\n", &pTab[0], cUtc / 4, cUtc % 4 * 15);
		}
		else {
			OutputVolDescLog("%s\t          CreateUtcOffset: -%02d:%02d\n", &pTab[0], (cUtc ^ 0x7e) / 4, (cUtc ^ 0x7e) % 4 * 15);
		}
	}
	if (lmUtcValid == 0x80) {
		if ((lmUtc & 0x40) == 0) {
			OutputVolDescLog("%s\t    LastModifiedUtcOffset: +%02d:%02d\n", &pTab[0], lmUtc / 4, lmUtc % 4 * 15);
		}
		else {
			OutputVolDescLog("%s\t    LastModifiedUtcOffset: -%02d:%02d\n", &pTab[0], (lmUtc ^ 0x7e) / 4, (lmUtc ^ 0x7e) % 4 * 15);
		}
	}
	if (laUtcValid == 0x80) {
		if ((laUtc & 0x40) == 0) {
			OutputVolDescLog("%s\t    LastAccessedUtcOffset: +%02d:%02d\n", &pTab[0], laUtc / 4, laUtc % 4 * 15);
		}
		else {
			OutputVolDescLog("%s\t    LastAccessedUtcOffset: -%02d:%02d\n", &pTab[0], (laUtc ^ 0x7e) / 4, (laUtc ^ 0x7e) % 4 * 15);
		}
	}
}

VOID OutputFsExFATDirectoryEntry0xa0(
	LPBYTE lpBuf,
	INT i,
	_TCHAR* pTab
) {
	OutputVolDescLog(
		"%sVolume GUID Directory Entry\n"
		"%s\t                EntryType: %02x\n"
		"%s\t           SecondaryCount: %02x\n"
		"%s\t              SetCheckSum: %04x\n"
		"%s\t      GeneralPrimaryFlags: %04x\n"
		"%s\t               VolumeGuid: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"
		, &pTab[0], &pTab[0], lpBuf[i * 32]
		, &pTab[0], lpBuf[i * 32 + 1]
		, &pTab[0], MAKEWORD(lpBuf[i * 32 + 2], lpBuf[i * 32 + 3])
		, &pTab[0], MAKEWORD(lpBuf[i * 32 + 4], lpBuf[i * 32 + 5])
		, &pTab[0], lpBuf[i * 32 + 6], lpBuf[i * 32 + 7], lpBuf[i * 32 + 8], lpBuf[i * 32 + 9]
		, lpBuf[i * 32 + 10], lpBuf[i * 32 + 11], lpBuf[i * 32 + 12], lpBuf[i * 32 + 13]
		, lpBuf[i * 32 + 14], lpBuf[i * 32 + 15], lpBuf[i * 32 + 16], lpBuf[i * 32 + 17]
		, lpBuf[i * 32 + 18], lpBuf[i * 32 + 19], lpBuf[i * 32 + 20], lpBuf[i * 32 + 21]
	);
}

VOID OutputFsExFATDirectoryEntry0xc0(
	LPBYTE lpBuf,
	INT NameLength,
	UINT FirstCluster,
	INT i,
	_TCHAR* pTab
) {
	OutputVolDescLog(
		"%sStream Extension Directory Entry\n"
		"%s\t                EntryType: %02x\n"
		"%s\t    GeneralSecondaryFlags: %02x\n"
		"%s\t               NameLength: %d\n"
		"%s\t                 NameHash: %04x\n"
		"%s\t          ValidDataLength: %llu\n"
		"%s\t             FirstCluster: %u\n"
		"%s\t               DataLength: %llu\n"
		, &pTab[0], &pTab[0], lpBuf[i * 32]
		, &pTab[0], lpBuf[i * 32 + 1]
		, &pTab[0], NameLength
		, &pTab[0], MAKEWORD(lpBuf[i * 32 + 4], lpBuf[i * 32 + 5])
		, &pTab[0], MAKEUINT64(MAKEUINT(MAKEWORD(lpBuf[i * 32 + 8], lpBuf[i * 32 + 9]), MAKEWORD(lpBuf[i * 32 + 10], lpBuf[i * 32 + 11]))
			, MAKEUINT(MAKEWORD(lpBuf[i * 32 + 12], lpBuf[i * 32 + 13]), MAKEWORD(lpBuf[i * 32 + 14], lpBuf[i * 32 + 15])))
		, &pTab[0], FirstCluster
		, &pTab[0], MAKEUINT64(MAKEUINT(MAKEWORD(lpBuf[i * 32 + 24], lpBuf[i * 32 + 25]), MAKEWORD(lpBuf[i * 32 + 26], lpBuf[i * 32 + 27]))
			, MAKEUINT(MAKEWORD(lpBuf[i * 32 + 28], lpBuf[i * 32 + 29]), MAKEWORD(lpBuf[i * 32 + 30], lpBuf[i * 32 + 31])))
	);
}

VOID OutputFsExFATDirectoryEntry0xc1(
	LPBYTE lpBuf,
	LPBOOL bName1st,
	INT i,
	_TCHAR* pTab
) {
	setlocale(LC_ALL, ".UTF8");
	if (*bName1st) {
		OutputVolDescLog(
			"%sFile Name Directory Entry\n"
			"%s\t                EntryType: %02x\n"
			"%s\t    GeneralSecondaryFlags: %02x\n"
			"%s\t                 FileName: %.15ls"
			, &pTab[0], &pTab[0], lpBuf[i * 32]
			, &pTab[0], lpBuf[i * 32 + 1]
			, &pTab[0], (LPCWSTR)&lpBuf[i * 32 + 2]
		);
		*bName1st = FALSE;
	}
	else {
		OutputVolDescLog("%ls", (LPCWSTR)&lpBuf[i * 32 + 2]);
	}
	setlocale(LC_ALL, "");
}

// https://web.archive.org/web/20090307042249/http://developer.apple.com/documentation/mac/Devices/Devices-121.html
VOID OutputFsDriveDescriptorRecord(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR("DriverDescriptorRecord")
		"\t                   device signature: %.2" CHARWIDTH "s\n"
		"\t           block size of the device: %u\n"
		"\t     number of blocks on the device: %u\n"
		"\tnumber of driver descriptor entries: %u\n"
		, &lpBuf[0]
		, MAKEWORD(lpBuf[3], lpBuf[2])
		, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], lpBuf[4]))
		, MAKEWORD(lpBuf[17], lpBuf[16])
	);
}

VOID OutputFsPartitionMap(
	LPBYTE lpBuf,
	LPBOOL bHfs
) {
	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR("PartitionMap")
		"\t              partition signature: %.2" CHARWIDTH "s\n"
		"\tnumber of blocks in partition map: %u\n"
		"\tfirst physical block of partition: %u\n"
		"\t    number of blocks in partition: %u\n"
		"\t                   partition name: %.32" CHARWIDTH "s\n"
		"\t                   partition type: %.32" CHARWIDTH "s\n"
		"\t first logical block of data area: %u\n"
		"\t    number of blocks in data area: %u\n"
		"\t     partition status information: 0x%08x\n"
		"\t first logical block of boot code: %u\n"
		"\t      size of boot code, in bytes: %u\n"
		"\t           boot code load address: %u\n"
		"\t            boot code entry point: %u\n"
		"\t               boot code checksum: %u\n"
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

VOID GetMacTime(
	LPBYTE lpBuf,
	time_t* lpuiTime,
	PCHAR lpBufTime,
	size_t lpBufTimeSize
) {
	*lpuiTime = MAKEUINT(MAKEWORD(lpBuf[3], lpBuf[2]), MAKEWORD(lpBuf[1], lpBuf[0]));

	// HFS starts from 1904, while UNIX starts from 1970
	// 365 days * 49 years + 366 days * 17 years
	time_t t = *lpuiTime - 2082844800;
	if (t > 0) {
		tm* tmTime = gmtime(&t);
		_tcsftime(lpBufTime, lpBufTimeSize, _T("%FT%T"), tmTime);
	}
}

VOID OutputFsExtentDataRecord(
	LPBYTE lpBuf
) {
	for (INT i = 0; i < 12; i += 4) {
		OutputVolDescLog(
			"\t\t               first allocation block: %u\n"
			"\t\t           number of allocation block: %u\n"
			, MAKEWORD(lpBuf[1 + i], lpBuf[0 + i])
			, MAKEWORD(lpBuf[3 + i], lpBuf[2 + i])
		);
	}
}

// https://developer.apple.com/library/archive/documentation/mac/Files/Files-102.html
VOID OutputFsMasterDirectoryBlocks(
	LPBYTE lpBuf,
	INT nLBA,
	LPUINT lpuiCatalogFileSize
) {
	time_t creationTime = 0;
	_TCHAR szBufc[128] = {};
	GetMacTime(&lpBuf[2], &creationTime, szBufc, 128);

	time_t modificationTime = 0;
	_TCHAR szBufm[128] = {};
	GetMacTime(&lpBuf[6], &modificationTime, szBufm, 128);

	time_t lastTime = 0;
	_TCHAR szBufl[128] = {};
	GetMacTime(&lpBuf[63], &lastTime, szBufl, 128);

	*lpuiCatalogFileSize = MAKEUINT(MAKEWORD(lpBuf[81], lpBuf[80]), MAKEWORD(lpBuf[79], lpBuf[78]));
	OutputVolDescWithLBALog2("Master Directory Blocks",
		"\t                       volume signature: %04x\n"
		"\t       date and time of volume creation: %llu (%s)\n"
		"\t     date and time of last modification: %llu (%s)\n"
		"\t                      volume attributes: %04x\n"
		"\t      number of files in root directory: %u\n"
		"\t           first block of volume bitmap: %u\n"
		"\t        start of next allocation search: %u\n"
		"\t  number of allocation blocks in volume: %u\n"
		"\t   size (in bytes) of allocation blocks: %u\n"
		"\t                     default clump size: %u\n"
		"\t       first allocation block in volume: %u\n"
		"\t            next unused catalog node ID: %u\n"
		"\t     number of unused allocation blocks: %u\n"
		"\t                     volume name length: %u\n"
		"\t                            volume name: %.*" CHARWIDTH "s\n"
		"\t           date and time of last backup: %llu (%s)\n"
		"\t          volume backup sequence number: %u\n"
		"\t                     volume write count: %u\n"
		"\t   clump size for extents overflow file: %u\n"
		"\t            clump size for catalog file: %u\n"
		"\tnumber of directories in root directory: %u\n"
		"\t              number of files in volume: %u\n"
		"\t        number of directories in volume: %u\n"
		"\t         information used by the Finder: %u\n"
		"\t         information used by the Finder: %u\n"
		"\t         information used by the Finder: %u\n"
		"\t         information used by the Finder: %u\n"
		"\t         information used by the Finder: %u\n"
		"\t         information used by the Finder: %u\n"
		"\t         information used by the Finder: %u\n"
		"\t         information used by the Finder: %u\n"
		"\t       size (in blocks) of volume cache: %u\n"
		"\tsize (in blocks) of volume bitmap cache: %u\n"
		"\tsize (in blocks) of common volume cache: %u\n"
		"\t          size of extents overflow file: %u\n"
		"\textent record for extents overflow file\n"
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
		, lpBuf[36], lpBuf[36], &lpBuf[37]
		, lastTime, szBufl
		, MAKEWORD(lpBuf[69], lpBuf[68])
		, MAKEUINT(MAKEWORD(lpBuf[73], lpBuf[72]), MAKEWORD(lpBuf[71], lpBuf[70]))
		, MAKEUINT(MAKEWORD(lpBuf[77], lpBuf[76]), MAKEWORD(lpBuf[75], lpBuf[74]))
		, *lpuiCatalogFileSize
		, MAKEWORD(lpBuf[83], lpBuf[82])
		, MAKEUINT(MAKEWORD(lpBuf[87], lpBuf[86]), MAKEWORD(lpBuf[85], lpBuf[84]))
		, MAKEUINT(MAKEWORD(lpBuf[91], lpBuf[90]), MAKEWORD(lpBuf[89], lpBuf[88]))
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
	);
	OutputFsExtentDataRecord(&lpBuf[132]);
	OutputVolDescLog(
		"\t                   size of catalog file: %u\n"
		"\t         extent record for catalog file:\n"
		, MAKEUINT(MAKEWORD(lpBuf[147], lpBuf[146]), MAKEWORD(lpBuf[145], lpBuf[144]))
	);
	OutputFsExtentDataRecord(&lpBuf[148]);
}

// https://developer.apple.com/library/archive/documentation/mac/Files/Files-105.html
VOID OutputFsCatalogFiles(
	LPBYTE lpBuf,
	INT nLBA,
	LPINT lpOfs,
	INT nAdditionalOfs
) {
	BYTE keyLength = lpBuf[0];
	OutputVolDescWithLBALog2("Catalog Files [ofs; %u, %#x]",
		"\t                         key length: %u\n"
		"\t                parent directory ID: %u\n"
		"\t           catalog node name length: %u\n"
		"\t                  catalog node name: %.*" CHARWIDTH "s\n"
		, nLBA, *lpOfs + nAdditionalOfs, *lpOfs + nAdditionalOfs, keyLength
		, MAKEUINT(MAKEWORD(lpBuf[5], lpBuf[4]), MAKEWORD(lpBuf[3], lpBuf[2]))
		, lpBuf[6], lpBuf[6], &lpBuf[7]
	);
	INT ofs = 7 + lpBuf[6];
	if (lpBuf[6] % 2 == 0) {
		ofs++;
	}
	OutputVolDescLog(
		"\t                        record type: %u\n"
		, lpBuf[ofs]);

	time_t creationTime = 0;
	_TCHAR szBufc[128] = {};
	time_t modificationTime = 0;
	_TCHAR szBufm[128] = {};
	time_t lastTime = 0;
	_TCHAR szBufl[128] = {};

	switch (lpBuf[ofs]) {
	case 1:
		GetMacTime(&lpBuf[ofs + 10], &creationTime, szBufc, 128);
		GetMacTime(&lpBuf[ofs + 14], &modificationTime, szBufm, 128);
		GetMacTime(&lpBuf[ofs + 18], &lastTime, szBufl, 128);
		OutputVolDescLog(
			"\t                    directory flags: %u\n"
			"\t                  directory valence: %u\n"
			"\t                       directory ID: %u\n"
			"\t          date and time of creation: %llu (%s)\n"
			"\t date and time of last modification: %llu (%s)\n"
			"\t       date and time of last backup: %llu (%s)\n"
			"\t                 Finder information\n"
			"\t\t        folder's window rectangle: %016llX\n"
			"\t\t                            flags: %04X\n"
			"\t\t      folder's location in window: %08X\n"
			"\t\t                    folder's view: %04X\n"
			"\t      additional Finder information\n"
			"\t\t                  scroll position: %08X\n"
			"\t\tdirectory ID cain of open folders: %u\n"
			"\t\t             script flag and code: %u\n"
			"\t\t                       comment ID: %u\n"
			"\t\t                home directory ID: %u\n"
			, MAKEWORD(lpBuf[ofs + 3], lpBuf[ofs + 2])
			, MAKEWORD(lpBuf[ofs + 5], lpBuf[ofs + 4])
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 9], lpBuf[ofs + 8]), MAKEWORD(lpBuf[ofs + 7], lpBuf[ofs + 6]))
			, creationTime, szBufc
			, modificationTime, szBufm
			, lastTime, szBufl
			, MAKEUINT64(MAKEUINT(MAKEWORD(lpBuf[ofs + 29], lpBuf[ofs + 28]), MAKEWORD(lpBuf[ofs + 27], lpBuf[ofs + 26]))
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 25], lpBuf[ofs + 24]), MAKEWORD(lpBuf[ofs + 23], lpBuf[ofs + 22])))
			, MAKEWORD(lpBuf[ofs + 31], lpBuf[ofs + 30])
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 35], lpBuf[ofs + 34]), MAKEWORD(lpBuf[ofs + 33], lpBuf[ofs + 32]))
			, MAKEWORD(lpBuf[ofs + 37], lpBuf[ofs + 36])
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 41], lpBuf[ofs + 40]), MAKEWORD(lpBuf[ofs + 39], lpBuf[ofs + 38]))
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 45], lpBuf[ofs + 44]), MAKEWORD(lpBuf[ofs + 43], lpBuf[ofs + 42]))
			, lpBuf[ofs + 46]
			, MAKEWORD(lpBuf[ofs + 49], lpBuf[ofs + 48])
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 53], lpBuf[ofs + 52]), MAKEWORD(lpBuf[ofs + 51], lpBuf[ofs + 50]))
		);
		*lpOfs += 70 + ofs;
		break;
	case 2:
		GetMacTime(&lpBuf[ofs + 44], &creationTime, szBufc, 128);
		GetMacTime(&lpBuf[ofs + 48], &modificationTime, szBufm, 128);
		GetMacTime(&lpBuf[ofs + 52], &lastTime, szBufl, 128);
		OutputVolDescLog(
			"\t                         file flags: %u\n"
			"\t                          file type: %u\n"
			"\t                 Finder information\n"
			"\t\t                        file type: %08X\n"
			"\t\t                     file creator: %08X\n"
			"\t\t                     Finder flags: %04X\n"
			"\t\t        file's location in window: %08X\n"
			"\t\t        window that contains file: %04X\n"
			"\t                            file ID: %u\n"
			"\t     first alloc. blk. of data fork: %u\n"
			"\t           logical EOF of data fork: %u\n"
			"\t          physical EOF of data fork: %u\n"
			"\t first alloc. blk. of resource fork: %u\n"
			"\t       logical EOF of resource fork: %u\n"
			"\t      physical EOF of resource fork: %u\n"
			"\t          date and time of creation: %llu (%s)\n"
			"\t date and time of last modification: %llu (%s)\n"
			"\t       date and time of last backup: %llu (%s)\n"
			"\t      additional Finder information\n"
			"\t\t                          icon ID: %u\n"
			"\t\t             script flag and code: %u\n"
			"\t\t                       comment ID: %u\n"
			"\t\t                home directory ID: %u\n"
			"\t                    file clump size: %u\n"
			"\t      first data fork extent record\n"
			, lpBuf[ofs + 2]
			, lpBuf[ofs + 3]
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 7], lpBuf[ofs + 6]), MAKEWORD(lpBuf[ofs + 5], lpBuf[ofs + 4]))
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 11], lpBuf[ofs + 10]), MAKEWORD(lpBuf[ofs + 9], lpBuf[ofs + 8]))
			, MAKEWORD(lpBuf[ofs + 13], lpBuf[ofs + 12])
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 17], lpBuf[ofs + 16]), MAKEWORD(lpBuf[ofs + 15], lpBuf[ofs + 14]))
			, MAKEWORD(lpBuf[ofs + 19], lpBuf[ofs + 18])
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 23], lpBuf[ofs + 22]), MAKEWORD(lpBuf[ofs + 21], lpBuf[ofs + 20]))
			, MAKEWORD(lpBuf[ofs + 25], lpBuf[ofs + 24])
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 29], lpBuf[ofs + 28]), MAKEWORD(lpBuf[ofs + 27], lpBuf[ofs + 26]))
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 33], lpBuf[ofs + 32]), MAKEWORD(lpBuf[ofs + 31], lpBuf[ofs + 30]))
			, MAKEWORD(lpBuf[ofs + 35], lpBuf[ofs + 34])
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 39], lpBuf[ofs + 38]), MAKEWORD(lpBuf[ofs + 37], lpBuf[ofs + 36]))
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 43], lpBuf[ofs + 42]), MAKEWORD(lpBuf[ofs + 41], lpBuf[ofs + 40]))
			, creationTime, szBufc
			, modificationTime, szBufm
			, lastTime, szBufl
			, MAKEWORD(lpBuf[ofs + 57], lpBuf[ofs + 56])
			, lpBuf[ofs + 64]
			, MAKEWORD(lpBuf[ofs + 67], lpBuf[ofs + 66])
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 71], lpBuf[ofs + 70]), MAKEWORD(lpBuf[ofs + 69], lpBuf[ofs + 68]))
			, MAKEWORD(lpBuf[ofs + 73], lpBuf[ofs + 72])
		);
		OutputFsExtentDataRecord(&lpBuf[ofs + 74]);
		OutputVolDescLog(
			"\t  first resource fork extent record\n"
		);
		OutputFsExtentDataRecord(&lpBuf[ofs + 86]);
		*lpOfs += 102 + ofs;
		break;
	case 3:
		OutputVolDescLog(
			"\t     parent ID for this directory: %u\n"
			"\t    name length of this directory: %u\n"
			"\t           name of this directory: %.*" CHARWIDTH "s\n"
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 13], lpBuf[ofs + 12]), MAKEWORD(lpBuf[ofs + 11], lpBuf[ofs + 10]))
			, lpBuf[ofs + 14], lpBuf[ofs + 14], &lpBuf[ofs + 15]
		);
		*lpOfs += 46 + ofs;
		break;
	case 4:
		OutputVolDescLog(
			"\t          parent ID for this file: %u\n"
			"\t         name length of this file: %u\n"
			"\t                name of this file: %.*" CHARWIDTH "s\n"
			, MAKEUINT(MAKEWORD(lpBuf[ofs + 13], lpBuf[ofs + 12]), MAKEWORD(lpBuf[ofs + 11], lpBuf[ofs + 10]))
			, lpBuf[ofs + 14], lpBuf[ofs + 14], &lpBuf[ofs + 15]
		);
		*lpOfs += 46 + ofs;
		break;
	default:
		*lpOfs += 42;
		break;
	}
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
	CHAR fname[32 + 1] = {};
	while (cur < uiDirSize) {
		LPBYTE dirEnt = lpBuf + cur;
		strncpy(fname, (LPCCH)&dirEnt[32], sizeof(fname) - 1);
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
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("PCE Warning msg & all stuff"), nLBA, (UINT)nLBA);
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
			strncpy(str, ptr, sizeof(str) - 1);
			OutputVolDescLog("\t%" CHARWIDTH "s\n", str);
		}
	}
}

VOID OutputFsPceBootSector(
	LPBYTE lpBuf,
	INT nLBA
) {
	CHAR str[24 + 1] = {};
	strncpy(str, (LPCCH)&lpBuf[32], sizeof(str) - 1);
	CHAR str2[50 + 1] = {};
	strncpy(str2, (LPCCH)&lpBuf[56], sizeof(str2) - 1);
	CHAR str3[17 + 1] = {};
	strncpy(str3, (LPCCH)&lpBuf[106], sizeof(str3) - 1);
	CHAR str4[7 + 1] = {};
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
	LPCCH nest,
	LPBYTE lpBuf
) {
	WORD sTime = MAKEWORD(lpBuf[0], lpBuf[1]);
	CHAR cTimeZone = (CHAR)(sTime >> 12 & 0x0f);
	SHORT nTime = sTime & 0xfff;

	OutputVolDescLog(
		"%s Date and Time: %u-%02u-%02uT%02u:%02u:%02u.%02u%02u%02u%+03d:%02d"
		, nest, MAKEWORD(lpBuf[2], lpBuf[3]), lpBuf[4], lpBuf[5], lpBuf[6], lpBuf[7], lpBuf[8], lpBuf[9], lpBuf[10], lpBuf[11], nTime / 60, nTime % 60);
	if (cTimeZone == 0) {
		OutputVolDescLog(" (UTC)\n");
	}
	else if (cTimeZone == 1) {
		OutputVolDescLog(" (LocalTime)\n");
	}
	else if (cTimeZone == 2) {
		OutputVolDescLog(" (OriginalTime)\n");
	}
	else {
		OutputVolDescLog(" (Reserved)\n");
	}
}

VOID OutputFsRegid(
	LPCCH nest,
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"%s            Flags: %u\n"
		"%s       Identifier: %.23" CHARWIDTH "s\n"
		"%sIdentifier Suffix: "
		, nest, lpBuf[0]
		, nest, &lpBuf[1]
		, nest
	);
	for (INT i = 24; i < 32; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog("\n");
}

VOID OutputFsBootDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR("Architecture Type"));
	OutputFsRegid("\t", lpBuf + 8);
	OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR("Boot Identifier"));
	OutputFsRegid("\t", lpBuf + 8);

	OutputVolDescLog(
		"\tBoot Extent Location: %u\n"
		"\t  Boot Extent Length: %u\n"
		"\t        Load Address: %u%u\n"
		"\t       Start Address: %u%u\n"
		, MAKEUINT(MAKEWORD(lpBuf[72], lpBuf[73]), MAKEWORD(lpBuf[74], lpBuf[75]))
		, MAKEUINT(MAKEWORD(lpBuf[76], lpBuf[77]), MAKEWORD(lpBuf[78], lpBuf[79]))
		, MAKEUINT(MAKEWORD(lpBuf[80], lpBuf[81]), MAKEWORD(lpBuf[82], lpBuf[83]))
		, MAKEUINT(MAKEWORD(lpBuf[84], lpBuf[85]), MAKEWORD(lpBuf[86], lpBuf[87]))
		, MAKEUINT(MAKEWORD(lpBuf[88], lpBuf[89]), MAKEWORD(lpBuf[90], lpBuf[91]))
		, MAKEUINT(MAKEWORD(lpBuf[92], lpBuf[93]), MAKEWORD(lpBuf[94], lpBuf[95]))
	);

	OutputFsRecordingDateAndTime("\tDescriptor Creation", lpBuf + 96);
	OutputVolDescLog(
		"\t   Flags: %u\n"
		"\tBoot Use: "
		, MAKEWORD(lpBuf[108], lpBuf[109])
	);
	for (INT i = 142; i < 2048; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog("\n");
}

VOID OutputFsTerminatingExtendedAreaDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLog("\t     Structure Data: ");
	for (INT i = 7; i < 2048; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog("\n");
}

VOID OutputFsNSRDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLog("\t     Structure Data: ");
	for (INT i = 8; i < 2048; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog("\n");
}

VOID OutputFsBeginningExtendedAreaDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLog("\t     Structure Data: ");
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
		"\t     Structure Type: %u\n"
		"\tStandard Identifier: %.5" CHARWIDTH "s\n"
		"\t  Structure Version: %u\n"
		, nLBA, (UINT)nLBA, lpBuf[0], &lpBuf[1], lpBuf[6]);
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
	LPCCH nest,
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"%s       Character Set Type: %u\n"
		"%sCharacter Set Information: %.63" CHARWIDTH "s\n"
		, nest, lpBuf[0]
		, nest, &lpBuf[1]
	);
}

VOID OutputFsExtentDescriptor(
	LPCCH nest,
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"%s  Extent Length: %u\n"
		"%sExtent Location: %u\n"
		, nest, MAKEUINT(MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3]))
		, nest, MAKEUINT(MAKEWORD(lpBuf[4], lpBuf[5]), MAKEWORD(lpBuf[6], lpBuf[7]))
	);
}

VOID OutputFsRecordedAddress(
	LPCCH nest,
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"%s      Logical Block Number: %u\n"
		"%sPartition Reference Number: %u\n"
		, nest, MAKEUINT(MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3]))
		, nest, MAKEWORD(lpBuf[4], lpBuf[5])
	);
}

VOID OutputFsShortAllocationDescriptor(
	LPCCH nest,
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"%s  Extent Length: %u\n"
		"%s          Flags: %d\n"
		"%sExtent Position: %u\n"
		, nest, MAKEUINT(MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3] & 0x3f))
		, nest, lpBuf[3] & 0xc0
		, nest, MAKEUINT(MAKEWORD(lpBuf[4], lpBuf[5]), MAKEWORD(lpBuf[6], lpBuf[3]))
	);
}

VOID OutputFsLongAllocationDescriptor(
	LPCCH nest,
	LPBYTE lpBuf
) {
#if 0
	OutputVolDescLog(
		"%sExtent Length: %u\n"
		"\t%s    Flags: %d\n"
		"\t%sExtent Location\n"
		, nest, MAKEUINT(MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3] & 0x7f))
		, nest, lpBuf[3] & 0x80, nest
	);
#else
	OutputVolDescLog(
		"%s     Extent Length: %u\n"
		"%s   Extent Location\n"
		, nest, MAKEUINT(MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3]))
		, nest
	);
#endif
	OutputFsRecordedAddress(nest, lpBuf + 4);

	OutputVolDescLog(
		"%sImplementation Use\n"
		"\t%s         Flags: %u\n"
		"\t%s     Unique Id: %u\n"
		, nest, nest, MAKEWORD(lpBuf[10], lpBuf[11])
		, nest, MAKEUINT(MAKEWORD(lpBuf[12], lpBuf[13]), MAKEWORD(lpBuf[14], lpBuf[15]))
	);
}

VOID OutputFsDescriptorTag(
	LPCCH nest,
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"%s           Descriptor Tag\n"
		"\t%s       Tag Identifier: %u\n"
		"\t%s   Descriptor Version: %u\n"
		"\t%s         Tag Checksum: %u\n"
		"\t%s    Tag Serial Number: %u\n"
		"\t%s       Descriptor CRC: %04x\n"
		"\t%sDescriptor CRC Length: %u\n"
		"\t%s         Tag Location: %u\n"
		, nest, nest, MAKEWORD(lpBuf[0], lpBuf[1])
		, nest, MAKEWORD(lpBuf[2], lpBuf[3])
		, nest, lpBuf[4]
		, nest, MAKEWORD(lpBuf[6], lpBuf[7])
		, nest, MAKEWORD(lpBuf[8], lpBuf[9])
		, nest, MAKEWORD(lpBuf[10], lpBuf[11])
		, nest, MAKEUINT(MAKEWORD(lpBuf[12], lpBuf[13]), MAKEWORD(lpBuf[14], lpBuf[15]))
	);
}

VOID OutputFsFileEntry(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\t                                    ICB Tag\n"
		"\t\tPrior Recorded Number of Direct Entries: %u\n"
		"\t\t                          Strategy Type: %u\n"
		"\t\t                     Strategy Parameter: %02u%02u\n"
		"\t\t              Maximum Number of Entries: %u\n"
		"\t\t                              File Type: %u\n"
		"\t\t                    Parent ICB Location\n"
		, MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19]))
		, MAKEWORD(lpBuf[20], lpBuf[21]), lpBuf[22], lpBuf[23]
		, MAKEWORD(lpBuf[24], lpBuf[25]), lpBuf[27]
	);
	OutputFsRecordedAddress("\t\t\t         ", lpBuf + 28);

	OutputVolDescLog(
		"\t\t                                  Flags: %#04x\n"
		"\t                                        Uid: %x\n"
		"\t                                        Gid: %x\n"
		"\t                                Permissions: %#04x\n"
		"\t                            File Link Count: %u\n"
		"\t                              Record Format: %u\n"
		"\t                  Record Display Attributes: %u\n"
		"\t                              Record Length: %u\n"
		"\t                         Information Length: %llu\n"
		"\t                    Logical Blocks Recorded: %llu\n"
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
	OutputFsRecordingDateAndTime("\t                       Access", lpBuf + 72);
	OutputFsRecordingDateAndTime("\t                 Modification", lpBuf + 84);
	OutputFsRecordingDateAndTime("\t                    Attribute", lpBuf + 96);
	OutputVolDescLog(
		"\t                                 Checkpoint: %u\n"
		, MAKEUINT(MAKEWORD(lpBuf[108], lpBuf[109]), MAKEWORD(lpBuf[110], lpBuf[111]))
	);
	OutputVolDescLog("\t                     Extended Attribute ICB\n");
	OutputFsLongAllocationDescriptor("\t\t                     ", lpBuf + 112);

	OutputVolDescLog("\t                  Implementation Identifier\n");
	OutputFsRegid("\t                          ", lpBuf + 128);

	UINT L_EA = MAKEUINT(MAKEWORD(lpBuf[168], lpBuf[169]), MAKEWORD(lpBuf[170], lpBuf[171]));
	UINT L_AD = MAKEUINT(MAKEWORD(lpBuf[172], lpBuf[173]), MAKEWORD(lpBuf[174], lpBuf[175]));
	OutputVolDescLog(
		"\t                                  Unique Id: %llu\n"
		"\t              Length of Extended Attributes: %u\n"
		"\t           Length of Allocation Descriptors: %u\n"
		, MAKEUINT64(MAKEUINT(MAKEWORD(lpBuf[160], lpBuf[161]), MAKEWORD(lpBuf[162], lpBuf[163]))
			, MAKEUINT(MAKEWORD(lpBuf[164], lpBuf[165]), MAKEWORD(lpBuf[166], lpBuf[167])))
		, L_EA, L_AD
	);

	OutputVolDescLog(
		"\t                        Extended Attributes\n"
		"\t\t   Extended Attribute Header Descriptor\n"
	);
	UINT uiOfs = 176;
	OutputFsDescriptorTag("\t\t\t          ", lpBuf + uiOfs);

	UINT ImplAttrPos = MAKEUINT(MAKEWORD(lpBuf[uiOfs + 16], lpBuf[uiOfs + 17]), MAKEWORD(lpBuf[uiOfs + 18], lpBuf[uiOfs + 19]));
	UINT AppAttrPos = MAKEUINT(MAKEWORD(lpBuf[uiOfs + 20], lpBuf[uiOfs + 21]), MAKEWORD(lpBuf[uiOfs + 22], lpBuf[uiOfs + 23]));
	OutputVolDescLog(
		"\t\t\t Implementation Attributes Location: %u\n"
		"\t\t\t    Application Attributes Location: %u\n"
		, ImplAttrPos, AppAttrPos
	);
	uiOfs += 24;

	while (uiOfs < 176 + L_EA) {
		UINT aType = MAKEUINT(MAKEWORD(lpBuf[uiOfs], lpBuf[uiOfs + 1])
			, MAKEWORD(lpBuf[uiOfs + 2], lpBuf[uiOfs + 3]));
		UINT A_L = MAKEUINT(MAKEWORD(lpBuf[uiOfs + 8], lpBuf[uiOfs + 9])
			, MAKEWORD(lpBuf[uiOfs + 10], lpBuf[uiOfs + 11]));
		UINT IU_L = MAKEUINT(MAKEWORD(lpBuf[uiOfs + 12], lpBuf[uiOfs + 13])
			, MAKEWORD(lpBuf[uiOfs + 14], lpBuf[uiOfs + 15]));
		if (aType == 1) {
			OutputVolDescLog(
				"\t\t              Character Set Information\n"
				"\t\t\t                     Attribute Type: %u\n"
				"\t\t\t                  Attribute Subtype: %u\n"
				"\t\t\t                   Attribute Length: %u\n"
				"\t\t\t            Escape Sequences Length: %u\n"
				"\t\t\t                 Character Set Type: %u\n"
				"\t\t\t                   Escape Sequences: "
				, aType, lpBuf[uiOfs + 4], A_L, IU_L, lpBuf[uiOfs + 16]
			);
			for (UINT i = uiOfs + 17; i < uiOfs + 17 + IU_L; i++) {
				OutputVolDescLog(" %02x", lpBuf[i]);
			}
			OutputVolDescLog("\n");
			uiOfs += 17 + IU_L;
		}
		else if (aType == 3) {
			OutputVolDescLog(
				"\t\t                   Altinate Permissions\n"
				"\t\t\t                     Attribute Type: %u\n"
				"\t\t\t                  Attribute Subtype: %u\n"
				"\t\t\t                   Attribute Length: %u\n"
				"\t\t\t               Owner Identification: %u\n"
				"\t\t\t               Group Identification: %u\n"
				"\t\t\t                         Permission: %u\n"
				, aType, lpBuf[uiOfs + 4], A_L, MAKEWORD(lpBuf[uiOfs + 12], lpBuf[uiOfs + 13])
				, MAKEWORD(lpBuf[uiOfs + 14], lpBuf[uiOfs + 15]), MAKEWORD(lpBuf[uiOfs + 16], lpBuf[uiOfs + 17])
			);
			uiOfs += 18;
		}
		else if (aType == 5) {
			OutputVolDescLog(
				"\t\t          File Times Extended Attribute\n"
				"\t\t\t                     Attribute Type: %u\n"
				"\t\t\t                  Attribute Subtype: %u\n"
				"\t\t\t                   Attribute Length: %u\n"
				"\t\t\t                        Data Length: %u\n"
				"\t\t\t               File Times Existence: %u\n"
				"\t\t\t                         File Times:"
				, aType, lpBuf[uiOfs + 4], A_L, IU_L
				, MAKEUINT(MAKEWORD(lpBuf[uiOfs + 16], lpBuf[uiOfs + 17]), MAKEWORD(lpBuf[uiOfs + 18], lpBuf[uiOfs + 19]))
			);
			for (UINT i = uiOfs + 20; i < uiOfs + 20 + IU_L; i++) {
				OutputVolDescLog(" %02x", lpBuf[i]);
			}
			OutputVolDescLog("\n");
			uiOfs += 20 + IU_L;
		}
		else if (aType == 6) {
			OutputVolDescLog(
				"\t\t   Information Times Extended Attribute\n"
				"\t\t\t                     Attribute Type: %u\n"
				"\t\t\t                  Attribute Subtype: %u\n"
				"\t\t\t                   Attribute Length: %u\n"
				"\t\t\t                        Data Length: %u\n"
				"\t\t\t        Information Times Existence: %u\n"
				"\t\t\t                  Information Times:"
				, aType, lpBuf[uiOfs + 4], A_L, IU_L
				, MAKEUINT(MAKEWORD(lpBuf[uiOfs + 16], lpBuf[uiOfs + 17]), MAKEWORD(lpBuf[uiOfs + 18], lpBuf[uiOfs + 19]))
			);
			for (UINT i = uiOfs + 20; i < uiOfs + 20 + IU_L; i++) {
				OutputVolDescLog(" %02x", lpBuf[i]);
			}
			OutputVolDescLog("\n");
			uiOfs += 20 + IU_L;
		}
		else if (aType == 12) {
			OutputVolDescLog(
				"\t\tDevice Specification Extended Attribute\n"
				"\t\t\t                     Attribute Type: %u\n"
				"\t\t\t                  Attribute Subtype: %u\n"
				"\t\t\t                   Attribute Length: %u\n"
				"\t\t\t          Implementation Use Length: %u\n"
				"\t\t\t        Major Device Identification: %u\n"
				"\t\t\t        Minor Device Identification: %u\n"
				"\t\t\t                 Implementation Use:"
				, aType, lpBuf[uiOfs + 4], A_L, IU_L
				, MAKEUINT(MAKEWORD(lpBuf[uiOfs + 16], lpBuf[uiOfs + 17]), MAKEWORD(lpBuf[uiOfs + 18], lpBuf[uiOfs + 19]))
				, MAKEUINT(MAKEWORD(lpBuf[uiOfs + 20], lpBuf[uiOfs + 21]), MAKEWORD(lpBuf[uiOfs + 22], lpBuf[uiOfs + 23]))
			);
			for (UINT i = uiOfs + 24; i < uiOfs + 24 + IU_L; i++) {
				OutputVolDescLog(" %02x", lpBuf[i]);
			}
			OutputVolDescLog("\n");
			uiOfs += 24 + IU_L;
		}
		else if (aType == 2048) {
			OutputVolDescLog(
				"\t\t  Implementation Use Extended Attribute\n"
				"\t\t\t                     Attribute Type: %u\n"
				"\t\t\t                  Attribute Subtype: %u\n"
				"\t\t\t                   Attribute Length: %u\n"
				"\t\t\t          Implementation Use Length: %u\n"
				"\t\t\t          Implementation Identifier\n"
				, aType, lpBuf[uiOfs + 4], A_L, IU_L
			);
			OutputFsRegid("\t\t\t\t              ", lpBuf + uiOfs + 16);

			OutputVolDescLog("\t\t\t                 Implementation Use:");
			for (UINT i = uiOfs + 48; i < uiOfs + 48 + IU_L; i++) {
				OutputVolDescLog(" %02x", lpBuf[i]);
			}
			OutputVolDescLog("\n");
			uiOfs += 48 + IU_L;
		}
		else {
			break;
		}
	}

	OutputVolDescLog("\t                     Allocation descriptors\n");
	OutputFsShortAllocationDescriptor("\t\t                        ", lpBuf + 176 + L_EA);
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
		"\t      File Version Number: %u\n"
		"\t     File Characteristics: %u\n"
		"\tLength of File Identifier: %u\n"
		"\tICB\n"
		, MAKEWORD(lpBuf[16 + uiOfs], lpBuf[17 + uiOfs]), lpBuf[18 + uiOfs], L_FI
	);
	OutputFsLongAllocationDescriptor("\t\t", lpBuf + 20 + uiOfs);

	WORD L_IU = MAKEWORD(lpBuf[36 + uiOfs], lpBuf[37 + uiOfs]);
	OutputVolDescLog(
		"\tLength of Implementation Use: %u\n"
		"\t          Implementation Use:\n"
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

VOID ConvUTF16toUTF8(
	LPCCH nest,
	BYTE byFlag,
	LPBYTE pIn,
	INT nInSize,
	WCHAR* pWOut
) {
	OutputVolDescLog("%sCompression ID: %02u, ", nest, byFlag);
	if (byFlag == 16) {
		for (INT i = 0, j = 0; i < nInSize; i++, j += 2) {
			pWOut[i] = (WCHAR)((pIn[j] << 8) | (pIn[j + 1]));
		}
	}
}

VOID OutputFsFileSetDescriptor(
	LPBYTE lpBuf,
	PUDF pUdf
) {
	OutputFsRecordingDateAndTime("\t                      Recording", lpBuf + 16);
	OutputVolDescLog(
		"\t                            Interchange Level: %u\n"
		"\t                    Maximum Interchange Level: %u\n"
		"\t                           Character Set List: %u\n"
		"\t                   Maximum Character Set List: %u\n"
		"\t                              File Set Number: %u\n"
		"\t                   File Set Descriptor Number: %u\n"
		"\t      Logical Volume Identifier Character Set\n"
		, MAKEWORD(lpBuf[28], lpBuf[29])
		, MAKEWORD(lpBuf[30], lpBuf[31])
		, MAKEUINT(MAKEWORD(lpBuf[32], lpBuf[33]), MAKEWORD(lpBuf[34], lpBuf[35]))
		, MAKEUINT(MAKEWORD(lpBuf[36], lpBuf[37]), MAKEWORD(lpBuf[38], lpBuf[39]))
		, MAKEUINT(MAKEWORD(lpBuf[40], lpBuf[41]), MAKEWORD(lpBuf[42], lpBuf[43]))
		, MAKEUINT(MAKEWORD(lpBuf[44], lpBuf[45]), MAKEWORD(lpBuf[46], lpBuf[47]))
	);
	OutputFsCharspec("\t\t                ", lpBuf + 48);

	WCHAR tmpW[128] = {};
	ConvUTF16toUTF8("\t", lpBuf[112], &lpBuf[113], 127, tmpW);
	if (lpBuf[112] == 16) {
		OutputVolDescLog("Logical Volume Identifier: %.63ls\n", tmpW);
		ZeroMemory(tmpW, sizeof(tmpW));
	}
	else {
		OutputVolDescLog("Logical Volume Identifier: %.127" CHARWIDTH "s\n", &lpBuf[113]);
	}

	OutputVolDescLog("\t                       File Set Character Set\n");
	OutputFsCharspec("\t\t                ", lpBuf + 240);

	ConvUTF16toUTF8("\t", lpBuf[304], &lpBuf[305], 31, tmpW);
	if (lpBuf[304] == 16) {
		OutputVolDescLog("      File Set Identifier: %.15ls\n", tmpW);
		ZeroMemory(tmpW, sizeof(tmpW));
	}
	else {
		OutputVolDescLog("      File Set Identifier: %.31" CHARWIDTH "s\n", &lpBuf[305]);
	}

	ConvUTF16toUTF8("\t", lpBuf[336], &lpBuf[337], 31, tmpW);
	if (lpBuf[336] == 16) {
		OutputVolDescLog("Copyright File Identifier: %.15ls\n", tmpW);
		ZeroMemory(tmpW, sizeof(tmpW));
	}
	else {
		OutputVolDescLog("Copyright File Identifier: %.31" CHARWIDTH "s\n", &lpBuf[337]);
	}

	ConvUTF16toUTF8("\t", lpBuf[368], &lpBuf[369], 31, tmpW);
	if (lpBuf[368] == 16) {
		OutputVolDescLog(" Abstract File Identifier: %.15ls\n", tmpW);
		ZeroMemory(tmpW, sizeof(tmpW));
	}
	else {
		OutputVolDescLog(" Abstract File Identifier: %.31" CHARWIDTH "s\n", &lpBuf[369]);
	}

	OutputVolDescLog("\t                           Root Directory ICB\n");
	OutputFsLongAllocationDescriptor("\t\t                       ", lpBuf + 400);
	pUdf->uiFileEntryLen = MAKEUINT(MAKEWORD(lpBuf[400], lpBuf[401]), MAKEWORD(lpBuf[402], lpBuf[403] & 0x7f));
	pUdf->uiFileEntryPos = MAKEUINT(MAKEWORD(lpBuf[404], lpBuf[405]), MAKEWORD(lpBuf[406], lpBuf[407]));

	OutputVolDescLog("\t                            Domain Identifier\n");
	OutputFsRegid("\t\t                        ", lpBuf + 416);

	OutputVolDescLog("\t                                  Next Extent\n");
	OutputFsLongAllocationDescriptor("\t\t                       ", lpBuf + 448);

	OutputVolDescLog("\t                  System Stream Directory ICB\n");
	OutputFsLongAllocationDescriptor("\t\t                       ", lpBuf + 464);
}

VOID OutputFsLogicalVolumeIntegrityDescriptor(
	LPBYTE lpBuf
) {
	OutputFsRecordingDateAndTime("\t         Recording", lpBuf + 16);
	OutputVolDescLog(
		"\t                  Integrity Type: %u\n"
		"\t           Next Integrity Extent\n"
		, MAKEUINT(MAKEWORD(lpBuf[28], lpBuf[29]), MAKEWORD(lpBuf[30], lpBuf[31])));
	OutputFsExtentDescriptor("\t\t             ", lpBuf + 32);

	OutputVolDescLog(
		"\tLogical Volume Header Descriptor\n"
		"\t\t                   Unique ID: %llu\n"
		, MAKEUINT64(MAKEUINT(MAKEWORD(lpBuf[40], lpBuf[41]), MAKEWORD(lpBuf[42], lpBuf[43]))
			, MAKEUINT(MAKEWORD(lpBuf[44], lpBuf[45]), MAKEWORD(lpBuf[46], lpBuf[47])))
	);

	UINT N_P =
		MAKEUINT(MAKEWORD(lpBuf[72], lpBuf[73]), MAKEWORD(lpBuf[74], lpBuf[75]));
	UINT L_IU =
		MAKEUINT(MAKEWORD(lpBuf[76], lpBuf[77]), MAKEWORD(lpBuf[78], lpBuf[79]));
	OutputVolDescLog(
		"\t            Number of Partitions: %u\n"
		"\t    Length of Implementation Use: %u\n"
		, N_P, L_IU);

	UINT nOfs = N_P * 4;
	if (0 < N_P) {
		OutputVolDescLog("\t                Free Space Table: ");
		for (UINT i = 0; i < nOfs; i += 4) {
			OutputVolDescLog("%u, "
				, MAKEUINT(MAKEWORD(lpBuf[80 + i], lpBuf[81 + i]), MAKEWORD(lpBuf[82 + i], lpBuf[83 + i])));
		}
		OutputVolDescLog("\n\t                      Size Table: ");
		for (UINT i = 80 + nOfs, j = 0; j < nOfs; j += 4) {
			OutputVolDescLog("%u, "
				, MAKEUINT(MAKEWORD(lpBuf[i + j], lpBuf[i + 1 + j]), MAKEWORD(lpBuf[i + 2 + j], lpBuf[i + 3 + j])));
		}
		OutputVolDescLog("\n");
	}
	if (0 < L_IU) {
		nOfs = 80 + N_P * 8;
		OutputVolDescLog(
			"\t               Implementation ID\n"
			"\t\t                       flags: %d\n"
			"\t\t                          id: %.23" CHARWIDTH "s\n"
			"\t\t                      suffix: %.8" CHARWIDTH "s\n"
			"\t                 Number of Files: %u\n"
			"\t           Number of Directories: %u\n"
			"\t       Minimum UDF Read Revision: %#04x\n"
			"\t      Minimum UDF Write Revision: %#04x\n"
			"\t      Maximum UDF Write Revision: %#04x\n"
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
		"\t Number of Allocation Descriptors: %u\n"
		, MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])),
		N_AD);
	if (0 < N_AD) {
		OutputVolDescLog("\tAllocation Descriptors\n");
		for (UINT i = 0; i < N_AD * 8; i += 8) {
			OutputFsExtentDescriptor("\t\t", lpBuf + 24 + i);
		}
	}
}

VOID OutputFsLogicalVolumeDescriptor(
	LPBYTE lpBuf,
	PUDF pUdf
) {
	OutputVolDescLog(
		"\t            Volume Descriptor Sequence Number: %u\n"
		"\t                     Descriptor Character Set\n"
		, MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19]))
	);
	OutputFsCharspec("\t\t                ", lpBuf + 20);

	WCHAR tmpW[128] = {};
	ConvUTF16toUTF8("\t", lpBuf[84], &lpBuf[85], 127, tmpW);
	if (lpBuf[84] == 16) {
		OutputVolDescLog("Logical Volume Identifier: %.63ls\n", tmpW);
	}
	else {
		OutputVolDescLog("Logical Volume Identifier: %.127" CHARWIDTH "s\n", &lpBuf[85]);
	}
	OutputVolDescLog(
		"\t                           Logical Block Size: %u\n"
		"\t                            Domain Identifier\n"
		, MAKEUINT(MAKEWORD(lpBuf[212], lpBuf[213]), MAKEWORD(lpBuf[214], lpBuf[215]))
	);
	OutputFsRegid("\t\t                        ", lpBuf + 216);

	OutputVolDescLog("\t                  Logical Volume Contents Use\n");
	OutputFsLongAllocationDescriptor("\t\t                       ", lpBuf + 248);
	pUdf->uiFSDLen = MAKEUINT(MAKEWORD(lpBuf[248], lpBuf[249]), MAKEWORD(lpBuf[250], lpBuf[251] & 0x7f));
	pUdf->uiFSDPos = MAKEUINT(MAKEWORD(lpBuf[252], lpBuf[253]), MAKEWORD(lpBuf[254], lpBuf[255]));

	UINT MT_L = MAKEUINT(MAKEWORD(lpBuf[264], lpBuf[265]), MAKEWORD(lpBuf[266], lpBuf[267]));
	UINT N_PM = MAKEUINT(MAKEWORD(lpBuf[268], lpBuf[269]), MAKEWORD(lpBuf[270], lpBuf[271]));
	OutputVolDescLog(
		"\t                             Map Table Length: %u\n"
		"\t                     Number of Partition Maps: %u\n"
		"\t                    Implementation Identifier\n"
		, MT_L
		, N_PM
	);
	OutputFsRegid("\t\t                        ", lpBuf + 272);

	OutputVolDescLog("\t                           Implementation Use: ");
	for (INT i = 304; i < 432; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog("\n\t                    Integrity Sequence Extent\n");
	OutputFsExtentDescriptor("\t\t                          ", lpBuf + 432);
	pUdf->uiLogicalVolumeIntegrityLen = MAKEUINT(MAKEWORD(lpBuf[432], lpBuf[433]), MAKEWORD(lpBuf[434], lpBuf[435]));
	pUdf->uiLogicalVolumeIntegrityPos = MAKEUINT(MAKEWORD(lpBuf[436], lpBuf[437]), MAKEWORD(lpBuf[438], lpBuf[439]));

	for (UINT k = 0; k < N_PM; k++) {
		if (0 < MT_L) {
			if (lpBuf[440] == 1) {
				OutputVolDescLog(
					"\t                               Partition Maps\n"
					"\t\t                       Partition Map Type: %d\n"
					"\t\t                     Partition Map Length: %d\n"
					"\t\t                   Volume Sequence Number: %d\n"
					"\t\t                         Partition Number: %d\n"
					, lpBuf[440], lpBuf[441]
					, MAKEWORD(lpBuf[442], lpBuf[443])
					, MAKEWORD(lpBuf[444], lpBuf[445])
				);
#if 0
				if (6 < MT_L) {
					OutputVolDescLog(
						"\tPartition Maps\n"
						"\t\t  Partition Map Type: %d\n"
						"\t\tPartition Map Length: %d\n"
						"\t\tPartition Identifier: "
						, lpBuf[446], lpBuf[447]
					);
					for (INT i = 448; i < 510; i++) {
						OutputVolDescLog("%02x", lpBuf[i]);
					}
					OutputVolDescLog("\n");
				}
#endif
			}
			else if (lpBuf[440] == 2) {
				OutputVolDescLog(
					"\t                               Partition Maps\n"
					"\t\t                     Partition Map Type: %d\n"
					"\t\t                   Partition Map Length: %d\n"
					"\t\t                   Partition Identifier: "
					, lpBuf[440], lpBuf[441]
				);
				for (INT i = 442; i < 504; i++) {
					OutputVolDescLog("%02x", lpBuf[i]);
				}
				OutputVolDescLog("\n");
			}
		}
	}
}

VOID OutputFsPartitionDescriptor(
	LPBYTE lpBuf,
	PUDF pUdf
) {
	OutputVolDescLog(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\t                  Partition Flags: %u\n"
		"\t                 Partition Number: %u\n"
		"\t               Partition Contents\n"
		, MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19]))
		, MAKEWORD(lpBuf[20], lpBuf[21])
		, MAKEWORD(lpBuf[22], lpBuf[23])
	);

	OutputFsRegid("\t\t            ", lpBuf + 24);
#if 0
	OutputVolDescLog(
		"\tPartition Contents Use\n"
		"\tPartition Header Descriptor\n"
		"\t\tUnallocated Space Table\n"
	);
	OutputFsShortAllocationDescriptor("\t\t\t", lpBuf + 56);

	OutputVolDescLog("\t\tUnallocated Space Bitmap\n");
	OutputFsShortAllocationDescriptor("\t\t\t", lpBuf + 64);

	OutputVolDescLog("\t\tPartition Integrity Table\n");
	OutputFsShortAllocationDescriptor("\t\t\t", lpBuf + 72);

	OutputVolDescLog("\t\tFreed Space Table\n");
	OutputFsShortAllocationDescriptor("\t\t\t", lpBuf + 80);

	OutputVolDescLog("\t\tFreed Space Bitmap\n");
	OutputFsShortAllocationDescriptor("\t\t\t", lpBuf + 88);
#else
	OutputVolDescLog("\t           Partition Contents Use: ");
	for (INT i = 56; i < 184; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
#endif
	pUdf->uiPartitionPos = MAKEUINT(MAKEWORD(lpBuf[188], lpBuf[189]), MAKEWORD(lpBuf[190], lpBuf[191]));
	pUdf->uiPartitionLen = MAKEUINT(MAKEWORD(lpBuf[192], lpBuf[193]), MAKEWORD(lpBuf[194], lpBuf[195]));
	UINT accessType = MAKEUINT(MAKEWORD(lpBuf[184], lpBuf[185]), MAKEWORD(lpBuf[186], lpBuf[187]));
	OutputVolDescLog("\n\t                      Access Type: %u ", accessType);

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
		"\t      Partition Starting Location: %u\n"
		"\t                 Partition Length: %u\n"
		"\t        Implementation Identifier\n"
		, pUdf->uiPartitionPos, pUdf->uiPartitionLen
	);
	OutputFsRegid("\t\t            ", lpBuf + 196);

	OutputVolDescLog("\t               Implementation Use: ");
	for (INT i = 228; i < 356; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog("\n");
}

VOID OutputFsImplementationUseVolumeDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\t               Volume Descriptor Sequence Number: %u\n"
		"\t                       Implementation Identifier\n",
		MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])));
	OutputFsRegid("\t\t                           ", lpBuf + 20);

	INT nOfs = 52;
	OutputVolDescLog("\t              Logical Volume Information Charset\n");
	OutputFsCharspec("\t\t                   ", lpBuf + nOfs);

	WCHAR tmpW[128] = {};
	ConvUTF16toUTF8("\t   ", lpBuf[nOfs + 64], &lpBuf[nOfs + 65], 127, tmpW);
	if (lpBuf[nOfs + 64] == 16) {
		OutputVolDescLog("Logical Volume Identifier: %.63ls\n", tmpW);
		ZeroMemory(tmpW, sizeof(tmpW));
	}
	else {
		OutputVolDescLog("Logical Volume Identifier: %.127" CHARWIDTH "s\n", &lpBuf[nOfs + 65]);
	}
	ConvUTF16toUTF8("\t", lpBuf[nOfs + 192], &lpBuf[nOfs + 193], 36, tmpW);
	if (lpBuf[nOfs + 192] == 16) {
		OutputVolDescLog("Logical Volume Information 1: %.18ls\n", tmpW);
		ZeroMemory(tmpW, sizeof(tmpW));
	}
	else {
		OutputVolDescLog("Logical Volume Information 1: %.36" CHARWIDTH "s\n", &lpBuf[nOfs + 193]);
	}
	ConvUTF16toUTF8("\t", lpBuf[nOfs + 228], &lpBuf[nOfs + 229], 36, tmpW);
	if (lpBuf[nOfs + 228] == 16) {
		OutputVolDescLog("Logical Volume Information 2: %.18ls\n", tmpW);
		ZeroMemory(tmpW, sizeof(tmpW));
	}
	else {
		OutputVolDescLog("Logical Volume Information 2: %.36" CHARWIDTH "s\n", &lpBuf[nOfs + 229]);
	}
	ConvUTF16toUTF8("\t", lpBuf[nOfs + 264], &lpBuf[nOfs + 265], 36, tmpW);
	if (lpBuf[nOfs + 264] == 16) {
		OutputVolDescLog("Logical Volume Information 3: %.18ls\n", tmpW);
	}
	else {
		OutputVolDescLog("Logical Volume Information 3: %.36" CHARWIDTH "s\n", &lpBuf[nOfs + 265]);
	}
	OutputVolDescLog("\t                                 Implemention ID\n");
	OutputFsRegid("\t\t                           ", lpBuf + 352);

	OutputVolDescLog("\t                              Implementation Use: ");
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
		"\tNext Volume Descriptor Sequence Extent\n"
		, MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19]))
	);
	OutputFsExtentDescriptor("\t\t", lpBuf + 20);
}

VOID OutputFsAnchorVolumeDescriptorPointer(
	LPBYTE lpBuf,
	PUDF pUdf
) {
	OutputVolDescLog("\t   Main Volume Descriptor Sequence Extent\n");
	OutputFsExtentDescriptor("\t\t                      ", lpBuf + 16);
	pUdf->uiPVDLen = MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19]));
	pUdf->uiPVDPos = MAKEUINT(MAKEWORD(lpBuf[20], lpBuf[21]), MAKEWORD(lpBuf[22], lpBuf[23]));
	OutputVolDescLog("\tReserve Volume Descriptor Sequence Extent\n");
	OutputFsExtentDescriptor("\t\t                      ", lpBuf + 24);
}

VOID OutputFsPrimaryVolumeDescriptorForUDF(
	LPBYTE lpBuf
) {
	OutputVolDescLog(
		"\t              Volume Descriptor Sequence Number: %u\n"
		"\t               Primary Volume Descriptor Number: %u\n"
		, MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19]))
		, MAKEUINT(MAKEWORD(lpBuf[20], lpBuf[21]), MAKEWORD(lpBuf[22], lpBuf[23]))
	);
	WCHAR tmpW[128] = {};
	ConvUTF16toUTF8("\t          ", lpBuf[24], &lpBuf[25], 31, tmpW);
	if (lpBuf[24] == 16) {
		OutputVolDescLog("Volume Identifier: %.15ls\n", tmpW);
		ZeroMemory(tmpW, sizeof(tmpW));
	}
	else {
		OutputVolDescLog("Volume Identifier: %.31" CHARWIDTH "s\n", &lpBuf[25]);
	}
	OutputVolDescLog(
		"\t                         Volume Sequence Number: %u\n"
		"\t                 Maximum Volume Sequence Number: %u\n"
		"\t                              Interchange Level: %u\n"
		"\t                      Maximum Interchange Level: %u\n"
		"\t                             Character Set List: %u\n"
		"\t                     Maximum Character Set List: %u\n"
		, MAKEWORD(lpBuf[56], lpBuf[57])
		, MAKEWORD(lpBuf[58], lpBuf[59])
		, MAKEWORD(lpBuf[60], lpBuf[61])
		, MAKEWORD(lpBuf[62], lpBuf[63])
		, MAKEUINT(MAKEWORD(lpBuf[64], lpBuf[65]), MAKEWORD(lpBuf[66], lpBuf[67]))
		, MAKEUINT(MAKEWORD(lpBuf[68], lpBuf[69]), MAKEWORD(lpBuf[70], lpBuf[71]))
	);
	ConvUTF16toUTF8("\t      ", lpBuf[72], &lpBuf[73], 127, tmpW);
	if (lpBuf[72] == 16) {
		OutputVolDescLog("Volume Set Identifier: %.63ls\n", tmpW);
	}
	else {
		OutputVolDescLog("Volume Set Identifier: %.127" CHARWIDTH "s\n", &lpBuf[73]);
	}
	OutputVolDescLog("\t                       Descriptor Character Set\n");
	OutputFsCharspec("\t\t                  ", lpBuf + 200);

	OutputVolDescLog("\t                      Explanatory Character Set\n");
	OutputFsCharspec("\t\t                  ", lpBuf + 264);

	OutputVolDescLog("\t                                Volume Abstract\n");
	OutputFsExtentDescriptor("\t                                ", lpBuf + 328);

	OutputVolDescLog("\t                        Volume Copyright Notice\n");
	OutputFsExtentDescriptor("\t\t                            ", lpBuf + 336);

	OutputVolDescLog("\t                         Application Identifier\n");
	OutputFsRegid("\t\t                          ", lpBuf + 344);

	OutputFsRecordingDateAndTime("\t                        Recording", lpBuf + 376);

	OutputVolDescLog("\t                      Implementation Identifier\n");
	OutputFsRegid("\t\t                          ", lpBuf + 388);

	OutputVolDescLog("\t                             Implementation Use: ");
	for (INT i = 420; i < 484; i++) {
		OutputVolDescLog("%02x", lpBuf[i]);
	}
	OutputVolDescLog(
		"\n\tPredecessor Volume Descriptor Sequence Location: %u\n"
		"\t                                          Flags: %u\n"
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
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Primary Volume Descriptor"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                      ", lpBuf);
		OutputFsPrimaryVolumeDescriptorForUDF(lpBuf);
		break;
	case 2:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Anchor Volume Descriptor Pointer"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                ", lpBuf);
		OutputFsAnchorVolumeDescriptorPointer(lpBuf, pUDF);
		break;
	case 3:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Volume Descriptor Pointer"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                           ", lpBuf);
		OutputFsVolumeDescriptorPointer(lpBuf);
		break;
	case 4:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Implementation Use Volume Descriptor"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                       ", lpBuf);
		OutputFsImplementationUseVolumeDescriptor(lpBuf);
		break;
	case 5:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Partition Descriptor"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t        ", lpBuf);
		OutputFsPartitionDescriptor(lpBuf, pUDF);
		break;
	case 6:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Logical Volume Descriptor"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                    ", lpBuf);
		OutputFsLogicalVolumeDescriptor(lpBuf, pUDF);
		break;
	case 7:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Unallocated Space Descriptor"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t        ", lpBuf);
		OutputFsUnallocatedSpaceDescriptor(lpBuf);
		break;
	case 8:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Terminating Descriptor"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t", lpBuf);
		OutputFsTerminatingDescriptor(lpBuf);
		break;
	case 9:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Logical Volume Integrity Descriptor"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t       ", lpBuf);
		OutputFsLogicalVolumeIntegrityDescriptor(lpBuf);
		break;
	case 256:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("File Set Descriptor"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                    ", lpBuf);
		OutputFsFileSetDescriptor(lpBuf, pUDF);
		break;
	case 257:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("File Identifier Descriptor"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                           ", lpBuf);
		OutputFsFileIdentifierDescriptor(lpBuf);
		break;
	case 258:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Allocation Extent Descriptor"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                           ", lpBuf);
		break;
	case 259:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Indirect Entry"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                           ", lpBuf);
		break;
	case 260:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Terminal Entry"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                           ", lpBuf);
		break;
	case 261:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("File Entry"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                  ", lpBuf);
		OutputFsFileEntry(lpBuf);
		break;
	case 262:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Extended Attribute Header Descriptor"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                           ", lpBuf);
		break;
	case 263:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Unallocated Space Entry"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                           ", lpBuf);
		break;
	case 264:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Space Bitmap Descriptor"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                           ", lpBuf);
		break;
	case 265:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Partition Integrity Entry"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                           ", lpBuf);
		break;
	case 266:
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Extended File Entry"), nLBA, (UINT)nLBA);
		OutputFsDescriptorTag("\t                           ", lpBuf);
		break;
	default:
		break;
	}
	OutputMainChannel(fileMainInfo, lpBuf, NULL, nLBA, DISC_MAIN_DATA_SIZE);
	return;
}
// End of ECMA-167
