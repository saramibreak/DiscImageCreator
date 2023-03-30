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
#include "buildDateTime.h"
#include "struct.h"
#include "calcHash.h"
#include "check.h"
#include "execIoctl.h"
#include "execScsiCmd.h"
#include "execScsiCmdforCD.h"
#include "execScsiCmdforCDCheck.h"
#include "execScsiCmdforDVD.h"
#include "execTapeCmd.h"
#include "get.h"
#include "init.h"
#include "output.h"
#include "set.h"
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
static UINT s_uiFix = 0;
static UINT s_uiSpeed = 0;
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

int execForDumping(PEXEC_TYPE pExecType, PEXT_ARG pExtArg, _TCHAR* pszFullPath, PDEVICE pDevice, PDISC pDisc)
{
	BOOL bRet = FALSE;
	MAIN_HEADER mainHeader = {};
	FILE* fpCcd = NULL;
	FILE* fpC2 = NULL;
	LPBYTE pPFullToc = NULL;
	try {
		_TCHAR szPath[_MAX_PATH] = { 0 };
		_tcsncpy(szPath, pszFullPath, sizeof(szPath) / sizeof(_TCHAR) - 1);
		PathRemoveFileSpec(szPath);

#ifndef _DEBUG
		if (*pExecType != drivespeed) {
			// 2nd: create logfile here (because logging all working)
			if (!InitLogFile(pExecType, pExtArg, pszFullPath)) {
				throw FALSE;
			}
		}
#endif
		if (!TestUnitReady(pExtArg, pDevice)) {
			throw FALSE;
		}
		if (!ReadDriveInformation(pExecType, pExtArg, pDevice, pDisc, s_uiSpeed)) {
			throw FALSE;
		}
		if (*pExecType == drivespeed) {
			pExtArg->byQuiet = TRUE;
			return TRUE;
		}

		make_crc_table();
		HASH hash = {};
		hash.uiMax = 0;
		if (*pExecType == disk) {
			hash.uiMax = 1; // .bin
		}
		else if (*pExecType == bd) {
			hash.uiMax = 2; // PIC.bin, .iso
		}
		else if (*pExecType == dvd || *pExecType == sacd) {
			hash.uiMax = 3; // PFI.bin, DMI.bin, .iso or .raw
		}
		else if (*pExecType == xbox) {
			hash.uiMax = 4; // SS.bin, PFI.bin, DMI.bin, .iso
		}
		hash.pHashChunk = (PHASH_CHUNK)calloc(hash.uiMax, sizeof(HASH_CHUNK));
		if (!hash.pHashChunk) {
			throw FALSE;
		}

		if (*pExecType == fd || *pExecType == disk) {
			bRet = ReadDisk(pExecType, pExtArg, pDevice, pDisc, pszFullPath, &hash);
		}
		else {
			if (*pExecType == cd || *pExecType == swap || *pExecType == gd || *pExecType == data || *pExecType == audio) {
				if (IsCDorRelatedDisc(pExecType, pDisc)) {
#ifdef _WIN32
					_declspec(align(4)) CDROM_TOC_FULL_TOC_DATA fullToc = { 0 };
#else
					__attribute__((aligned(4))) CDROM_TOC_FULL_TOC_DATA fullToc = {};
#endif
					if (!ReadCDForCheckingSubQ1stIndex(pExecType, pExtArg, pDevice, pDisc)) {
						throw FALSE;
					}
					PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData = NULL;
					WORD wTocEntries = 0;
					if (*pExecType != swap && *pExecType != gd) {
						if (!ReadTOCFull(pExtArg, pDevice, pDisc, &fullToc, &pTocData, &wTocEntries, &pPFullToc)) {
							throw FALSE;
						}
					}
					// call this here because "Invalid TOC" occurs by GD-ROM
					if (!ReadTOC(pExtArg, pExecType, pDevice, pDisc, pszFullPath)) {
						throw FALSE;
					}
					if (!IsEnoughDiskSpaceForDump(pExecType, pDisc, szPath)) {
						throw FALSE;
					}

					InitMainDataHeader(pExecType, pExtArg, &mainHeader, (INT)s_nStartLBA);
					if (!InitSubData(pExecType, &pDisc)) {
						throw FALSE;
					}
					if (!InitTocFullData(pExecType, &pDisc)) {
						throw FALSE;
					}
					if (!InitTocTextData(pExecType, pDevice, &pDisc)) {
						throw FALSE;
					}
					if (pExtArg->byScanProtectViaFile || pExtArg->byIntentionalSub) {
						if (!InitProtectData(&pDisc)) {
							throw FALSE;
						}
					}
					make_scrambled_table();
					make_crc16_table();
#if 0
					make_crc6_table();
#endif
					CDFLAG::_READ_CD::_ERROR_FLAGS c2 = CDFLAG::_READ_CD::NoC2;
					ReadCDForCheckingByteOrder(pExecType, pExtArg, pDevice, &c2);
					if (pExtArg->byC2) {
						if (pDevice->FEATURE.byC2ErrorData && c2 != CDFLAG::_READ_CD::NoC2) {
							if (NULL == (fpC2 = CreateOrOpenFile(
								pszFullPath, NULL, NULL, NULL, NULL, _T(".c2"), _T("wb"), 0, 0))) {
								OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
								throw FALSE;
							}
							if (!InitC2(&pDisc)) {
								throw FALSE;
							}
						}
						if (pExtArg->uiSubAddionalNum == 0 && *pExecType != gd) {
							OutputString("[WARNING] /c2 and /s 0 can't be used together. Changed /s 0 to /s 1.\n");
							pExtArg->uiSubAddionalNum = 1;
						}
						else if (pExtArg->uiSubAddionalNum != 2 && IsPlextor712OrNewer(pDevice)) {
							OutputString("[INFO] This drive has 295 offset in the c2. Changed to /s 2.\n");
							pExtArg->uiSubAddionalNum = 2;
						}
					}
					else {
						OutputString("[WARNING] /c2 isn't set. The result of dumping may be incorrect if c2 error exists.\n");
					}

					if (pDisc->SCSI.wCurrentMedia == ProfileCdrom ||
						pDisc->SCSI.wCurrentMedia == ProfileCdRecordable ||
						pDisc->SCSI.wCurrentMedia == ProfileCdRewritable) {
						ReadDiscInformation(pExtArg, pDevice);
						if (pDisc->SCSI.wCurrentMedia == ProfileCdRecordable ||
							pDisc->SCSI.wCurrentMedia == ProfileCdRewritable) {
							ReadTOCPma(pExtArg, pDevice);
							ReadTOCAtip(pExtArg, pDevice);
						}
					}

					// This func needs the TOC
					if (!ReadCDForSearchingOffset(pExecType, pExtArg, pDevice, pDisc)) {
						throw FALSE;
					}
					if (pExtArg->byC2 && pDevice->byPlxtrDrive == _PLXTR_DRIVE_TYPE::No) {
						// Re-set c2 flag
						c2 = pDevice->supportedC2Type;
					}
					// needs to call ReadCDForSearchingOffset
					if (pDisc->SUB.nSubChannelOffset && pExtArg->uiSubAddionalNum == 0) {
						OutputString("[INFO] SubChannel offset exists in this drive. Changed /s 0 to /s 1.\n");
						pExtArg->uiSubAddionalNum = 1;
					}

					DISC_PER_SECTOR discPerSector = {};
					memcpy(&discPerSector.mainHeader, &mainHeader, sizeof(MAIN_HEADER));

					if (*pExecType == gd) {
						if (IsValidPlextorDrive(pExtArg, pDevice) && pExtArg->uiSubAddionalNum == 0) {
							if (IsPlextor712OrNewer(pDevice)) {
								OutputString("[INFO] This drive has 295 offset in the c2. Changed /s 0 to /s 2.\n");
								pExtArg->uiSubAddionalNum = 2;
							}
							else {
								OutputString("[INFO] This drive has 294 offset in the c2. Changed /s 0 to /s 1.\n");
								pExtArg->uiSubAddionalNum = 1;
							}
						}
						// This func needs the combined offsets
						if (!ReadGDForTOC(pExtArg, pDevice, pDisc)) {
							throw FALSE;
						}
						if (!ReadTOCText(pExtArg, pDevice, pDisc, NULL)) {
							throw FALSE;
						}
						if (!ReadGDForCheckingSubQAdr(pExecType, pExtArg, pDevice, pDisc, &discPerSector)) {
							throw FALSE;
						}
					}
					else {
						if (*pExecType == cd || *pExecType == swap) {
							// This func needs the combined offsets
							if (!ReadCDForCheckingReadInOut(pExecType, pExtArg, pDevice, pDisc)) {
								throw FALSE;
							}
							// open ccd here because ccd is written by ReadTOCFull
							if (NULL == (fpCcd = CreateOrOpenFile(pszFullPath,
								NULL, NULL, NULL, NULL, _T(".ccd"), _T(WFLAG), 0, 0))) {
								OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
								throw FALSE;
							}
							if (setvbuf(fpCcd, NULL, _IONBF, 0) != 0) {
								OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							}
						}
						if (*pExecType != swap) {
							WriteCcdFirst(pExecType, pExtArg, pDevice, pDisc, &discPerSector, &fullToc, pTocData, wTocEntries, fpCcd);
							SetAndOutputTocFull(pDisc, &fullToc, pTocData, wTocEntries, fpCcd);
						}
					}
					if (*pExecType != swap) {
						if (!ReadCDCheck(pExecType, pExtArg, pDevice, pDisc)) {
							throw FALSE;
						}
						if (pExtArg->byVerifyAudioCDOfs && pExtArg->uiVerifyAudio == READ_AUDIO_DISC_WITHOUT_DUMPING &&
							pDisc->SCSI.trkType == TRACK_TYPE::audioOnly && pDisc->MAIN.bResetOffset) {
							throw TRUE;
						}
					}

					if (*pExecType == cd) {
						if (IsPregapOfTrack1ReadableDrive(pDevice) && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
							if (!ReadCDOutOfRange(pExecType, pExtArg, pDevice, pDisc, pszFullPath)) {
								throw FALSE;
							}
						}
						bRet = ReadCDAll(pExecType, pExtArg, pDevice, pDisc, &discPerSector, c2, pszFullPath, fpCcd, fpC2);

						if (IsPregapOfTrack1ReadableDrive(pDevice) && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
							ConcatenateFromPregapToLeadout(pDisc, pszFullPath);
						}
#if 0
						if (!pExtArg->byPre &&
							((pDisc->MAIN.bManySamples & PLUS_10000_SAMPLES) == PLUS_10000_SAMPLES ||
							(pDisc->MAIN.bManySamples & MINUS_10000_SAMPLES) == MINUS_10000_SAMPLES)) {
							ReadCDAdditional(pExecType, pExtArg, pDevice, pDisc, pszFullPath);
						}
#endif
					}
					else if (*pExecType == swap) {
						bRet = ReadCDForSwap(pExecType, pExtArg, pDevice, pDisc, &discPerSector
							, c2, pszFullPath, 0, pDisc->SCSI.nAllLength, fpCcd, fpC2);
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
						bRet = ReadCDPartial(pExecType, pExtArg, pDevice, pDisc, &discPerSector
							, c2, pszFullPath, FIRST_LBA_FOR_GD, 549149 + 1, fpC2);
					}
					else if (*pExecType == data || *pExecType == audio) {
						bRet = ReadCDPartial(pExecType, pExtArg, pDevice, pDisc, &discPerSector
							, c2, pszFullPath, (INT)s_nStartLBA, (INT)s_nEndLBA, fpC2);
					}
				}
				else {
					OutputString("Wrong command. The disc isn't CD, CD-R, CD-RW, GD\n");
				}
			}
			else if (*pExecType == dvd) {
				pDisc->DVD.discType = DISC_TYPE_DVD::formal;
				if (IsDVDorRelatedDisc(pDisc)) {
					DVDGetRegion(pDevice);
					if (pExtArg->byScanProtectViaFile) {
						if (!InitProtectData(&pDisc)) {
							throw FALSE;
						}
					}
					if (pDisc->SCSI.wCurrentMedia == ProfileDvdRam ||
						pDisc->SCSI.wCurrentMedia == ProfileDvdPlusR ||
						pDisc->SCSI.wCurrentMedia == ProfileHDDVDRam) {
						ReadTOC(pExtArg, pExecType, pDevice, pDisc, pszFullPath);
					}
					if (!IsEnoughDiskSpaceForDump(pExecType, pDisc, szPath)) {
						throw FALSE;
					}
					AnalyzeIfoFile(pDevice, pDisc);

					bRet = ReadDiscStructure(pExecType, pExtArg, pDevice, pDisc, pszFullPath, &hash);
					if (pExtArg->byCmi) {
						bRet = ReadDVDForCMI(pExtArg, pDevice, pDisc);
					}
					if (bRet) {
						if (pExtArg->byRawDump) {
							while (1) {
								if (pExtArg->byFix) {
									pDisc->DVD.fixNum = s_uiFix;
								}
								bRet = ReadDVDRaw(pExtArg, pDevice, pDisc, pszFullPath, &hash);
								if (pExtArg->byFix && bRet > 6) {
									s_uiFix = (UINT)bRet;
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
						else if (pExtArg->byReverse) {
							bRet = ReadDVDReverse(pExtArg, pDevice, pszFullPath, (INT)s_nStartLBA, (INT)s_nEndLBA);
						}
						else {
							CONST size_t bufSize = 12;
							_TCHAR szBuf[bufSize] = {};
#ifdef _WIN32
							_sntprintf(szBuf, bufSize, _T("%c:\\*"), pDevice->byDriveLetter);
#else
							_sntprintf(szBuf, bufSize, _T("%s/*"), pDevice->drivepath);
#endif
							szBuf[4] = 0;
							UINT64 uiDiscSize = 0;
							bRet = GetDiscSize(szBuf, &uiDiscSize);
							if (bRet && uiDiscSize > 8547991552) {
								OutputLog(standardOut | fileDisc, "Detected disguised file size: %llu\n", uiDiscSize);
							}
							bRet = ReadDVD(pExecType, pExtArg, pDevice, pDisc, pszFullPath, &hash);
						}
					}
				}
				else {
					OutputString("Wrong command. The disc isn't DVD, DVD-R, DVD-RW\n");
				}
			}
			else if (*pExecType == xbox) {
				pDisc->DVD.discType = DISC_TYPE_DVD::xboxdvd;
				if (!IsEnoughDiskSpaceForDump(pExecType, pDisc, szPath)) {
					throw FALSE;
				}
				bRet = ReadXboxDVD(pExecType, pExtArg, pDevice, pDisc, pszFullPath, &hash);
			}
			else if (*pExecType == xboxswap || *pExecType == xgd2swap || *pExecType == xgd3swap) {
				pDisc->DVD.discType = DISC_TYPE_DVD::xboxdvd;
				bRet = ReadXboxDVDBySwap(pExecType, pExtArg, pDevice, pDisc, pszFullPath, &hash);
			}
			else if (*pExecType == bd) {
				if (IsBDorRelatedDisc(pDisc)) {
					if (!ReadTOC(pExtArg, pExecType, pDevice, pDisc, pszFullPath)) {
						throw FALSE;
					}
					if (!IsEnoughDiskSpaceForDump(pExecType, pDisc, szPath)) {
						throw FALSE;
					}
					bRet = ReadDiscStructure(pExecType, pExtArg, pDevice, pDisc, pszFullPath, &hash);
					if (bRet) {
						bRet = ReadDVD(pExecType, pExtArg, pDevice, pDisc, pszFullPath, &hash);
					}
				}
				else {
					OutputString("Wrong command. The disc isn't BD\n");
				}
			}
			else if (*pExecType == sacd) {
				if (IsValidPS3Drive(pDevice)) {
					if (!ReadTOC(pExtArg, pExecType, pDevice, pDisc, pszFullPath)) {
						throw FALSE;
					}
					if (!IsEnoughDiskSpaceForDump(pExecType, pDisc, szPath)) {
						throw FALSE;
					}
					bRet = ReadSACD(pExtArg, pDevice, pDisc, pszFullPath, &hash);
				}
			}
		}
		if (bRet && (*pExecType != audio && *pExecType != data)) {
			bRet = ReadWriteDat(pExecType, pExtArg, pDisc, pszFullPath, s_szDir, SUB_DESYNC_TYPE::NoDesync, &hash);
			if (pDisc->SUB.byIdxDesync) {
				bRet = ReadWriteDat(pExecType, pExtArg, pDisc, pszFullPath, s_szDir, SUB_DESYNC_TYPE::IdxDesync, &hash);
			}
			if (pDisc->SUB.byCtlDesync) {
				bRet = ReadWriteDat(pExecType, pExtArg, pDisc, pszFullPath, s_szDir, SUB_DESYNC_TYPE::CtlDesync, &hash);
			}
		}
		FreeAndNull(hash.pHashChunk);
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FlushLog();
	if (*pExecType == cd || *pExecType == swap) {
		FcloseAndNull(fpCcd);
	}
	if (*pExecType == cd || *pExecType == audio || *pExecType == data) {
		FreeAndNull(pPFullToc);
	}
	if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		FcloseAndNull(fpC2);
	}
#ifndef _DEBUG
	TerminateLogFile(pExecType, pExtArg);
#endif
	return bRet;
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
	else if (*pExecType == tape) {
#ifndef _DEBUG
		if (!InitLogFile(pExecType, pExtArg, pszFullPath)) {
			return FALSE;
		}
#endif
		bRet = ReadTape(pszFullPath);
	}
	else {
		DEVICE device = {};
#ifdef _WIN32
		device.byDriveLetter = (BYTE)(argv[2][0]);
#else
		strncpy(device.drivepath, argv[2], sizeof(device.drivepath) - 1);
#endif
		if (!GetHandle(&device)) {
			return FALSE;
		}

		// 1st: set TimeOutValue here (because use ScsiPassThroughDirect)
		if (pExtArg->byScanProtectViaFile) {
			device.dwTimeOutValue = pExtArg->dwTimeoutNum;
			GetFilenameToSkipError(pExtArg->FILE.readError);
			GetFilenameToFixError(pExtArg->FILE.c2Error);
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

			bRet = execForDumping(pExecType, pExtArg, pszFullPath, &device, pDisc);

			if (bRet && pExtArg->byVerifyAudioCDOfs && pExtArg->uiVerifyAudio == READ_AUDIO_DISC_WITH_OFFSET &&
				pDisc->SCSI.trkType == TRACK_TYPE::audioOnly && pDisc->MAIN.bResetOffset) {
				pExtArg->uiVerifyAudio = READ_AUDIO_DISC_WITHOUT_OFFSET;
				INT ch = 0;
				do {
					OutputString("Would you like to also generate hashes for a non-offset dump to verify with another disc? [y/n]\n");
					ch = _getch();
					if (ch == 'y') {
						_TCHAR szFullPathNoOfs[_MAX_PATH] = {};
						_tcsncpy(szFullPathNoOfs, pszFullPath, sizeof(szFullPathNoOfs) / sizeof(_TCHAR) - 1);
						PathRemoveExtension(szFullPathNoOfs);
						_tcsncat(szFullPathNoOfs, _T("_NoOffset"), 9);
						PathRenameExtension(szFullPathNoOfs, s_szExt);

						execForDumping(pExecType, pExtArg, szFullPathNoOfs, &device, pDisc);
					}
				} while (ch != 'y' && ch != 'n');
			}

			if (*pExecType == cd || *pExecType == swap || *pExecType == gd ||
				*pExecType == data || *pExecType == audio || *pExecType == bd) {
				TerminateLBAPerTrack(&pDisc);
				if (*pExecType == cd || *pExecType == swap || *pExecType == gd ||
					*pExecType == data || *pExecType == audio) {
					TerminateSubData(pExecType, &pDisc);
					TerminateTocFullData(&pDisc);
					TerminateTocTextData(pExecType, &device, &pDisc);
				}
			}
			if (pExtArg->byScanProtectViaFile || pExtArg->byIntentionalSub) {
				TerminateProtectData(&pDisc);
			}
			if (pExtArg->byC2 && device.FEATURE.byC2ErrorData) {
				TerminateC2(&pDisc);
			}
		}
		if (device.hDevice && !CloseHandle(device.hDevice)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	return bRet;
}

int appendExtIfNotExt(_TCHAR* szPathFromArg, size_t pathLen, _TCHAR* szTmpPath)
{
	_TCHAR ext[5] = {};
	_tcsncpy(ext, &szPathFromArg[pathLen - 4], sizeof(ext) / sizeof(ext[0]) - 1);
	if (_tcsncmp(ext, _T(".bin"), sizeof(ext) / sizeof(ext[0])) != 0 &&
		_tcsncmp(ext, _T(".iso"), sizeof(ext) / sizeof(ext[0])) != 0 &&
		_tcsncmp(ext, _T(".sub"), sizeof(ext) / sizeof(ext[0])) != 0 &&
		_tcsncmp(ext, _T(".mds"), sizeof(ext) / sizeof(ext[0])) != 0 &&
		_tcsncmp(ext, _T(".mdf"), sizeof(ext) / sizeof(ext[0])) != 0
		) {
		OutputString("valid extension was omitted. -> ");
		size_t len = _tcslen(szTmpPath);
		if (len + sizeof(ext) / sizeof(ext[0]) > _MAX_PATH) {
			OutputString("can't set extension because PATH too long\n");
			return FALSE;
		}
		else {
			OutputString("set .bin\n");
			_tcsncat(szTmpPath, _T(".bin"), sizeof(ext) / sizeof(ext[0]));
		}
	}
	return TRUE;
}

int printAndSetPath(_TCHAR* szPathFromArg, _TCHAR* pszFullPath, size_t stFullPathlen)
{
	_TCHAR szTmpPath[_MAX_PATH] = {};
	_tcsncpy(szTmpPath, szPathFromArg, sizeof(szTmpPath) / sizeof(szTmpPath[0]) - 1);
	appendExtIfNotExt(szPathFromArg, _tcslen(szPathFromArg), szTmpPath);

	if (!GetCurrentDirectory(sizeof(s_szCurrentdir) / sizeof(s_szCurrentdir[0]), s_szCurrentdir)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	_tsplitpath(szTmpPath, s_szDrive, s_szDir, s_szFname, s_szExt);

	if (!s_szDrive[0] || !s_szDir[0]) {
		_tcsncpy(pszFullPath, s_szCurrentdir, stFullPathlen);
		pszFullPath[stFullPathlen - 1] = 0;
		if (s_szDir[0]) {
			if (!PathAppend(pszFullPath, s_szDir)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
		}
		if (!PathFileExists(pszFullPath)) {
			OutputErrorString("%s doesn't exist, so create.\n", pszFullPath);
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
		_tsplitpath(pszFullPath, s_szDrive, s_szDir, NULL, NULL);

		if (s_szExt[0] && _tcslen(pszFullPath) + _tcslen(s_szExt) < stFullPathlen) {
			_tcsncat(pszFullPath, s_szExt, stFullPathlen - _tcslen(pszFullPath));
		}
	}
	else {
		_tcsncpy(pszFullPath, s_szDrive, stFullPathlen);
		_tcsncat(pszFullPath, s_szDir, stFullPathlen - _tcslen(pszFullPath));
		if (!PathFileExists(pszFullPath)) {
			OutputErrorString("%s doesn't exist, so create.\n", pszFullPath);
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
		_tcsncpy(pszFullPath, szTmpPath, stFullPathlen);
	}
	OutputString(
		"CurrentDirectory\n"
		"\t%s\n"
		"WorkingPath\n"
		"\t Argument: %s\n"
		"\t FullPath: %s\n"
		"\t    Drive: %s\n"
		"\tDirectory: %s\n"
		"\t Filename: %s\n"
		"\tExtension: %s\n",
		s_szCurrentdir, szTmpPath, pszFullPath, s_szDrive, s_szDir, s_szFname, s_szExt);

	return TRUE;
}

int SetOptionVrfy(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	pExtArg->byVerifyAudioCDOfs = TRUE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->uiVerifyAudio = (UINT)_tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString("[ERROR] [%s] is invalid argument. Please input integer.\n", endptr);
			return FALSE;
		}
		else if (pExtArg->uiVerifyAudio != READ_AUDIO_DISC_WITHOUT_OFFSET &&
			pExtArg->uiVerifyAudio != READ_AUDIO_DISC_WITH_OFFSET &&
			pExtArg->uiVerifyAudio != READ_AUDIO_DISC_WITHOUT_DUMPING) {
			OutputErrorString("[ERROR] [%s] is invalid argument. 0 or 1 or 2 can be used.\n", endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->uiVerifyAudio = READ_AUDIO_DISC_WITH_OFFSET;
		OutputString("[INFO] /vrfy val was omitted. set [%u]\n", pExtArg->uiVerifyAudio);
	}
	return TRUE;
}

int SetOptionMr(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	pExtArg->byMultiSectorReading = TRUE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->uiRetryCnt = (UINT)_tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->uiRetryCnt = 50;
		OutputString("/mr val was omitted. set [%u]\n", pExtArg->uiRetryCnt);
	}
	return TRUE;
}

int SetOptionPs(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	pExtArg->byPadSector = TRUE;
	if (argc > * i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->uiPadNum = (UINT)_tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
			return FALSE;
		}
	}
	else {
		OutputString("/ps val was omitted. set [%u]\n", pExtArg->uiPadNum);
	}
	return TRUE;
}

int SetOptionFix(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->byFix = TRUE;
		s_uiFix = (UINT)_tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
			return FALSE;
		}
	}
	else {
		OutputErrorString("/fix val was omitted. Please input integer.\n");
		return FALSE;
	}
	return TRUE;
}

int SetOptionRr(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->uiMaxRereadNum = (UINT)_tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->uiMaxRereadNum = 10;
		OutputString("/rr val was omitted. set [%u]\n", pExtArg->uiMaxRereadNum);
	}
	return TRUE;
}

int SetOptionNss(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	pExtArg->byNoSkipSS = TRUE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->uiMaxRereadNum = (UINT)_tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->uiMaxRereadNum = 100;
		OutputString("/nss val was omitted. set [%u]\n", pExtArg->uiMaxRereadNum);
	}
	return TRUE;
}

int SetOptionVn(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	pExtArg->byVideoNow = TRUE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->nAudioCDOffsetNum = (INT)_tcstol(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->nAudioCDOffsetNum = 0;
		OutputString("/vn val was omitted. set [%d]\n", pExtArg->nAudioCDOffsetNum);
	}
	return TRUE;
}

int SetOptionSk(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->uiSkipSectors = (UINT)_tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
			return FALSE;
		}
		if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
			pExtArg->uiSkipSectors2 = (UINT)_tcstoul(argv[(*i)++], &endptr, 10);
			if (*endptr) {
				OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
				return FALSE;
			}
		}
	}
	else {
		pExtArg->uiSkipSectors = 0;
		OutputString("/sk val was omitted. set [%u]\n", pExtArg->uiSkipSectors);
	}
	return TRUE;
}

