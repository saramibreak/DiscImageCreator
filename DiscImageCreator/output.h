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
#pragma once
#include "forwardDeclaration.h"
#include "struct.h"

#define BOOLEAN_TO_STRING_TRUE_FALSE(_b_)		((_b_) ? _T("True") : _T("False"))
#define BOOLEAN_TO_STRING_YES_NO(_b_)			((_b_) ? _T("Yes") : _T("No"))

#define STR_DOUBLE_HYPHEN_B	"========== "
#define STR_DOUBLE_HYPHEN_E	" ==========\n"
#define STR_LBA				"LBA[%06d, %#07x]: "
#define STR_OPCODE			"OpCode[%#02x]: "
#define STR_C2FLAG			"C2flag[%d]: "
#define STR_SUBCODE			"SubCode[%x]: "
#define STR_TRACK			"Track[%02d]: "
#define STR_SUB				"Sub"
#define STR_NO_SUPPORT		" doesn't support on this drive\n"

#define OUTPUT_DHYPHEN_PLUS_STR(str)					STR_DOUBLE_HYPHEN_B str STR_DOUBLE_HYPHEN_E
#define OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(str)			STR_DOUBLE_HYPHEN_B STR_LBA str STR_DOUBLE_HYPHEN_E
#define OUTPUT_DHYPHEN_PLUS_STR_WITH_SUBCH_F(str)		STR_DOUBLE_HYPHEN_B STR_OPCODE STR_SUBCODE str STR_DOUBLE_HYPHEN_E
#define OUTPUT_DHYPHEN_PLUS_STR_WITH_C2_SUBCH_F(str)	STR_DOUBLE_HYPHEN_B STR_OPCODE STR_C2FLAG STR_SUBCODE str STR_DOUBLE_HYPHEN_E
#define OUTPUT_DHYPHEN_PLUS_STR_WITH_TRACK_F(str)		STR_DOUBLE_HYPHEN_B STR_OPCODE STR_SUBCODE STR_TRACK str STR_DOUBLE_HYPHEN_E
#define OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA				STR_DOUBLE_HYPHEN_B STR_LBA "%" CHARWIDTH "s" STR_DOUBLE_HYPHEN_E
#define OUTPUT_DHYPHEN_PLUS_STR_WITH_SUBCH				STR_DOUBLE_HYPHEN_B STR_SUBCODE "%" CHARWIDTH "s" STR_DOUBLE_HYPHEN_E
#define OUTPUT_DHYPHEN_PLUS_STR_WITH_TRACK				STR_DOUBLE_HYPHEN_B STR_TRACK "%" CHARWIDTH "s" STR_DOUBLE_HYPHEN_E
#define OUTPUT_STR_NO_SUPPORT(str)						str STR_NO_SUPPORT

// http://www.katsuster.net/index.php?arg_act=cmd_show_diary&arg_date=20160108
#define OutputString(str, ...)		_tprintf(_T(str), ##__VA_ARGS__);
#define OutputStringWithT(str, ...)	_tprintf(str, ##__VA_ARGS__);

#ifdef _DEBUG
#define FlushLog()

