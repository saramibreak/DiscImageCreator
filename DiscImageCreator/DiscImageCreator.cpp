// DiscImageCreator.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//
/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "calcHash.h"
#include "check.h"
#include "execIoctl.h"
#include "execScsiCmd.h"
#include "execScsiCmdforCD.h"
#include "execScsiCmdforDVD.h"
#include "get.h"
#include "init.h"
#include "output.h"
#include "xml.h"
#include "_external\prngcd.h"

#define DEFAULT_REREAD_VAL			(1024)
#define DEFAULT_MAX_C2_ERROR_VAL	(4096)
#define DEFAULT_REREAD_SPEED_VAL	(4)
#define DEFAULT_SPTD_TIMEOUT_VAL	(60)

BYTE g_aSyncHeader[SYNC_SIZE] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0x00
};

// These static variable is set at printAndSetPath().
static _TCHAR s_szCurrentdir[_MAX_PATH];
static _TCHAR s_szDrive[_MAX_DRIVE];
static _TCHAR s_szDir[_MAX_DIR];
static _TCHAR s_szFname[_MAX_FNAME];
static _TCHAR s_szExt[_MAX_EXT];

// These static variable is set at checkArg().
static DWORD s_dwSpeed = 0;
static INT s_nStartLBA = 0;
static INT s_nEndLBA = 0;

#define playtime (200)
#define c4 (262)
#define d4 (294)
#define e4 (330)
#define f4 (349)
#define g4 (392)
#define a4 (440)
#define b4 (494)
#define c5 (523)
#define d5 (587)
#define e5 (659)
#define f5 (698)
#define g5 (784)
#define a5 (880)
#define b5 (988)
#define c6 (1047)

