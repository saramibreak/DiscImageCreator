/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "convert.h"
#include "get.h"
#include "output.h"
#include "outputScsiCmdLogforCD.h"

VOID OutputFsBootRecord(
	LPBYTE lpBuf
	)
{
	CHAR str[2][32] = { "0" };
	strncpy(str[0], (LPCH)&lpBuf[7], sizeof(str[0]));
	strncpy(str[1], (LPCH)&lpBuf[39], sizeof(str[1]));
	OutputVolDescLogA(
		"\t                       Boot System Identifier: %s\n"
		"\t                              Boot Identifier: %s\n"
		"\t                              Boot System Use: "
		, str[0], str[1]);
	for (INT i = 71; i < 2048; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputFsVolumeDescriptorFirst(
	LPBYTE lpBuf,
	CHAR str32[][32]
	)
{
	DWORD vss = GetSizeOrDwordForVolDesc(lpBuf + 80);
	OutputVolDescLogA(
		"\t                            System Identifier: %.32s\n"
		"\t                            Volume Identifier: %.32s\n"
		"\t                            Volume Space Size: %lu\n"
		, str32[0], str32[1], vss);
}

VOID OutputFsDirectoryRecord(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	DWORD dwExtentPos,
	DWORD dwDataLen,
	LPSTR fname
	)
{
	CHAR str[128] { 0 };
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
		strncat(str, "Disassociated, ", 15);
	}
	if (nFileFlag & 0x08) {
		strncat(str, "File has record format, ", 24);
	}
	else {
		strncat(str, "File has't record format, ", 26);
	}
	if (nFileFlag & 0x10) {
		strncat(str, "Owner/Group ID has, ", 20);
	}
	else {
		strncat(str, "Owner/Group ID has't, ", 22);
	}
	if (nFileFlag & 0x80) {
		strncat(str, "Next Directory Record has", 25);
	}
	else {
		strncat(str, "Final Directory Record", 22);
	}
	WORD vsn = GetSizeOrWordForVolDesc(lpBuf + 28);
	OutputVolDescLogA(
		"\t\t      Length of Directory Record: %u\n"
		"\t\tExtended Attribute Record Length: %u\n"
		"\t\t              Location of Extent: %lu\n"
		"\t\t                     Data Length: %lu\n"
		"\t\t         Recording Date and Time: %u-%02u-%02u %02u:%02u:%02u +%02u\n"
		"\t\t                      File Flags: %u (%s)\n"
		"\t\t                  File Unit Size: %u\n"
		"\t\t             Interleave Gap Size: %u\n"
		"\t\t          Volume Sequence Number: %u\n"
		"\t\t       Length of File Identifier: %u\n"
		"\t\t                 File Identifier: "
		, lpBuf[0], lpBuf[1], dwExtentPos, dwDataLen, lpBuf[18] + 1900, lpBuf[19]
		, lpBuf[20], lpBuf[21], lpBuf[22], lpBuf[23], lpBuf[24], lpBuf[25], str
		, lpBuf[26], lpBuf[27], vsn, lpBuf[32]);
	for (INT n = 0; n < lpBuf[32]; n++) {
		OutputVolDescLogA("%c", lpBuf[33 + n]);
		fname[n] = (CHAR)lpBuf[33 + n];
	}
	OutputVolDescLogA("\n");
	if (pExtArg->byReadContinue || pExtArg->byIntentionalSub) {
		if ((nFileFlag & 0x02) == 0 && pDisc->PROTECT.byExist) {
			if (pDisc->PROTECT.ERROR_SECTOR.nExtentPos < (INT)dwExtentPos) {
				if (pDisc->PROTECT.byTmpForSafeDisc) {
					pDisc->PROTECT.ERROR_SECTOR.nNextExtentPos = (INT)dwExtentPos;
					pDisc->PROTECT.byTmpForSafeDisc = FALSE;
				}
				if ((INT)dwExtentPos <= pDisc->PROTECT.ERROR_SECTOR.nNextExtentPos) {
					pDisc->PROTECT.ERROR_SECTOR.nSectorSize = (INT)dwExtentPos - pDisc->PROTECT.ERROR_SECTOR.nExtentPos - 1;
					pDisc->PROTECT.ERROR_SECTOR.nNextExtentPos = (INT)dwExtentPos;
				}
			}
		}
		if (!strncmp(fname, "CD.IDX", 6)) {
			pDisc->PROTECT.byExist = cdidx;
			strncpy(pDisc->PROTECT.name, fname, 6);
			pDisc->PROTECT.ERROR_SECTOR.nExtentPos = (INT)dwExtentPos;
			pDisc->PROTECT.ERROR_SECTOR.nSectorSize = (INT)(dwDataLen / DISC_RAW_READ_SIZE - 1);
		}
		else if (!strncmp(fname, "__CDS.exe", 9)) {
			pDisc->PROTECT.byExist = cds300;
			strncpy(pDisc->PROTECT.name, fname, 9);
		}
		else if (!strncmp(fname, "LASERLOK.IN", 11)) {
			pDisc->PROTECT.byExist = laserlock;
			strncpy(pDisc->PROTECT.name, fname, 11);
			pDisc->PROTECT.ERROR_SECTOR.nExtentPos = (INT)dwExtentPos;
			pDisc->PROTECT.ERROR_SECTOR.nSectorSize = (INT)(dwDataLen / DISC_RAW_READ_SIZE - 1);
		}
		else if (!strncmp(fname, "PROTECT.PRO", 11)) {
			pDisc->PROTECT.byExist = proring;
			strncpy(pDisc->PROTECT.name, fname, 11);
			pDisc->PROTECT.ERROR_SECTOR.nExtentPos = (INT)dwExtentPos;
			pDisc->PROTECT.ERROR_SECTOR.nSectorSize = (INT)(dwDataLen / DISC_RAW_READ_SIZE - 1);
		}
		else if (!strncmp(fname, "00000001.LT1", 12)) {
			pDisc->PROTECT.byExist = safeDiscLite;
			strncpy(pDisc->PROTECT.name, fname, 12);
			pDisc->PROTECT.byTmpForSafeDisc = TRUE;
			pDisc->PROTECT.ERROR_SECTOR.nExtentPos = (INT)(dwExtentPos + dwDataLen / DISC_RAW_READ_SIZE);
		}
		else if (!strncmp(fname, "00000001.TMP", 12)) {
			pDisc->PROTECT.byExist = safeDisc;
			strncpy(pDisc->PROTECT.name, fname, 12);
			pDisc->PROTECT.byTmpForSafeDisc = TRUE;
			pDisc->PROTECT.ERROR_SECTOR.nExtentPos = (INT)(dwExtentPos + dwDataLen / DISC_RAW_READ_SIZE);
		}
		else if (!strncmp(fname, "00002.tmp", 9)) {
			pDisc->PROTECT.byExist = smartE;
			strncpy(pDisc->PROTECT.name, fname, 9);
			pDisc->PROTECT.ERROR_SECTOR.nExtentPos = (INT)dwExtentPos;
			pDisc->PROTECT.ERROR_SECTOR.nSectorSize = (INT)(dwDataLen / DISC_RAW_READ_SIZE - 1);
		}
		else if (!strncmp(fname, "CMS16.DLL", 9) && pDisc->PROTECT.byExist == no) {
			pDisc->PROTECT.byExist = securomOld;
			strncpy(pDisc->PROTECT.name, fname, 9);
		}
		else if ((!strncmp(fname, "cms32_95.dll", 12) || !strncmp(fname, "CMS32_NT.DLL", 12))
			&& pDisc->PROTECT.byExist == no) {
			pDisc->PROTECT.byExist = securomOld;
			strncpy(pDisc->PROTECT.name, fname, 12);
		}
		else if (pDisc->PROTECT.byExist == no) {
			// for CodeLock, ProtectCD-VOB, SecuRomNEW
			LPCH p = strstr(fname, ".EXE");
			if (!p) {
				// Doesn't exist stristr in C/C++...
				p = strstr(fname, ".exe");
				if (!p) {
					p = strstr(fname, ".DLL");
					if (!p) {
						p = strstr(fname, ".dll");
					}
				}
			}
			if (p) {
				if (pDisc->PROTECT.nCntForExe == EXELBA_STORE_SIZE) {
					OutputLogA(standardError | fileMainError, "Reached MAX .exe num\n");
					return;
				}
				size_t len = (size_t)(p - fname + 4);
				pDisc->PROTECT.pExtentPosForExe[pDisc->PROTECT.nCntForExe] = (INT)dwExtentPos;
				strncpy(pDisc->PROTECT.pNameForExe[pDisc->PROTECT.nCntForExe], fname, len);
				pDisc->PROTECT.nCntForExe++;
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
	)
{
	WORD vss = GetSizeOrWordForVolDesc(lpBuf + 120);
	WORD vsn = GetSizeOrWordForVolDesc(lpBuf + 124);
	WORD lbs = GetSizeOrWordForVolDesc(lpBuf + 128);
	DWORD pts = GetSizeOrDwordForVolDesc(lpBuf + 132);
	DWORD lopt = MAKEDWORD(MAKEWORD(lpBuf[140], lpBuf[141]),
		MAKEWORD(lpBuf[142], lpBuf[143]));
	pDisc->MAIN.bPathType = lType;
	if (lopt == 0) {
		lopt = MAKEDWORD(MAKEWORD(lpBuf[151], lpBuf[150]),
			MAKEWORD(lpBuf[149], lpBuf[148]));
		pDisc->MAIN.bPathType = mType;
	}
	// for Codelock
	if (pExtArg->byReadContinue) {
		if (pDisc->PROTECT.nPrevLBAOfPathTablePos == 0) {
			pDisc->PROTECT.nPrevLBAOfPathTablePos = (INT)lopt - 1;
		}
	}
	DWORD loopt = MAKEDWORD(MAKEWORD(lpBuf[144], lpBuf[145]),
		MAKEWORD(lpBuf[146], lpBuf[147]));
	if (loopt == 0) {
		loopt = MAKEDWORD(MAKEWORD(lpBuf[155], lpBuf[154]),
			MAKEWORD(lpBuf[153], lpBuf[152]));
	}
	OutputVolDescLogA(
		"\t                              Volume Set Size: %u\n"
		"\t                       Volume Sequence Number: %u\n"
		"\t                           Logical Block Size: %u\n"
		"\t                              Path Table Size: %lu\n"
		"\t         Location of Occurrence of Path Table: %lu\n"
		"\tLocation of Optional Occurrence of Path Table: %lu\n"
		, vss, vsn, lbs, pts, lopt, loopt);

	DWORD dwExtentPos = GetSizeOrDwordForVolDesc(lpBuf + 158);
	DWORD dwDataLen = GetSizeOrDwordForVolDesc(lpBuf + 166);
	CHAR fname[64] = { 0 };
	OutputFsDirectoryRecord(pExtArg, pDisc, lpBuf + 156, dwExtentPos, dwDataLen, fname);
	if (bTCHAR) {
		OutputVolDescLogA(
			"\t                        Volume Set Identifier: %.64s\n"
			"\t                         Publisher Identifier: %.64s\n"
			"\t                     Data Preparer Identifier: %.64s\n"
			"\t                       Application Identifier: %.64s\n"
			"\t                    Copyright File Identifier: %.18s\n"
			"\t                     Abstract File Identifier: %.18s\n"
			"\t                Bibliographic File Identifier: %.18s\n"
			, str128[0], str128[1], str128[2], str128[3], str37[0], str37[1], str37[2]);
	}
	else {
		OutputVolDescLogA(
			"\t                        Volume Set Identifier: %.128s\n"
			"\t                         Publisher Identifier: %.128s\n"
			"\t                     Data Preparer Identifier: %.128s\n"
			"\t                       Application Identifier: %.128s\n"
			"\t                    Copyright File Identifier: %.37s\n"
			"\t                     Abstract File Identifier: %.37s\n"
			"\t                Bibliographic File Identifier: %.37s\n"
			, str128[0], str128[1], str128[2], str128[3], str37[0], str37[1], str37[2]);
	}
}

VOID OutputFsVolumeDescriptorForTime(
	LPBYTE lpBuf
	)
{
	CHAR year[4][4] = { "0" };
	CHAR month[4][2] = { "0" };
	CHAR day[4][2] = { "0" };
	CHAR hour[4][2] = { "0" };
	CHAR time[4][2] = { "0" };
	CHAR second[4][2] = { "0" };
	CHAR milisecond[4][2] = { "0" };
	strncpy(year[0], (LPCH)&lpBuf[813], sizeof(year[0]));
	strncpy(month[0], (LPCH)&lpBuf[817], sizeof(month[0]));
	strncpy(day[0], (LPCH)&lpBuf[819], sizeof(day[0]));
	strncpy(hour[0], (LPCH)&lpBuf[821], sizeof(hour[0]));
	strncpy(time[0], (LPCH)&lpBuf[823], sizeof(time[0]));
	strncpy(second[0], (LPCH)&lpBuf[825], sizeof(second[0]));
	strncpy(milisecond[0], (LPCH)&lpBuf[827], sizeof(milisecond[0]));
	strncpy(year[1], (LPCH)&lpBuf[830], sizeof(year[1]));
	strncpy(month[1], (LPCH)&lpBuf[834], sizeof(month[1]));
	strncpy(day[1], (LPCH)&lpBuf[836], sizeof(day[1]));
	strncpy(hour[1], (LPCH)&lpBuf[838], sizeof(hour[1]));
	strncpy(time[1], (LPCH)&lpBuf[840], sizeof(time[1]));
	strncpy(second[1], (LPCH)&lpBuf[842], sizeof(second[1]));
	strncpy(milisecond[1], (LPCH)&lpBuf[844], sizeof(milisecond[1]));
	strncpy(year[2], (LPCH)&lpBuf[847], sizeof(year[2]));
	strncpy(month[2], (LPCH)&lpBuf[851], sizeof(month[2]));
	strncpy(day[2], (LPCH)&lpBuf[853], sizeof(day[2]));
	strncpy(hour[2], (LPCH)&lpBuf[855], sizeof(hour[2]));
	strncpy(time[2], (LPCH)&lpBuf[857], sizeof(time[2]));
	strncpy(second[2], (LPCH)&lpBuf[859], sizeof(second[2]));
	strncpy(milisecond[2], (LPCH)&lpBuf[861], sizeof(milisecond[2]));
	strncpy(year[3], (LPCH)&lpBuf[864], sizeof(year[3]));
	strncpy(month[3], (LPCH)&lpBuf[868], sizeof(month[3]));
	strncpy(day[3], (LPCH)&lpBuf[870], sizeof(day[3]));
	strncpy(hour[3], (LPCH)&lpBuf[872], sizeof(hour[3]));
	strncpy(time[3], (LPCH)&lpBuf[874], sizeof(time[3]));
	strncpy(second[3], (LPCH)&lpBuf[876], sizeof(second[3]));
	strncpy(milisecond[3], (LPCH)&lpBuf[878], sizeof(milisecond[3]));
	OutputVolDescLogA(
		"\t                Volume Creation Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%u\n"
		"\t            Volume Modification Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%u\n"
		"\t              Volume Expiration Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%u\n"
		"\t               Volume Effective Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%u\n"
		"\t                       File Structure Version: %u\n"
		"\t                              Application Use: ",
		year[0], month[0], day[0], hour[0], time[0], second[0], milisecond[0], lpBuf[829],
		year[1], month[1], day[1], hour[1], time[1], second[1], milisecond[1], lpBuf[846],
		year[2], month[2], day[2], hour[2], time[2], second[2], milisecond[2], lpBuf[863],
		year[3], month[3], day[3], hour[3], time[3], second[3], milisecond[3], lpBuf[880],
		lpBuf[881]);
	for (INT i = 883; i <= 1394; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputFsVolumeDescriptorForISO9660(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf
	)
{
	CHAR str32[3][32] = { 0 };
	CHAR str128[4][128] = { 0 };
	CHAR str37[3][37] = { 0 };
	strncpy(str32[0], (LPCH)&lpBuf[8], sizeof(str32[0]));
	strncpy(str32[1], (LPCH)&lpBuf[40], sizeof(str32[1]));
	strncpy(str128[0], (LPCH)&lpBuf[190], sizeof(str128[0]));
	strncpy(str128[1], (LPCH)&lpBuf[318], sizeof(str128[1]));
	strncpy(str128[2], (LPCH)&lpBuf[446], sizeof(str128[2]));
	strncpy(str128[3], (LPCH)&lpBuf[574], sizeof(str128[3]));
	strncpy(str37[0], (LPCH)&lpBuf[702], sizeof(str37[0]));
	strncpy(str37[1], (LPCH)&lpBuf[739], sizeof(str37[1]));
	strncpy(str37[2], (LPCH)&lpBuf[776], sizeof(str37[2]));
	OutputFsVolumeDescriptorFirst(lpBuf, str32);
	if (lpBuf[0] == 2) {
		strncpy(str32[2], (LPCH)&lpBuf[88], sizeof(str32[2]));
		OutputVolDescLogA(
			"\t                             Escape Sequences: %.32s\n", str32[2]);
	}
	OutputFsVolumeDescriptorSecond(pExtArg, pDisc, lpBuf, str128, str37, FALSE);
	OutputFsVolumeDescriptorForTime(lpBuf);
}

VOID OutputFsVolumeDescriptorForJoliet(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf
	)
{
	CHAR str32[3][32] = { 0 };
	CHAR str128[4][128] = { 0 };
	CHAR str37[3][37] = { 0 };
	WCHAR tmp16[3][16] = { 0 };
	WCHAR tmp64[4][64] = { 0 };
	WCHAR tmp18[3][18] = { 0 };
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
		strncpy(str32[0], (LPCH)&lpBuf[8], sizeof(str32[0]));
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

	OutputFsVolumeDescriptorFirst(lpBuf, str32);

	strncpy(str32[2], (LPCH)&lpBuf[88], sizeof(str32[2]));
	OutputVolDescLogA(
		"\t                             Escape Sequences: %.32s\n", str32[2]);
	OutputFsVolumeDescriptorSecond(pExtArg, pDisc, lpBuf, str128, str37, TRUE);
	OutputFsVolumeDescriptorForTime(lpBuf);
}

VOID OutputFsVolumePartitionDescriptor(
	LPBYTE lpBuf
	)
{
	CHAR str[2][32] = { 0 };
	strncpy(str[0], (LPCH)&lpBuf[8], sizeof(str[0]));
	strncpy(str[1], (LPCH)&lpBuf[40], sizeof(str[1]));
	OutputVolDescLogA(
		"\t          System Identifier: %.32s\n"
		"\tVolume Partition Identifier: %.32s\n"
		"\t  Volume Partition Location: %lu\n"
		"\t      Volume Partition Size: %lu\n"
		"\t                 System Use: ",
		str[0],
		str[1],
		MAKELONG(MAKEWORD(lpBuf[76], lpBuf[77]),
		MAKEWORD(lpBuf[78], lpBuf[79])),
		MAKELONG(MAKEWORD(lpBuf[84], lpBuf[85]),
		MAKEWORD(lpBuf[86], lpBuf[87])));
	for (INT i = 88; i < 2048; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputFsVolumeDescriptor(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nLBA
	)
{
	// 0 is Boot Record.
	// 1 is Primary Volume Descriptor.
	// 2 is Supplementary Volume Descriptor.
	// 3 is Volume Partition Descriptor.
	// 4-254 is reserved.
	// 255 is Volume Descriptor Set Terminator.
	OutputVolDescWithLBALogA(Volume Descriptor,
		"\t                       Volume Descriptor Type: %u\n"
		"\t                          Standard Identifier: %.5s\n"
		"\t                    Volume Descriptor Version: %u\n"
		, nLBA, lpBuf[0], (LPCH)&lpBuf[1], lpBuf[6]);

	if (lpBuf[0] == 0) {
		OutputFsBootRecord(lpBuf);
	}
	else if (lpBuf[0] == 1) {
		OutputFsVolumeDescriptorForISO9660(pExtArg, pDisc, lpBuf);
	}
	else if (lpBuf[0] == 2) {
		OutputFsVolumeDescriptorForJoliet(pExtArg, pDisc, lpBuf);
	}
	else if (lpBuf[0] == 3) {
		OutputFsVolumePartitionDescriptor(lpBuf);
	}
	else if (lpBuf[0] == 255) {
		if (pExtArg->byReadContinue) {
			pDisc->PROTECT.nNextLBAOfLastVolDesc = nLBA + 1;
		}
	}
}

VOID OutputFsPathTableRecord(
	PDISC pDisc,
	LPBYTE lpBuf,
	DWORD dwPathTblPos,
	DWORD dwPathTblSize,
	PDIRECTORY_RECORD pDirRec,
	LPINT nDirPosNum
	)
{
	OutputVolDescLogA(
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Path Table Record), (INT)dwPathTblPos, (INT)dwPathTblPos);
	for (DWORD i = 0; i < dwPathTblSize;) {
		pDirRec[*nDirPosNum].uiDirNameLen = lpBuf[i];
		if (pDisc->MAIN.bPathType == lType) {
			pDirRec[*nDirPosNum].uiPosOfDir = MAKEDWORD(MAKEWORD(lpBuf[2 + i], lpBuf[3 + i]),
				MAKEWORD(lpBuf[4 + i], lpBuf[5 + i]));
		}
		else {
			pDirRec[*nDirPosNum].uiPosOfDir = MAKEDWORD(MAKEWORD(lpBuf[5 + i], lpBuf[4 + i]),
				MAKEWORD(lpBuf[3 + i], lpBuf[2 + i]));
		}
		if (pDirRec[*nDirPosNum].uiDirNameLen > 0) {
			if (pDisc->MAIN.bPathType == lType) {
				pDirRec[*nDirPosNum].uiNumOfUpperDir = MAKEWORD(lpBuf[6 + i], lpBuf[7 + i]);
			}
			else {
				pDirRec[*nDirPosNum].uiNumOfUpperDir = MAKEWORD(lpBuf[7 + i], lpBuf[6 + i]);
			}
			OutputVolDescLogA(
				"\t     Length of Directory Identifier: %u\n"
				"\tLength of Extended Attribute Record: %u\n"
				"\t                 Position of Extent: %u\n"
				"\t          Number of Upper Directory: %u\n"
				"\t               Directory Identifier: "
				, pDirRec[*nDirPosNum].uiDirNameLen, lpBuf[1 + i]
				, pDirRec[*nDirPosNum].uiPosOfDir, pDirRec[*nDirPosNum].uiNumOfUpperDir);
			for (size_t n = 0; n < pDirRec[*nDirPosNum].uiDirNameLen; n++) {
				OutputVolDescLogA("%c", lpBuf[8 + i + n]);
				pDirRec[*nDirPosNum].szDirName[n] = (CHAR)lpBuf[8 + i + n];
			}
			OutputVolDescLogA("\n\n");

			i += 8 + pDirRec[*nDirPosNum].uiDirNameLen;
			if ((i % 2) != 0) {
				i++;
			}
			*nDirPosNum = *nDirPosNum + 1;
		}
	}
}

// http://www.dubeyko.com/development/FileSystems/HFS/hfs_ondisk_layout/Files-102.html
VOID OutputFsMasterDirectoryBlocks(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	CHAR str[27] = { 0 };
	strncpy(str, (LPCH)&lpBuf[36], sizeof(str));
	OutputVolDescWithLBALogA(Master Directory Blocks,
		"\t                       volume signature: %04x\n"
		"\t       date and time of volume creation: %08lx\n"
		"\t     date and time of last modification: %08lx\n"
		"\t                      volume attributes: %04x\n"
		"\t      number of files in root directory: %04x\n"
		"\t           first block of volume bitmap: %04x\n"
		"\t        start of next allocation search: %04x\n"
		"\t  number of allocation blocks in volume: %04x\n"
		"\t   size (in bytes) of allocation blocks: %08lx\n"
		"\t                     default clump size: %08lx\n"
		"\t       first allocation block in volume: %04x\n"
		"\t            next unused catalog node ID: %08lx\n"
		"\t     number of unused allocation blocks: %04x\n"
		"\t                            volume name: %.27s\n"
		"\t           date and time of last backup: %08lx\n"
		"\t          volume backup sequence number: %04x\n"
		"\t                     volume write count: %08lx\n"
		"\t   clump size for extents overflow file: %08lx\n"
		"\t            clump size for catalog file: %08lx\n"
		"\tnumber of directories in root directory: %04x\n"
		"\t              number of files in volume: %08lx\n"
		"\t        number of directories in volume: %08lx\n"
		"\t         information used by the Finder: %08lx\n"
		"\t         information used by the Finder: %08lx\n"
		"\t         information used by the Finder: %08lx\n"
		"\t         information used by the Finder: %08lx\n"
		"\t         information used by the Finder: %08lx\n"
		"\t         information used by the Finder: %08lx\n"
		"\t         information used by the Finder: %08lx\n"
		"\t         information used by the Finder: %08lx\n"
		"\t       size (in blocks) of volume cache: %04x\n"
		"\tsize (in blocks) of volume bitmap cache: %04x\n"
		"\tsize (in blocks) of common volume cache: %04x\n"
		"\t          size of extents overflow file: %08lx\n"
		"\textent record for extents overflow file: %08lx%08lx%08lx\n"
		"\t                   size of catalog file: %08lx\n"
		"\t         extent record for catalog file: %08lx%08lx%08lx\n"
		, nLBA, MAKEWORD(lpBuf[1], lpBuf[0]),
		MAKELONG(MAKEWORD(lpBuf[5], lpBuf[4]), MAKEWORD(lpBuf[3], lpBuf[2])),
		MAKELONG(MAKEWORD(lpBuf[9], lpBuf[8]), MAKEWORD(lpBuf[7], lpBuf[6])),
		MAKEWORD(lpBuf[11], lpBuf[10]),
		MAKEWORD(lpBuf[13], lpBuf[12]),
		MAKEWORD(lpBuf[15], lpBuf[14]),
		MAKEWORD(lpBuf[17], lpBuf[16]),
		MAKEWORD(lpBuf[19], lpBuf[18]),
		MAKELONG(MAKEWORD(lpBuf[23], lpBuf[22]), MAKEWORD(lpBuf[21], lpBuf[20])),
		MAKELONG(MAKEWORD(lpBuf[27], lpBuf[26]), MAKEWORD(lpBuf[25], lpBuf[24])),
		MAKEWORD(lpBuf[29], lpBuf[28]),
		MAKELONG(MAKEWORD(lpBuf[33], lpBuf[32]), MAKEWORD(lpBuf[31], lpBuf[30])),
		MAKEWORD(lpBuf[35], lpBuf[34]),
		str,
		MAKELONG(MAKEWORD(lpBuf[66], lpBuf[65]), MAKEWORD(lpBuf[64], lpBuf[63])),
		MAKEWORD(lpBuf[68], lpBuf[67]),
		MAKELONG(MAKEWORD(lpBuf[72], lpBuf[71]), MAKEWORD(lpBuf[70], lpBuf[69])),
		MAKELONG(MAKEWORD(lpBuf[76], lpBuf[75]), MAKEWORD(lpBuf[74], lpBuf[73])),
		MAKELONG(MAKEWORD(lpBuf[80], lpBuf[79]), MAKEWORD(lpBuf[78], lpBuf[77])),
		MAKEWORD(lpBuf[82], lpBuf[81]),
		MAKELONG(MAKEWORD(lpBuf[86], lpBuf[85]), MAKEWORD(lpBuf[84], lpBuf[83])),
		MAKELONG(MAKEWORD(lpBuf[90], lpBuf[89]), MAKEWORD(lpBuf[88], lpBuf[87])),
		MAKELONG(MAKEWORD(lpBuf[94], lpBuf[93]), MAKEWORD(lpBuf[92], lpBuf[91])),
		MAKELONG(MAKEWORD(lpBuf[98], lpBuf[97]), MAKEWORD(lpBuf[96], lpBuf[95])),
		MAKELONG(MAKEWORD(lpBuf[102], lpBuf[101]), MAKEWORD(lpBuf[100], lpBuf[99])),
		MAKELONG(MAKEWORD(lpBuf[106], lpBuf[105]), MAKEWORD(lpBuf[104], lpBuf[103])),
		MAKELONG(MAKEWORD(lpBuf[110], lpBuf[109]), MAKEWORD(lpBuf[108], lpBuf[107])),
		MAKELONG(MAKEWORD(lpBuf[114], lpBuf[113]), MAKEWORD(lpBuf[112], lpBuf[111])),
		MAKELONG(MAKEWORD(lpBuf[118], lpBuf[117]), MAKEWORD(lpBuf[116], lpBuf[115])),
		MAKELONG(MAKEWORD(lpBuf[122], lpBuf[121]), MAKEWORD(lpBuf[120], lpBuf[119])),
		MAKEWORD(lpBuf[124], lpBuf[123]),
		MAKEWORD(lpBuf[126], lpBuf[125]),
		MAKEWORD(lpBuf[128], lpBuf[127]),
		MAKELONG(MAKEWORD(lpBuf[132], lpBuf[131]), MAKEWORD(lpBuf[130], lpBuf[129])),
		MAKELONG(MAKEWORD(lpBuf[136], lpBuf[135]), MAKEWORD(lpBuf[134], lpBuf[133])),
		MAKELONG(MAKEWORD(lpBuf[140], lpBuf[139]), MAKEWORD(lpBuf[138], lpBuf[137])),
		MAKELONG(MAKEWORD(lpBuf[144], lpBuf[143]), MAKEWORD(lpBuf[142], lpBuf[141])),
		MAKELONG(MAKEWORD(lpBuf[148], lpBuf[147]), MAKEWORD(lpBuf[146], lpBuf[145])),
		MAKELONG(MAKEWORD(lpBuf[152], lpBuf[151]), MAKEWORD(lpBuf[150], lpBuf[149])),
		MAKELONG(MAKEWORD(lpBuf[156], lpBuf[155]), MAKEWORD(lpBuf[154], lpBuf[153])),
		MAKELONG(MAKEWORD(lpBuf[160], lpBuf[159]), MAKEWORD(lpBuf[158], lpBuf[157])));
}

VOID OutputFs3doHeader(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputVolDescWithLBALogA(3DO Header,
		"\t                Record Type: %#04x\n"
		"\t                 Sync Bytes: %#04x %#04x %#04x %#04x %#04x\n"
		"\t             Record Version: %#04x\n"
		"\t               Volume Flags: %#04x\n"
		"\t             Volume Comment: "
		, nLBA, lpBuf[0], lpBuf[1], lpBuf[2], lpBuf[3]
		, lpBuf[4], lpBuf[5], lpBuf[6],	lpBuf[7]);
	for (INT i = 0; i < 32; i++) {
		OutputVolDescLogA("%c", lpBuf[8 + i]);
	}
	OutputVolDescLogA(
		"\n"
		"\t               Volume Label: ");
	for (INT i = 0; i < 32; i++) {
		OutputVolDescLogA("%c", lpBuf[40 + i]);
	}
	LONG dwNumOfCopy =
		MAKELONG(MAKEWORD(lpBuf[99], lpBuf[98]), MAKEWORD(lpBuf[97], lpBuf[96]));
	OutputVolDescLogA(
		"\n"
		"\t                  Volume ID: %#10lx\n"
		"\t         Logical Block Size: %lu\n"
		"\t          Volume Space Size: %lu + 152\n"
		"\t                Root Dir ID: %#10lx\n"
		"\t            Root Dir Blocks: %lu\n"
		"\t        Root Dir Block Size: %lu\n"
		"\tNum of Pos of Root Dir Copy: %lu\n"
		"\t            Pos of Root Dir: %lu\n",
		MAKELONG(MAKEWORD(lpBuf[75], lpBuf[74]), MAKEWORD(lpBuf[73], lpBuf[72])),
		MAKELONG(MAKEWORD(lpBuf[79], lpBuf[78]), MAKEWORD(lpBuf[77], lpBuf[76])),
		MAKELONG(MAKEWORD(lpBuf[83], lpBuf[82]), MAKEWORD(lpBuf[81], lpBuf[80])),
		MAKELONG(MAKEWORD(lpBuf[87], lpBuf[86]), MAKEWORD(lpBuf[85], lpBuf[84])),
		MAKELONG(MAKEWORD(lpBuf[91], lpBuf[90]), MAKEWORD(lpBuf[89], lpBuf[88])),
		MAKELONG(MAKEWORD(lpBuf[95], lpBuf[94]), MAKEWORD(lpBuf[93], lpBuf[92])),
		dwNumOfCopy,
		MAKELONG(MAKEWORD(lpBuf[103], lpBuf[102]), MAKEWORD(lpBuf[101], lpBuf[100])));

	for (LONG i = 0; i < dwNumOfCopy; i++) {
		OutputVolDescLogA(
			"\t       Pos of Root Dir Copy: %lu\n",
			MAKELONG(MAKEWORD(lpBuf[107 + i * 4], lpBuf[106 + i * 4]),
			MAKEWORD(lpBuf[105 + i * 4], lpBuf[104 + i * 4])));
	}
}

VOID OutputFs3doDirectoryRecord(
	LPBYTE lpBuf,
	INT nLBA,
	LPCH pPath,
	LONG lDirSize
	)
{
	OutputVolDescWithLBALogA(Directory Record, 
		"\tcurrentDir: %s\n"
		"\t========== Directory Header ==========\n"
		"\t      nextBlock: %#08lx\n"
		"\t      prevBlock: %#08lx\n"
		"\t          flags: %lu\n"
		"\t  directorySize: %lu\n"
		"\tdirectoryOffset: %lu\n"
		, nLBA, pPath
		, MAKELONG(MAKEWORD(lpBuf[3], lpBuf[2]), MAKEWORD(lpBuf[1], lpBuf[0]))
		, MAKELONG(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], lpBuf[4]))
		, MAKELONG(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], lpBuf[8]))
		, lDirSize
		, MAKELONG(MAKEWORD(lpBuf[19], lpBuf[18]), MAKEWORD(lpBuf[17], lpBuf[16])));

	LONG cur = THREEDO_DIR_HEADER_SIZE;
	LONG lastCopy = 0;
	CHAR fname[32] = { 0 };
	while (cur < lDirSize) {
		LPBYTE dirEnt = lpBuf + cur;
		strncpy(fname, (LPCH)&dirEnt[32], sizeof(fname));
		lastCopy = MAKELONG(MAKEWORD(dirEnt[67], dirEnt[66]), MAKEWORD(dirEnt[65], dirEnt[64]));
		cur += THREEDO_DIR_ENTRY_SIZE;
		OutputVolDescLogA(
			"\t========== Directory Entry ==========\n"
			"\t            flags: %#010lx\n"
			"\t               id: %#08lx\n"
			"\t              ext: %c%c%c%c\n"
			"\t        blockSize: %lu\n"
			"\t entryLengthBytes: %lu\n"
			"\tentryLengthBlocks: %lu\n"
			"\t            burst: %lu\n"
			"\t              gap: %lu\n"
			"\t         fileName: %s\n"
			"\t         copy num: %lu\n"
			"\t         data pos: %lu\n"
			, MAKELONG(MAKEWORD(dirEnt[3], dirEnt[2]), MAKEWORD(dirEnt[1], dirEnt[0]))
			, MAKELONG(MAKEWORD(dirEnt[7], dirEnt[6]), MAKEWORD(dirEnt[5], dirEnt[4]))
			, dirEnt[8], dirEnt[9], dirEnt[10], dirEnt[11]
			, MAKELONG(MAKEWORD(dirEnt[15], dirEnt[14]), MAKEWORD(dirEnt[13], dirEnt[12]))
			, MAKELONG(MAKEWORD(dirEnt[19], dirEnt[18]), MAKEWORD(dirEnt[17], dirEnt[16]))
			, MAKELONG(MAKEWORD(dirEnt[23], dirEnt[22]), MAKEWORD(dirEnt[21], dirEnt[20]))
			, MAKELONG(MAKEWORD(dirEnt[27], dirEnt[26]), MAKEWORD(dirEnt[25], dirEnt[24]))
			, MAKELONG(MAKEWORD(dirEnt[31], dirEnt[30]), MAKEWORD(dirEnt[29], dirEnt[28]))
			, fname, lastCopy
			, MAKELONG(MAKEWORD(dirEnt[71], dirEnt[70]), MAKEWORD(dirEnt[69], dirEnt[68])));
		for (LONG i = 0; i < lastCopy; i++) {
			LPBYTE pCopyPos = lpBuf + cur + sizeof(LONG) * i;
			OutputVolDescLogA("\t    data copy pos: %lu\n"
				, MAKELONG(MAKEWORD(pCopyPos[3], pCopyPos[2]), MAKEWORD(pCopyPos[1], pCopyPos[0])));
			cur += sizeof(LONG);
		}
	}
}

VOID OutputFsPceStuff(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputVolDescLogA(
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(PCE Warning msg & all stuff), nLBA, nLBA);
	CHAR str[39] = { 0 };
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
			OutputVolDescLogA("\t%s\n", str);
		}
	}
}

VOID OutputFsPceBootSector(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	CHAR str[24] = { 0 };
	strncpy(str, (LPCH)&lpBuf[32], sizeof(str));
	CHAR str2[50] = { 0 };
	strncpy(str2, (LPCH)&lpBuf[56], sizeof(str2));
	CHAR str3[17] = { 0 };
	strncpy(str3, (LPCH)&lpBuf[106], sizeof(str3) - 1);
	CHAR str4[7] = { 0 };
	strncpy(str4, (LPCH)&lpBuf[122], sizeof(str4) - 1);
	OutputVolDescWithLBALogA(PCE Boot Sector, 
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
		"\t                           system: %s\n"
		"\t                        copyright: %s\n"
		"\t                     program name: %s\n"
		"\t                         reserved: %s\n"
		, nLBA, lpBuf[0], lpBuf[1], lpBuf[2], lpBuf[3],	MAKEWORD(lpBuf[4], lpBuf[5]),
		MAKEWORD(lpBuf[6], lpBuf[7]), lpBuf[8], lpBuf[9], lpBuf[10], lpBuf[11], lpBuf[12],
		lpBuf[13], lpBuf[14], lpBuf[15], lpBuf[16],	lpBuf[17], MAKEWORD(lpBuf[18], lpBuf[19]),
		lpBuf[20], lpBuf[21], lpBuf[22], lpBuf[23], lpBuf[24],
		lpBuf[25], lpBuf[26], lpBuf[27], lpBuf[28], lpBuf[29], lpBuf[30], lpBuf[31],
		str, str2, str3, str4
		);
}

VOID OutputFsPcfxHeader(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputVolDescWithLBALogA(PCFX Warning msg, "\t%s\n", nLBA, (LPCH)&lpBuf[16]);
}

VOID OutputFsPcfxSector(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputVolDescWithLBALogA(PCFX Title Maker YMD,
		"\t       Title: %s\n"
		"\tMaker(short): %s\n"
		"\t Maker(long): %s\n"
		"\t         YMD: %s\n"
		, nLBA, (LPCH)&lpBuf[0], (LPCH)&lpBuf[48], (LPCH)&lpBuf[52], (LPCH)&lpBuf[120]
		);
}

VOID OutputFsImageDosHeader(
	PIMAGE_DOS_HEADER pIdh
	)
{
	OutputVolDescLogA(
		"\t========== Image Dos Header (%u byte) ==========\n"
		"\t                     Magic number: %04x\n"
		"\t       Bytes on last page of file: %04x\n"
		"\t                    Pages in file: %04x\n"
		"\t                      Relocations: %04x\n"
		"\t     Size of header in paragraphs: %04x\n"
		"\t  Minimum extra paragraphs needed: %04x\n"
		"\t  Maximum extra paragraphs needed: %04x\n"
		"\t      Initial (relative) SS value: %04x\n"
		"\t                 Initial SP value: %04x\n"
		"\t                         Checksum: %04x\n"
		"\t                 Initial IP value: %04x\n"
		"\t      Initial (relative) CS value: %04x\n"
		"\t File address of relocation table: %04x\n"
		"\t                   Overlay number: %04x\n"
		"\t   OEM identifier (for e_oeminfo): %04x\n"
		"\tOEM information; e_oemid specific: %04x\n"
		"\t   File address of new exe header: %08lx\n"
		, sizeof(IMAGE_DOS_HEADER)
		, pIdh->e_magic, pIdh->e_cblp, pIdh->e_cp, pIdh->e_crlc, pIdh->e_cparhdr
		, pIdh->e_minalloc, pIdh->e_maxalloc, pIdh->e_ss, pIdh->e_sp, pIdh->e_csum
		, pIdh->e_ip, pIdh->e_cs, pIdh->e_lfarlc, pIdh->e_ovno, pIdh->e_oemid
		, pIdh->e_oeminfo, pIdh->e_lfanew
		);
}

VOID OutputFsImageOS2Header(
	PIMAGE_OS2_HEADER pIoh
	)
{
	OutputVolDescLogA(
		"\t========== Image OS/2 .EXE header (%u byte) ==========\n"
		"\t                      Magic number: %04x\n"
		"\t                    Version number: %02x\n"
		"\t                   Revision number: %02x\n"
		"\t             Offset of Entry Table: %04x\n"
		"\t    Number of bytes in Entry Table: %04x\n"
		"\t            Checksum of whole file: %08lx\n"
		"\t                         Flag word: %04x\n"
		"\t     Automatic data segment number: %04x\n"
		"\t           Initial heap allocation: %04x\n"
		"\t          Initial stack allocation: %04x\n"
		"\t             Initial CS:IP setting: %08lx\n"
		"\t             Initial SS:SP setting: %08lx\n"
		"\t            Count of file segments: %04x\n"
		"\t Entries in Module Reference Table: %04x\n"
		"\t   Size of non-resident name table: %04x\n"
		"\t           Offset of Segment Table: %04x\n"
		"\t          Offset of Resource Table: %04x\n"
		"\t     Offset of resident name table: %04x\n"
		"\t  Offset of Module Reference Table: %04x\n"
		"\t    Offset of Imported Names Table: %04x\n"
		"\tOffset of Non-resident Names Table: %08lx\n"
		"\t          Count of movable entries: %04x\n"
		"\t     Segment alignment shift count: %04x\n"
		"\t        Count of resource segments: %04x\n"
		"\t           Target Operating system: %02x\n"
		"\t                  Other .EXE flags: %02x\n"
		"\t           offset to return thunks: %04x\n"
		"\t      offset to segment ref. bytes: %04x\n"
		"\t       Minimum code swap area size: %04x\n"
		"\t   Expected Windows version number: %04x\n"
		, sizeof(IMAGE_OS2_HEADER)
		, pIoh->ne_magic, pIoh->ne_ver, pIoh->ne_rev, pIoh->ne_enttab, pIoh->ne_cbenttab
		, pIoh->ne_crc, pIoh->ne_flags, pIoh->ne_autodata, pIoh->ne_heap, pIoh->ne_stack
		, pIoh->ne_csip, pIoh->ne_sssp, pIoh->ne_cseg, pIoh->ne_cmod, pIoh->ne_cbnrestab
		, pIoh->ne_segtab, pIoh->ne_rsrctab, pIoh->ne_restab, pIoh->ne_modtab
		, pIoh->ne_imptab, pIoh->ne_nrestab, pIoh->ne_cmovent, pIoh->ne_align
		, pIoh->ne_cres, pIoh->ne_exetyp, pIoh->ne_flagsothers, pIoh->ne_pretthunks
		, pIoh->ne_psegrefbytes, pIoh->ne_swaparea, pIoh->ne_expver
		);
}

VOID OutputFsImageNtHeader(
	PIMAGE_NT_HEADERS32 pInh
	)
{
	OutputVolDescLogA(
		"\t========== Image NT Header (%u byte) ==========\n"
		"\tSignature: %08lx\n"
		"\t========== FileHeader ==========\n"
		"\t\t             Machine: %04x\n"
		"\t\t    NumberOfSections: %04x\n"
		"\t\t       TimeDateStamp: %08lx\n"
		"\t\tPointerToSymbolTable: %08lx\n"
		"\t\t     NumberOfSymbols: %08lx\n"
		"\t\tSizeOfOptionalHeader: %04x\n"
		"\t\t     Characteristics: %04x\n"
		"\t========== OptionalHeader ==========\n"
		"\t\t                      Magic: %04x\n"
		"\t\t         MajorLinkerVersion: %02x\n"
		"\t\t         MinorLinkerVersion: %02x\n"
		"\t\t                 SizeOfCode: %08lx\n"
		"\t\t      SizeOfInitializedData: %08lx\n"
		"\t\t    SizeOfUninitializedData: %08lx\n"
		"\t\t        AddressOfEntryPoint: %08lx\n"
		"\t\t                 BaseOfCode: %08lx\n"
		"\t\t                 BaseOfData: %08lx\n"
		"\t\t                  ImageBase: %08lx\n"
		"\t\t           SectionAlignment: %08lx\n"
		"\t\t              FileAlignment: %08lx\n"
		"\t\tMajorOperatingSystemVersion: %04x\n"
		"\t\tMinorOperatingSystemVersion: %04x\n"
		"\t\t          MajorImageVersion: %04x\n"
		"\t\t          MinorImageVersion: %04x\n"
		"\t\t      MajorSubsystemVersion: %04x\n"
		"\t\t      MinorSubsystemVersion: %04x\n"
		"\t\t          Win32VersionValue: %08lx\n"
		"\t\t                SizeOfImage: %08lx\n"
		"\t\t              SizeOfHeaders: %08lx\n"
		"\t\t                   CheckSum: %08lx\n"
		"\t\t                  Subsystem: %04x\n"
		"\t\t         DllCharacteristics: %04x\n"
		"\t\t         SizeOfStackReserve: %08lx\n"
		"\t\t          SizeOfStackCommit: %08lx\n"
		"\t\t          SizeOfHeapReserve: %08lx\n"
		"\t\t           SizeOfHeapCommit: %08lx\n"
		"\t\t                LoaderFlags: %08lx\n"
		"\t\t        NumberOfRvaAndSizes: %08lx\n"
		"\t\t              DataDirectory:\n"
		, sizeof(IMAGE_NT_HEADERS32)
		, pInh->Signature, pInh->FileHeader.Machine, pInh->FileHeader.NumberOfSections
		, pInh->FileHeader.TimeDateStamp, pInh->FileHeader.PointerToSymbolTable
		, pInh->FileHeader.NumberOfSymbols, pInh->FileHeader.SizeOfOptionalHeader
		, pInh->FileHeader.Characteristics
		, pInh->OptionalHeader.Magic, pInh->OptionalHeader.MajorLinkerVersion
		, pInh->OptionalHeader.MinorLinkerVersion, pInh->OptionalHeader.SizeOfCode
		, pInh->OptionalHeader.SizeOfInitializedData, pInh->OptionalHeader.SizeOfUninitializedData
		, pInh->OptionalHeader.AddressOfEntryPoint, pInh->OptionalHeader.BaseOfCode
		, pInh->OptionalHeader.BaseOfData, pInh->OptionalHeader.ImageBase
		, pInh->OptionalHeader.SectionAlignment, pInh->OptionalHeader.FileAlignment
		, pInh->OptionalHeader.MajorOperatingSystemVersion, pInh->OptionalHeader.MinorOperatingSystemVersion
		, pInh->OptionalHeader.MajorImageVersion, pInh->OptionalHeader.MinorImageVersion
		, pInh->OptionalHeader.MajorSubsystemVersion, pInh->OptionalHeader.MinorSubsystemVersion
		, pInh->OptionalHeader.Win32VersionValue, pInh->OptionalHeader.SizeOfImage
		, pInh->OptionalHeader.SizeOfHeaders, pInh->OptionalHeader.CheckSum
		, pInh->OptionalHeader.Subsystem, pInh->OptionalHeader.DllCharacteristics
		, pInh->OptionalHeader.SizeOfStackReserve, pInh->OptionalHeader.SizeOfStackCommit
		, pInh->OptionalHeader.SizeOfHeapReserve, pInh->OptionalHeader.SizeOfHeapCommit
		, pInh->OptionalHeader.LoaderFlags, pInh->OptionalHeader.NumberOfRvaAndSizes
		);
	for (INT i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
		OutputVolDescLogA(
			"\t\t\tVirtualAddress[%d]: %08lx\n"
			"\t\t\t          Size[%d]: %08lx\n"
			, i, pInh->OptionalHeader.DataDirectory[i].VirtualAddress
			, i, pInh->OptionalHeader.DataDirectory[i].Size
			);
	}
}

VOID OutputFsImageSectionHeader(
	PDISC pDisc,
	PIMAGE_SECTION_HEADER pIsh
	)
{
	OutputVolDescLogA(
		"\t========== Image Section Header (%u byte) ==========\n"
		"\t                Name: %s\n"
		"\t      VirtualAddress: %08lx\n"
		"\t       SizeOfRawData: %08lx\n"
		"\t    PointerToRawData: %08lx\n"
		"\tPointerToRelocations: %08lx\n"
		"\tPointerToLinenumbers: %08lx\n"
		"\t NumberOfRelocations: %04x\n"
		"\t NumberOfLinenumbers: %04x\n"
		"\t     Characteristics: %08lx\n"
		, sizeof(IMAGE_SECTION_HEADER)
		, pIsh->Name, pIsh->VirtualAddress, pIsh->SizeOfRawData, pIsh->PointerToRawData
		, pIsh->PointerToRelocations, pIsh->PointerToLinenumbers, pIsh->NumberOfRelocations
		, pIsh->NumberOfLinenumbers, pIsh->Characteristics
	);
	if (!strncmp((LPCH)pIsh->Name, "icd1", 4)) {
		pDisc->PROTECT.byExist = codelock;
		strcpy(pDisc->PROTECT.name, (LPCH)pIsh->Name);
		pDisc->PROTECT.ERROR_SECTOR.nExtentPos = pDisc->PROTECT.nNextLBAOfLastVolDesc;
		pDisc->PROTECT.ERROR_SECTOR.nSectorSize
			= pDisc->PROTECT.nPrevLBAOfPathTablePos - pDisc->PROTECT.nNextLBAOfLastVolDesc;
	}
	else if (!strncmp((LPCH)pIsh->Name, ".vob.pcd", 8)) {
		pDisc->PROTECT.byExist = protectCDVOB;
		strcpy(pDisc->PROTECT.name, (LPCH)pIsh->Name);
	}
	else if (!strncmp((LPCH)pIsh->Name, ".cms_t", 6) || !strncmp((LPCH)pIsh->Name, ".cms_d", 6)) {
		pDisc->PROTECT.byExist = securomNew;
		strcpy(pDisc->PROTECT.name, (LPCH)pIsh->Name);
	}
}

VOID OutputTocForGD(
	PDISC pDisc
	)
{
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(TOC For GD (HD Area)));
	for (INT r = pDisc->GDROM_TOC.FirstTrack - 1; r < pDisc->GDROM_TOC.LastTrack; r++) {
		OutputDiscLogA("\tTrack %2u, Ctl %u, Mode %u"
			, pDisc->GDROM_TOC.TrackData[r].TrackNumber
			, pDisc->GDROM_TOC.TrackData[r].Control
			, pDisc->GDROM_TOC.TrackData[r].Adr);
		if (pDisc->GDROM_TOC.TrackData[r].TrackNumber == 3) {
			if (pDisc->GDROM_TOC.TrackData[r].TrackNumber == pDisc->GDROM_TOC.LastTrack) {
				OutputDiscLogA(", LBA %6ld-%6ld, Length %6ld\n"
					, pDisc->GDROM_TOC.TrackData[r].Address - 150
					, pDisc->GDROM_TOC.Length - 150
					, pDisc->GDROM_TOC.Length - 150 - 45000);
			}
			else {
				OutputDiscLogA(", LBA %6ld-%6ld, Length %6ld\n"
					, pDisc->GDROM_TOC.TrackData[r].Address - 150
					, pDisc->GDROM_TOC.TrackData[r + 1].Address - 1 - 300
					, pDisc->GDROM_TOC.TrackData[r + 1].Address - pDisc->GDROM_TOC.TrackData[r].Address);
			}
		}
		else if (pDisc->GDROM_TOC.TrackData[r].TrackNumber == pDisc->GDROM_TOC.LastTrack) {
			OutputDiscLogA(", LBA %6ld-%6ld, Length %6ld\n"
				, pDisc->GDROM_TOC.TrackData[r].Address - 375
				, pDisc->GDROM_TOC.Length - 1 - 150
				, pDisc->GDROM_TOC.Length - pDisc->GDROM_TOC.TrackData[r].Address + 225);
		}
		else if (pDisc->GDROM_TOC.TrackData[r].TrackNumber == pDisc->GDROM_TOC.LastTrack - 1) {
			OutputDiscLogA(", LBA %6ld-%6ld, Length %6ld\n"
				, pDisc->GDROM_TOC.TrackData[r].Address - 300
				, pDisc->GDROM_TOC.TrackData[r + 1].Address - 1 - 375
				, pDisc->GDROM_TOC.TrackData[r + 1].Address - 75 - pDisc->GDROM_TOC.TrackData[r].Address);
		}
		else {
			OutputDiscLogA(", LBA %6ld-%6ld, Length %6ld\n"
				, pDisc->GDROM_TOC.TrackData[r].Address - 300
				, pDisc->GDROM_TOC.TrackData[r + 1].Address - 1 - 300
				, pDisc->GDROM_TOC.TrackData[r + 1].Address - pDisc->GDROM_TOC.TrackData[r].Address);
		}
		pDisc->SCSI.lpFirstLBAListOnToc[r] = pDisc->GDROM_TOC.TrackData[r].Address;
	}
	OutputDiscLogA(
		"                                                 Total %6ld\n"
		, pDisc->GDROM_TOC.Length - 150 - 45000);
}