extern _TCHAR logBuffer[DISC_MAIN_DATA_SIZE];
#define OutputDebugStringEx(str, ...) \
{ \
	_sntprintf(logBuffer, DISC_MAIN_DATA_SIZE, _T(str), ##__VA_ARGS__); \
	logBuffer[2047] = 0; \
	OutputDebugString(logBuffer); \
}
#define OutputDebugStringWithLBAEx(str, nLBA, track, ...) \
{ \
	_sntprintf(logBuffer, DISC_MAIN_DATA_SIZE, _T(str), nLBA, nLBA, track, ##__VA_ARGS__); \
	logBuffer[2047] = 0; \
	OutputDebugString(logBuffer); \
}
#define OutputDebugStringWithLBAEx2(str, nLBA, ...) \
{ \
	_sntprintf(logBuffer, DISC_MAIN_DATA_SIZE, _T(str), nLBA, nLBA, ##__VA_ARGS__); \
	logBuffer[2047] = 0; \
	OutputDebugString(logBuffer); \
}

#define OutputErrorString(str, ...)	OutputDebugStringEx(str, ##__VA_ARGS__)
#define OutputDiscLog(str, ...)		OutputDebugStringEx(str, ##__VA_ARGS__)
#define OutputDiscWithLBALog(str, nLBA, ...) \
	OutputDebugStringWithLBAEx2(STR_LBA str, nLBA, ##__VA_ARGS__);

#define OutputVolDescLog(str, ...)		OutputDebugStringEx(str, ##__VA_ARGS__)
#define OutputVolDescWithLBALog1(str1, nLBA, ...) \
	OutputDebugStringWithLBAEx2(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(str1), nLBA, ##__VA_ARGS__);
#define OutputVolDescWithLBALog2(str1, str2, nLBA, ...) \
	OutputDebugStringWithLBAEx(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(str1) str2, nLBA, ##__VA_ARGS__);

#define OutputDriveLog(str, ...)		OutputDebugStringEx(str, ##__VA_ARGS__)
#define OutputDriveNoSupportLog(str, ...) \
	OutputDebugStringEx(OUTPUT_STR_NO_SUPPORT(str), ##__VA_ARGS__);

#define OutputMainInfoLog(str, ...)	OutputDebugStringEx(str, ##__VA_ARGS__)
#define OutputMainInfoWithLBALog(str, nLBA, track, ...) \
	OutputDebugStringWithLBAEx(STR_LBA STR_TRACK str, nLBA, track, ##__VA_ARGS__)

#define OutputMainErrorLog(str, ...)	OutputDebugStringEx(str, ##__VA_ARGS__)
#define OutputMainErrorWithLBALog(str, nLBA, track, ...) \
	OutputDebugStringWithLBAEx(STR_LBA STR_TRACK str, nLBA, track, ##__VA_ARGS__)

#define OutputSubInfoLog(str, ...)		OutputDebugStringEx(str, ##__VA_ARGS__)
#define OutputSubInfoWithLBALog(str, nLBA, track, ...) \
	OutputDebugStringWithLBAEx(STR_LBA STR_TRACK str, nLBA, track, ##__VA_ARGS__)

#define OutputSubIntentionalLog(str, ...)		OutputDebugStringEx(str, ##__VA_ARGS__)

#define OutputSubReadableLog(str, ...)		OutputDebugStringEx(str, ##__VA_ARGS__)

#define OutputSubErrorLog(str, ...)	OutputDebugStringEx(str, ##__VA_ARGS__)
#define OutputSubErrorWithLBALog(str, nLBA, track, ...) \
	OutputDebugStringWithLBAEx(STR_LBA STR_TRACK STR_SUB str, nLBA, track, ##__VA_ARGS__)

#define OutputC2ErrorLog(str, ...)		OutputDebugStringEx(str, ##__VA_ARGS__)
#define OutputC2ErrorWithLBALog(str, nLBA, ...) \
	OutputDebugStringWithLBAEx2(STR_LBA str, nLBA, ##__VA_ARGS__);

#define OutputRawReadableLog(str, ...)		OutputDebugStringEx(str, ##__VA_ARGS__)
#define OutputMdsReadableLog(str, ...)		OutputDebugStringEx(str, ##__VA_ARGS__)

#define OutputLog(type, str, ...)		OutputDebugStringEx(str, ##__VA_ARGS__)
#else
// If it uses g_LogFile, call InitLogFile()
extern _LOG_FILE g_LogFile;
#define FlushLog() \
{ \
	fflush(g_LogFile.fpDisc); \
	fflush(g_LogFile.fpVolDesc); \
	fflush(g_LogFile.fpDrive); \
	fflush(g_LogFile.fpMainInfo); \
	fflush(g_LogFile.fpMainError); \
	fflush(g_LogFile.fpSubInfo); \
	fflush(g_LogFile.fpSubIntention); \
	fflush(g_LogFile.fpSubError); \
	fflush(g_LogFile.fpC2Error); \
}

#define OutputErrorString(str, ...)			_ftprintf(stderr, _T(str), ##__VA_ARGS__);
#define OutputErrorStringWithT(str, ...)	_ftprintf(stderr, str, ##__VA_ARGS__);

#define OutputDiscLog(str, ...)			_ftprintf(g_LogFile.fpDisc, _T(str), ##__VA_ARGS__);
#define OutputDiscLogWithT(str, ...)	_ftprintf(g_LogFile.fpDisc, str, ##__VA_ARGS__);
#define OutputDiscWithLBALog(str, nLBA, ...) \
	_ftprintf(g_LogFile.fpDisc, _T(STR_LBA str), nLBA, (UINT)nLBA, ##__VA_ARGS__);
#define OutputDiscWithLBALogWithT(str, nLBA, ...) \
	fwprintf(g_LogFile.fpDisc, STR_LBA str, nLBA, (UINT)nLBA, ##__VA_ARGS__);

#define OutputVolDescLog(str, ...)		_ftprintf(g_LogFile.fpVolDesc, _T(str), ##__VA_ARGS__);
#define OutputVolDescLogWithT(str, ...)	_ftprintf(g_LogFile.fpVolDesc, str, ##__VA_ARGS__);
#define OutputVolDescWithLBALog1(str1, nLBA, ...) \
	_ftprintf(g_LogFile.fpVolDesc, _T(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(str1)), nLBA, (UINT)nLBA, ##__VA_ARGS__);
#define OutputVolDescWithLBALog2(str1, str2, nLBA, ...) \
	_ftprintf(g_LogFile.fpVolDesc, _T(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(str1)) str2, nLBA, (UINT)nLBA, ##__VA_ARGS__);

#define OutputDriveLog(str, ...)		_ftprintf(g_LogFile.fpDrive, _T(str), ##__VA_ARGS__);
#define OutputDriveLogWithT(str, ...)	_ftprintf(g_LogFile.fpDrive, str, ##__VA_ARGS__);
#define OutputDriveNoSupportLog(str, ...) \
	_ftprintf(g_LogFile.fpDrive, _T(OUTPUT_STR_NO_SUPPORT(str)), ##__VA_ARGS__);

#define OutputMainInfoLog(str, ...)			_ftprintf(g_LogFile.fpMainInfo, _T(str), ##__VA_ARGS__);
#define OutputMainInfoLogWithT(str, ...)	_ftprintf(g_LogFile.fpMainInfo, str, ##__VA_ARGS__);
#define OutputMainInfoWithLBALog(str, nLBA, track, ...) \
	_ftprintf(g_LogFile.fpMainInfo, _T(STR_LBA STR_TRACK str), nLBA, (UINT)nLBA, track, ##__VA_ARGS__);
#define OutputMainInfoWithLBALogWithT(str, nLBA, track, ...) \
	_ftprintf(g_LogFile.fpMainInfo, STR_LBA STR_TRACK str, nLBA, (UINT)nLBA, track, ##__VA_ARGS__);

#define OutputMainErrorLog(str, ...)		_ftprintf(g_LogFile.fpMainError, _T(str), ##__VA_ARGS__);
#define OutputMainErrorLogWithT(str, ...)	_ftprintf(g_LogFile.fpMainError, str, ##__VA_ARGS__);
#define OutputMainErrorWithLBALog(str, nLBA, track, ...) \
	_ftprintf(g_LogFile.fpMainError, _T(STR_LBA STR_TRACK str), nLBA, (UINT)nLBA, track, ##__VA_ARGS__);
#define OutputMainErrorWithLBALogWithT(str, nLBA, track, ...) \
	_ftprintf(g_LogFile.fpMainError, STR_LBA STR_TRACK str, nLBA, (UINT)nLBA, track, ##__VA_ARGS__);

#define OutputSubInfoLog(str, ...)		_ftprintf(g_LogFile.fpSubInfo, _T(str), ##__VA_ARGS__);
#define OutputSubInfoLogWithT(str, ...)	_ftprintf(g_LogFile.fpSubInfo, str, ##__VA_ARGS__);
#define OutputSubInfoWithLBALog(str, nLBA, track, ...) \
	_ftprintf(g_LogFile.fpSubInfo, _T(STR_LBA STR_TRACK str), nLBA, (UINT)nLBA, track, ##__VA_ARGS__);
#define OutputSubInfoWithLBALogWithT(str, nLBA, track, ...) \
	_ftprintf(g_LogFile.fpSubInfo, STR_LBA STR_TRACK str, nLBA, (UINT)nLBA, track, ##__VA_ARGS__);

#define OutputSubIntentionalLog(str, ...)		_ftprintf(g_LogFile.fpSubIntention, _T(str), ##__VA_ARGS__);
#define OutputSubIntentionalLogWithT(str, ...)	_ftprintf(g_LogFile.fpSubIntention, str, ##__VA_ARGS__);

#define OutputSubReadableLog(str, ...)		_ftprintf(g_LogFile.fpSubReadable, _T(str), ##__VA_ARGS__);

#define OutputSubErrorLog(str, ...)			_ftprintf(g_LogFile.fpSubError, _T(str), ##__VA_ARGS__);
#define OutputSubErrorLogWithT(str, ...)	_ftprintf(g_LogFile.fpSubError, str, ##__VA_ARGS__);
#define OutputSubErrorWithLBALog(str, nLBA, track, ...) \
	_ftprintf(g_LogFile.fpSubError, _T(STR_LBA STR_TRACK STR_SUB str), nLBA, (UINT)nLBA, track, ##__VA_ARGS__);
#define OutputSubErrorWithLBALogWithT(str, nLBA, track, ...) \
	_ftprintf(g_LogFile.fpSubError, STR_LBA STR_TRACK STR_SUB str, nLBA, (UINT)nLBA, track, ##__VA_ARGS__);

#define OutputC2ErrorLog(str, ...)		_ftprintf(g_LogFile.fpC2Error, _T(str), ##__VA_ARGS__);
#define OutputC2ErrorLogWithT(str, ...)	_ftprintf(g_LogFile.fpC2Error, str, ##__VA_ARGS__);
#define OutputC2ErrorWithLBALog(str, nLBA, ...) \
	_ftprintf(g_LogFile.fpC2Error, _T(STR_LBA str), nLBA, (UINT)nLBA, ##__VA_ARGS__);
#define OutputC2ErrorWithLBALogWithT(str, nLBA, ...) \
	_ftprintf(g_LogFile.fpC2Error, STR_LBA str, nLBA, (UINT)nLBA, ##__VA_ARGS__);

#define OutputRawReadableLog(str, ...)		_ftprintf(g_LogFile.fpRawReadable, _T(str), ##__VA_ARGS__);
#define OutputMdsReadableLog(str, ...)		_ftprintf(g_LogFile.fpMdsReadable, _T(str), ##__VA_ARGS__);

#define OutputLog(type, str, ...) \
{ \
	INT t = type; \
	if ((t & standardOut) == standardOut) { \
		OutputString(str, ##__VA_ARGS__); \
	} \
	if ((t & standardError) == standardError) { \
		OutputErrorString(str, ##__VA_ARGS__); \
	} \
	if ((t & fileDisc) == fileDisc) { \
		OutputDiscLog(str, ##__VA_ARGS__); \
	} \
	if ((t & fileVolDesc) == fileVolDesc) { \
		OutputVolDescLog(str, ##__VA_ARGS__); \
	} \
	if ((t & fileDrive) == fileDrive) { \
		OutputDriveLog(str, ##__VA_ARGS__); \
	} \
	if ((t & fileMainInfo) == fileMainInfo) { \
		OutputMainInfoLog(str, ##__VA_ARGS__); \
	} \
	if ((t & fileMainError) == fileMainError) { \
		OutputMainErrorLog(str, ##__VA_ARGS__); \
	} \
	if ((t & fileSubInfo) == fileSubInfo) { \
		OutputSubInfoLog(str, ##__VA_ARGS__); \
	} \
	if ((t & fileSubIntention) == fileSubIntention) { \
		OutputSubIntentionalLog(str, ##__VA_ARGS__); \
	} \
	if ((t & fileSubError) == fileSubError) { \
		OutputSubErrorLog(str, ##__VA_ARGS__); \
	} \
	if ((t & fileC2Error) == fileC2Error) { \
		OutputC2ErrorLog(str, ##__VA_ARGS__); \
	} \
	if ((t & fileMds) == fileMds) { \
		OutputMdsReadableLog(str, ##__VA_ARGS__); \
	} \
}
#define OutputLogWithT(type, str, ...) \
{ \
	INT t = type; \
	if ((t & standardOut) == standardOut) { \
		OutputStringWithT(str, ##__VA_ARGS__); \
	} \
	if ((t & standardError) == standardError) { \
		OutputErrorStringWithT(str, ##__VA_ARGS__); \
	} \
	if ((t & fileDisc) == fileDisc) { \
		OutputDiscLogWithT(str, ##__VA_ARGS__); \
	} \
	if ((t & fileVolDesc) == fileVolDesc) { \
		OutputVolDescLogWithT(str, ##__VA_ARGS__); \
	} \
	if ((t & fileDrive) == fileDrive) { \
		OutputDriveLogWithT(str, ##__VA_ARGS__); \
	} \
	if ((t & fileMainInfo) == fileMainInfo) { \
		OutputMainInfoLogWithT(str, ##__VA_ARGS__); \
	} \
	if ((t & fileMainError) == fileMainError) { \
		OutputMainErrorLogWithT(str, ##__VA_ARGS__); \
	} \
	if ((t & fileSubInfo) == fileSubInfo) { \
		OutputSubInfoLogWithT(str, ##__VA_ARGS__); \
	} \
	if ((t & fileSubIntention) == fileSubIntention) { \
		OutputSubIntentionalLogWithT(str, ##__VA_ARGS__); \
	} \
	if ((t & fileSubError) == fileSubError) { \
		OutputSubErrorLogWithT(str, ##__VA_ARGS__); \
	} \
	if ((t & fileC2Error) == fileC2Error) { \
		OutputC2ErrorLogWithT(str, ##__VA_ARGS__); \
	} \
}
#endif

#ifdef UNICODE
#define WFLAG "w, ccs=UTF-8"
#define APLUSFLAG "a+, ccs=UTF-8"
#else
#define WFLAG "w"
#define APLUSFLAG "a+"
#endif

#define FcloseAndNull(fp) \
{ \
	if (fp) { \
		fclose(fp); \
		fp = NULL; \
	} \
}

#define FreeAndNull(lpBuf) \
{ \
	if (lpBuf) { \
		free(lpBuf); \
		lpBuf = NULL; \
	} \
}

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
);

FILE* OpenProgrammabledFile(
	LPCTSTR pszFname,
	LPCTSTR pszMode
);

VOID WriteCcdForDisc(
	WORD wTocEntries,
	BYTE LastCompleteSession,
	FILE* fpCcd
);

VOID WriteCcdForDiscCDTextLength(
	WORD wCDTextLength,
	FILE* fpCcd
);

VOID WriteCcdForDiscCatalog(
	PDISC pDisc,
	FILE* fpCcd
);

VOID WriteCcdForCDText(
	WORD dwTocTextEntries,
	FILE* fpCcd
);

VOID WriteCcdForCDTextEntry(
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	WORD dwTocTextEntries,
	FILE* fpCcd
);

VOID WriteCcdForSession(
	BYTE SessionNumber,
	BYTE byMode,
	FILE* fpCcd
);

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
);

VOID WriteCcdForEntry(
	PCDROM_TOC_FULL_TOC_DATA_BLOCK toc,
	UINT a,
	FILE* fpCcd
);

VOID WriteMainChannel(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nLBA,
	FILE* fpImg
);

VOID WriteC2(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nLBA,
	FILE* fpC2
);

VOID WriteSubChannel(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	LPBYTE lpSubcodeRaw,
	INT nLBA,
	FILE* fpSub
);

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
);

BOOL WriteParsingSubfile(
	LPCTSTR pszSubfile
);

BOOL WriteParsingMdsfile(
	LPCTSTR pszMdsfile
);

BOOL DescrambleMainChannelForGD(
	LPCTSTR pszPath,
	LPTSTR pszOutPath
);

BOOL CreateBinCueForGD(
	PDISC pDisc,
	LPCTSTR pszPath
);

VOID DescrambleMainChannelAll(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpScrambledBuf,
	FILE* fpImg
);

VOID DescrambleMainChannelPartial(
	INT nStartLBA,
	INT nEndLBA,
	LPBYTE lpScrambledBuf,
	FILE* fpImg
);

BOOL CreateBinCueCcd(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPCTSTR pszPath,
	LPCTSTR pszImgName,
	BOOL bCanCDText,
	FILE* fpImg,
	FILE* fpCcd
);

VOID OutputIntentionalSubchannel(
	INT nLBA,
	LPBYTE lpSubcode
);

VOID OutputHashData(
	FILE* fpHash,
	LPCTSTR filename,
	UINT64 ui64FileSize,
	DWORD crc32,
	LPBYTE digest,
	LPBYTE Message_Digest
);

VOID OutputLastErrorNumAndString(
	LPCTSTR pszFuncName,
	LONG lLineNum
);
#ifdef _WIN32
BOOL OutputWindowsVersion(
	VOID
);
#endif

BOOL OutputMergedFile(
	LPCTSTR pszFullPath,
	LPCTSTR pszFullPath2
);

size_t WriteBufWithCalc(
	LPBYTE lpBuf,
	size_t writeSize,
	ULONG ulTransferLen,
	FILE* fp,
	PHASH pHash
);

VOID OutputMainChannel(
	LOG_TYPE type,
	LPBYTE lpBuf,
	LPCTSTR szLabel,
	INT nLBA,
	DWORD dwSize
);