int SetOptionS(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->uiSubAddionalNum = (UINT)_tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->uiSubAddionalNum = 1;
		OutputString("/s val was omitted. set [%u]\n", pExtArg->uiSubAddionalNum);
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
			OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->dwTimeoutNum = DEFAULT_SPTD_TIMEOUT_VAL;
		OutputString("/sf val was omitted. set [%d]\n", DEFAULT_SPTD_TIMEOUT_VAL);
	}
	return TRUE;
}

int SetOptionF(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	pExtArg->byFua = TRUE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->uiCacheDelNum = (UINT)_tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->uiCacheDelNum = DEFAULT_CACHE_DELETE_VAL;
		OutputString(
			"/f val was omitted. set [%d]\n", DEFAULT_CACHE_DELETE_VAL);
	}
	return TRUE;
}

int SetOptionC2(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	pExtArg->byC2 = TRUE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->uiMaxRereadNum = (UINT)_tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
			return FALSE;
		}
		if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
			pExtArg->uiC2Offset = (UINT)_tcstoul(argv[(*i)++], &endptr, 10);
			if (*endptr) {
				OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
				return FALSE;
			}
			else if (CD_RAW_READ_C2_294_SIZE * 2 < pExtArg->uiC2Offset) {
				OutputErrorString("[%u] is invalid argument. Please input from 0 to %u.\n", pExtArg->uiC2Offset, CD_RAW_READ_C2_294_SIZE * 2 - 1);
				return FALSE;
			}
			if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
				pExtArg->nC2RereadingType = (INT)_tcstol(argv[(*i)++], &endptr, 10);
				if (*endptr) {
					OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
					return FALSE;
				}
				if (pExtArg->nC2RereadingType != 0) {
					if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1) && pExtArg->nC2RereadingType == 1) {
						pExtArg->nStartLBAForC2 = (INT)_tcstol(argv[(*i)++], &endptr, 10);
						if (*endptr) {
							OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
							return FALSE;
						}
						if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
							pExtArg->nEndLBAForC2 = (INT)_tcstol(argv[(*i)++], &endptr, 10);
							if (*endptr) {
								OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
								return FALSE;
							}
						}
						else {
							pExtArg->nEndLBAForC2 = 0;
							OutputString("/c2 val5 was omitted. set [%d]\n", 0);
						}
					}
					else {
						pExtArg->nStartLBAForC2 = 0;
						OutputString("/c2 val4 was omitted. set [%d]\n", 0);
						pExtArg->nEndLBAForC2 = 0;
						OutputString("/c2 val5 was omitted. set [%d]\n", 0);
					}
				}
			}
			else {
				pExtArg->nC2RereadingType = 0;
				OutputString("/c2 val3 was omitted. set [%d]\n", 0);
			}
		}
		else {
			pExtArg->uiC2Offset = 0;
			OutputString("/c2 val2 was omitted. set [%d]\n", 0);
		}
	}
	else {
		pExtArg->uiMaxRereadNum = DEFAULT_REREAD_VAL;
		OutputString("/c2 val1 was omitted. set [%d]\n", DEFAULT_REREAD_VAL);
		pExtArg->uiC2Offset = 0;
		OutputString("/c2 val2 was omitted. set [%d]\n", 0);
		pExtArg->nC2RereadingType = 0;
		OutputString("/c2 val3 was omitted. set [%d]\n", 0);
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
			OutputErrorString("Bad arg: [%s] Please enter pack or raw\n", argv[*i]);
			return FALSE;
		}
	}
	else {
		pExtArg->byRaw = TRUE;
		OutputString("submode of /be was omitted. set [raw]\n");
	}
	return TRUE;
}