VOID OutputTocWithPregap(
	PDISC pDisc
	)
{
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(TOC with pregap));
	for (UINT r = 0; r < pDisc->SCSI.toc.LastTrack; r++) {
		OutputDiscLogA(
			"\tTrack %2u, Ctl %u, Mode %u", r + 1,
			pDisc->SUB.lpCtlList[r], pDisc->MAIN.lpModeList[r]);
		for (UINT k = 0; k < MAXIMUM_NUMBER_INDEXES; k++) {
			if (pDisc->SUB.lpFirstLBAListOnSub[r][k] != -1) {
				OutputDiscLogA(", Index%u %6d", k,
					pDisc->SUB.lpFirstLBAListOnSub[r][k]);
			}
			else if (k == 0) {
				OutputDiscLogA(",              ");
			}
		}
		OutputDiscLogA("\n");
	}
}

VOID OutputCDOffset(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	BOOL bGetDriveOffset,
	INT nDriveSampleOffset,
	INT nDriveOffset,
	INT nSubchOffset
	)
{
	OutputDiscLogA("======= Offset");
	if (bGetDriveOffset) {
		OutputDiscLogA(
			"(Drive offset data referes to http://www.accuraterip.com) =======");
	}
	if (pExtArg->byAdd && pDisc->SCSI.byAudioOnly) {
		pDisc->MAIN.nCombinedOffset += pExtArg->nAudioCDOffsetNum * 4;
		pExtArg->nAudioCDOffsetNum = 0; // If it is possible, I want to repair it by a better method...
		OutputDiscLogA(
			"\n"
			"\t       Combined Offset(Byte) %6d, (Samples) %5d\n"
			"\t-         Drive Offset(Byte) %6d, (Samples) %5d\n"
			"\t----------------------------------------------------\n"
			"\t User Specified Offset(Byte) %6d, (Samples) %5d\n",
			pDisc->MAIN.nCombinedOffset, pDisc->MAIN.nCombinedOffset / 4,
			nDriveOffset, nDriveSampleOffset,
			pDisc->MAIN.nCombinedOffset - nDriveOffset,
			(pDisc->MAIN.nCombinedOffset - nDriveOffset) / 4);
	}
	else {
		OutputDiscLogA(
			"\n"
			"\t Combined Offset(Byte) %6d, (Samples) %5d\n"
			"\t-   Drive Offset(Byte) %6d, (Samples) %5d\n"
			"\t----------------------------------------------\n"
			"\t       CD Offset(Byte) %6d, (Samples) %5d\n",
			pDisc->MAIN.nCombinedOffset, pDisc->MAIN.nCombinedOffset / 4,
			nDriveOffset, nDriveSampleOffset,
			pDisc->MAIN.nCombinedOffset - nDriveOffset,
			(pDisc->MAIN.nCombinedOffset - nDriveOffset) / 4);
	}

	if (0 < pDisc->MAIN.nCombinedOffset) {
		pDisc->MAIN.nAdjustSectorNum =
			pDisc->MAIN.nCombinedOffset / CD_RAW_SECTOR_SIZE + 1;
	}
	else if (pDisc->MAIN.nCombinedOffset < 0) {
		pDisc->MAIN.nAdjustSectorNum =
			pDisc->MAIN.nCombinedOffset / CD_RAW_SECTOR_SIZE - 1;
	}
	OutputDiscLogA("\tOverread sector: %d\n", pDisc->MAIN.nAdjustSectorNum);
	if (nSubchOffset != 0xff) {
		OutputDiscLogA("\tSubch Offset: %d\n", nSubchOffset);
	}
}