int soundBeep(int nRet)
{
	if (nRet) {
		if (!Beep(c4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(d4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(e4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(f4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(g4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(a4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(b4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(c5, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
	}
	else {
		if (!Beep(c5, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(b4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(a4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(g4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(f4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(e4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(d4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(c4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
	}
	return TRUE;
}

int exec(_TCHAR* argv[], PEXEC_TYPE pExecType, PEXT_ARG pExtArg, _TCHAR* pszFullPath)
{
	BOOL bRet = FALSE;
	SetLastError(NO_ERROR);

	if (*pExecType == sub) {
		bRet = WriteParsingSubfile(pszFullPath);
	}
	else {
		_TCHAR szBuf[8] = { 0 };
		_sntprintf(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), _T("\\\\.\\%c:"), argv[2][0]);
		szBuf[7] = 0;
		DEVICE devData = { 0 };
		devData.hDevice = CreateFile(szBuf, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (devData.hDevice == INVALID_HANDLE_VALUE) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		// 1st: set TimeOutValue here (because use ScsiPassThroughDirect)
		if (pExtArg->byScanProtectViaFile) {
			devData.dwTimeOutValue = pExtArg->dwTimeoutNum;
		}
		else {
			devData.dwTimeOutValue = DEFAULT_SPTD_TIMEOUT_VAL;
		}
		if (*pExecType == stop) {
			bRet = StartStopUnit(pExtArg, &devData, STOP_UNIT_CODE, STOP_UNIT_CODE);
		}
		else if (*pExecType == start) {
			bRet = StartStopUnit(pExtArg, &devData, START_UNIT_CODE, STOP_UNIT_CODE);
		}
		else if (*pExecType == eject) {
			bRet = StartStopUnit(pExtArg, &devData, STOP_UNIT_CODE, START_UNIT_CODE);
		}
		else if (*pExecType == closetray) {
			bRet = StartStopUnit(pExtArg, &devData, START_UNIT_CODE, START_UNIT_CODE);
		}
		else if (*pExecType == reset) {
			bRet = Reset(pExtArg, &devData);
		}
		else {
			DISC discData = { '\0' };
			PDISC pDisc = &discData;
			MAIN_HEADER mainHeader = { 0 };
			FILE* fpCcd = NULL;
			FILE* fpC2 = NULL;
			PC2_ERROR_PER_SECTOR pC2ErrorPerSector = NULL;
			C2_ERROR_ALLOCATION_CONTEXT c2ErrorContext = { FALSE };
			try {
#ifndef _DEBUG
				// 2nd: create logfile here (because logging all working)
				if (!InitLogFile(pExecType, pExtArg, pszFullPath)) {
					throw FALSE;
				}
#endif
				if (!TestUnitReady(pExtArg, &devData)) {
					throw FALSE;
				}
				if (*pExecType == fd) {
					if (!DiskGetMediaTypes(&devData, pszFullPath)) {
						throw FALSE;
					}
				}
				else {
					ReadDriveInformation(pExecType, pExtArg, &devData, pDisc, s_dwSpeed);
					if (*pExecType == cd || *pExecType == gd || *pExecType == data || *pExecType == audio) {
						if (discData.SCSI.wCurrentMedia == ProfileCdrom ||
							discData.SCSI.wCurrentMedia == ProfileCdRecordable ||
							discData.SCSI.wCurrentMedia == ProfileCdRewritable ||
							(discData.SCSI.wCurrentMedia == ProfileInvalid && (*pExecType == gd))) {
							if (!pExtArg->byC2) {
								OutputString(
									_T("[WARNING] /c2 isn't set. The result of ripping may be incorrect if c2 error exists.\n"));
							}
							if (!ReadTOC(pExtArg, pExecType, &devData, pDisc)) {
								throw FALSE;
							}
							// 6th: open ccd here (because use ReadTOCFull and from there)
							if (*pExecType == cd && !pExtArg->byReverse) {
								if (NULL == (fpCcd = CreateOrOpenFile(pszFullPath, NULL,
									NULL, NULL, NULL, _T(".ccd"), _T(WFLAG), 0, 0))) {
									OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
									throw FALSE;
								}
							}
							if (discData.SCSI.wCurrentMedia == ProfileCdrom) {
								ReadDiscInformation(pExtArg, &devData);
							}
							else if (discData.SCSI.wCurrentMedia == ProfileCdRecordable ||
								discData.SCSI.wCurrentMedia == ProfileCdRewritable) {
								ReadTOCAtip(pExtArg, &devData);
							}
							InitMainDataHeader(pExecType, pExtArg, &mainHeader, s_nStartLBA);
							if (!InitSubData(pExecType, &pDisc)) {
								throw FALSE;
							}
							if (!InitTocFullData(pExecType, &pDisc)) {
								throw FALSE;
							}
							if (!InitTocTextData(pExecType, &devData, &pDisc)) {
								throw FALSE;
							}
							if (!InitProtectData(&pDisc)) {
								throw FALSE;
							}
							make_scrambled_table();
							make_crc_table();
							MakeCrc16CCITTTable();
#if 0
							MakeCrc6ITUTable();
#endif
							CDFLAG::_READ_CD::_ERROR_FLAGS c2 = CDFLAG::_READ_CD::NoC2;
							ReadCDForCheckingByteOrder(pExtArg, &devData, &c2);
							if (pExtArg->byC2 && devData.FEATURE.byC2ErrorData && c2 != CDFLAG::_READ_CD::NoC2) {
								if (!InitC2ErrorData(pExtArg,
									&pC2ErrorPerSector, CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE, &c2ErrorContext)) {
									throw FALSE;
								}
								if (NULL == (fpC2 = CreateOrOpenFile(
									pszFullPath, NULL, NULL, NULL, NULL, _T(".c2"), _T("wb"), 0, 0))) {
									OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
									throw FALSE;
								}
							}
//							if (*pExecType != data) {
								if (!ReadCDForSearchingOffset(pExecType, pExtArg, &devData, pDisc)) {
									throw FALSE;
								}
//							}
							if (pDisc->SUB.nSubChannelOffset && pExtArg->dwSubAddionalNum == 0) {
								OutputString(
									_T("[WARNING] SubChannel Offset exists in this drive. Changed /s 0 to /s 1.\n"));
								pExtArg->dwSubAddionalNum = 1;
							}
							if (/**pExecType == data && (pExtArg->byD8 || devData.byPlxtrDrive) ||*/
								(*pExecType != data && *pExecType != gd)) {
								if (!ReadCDForCheckingReadInOut(pExtArg, &devData, pDisc)) {
									throw FALSE;
								}
							}
							DISC_PER_SECTOR discPerSector = { 0 };
							memcpy(&discPerSector.mainHeader, &mainHeader, sizeof(MAIN_HEADER));
							if (!ReadTOCFull(pExtArg, &devData, &discData, &discPerSector, fpCcd)) {
								throw FALSE;
							}
							if (!pDisc->SCSI.bMultiSession && pExtArg->byRawDump) {
								OutputString(
									_T("[WARNING] This disc isn't Multi-Session. /raw is ignored.\n"));
								pExtArg->byRawDump = FALSE;
							}
							if (!pExtArg->byReverse) {
								// Typically, CD+G data is included in audio only disc
								// But exceptionally, WonderMega Collection (SCD)(mixed disc) exists CD+G data.
								if (!ReadCDForCheckingSubRtoW(pExtArg, &devData, pDisc)) {
									throw FALSE;
								}
								if (pDisc->SCSI.trackType != TRACK_TYPE::audioOnly) {
									if (!ReadCDForFileSystem(pExtArg, &devData, pDisc)) {
										throw FALSE;
									}
									if (pExtArg->byScanProtectViaSector) {
										if (!ReadCDForScanningProtectViaSector(pExtArg, &devData, pDisc)) {
											throw FALSE;
										}
									}
									if ((pExtArg->byScanProtectViaFile || pExtArg->byScanProtectViaSector) &&
										pDisc->PROTECT.byExist == PROTECT_TYPE::no) {
										OutputString(
											_T("[WARNING] Protection can't be detected. /sf, /ss is ignored.\n"));
										pExtArg->byScanProtectViaFile = FALSE;
										pExtArg->byScanProtectViaSector = FALSE;
									}
								}
							}
							if (*pExecType == cd) {
								bRet = ReadCDAll(pExecType, pExtArg, &devData, pDisc, &discPerSector
									, pC2ErrorPerSector, c2, pszFullPath, fpCcd, fpC2);
							}
							else if (*pExecType == gd) {
								if (!ReadCDForGDTOC(pExtArg, &devData, pDisc)) {
									throw FALSE;
								}
								bRet = ReadCDPartial(pExecType, pExtArg, &devData, pDisc, &discPerSector, pC2ErrorPerSector
									, c2, pszFullPath, FIRST_LBA_FOR_GD, 549149 + 1, CDFLAG::_READ_CD::CDDA, fpC2);
							}
							else if (*pExecType == data) {
								bRet = ReadCDPartial(pExecType, pExtArg, &devData, pDisc, &discPerSector, pC2ErrorPerSector
									, c2, pszFullPath, s_nStartLBA, s_nEndLBA, CDFLAG::_READ_CD::All, fpC2);
							}
							else if (*pExecType == audio) {
								bRet = ReadCDPartial(pExecType, pExtArg, &devData, pDisc, &discPerSector, pC2ErrorPerSector
									, c2, pszFullPath, s_nStartLBA, s_nEndLBA, CDFLAG::_READ_CD::CDDA, fpC2);
							}
						}
						else {
							OutputString(_T("Wrong command. The disc isn't CD, CD-R, CD-RW, GD\n"));
						}
					}
					else if (*pExecType == dvd) {
						if (discData.SCSI.wCurrentMedia == ProfileDvdRom ||
							discData.SCSI.wCurrentMedia == ProfileDvdRecordable ||
							discData.SCSI.wCurrentMedia == ProfileDvdRam ||
							discData.SCSI.wCurrentMedia == ProfileDvdRewritable ||
							discData.SCSI.wCurrentMedia == ProfileDvdRWSequential ||
							discData.SCSI.wCurrentMedia == ProfileDvdDashRDualLayer ||
							discData.SCSI.wCurrentMedia == ProfileDvdDashRLayerJump ||
							discData.SCSI.wCurrentMedia == ProfileDvdPlusRW ||
							discData.SCSI.wCurrentMedia == ProfileDvdPlusR) {
							bRet = ReadDVDStructure(pExtArg, &devData, &discData);

							if (pExtArg->byCmi) {
								bRet = ReadDVDForCMI(pExtArg, &devData, &discData);
							}
							if (bRet) {
								if (argv[5] && !_tcsncmp(argv[5], _T("raw"), 3)) {
#if 0
									bRet = ReadDVDRaw(&devData, &discData, szVendorId, pszFullPath);
#endif
								}
								else {
									bRet = ReadDVD(pExtArg, &devData, &discData, pszFullPath);
								}
							}
						}
						else {
							OutputString(_T("Wrong command. The disc isn't DVD, DVD-R, DVD-RW\n"));
						}
					}
				}
				if (bRet && (*pExecType == cd || *pExecType == dvd || *pExecType == gd)) {
					bRet = ReadWriteDat(pExecType, pszFullPath, discData.SCSI.toc.LastTrack, s_szDrive, s_szDir, s_szFname);
				}
			}
			catch (BOOL bErr) {
				bRet = bErr;
			}
			TerminateLBAPerTrack(&pDisc);
			TerminateSubData(pExecType, &pDisc);
			TerminateProtectData(&pDisc);
			TerminateTocFullData(&pDisc);
			if (devData.bySuccessReadToc) {
				TerminateTocTextData(pExecType, &devData, &pDisc);
			}
			FcloseAndNull(fpCcd);
			if (pExtArg->byC2 && devData.FEATURE.byC2ErrorData) {
				FcloseAndNull(fpC2);
				TerminateC2ErrorData(pExtArg, &devData, &pC2ErrorPerSector, &c2ErrorContext);
			}
#ifndef _DEBUG
			TerminateLogFile(pExecType, pExtArg);
#endif
		}
		if (devData.hDevice && !CloseHandle(devData.hDevice)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	return bRet;
}

int printAndSetPath(_TCHAR* szPathFromArg, _TCHAR* pszFullPath)
{
	if (!GetCurrentDirectory(sizeof(s_szCurrentdir) / sizeof(s_szCurrentdir[0]), s_szCurrentdir)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	_tsplitpath(szPathFromArg, s_szDrive, s_szDir, s_szFname, s_szExt);

	if (!s_szDrive[0] || !s_szDir[0]) {
		_tcsncpy(pszFullPath, s_szCurrentdir, _MAX_PATH);
		pszFullPath[_MAX_PATH] = 0;
		if (s_szDir[0]) {
			if (!PathAppend(pszFullPath, s_szDir)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
		}
		if (!PathFileExists(pszFullPath)) {
			OutputErrorString(_T("%s doesn't exist, so create.\n"), pszFullPath);
#ifdef UNICODE
			if (SHCreateDirectory(NULL, pszFullPath)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
#else
			if (!MakeSureDirectoryPathExists(pszFullPath)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
#endif
		}
		if (!PathAppend(pszFullPath, s_szFname)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		_tsplitpath(pszFullPath, s_szDrive, s_szDir, s_szFname, NULL);
	}
	else {
		_tcsncpy(pszFullPath, szPathFromArg, _MAX_PATH);
		if (!PathFileExists(s_szDir)) {
			OutputErrorString(_T("%s doesn't exist, so create.\n"), pszFullPath);
#ifdef UNICODE
			if (SHCreateDirectory(NULL, s_szDir)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
#else
			if (!MakeSureDirectoryPathExists(pszFullPath)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
#endif
		}
	}
	OutputString(
		_T("CurrentDirectory\n")
		_T("\t%s\n")
		_T("WorkingPath\n")
		_T("\t Argument: %s\n")
		_T("\t FullPath: %s\n")
		_T("\t    Drive: %s\n")
		_T("\tDirectory: %s\n")
		_T("\t Filename: %s\n")
		_T("\tExtension: %s\n"),
		s_szCurrentdir, szPathFromArg, pszFullPath, s_szDrive, s_szDir, s_szFname, s_szExt);

	return TRUE;
}

int SetOptionS(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->dwSubAddionalNum = _tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->dwSubAddionalNum = 1;
		OutputString(_T("/s val is omitted. set [%d]\n"), 1);
	}
	return TRUE;
}

int SetOptionSf(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	pExtArg->byScanProtectViaFile = TRUE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->dwTimeoutNum = _tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->dwTimeoutNum = DEFAULT_SPTD_TIMEOUT_VAL;
		OutputString(
			_T("/sf val is omitted. set [%d]\n"), DEFAULT_SPTD_TIMEOUT_VAL);
	}
	return TRUE;
}

int SetOptionC2(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	pExtArg->byC2 = TRUE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->dwMaxRereadNum = _tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
			return FALSE;
		}
		if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
			pExtArg->dwMaxC2ErrorNum = _tcstoul(argv[(*i)++], &endptr, 10) * 2;
			if (*endptr) {
				OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
				return FALSE;
			}
			else if (pExtArg->dwMaxC2ErrorNum == 0) {
				OutputString(
					_T("/c2 val2 is 0. Changed default val:[%d]\n")
					, DEFAULT_MAX_C2_ERROR_VAL);
				pExtArg->dwMaxC2ErrorNum = DEFAULT_MAX_C2_ERROR_VAL * 2;
			}
			if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
				pExtArg->dwRereadSpeedNum = _tcstoul(argv[(*i)++], &endptr, 10);
				if (*endptr) {
					OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
					return FALSE;
				}
			}
			else {
				pExtArg->dwRereadSpeedNum = DEFAULT_REREAD_SPEED_VAL;
				OutputString(
					_T("/c2 val3 is omitted. set [%d]\n")
					, DEFAULT_REREAD_SPEED_VAL);
			}
		}
		else {
			pExtArg->dwMaxC2ErrorNum = DEFAULT_MAX_C2_ERROR_VAL * 2;
			pExtArg->dwRereadSpeedNum = DEFAULT_REREAD_SPEED_VAL;
			OutputString(
				_T("/c2 val2 is omitted. set [%d]\n")
				_T("/c2 val3 is omitted. set [%d]\n")
				, DEFAULT_MAX_C2_ERROR_VAL
				, DEFAULT_REREAD_SPEED_VAL);
		}
	}
	else {
		pExtArg->dwMaxRereadNum = DEFAULT_REREAD_VAL;
		pExtArg->dwMaxC2ErrorNum = DEFAULT_MAX_C2_ERROR_VAL * 2;
		pExtArg->dwRereadSpeedNum = DEFAULT_REREAD_SPEED_VAL;
		OutputString(
			_T("/c2 val1 is omitted. set [%d]\n")
			_T("/c2 val2 is omitted. set [%d]\n")
			_T("/c2 val3 is omitted. set [%d]\n")
			, DEFAULT_REREAD_VAL
			, DEFAULT_MAX_C2_ERROR_VAL
			, DEFAULT_REREAD_SPEED_VAL);
	}
	return TRUE;
}

int SetOptionBe(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	pExtArg->byBe = TRUE;
	pExtArg->byD8 = FALSE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		if (!_tcsncmp(argv[*i], _T("pack"), 4)) {
			pExtArg->byPack = TRUE;
			(*i)++;
		}
		else if (!_tcsncmp(argv[*i], _T("raw"), 3)) {
			pExtArg->byRaw = TRUE;
			(*i)++;
		}
		else {
			OutputErrorString(_T("Bad arg: [%s] Please enter pack or raw\n"), argv[*i]);
			return FALSE;
		}
	}
	else {
		pExtArg->byRaw = TRUE;
		OutputString(_T("submode of /be is omitted. set [raw]\n"));
	}
	return TRUE;
}

int SetOptionA(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	pExtArg->byAdd = TRUE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->nAudioCDOffsetNum = _tcstol(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->nAudioCDOffsetNum = 0;
		OutputString(_T("/a val is omitted. set [%d]\n"), 0);
	}
	return TRUE;
}

int checkArg(int argc, _TCHAR* argv[], PEXEC_TYPE pExecType, PEXT_ARG pExtArg, _TCHAR* pszFullPath)
{
	_TCHAR* endptr = NULL;
	size_t cmdLen = 0;
	if (argc == 1) {
		return FALSE;
	}
	else {
		cmdLen = _tcslen(argv[1]);
		if (argc >= 5 && cmdLen == 2 && !_tcsncmp(argv[1], _T("cd"), 2)) {
			s_dwSpeed = _tcstoul(argv[4], &endptr, 10);
			if (*endptr) {
				OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
				return FALSE;
			}
			pExtArg->dwSubAddionalNum = 1;
			for (INT i = 6; i <= argc; i++) {
				cmdLen = _tcslen(argv[i - 1]);
				if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/q"), 2)) {
					pExtArg->byQuiet = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/a"), 2)) {
					if (!SetOptionA(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/be"), 3)) {
					if (!SetOptionBe(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/d8"), 3)) {
					pExtArg->byBe = FALSE;
					pExtArg->byD8 = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/c2"), 3)) {
					if (!SetOptionC2(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/f"), 2)) {
					pExtArg->byFua = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/m"), 2)) {
					pExtArg->byMCN = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/p"), 2)) {
					pExtArg->byPre = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/sf"), 3)) {
					if (!SetOptionSf(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/ss"), 3)) {
					pExtArg->byScanProtectViaSector = TRUE;
				}
				else if (cmdLen == 4 && !_tcsncmp(argv[i - 1], _T("/raw"), 4)) {
					pExtArg->byRawDump = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/np"), 3)) {
					pExtArg->bySkipSubP = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/nq"), 3)) {
					pExtArg->bySkipSubQ = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/nr"), 3)) {
					pExtArg->bySkipSubRtoW = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/nl"), 3)) {
					pExtArg->byLibCrypt = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/ns"), 3)) {
					pExtArg->byIntentionalSub = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/s"), 2)) {
					if (!SetOptionS(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else {
					OutputErrorString(_T("Unknown option: [%s]\n"), argv[i - 1]);
					return FALSE;
				}
			}
			*pExecType = cd;
			printAndSetPath(argv[3], pszFullPath);
		}
		else if (argc >= 5 && cmdLen == 2 && !_tcsncmp(argv[1], _T("gd"), 2)) {
			s_dwSpeed = _tcstoul(argv[4], &endptr, 10);
			if (*endptr) {
				OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
				return FALSE;
			}
			pExtArg->dwSubAddionalNum = 0;
			for (INT i = 6; i <= argc; i++) {
				cmdLen = _tcslen(argv[i - 1]);
				if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/q"), 2)) {
					pExtArg->byQuiet = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/be"), 3)) {
					if (!SetOptionBe(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/d8"), 3)) {
					pExtArg->byBe = FALSE;
					pExtArg->byD8 = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/c2"), 3)) {
					if (!SetOptionC2(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/np"), 3)) {
					pExtArg->bySkipSubP = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/nq"), 3)) {
					pExtArg->bySkipSubQ = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/nr"), 3)) {
					pExtArg->bySkipSubRtoW = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/s"), 2)) {
					if (!SetOptionS(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else {
					OutputErrorString(_T("Unknown option: [%s]\n"), argv[i - 1]);
					return FALSE;
				}
			}
			*pExecType = gd;
			printAndSetPath(argv[3], pszFullPath);
		}
		else if (argc >= 5 && cmdLen == 3 && !_tcsncmp(argv[1], _T("dvd"), 3)) {
			s_dwSpeed = _tcstoul(argv[4], &endptr, 10);
			if (*endptr) {
				OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
				return FALSE;
			}
			for (INT i = 6; i <= argc; i++) {
				cmdLen = _tcslen(argv[i - 1]);
				if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/c"), 2)) {
					pExtArg->byCmi = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/f"), 2)) {
					pExtArg->byFua = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/q"), 2)) {
					pExtArg->byQuiet = TRUE;
				}
				else {
					OutputErrorString(_T("Unknown option: [%s]\n"), argv[i - 1]);
					return FALSE;
				}
			}
			*pExecType = dvd;
			printAndSetPath(argv[3], pszFullPath);
		}
		else if (argc >= 7 && (cmdLen == 4 && !_tcsncmp(argv[1], _T("data"), 4) ||
			cmdLen == 5 && !_tcsncmp(argv[1], _T("audio"), 5))) {
			s_dwSpeed = _tcstoul(argv[4], &endptr, 10);
			if (*endptr) {
				OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
				return FALSE;
			}
			s_nStartLBA = _tcstol(argv[5], &endptr, 10);
			if (*endptr) {
				OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
				return FALSE;
			}
			s_nEndLBA = _tcstol(argv[6], &endptr, 10);
			if (*endptr) {
				OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
				return FALSE;
			}
			pExtArg->dwSubAddionalNum = 1;
			for (INT i = 8; i <= argc; i++) {
				cmdLen = _tcslen(argv[i - 1]);
				if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/q"), 2)) {
					pExtArg->byQuiet = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/a"), 2)) {
					if (!SetOptionA(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/be"), 3)) {
					if (!SetOptionBe(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/d8"), 3)) {
					pExtArg->byBe = FALSE;
					pExtArg->byD8 = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/c2"), 3)) {
					if (!SetOptionC2(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/r"), 2)) {
					pExtArg->byReverse = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/sf"), 3)) {
					if (!SetOptionSf(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/ss"), 3)) {
					pExtArg->byScanProtectViaSector = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/np"), 3)) {
					pExtArg->bySkipSubP = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/nq"), 3)) {
					pExtArg->bySkipSubQ = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/nr"), 3)) {
					pExtArg->bySkipSubRtoW = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/s"), 2)) {
					if (!SetOptionS(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else {
					OutputErrorString(_T("Unknown option: [%s]\n"), argv[i - 1]);
					return FALSE;
				}
			}
			if (!_tcsncmp(argv[1], _T("data"), 4)) {
				*pExecType = data;
			}
			else if (!_tcsncmp(argv[1], _T("audio"), 5)) {
				*pExecType = audio;
			}
			printAndSetPath(argv[3], pszFullPath);
		}
		else if (argc == 4) {
			if (_tcslen(argv[1]) == 2 && !_tcsncmp(argv[1], _T("fd"), 2)) {
				*pExecType = fd;
				printAndSetPath(argv[3], pszFullPath);
			}
			else {
				OutputErrorString(_T("Invalid argument\n"));
				return FALSE;
			}
		}
		else if (argc == 3) {
			cmdLen = _tcslen(argv[1]);
			if (cmdLen == 4 && !_tcsncmp(argv[1], _T("stop"), 4)) {
				*pExecType = stop;
			}
			else if (cmdLen == 5 && !_tcsncmp(argv[1], _T("start"), 5)) {
				*pExecType = start;
			}
			else if (cmdLen == 5 && !_tcsncmp(argv[1], _T("eject"), 5)) {
				*pExecType = eject;
			}
			else if (cmdLen == 5 && !_tcsncmp(argv[1], _T("close"), 5)) {
				*pExecType = closetray;
			}
			else if (cmdLen == 5 && !_tcsncmp(argv[1], _T("reset"), 5)) {
				*pExecType = reset;
			}
			else if (cmdLen == 3 && !_tcsncmp(argv[1], _T("sub"), 3)) {
				*pExecType = sub;
				printAndSetPath(argv[2], pszFullPath);
			}
			else {
				OutputErrorString(_T("Invalid argument\n"));
				return FALSE;
			}
		}
		else {
			if (argc > 1) {
				OutputErrorString(_T("Invalid argument\n"));
			}
			return FALSE;
		}
	}
	return TRUE;
}

int createCmdFile(int argc, _TCHAR* argv[], _TCHAR* pszFullPath)
{
	if (argc >= 4) {
		FILE* fpCmd = CreateOrOpenFile(
			pszFullPath, _T("_cmd"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0);
		if (!fpCmd) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		fwrite(_T(__DATE__), _tcslen(_T(__DATE__)), sizeof(_TCHAR), fpCmd);
		_fputts(_T(" "), fpCmd);
		fwrite(_T(__TIME__), _tcslen(_T(__TIME__)), sizeof(_TCHAR), fpCmd);
		_fputts(_T("\n"), fpCmd);
		for (int i = 0; i < argc; i++) {
			fwrite(argv[i], _tcslen(argv[i]), sizeof(_TCHAR), fpCmd);
			_fputts(_T(" "), fpCmd);
		}
		FcloseAndNull(fpCmd);
	}
	return TRUE;
}

void printUsage(void)
{
	OutputString(
		_T("Usage\n")
		_T("\tcd <DriveLetter> <Filename> <DriveSpeed(0-72)> [/q] [/a (val)]\n")
		_T("\t   [/be (str) or /d8] [/c2 (val1) (val2) (val3)] [/f] [/m] [/p]\n")
		_T("\t   [/raw] [/sf (val)] [/ss] [/np] [/nq] [/nr] [/ns] [/s (val)]\n")
		_T("\t\tRipping a CD from A to Z\n")
		_T("\t\tFor PLEXTOR or drive that can scramble ripping\n")
		_T("\tdata <DriveLetter> <Filename> <DriveSpeed(0-72)> <StartLBA> <EndLBA+1>\n")
		_T("\t     [/q] [/be (str) or /d8] [/c2 (val1) (val2) (val3)] [/sf (val)]\n")
		_T("\t     [/ss] [/r] [/np] [/nq] [/nr] [/ns] [/s (val)]\n")
		_T("\t\tRipping a CD from start to end (using 'all' flag)\n")
		_T("\t\tFor no PLEXTOR or drive that can't scramble ripping\n")
		_T("\taudio <DriveLetter> <Filename> <DriveSpeed(0-72)> <StartLBA> <EndLBA+1>\n")
		_T("\t      [/q] [/a (val)] [/be (str) or /d8] [/c2 (val1) (val2) (val3)]\n")
		_T("\t      [/sf (val)] [/np] [/nq] [/nr] [/ns] [/s (val)]\n")
		_T("\t\tRipping a CD from start to end (using 'cdda' flag)\n")
		_T("\t\tFor dumping a lead-in, lead-out mainly\n")
		_T("\tgd <DriveLetter> <Filename> <DriveSpeed(0-72)> [/q] [/be (str) or /d8]\n")
		_T("\t   [/c2 (val1) (val2) (val3)] [/np] [/nq] [/nr] [/ns] [/s (val)]\n")
		_T("\t\tRipping a HD area of GD from A to Z\n")
		_T("\tdvd <DriveLetter> <Filename> <DriveSpeed(0-16)> [/c] [/f] [/q]\n")
	);
	_tsystem(_T("pause"));
	OutputString(
		_T("\t\tRipping a DVD from A to Z\n")
		_T("\tfd <DriveLetter> <Filename>\n")
		_T("\t\tRipping a floppy disk\n")
		_T("\tstop <DriveLetter>\n")
		_T("\t\tSpin off the disc\n")
		_T("\tstart <DriveLetter>\n")
		_T("\t\tSpin up the disc\n")
		_T("\teject <DriveLetter>\n")
		_T("\t\tEject the tray\n")
		_T("\tclose <DriveLetter>\n")
		_T("\t\tClose the tray\n")
		_T("\treset <DriveLetter>\n")
		_T("\t\tReset the drive (Only PLEXTOR)\n")
		_T("\tsub <Subfile>\n")
		_T("\t\tParse CloneCD sub file and output to readable format\n")
		_T("Option (generic)\n")
		_T("\t/f\tUse 'Force Unit Access' flag to defeat the cache (very slow)\n")
		_T("\t/q\tDisable beep\n")
		_T("Option (for CD read mode)\n")
		_T("\t/a\tAdd CD offset manually (Only Audio CD)\n")
		_T("\t\t\tval\tsamples value\n")
		_T("\t/be\tUse 0xbe as the opcode for Reading CD forcibly\n")
		_T("\t\t\tstr\t raw: sub channel mode is raw (default)\n")
		_T("\t\t\t   \tpack: sub channel mode is pack\n")
	);
	_tsystem(_T("pause"));
	OutputString(
		_T("\t/d8\tUse 0xd8 as the opcode for Reading CD forcibly\n")
		_T("\t/c2\tContinue reading CD to recover C2 error existing sector\n")
		_T("\t\t\tval1\tvalue to reread (default: 1024)\n")
		_T("\t\t\tval2\tvalue to fix a C2 error (default: 4096)\n")
		_T("\t\t\tval3\tvalue to reread speed (default: 4)\n")
		_T("\t/m\tUse if MCN exists in the first pregap sector of the track\n")
		_T("\t\t\tFor some PC-Engine\n")
		_T("\t/p\tRipping the AMSF from 00:00:00 to 00:01:74\n")
		_T("\t\t\tFor SagaFrontier Original Sound Track (Disc 3) etc.\n")
		_T("\t\t\tSupport drive: PLEXTOR PX-W5224, PREMIUM, PREMIUM2\n")
		_T("\t\t\t               PX-704, 708, 712, 714, 716, 755, 760\n")
		_T("\t/r\tRead CD from the reverse\n")
		_T("\t\t\tFor Alpha-Disc, Tages (very slow)\n")
		_T("\t/raw\tRead the lead-out of 1st session and the lead-in of 2nd session\n")
		_T("\t\t\tFor Multi-session\n")
		_T("\t/sf\tScan file to detect protect. If reading error exists,\n")
		_T("\t   \tcontinue reading and ignore c2 error on specific sector\n")
		_T("\t\t\tFor CodeLock, LaserLock, RingProtect, RingPROTECH\n")
		_T("\t\t\t     SafeDisc, SmartE, CD.IDX, ProtectCD-VOB, CDS300\n")
		_T("\t\t\tval\ttimeout value (default: 60)\n")
		_T("\t/ss\tScan sector to detect protect. If reading error exists,\n")
		_T("\t   \tcontinue reading and ignore c2 error on specific sector\n")
		_T("\t\t\tFor ProtectCD-VOB\n")
		_T("Option (for CD SubChannel)\n")
	);
	_tsystem(_T("pause"));
	OutputString(
		_T("\t/np\tNot fix SubP\n")
		_T("\t/nq\tNot fix SubQ\n")
		_T("\t/nr\tNot fix SubRtoW\n")
		_T("\t/nl\tNot fix SubQ (RMSF, AMSF, CRC) (LBA 14100 - 16199)\n")
		_T("\t   \t                               (LBA 42000 - 44399)\n")
		_T("\t\t\tFor PlayStation LibCrypt\n")
		_T("\t/ns\tNot fix SubQ (RMSF, AMSF, CRC) (LBA 0 - 7, 5000 - 18799)\n")
		_T("\t   \t                            or (LBA 30800 - 34799)\n")
		_T("\t   \t                            or (LBA 40000 - 45799)\n")
		_T("\t\t\tFor SecuROM\n")
		_T("\t/s\tUse if it reads subchannel precisely\n")
		_T("\t\t\tval\t0: no read next sub (fast, but lack precision)\n")
		_T("\t\t\t   \t1: read next sub (normal, this val is default)\n")
		_T("\t\t\t   \t2: read next & next next sub (slow, precision)\n")
		_T("Option (for DVD)\n")
		_T("\t/c\tLog Copyright Management Information\n")
		);
	_tsystem(_T("pause"));
}

int printSeveralInfo()
{
	if (!OutputWindowsVersion()) {
		return FALSE;
	}
	OutputString(_T("AppVersion\n"));
#ifdef _WIN64
	OutputString(_T("\tx64, "));
#else
	OutputString(_T("\tx86, "));
#endif
#ifdef UNICODE
	OutputString(_T("UnicodeBuild, "));
#else
	OutputString(_T("AnsiBuild, "));
#endif
	OutputString(_T("%s %s\n"), _T(__DATE__), _T(__TIME__));
	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#ifdef UNICODE
	if (_setmode(_fileno(stdin), _O_U8TEXT) == -1) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return EXIT_FAILURE;
	}
	if (_setmode(_fileno(stdout), _O_U8TEXT) == -1) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return EXIT_FAILURE;
	}
	if (_setmode(_fileno(stderr), _O_U8TEXT) == -1) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return EXIT_FAILURE;
	}
#endif
	int nRet = printSeveralInfo();
	if (nRet) {
		EXEC_TYPE execType;
		EXT_ARG extArg = { 0 };
		_TCHAR szFullPath[_MAX_PATH + 1] = { 0 };
		if (!checkArg(argc, argv, &execType, &extArg, szFullPath)) {
			printUsage();
			nRet = FALSE;
		}
		else {
			time_t now;
			struct tm* ts;
			_TCHAR szBuf[128] = { 0 };

			now = time(NULL);
			ts = localtime(&now);
			_tcsftime(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), _T("%Y-%m-%d(%a) %H:%M:%S"), ts);
			OutputString(_T("Start time: %s\n"), szBuf);

			nRet = createCmdFile(argc, argv, szFullPath);
			if (nRet) {
				nRet = exec(argv, &execType, &extArg, szFullPath);
			}

			now = time(NULL);
			ts = localtime(&now);
			_tcsftime(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), _T("%Y-%m-%d(%a) %H:%M:%S"), ts);
			OutputString(_T("End time: %s\n"), szBuf);
		}
		if (!extArg.byQuiet) {
			nRet = soundBeep(nRet);
		}
	}
#ifdef _DEBUG
	_tsystem(_T("pause"));
#endif
	return nRet = nRet == TRUE ? EXIT_SUCCESS : EXIT_FAILURE;
}