int SetOptionA(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	pExtArg->byAdd = TRUE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->nAudioCDOffsetNum = (INT)_tcstol(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->nAudioCDOffsetNum = 0;
		OutputString("/a val was omitted. set [%d]\n", 0);
	}
	return TRUE;
}

int checkArg(int argc, _TCHAR* argv[], PEXEC_TYPE pExecType, PEXT_ARG pExtArg, _TCHAR* pszFullPath, size_t stFullPathlen)
{
	_TCHAR* endptr = NULL;
	size_t cmdLen = 0;
	if (argc == 1) {
		return FALSE;
	}
	else {
		cmdLen = _tcslen(argv[1]);
		if (argc >= 5 && ((cmdLen == 2 && !_tcsncmp(argv[1], _T("cd"), cmdLen)) ||
			(cmdLen == 4 && !_tcsncmp(argv[1], _T("swap"), cmdLen)))) {
			if (cmdLen == 2) {
				*pExecType = cd;
			}
			else if (cmdLen == 4) {
				*pExecType = swap;
			}
			s_uiSpeed = (UINT)_tcstoul(argv[4], &endptr, 10);
			if (*endptr) {
				OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
				return FALSE;
			}
			pExtArg->uiSubAddionalNum = 1;
			for (INT i = 6; i <= argc; i++) {
				cmdLen = _tcslen(argv[i - 1]);
				if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/q"), cmdLen)) {
					pExtArg->byQuiet = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/d"), cmdLen)) {
					pExtArg->byDatExpand = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/a"), cmdLen)) {
					if (!SetOptionA(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/be"), cmdLen)) {
					if (!SetOptionBe(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/d8"), cmdLen)) {
					pExtArg->byBe = FALSE;
					pExtArg->byD8 = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/c2"), cmdLen)) {
					if (!SetOptionC2(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
					if (pExtArg->byPre) {
						OutputErrorString("/c2 can't use with /p. /c2 was disabled\n");
						pExtArg->byC2 = FALSE;
					}
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/f"), cmdLen)) {
					if (!SetOptionF(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/p"), cmdLen)) {
					pExtArg->byPre = TRUE;
					if (pExtArg->byC2) {
						OutputErrorString("/p can't use with /c2. /p was disabled\n");
						pExtArg->byPre = FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/sf"), cmdLen)) {
					if (!SetOptionSf(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/ss"), cmdLen)) {
					pExtArg->byScanProtectViaSector = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/am"), cmdLen)) {
					pExtArg->byScanAntiModStr = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/ms"), cmdLen)) {
					pExtArg->byMultiSession = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/vn"), cmdLen)) {
					if (!SetOptionVn(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 4 && !_tcsncmp(argv[i - 1], _T("/vnc"), cmdLen)) {
					pExtArg->byVideoNowColor = TRUE;
				}
				else if (cmdLen == 4 && !_tcsncmp(argv[i - 1], _T("/vnx"), cmdLen)) {
					pExtArg->byVideoNowXp = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/aj"), cmdLen)) {
					pExtArg->byAtari = TRUE;
				}
				else if (cmdLen == 5 && !_tcsncmp(argv[i - 1], _T("/mscf"), cmdLen)) {
					pExtArg->byMicroSoftCabFile = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/mr"), cmdLen)) {
					if (!SetOptionMr(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/np"), cmdLen)) {
					pExtArg->bySkipSubP = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/nq"), cmdLen)) {
					pExtArg->bySkipSubQ = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/nr"), cmdLen)) {
					pExtArg->bySkipSubRtoW = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/nl"), cmdLen)) {
					pExtArg->byLibCrypt = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/ns"), cmdLen)) {
					pExtArg->byIntentionalSub = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/s"), cmdLen)) {
					if (!SetOptionS(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (*pExecType == swap && cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/74"), cmdLen)) {
					pExtArg->by74Min = TRUE;
				}
				else if (cmdLen == 5 && !_tcsncmp(argv[i - 1], _T("/vrfy"), cmdLen)) {
					if (!SetOptionVrfy(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else {
					OutputErrorString("Unknown option: [%s]\n", argv[i - 1]);
					return FALSE;
				}
			}
			printAndSetPath(argv[3], pszFullPath, stFullPathlen);
		}
		else if (argc >= 5 && cmdLen == 2 && !_tcsncmp(argv[1], _T("gd"), cmdLen)) {
			*pExecType = gd;
			s_uiSpeed = (UINT)_tcstoul(argv[4], &endptr, 10);
			if (*endptr) {
				OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
				return FALSE;
			}
			pExtArg->uiSubAddionalNum = 0;
			for (INT i = 6; i <= argc; i++) {
				cmdLen = _tcslen(argv[i - 1]);
				if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/q"), cmdLen)) {
					pExtArg->byQuiet = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/d"), cmdLen)) {
					pExtArg->byDatExpand = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/be"), cmdLen)) {
					if (!SetOptionBe(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/d8"), cmdLen)) {
					pExtArg->byBe = FALSE;
					pExtArg->byD8 = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/c2"), cmdLen)) {
					if (!SetOptionC2(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/f"), cmdLen)) {
					if (!SetOptionF(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/np"), cmdLen)) {
					pExtArg->bySkipSubP = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/nq"), cmdLen)) {
					pExtArg->bySkipSubQ = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/nr"), cmdLen)) {
					pExtArg->bySkipSubRtoW = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/s"), cmdLen)) {
					if (!SetOptionS(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else {
					OutputErrorString("Unknown option: [%s]\n", argv[i - 1]);
					return FALSE;
				}
			}
			printAndSetPath(argv[3], pszFullPath, stFullPathlen);
		}
		else if (argc >= 7 && ((cmdLen == 4 && !_tcsncmp(argv[1], _T("data"), cmdLen)) ||
			(cmdLen == 5 && !_tcsncmp(argv[1], _T("audio"), cmdLen)))) {
			if (cmdLen == 4) {
				*pExecType = data;
			}
			else if (cmdLen == 5) {
				*pExecType = audio;
			}
			s_uiSpeed = (UINT)_tcstoul(argv[4], &endptr, 10);
			if (*endptr) {
				OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
				return FALSE;
			}
			s_nStartLBA = _tcstol(argv[5], &endptr, 10);
			if (*endptr) {
				OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
				return FALSE;
			}
			s_nEndLBA = _tcstol(argv[6], &endptr, 10);
			if (*endptr) {
				OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
				return FALSE;
			}
			pExtArg->uiSubAddionalNum = 1;
			for (INT i = 8; i <= argc; i++) {
				cmdLen = _tcslen(argv[i - 1]);
				if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/q"), cmdLen)) {
					pExtArg->byQuiet = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/d"), cmdLen)) {
					pExtArg->byDatExpand = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/a"), cmdLen)) {
					if (!SetOptionA(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/be"), cmdLen)) {
					if (!SetOptionBe(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/d8"), cmdLen)) {
					pExtArg->byBe = FALSE;
					pExtArg->byD8 = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/c2"), cmdLen)) {
					if (!SetOptionC2(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/f"), cmdLen)) {
					if (!SetOptionF(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/r"), cmdLen)) {
					pExtArg->byReverse = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/sf"), cmdLen)) {
					if (!SetOptionSf(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/ss"), cmdLen)) {
					pExtArg->byScanProtectViaSector = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/am"), cmdLen)) {
					pExtArg->byScanAntiModStr = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/ms"), cmdLen)) {
					pExtArg->byMultiSession = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/np"), cmdLen)) {
					pExtArg->bySkipSubP = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/nq"), cmdLen)) {
					pExtArg->bySkipSubQ = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/nr"), cmdLen)) {
					pExtArg->bySkipSubRtoW = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/s"), cmdLen)) {
					if (!SetOptionS(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/sk"), cmdLen)) {
					if (!SetOptionSk(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else {
					OutputErrorString("Unknown option: [%s]\n", argv[i - 1]);
					return FALSE;
				}
			}
			printAndSetPath(argv[3], pszFullPath, stFullPathlen);
		}
		else if (argc >= 5 && cmdLen == 3 && !_tcsncmp(argv[1], _T("dvd"), cmdLen)) {
			*pExecType = dvd;
			s_uiSpeed = (UINT)_tcstoul(argv[4], &endptr, 10);
			if (*endptr) {
				OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
				return FALSE;
			}
			for (INT i = 6; i <= argc; i++) {
				cmdLen = _tcslen(argv[i - 1]);
				if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/c"), cmdLen)) {
					pExtArg->byCmi = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/f"), cmdLen)) {
					if (!SetOptionF(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (argc >= 8 && cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/r"), cmdLen)) {
					pExtArg->byReverse = TRUE;
					s_nStartLBA = _tcstol(argv[i], &endptr, 10);
					if (*endptr) {
						OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
						return FALSE;
					}
					s_nEndLBA = _tcstol(argv[i + 1], &endptr, 10);
					if (*endptr) {
						OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
						return FALSE;
					}
					i += 2;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/sf"), cmdLen)) {
					if (!SetOptionSf(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 4 && !_tcsncmp(argv[i - 1], _T("/raw"), cmdLen)) {
					pExtArg->byRawDump = TRUE;
				}
				else if (cmdLen == 4 && !_tcsncmp(argv[i - 1], _T("/fix"), cmdLen)) {
					if (!SetOptionFix(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/re"), cmdLen)) {
					pExtArg->byResume = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/q"), cmdLen)) {
					pExtArg->byQuiet = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/d"), cmdLen)) {
					pExtArg->byDatExpand = TRUE;
				}
				else if (cmdLen == 5 && !_tcsncmp(argv[i - 1], _T("/avdp"), cmdLen)) {
					pExtArg->byAnchorVolumeDescriptorPointer = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/ps"), cmdLen)) {
					if (!SetOptionPs(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/rr"), cmdLen)) {
					if (!SetOptionRr(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/sk"), cmdLen)) {
					if (!SetOptionSk(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else {
					OutputErrorString("Unknown option: [%s]\n", argv[i - 1]);
					return FALSE;
				}
			}
			printAndSetPath(argv[3], pszFullPath, stFullPathlen);
		}
		else if (argc >= 5 && cmdLen == 4 && !_tcsncmp(argv[1], _T("xbox"), 4)) {
			*pExecType = xbox;
			s_uiSpeed = (UINT)_tcstoul(argv[4], &endptr, 10);
			if (*endptr) {
				OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
				return FALSE;
			}
			for (INT i = 6; i <= argc; i++) {
				cmdLen = _tcslen(argv[i - 1]);
				if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/f"), cmdLen)) {
					if (!SetOptionF(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/q"), cmdLen)) {
					pExtArg->byQuiet = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/d"), cmdLen)) {
					pExtArg->byDatExpand = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/rr"), cmdLen)) {
					if (!SetOptionRr(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 4 && !_tcsncmp(argv[i - 1], _T("/nss"), cmdLen)) {
					if (!SetOptionNss(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else {
					OutputErrorString("Unknown option: [%s]\n", argv[i - 1]);
					return FALSE;
				}
			}
			printAndSetPath(argv[3], pszFullPath, stFullPathlen);
		}
		else if (argc >= 21 && cmdLen == 8 && !_tcsncmp(argv[1], _T("xboxswap"), cmdLen)) {
			*pExecType = xboxswap;
			s_uiSpeed = (UINT)_tcstoul(argv[4], &endptr, 10);
			if (*endptr) {
				OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
				return FALSE;
			}
			for (INT i = 5; i < 21; i++) {
				pExtArg->uiSecuritySector[i - 5] = (UINT)_tcstoul(argv[i], &endptr, 10);
				if (*endptr) {
					OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
					return FALSE;
				}
			}
			for (INT i = 22; i <= argc; i++) {
				cmdLen = _tcslen(argv[i - 1]);
				if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/f"), cmdLen)) {
					if (!SetOptionF(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/q"), cmdLen)) {
					pExtArg->byQuiet = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/d"), cmdLen)) {
					pExtArg->byDatExpand = TRUE;
				}
				else if (cmdLen == 4 && !_tcsncmp(argv[i - 1], _T("/nss"), cmdLen)) {
					if (!SetOptionNss(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else {
					OutputErrorString("Unknown option: [%s]\n", argv[i]);
					return FALSE;
				}
			}
			printAndSetPath(argv[3], pszFullPath, stFullPathlen);
		}
		else if (argc >= 8 && (cmdLen == 8 && (!_tcsncmp(argv[1], _T("xgd2swap"), cmdLen) || !_tcsncmp(argv[1], _T("xgd3swap"), cmdLen)))) {
			pExtArg->nAllSectors = (INT)_tcstoul(argv[5], &endptr, 10);
			if (*endptr) {
				OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
				return FALSE;
			}
			if (pExtArg->nAllSectors < 4000000) {
				*pExecType = xgd2swap;
			}
			else {
				*pExecType = xgd3swap;
			}
			s_uiSpeed = (UINT)_tcstoul(argv[4], &endptr, 10);
			if (*endptr) {
				OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
				return FALSE;
			}
			for (INT i = 6; i < 8; i++) {
				pExtArg->uiSecuritySector[i - 6] = (UINT)_tcstoul(argv[i], &endptr, 10);
				if (*endptr) {
					OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
					return FALSE;
				}
			}
			for (INT i = 9; i <= argc; i++) {
				cmdLen = _tcslen(argv[i - 1]);
				if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/f"), cmdLen)) {
					if (!SetOptionF(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/q"), cmdLen)) {
					pExtArg->byQuiet = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/d"), cmdLen)) {
					pExtArg->byDatExpand = TRUE;
				}
				else if (cmdLen == 4 && !_tcsncmp(argv[i - 1], _T("/nss"), cmdLen)) {
					if (!SetOptionNss(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else {
					OutputErrorString("Unknown option: [%s]\n", argv[i]);
					return FALSE;
				}
			}
			printAndSetPath(argv[3], pszFullPath, stFullPathlen);
		}
		else if (argc >= 5 && cmdLen == 4 && !_tcsncmp(argv[1], _T("sacd"), cmdLen)) {
			*pExecType = sacd;
			for (INT i = 6; i <= argc; i++) {
				if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/q"), 2)) {
					pExtArg->byQuiet = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/d"), cmdLen)) {
					pExtArg->byDatExpand = TRUE;
				}
			}
			printAndSetPath(argv[3], pszFullPath, stFullPathlen);
		}
		else if (argc >= 5 && cmdLen == 2 && !_tcsncmp(argv[1], _T("bd"), cmdLen)) {
			*pExecType = bd;
			s_uiSpeed = (UINT)_tcstoul(argv[4], &endptr, 10);
			if (*endptr) {
				OutputErrorString("[%s] is invalid argument. Please input integer.\n", endptr);
				return FALSE;
			}
			for (INT i = 6; i <= argc; i++) {
				cmdLen = _tcslen(argv[i - 1]);
				if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/f"), cmdLen)) {
					if (!SetOptionF(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/q"), cmdLen)) {
					pExtArg->byQuiet = TRUE;
				}
				else if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/d"), cmdLen)) {
					pExtArg->byDatExpand = TRUE;
				}
				else if (cmdLen == 3 && !_tcsncmp(argv[i - 1], _T("/rr"), cmdLen)) {
					if (!SetOptionRr(argc, argv, pExtArg, &i)) {
						return FALSE;
					}
				}
				else if (cmdLen == 5 && !_tcsncmp(argv[i - 1], _T("/avdp"), cmdLen)) {
					pExtArg->byAnchorVolumeDescriptorPointer = TRUE;
				}
				else {
					OutputErrorString("Unknown option: [%s]\n", argv[i - 1]);
					return FALSE;
				}
			}
			printAndSetPath(argv[3], pszFullPath, stFullPathlen);
		}
		else if (argc >= 4) {
			cmdLen = _tcslen(argv[1]);
			if (cmdLen == 2 && !_tcsncmp(argv[1], _T("fd"), cmdLen)) {
				*pExecType = fd;
				printAndSetPath(argv[3], pszFullPath, stFullPathlen);
				for (INT i = 5; i <= argc; i++) {
					cmdLen = _tcslen(argv[i - 1]);
					if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/d"), cmdLen)) {
						pExtArg->byDatExpand = TRUE;
					}
				}
			}
			else if (cmdLen == 4 && !_tcsncmp(argv[1], _T("disk"), cmdLen)) {
				*pExecType = disk;
				printAndSetPath(argv[3], pszFullPath, stFullPathlen);
				for (INT i = 5; i <= argc; i++) {
					cmdLen = _tcslen(argv[i - 1]);
					if (cmdLen == 2 && !_tcsncmp(argv[i - 1], _T("/d"), cmdLen)) {
						pExtArg->byDatExpand = TRUE;
					}
				}
			}
			else if (cmdLen == 5 && !_tcsncmp(argv[1], _T("merge"), cmdLen)) {
				*pExecType = merge;
				printAndSetPath(argv[2], pszFullPath, stFullPathlen);
			}
			else {
				OutputErrorString("Invalid argument\n");
				return FALSE;
			}
		}
		else if (argc == 3) {
			cmdLen = _tcslen(argv[1]);
			if (cmdLen == 4 && !_tcsncmp(argv[1], _T("stop"), cmdLen)) {
				*pExecType = stop;
			}
			else if (cmdLen == 5 && !_tcsncmp(argv[1], _T("start"), cmdLen)) {
				*pExecType = start;
			}
			else if (cmdLen == 5 && !_tcsncmp(argv[1], _T("eject"), cmdLen)) {
				*pExecType = ejecttray;
			}
			else if (cmdLen == 5 && !_tcsncmp(argv[1], _T("close"), cmdLen)) {
				*pExecType = closetray;
			}
			else if (cmdLen == 5 && !_tcsncmp(argv[1], _T("reset"), cmdLen)) {
				*pExecType = reset;
			}
			else if (cmdLen == 2 && !_tcsncmp(argv[1], _T("ls"), cmdLen)) {
				*pExecType = drivespeed;
			}
			else if (cmdLen == 3 && !_tcsncmp(argv[1], _T("sub"), cmdLen)) {
				*pExecType = sub;
				printAndSetPath(argv[2], pszFullPath, stFullPathlen);
			}
			else if (cmdLen == 3 && !_tcsncmp(argv[1], _T("mds"), cmdLen)) {
				*pExecType = mds;
				printAndSetPath(argv[2], pszFullPath, stFullPathlen);
			}
			else if (cmdLen == 4 && !_tcsncmp(argv[1], _T("tape"), cmdLen)) {
				*pExecType = tape;
				printAndSetPath(argv[2], pszFullPath, stFullPathlen);
			}
			else {
				OutputErrorString("Invalid argument\n");
				return FALSE;
			}
		}
		else {
			if (argc > 1) {
				OutputErrorString("Invalid argument\n");
			}
			return FALSE;
		}
	}
	return TRUE;
}

int createCmdFile(int argc, _TCHAR* argv[], _TCHAR* pszFullPath, LPTSTR pszDateTime)
{
	if (argc >= 4) {
		_TCHAR date[17] = {};
		_sntprintf(date, sizeof(date) / sizeof(_TCHAR), _T("_%s"), pszDateTime);
		FILE* fpCmd = CreateOrOpenFile(
			pszFullPath, date, NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0);
		if (!fpCmd) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
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

void printUsage(void)
{
	// Default console window size (windows)
	// 80 x 25 (windows 7)
	// 120 x 30 (windows 10)
	OutputString(
		"Usage\n"
		"\tcd <DriveLetter> <Filename> <DriveSpeed(0-72)> [/q] [/d] [/a (val)] [/aj] [/p]\n"
		"\t   [/be (str) or /d8] [/c2 (val1) (val2) (val3) (val4) (val5)] [/f (val)] [/ms]\n"
		"\t   [/vn (val)] [/vnc] [/vnx] [/mscf] [/sf (val)] [/ss] [/np] [/nq] [/nr]\n"
		"\t   [/nl] [/ns] [/s (val)] [/vrfy (val)]\n"
		"\t\tDump a CD from A to Z\n"
		"\t\tFor PLEXTOR or drive that can scramble Dumping\n"
		"\tswap <DriveLetter> <Filename> <DriveSpeed(0-72)> [/q] [/d] [/a (val)]\n"
		"\t   [/be (str) or /d8] [/c2 (val1) (val2) (val3) (val4) (val5)] [/f (val)]\n"
		"\t   [/p] [/ms] [/sf (val)] [/ss] [/np] [/nq] [/nr] [/nl] [/ns] [/s (val)] [/74]\n"
		"\t\tDump a CD from A to Z using swap trick\n"
		"\t\tFor no PLEXTOR or drive that can't scramble dumping\n"
		"\tdata <DriveLetter> <Filename> <DriveSpeed(0-72)> <StartLBA> <EndLBA+1>\n"
		"\t     [/q] [/d] [/be (str) or /d8] [/c2 (val1) (val2) (val3) (val4) (val5)] [/f (val)]\n"
		"\t     [/sf (val)] [/sk (val1) (val2)] [/ss] [/r] [/np] [/nq] [/nr] [/s (val)]\n"
		"\t\tDump a CD from start to end (using 'all' flag)\n"
		"\t\tFor no PLEXTOR or drive that can't scramble dumping\n"
		"\taudio <DriveLetter> <Filename> <DriveSpeed(0-72)> <StartLBA> <EndLBA+1>\n"
		"\t      [/q] [/d] [/a (val)] [/c2 (val1) (val2) (val3) (val4) (val5)] [/f (val)]\n"
		"\t      [/be (str) or /d8] [/sf (val)] [/np] [/nq] [/nr] [/s (val)]\n"
		"\t\tDump a CD from start to end (using 'cdda' flag)\n"
		"\t\tFor dumping a lead-in, lead-out mainly\n"
		"\tgd <DriveLetter> <Filename> <DriveSpeed(0-72)> [/q] [/d] [/be (str) or /d8]\n"
		"\t   [/c2 (val1) (val2) (val3) (val4) (val5)] [/f (val)] [/np] [/nq] [/nr] [/s (val)]\n"
		"\t\tDump a HD area of GD from A to Z\n"
		"\tdvd <DriveLetter> <Filename> <DriveSpeed(0-16)> [/c] [/f (val)] [/raw] [/q] [/d]\n"
		"\t    [/r (startLBA) (EndLBA)] [/avdp] [/ps (val)] [/rr (val)] [/sk (val)]\n"
	);
	stopMessage();
	OutputString(
		"\t\tDump a DVD from A to Z\n"
		"\txbox <DriveLetter> <Filename> <DriveSpeed(0-16)> [/f (val)] [/q] [/d] [/nss (val)] [/rr (val)]\n"
		"\t\tDump a xbox disc from A to Z\n"
		"\txboxswap <DriveLetter> <Filename> <DriveSpeed(0-16)>\n"
		"\t                                  <StartLBAOfSecuritySector1>\n"
		"\t                                  <StartLBAOfSecuritySector2>\n"
		"\t                                                 :            \n"
		"\t                                  <StartLBAOfSecuritySector16> [/f (val)] [/q] [/d]\n"
		"\t\tDump a Xbox disc from A to Z using swap trick\n"
		"\txgd2swap <DriveLetter> <Filename> <DriveSpeed(0-16)> <AllSectorLength>\n"
		"\t         <StartLBAOfSecuritySector1> <StartLBAOfSecuritySector2> [/f (val)] [/q] [/d]\n"
		"\t\tDump a XGD2 disc from A to Z using swap trick\n"
		"\txgd3swap <DriveLetter> <Filename> <DriveSpeed(0-16)> <AllSectorLength>\n"
		"\t         <StartLBAOfSecuritySector1> <StartLBAOfSecuritySector2> [/f (val)] [/q] [/d]\n"
		"\t\tDump a XGD3 disc from A to Z using swap trick\n"
		"\tsacd <DriveLetter> <Filename> <DriveSpeed(0-16)> [/q] [/d]\n"
		"\t\tDump a Super Audio CD from A to Z\n"
		"\tbd <DriveLetter> <Filename> <DriveSpeed(0-12)> [/f (val)] [/q] [/d] [/avdp] [/rr (val)]\n"
		"\t\tDump a BD from A to Z\n"
		"\tfd <DriveLetter> <Filename> [/d]\n"
		"\t\tDump a floppy disk\n"
		"\tdisk <DriveLetter> <Filename> [/d]\n"
		"\t\tDump a removable media other than floppy\n"
		"\tstop <DriveLetter>\n"
		"\t\tSpin off the disc\n"
		"\tstart <DriveLetter>\n"
		"\t\tSpin up the disc\n"
		"\teject <DriveLetter>\n"
		"\t\tEject the tray\n"
	);
	stopMessage();
	OutputString(
		"\tclose <DriveLetter>\n"
		"\t\tClose the tray\n"
		"\treset <DriveLetter>\n"
		"\t\tReset the drive (Only PLEXTOR)\n"
		"\tls <DriveLetter>\n"
		"\t\tShow maximum speed of the drive\n"
		"\tsub <Subfile>\n"
		"\t\tParse CloneCD sub file and output to readable format\n"
		"\tmds <Mdsfile>\n"
		"\t\tParse Alchohol 120/52 mds file and output to readable format\n"
		"\tmerge <plextor image file> <optiarc image file>\n"
		"\t\tmerge the two files (for physical error protection)\n"
		"Option (generic)\n"
		"\t/f\tUse 'Force Unit Access' flag to delete the drive cache\n"
		"\t\t\tval\tdelete per specified value (default: 1)\n"
		"\t/q\tDisable beep\n"
		"\t/d\tGenerate hash SHA-224, SHA-256, SHA-384, SHA-512, XXH3 64, XXH3 128\n"
		"Option (for CD read mode)\n"
		"\t/a\tAdd CD offset manually (Only Audio CD)\n"
		"\t\t\tval\tsamples value\n"
		"\t/vrfy\tCheck non-zero byte in the lead-out and pregap of 1st track (Only Audio CD)\n"
		"\t\t\tval\t0: audio cd can be read without offset (default)\n"
		"\t\t\t   \t1: audio cd can be read with offset\n"
		"\t\t\t   \t    and select 'y' then read without offset or select 'n' then not read\n"
		"\t\t\t   \t2: logs are only generated with offset\n"
		"\t/be\tUse 0xbe as the opcode for Reading CD forcibly\n"
		"\t\t\tstr\t raw: sub channel mode is raw (default)\n"
		"\t\t\t   \tpack: sub channel mode is pack\n"
		"\t/d8\tUse 0xd8 as the opcode for Reading CD forcibly\n"
	);
	stopMessage();
	OutputString(
		"\t/c2\tContinue reading CD to recover C2 error existing sector\n"
		"\t\t\tval1\tvalue to reread (default: 4000)\n"
		"\t\t\tval2\tvalue to set the C2 offset (default: 0)\n"
		"\t\t\tval3\t0: reread sector c2 error is reported (default)\n"
		"\t\t\t    \t1: reread all (or from first to last) sector\n"
		"\t\t\tval4\tfirst LBA to reread (default: 0)\n"
		"\t\t\tval5\tlast LBA to reread (default: end-of-sector)\n"
		"\t\t\t    \tval3, 4 is used when val2 is 1\n"
		"\t/p\tDumping the AMSF from 00:00:00 to 00:01:74\n"
		"\t\t\tFor SagaFrontier Original Sound Track (Disc 3) etc.\n"
		"\t\t\tSupport drive: PLEXTOR PX-W4012, 5224, PREMIUM, PREMIUM2\n"
		"\t\t\t               PX-704, 708, 712, 714, 716, 755, 760\n"
		"\t/r\tRead CD from the reverse\n"
		"\t\t\tFor Alpha-Disc, Tages (very slow)\n"
		"\t/ms\tRead the lead-out of 1st session and the lead-in of 2nd session\n"
		"\t\t\tFor Multi-session\n"
		"\t/74\tRead the lead-out about 74:00:00\n"
		"\t\t\tFor ring data (a.k.a Saturn Ring) of Sega Saturn\n"
		"\t/sf\tScan file to detect protect. If reading error exists,\n"
		"\t   \tcontinue reading and ignore c2 error on specific sector\n"
		"\t\t\tFor CodeLock, LaserLock, RingProtect, RingPROTECH\n"
		"\t\t\t    SafeDisc, SmartE, ProtectCD-VOB, CDS300\n"
		"\t\t\tval\ttimeout value (default: 60)\n"
		"\t/sk\tSkip sector for physical protect (e.g. proring, LaserLock etc.)\n"
		"\t\t\tval1\tsector num\n"
		"\t\t\tval2\tsector num (optional)\n"
		"\t/ss\tScan sector to detect protect. If reading error exists,\n"
		"\t   \tcontinue reading and ignore c2 error on specific sector\n"
		"\t\t\tFor ProtectCD-VOB\n"
		"\t/am\tScan anti-mod string\n"
	);
	stopMessage();
	OutputString(
		"\t\t\tFor PlayStation\n"
		"\t/vn\tSearch specific bytes\n"
		"\t\t\tFor VideoNow\n"
		"\t\t\tval\tCombined offset is shifted for negative direction if positive value is set\n"
		"\t/vnc\tSearch specific bytes\n"
		"\t\t\tFor VideoNow Color\n"
		"\t/vnx\tSearch specific bytes\n"
		"\t\t\tFor VideoNow XP\n"
		"\t/aj\tSearch specific bytes\n"
		"\t\t\tFor Atari Jaguar CD\n"
		"\t/mscf\tExtract MicroSoftCabFile (.cab)\n"
		"\t\t\tFor output exe file info in detail\n"
		"\t/mr\tMulti sector reading of the lead-out for 0xf1 drive\n"
		"\t\t\tval\tretry count (default 50)\n"
		"Option (for CD SubChannel)\n"
		"\t/np\tNot fix SubP\n"
		"\t/nq\tNot fix SubQ\n"
		"\t/nr\tNot fix SubRtoW\n"
		"\t/nl\tNot fix SubQ (RMSF, AMSF, CRC) (LBA 10000 - 19999)\n"
		"\t   \t                               (LBA 40000 - 49999)\n"
		"\t\t\tFor PlayStation LibCrypt\n"
		"\t/ns\tNot fix SubQ (RMSF, AMSF, CRC) (LBA 0 - 7, 5000 - 24999)\n"
		"\t   \t                            or (LBA 30000 - 49999)\n"
		"\t\t\tFor SecuROM\n"
		"\t/s\tUse if it reads subchannel precisely\n"
		"\t\t\tval\t0: no read next sub (fast, but lack precision)\n"
		"\t\t\t   \t1: read next sub (normal, this val is default)\n"
		"\t\t\t   \t2: read next & next next sub (slow, precision)\n"
		"Option (for DVD)\n"
	);
	stopMessage();
	OutputString(
		"\t/c\tLog Copyright Management Information\n"
		"\t/rr\tRetry reading when error occurs\n"
		"\t\t\tval\tMax retry value (default: 10)\n"
		"\t/sk\tSkip sector for protect (ARccOS, RipGuard)\n"
		"\t\t\tval\tsector num\n"
		"\t/nss\tNo skip reading SS sectors (only XBOX)\n"
		"\t\t\tval\tMax retry value (default: 100)\n"
		"\t/r\tRead DVD from the reverse\n"
		"\t/raw\tDumping DVD by raw (2064 or 2384 bytes/sector)\n"
		"\t\t\tComfirmed drive: Mediatec MT chip (Lite-on etc.), PLEXTOR\n"
		"\t\t\t               Hitachi-LG GDR, GCC\n"
		"\t\t\t -> GDR (8082N, 8161B to 8164B) and GCC (4160N, 4240N to 4247N)\n"
		"\t\t\t    supports GC/Wii dumping\n"
		"\t/avdp\tUse Anchor Volume Descriptor Pointer as file length\n"
		"\t/ps\tThe sector is padded when reading error occurs\n"
		"\t\t\tval\t0: Padded by 0x00\n"
		"\t\t\tval\t1: Padded by 0xAA\n"
		"\t/re\tResume raw (GC/Wii) dumping\n"
		"\t/fix\tRedump specified frame when completed raw (GC/Wii) dumping but failed to unscramble\n"
		"\t\t\tval\tframe number that unscrambler.exe showed in the command line screen\n"
		"See also -> https://github.com/saramibreak/DiscImageCreator/wiki \n"
	);
	stopMessage();
}

int printSeveralInfo(LPTSTR pszDateTime, size_t dateTimeSize)
{
#if 0
	if (!OutputWindowsVersion()) {
		return FALSE;
	}
#endif
	OutputString("AppVersion\n");
#if (defined(_WIN32) && defined(_WIN64)) || (defined(__linux__) && defined(__x86_64__))
	OutputString("\t64 bit, ");
#else
	OutputString("\t32 bit, ");
#endif
#ifdef UNICODE
	OutputString("UnicodeBuild, ");
#else
	OutputString("AnsiBuild, ");
#endif
	_sntprintf(pszDateTime, dateTimeSize, _T("%sT%s"), _T(BUILD_DATE), _T(BUILD_TIME));
	OutputString("%s\n", pszDateTime);
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
		extArg.uiCacheDelNum = DEFAULT_CACHE_DELETE_VAL;
		_TCHAR szFullPath[_MAX_PATH] = {};
		if (!checkArg(argc, argv, &execType, &extArg, szFullPath, sizeof(szFullPath) / sizeof(szFullPath[0]))) {
			if (argc == 1) {
				printUsage();
			}
			else {
				OutputErrorString("If you see the usage, execute .exe without argument\n");
			}
			nRet = FALSE;
		}
		else {
			_TCHAR szBuf[128] = {};
			time_t now = time(NULL);
			tm* ts = localtime(&now);
			_tcsftime(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), _T("%FT%T%z"), ts);
			OutputString("StartTime: %s\n", szBuf);

			if (execType != merge) {
				nRet = createCmdFile(argc, argv, szFullPath, szDateTime);
			}
			if (nRet) {
				nRet = exec(argv, &execType, &extArg, szFullPath);
			}

			now = time(NULL);
			ts = localtime(&now);
			_tcsftime(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), _T("%FT%T%z"), ts);
			OutputString("EndTime: %s\n", szBuf);
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
	nRet = nRet == TRUE ? EXIT_SUCCESS : EXIT_FAILURE;
	return nRet;
}