VOID OutputCDC2Error296(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA
	)
{
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(type);
#endif
	OutputLogA(type,
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(C2 error)
		"\t               +0 +1 +2 +3 +4 +5 +6 +7\n"
		, nLBA, nLBA);

	for (INT i = 0; i < CD_RAW_READ_C2_SIZE; i += 8) {
		OutputLogA(type,
			"\t%3x(%3d, %4d) %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i, i, i * 8, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3],
			lpBuf[i + 4], lpBuf[i + 5], lpBuf[i + 6], lpBuf[i + 7]);
	}
}

VOID OutputCDMain(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA,
	INT nSize
	)
{
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(type);
#endif
	OutputLogA(type,
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Main Channel)
		"\t          +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n"
		, nLBA, nLBA);

	for (INT i = 0; i < nSize; i += 16) {
		OutputLogA(type,
			"\t%3x(%4d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i, i, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3], lpBuf[i + 4], lpBuf[i + 5],
			lpBuf[i + 6], lpBuf[i + 7], lpBuf[i + 8], lpBuf[i + 9], lpBuf[i + 10], lpBuf[i + 11],
			lpBuf[i + 12], lpBuf[i + 13], lpBuf[i + 14], lpBuf[i + 15]);
	}
}

VOID OutputCDSub96Align(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Sub Channel)
		"\t  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B\n"
		, nLBA, nLBA);

	for (INT i = 0, ch = 0x50; i < CD_RAW_READ_SUBCODE_SIZE; i += 12, ch++) {
		OutputDiscLogA(
			"\t%c %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			ch, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3], lpBuf[i + 4], lpBuf[i + 5],
			lpBuf[i + 6], lpBuf[i + 7], lpBuf[i + 8], lpBuf[i + 9], lpBuf[i + 10], lpBuf[i + 11]);
	}
}

