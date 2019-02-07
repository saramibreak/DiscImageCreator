// DiscImageCreator.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//
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
#include "buildDateTime.h"
#include "struct.h"
#include "calcHash.h"
#include "check.h"
#include "execIoctl.h"
#include "execScsiCmd.h"
#include "execScsiCmdforCD.h"
#include "execScsiCmdforCDCheck.h"
#include "execScsiCmdforDVD.h"
#include "get.h"
#include "init.h"
#include "output.h"
#include "xml.h"
#include "_external/prngcd.h"

#define DEFAULT_REREAD_VAL			(4000)
#define DEFAULT_CACHE_DELETE_VAL	(1)
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
static DWORD s_dwFix = 0;
static DWORD s_dwSpeed = 0;
static LONG s_nStartLBA = 0;
static LONG s_nEndLBA = 0;

#define playtime (200)
#define c4 (262)
#define d4 (294)
#define e4 (330)
#define f4 (349)
#define g4 (392)
#define a4 (440)
#define b4 (494)
#define c5 (523)

int soundBeep(int nRet)
{
	if (nRet) {
		if (!Beep(c4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!Beep(d4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!Beep(e4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!Beep(f4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!Beep(g4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!Beep(a4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!Beep(b4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!Beep(c5, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	else {
		if (!Beep(c5, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!Beep(b4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!Beep(a4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!Beep(g4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!Beep(f4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!Beep(e4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!Beep(d4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!Beep(c4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
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
	else if (*pExecType == mds) {
		bRet = WriteParsingMdsfile(pszFullPath);
	}
	else if (*pExecType == merge) {
		bRet = OutputMergedFile(pszFullPath, argv[3]);
	}
	else {
		DEVICE device = {};
#ifdef _WIN32
		device.byDriveLetter = (BYTE)(argv[2][0]);
#else
		strncpy(device.drivepath, argv[2], sizeof(device.drivepath));
#endif
		if (!GetHandle(&device)) {
			return FALSE;
		}

		// 1st: set TimeOutValue here (because use ScsiPassThroughDirect)
		if (pExtArg->byScanProtectViaFile) {
			device.dwTimeOutValue = pExtArg->dwTimeoutNum;
			GetFilenameToSkipError(pExtArg->FILE.readError);
			GetFilenameToFixError(pExtArg->FILE.edceccError);
		}
		else {
			device.dwTimeOutValue = DEFAULT_SPTD_TIMEOUT_VAL;
		}
		if (*pExecType == stop) {
			bRet = StartStopUnit(pExtArg, &device, STOP_UNIT_CODE, STOP_UNIT_CODE);
			pExtArg->byQuiet = TRUE;
		}
		else if (*pExecType == start) {
			bRet = StartStopUnit(pExtArg, &device, START_UNIT_CODE, STOP_UNIT_CODE);
			pExtArg->byQuiet = TRUE;
		}
		else if (*pExecType == ejecttray) {
			bRet = StartStopUnit(pExtArg, &device, STOP_UNIT_CODE, START_UNIT_CODE);
			pExtArg->byQuiet = TRUE;
		}
		else if (*pExecType == closetray) {
			bRet = StartStopUnit(pExtArg, &device, START_UNIT_CODE, START_UNIT_CODE);
			pExtArg->byQuiet = TRUE;
		}
		else if (*pExecType == reset) {
			bRet = Reset(pExtArg, &device);
			pExtArg->byQuiet = TRUE;
		}
		else {
			DISC discData = {};
			PDISC pDisc = &discData;
			MAIN_HEADER mainHeader = {};
			FILE* fpCcd = NULL;
			FILE* fpC2 = NULL;

			try {
#ifndef _DEBUG
				if (*pExecType != drivespeed) {
					// 2nd: create logfile here (because logging all working)
					if (!InitLogFile(pExecType, pExtArg, pszFullPath)) {
						throw FALSE;
					}
				}
#endif
				if (!TestUnitReady(pExtArg, &device)) {
					throw FALSE;
				}
				if (*pExecType == fd) {
					bRet = DiskGetMediaTypes(&device, pszFullPath);
				}
				else {
					if (!ReadDriveInformation(pExecType, pExtArg, &device, pDisc, s_dwSpeed)) {
						throw FALSE;
					}
					if (*pExecType == drivespeed) {
						pExtArg->byQuiet = TRUE;
						throw TRUE;
					}
					if (*pExecType == cd || *pExecType == swap || *pExecType == gd || *pExecType == data || *pExecType == audio) {
						if (discData.SCSI.wCurrentMedia == ProfileCdrom ||
							discData.SCSI.wCurrentMedia == ProfileCdRecordable ||
							discData.SCSI.wCurrentMedia == ProfileCdRewritable ||
							(discData.SCSI.wCurrentMedia == ProfileInvalid && (*pExecType == gd))) {
							// call this here because "Invalid TOC" occurs by GD-ROM
							if (!ReadTOC(pExtArg, pExecType, &device, pDisc)) {
								throw FALSE;
							}

							InitMainDataHeader(pExecType, pExtArg, &mainHeader, (INT)s_nStartLBA);
							if (!InitSubData(pExecType, &pDisc)) {
								throw FALSE;
							}
							if (!InitTocFullData(pExecType, &pDisc)) {
								throw FALSE;
							}
							if (!InitTocTextData(pExecType, &device, &pDisc)) {
								throw FALSE;
							}
							if (pExtArg->byScanProtectViaFile || pExtArg->byIntentionalSub) {
								if (!InitProtectData(&pDisc)) {
									throw FALSE;
								}
							}
							make_scrambled_table();
							make_crc_table();
							make_crc16_table();
#if 0
							make_crc6_table();
#endif
							CDFLAG::_READ_CD::_ERROR_FLAGS c2 = CDFLAG::_READ_CD::NoC2;
							ReadCDForCheckingByteOrder(pExtArg, &device, &c2);
							if (pExtArg->byC2) {
								if (device.FEATURE.byC2ErrorData && c2 != CDFLAG::_READ_CD::NoC2) {
									if (NULL == (fpC2 = CreateOrOpenFile(
										pszFullPath, NULL, NULL, NULL, NULL, _T(".c2"), _T("wb"), 0, 0))) {
										OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
										throw FALSE;
									}
									if (!InitC2(&pDisc)) {
										throw FALSE;
									}
								}
								if (pExtArg->dwSubAddionalNum == 0 && *pExecType != gd) {
									OutputString(
										_T("[WARNING] /c2 and /s 0 can't use together. Changed /s 0 to /s 1.\n"));
									pExtArg->dwSubAddionalNum = 1;
								}
							}
							else {
								OutputString(
									_T("[WARNING] /c2 isn't set. The result of dumping may be incorrect if c2 error exists.\n"));
							}

							if (discData.SCSI.wCurrentMedia == ProfileCdrom) {
								ReadDiscInformation(pExtArg, &device);
							}
							else if (discData.SCSI.wCurrentMedia == ProfileCdRecordable ||
								discData.SCSI.wCurrentMedia == ProfileCdRewritable) {
								ReadTOCAtip(pExtArg, &device);
							}

							// This func needs the TOC
							if (!ReadCDForSearchingOffset(pExecType, pExtArg, &device, pDisc)) {
								throw FALSE;
							}
							if (pExtArg->byC2 && device.byPlxtrDrive == _PLXTR_DRIVE_TYPE::No) {
								// Re-set c2 flag
								c2 = device.supportedC2Type;
							}
							// needs to call ReadCDForSearchingOffset
							if (pDisc->SUB.nSubChannelOffset && pExtArg->dwSubAddionalNum == 0) {
								OutputString(
									_T("[INFO] SubChannel offset exists in this drive. Changed /s 0 to /s 1.\n"));
								pExtArg->dwSubAddionalNum = 1;
							}

							DISC_PER_SECTOR discPerSector = {};
							memcpy(&discPerSector.mainHeader, &mainHeader, sizeof(MAIN_HEADER));
							CDFLAG::_READ_CD::_EXPECTED_SECTOR_TYPE flg = CDFLAG::_READ_CD::All;

							if (*pExecType == gd) {
								flg = CDFLAG::_READ_CD::CDDA;
								if (IsValidPlextorDrive(&device) && pExtArg->dwSubAddionalNum == 0) {
									pExtArg->dwSubAddionalNum = 1;
								}
								// This func needs the combined offsets
								if (!ReadGDForTOC(pExtArg, &device, pDisc)) {
									throw FALSE;
								}
								if (!ReadTOCText(pExtArg, &device, &discData, NULL)) {
									throw FALSE;
								}
								if (!ReadGDForCheckingSubQAdr(pExtArg, &device, &discData, &discPerSector)) {
									throw FALSE;
								}
							}
							else {
								if (*pExecType == cd || *pExecType == swap) {
									// This func needs the combined offsets
									if (!ReadCDForCheckingReadInOut(pExecType, pExtArg, &device, pDisc)) {
										throw FALSE;
									}
									// open ccd here because ccd is written by ReadTOCFull
									if (NULL == (fpCcd = CreateOrOpenFile(pszFullPath, NULL,
										NULL, NULL, NULL, _T(".ccd"), _T(WFLAG), 0, 0))) {
										OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
										throw FALSE;
									}
								}
								if (*pExecType != swap) {
									if (!ReadTOCFull(pExtArg, &device, &discData, &discPerSector, fpCcd)) {
										throw FALSE;
									}
								}
							}
							if (*pExecType != swap) {
								if (!ReadCDCheck(pExecType, pExtArg, &device, pDisc, flg)) {
									throw FALSE;
								}
							}
							if (*pExecType == cd) {
								bRet = ReadCDAll(pExecType, pExtArg, &device, pDisc
									, &discPerSector, c2, pszFullPath, fpCcd, fpC2);
							}
							else if (*pExecType == swap) {
								bRet = ReadCDForSwap(pExecType, pExtArg, &device, pDisc, &discPerSector
									, c2, pszFullPath, 0, pDisc->SCSI.nAllLength, CDFLAG::_READ_CD::CDDA, fpCcd, fpC2);
							}
							else if (*pExecType == gd) {
#if 0
								CHAR tmpFname[_MAX_FNAME];
								CHAR tmpPath[_MAX_PATH];
								_tcsncpy(tmpFname, s_szFname, _MAX_FNAME);
								_tcsncat(tmpFname, "_pre", 4);
								_tmakepath(tmpPath, s_szDrive, s_szDir, tmpFname, s_szExt);

								bRet = ReadCDPartial(pExecType, pExtArg, &device, pDisc, &discPerSector
									, c2, tmpPath, 0, 38700, CDFLAG::_READ_CD::CDDA, fpC2);
#endif
								bRet = ReadCDPartial(pExecType, pExtArg, &device, pDisc, &discPerSector
									, c2, pszFullPath, FIRST_LBA_FOR_GD, 549149 + 1, CDFLAG::_READ_CD::CDDA, fpC2);
							}
							else if (*pExecType == data) {
								bRet = ReadCDPartial(pExecType, pExtArg, &device, pDisc, &discPerSector
									, c2, pszFullPath, (INT)s_nStartLBA, (INT)s_nEndLBA, CDFLAG::_READ_CD::All, fpC2);
							}
							else if (*pExecType == audio) {
								bRet = ReadCDPartial(pExecType, pExtArg, &device, pDisc, &discPerSector
									, c2, pszFullPath, (INT)s_nStartLBA, (INT)s_nEndLBA, CDFLAG::_READ_CD::CDDA, fpC2);
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
							discData.SCSI.wCurrentMedia == ProfileDvdPlusR ||
							discData.SCSI.wCurrentMedia == ProfileDvdPlusRWDualLayer ||
							discData.SCSI.wCurrentMedia == ProfileDvdPlusRDualLayer ||
							discData.SCSI.wCurrentMedia == ProfileHDDVDRom ||
							discData.SCSI.wCurrentMedia == ProfileHDDVDRecordable ||
							discData.SCSI.wCurrentMedia == ProfileHDDVDRam ||
							discData.SCSI.wCurrentMedia == ProfileHDDVDRewritable ||
							discData.SCSI.wCurrentMedia == ProfileHDDVDRDualLayer ||
							discData.SCSI.wCurrentMedia == ProfileHDDVDRWDualLayer
							) {
							bRet = ReadDiscStructure(pExecType, pExtArg, &device, &discData, pszFullPath);

							if (pExtArg->byCmi) {
								bRet = ReadDVDForCMI(pExtArg, &device, &discData);
							}
							if (bRet) {
								if (pExtArg->byRawDump) {
									while(1) {
										if (pExtArg->byFix) {
											pDisc->DVD.dwFixNum = s_dwFix;
										}
										bRet = ReadDVDRaw(pExtArg, &device, &discData, pszFullPath);
										if (pExtArg->byFix && bRet > 6) {
											s_dwFix = (DWORD)bRet;
										}
										else {
											// 0 == no error
											if (bRet == 0) {
												bRet = TRUE;
											}
											break;
										}
									}
								}
								else {
									bRet = ReadDVD(pExecType, pExtArg, &device, &discData, pszFullPath);
								}
							}
						}
						else {
							OutputString(_T("Wrong command. The disc isn't DVD, DVD-R, DVD-RW\n"));
						}
					}
					else if (*pExecType == xbox) {
						bRet = ReadXboxDVD(pExecType, pExtArg, &device, pDisc, pszFullPath);
					}
					else if (*pExecType == xboxswap || *pExecType == xgd2swap || *pExecType == xgd3swap) {
						bRet = ReadXboxDVDBySwap(pExecType, pExtArg, &device, pDisc, pszFullPath);
					}
					else if (*pExecType == bd) {
						if (discData.SCSI.wCurrentMedia == ProfileBDRom ||
							discData.SCSI.wCurrentMedia == ProfileBDRSequentialWritable ||
							discData.SCSI.wCurrentMedia == ProfileBDRRandomWritable ||
							discData.SCSI.wCurrentMedia == ProfileBDRewritable
							) {
							if (!ReadTOC(pExtArg, pExecType, &device, &discData)) {
								throw FALSE;
							}
							bRet = ReadDiscStructure(pExecType, pExtArg, &device, &discData, pszFullPath);
							if (bRet) {
								bRet = ReadDVD(pExecType, pExtArg, &device, &discData, pszFullPath);
							}
						}
						else {
							OutputString(_T("Wrong command. The disc isn't BD\n"));
						}
					}
				}
				if (bRet && (*pExecType != audio && *pExecType != data)) {
					bRet = ReadWriteDat(pExecType, pExtArg, pDisc
						, pszFullPath, s_szDrive, s_szDir, s_szFname, FALSE);
					if (pDisc->SUB.byDesync) {
						bRet = ReadWriteDat(pExecType, pExtArg, pDisc
							, pszFullPath, s_szDrive, s_szDir, s_szFname, TRUE);
					}
				}
			}
			catch (BOOL bErr) {
				bRet = bErr;
			}
			FlushLog();
			if (*pExecType == cd || *pExecType == swap || *pExecType == gd ||
				*pExecType == data || *pExecType == audio || *pExecType == bd) {
				TerminateLBAPerTrack(&pDisc);
				if (*pExecType == cd || *pExecType == swap || *pExecType == gd ||
					*pExecType == data || *pExecType == audio) {
					TerminateSubData(pExecType, &pDisc);
					TerminateTocFullData(&pDisc);
					TerminateTocTextData(pExecType, &device, &pDisc);
					FcloseAndNull(fpCcd);
				}
			}
			if (pExtArg->byScanProtectViaFile || pExtArg->byIntentionalSub) {
				TerminateProtectData(&pDisc);
			}
			if (pExtArg->byC2 && device.FEATURE.byC2ErrorData) {
				FcloseAndNull(fpC2);
				TerminateC2(&pDisc);
			}
#ifndef _DEBUG
			TerminateLogFile(pExecType, pExtArg);
#endif
		}
		if (device.hDevice && !CloseHandle(device.hDevice)) {
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
		if (s_szExt[0] && _tcslen(pszFullPath) + _tcslen(s_szExt) < _MAX_PATH) {
			_tcsncat(pszFullPath, s_szExt, _tcslen(s_szExt));
		}
	}
	else {
		_tcsncpy(pszFullPath, s_szDrive, _tcslen(s_szDrive));
		_tcsncat(pszFullPath, s_szDir, _tcslen(s_szDir));
		if (!PathFileExists(pszFullPath)) {
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
		_tcsncpy(pszFullPath, szPathFromArg, _MAX_PATH);
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

int SetOptionSk(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->dwSkipSectors = _tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
			return FALSE;
		}
		if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
			pExtArg->dwSkipSectors2 = _tcstoul(argv[(*i)++], &endptr, 10);
			if (*endptr) {
				OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
				return FALSE;
			}
		}
	}
	else {
		pExtArg->dwSkipSectors = 0;
		OutputString(_T("/sk val is omitted. set [%ld]\n"), pExtArg->dwSkipSectors);
	}
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
		OutputString(_T("/s val is omitted. set [%ld]\n"), pExtArg->dwSubAddionalNum);
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

int SetOptionF(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	pExtArg->byFua = TRUE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->dwCacheDelNum = _tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->dwCacheDelNum = DEFAULT_CACHE_DELETE_VAL;
		OutputString(
			_T("/f val is omitted. set [%d]\n"), DEFAULT_CACHE_DELETE_VAL);
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
			pExtArg->nC2RereadingType = _tcstol(argv[(*i)++], &endptr, 10);
			if (*endptr) {
				OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
				return FALSE;
			}
			if (pExtArg->nC2RereadingType != 0) {
				if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1) && pExtArg->nC2RereadingType == 1) {
					pExtArg->nStartLBAForC2 = _tcstol(argv[(*i)++], &endptr, 10);
					if (*endptr) {
						OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
						return FALSE;
					}
					if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
						pExtArg->nEndLBAForC2 = _tcstol(argv[(*i)++], &endptr, 10);
						if (*endptr) {
							OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
							return FALSE;
						}
					}
					else {
						pExtArg->nEndLBAForC2 = 0;
						OutputString(_T("/c2 val4 is omitted. set [%d]\n"), 0);
					}
				}
				else {
					pExtArg->nStartLBAForC2 = 0;
					OutputString(_T("/c2 val3 is omitted. set [%d]\n"), 0);
					pExtArg->nEndLBAForC2 = 0;
					OutputString(_T("/c2 val4 is omitted. set [%d]\n"), 0);
				}
			}
		}
		else {
			pExtArg->nC2RereadingType = 0;
			OutputString(_T("/c2 val2 is omitted. set [%d]\n"), 0);
		}
	}
	else {
		pExtArg->dwMaxRereadNum = DEFAULT_REREAD_VAL;
		OutputString(_T("/c2 val1 is omitted. set [%d]\n"), DEFAULT_REREAD_VAL);
		pExtArg->nC2RereadingType = 0;
		OutputString(_T("/c2 val2 is omitted. set [%d]\n"), 0);
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
		if (argc >= 5 && ((cmdLen == 2 && !_tcsncmp(argv[1], _T("cd"), 2)) ||
			(cmdLen == 4 && !_tcsncmp(argv[1], _T("swap"), 4)))) {
			if (cmdLen == 2) {
				*pExecType = cd;
			}
			else if (cmdLen == 4) {
				*pExecType = swap;
			}
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
					if (!SetOptionF(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
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
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/am"), 3)) {
					pExtArg->byScanAntiModStr = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/ms"), 3)) {
					pExtArg->byMultiSession = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/vn"), 3)) {
					pExtArg->byVideoNow = TRUE;
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
				else if (*pExecType == swap && cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/74"), 3)) {
					pExtArg->by74Min = TRUE;
				}
				else {
					OutputErrorString(_T("Unknown option: [%s]\n"), argv[i - 1]);
					return FALSE;
				}
			}
			printAndSetPath(argv[3], pszFullPath);
		}
		else if (argc >= 5 && cmdLen == 2 && !_tcsncmp(argv[1], _T("gd"), 2)) {
			*pExecType = gd;
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
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/f"), 2)) {
					if (!SetOptionF(argc, argv, pExtArg, &i)) {
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
			printAndSetPath(argv[3], pszFullPath);
		}
		else if (argc >= 7 && ((cmdLen == 4 && !_tcsncmp(argv[1], _T("data"), 4)) ||
			(cmdLen == 5 && !_tcsncmp(argv[1], _T("audio"), 5)))) {
			if (cmdLen == 4) {
				*pExecType = data;
			}
			else if (cmdLen == 5) {
				*pExecType = audio;
			}
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
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/am"), 3)) {
					pExtArg->byScanAntiModStr = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/ms"), 3)) {
					pExtArg->byMultiSession = TRUE;
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
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/sk"), 3)) {
					if (!SetOptionSk(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else {
					OutputErrorString(_T("Unknown option: [%s]\n"), argv[i - 1]);
					return FALSE;
				}
			}
			printAndSetPath(argv[3], pszFullPath);
		}
		else if (argc >= 5 && cmdLen == 3 && !_tcsncmp(argv[1], _T("dvd"), 3)) {
			*pExecType = dvd;
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
					if (!SetOptionF(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 4 && !_tcsncmp(argv[i - 1], _T("/raw"), 4)) {
					pExtArg->byRawDump = TRUE;
				}
				else if (cmdLen == 4 && !_tcsncmp(argv[i - 1], _T("/fix"), 4)) {
					pExtArg->byFix = TRUE;
					s_dwFix = _tcstoul(argv[i++], &endptr, 10);
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/re"), 3)) {
					pExtArg->byResume = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/q"), 2)) {
					pExtArg->byQuiet = TRUE;
				}
				else {
					OutputErrorString(_T("Unknown option: [%s]\n"), argv[i - 1]);
					return FALSE;
				}
			}
			printAndSetPath(argv[3], pszFullPath);
		}
		else if (argc >= 5 && ((cmdLen == 2 && !_tcsncmp(argv[1], _T("bd"), 2)) ||
			(cmdLen == 4 && !_tcsncmp(argv[1], _T("xbox"), 4)))) {
			if (cmdLen == 2) {
				*pExecType = bd;
			}
			else if (cmdLen == 4) {
				*pExecType = xbox;
			}
			s_dwSpeed = _tcstoul(argv[4], &endptr, 10);
			if (*endptr) {
				OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
				return FALSE;
			}
			for (INT i = 6; i <= argc; i++) {
				cmdLen = _tcslen(argv[i - 1]);
				if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/f"), 2)) {
					if (!SetOptionF(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/q"), 2)) {
					pExtArg->byQuiet = TRUE;
				}
				else {
					OutputErrorString(_T("Unknown option: [%s]\n"), argv[i - 1]);
					return FALSE;
				}
			}
			printAndSetPath(argv[3], pszFullPath);
		}
		else if (argc >= 21 && ((cmdLen == 8 && !_tcsncmp(argv[1], _T("xboxswap"), 8)))) {
			*pExecType = xboxswap;
			s_dwSpeed = _tcstoul(argv[4], &endptr, 10);
			if (*endptr) {
				OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
				return FALSE;
			}
			for (INT i = 5; i < 21; i++) {
				pExtArg->dwSecuritySector[i - 5] = _tcstoul(argv[i], &endptr, 10);
				if (*endptr) {
					OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
					return FALSE;
				}
			}
			for (INT i = 21; i < argc; i++) {
				cmdLen = _tcslen(argv[i]);
				if (cmdLen == 2 && !_tcsncmp(argv[i], _T("/f"), 2)) {
					if (!SetOptionF(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i], _T("/q"), 2)) {
					pExtArg->byQuiet = TRUE;
				}
				else {
					OutputErrorString(_T("Unknown option: [%s]\n"), argv[i]);
					return FALSE;
				}
			}
			printAndSetPath(argv[3], pszFullPath);
		}
		else if (argc >= 8 && ((cmdLen == 8 && (!_tcsncmp(argv[1], _T("xgd2swap"), 8) || !_tcsncmp(argv[1], _T("xgd3swap"), 8))))) {
			pExtArg->nAllSectors = (INT)_tcstoul(argv[5], &endptr, 10);
			if (*endptr) {
				OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
				return FALSE;
			}
			if (pExtArg->nAllSectors < 4000000) {
				*pExecType = xgd2swap;
			}
			else {
				*pExecType = xgd3swap;
			}
			s_dwSpeed = _tcstoul(argv[4], &endptr, 10);
			if (*endptr) {
				OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
				return FALSE;
			}
			for (INT i = 6; i < 8; i++) {
				pExtArg->dwSecuritySector[i - 6] = _tcstoul(argv[i], &endptr, 10);
				if (*endptr) {
					OutputErrorString(_T("[%s] is invalid argument. Please input integer.\n"), endptr);
					return FALSE;
				}
			}
			for (INT i = 8; i < argc; i++) {
				cmdLen = _tcslen(argv[i]);
				if (cmdLen == 2 && !_tcsncmp(argv[i], _T("/f"), 2)) {
					if (!SetOptionF(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i], _T("/q"), 2)) {
					pExtArg->byQuiet = TRUE;
				}
				else {
					OutputErrorString(_T("Unknown option: [%s]\n"), argv[i]);
					return FALSE;
				}
			}
			printAndSetPath(argv[3], pszFullPath);
		}
		else if (argc == 4) {
			cmdLen = _tcslen(argv[1]);
			if (cmdLen == 2 && !_tcsncmp(argv[1], _T("fd"), 2)) {
				*pExecType = fd;
				printAndSetPath(argv[3], pszFullPath);
			}
			else if (cmdLen == 5 && !_tcsncmp(argv[1], _T("merge"), 5)) {
				*pExecType = merge;
				printAndSetPath(argv[2], pszFullPath);
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
				*pExecType = ejecttray;
			}
			else if (cmdLen == 5 && !_tcsncmp(argv[1], _T("close"), 5)) {
				*pExecType = closetray;
			}
			else if (cmdLen == 5 && !_tcsncmp(argv[1], _T("reset"), 5)) {
				*pExecType = reset;
			}
			else if (cmdLen == 2 && !_tcsncmp(argv[1], _T("ls"), 2)) {
				*pExecType = drivespeed;
			}
			else if (cmdLen == 3 && !_tcsncmp(argv[1], _T("sub"), 3)) {
				*pExecType = sub;
				printAndSetPath(argv[2], pszFullPath);
			}
			else if (cmdLen == 3 && !_tcsncmp(argv[1], _T("mds"), 3)) {
				*pExecType = mds;
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

int createCmdFile(int argc, _TCHAR* argv[], _TCHAR* pszFullPath, LPTSTR pszDateTime)
{
	if (argc >= 4) {
		FILE* fpCmd = CreateOrOpenFile(
			pszFullPath, _T("_cmd"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0);
		if (!fpCmd) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}

		_ftprintf(fpCmd, _T("%s"), pszDateTime);

		for (int i = 0; i < argc; i++) {
			_ftprintf(fpCmd, _T("%s "), argv[i]);
		}
		FcloseAndNull(fpCmd);
	}
	return TRUE;
}

int stopMessage(void)
{
#ifdef _WIN32
	int ret = _tsystem(_T("pause"));
#else
	int ret = _tsystem(_T("echo -n \"Hit Enter key to continue...\"; read a"));
#endif
	return ret;
}

int printUsage(void)
{
	OutputString(
		_T("Usage\n")
		_T("\tcd <DriveLetter> <Filename> <DriveSpeed(0-72)> [/q] [/a (val)]\n")
		_T("\t   [/be (str) or /d8] [/c2 (val1) (val2) (val3) (val4)] [/f (val)] [/m]\n")
		_T("\t   [/p] [/ms] [/vn] [/sf (val)] [/ss] [/np] [/nq] [/nr] [/ns] [/s (val)]\n")
		_T("\t\tDump a CD from A to Z\n")
		_T("\t\tFor PLEXTOR or drive that can scramble Dumping\n")
		_T("\tswap <DriveLetter> <Filename> <DriveSpeed(0-72)> [/q] [/a (val)]\n")
		_T("\t   [/be (str) or /d8] [/c2 (val1) (val2) (val3) (val4)] [/f (val)] [/m]\n")
		_T("\t   [/p] [/ms] [/sf (val)] [/ss] [/np] [/nq] [/nr] [/ns] [/s (val)] [/74]\n")
		_T("\t\tDump a CD from A to Z using swap trick\n")
		_T("\t\tFor no PLEXTOR or drive that can't scramble dumping\n")
		_T("\tdata <DriveLetter> <Filename> <DriveSpeed(0-72)> <StartLBA> <EndLBA+1>\n")
		_T("\t     [/q] [/be (str) or /d8] [/c2 (val1) (val2) (val3) (val4)]\n")
		_T("\t     [/sf (val)] [/sk (val1) (val2)] [/ss] [/r] [/np] [/nq] [/nr] [/ns] [/s (val)]\n")
		_T("\t\tDump a CD from start to end (using 'all' flag)\n")
		_T("\t\tFor no PLEXTOR or drive that can't scramble dumping\n")
		_T("\taudio <DriveLetter> <Filename> <DriveSpeed(0-72)> <StartLBA> <EndLBA+1>\n")
		_T("\t      [/q] [/a (val)] [/c2 (val1) (val2) (val3) (val4)]\n")
		_T("\t      [/be (str) or /d8] [/sf (val)] [/np] [/nq] [/nr] [/ns] [/s (val)]\n")
		_T("\t\tDump a CD from start to end (using 'cdda' flag)\n")
	);
	int ret = stopMessage();
	OutputString(
		_T("\t\tFor dumping a lead-in, lead-out mainly\n")
		_T("\tgd <DriveLetter> <Filename> <DriveSpeed(0-72)> [/q] [/be (str) or /d8]\n")
		_T("\t   [/c2 (val1) (val2) (val3) (val4)] [/np] [/nq] [/nr] [/ns] [/s (val)]\n")
		_T("\t\tDump a HD area of GD from A to Z\n")
		_T("\tdvd <DriveLetter> <Filename> <DriveSpeed(0-16)> [/c] [/f (val)] [/raw] [/q]\n")
		_T("\t\tDump a DVD from A to Z\n")
		_T("\txbox <DriveLetter> <Filename> <DriveSpeed(0-16)> [/f (val)] [/q]\n")
		_T("\t\tDump a disc from A to Z\n")
		_T("\txboxswap <DriveLetter> <Filename> <DriveSpeed(0-16)>\n")
		_T("\t                                  <StartLBAOfSecuritySector_1>\n")
		_T("\t                                  <StartLBAOfSecuritySector_2>\n")
		_T("\t                                                 :            \n")
		_T("\t                                  <StartLBAOfSecuritySector_16> [/f (val)] [/q]\n")
		_T("\t\tDump a Xbox disc from A to Z using swap trick\n")
		_T("\txgd2swap <DriveLetter> <Filename> <DriveSpeed(0-16)> <AllSectorLength>\n")
		_T("\t          <StartLBAOfSecuritySector_1> <StartLBAOfSecuritySector_2> [/f (val)] [/q]\n")
		_T("\t\tDump a XGD2 disc from A to Z using swap trick\n")
		_T("\txgd3swap <DriveLetter> <Filename> <DriveSpeed(0-16)> <AllSectorLength>\n")
		_T("\t          <StartLBAOfSecuritySector_1> <StartLBAOfSecuritySector_2> [/f (val)] [/q]\n")
		_T("\t\tDump a XGD3 disc from A to Z using swap trick\n")
		_T("\tbd <DriveLetter> <Filename> <DriveSpeed(0-12)> [/f (val)] [/q]\n")
		_T("\t\tDump a BD from A to Z\n")
		_T("\tfd <DriveLetter> <Filename>\n")
		_T("\t\tDump a floppy disk\n")
		_T("\tstop <DriveLetter>\n")
	);
	ret = stopMessage();
	OutputString(
		_T("\t\tSpin off the disc\n")
		_T("\tstart <DriveLetter>\n")
		_T("\t\tSpin up the disc\n")
		_T("\teject <DriveLetter>\n")
		_T("\t\tEject the tray\n")
		_T("\tclose <DriveLetter>\n")
		_T("\t\tClose the tray\n")
		_T("\treset <DriveLetter>\n")
		_T("\t\tReset the drive (Only PLEXTOR)\n")
		_T("\tls <DriveLetter>\n")
		_T("\t\tShow maximum speed of the drive\n")
		_T("\tsub <Subfile>\n")
		_T("\t\tParse CloneCD sub file and output to readable format\n")
		_T("\tmds <Mdsfile>\n")
		_T("\t\tParse Alchohol 120/52 mds file and output to readable format\n")
		_T("\tmerge <plextor image file> <optiarc image file>\n")
		_T("\t\tmerge the two files (for physical error protection)\n")
		_T("Option (generic)\n")
		_T("\t/f\tUse 'Force Unit Access' flag to delete the drive cache\n")
		_T("\t\t\tval\tdelete per specified value (default: 1)\n")
		_T("\t/q\tDisable beep\n")
		_T("Option (for CD read mode)\n")
		_T("\t/a\tAdd CD offset manually (Only Audio CD)\n")
		_T("\t\t\tval\tsamples value\n")
	);
	ret = stopMessage();
	OutputString(
		_T("\t/be\tUse 0xbe as the opcode for Reading CD forcibly\n")
		_T("\t\t\tstr\t raw: sub channel mode is raw (default)\n")
		_T("\t\t\t   \tpack: sub channel mode is pack\n")
		_T("\t/d8\tUse 0xd8 as the opcode for Reading CD forcibly\n")
		_T("\t/c2\tContinue reading CD to recover C2 error existing sector\n")
		_T("\t\t\tval1\tvalue to reread (default: 4000)\n")
		_T("\t\t\tval2\t0: reread sector c2 error is reported (default)\n")
		_T("\t\t\t    \t1: reread all (or from first to last) sector\n")
		_T("\t\t\tval3\tfirst LBA to reread (default: 0)\n")
		_T("\t\t\tval4\tlast LBA to reread (default: end-of-sector)\n")
		_T("\t\t\t    \tval3, 4 is used when val2 is 1\n")
		_T("\t/m\tUse if MCN exists in the first pregap sector of the track\n")
		_T("\t\t\tFor some PC-Engine\n")
		_T("\t/p\tDumping the AMSF from 00:00:00 to 00:01:74\n")
		_T("\t\t\tFor SagaFrontier Original Sound Track (Disc 3) etc.\n")
		_T("\t\t\tSupport drive: PLEXTOR PX-W5224, PREMIUM, PREMIUM2\n")
		_T("\t\t\t               PX-704, 708, 712, 714, 716, 755, 760\n")
		_T("\t/r\tRead CD from the reverse\n")
		_T("\t\t\tFor Alpha-Disc, Tages (very slow)\n")
		_T("\t/ms\tRead the lead-out of 1st session and the lead-in of 2nd session\n")
		_T("\t\t\tFor Multi-session\n")
		_T("\t/74\tRead the lead-out about 74:00:00\n")
		_T("\t\t\tFor ring data (a.k.a Saturn Ring) of Sega Saturn\n")
		_T("\t/sf\tScan file to detect protect. If reading error exists,\n")
	);
	ret = stopMessage();
	OutputString(
		_T("\t   \tcontinue reading and ignore c2 error on specific sector\n")
		_T("\t\t\tFor CodeLock, LaserLock, RingProtect, RingPROTECH\n")
		_T("\t\t\t    SafeDisc, SmartE, ProtectCD-VOB, CDS300\n")
		_T("\t\t\tval\ttimeout value (default: 60)\n")
		_T("\t/sk\tSkip sector for physical protect (e.g. proring, LaserLock etc.)\n")
		_T("\t\t\tval1\tsector num\n")
		_T("\t\t\tval2\tsector num (optional)\n")
		_T("\t/ss\tScan sector to detect protect. If reading error exists,\n")
		_T("\t   \tcontinue reading and ignore c2 error on specific sector\n")
		_T("\t\t\tFor ProtectCD-VOB\n")
		_T("\t/am\tScan anti-mod string\n")
		_T("\t\t\tFor PlayStation\n")
		_T("\t/vn\tSearch specific bytes\n")
		_T("\t\t\tFor VideoNow\n")
		_T("Option (for CD SubChannel)\n")
		_T("\t/np\tNot fix SubP\n")
		_T("\t/nq\tNot fix SubQ\n")
		_T("\t/nr\tNot fix SubRtoW\n")
		_T("\t/nl\tNot fix SubQ (RMSF, AMSF, CRC) (LBA 10000 - 19999)\n")
		_T("\t   \t                               (LBA 40000 - 49999)\n")
		_T("\t\t\tFor PlayStation LibCrypt\n")
		_T("\t/ns\tNot fix SubQ (RMSF, AMSF, CRC) (LBA 0 - 7, 5000 - 24999)\n")
		_T("\t   \t                            or (LBA 30000 - 49999)\n")
		_T("\t\t\tFor SecuROM\n")
		_T("\t/s\tUse if it reads subchannel precisely\n")
		_T("\t\t\tval\t0: no read next sub (fast, but lack precision)\n")
		_T("\t\t\t   \t1: read next sub (normal, this val is default)\n")
		_T("\t\t\t   \t2: read next & next next sub (slow, precision)\n")
		_T("Option (for DVD)\n")
		_T("\t/c\tLog Copyright Management Information\n")
		_T("\t/raw\tDumping DVD by raw (2064 byte/sector)\n")
		_T("\t\t\tComfirmed drive: Mediatec MT chip (Lite-on etc.), PLEXTOR\n")
		_T("\t\t\t               Hitachi-LG GDR, GCC\n")
		_T("\t\t\t -> GDR (8082N, 8161B to 8164B) and GCC (4160N, 4240N to 4247N)\n")
		_T("\t\t\t    supports GC/Wii dumping\n")
	);
	ret = stopMessage();
	return ret;
}

int printSeveralInfo(LPTSTR pszDateTime, size_t dateTimeSize)
{
#if 0
	if (!OutputWindowsVersion()) {
		return FALSE;
	}
#endif
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
	_sntprintf(pszDateTime, dateTimeSize, _T("%s %s\n"), BUILD_DATE, BUILD_TIME);
	OutputString(_T("%s"), pszDateTime);
	return TRUE;
}
#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
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
	_TCHAR szDateTime[20] = {};
	int nRet = printSeveralInfo(szDateTime, sizeof(szDateTime));
	if (nRet) {
		EXEC_TYPE execType;
		EXT_ARG extArg = {};
		extArg.dwCacheDelNum = DEFAULT_CACHE_DELETE_VAL;
		_TCHAR szFullPath[_MAX_PATH + 1] = {};
		if (!checkArg(argc, argv, &execType, &extArg, szFullPath)) {
			printUsage();
			nRet = FALSE;
		}
		else {
			time_t now;
			struct tm* ts;
			_TCHAR szBuf[128] = {};

			now = time(NULL);
			ts = localtime(&now);
			_tcsftime(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), _T("%Y/%m/%d(%a) %H:%M:%S"), ts);
			OutputString(_T("StartTime: %s\n"), szBuf);

			if (execType != merge) {
				nRet = createCmdFile(argc, argv, szFullPath, szDateTime);
			}
			if (nRet) {
				nRet = exec(argv, &execType, &extArg, szFullPath);
			}

			now = time(NULL);
			ts = localtime(&now);
			_tcsftime(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), _T("%Y/%m/%d(%a) %H:%M:%S"), ts);
			OutputString(_T("EndTime: %s\n"), szBuf);
		}
		if (!extArg.byQuiet) {
			if (!soundBeep(nRet)) {
				nRet = FALSE;
			}
		}
	}
#ifdef _DEBUG
	_tsystem(_T("pause"));
#endif
	return nRet = nRet == TRUE ? EXIT_SUCCESS : EXIT_FAILURE;
}