VOID OutputCDSub96Raw(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA
	)
{
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(type);
#endif
	OutputLogA(type,
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Sub Channel(Raw))
		"\t    +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n"
		, nLBA, nLBA);

	for (INT i = 0; i < CD_RAW_READ_SUBCODE_SIZE; i += 16) {
		OutputLogA(type,
			"\t%3X %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3], lpBuf[i + 4], lpBuf[i + 5],
			lpBuf[i + 6], lpBuf[i + 7], lpBuf[i + 8], lpBuf[i + 9], lpBuf[i + 10], lpBuf[i + 11],
			lpBuf[i + 12], lpBuf[i + 13], lpBuf[i + 14], lpBuf[i + 15]);
	}
}

VOID OutputCDSubToLog(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPBYTE lpSubcodeRaw,
	INT nLBA,
	INT byTrackNum,
	FILE* fpParse
	)
{
	CONST INT BufSize = 256;
	_TCHAR szSub0[BufSize] = { 0 };
	_sntprintf(szSub0, BufSize,
		_T(STR_LBA "P[%02x], Q[%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]{")
		, nLBA, nLBA, lpSubcode[0], lpSubcode[12], lpSubcode[13], lpSubcode[14]
		, lpSubcode[15], lpSubcode[16], lpSubcode[17], lpSubcode[18], lpSubcode[19]
		, lpSubcode[20], lpSubcode[21], lpSubcode[22], lpSubcode[23]);

	_TCHAR szSub[BufSize] = { 0 };
	// Ctl
	switch ((lpSubcode[12] >> 4) & 0x0f) {
	case 0:
		_sntprintf(szSub, BufSize,
			_T("Audio, 2ch, Copy NG, Pre-emphasis No, "));
		break;
	case AUDIO_WITH_PREEMPHASIS:
		_sntprintf(szSub, BufSize,
			_T("Audio, 2ch, Copy NG, Pre-emphasis Yes, "));
		break;
	case DIGITAL_COPY_PERMITTED:
		_sntprintf(szSub, BufSize,
			_T("Audio, 2ch, Copy OK, Pre-emphasis No, "));
		break;
	case DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		_sntprintf(szSub, BufSize,
			_T("Audio, 2ch, Copy OK, Pre-emphasis Yes, "));
		break;
	case AUDIO_DATA_TRACK:
		_sntprintf(szSub, BufSize,
			_T(" Data,      Copy NG,                  "));
		break;
	case AUDIO_DATA_TRACK | DIGITAL_COPY_PERMITTED:
		_sntprintf(szSub, BufSize,
			_T(" Data,      Copy OK,                  "));
		break;
	case TWO_FOUR_CHANNEL_AUDIO:
		_sntprintf(szSub, BufSize,
			_T("Audio, 4ch, Copy NG, Pre-emphasis No, "));
		break;
	case TWO_FOUR_CHANNEL_AUDIO | AUDIO_WITH_PREEMPHASIS:
		_sntprintf(szSub, BufSize,
			_T("Audio, 4ch, Copy NG, Pre-emphasis Yes, "));
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED:
		_sntprintf(szSub, BufSize,
			_T("Audio, 4ch, Copy OK, Pre-emphasis No, "));
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		_sntprintf(szSub, BufSize,
			_T("Audio, 4ch, Copy OK, Pre-emphasis Yes, "));
		break;
	default:
		_sntprintf(szSub, BufSize,
			_T("Unknown,                               "));
		break;
	}

	// ADR
	_TCHAR szSub2[BufSize] = { 0 };
	switch (lpSubcode[12] & 0x0f) {
	case ADR_ENCODES_CURRENT_POSITION:
		if (/*nLBA < -150 && */lpSubcode[13] == 0) {
			// lead-in area
			if (lpSubcode[14] == 0xa0) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], AMSF[%02x:%02x:%02x], TrackNumOf1stTrack[%02x], ProgramAreaFormat[%02x]"),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17], lpSubcode[19], lpSubcode[20]);
			}
			else if (lpSubcode[14] == 0xa1) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], AMSF[%02x:%02x:%02x], TrackNumOfLastTrack[%02x]"),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17], lpSubcode[19]);
			}
			else if (lpSubcode[14] == 0xa2) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], AMSF[%02x:%02x:%02x], StartTimeOfLead-out[%02x:%02x:%02x]"),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
			else {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], AMSF[%02x:%02x:%02x], StartTimeOfTrack[%02x:%02x:%02x]"),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
		}
		else if (lpSubcode[13] == 0xaa) {
			// lead-out area
			_sntprintf(szSub2, BufSize,
				_T("LeadOut  , Idx[%02x], RMSF[%02x:%02x:%02x], AMSF[%02x:%02x:%02x]"),
				lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
				lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		else {
			_sntprintf(szSub2, BufSize,
				_T("Track[%02x], Idx[%02x], RMSF[%02x:%02x:%02x], AMSF[%02x:%02x:%02x]"),
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16],
				lpSubcode[17], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		break;
	case ADR_ENCODES_MEDIA_CATALOG: {
		_TCHAR szCatalog[META_CATALOG_SIZE] = { 0 };
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0,
			pDisc->SUB.szCatalog, sizeof(pDisc->SUB.szCatalog), szCatalog, sizeof(szCatalog));
#else
		strncpy(szCatalog, pDisc->SUB.szCatalog, sizeof(szCatalog) / sizeof(szCatalog[0]));
#endif
		_sntprintf(szSub2, BufSize,
			_T("MediaCatalogNumber [%13s], AMSF[     :%02x]"), szCatalog, lpSubcode[21]);
		break;
	}
	case ADR_ENCODES_ISRC: {
		if (byTrackNum == 0) {
			OutputSubErrorWithLBALogA("Internal Error. TrackNum is 0\n", nLBA, byTrackNum);
		}
		else {
			_TCHAR szISRC[META_ISRC_SIZE] = { 0 };
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, 0,
				pDisc->SUB.pszISRC[byTrackNum - 1], META_ISRC_SIZE, szISRC, sizeof(szISRC));
#else
			strncpy(szISRC, pDisc->SUB.pszISRC[byTrackNum - 1], sizeof(szISRC) / sizeof(szISRC[0]));
#endif
			_sntprintf(szSub2, BufSize,
				_T("ItnStdRecordingCode [%12s], AMSF[     :%02x]"), szISRC, lpSubcode[21]);
		}
		break;
	}
	case 5:
		if (lpSubcode[13] == 0) {
			if (lpSubcode[14] == 0xb0) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], StartTimeForTheNextSession[%02x:%02x:%02x], NumberOfDifferentMode-5[%02x], OutermostLead-out[%02x:%02x:%02x]"),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[18], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
			else if (lpSubcode[14] == 0xb1) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], NumberOfSkipIntervalPointers[%02x], NumberOfSkipTrackAssignmentsInPoint[%02x]"),
					lpSubcode[14], lpSubcode[19], lpSubcode[20]);
			}
			else if (lpSubcode[14] == 0xb2 || lpSubcode[14] == 0xb3 || lpSubcode[14] == 0xb4) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], TrackNumberToSkipUponPlayback[%02x %02x %02x %02x %02x %02x %02x]"),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[18], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
			else if (lpSubcode[14] == 0xc0) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], OptimumRecordingPower[%02x], StartTimeOfTheFirstLead-in[%02x:%02x:%02x]"),
					lpSubcode[14], lpSubcode[15], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
			else if (lpSubcode[14] == 0xc1) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], CopyOfInfoFromA1Point[%02x %02x %02x %02x %02x %02x %02x]"),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[18], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
			else {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], SkipIntervalStopTime[%02x:%02x:%02x], SkipIntervalStartTime[%02x:%02x:%02x]"),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
		}
		else if (lpSubcode[13] == 0xaa) {
			// lead-out area
			_sntprintf(szSub2, BufSize,
				_T("LeadOutAdr5, Track[%02u], Idx[%02x], StartTime[%02x:%02x:%02x]"),
				BcdToDec((BYTE)(lpSubcode[14] >> 4 & 0x0f)), lpSubcode[14] & 0x0f,
				lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		break;
	case 0x0c:
		if (lpSubcode[13] == 0) {
			if (lpSubcode[14] == 0xb1) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], 15[%02x], 16[%02x], 17[%02x], 18[%02x], 19[%02x], 20[%02x], 21[%02x]"),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[18], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
		}
		break;
	default:
		_sntprintf(szSub2, BufSize,
			_T("Adr[%02x], Track[%02x], Idx[%02x], RMSF[%02x:%02x:%02x], AMSF[%02x:%02x:%02x]"),
			lpSubcode[12], lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16],
			lpSubcode[17], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		break;
	}

	SUB_R_TO_W scRW[4] = { 0 };
	BYTE tmpCode[24] = { 0 };
	_TCHAR szSub3[128] = { 0 };
	_tcsncat(szSub3, _T("}, RtoW["), 8);
	for (INT i = 0; i < 4; i++) {
		for (INT j = 0; j < 24; j++) {
			tmpCode[j] = (BYTE)(*(lpSubcodeRaw + (i * 24 + j)) & 0x3f);
		}
		memcpy(&scRW[i], tmpCode, sizeof(scRW[i]));
		switch (scRW[i].command) {
		case 0: // MODE 0, ITEM 0
			_tcsncat(szSub3, _T("0"), 1);
			break;
		case 8: // MODE 1, ITEM 0
			_tcsncat(szSub3, _T("Line-Graphics"), 13);
			break;
		case 9: // MODE 1, ITEM 1
			_tcsncat(szSub3, _T("TV-Graphics"), 11);
			break;
		case 10: // MODE 1, ITEM 2
			_tcsncat(szSub3, _T("Extended-TV-Graphics"), 20);
			break;
		case 20: // MODE 2, ITEM 4
			_tcsncat(szSub3, _T("CDText"), 6);
			break;
		case 24: // MODE 3, ITEM 0
			_tcsncat(szSub3, _T("Midi"), 4);
			break;
		case 56: // MODE 7, ITEM 0
			_tcsncat(szSub3, _T("User"), 4);
			break;
		default:
			_tcsncat(szSub3, _T("Reserved"), 8);
			break;
		}
		if (i < 3) {
			_tcsncat(szSub3, _T(", "), 2);
		}
		else {
			_tcsncat(szSub3, _T("]\n"), 2);
		}
	}
	_ftprintf(fpParse, _T("%s%s%s%s"), szSub0, szSub, szSub2, szSub3);
}
