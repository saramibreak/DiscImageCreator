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
#include "struct.h"
#include "calcHash.h"
#include "check.h"
#include "convert.h"
#include "execScsiCmd.h"
#include "execScsiCmdforCD.h"
#include "execScsiCmdforCDCheck.h"
#include "execScsiCmdforFileSystem.h"
#include "get.h"
#include "init.h"
#include "output.h"
#include "outputScsiCmdLogforCD.h"
#include "set.h"
#include "_external/NonStandardFunction.h"

BOOL ReadCDForSubChannelOffset(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpCmd,
	INT nLBA,
	LPBYTE lpBuf,
	UINT uiBufLen
) {
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	LPBYTE lpBufTmp = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		uiBufLen, &lpBufTmp, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	memcpy(lpBufTmp, lpBuf, uiBufLen);
	BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = {};
	for (INT i = 0; i < 5; i++) {
		if (uiBufLen == CD_RAW_READ_SUBCODE_SIZE) {
			AlignRowSubcode(lpSubcode, lpBufTmp);
		}
		else if (uiBufLen == CD_RAW_SECTOR_WITH_SUBCODE_SIZE) {
			AlignRowSubcode(lpSubcode, lpBufTmp + CD_RAW_SECTOR_SIZE);
		}
		else if (uiBufLen == CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE) {
			if (pDevice->driveOrder == DRIVE_DATA_ORDER::MainSubC2) {
				AlignRowSubcode(lpSubcode, lpBufTmp + CD_RAW_SECTOR_SIZE);
			}
			else {
				AlignRowSubcode(lpSubcode, lpBufTmp + CD_RAW_SECTOR_WITH_C2_294_SIZE);
			}
		}
		else if (uiBufLen == CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE) {
			if (pDevice->driveOrder == DRIVE_DATA_ORDER::MainSubC2) {
				AlignRowSubcode(lpSubcode, lpBufTmp + CD_RAW_SECTOR_SIZE);
			}
			else {
				AlignRowSubcode(lpSubcode, lpBufTmp + CD_RAW_SECTOR_WITH_C2_SIZE);
			}
		}
		OutputCDSub96Align(fileDisc, lpSubcode, nLBA);
		BOOL bCheckSubQAllZero = TRUE;
		for (INT j = 12; j < 24; j++) {
			if (lpSubcode[j] != 0) {
				bCheckSubQAllZero = FALSE;
				break;
			}
		}
		if (bCheckSubQAllZero) {
			OutputDiscLog("SubQ is all zero... (BufLen: %u)\n", uiBufLen);
			break;
		}

		if ((lpSubcode[12] & 0x0f) == ADR_ENCODES_CURRENT_POSITION) {
			pDisc->SUB.nSubChannelOffset = MSFtoLBA(BcdToDec(lpSubcode[19]),
				BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21])) - 150 - nLBA;
			break;
		}
		else {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, ++nLBA
				, lpBufTmp, uiBufLen, _T(__FUNCTION__), __LINE__)) {
				bRet = FALSE;
				break;
			}
		}
	}
	FreeAndNull(pBuf);
	return bRet;
}

BOOL CheckFrameOfVideoNowColorOrXp(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPBYTE lpCmd,
	INT tmpLBA,
	LPBYTE aVideoNowBytes,
	UINT uiBufSize,
	LPINT nSector,
	INT nRoopCnt,
	INT nHeaderOfs,
	INT nImcompFrmSize
) {
	OutputLog(standardOut | fileDisc, "Search incomplete frame of track 01 (see _maininfo.txt)\n");
	BOOL bRet = TRUE;
	LPBYTE pBuf3 = NULL;
	LPBYTE lpBuf3 = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf3,
		CD_RAW_SECTOR_SIZE * (UINT)nRoopCnt, &lpBuf3, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	BYTE aBuf[CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE] = {};
	(*nSector)--;
	for (INT j = *nSector; j < nRoopCnt + *nSector; j++) {
		if (!ExecReadCD(pExtArg, pDevice, lpCmd, tmpLBA + j
			, aBuf, uiBufSize, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		memcpy(lpBuf3 + CD_RAW_SECTOR_SIZE * (j - *nSector), aBuf, CD_RAW_SECTOR_SIZE);
		OutputMainChannel(fileMainInfo, lpBuf3 + CD_RAW_SECTOR_SIZE * (j - *nSector), NULL, tmpLBA + j, CD_RAW_SECTOR_SIZE);
	}
	INT n1stHeaderOfs = 0;
	INT n2ndHeaderOfs = 0;
	for (INT m = 0; m < CD_RAW_SECTOR_SIZE * nRoopCnt; m++) {
		for (size_t d = 0; d < sizeof(aVideoNowBytes); d++) {
			if (lpBuf3[m + d] != aVideoNowBytes[d]) {
				bRet = FALSE;
				break;
			}
			if (d == sizeof(aVideoNowBytes) - 1) {
				if (n1stHeaderOfs == 0) {
					n1stHeaderOfs = m;
					OutputLog(standardOut | fileDisc, "1stHeader (81,e3,e3,c7,c7,81,81,e3...) Ofs: %d (0x%x)\n"
						, n1stHeaderOfs, (UINT)n1stHeaderOfs);
					m += nHeaderOfs;
					break;
				}
				else {
					n2ndHeaderOfs = m;
					OutputLog(standardOut | fileDisc, "2ndHeader (81,e3,e3,c7,c7,81,81,e3...) Ofs: %d (0x%x)\n"
						, n2ndHeaderOfs, (UINT)n2ndHeaderOfs);
					OutputLog(standardOut | fileDisc
						, "Empty bytes which are needed in this disc: %d [%d - (%d - %d)]\n"
						, nImcompFrmSize - (n2ndHeaderOfs - n1stHeaderOfs)
						, nImcompFrmSize, n2ndHeaderOfs, n1stHeaderOfs
					);
					pExtArg->nAudioCDOffsetNum = nImcompFrmSize - (n2ndHeaderOfs - n1stHeaderOfs);
					bRet = TRUE;
					break;
				}
			}
		}
		if (bRet) {
			break;
		}
	}
	FreeAndNull(pBuf3);
	return bRet;
}

BOOL ExecSearchingOffset(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpCmd,
	INT nLBA,
	LPBYTE lpBuf,
	UINT uiBufSize,
	BOOL bGetDriveOffset,
	INT nDriveOffset
) {
	BOOL bRet = ExecReadCD(pExtArg, pDevice, lpCmd
		, nLBA, lpBuf, uiBufSize, _T(__FUNCTION__), __LINE__);

	if (!bRet) {
		if (*pExecType == gd) {
			OutputErrorString(
				"Couldn't read data sectors at scrambled state [OpCode: %#02x, C2flag: %d, SubCode: %x]\n"
				, lpCmd[0], (lpCmd[9] & 0x6) >> 1, lpCmd[10]);
		}
		else {
			if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
				OutputLog(standardError | fileDrive,
					"This drive doesn't support [OpCode: %#02x, SubCode: %x]\n", lpCmd[0], lpCmd[10]);
			}
			else {
				OutputErrorString(
					"This drive can't read data sectors at scrambled state [OpCode: %#02x, C2flag: %d, SubCode: %x]\n"
					, lpCmd[0], (lpCmd[9] & 0x6) >> 1, lpCmd[10]);
			}
		}
		return FALSE;
	}
	else {
		if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
			OutputLog(standardOut | fileDrive,
				"This drive supports [OpCode: %#02x, SubCode: %x]\n", lpCmd[0], lpCmd[10]);
		}
		else {
			if (*pExecType != data) {
				OutputLog(standardOut | fileDrive,
					"This drive can read data sectors at scrambled state [OpCode: %#02x, C2flag: %d, SubCode: %x]\n"
					, lpCmd[0], (lpCmd[9] & 0x6) >> 1, lpCmd[10]);
			}
		}
	}
	if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
		if (lpCmd[10] == CDFLAG::_PLXTR_READ_CDDA::MainQ ||
			lpCmd[10] == CDFLAG::_PLXTR_READ_CDDA::Raw) {
			// because check only
			return TRUE;
		}
		OutputLog(fileDisc | fileMainInfo, 
			OUTPUT_DHYPHEN_PLUS_STR_WITH_SUBCH_F("Check Drive + CD offset"), lpCmd[0], lpCmd[10]);
	}
	else if ((!pExtArg->byD8 && !pDevice->byPlxtrDrive) || pExtArg->byBe) {
		if (lpCmd[10] == CDFLAG::_READ_CD::Q) {
			// because check only
			return TRUE;
		}
		OutputLog(fileDisc | fileMainInfo,
			OUTPUT_DHYPHEN_PLUS_STR_WITH_C2_SUBCH_F("Check Drive + CD offset"), lpCmd[0], (lpCmd[9] & 0x6) >> 1, lpCmd[10]);
	}

	if (pDisc->SCSI.trkType != TRACK_TYPE::audioOnly || *pExecType == swap) {
		if (*pExecType != data) {
			OutputMainChannel(fileMainInfo, lpBuf, NULL, nLBA, CD_RAW_SECTOR_SIZE);
		}
	}
	if (uiBufSize == CD_RAW_SECTOR_WITH_SUBCODE_SIZE ||
		uiBufSize == CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE ||
		uiBufSize == CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE) {
		if (!ReadCDForSubChannelOffset(pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf, uiBufSize)) {
			return FALSE;
		}
	}
	if (*pExecType == data) {
		if (pDisc->SUB.nSubChannelOffset != 0xff) {
			OutputDiscLog("\tSubChannel Offset: %d\n", pDisc->SUB.nSubChannelOffset);
		}
	}
	else {
		if (pDisc->SCSI.trkType != TRACK_TYPE::audioOnly || *pExecType == swap) {
			// http://forum.redump.org/post/81925/#p81925
			INT nSectorNum = 5;
			LPBYTE pBuf2 = NULL;
			LPBYTE lpBuf2 = NULL;
			if (!GetAlignedCallocatedBuffer(pDevice, &pBuf2,
				CD_RAW_SECTOR_SIZE * (UINT)nSectorNum, &lpBuf2, _T(__FUNCTION__), __LINE__)) {
				return FALSE;
			}
			memcpy(lpBuf2, lpBuf, CD_RAW_SECTOR_SIZE);
			BYTE aBuf[CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE] = {};

			INT nTmpCombinedOffset = 0;
			for (INT k = 0; k < 2; k++) {
				for (INT i = 1; i < nSectorNum; i++) {
					if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA + i
						, aBuf, uiBufSize, _T(__FUNCTION__), __LINE__)) {
						return FALSE;
					}
					OutputMainChannel(fileMainInfo, aBuf, NULL, nLBA + i, CD_RAW_SECTOR_SIZE);
					memcpy(lpBuf2 + CD_RAW_SECTOR_SIZE * i, aBuf, CD_RAW_SECTOR_SIZE);
				}

				if (pDisc->SCSI.trkType != TRACK_TYPE::audioOnly || *pExecType == swap) {
					if (!GetWriteOffset(pDisc, lpBuf2, nSectorNum, nLBA)) {
						if (pDisc->SCSI.trkType == TRACK_TYPE::dataExist ||
							pDisc->SCSI.trkType == TRACK_TYPE::pregapDataIn1stTrack) {
							OutputLog(standardError | fileDisc, "Failed to get write-offset\n");
							return FALSE;
						}
						OutputLog(standardOut | fileDisc,
							"There isn't data sector in pregap sector of track 1. Change the TRACK_TYPE to audioOnly\n");
						pDisc->SCSI.trkType = TRACK_TYPE::audioOnly;
						break;
					}
					else {
						if (pDisc->MAIN.nCombinedOffset > 0) {
							INT nOverRead = pDisc->MAIN.nCombinedOffset / CD_RAW_SECTOR_SIZE;
							// [FMT] Sangokushi IV http://forum.redump.org/post/82784/#p82784
							// [FMT] Lip 3: Lipstick Adventure 3 http://forum.redump.org/post/80180/#p80180
							// [FMT] Gulf War: Soukouden http://forum.redump.org/topic/16418/addedfmt-5-new-dumps/
							if (nOverRead > 0) {
								if (k == 0) {
									SetAndOutputCDOffset(pExtArg, pDisc, bGetDriveOffset
										, pDevice->nDriveSampleOffset, nDriveOffset, pDisc->SUB.nSubChannelOffset);
									nTmpCombinedOffset = pDisc->MAIN.nCombinedOffset;
									nLBA += nOverRead;
									if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA
										, aBuf, uiBufSize, _T(__FUNCTION__), __LINE__)) {
										return FALSE;
									}
									OutputMainChannel(fileMainInfo, aBuf, NULL, nLBA, CD_RAW_SECTOR_SIZE);
									memcpy(lpBuf2, aBuf, CD_RAW_SECTOR_SIZE);
								}
								else if (k == 1) {
									if (nTmpCombinedOffset != pDisc->MAIN.nCombinedOffset) {
										OutputDiscLog("There is a different combined offset. See also _mainInfo.txt\n");
									}
								}
							}
							else {
								break;
							}
						}
						else {
							break;
						}
					}
				}
			}
			FreeAndNull(pBuf2);
		}
		else if (pDisc->SCSI.trkType == TRACK_TYPE::audioOnly &&
			(pExtArg->byVideoNow || pExtArg->byVideoNowColor || pExtArg->byVideoNowXp)) {
			LPBYTE pBuf2 = NULL;
			LPBYTE lpBuf2 = NULL;
			if (!GetAlignedCallocatedBuffer(pDevice, &pBuf2,
				CD_RAW_SECTOR_SIZE * 15, &lpBuf2, _T(__FUNCTION__), __LINE__)) {
				return FALSE;
			}
			memcpy(lpBuf2, lpBuf, CD_RAW_SECTOR_SIZE);
			BYTE aBuf[CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE] = {};

			for (INT k = 1; k < 15; k++) {
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA + k
					, aBuf, uiBufSize, _T(__FUNCTION__), __LINE__)) {
					return FALSE;
				}
				memcpy(lpBuf2 + CD_RAW_SECTOR_SIZE * k, aBuf, CD_RAW_SECTOR_SIZE);
			}
			BYTE aVideoNowBytesColor[] = {
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x80,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x80,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x80,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x80,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x80,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x80,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x80,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x80,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x80,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x80,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x80,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x80,
			};
			BYTE aVideoNowBytesXp[] = {
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x5a,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x61,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x77,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x92,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0xa8,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0xb7,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0xbf,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0xc4,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0xc1,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0xae,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x90,
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3, 0xc7, 0x7c,
			};
			// VideoNow B&W
			CONST BYTE aVideoNowBytesOrg[] = {
				0xe1, 0xe1, 0xe1, 0x01, 0xe1, 0xe1, 0xe1, 0x00,
			};
			INT nSector = 1;

			for (INT i = 0; i < CD_RAW_SECTOR_SIZE * 15; i++) {
				if (pExtArg->byVideoNowColor) {
					for (size_t c = 0; c < sizeof(aVideoNowBytesColor); c++) {
						if (lpBuf2[i + c] != aVideoNowBytesColor[c]) {
							bRet = FALSE;
							break;
						}
						if (c == sizeof(aVideoNowBytesColor) - 1) {
							OutputLog(standardOut | fileDisc, "Detected VideoNow Color or Jr.\n");
							// Color or Jr. disc have 19600 bytes per frame, but last of the 1st track have incomplete frame (18032 bytes)
							// If it's not 18032, it needs to insert empty bytes at the head of the 1st frame
							bRet = CheckFrameOfVideoNowColorOrXp(pExtArg, pDevice
								, lpCmd, 883, aVideoNowBytesColor, uiBufSize, &nSector, 9, 400, 18032);
						}
					}
				}
				else if (pExtArg->byVideoNowXp) {
					for (size_t c = 0; c < sizeof(aVideoNowBytesXp); c++) {
						if (lpBuf2[i + c] != aVideoNowBytesXp[c]) {
							bRet = FALSE;
							break;
						}
						if (c == sizeof(aVideoNowBytesXp) - 1) {
							// Xp disc have 19760 bytes per frame, but last of the 1st track have incomplete frame (20832 bytes)
							// If it's not 20832, it needs to insert empty bytes at the head of the 1st frame
							OutputLog(standardOut | fileDisc, "Detected VideoNow XP\n");
							bRet = CheckFrameOfVideoNowColorOrXp(pExtArg, pDevice
								, lpCmd, 882, aVideoNowBytesXp, uiBufSize, &nSector, 10, 560, 20832);
						}
					}
				}
				else if (pExtArg->byVideoNow) {
					for (size_t c = 0; c < sizeof(aVideoNowBytesOrg); c++) {
						if (lpBuf2[i + c] != aVideoNowBytesOrg[c]) {
							bRet = FALSE;
							break;
						}
						if (c == sizeof(aVideoNowBytesOrg) - 1) {
							OutputLog(standardOut | fileDisc, "Detected VideoNow B&W\n");
							bRet = TRUE;
						}
					}
				}
				if (bRet) {
					pDisc->MAIN.nCombinedOffset = i - pExtArg->nAudioCDOffsetNum;
					OutputMainChannel(fileMainInfo, lpBuf2 + CD_RAW_SECTOR_SIZE * nSector, NULL, nLBA + nSector, CD_RAW_SECTOR_SIZE);
					break;
				}
				else if (i == CD_RAW_SECTOR_SIZE * nSector - 1) {
					nSector++;
				}
			}
			if (!bRet) {
				OutputLog(standardError | fileDisc, "Couldn't find VideoNow header\n");
			}
			FreeAndNull(pBuf2);
		}
		else if (pDisc->SCSI.trkType == TRACK_TYPE::audioOnly && pExtArg->byAtari) {
			// Atari Jaguar CD Header
			//  00 00 54 41 49 52 54 41  49 52 54 41 49 52 54 41   ..TAIRTAIRTAIRTA
			//  49 52 54 41 49 52 54 41  49 52 54 41 49 52 54 41   IRTAIRTAIRTAIRTA
			//  49 52 54 41 49 52 54 41  49 52 54 41 49 52 54 41   IRTAIRTAIRTAIRTA
			//  49 52 54 41 49 52 54 41  49 52 54 41 49 52 54 41   IRTAIRTAIRTAIRTA
			//  49 52 54 41 52 41 20 49  50 41 52 50 56 4F 44 45   IRTARA IPARPVODE
			//  44 20 54 41 20 41 45 48  44 41 52 45 41 20 52 54   D TA AEHDAREA RT
			//  20 49                                               I
			// => "ATRIATRI ... ATARI APPROVED DATA HEADER ATRI "

			// Atari Jaguar CD Tailer
			//       54 41 52 41 20 49  50 41 52 50 56 4F 44 45     TARA IPARPVODE
			// 44 20 54 41 20 41 41 54  4C 49 52 45 41 20 52 54   D TA AATLIREA RT
			// 20 49 54 41 49 52 54 41  49 52 54 41 49 52 54 41    ITAIRTAIRTAIRTA
			// 49 52 54 41 49 52 54 41  49 52 54 41 49 52 54 41   IRTAIRTAIRTAIRTA
			// 49 52 54 41 49 52 54 41  49 52 54 41 49 52 54 41   IRTAIRTAIRTAIRTA
			// 49 52 54 41 49 52 54 41  49 52 54 41 49 52 54 41   IRTAIRTAIRTAIRTA
			// 49 52                                              IR
			// => "ATARI APPROVED DATA TAILER ATRI ATRIATRI ..."
			CONST BYTE aAtariBytes[] = {
				0x54, 0x41, 0x49, 0x52, 0x54, 0x41, 0x49, 0x52,
			};
			INT nSector = 0;
			nLBA = pDisc->SCSI.n1stLBAof2ndSession;
			do {
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA
					, lpBuf, uiBufSize, _T(__FUNCTION__), __LINE__)) {
					return FALSE;
				}
				for (INT i = 0; i < CD_RAW_SECTOR_SIZE; i++) {
					for (size_t c = 0; c < sizeof(aAtariBytes); c++) {
						if (lpBuf[i + c] != aAtariBytes[c]) {
							bRet = FALSE;
							break;
						}
						if (c == sizeof(aAtariBytes) - 1) {
							OutputLog(standardOut | fileDisc, "Detected Atari Jaguar CD Header\n");
							bRet = TRUE;
						}
					}
					if (bRet) {
						pDisc->MAIN.nCombinedOffset = i - 2 + CD_RAW_SECTOR_SIZE * nSector;
						OutputMainChannel(fileMainInfo, lpBuf, NULL, nLBA, CD_RAW_SECTOR_SIZE);
						break;
					}
				}
				if (!bRet) {
					nSector--;
					nLBA += nSector;
				}
			} while (!bRet);
		}
		SetAndOutputCDOffset(pExtArg, pDisc, bGetDriveOffset
			, pDevice->nDriveSampleOffset, nDriveOffset, pDisc->SUB.nSubChannelOffset);
	}
	return TRUE;
}

BOOL ReadCDForSearchingOffset(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	BOOL bRet = TRUE;
	BOOL bGetDriveOffset = GetDriveOffsetAuto(pDevice, &pDevice->nDriveSampleOffset);
#ifdef _DEBUG
	if (pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX760A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX755A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX716AL ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX716A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX714A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX712A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX708A2 ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX708A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX704A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PREMIUM2 ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PREMIUM ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXW5224A
		) {
		pDevice->nDriveSampleOffset = 30;
		bGetDriveOffset = TRUE;
	}
	else if (
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXW4824A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXW4012A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXW4012S
		) {
		pDevice->nDriveSampleOffset = 98;
		bGetDriveOffset = TRUE;
	}
	else if (!strncmp(pDevice->szProductId, "DVD-ROM TS-H353A", 16)) {
		pDevice->nDriveSampleOffset = 6;
		bGetDriveOffset = TRUE;
	}
#endif
	if (!bGetDriveOffset) {
		GetDriveOffsetManually(&pDevice->nDriveSampleOffset);
	}

	INT nDriveOffset = pDevice->nDriveSampleOffset * 4; // byte size * 4 = sample size
	if (pDisc->SCSI.trkType != TRACK_TYPE::dataExist &&
		pDisc->SCSI.trkType != TRACK_TYPE::pregapDataIn1stTrack) {
		pDisc->MAIN.nCombinedOffset = nDriveOffset;
	}
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	if (*pExecType == gd) {
		pDisc->SCSI.n1stLBAofDataTrk = FIRST_LBA_FOR_GD;
	}
	FlushLog();
	BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
	if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
		CDB::_PLXTR_READ_CDDA cdb = {};
		SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::NoSub);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);

		INT nLBA = pDisc->SCSI.n1stLBAofDataTrk;
		ZeroMemory(lpBuf, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE);

		if (pDisc->SCSI.trkType != TRACK_TYPE::audioOnly) {
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
				, CD_RAW_SECTOR_SIZE, bGetDriveOffset, nDriveOffset)) {
				bRet = FALSE;
			}
		}
		lpCmd[10] = (BYTE)CDFLAG::_PLXTR_READ_CDDA::MainQ;
		if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
			, CD_RAW_SECTOR_SIZE + 16, bGetDriveOffset, nDriveOffset)) {
			// not return FALSE
		}
		lpCmd[10] = (BYTE)CDFLAG::_PLXTR_READ_CDDA::MainPack;
		for (INT n = 1; n <= 10; n++) {
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
				, CD_RAW_SECTOR_WITH_SUBCODE_SIZE, bGetDriveOffset, nDriveOffset)) {
				if (n == 10) {
					bRet = FALSE;
					break;
				}
				StartStopUnit(pExtArg, pDevice, STOP_UNIT_CODE, STOP_UNIT_CODE);
				UINT milliseconds = 10000;
				OutputErrorString("Retry %d/10 after %u milliseconds\n", n, milliseconds);
				Sleep(milliseconds);
				continue;
			}
			else {
				break;
			}
		}
#if 0
		lpCmd[10] = (BYTE)CDFLAG::_PLXTR_READ_CDDA::Raw;
		if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
			, CD_RAW_READ_SUBCODE_SIZE, bGetDriveOffset, nDriveOffset)) {
			// not return FALSE
		}
#endif
		lpCmd[10] = (BYTE)CDFLAG::_PLXTR_READ_CDDA::MainC2Raw;
		if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
			, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, bGetDriveOffset, nDriveOffset)) {
			pExtArg->byC2 = FALSE;
			pDevice->FEATURE.byC2ErrorData = FALSE;
			// not return FALSE
		}
	}
	else {
		CDFLAG::_READ_CD::_EXPECTED_SECTOR_TYPE flg = CDFLAG::_READ_CD::CDDA;
		if (*pExecType == data) {
			flg = CDFLAG::_READ_CD::All;
		}
		CDB::_READ_CD cdb = {};
		SetReadCDCommand(pDevice, &cdb, flg
			, 1, CDFLAG::_READ_CD::NoC2, CDFLAG::_READ_CD::Raw);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);

		INT nLBA = pDisc->SCSI.n1stLBAofDataTrk;
		ZeroMemory(lpBuf, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE);

		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			SetReadCDCommand(pDevice, &cdb, flg
				, 1, CDFLAG::_READ_CD::byte294, CDFLAG::_READ_CD::NoSub);
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
			if (pDisc->SCSI.trkType != TRACK_TYPE::audioOnly) {
				// Audio only disc doesn't call this because of NoSub mode 
				if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_WITH_C2_294_SIZE, bGetDriveOffset, nDriveOffset)) {
					// not return FALSE
				}
			}
			lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Raw;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
				, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, bGetDriveOffset, nDriveOffset)) {
				bRet = FALSE;
			}
			lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Q;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
				, CD_RAW_SECTOR_WITH_C2_294_SIZE + 16, bGetDriveOffset, nDriveOffset)) {
				// not return FALSE
			}
			lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Pack;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
				, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, bGetDriveOffset, nDriveOffset)) {
				// not return FALSE
			}

			if (bRet) {
				pDevice->supportedC2Type = CDFLAG::_READ_CD::byte294;
			}
			else {
				bRet = TRUE;
				SetReadCDCommand(pDevice, &cdb, flg
					, 1, CDFLAG::_READ_CD::byte296, CDFLAG::_READ_CD::NoSub);
				memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
				if (pDisc->SCSI.trkType != TRACK_TYPE::audioOnly) {
					if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
						, CD_RAW_SECTOR_WITH_C2_SIZE, bGetDriveOffset, nDriveOffset)) {
						// not return FALSE
					}
				}
				lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Raw;
				if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE, bGetDriveOffset, nDriveOffset)) {
					bRet = FALSE;
				}
				lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Q;
				if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_WITH_C2_SIZE + 16, bGetDriveOffset, nDriveOffset)) {
					// not return FALSE
				}
				lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Pack;
				if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE, bGetDriveOffset, nDriveOffset)) {
					// not return FALSE
				}

				if (bRet) {
					pDevice->supportedC2Type = CDFLAG::_READ_CD::byte296;
					pDevice->TRANSFER.uiBufLen = CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE;
					pDevice->TRANSFER.uiBufSubOffset = CD_RAW_SECTOR_WITH_C2_SIZE;
				}
				else {
					pDevice->supportedC2Type = CDFLAG::_READ_CD::NoC2;
				}
			}
		}
		else {
			if (*pExecType != data && pDisc->SCSI.trkType != TRACK_TYPE::audioOnly) {
				lpCmd[10] = (BYTE)CDFLAG::_READ_CD::NoSub;
				// Audio only disc doesn't call this because of NoSub mode 
				if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_SIZE, bGetDriveOffset, nDriveOffset)) {
					// not return FALSE
				}
			}
			lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Raw;
			for (INT n = 1; n <= 10; n++) {
				if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_WITH_SUBCODE_SIZE, bGetDriveOffset, nDriveOffset)) {
					if (n == 10) {
						bRet = FALSE;
						break;
					}
					StartStopUnit(pExtArg, pDevice, STOP_UNIT_CODE, STOP_UNIT_CODE);
					UINT milliseconds = 10000;
					OutputErrorString("Retry %d/10 after %u milliseconds\n", n, milliseconds);
					Sleep(milliseconds);
					continue;
				}
				else {
					break;
				}
			}
			lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Q;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
				, CD_RAW_SECTOR_SIZE + 16, bGetDriveOffset, nDriveOffset)) {
				// not return FALSE
			}
			lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Pack;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
				, CD_RAW_SECTOR_WITH_SUBCODE_SIZE, bGetDriveOffset, nDriveOffset)) {
				// not return FALSE
			}
		}
	}
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadCDForCheckingPregapSync(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
	SetReadDiscCommand(pExecType, pExtArg, pDevice, 1, CDFLAG::_READ_CD::NoC2, CDFLAG::_READ_CD::Pack, lpCmd, FALSE);

	BYTE aBuf[CD_RAW_SECTOR_WITH_SUBCODE_SIZE] = {};
	BYTE byScsiStatus = 0;
	BOOL bFound = FALSE;
	INT nSyncPos = 0;
	INT nSectorNum = 0;

	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("Check Pregap sync, msf, mode"));
	for (INT nLBA = PREGAP_START_LBA; nLBA < -1150; nLBA++) {
		if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, aBuf,
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
		if (!bFound) {
			for (INT i = 0; i < CD_RAW_SECTOR_SIZE; i++) {
				if (IsValidMainDataHeader(aBuf + i) &&
					aBuf[i + 12] == 0x01 && aBuf[i + 13] == 0x80 && aBuf[i + 14] == 0) {
					nSyncPos = i;
					bFound = TRUE;
					break;
				}
			}
			if (!bFound) {
				OutputString("\rChecking Pregap sync, msf, mode (LBA) %6d", nLBA);
				continue;
			}
		}
		OutputDiscLog("\t%3d : %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n"
			, ++nSectorNum, aBuf[nSyncPos], aBuf[nSyncPos + 1], aBuf[nSyncPos + 2], aBuf[nSyncPos + 3]
			, aBuf[nSyncPos + 4], aBuf[nSyncPos + 5], aBuf[nSyncPos + 6], aBuf[nSyncPos + 7]
			, aBuf[nSyncPos + 8], aBuf[nSyncPos + 9], aBuf[nSyncPos + 10], aBuf[nSyncPos + 11]
			, aBuf[nSyncPos + 12], aBuf[nSyncPos + 13], aBuf[nSyncPos + 14], aBuf[nSyncPos + 15]
		);
		OutputString("\rChecking Pregap sync, msf, mode (LBA) %6d", nLBA);
		if (aBuf[nSyncPos + 12] == 0x01 && aBuf[nSyncPos + 13] == 0x81 && aBuf[nSyncPos + 14] == 0x74) {
			break;
		}
	}
	OutputString("\n");
	return TRUE;
}

BOOL IsThereNonZeroByte(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpCmd,
	INT nLBA,
	LPBYTE aBuf,
	LPINT nPos
) {
	BYTE byScsiStatus = 0;
	BOOL bRet = TRUE;

	if (IsValid0xF1SupportedDrive(pDevice) &&
		0 < nLBA - pDisc->SCSI.nAllLength && nLBA - pDisc->SCSI.nAllLength < (INT)pDisc->uiCachedSectorNum) {
		memcpy(aBuf, pDisc->lpCachedBuf + F1_BUFFER_SIZE * (nLBA - pDisc->SCSI.nAllLength), CD_RAW_SECTOR_SIZE);
	}
	else if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, aBuf,
		CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputLog(standardOut | fileDisc, "can't read\n");
		bRet = FALSE;
	}

	if (bRet) {
		INT nTmp = -0xff;
		if (nLBA < 76) {
			for (INT i = 0; i < CD_RAW_SECTOR_SIZE; i++) {
				if (aBuf[i] != 0) {
					nTmp = i;
					break;
				}
			}
		}
		else {
			for (INT i = CD_RAW_SECTOR_SIZE - 1; 0 <= i; i--) {
				if (aBuf[i] != 0) {
					nTmp = i;
					break;
				}
			}
		}
		*nPos = nTmp;
	}
	return bRet;
}

#define NOT_FOUND (-0xff)

BOOL ReadAudioCDForCheckingReadInOut(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	if (pExtArg->byAdd) {
		return TRUE;
	}
	if (pDisc->SCSI.lp1stLBAListOnToc[0] != 0 && !pExtArg->byPre) {
		OutputLog(standardOut | fileDisc, "1st LBA of TOC isn't 0. It's possibly HTOA disc. If you want to dump the pregap sector of the track 1, use /p\n");
		return TRUE;
	}

	BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
	SetReadDiscCommand(pExecType, pExtArg, pDevice, 1, CDFLAG::_READ_CD::byte294, CDFLAG::_READ_CD::Raw, lpCmd, FALSE);

	BYTE aBuf[CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE] = {};
	INT nLastSector = NOT_FOUND;
	INT nLastSectorPos = -1;
	INT n1stSector = NOT_FOUND;
	INT n1stSectorPos = -1;

	OutputLog(standardOut | fileDisc, "Check the last non-zero byte -> ");

	for (INT i = 74; -75 <= i; i--) {
		if (IsThereNonZeroByte(pExtArg, pDevice, pDisc, lpCmd, pDisc->SCSI.nAllLength + i, aBuf, &nLastSectorPos) && nLastSectorPos >= 0) {
			OutputMainChannel(fileMainInfo, aBuf, "last non-zero byte", pDisc->SCSI.nAllLength + i, CD_RAW_SECTOR_SIZE);
			OutputLog(standardOut | fileDisc, "Detected in %#x(%d) of LBA %d + %d\n", nLastSectorPos, nLastSectorPos, pDisc->SCSI.nAllLength, i);
			nLastSector = i;
			break;
		}
	}
	if (nLastSector == NOT_FOUND) {
		OutputLog(standardOut | fileDisc, "Not found from the last sector - 75 to the last sector + 75\n");
	}
	else if (17 <= nLastSector) {
		OutputLog(standardOut | fileDisc, "Non-zero byte position is over the +10,000 samples. Not fix the offset\n");
		pDisc->MAIN.bManySamples |= PLUS_10000_SAMPLES;
	}

	OutputLog(standardOut | fileDisc, "Check the 1st non-zero byte -> ");

	for (INT i = -75; i < 76; i++) {
		if (IsThereNonZeroByte(pExtArg, pDevice, pDisc, lpCmd, i, aBuf, &n1stSectorPos) && n1stSectorPos >= 0) {
			OutputMainChannel(fileMainInfo, aBuf, "1st non-zero byte", i, CD_RAW_SECTOR_SIZE);
			OutputLog(standardOut | fileDisc, "Detected in %#x(%d) of LBA %d\n", n1stSectorPos, n1stSectorPos, i);
			n1stSector = i;
			break;
		}
	}
	if (n1stSector == NOT_FOUND) {
		OutputLog(standardOut | fileDisc, "Not found from LBA -75 to 75\n");
	}
	else if (n1stSector <= -17) {
		OutputLog(standardOut | fileDisc, "Non-zero byte position is over the -10,000 samples. Not fix the offset\n");
		pDisc->MAIN.bManySamples |= MINUS_10000_SAMPLES;
	}

	if (0 <= n1stSector && pExtArg->byPre) {
		OutputLog(standardOut | fileDisc, "This disc doesn't have non-zero byte in the pregap area of track 1. /p was disabled\n");
		pExtArg->byPre = FALSE;
	}

	if ((pDisc->MAIN.bManySamples & PLUS_10000_SAMPLES) == PLUS_10000_SAMPLES ||
		(pDisc->MAIN.bManySamples & MINUS_10000_SAMPLES) == MINUS_10000_SAMPLES) {
		return TRUE;
	}
	else if (nLastSector != NOT_FOUND || n1stSector != NOT_FOUND) {
		if (nLastSector == n1stSector) {
			INT nTmpOffset = CD_RAW_SECTOR_SIZE * n1stSector + n1stSectorPos;
			if (nLastSectorPos + 1 == n1stSectorPos) {
				// [Audio-CD] Kirby's Dream Collection: Special Edition: Compilation Soundtrack ~ Hoshi no Kirby: 20 Shuunen Memorial Soundtrack
				INT nSample = (nTmpOffset - pDisc->MAIN.nCombinedOffset) / 4;
				OutputLog(standardOut | fileDisc, "Last non-zero byte position is equal to the 1st non-zero byte position. Set /a %d\n", nSample);
				ResetAndOutputCDOffset(pDevice, pExtArg, pDisc, nSample);
			}
			else if (n1stSectorPos < nLastSectorPos + 1) {
				OutputLog(standardOut | fileDisc, "Can't fix the combined offset because the last non-zero byte position is bigger than the 1st non-zero byte position\n");
			}
			else if (nLastSectorPos + 1 < n1stSectorPos) {
				if (nTmpOffset < pDisc->MAIN.nCombinedOffset) {
					INT nPos = nTmpOffset;
					INT corr = 4 - nPos % 4;
					if (corr != 4) {
						nPos -= corr;
					}
					INT nSample = (nPos - pDisc->MAIN.nCombinedOffset) / 4;
					OutputLog(standardOut | fileDisc
						, "Last non-zero byte position is smaller than the 1st non-zero byte position and the 1st non-zero byte position is smaller than the combined offset. Set /a %d\n", nSample);
					ResetAndOutputCDOffset(pDevice, pExtArg, pDisc, nSample);
				}
				else if (pDisc->MAIN.nCombinedOffset < CD_RAW_SECTOR_SIZE * nLastSector + nLastSectorPos){
					INT nPos = CD_RAW_SECTOR_SIZE * nLastSector + nLastSectorPos;
					INT corr = 4 - nPos % 4;
					if (corr != 4) {
						nPos += corr;
					}
					INT nSample = (nPos - pDisc->MAIN.nCombinedOffset) / 4;
					OutputLog(standardOut | fileDisc
						, "Last non-zero byte position is smaller than the 1st non-zero byte position and bigger than the combined offset. Set /a %d\n", nSample);
					ResetAndOutputCDOffset(pDevice, pExtArg, pDisc, nSample);
				}
				else {
					OutputLog(standardOut | fileDisc, "No need to fix the combined offset\n");
				}
			}
		}
		else if (nLastSector < n1stSector) {
			if (0 <= nLastSector && 0 <= n1stSector) {
				if (nLastSector == 0 && nLastSectorPos < pDisc->MAIN.nCombinedOffset) {
					OutputLog(standardOut | fileDisc, "Last non-zero byte position is smaller than the combined offset. No need to fix the combined offset\n");
				}
				else {
					INT nPos = CD_RAW_SECTOR_SIZE * nLastSector + nLastSectorPos + 1;
					INT corr = 4 - nPos % 4;
					if (corr != 4) {
						nPos += corr;
					}
					INT nSample = (nPos - pDisc->MAIN.nCombinedOffset) / 4;
					OutputLog(standardOut | fileDisc, "Last non-zero byte position is smaller than the 1st non-zero byte position. Set /a %d\n", nSample);
					ResetAndOutputCDOffset(pDevice, pExtArg, pDisc, nSample);
				}
			}
			else if (nLastSector < 0 && 0 <= n1stSector) {
				if (n1stSector == 0 && n1stSectorPos < pDisc->MAIN.nCombinedOffset) {
					INT nPos = n1stSectorPos;
					INT corr = 4 - nPos % 4;
					if (corr != 4) {
						nPos -= corr;
					}
					INT nSample = (nPos - pDisc->MAIN.nCombinedOffset) / 4;
					OutputLog(standardOut | fileDisc, "1st non-zero byte position is smaller than the combined offset. Set /a %d\n", nSample);
					ResetAndOutputCDOffset(pDevice, pExtArg, pDisc, nSample);
				}
				else {
					OutputLog(standardOut | fileDisc, "No need to fix the combined offset\n");
				}
			}
			else if (nLastSector < 0 && n1stSector < 0) {
				INT diff = n1stSectorPos - CD_RAW_SECTOR_SIZE;
				INT mod = diff % 4;
				INT corr = 0;
				if (mod != 0) {
					corr = mod / abs(mod);
				}
				INT nSample = (diff / 4 + corr) - 588 * (abs(n1stSector) - 1);
				OutputLog(standardOut | fileDisc, "Last non-zero byte position is smaller than the 1st non-zero byte position. Set /a %d\n", nSample);
				pDisc->MAIN.nCombinedOffset = nSample * 4;
				SetAndOutputCDOffset(pExtArg, pDisc, TRUE, pDevice->nDriveSampleOffset
					, pDevice->nDriveSampleOffset * 4, pDisc->SUB.nSubChannelOffset);
				pDisc->MAIN.bResetOffset = TRUE;
			}
		}
		else if (n1stSector < nLastSector) {
			if (0 <= n1stSector && 0 <= nLastSector) {
				OutputLog(standardOut | fileDisc, "Can't fix the combined offset because the last non-zero byte position is bigger than 1st non-zero byte position\n");
			}
			else if (n1stSector < 0 && 0 <= nLastSector) {
				if (n1stSector != NOT_FOUND) {
					OutputLog(standardOut | fileDisc, "Can't fix the combined offset because non-zero byte exists in the lead-out and the pregap area\n");
				}
				else {
					if (nLastSector == 0 && nLastSectorPos < pDisc->MAIN.nCombinedOffset) {
						OutputLog(standardOut | fileDisc, "Last non-zero byte position is smaller than the combined offset. No need to fix the combined offset\n");
					}
					else {
					INT nPos = CD_RAW_SECTOR_SIZE * nLastSector + nLastSectorPos + 1;
						INT corr = 4 - nPos % 4;
						if (corr != 4) {
							nPos += corr;
						}
						INT nSample = (nPos - pDisc->MAIN.nCombinedOffset) / 4;
						OutputLog(standardOut | fileDisc, "Last non-zero byte position is bigger than the combined offset. Set /a %d\n", nSample);
						ResetAndOutputCDOffset(pDevice, pExtArg, pDisc, nSample);
					}
				}
			}
			else if (n1stSector < 0 && nLastSector < 0) {
				if (n1stSector != NOT_FOUND && nLastSector != NOT_FOUND) {
					OutputLog(standardOut | fileDisc, "Can't fix the combined offset because the 1st non-zero byte position is bigger than the last non-zero byte position\n");
				}
				else {
					OutputLog(standardOut | fileDisc, "No need to fix the combined offset\n");
				}
			}
		}
	}
	return TRUE;
}

BOOL ReadCDForCheckingReadInOut(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
	SetReadDiscCommand(pExecType, pExtArg, pDevice, 1, CDFLAG::_READ_CD::NoC2, CDFLAG::_READ_CD::NoSub, lpCmd, FALSE);

	INT nLBA = 0;
	if (pDisc->MAIN.nCombinedOffset < 0) {
		OutputLog(standardOut | fileDrive, "Checking reading lead-in -> ");
		nLBA = -1;
	}
	else if (0 < pDisc->MAIN.nCombinedOffset && *pExecType == cd) {
		OutputLog(standardOut | fileDrive, "Checking reading lead-out -> ");
		nLBA = pDisc->SCSI.nAllLength;
	}
	BOOL bRet = TRUE;
	BYTE aBuf[CD_RAW_SECTOR_SIZE] = {};
	BYTE byScsiStatus = 0;
	if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, aBuf,
		CD_RAW_SECTOR_SIZE, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		if (pDisc->MAIN.nCombinedOffset < 0) {
			OutputLog(standardOut | fileDrive, "This drive can't read the lead-in\n");
		}
		else if (0 < pDisc->MAIN.nCombinedOffset) {
			OutputLog(standardOut | fileDrive, "This drive can't read the lead-out\n");
			if (IsValid0xF1SupportedDrive(pDevice)) {
				if (pDisc->SCSI.bMultiSession) {
					OutputLog(standardError | fileDrive, "Multi-session disc is not supported by this drive. Use Plextor DVD model (See README.md)\n");
					return FALSE;
				}
				if (!pExtArg->byMultiSectorReading) {
					OutputLog(standardError | fileDrive, "/mr <val> is needed to read the lead-out. <val> is 50 by default\n");
					return FALSE;
				}
				OutputLog(standardOut | fileDrive, "But 0xF1 opcode is supported\n");
				INT ct = 20;
				OutputLog(standardOut | fileDisc
					, OUTPUT_DHYPHEN_PLUS_STR("Reading %d - %d INTO CACHE")
					, nLBA - 1 - ct, nLBA - 1);

				BYTE aLastSector[CD_RAW_SECTOR_SIZE] = {};
				BOOL bCached = FALSE;
				for (UINT r = 0; r < pExtArg->uiRetryCnt && !bCached; r++) {
					for (INT x = nLBA - 1 - ct; x <= nLBA - 1; ++x) {
						if (!ExecReadCD(pExtArg, pDevice, lpCmd, x, aBuf,
							CD_RAW_SECTOR_SIZE, _T(__FUNCTION__), __LINE__)
							|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
							return FALSE;
						}
						if (x == nLBA - 1) {
							memcpy(aLastSector, aBuf, CD_RAW_SECTOR_SIZE);
							OutputMainChannel(fileMainInfo, aLastSector, "Last Sector", nLBA - 1, CD_RAW_SECTOR_SIZE);
						}
					}
					LPBYTE lpOutBuf = NULL;
					if (!GetAlignedCallocatedBuffer(pDevice, &pDisc->lpCachedBuf
						, (UINT)F1_BUFFER_SIZE * 100, &lpOutBuf, _T(__FUNCTION__), __LINE__)) {
						return FALSE;
					}
					try {
						INT nStartLBA = nLBA - 1 - ct;
						for (INT nLineNum = 0; nLineNum < 120; ++nLineNum) {
							if (!ReadCacheForLgAsus(pExtArg, pDevice, pDisc, lpOutBuf, nLineNum, nStartLBA + nLineNum, &bCached)) {
								throw FALSE;
							}
							if (bCached) {
								break;
							}
						}
						if (!bCached) {
							OutputLog(standardError | fileDisc, "Unabled to get the cache. Retry %u/%u\n", r + 1, pExtArg->uiRetryCnt);
							continue;
						}
						else if ((INT)pDisc->uiCachedSectorNum < pDisc->MAIN.nAdjustSectorNum + 1) {
							OutputLog(standardError | fileDisc, "Cache is short. Retry %u/%u\n", r + 1, pExtArg->uiRetryCnt);
							bCached = FALSE;
							continue;
						}
					}
					catch (BOOL bErr) {
						bRet = bErr;
						FreeAndNull(pDisc->lpCachedBuf);
					}
				}
				if (!bCached) {
					OutputErrorString("Failed to get the cache\n");
					bRet = FALSE;
				}
			}
			else {
				return FALSE;
			}
		}
	}
	else {
		if (nLBA != 0) {
			OutputLog(standardOut | fileDrive, "OK\n");
		}
	}
	return bRet;
}

BOOL ReadCDForCheckingSubQ1stIndex(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
	if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
		pDevice->sub = CDFLAG::_READ_CD::Pack;
		SetReadDiscCommand(pExecType, pExtArg, pDevice, 1, CDFLAG::_READ_CD::NoC2, CDFLAG::_READ_CD::Pack, lpCmd, FALSE);
	}
	else {
		SetReadDiscCommand(pExecType, pExtArg, pDevice, 1, CDFLAG::_READ_CD::NoC2, CDFLAG::_READ_CD::Raw, lpCmd, FALSE);
	}

	INT nLBA = 0;
	BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = {};
	BYTE aBuf[CD_RAW_SECTOR_WITH_SUBCODE_SIZE] = {};
	BYTE byScsiStatus = 0;
	do {
		if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, aBuf,
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
		else {
			AlignRowSubcode(lpSubcode, aBuf + CD_RAW_SECTOR_SIZE);
			if ((BYTE)(lpSubcode[12] & 0x0f) == ADR_ENCODES_CURRENT_POSITION) {
				pDisc->SUB.byCtlOfLBA0 = BcdToDec((BYTE)((lpSubcode[12] >> 4) & 0x0f));
				pDisc->SUB.byIdxOfLBA0 = BcdToDec(lpSubcode[14]);
				break;
			}
			else {
				nLBA++;
			}
		}
	} while (1);
#if 0
	OutputMainChannel(fileMainInfo, aBuf, NULL, nLBA, CD_RAW_SECTOR_WITH_SUBCODE_SIZE);
#endif
	return TRUE;
}

BOOL ReadCDForCheckingSubQAdrFirst(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE* ppBuf,
	LPBYTE* lpBuf,
	LPBYTE lpCmd,
	LPUINT uiBufLen,
	LPINT nOfs
) {
	if (!GetAlignedCallocatedBuffer(pDevice, ppBuf,
		CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * 2, lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	CDFLAG::_READ_CD::_ERROR_FLAGS c2 = CDFLAG::_READ_CD::NoC2;
	CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION flg = CDFLAG::_READ_CD::Pack;
	BYTE byTransferLen = 1;
	if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		byTransferLen = 2;
		*uiBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * 2;
		c2 = CDFLAG::_READ_CD::byte294;
		flg = CDFLAG::_READ_CD::Raw;
	}
	else if (*pExecType == gd) {
		flg = CDFLAG::_READ_CD::Raw;
	}
	SetReadDiscCommand(NULL, pExtArg, pDevice, byTransferLen, c2, flg, lpCmd, FALSE);
	*nOfs = pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE;
	if (pDisc->MAIN.nCombinedOffset < 0) {
		*nOfs = CD_RAW_SECTOR_SIZE + *nOfs;
	}
	return TRUE;
}

BOOL ReadCDForCheckingSubQAdr(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	LPBYTE lpCmd,
	LPBYTE lpBuf,
	UINT uiBufLen,
	INT nOfs,
	BYTE byIdxOfTrack,
	LPBYTE byMode,
	BYTE bySessionNum,
	FILE* fpCcd
) {
	BOOL bCheckMCN = FALSE;
	BOOL bCheckISRC = FALSE;
	BOOL bCheckAdr6 = FALSE;
	CHAR szTmpCatalog[META_CATALOG_SIZE] = {};
	CHAR szTmpISRC[META_ISRC_SIZE] = {};
	CHAR szTmpAdr6[META_ADR6_SIZE] = {};
	INT nMCNIdx = 0;
	INT nISRCIdx = 0;
	INT nTmpMCNLBAList[25] = { -1 };
	INT nTmpISRCLBAList[25] = { -1 };
	INT nTmpLBA = pDisc->SCSI.lp1stLBAListOnToc[byIdxOfTrack];
	INT nTmpNextLBA = 0;
	if (byIdxOfTrack + 1 < pDisc->SCSI.byLastDataTrkNum) {
		nTmpNextLBA = pDisc->SCSI.lp1stLBAListOnToc[byIdxOfTrack + 1] - nTmpLBA;
	}
	else {
		nTmpNextLBA = pDisc->SCSI.nAllLength - nTmpLBA;
	}
	pDiscPerSector->byTrackNum = BYTE(byIdxOfTrack + 1);
	INT nSubOfs = CD_RAW_SECTOR_SIZE;
	if (pDevice->driveOrder == DRIVE_DATA_ORDER::MainC2Sub) {
		nSubOfs = CD_RAW_SECTOR_WITH_C2_294_SIZE;
	}

	OutputLog(fileDisc | fileMainInfo, 
		OUTPUT_DHYPHEN_PLUS_STR_WITH_TRACK_F("Check MCN and/or ISRC"), lpCmd[0], lpCmd[10], byIdxOfTrack + 1);
	for (INT nLBA = nTmpLBA; nLBA < nTmpLBA + 500; nLBA++) {
		if (500 > nTmpNextLBA) {
			bCheckMCN = FALSE;
			bCheckISRC = FALSE;
			bCheckAdr6 = FALSE;
			break;
		}
		if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, lpBuf,
			uiBufLen, _T(__FUNCTION__), __LINE__)) {
			// skip checking
			return TRUE;
		}
		AlignRowSubcode(pDiscPerSector->subcode.current, lpBuf + nSubOfs);
#if 0
		OutputMainChannel(fileMainInfo, lpBuf2, NULL, nLBA, CD_RAW_SECTOR_SIZE);
		OutputCDSub96Align(pDiscPerSector->subcode.current, nLBA);
#endif
		if (nLBA == nTmpLBA) {
			memcpy(pDiscPerSector->mainHeader.current, lpBuf + nOfs, MAINHEADER_MODE1_SIZE);
			// this func is used to get a SubChannel Offset
			SetTmpSubchFromBuffer(&pDiscPerSector->subch.current, pDiscPerSector->subcode.current);
			pDiscPerSector->subch.current.byCtl = (BYTE)((BYTE)(pDiscPerSector->subcode.current[12] >> 4) & 0x0f);
			*byMode = GetMode(pDiscPerSector, unscrambled);
		}
		BOOL bCRC = FALSE;
		WORD crc16 = (WORD)GetCrc16CCITT(10, &pDiscPerSector->subcode.current[12]);
		BYTE tmp1 = HIBYTE(crc16);
		BYTE tmp2 = LOBYTE(crc16);
		if (pDiscPerSector->subcode.current[22] == tmp1 && pDiscPerSector->subcode.current[23] == tmp2) {
			bCRC = TRUE;
		}
		BYTE byAdr = (BYTE)(pDiscPerSector->subcode.current[12] & 0x0f);
		if (byAdr == ADR_ENCODES_MEDIA_CATALOG) {
#if 0
			if (!bCRC) {
				SetBufferFromMCN(pDisc, pDiscPerSector->subcode.current);
				bCRC = TRUE;
			}
#endif
			BOOL bMCN = IsValidSubQAdrMCN(pDiscPerSector->subcode.current);
#if 0
			if (!bMCN && bCRC) {
				// force a invalid MCN to valid MCN
				bMCN = bCRC;
			}
#endif
			if (bMCN && bCRC) {
				nTmpMCNLBAList[nMCNIdx++] = nLBA;
				CHAR szCatalog[META_CATALOG_SIZE] = {};
				if (!bCheckMCN) {
					SetMCNToString(pDisc, pDiscPerSector->subcode.current, szCatalog, FALSE);
					strncpy(szTmpCatalog, szCatalog, sizeof(szTmpCatalog) / sizeof(szTmpCatalog[0]));
					szTmpCatalog[META_CATALOG_SIZE - 1] = 0;
					bCheckMCN = bMCN;
				}
				else if (!pDisc->SUB.byCatalog) {
					SetMCNToString(pDisc, pDiscPerSector->subcode.current, szCatalog, FALSE);
					if (!strncmp(szTmpCatalog, szCatalog, sizeof(szTmpCatalog) / sizeof(szTmpCatalog[0]))) {
						strncpy(pDisc->SUB.szCatalog, szCatalog, sizeof(pDisc->SUB.szCatalog) / sizeof(pDisc->SUB.szCatalog[0]));
						pDisc->SUB.byCatalog = (BYTE)bMCN;
						OutputCDSub96Align(fileDisc, pDiscPerSector->subcode.current, nLBA);
						OutputDiscLog("\tMCN: [%" CHARWIDTH "s]\n", szCatalog);
						WriteCcdForDiscCatalog(pDisc, fpCcd);
					}
				}
			}
		}
		else if (byAdr == ADR_ENCODES_ISRC) {
			BOOL bISRC = IsValidSubQAdrISRC(pDiscPerSector->subcode.current);
#if 0
			if (!bISRC && bCRC) {
				// force a invalid ISRC to valid ISRC
				bISRC = bCRC;
			}
#endif
			if (bISRC && bCRC) {
				nTmpISRCLBAList[nISRCIdx++] = nLBA;
				CHAR szISRC[META_ISRC_SIZE] = {};
				if (!bCheckISRC) {
					SetISRCToString(pDisc, pDiscPerSector, szISRC, FALSE);
					strncpy(szTmpISRC, szISRC, sizeof(szTmpISRC) / sizeof(szTmpISRC[0]));
					szTmpISRC[META_ISRC_SIZE - 1] = 0;
					bCheckISRC = bISRC;
				}
				else if (!pDisc->SUB.lpISRCList[byIdxOfTrack]) {
					SetISRCToString(pDisc, pDiscPerSector, szISRC, FALSE);
					if (!strncmp(szTmpISRC, szISRC, sizeof(szISRC) / sizeof(szISRC[0]))) {
						strncpy(pDisc->SUB.pszISRC[byIdxOfTrack], szISRC, META_ISRC_SIZE);
						pDisc->SUB.lpISRCList[byIdxOfTrack] = bISRC;
						OutputCDSub96Align(fileDisc, pDiscPerSector->subcode.current, nLBA);
						OutputDiscLog(
							"\tISRC: [%" CHARWIDTH "s]\n"
							"\t    => Country: %.2" CHARWIDTH "s\n"
							"\t    =>   Owner: %.3" CHARWIDTH "s\n"
							"\t    =>    Year: %.2" CHARWIDTH "s\n"
							"\t    =>  Serial: %.5" CHARWIDTH "s\n"
							, szISRC, szISRC, &szISRC[2], &szISRC[5], &szISRC[7]
						);
					}
				}
			}
		}
		else if (byAdr == ADR_ENCODES_6) {
			BOOL bAdr6 = TRUE;
			CHAR szAdr6[META_ADR6_SIZE] = {};
			if (!bCheckAdr6) {
				SetAdr6ToString(pDisc, pDiscPerSector->subcode.current, szAdr6, FALSE);
				strncpy(szTmpAdr6, szAdr6, sizeof(szTmpAdr6) / sizeof(szTmpAdr6[0]));
				szTmpAdr6[META_ADR6_SIZE - 1] = 0;
				bCheckAdr6 = bAdr6;
			}
			else if (!pDisc->SUB.byAdr6) {
				SetAdr6ToString(pDisc, pDiscPerSector->subcode.current, szAdr6, FALSE);
				if (!strncmp(szTmpAdr6, szAdr6, sizeof(szTmpAdr6) / sizeof(szTmpAdr6[0]))) {
					strncpy(pDisc->SUB.szAdr6, szAdr6, sizeof(pDisc->SUB.szAdr6) / sizeof(pDisc->SUB.szAdr6[0]));
					pDisc->SUB.byAdr6 = (BYTE)bAdr6;
				}
			}
		}
		else {
			OutputMainInfoWithLBALog("No MCN, ISRC sector\n", nLBA, byIdxOfTrack + 1);
		}
	}
	if (bCheckMCN) {
		SetLBAForFirstAdr(pDisc->SUB.n1stLBAForMCN, pDisc->SUB.nRangeLBAForMCN,
			_T("MCN"), nTmpMCNLBAList, (BYTE)(bySessionNum - 1), pDevice->byPlxtrDrive);
	}
	if (bCheckISRC) {
		SetLBAForFirstAdr(pDisc->SUB.n1stLBAForISRC, pDisc->SUB.nRangeLBAForISRC,
			_T("ISRC"), nTmpISRCLBAList, (BYTE)(bySessionNum - 1), pDevice->byPlxtrDrive);
	}
	if (!bCheckMCN && !bCheckISRC) {
		OutputDiscLog("\tNothing\n");
	}
	return TRUE;
}

BOOL ReadCDForCheckingSubRtoW(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
	UINT uiBufLen = CD_RAW_SECTOR_SIZE + CD_RAW_READ_SUBCODE_SIZE;
	CDFLAG::_READ_CD::_ERROR_FLAGS c2 = CDFLAG::_READ_CD::NoC2;
	CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION flg = CDFLAG::_READ_CD::Pack;
	if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		uiBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
		c2 = CDFLAG::_READ_CD::byte294;
		flg = CDFLAG::_READ_CD::Raw;
	}
	else if (*pExecType == gd) {
		flg = CDFLAG::_READ_CD::Raw;
	}
	SetReadDiscCommand(NULL, pExtArg, pDevice, 1, c2, flg, lpCmd, FALSE);

	for (BYTE i = (BYTE)(pDisc->SCSI.toc.FirstTrack - 1); i < pDisc->SCSI.toc.LastTrack; i++) {
		try {
			OutputDiscLog(
				OUTPUT_DHYPHEN_PLUS_STR_WITH_TRACK_F("Check CD + G")
				, lpCmd[0], lpCmd[10], i + 1);
			INT nTmpLBA = pDisc->SCSI.lp1stLBAListOnToc[i] + 100;
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, nTmpLBA, lpBuf,
				uiBufLen, _T(__FUNCTION__), __LINE__)) {
				// skip checking
				continue;
			}
			BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = {};
			BYTE lpSubcodeOrg[CD_RAW_READ_SUBCODE_SIZE] = {};
			if (uiBufLen == CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE) {
				AlignRowSubcode(lpSubcode, lpBuf + CD_RAW_SECTOR_WITH_C2_294_SIZE);
				memcpy(lpSubcodeOrg, lpBuf + CD_RAW_SECTOR_WITH_C2_294_SIZE, CD_RAW_READ_SUBCODE_SIZE);
			}
			else {
				AlignRowSubcode(lpSubcode, lpBuf + CD_RAW_SECTOR_SIZE);
				memcpy(lpSubcodeOrg, lpBuf + CD_RAW_SECTOR_SIZE, CD_RAW_READ_SUBCODE_SIZE);
			}
			OutputCDSub96Align(fileDisc, lpSubcode, nTmpLBA);

			SUB_R_TO_W scRW[4] = {};
			BYTE tmpCode[24] = {};
			INT nRtoW = 0;
			BOOL bCDG = FALSE;
			BOOL bCDEG = FALSE;
			for (INT k = 0; k < 4; k++) {
				for (INT j = 0; j < 24; j++) {
					tmpCode[j] = (BYTE)(*(lpSubcodeOrg + (k * 24 + j)) & 0x3f);
				}
				memcpy(&scRW[k], tmpCode, sizeof(scRW[k]));
				switch (scRW[k].command) {
				case 0: // MODE 0, ITEM 0
					break;
				case 8: // MODE 1, ITEM 0
					break;
				case 9: // MODE 1, ITEM 1
					bCDG = TRUE;
					break;
				case 10: // MODE 1, ITEM 2
					bCDEG = TRUE;
					break;
				case 17: // MODE 2, ITEM 1,2,3,5,6,7 or MODE 4
				case 18:
				case 19:
				case 21:
				case 22:
				case 23:
				case 32:
					break;
				case 24: // MODE 3, ITEM 0
					break;
				case 56: // MODE 7, ITEM 0
					break;
				default:
					break;
				}
			}
			INT nR = 0;
			INT nS = 0;
			INT nT = 0;
			INT nU = 0;
			INT nV = 0;
			INT nW = 0;
			for (INT j = 24; j < CD_RAW_READ_SUBCODE_SIZE; j++) {
				if (24 <= j && j < 36) {
					nR += lpSubcode[j];
				}
				else if (36 <= j && j < 48) {
					nS += lpSubcode[j];
				}
				else if (48 <= j && j < 60) {
					nT += lpSubcode[j];
				}
				else if (60 <= j && j < 72) {
					nU += lpSubcode[j];
				}
				else if (72 <= j && j < 84) {
					nV += lpSubcode[j];
				}
				else if (84 <= j && j < CD_RAW_READ_SUBCODE_SIZE) {
					nW += lpSubcode[j];
				}
				nRtoW += lpSubcode[j];
			}
			// 0xff * 72 = 0x47b8
			if (nRtoW == 0x47b8) {
				// Why R-W bit is full? Basically, a R-W bit should be off except CD+G or CD-MIDI
				//  Alanis Morissette - Jagged Little Pill (UK)
				//  WipEout 2097: The Soundtrack
				//  and more..
				// Sub Channel LBA 75
				// 	  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B
				// 	P 00 00 00 00 00 00 00 00 00 00 00 00
				// 	Q 01 01 01 00 01 00 00 00 03 00 2c b9
				// 	R ff ff ff ff ff ff ff ff ff ff ff ff
				// 	S ff ff ff ff ff ff ff ff ff ff ff ff
				// 	T ff ff ff ff ff ff ff ff ff ff ff ff
				// 	U ff ff ff ff ff ff ff ff ff ff ff ff
				// 	V ff ff ff ff ff ff ff ff ff ff ff ff
				// 	W ff ff ff ff ff ff ff ff ff ff ff ff
				pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::Full;
				OutputDiscLog("\tAll RtoW is 0xff\n");
			}
			// 0x7f * 6 + 0xff * 66 = 0x44b8
			else if (nRtoW == 0x44b8) {
				// [IBM-PC] Panzer General (US) (rerelease)
				// ========== LBA[000000, 0000000], Sub Channel ==========
				// 	  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B
				//	P FF FF FF FF FF FF FF FF FF FF FF FF
				//	Q 41 01 01 00 00 00 00 00 02 00 28 32
				//	R 7F FF FF FF FF FF FF FF FF FF FF FF
				//	S 7F FF FF FF FF FF FF FF FF FF FF FF
				//	T 7F FF FF FF FF FF FF FF FF FF FF FF
				//	U 7F FF FF FF FF FF FF FF FF FF FF FF
				//	V 7F FF FF FF FF FF FF FF FF FF FF FF
				//	W 7F FF FF FF FF FF FF FF FF FF FF FF
				pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::Full;
				OutputDiscLog("\tAll RtoW is 0x7f, 0xff\n");
			}
			// (0x57 + 0x33 + 0x16) * 24 = 0xeb8
			else if (nRtoW == 0xeb8) {
				// [3DO] MegaRace (Japan) subch 0x02 on Plextor
				// ========== LBA[000000, 0000000], Sub Channel ==========
				// 	  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B
				// 	P ff ff ff ff ff ff ff ff ff ff ff ff
				// 	Q 41 01 01 00 00 00 00 00 02 00 28 32
				// 	R 57 33 13 57 33 13 57 33 13 57 33 13
				// 	S 57 33 13 57 33 13 57 33 13 57 33 13
				// 	T 57 33 13 57 33 13 57 33 13 57 33 13
				// 	U 57 33 13 57 33 13 57 33 13 57 33 13
				// 	V 57 33 13 57 33 13 57 33 13 57 33 13
				// 	W 57 33 13 57 33 13 57 33 13 57 33 13
				pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::Full;
				OutputDiscLog("\tAll RtoW is 0x57, 0x33, 0x13\n");
			}
			// 0x33 * 72 = 0xe58
			else if (nRtoW == 0xe58) {
				// [3DO] MegaRace (Japan) subch 0x08 on Plextor
				// ========== LBA[000100, 0x00064], Sub Channel ==========
				// 	  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B
				// 	P 00 00 00 00 00 00 00 00 00 00 00 00
				// 	Q 41 01 01 00 01 25 00 00 03 25 01 87
				// 	R 33 33 33 33 33 33 33 33 33 33 33 33
				// 	S 33 33 33 33 33 33 33 33 33 33 33 33
				// 	T 33 33 33 33 33 33 33 33 33 33 33 33
				// 	U 33 33 33 33 33 33 33 33 33 33 33 33
				// 	V 33 33 33 33 33 33 33 33 33 33 33 33
				// 	W 33 33 33 33 33 33 33 33 33 33 33 33
				pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::Full;
				OutputDiscLog("\tAll RtoW is 0x33\n");
			}
			else {
				BOOL bAnyFull = FALSE;
				// 0xff * 12 = 0xbf4
				if (nR == 0xbf4) {
					OutputDiscLog("\tAll R is 0xff\n");
					bAnyFull = TRUE;
				}
				if (nS == 0xbf4) {
					OutputDiscLog("\tAll S is 0xff\n");
					bAnyFull = TRUE;
				}
				if (nT == 0xbf4) {
					OutputDiscLog("\tAll T is 0xff\n");
					bAnyFull = TRUE;
				}
				if (nU == 0xbf4) {
					OutputDiscLog("\tAll U is 0xff\n");
					bAnyFull = TRUE;
				}
				if (nV == 0xbf4) {
					OutputDiscLog("\tAll V is 0xff\n");
					bAnyFull = TRUE;
				}
				if (nW == 0xbf4) {
					OutputDiscLog("\tAll W is 0xff\n");
					bAnyFull = TRUE;
				}
				if (bAnyFull) {
					pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::AnyFull;
				}
				else {
					if (bCDG && nRtoW > 0 && nRtoW != 0x200) {
						pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::CDG;
						OutputDiscLog("\tCD+G\n");
					}
					else if (bCDEG && nRtoW > 0 && nRtoW != 0x200) {
						pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::CDG;
						OutputDiscLog("\tCD+EG\n");
					}
					else if ((0 <= nR && nR <= 0x03) && (0 <= nS && nS <= 0x03) &&
						(0 <= nT && nT <= 0x03) && (0 <= nU && nU <= 0x03) &&
						(0 <= nV && nV <= 0x03) && (0 <= nW && nW <= 0x03) && nRtoW != 0) {
						pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::PSXSpecific;
						OutputDiscLog("\tRandom data exists (PSX)\n");
					}
					else {
						pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::Zero;
						OutputDiscLog("\tNothing\n");
					}
				}
			}
		}
		catch (BOOL bErr) {
			bRet = bErr;
		}
		OutputString(
			"\rChecking SubRtoW (Track) %2d/%2u", i + 1, pDisc->SCSI.toc.LastTrack);
	}
	OutputString("\n");
	FreeAndNull(pBuf);
	return bRet;
}

BOOL IsImageSig(
	LPBYTE lpBuf,
	INT nSignature
) {
	WORD wMagic = MAKEWORD(lpBuf[0], lpBuf[1]);
	if (wMagic == nSignature) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsSecuromDllSig(
	LPBYTE lpBuf,
	UINT i
) {
	if (lpBuf[0 + i] == 0xca && lpBuf[1 + i] == 0xdd && lpBuf[2 + i] == 0xdd && lpBuf[3 + i] == 0xac &&
		lpBuf[4 + i] == 0x03 && lpBuf[5 + i] == 0xca && lpBuf[6 + i] == 0xdd && lpBuf[7 + i] == 0x00) {
		return TRUE;
	}
	return FALSE;
}

LONG GetOfsOfSecuromDllSig(
	LPBYTE lpBuf,
	UINT i
) {
	return MAKELONG(MAKEWORD(lpBuf[i + 82], lpBuf[i + 83]), MAKEWORD(lpBuf[i + 84], lpBuf[i + 85]));
}

#ifdef _WIN32
LRESULT WINAPI CabinetCallback(
	IN PVOID pMyInstallData,
	IN UINT Notification,
	IN UINT Param1,
	IN UINT Param2
) {
	UNREFERENCED_PARAMETER(Param2);
	LRESULT lRetVal = NO_ERROR;
	TCHAR szTarget[_MAX_PATH];
	FILE_IN_CABINET_INFO *pInfo = NULL;
	FILEPATHS *pFilePaths = NULL;

	memcpy(szTarget, pMyInstallData, _MAX_PATH);
	switch (Notification) {
	case SPFILENOTIFY_CABINETINFO:
		break;
	case SPFILENOTIFY_FILEINCABINET:
		pInfo = (FILE_IN_CABINET_INFO *)Param1;
		lstrcat(szTarget, pInfo->NameInCabinet);
		lstrcpy(pInfo->FullTargetName, szTarget);
		lRetVal = FILEOP_DOIT;  // Extract the file.
		break;
	case SPFILENOTIFY_NEEDNEWCABINET: // Unexpected.
		break;
	case SPFILENOTIFY_FILEEXTRACTED:
		pFilePaths = (FILEPATHS *)Param1;
//		printf("Extracting %s\n", pFilePaths->Target);
		break;
	case SPFILENOTIFY_FILEOPDELAYED:
		break;
	}
	return lRetVal;
}

BOOL ReadExeFromFile(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPCTSTR szFullPath,
	LPCTSTR szFileName
) {
	FILE* fp = _tfopen(szFullPath, _T("rb"));
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Failed to OpenFile: %s\n", szFullPath);
		return FALSE;
	}
	size_t bufsize = DISC_MAIN_DATA_SIZE * 32;
	LPBYTE lpBuf = NULL;
	if (NULL == (lpBuf = (LPBYTE)calloc(bufsize, sizeof(BYTE)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FcloseAndNull(fp);
		return FALSE;
	}
	fread(lpBuf, sizeof(BYTE), bufsize, fp);

	if (IsImageSig(lpBuf, IMAGE_DOS_SIGNATURE)) {
		PIMAGE_DOS_HEADER pIDh = (PIMAGE_DOS_HEADER)&lpBuf[0];
		OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR("%s"), szFileName);
		OutputFsImageDosHeader(pIDh);

		if (pIDh->e_lfanew >= (LONG)bufsize) {
			OutputVolDescLog("File address of new exe header is too big. Skip reading Image NT Header etc.\n");
		}
		else {
			if (IsImageSig(&lpBuf[pIDh->e_lfanew], IMAGE_NT_SIGNATURE)) {
				PIMAGE_NT_HEADERS32 pINH = (PIMAGE_NT_HEADERS32)&lpBuf[pIDh->e_lfanew];
				OutputFsImageNtHeader(pINH);

				DWORD dwExportVirtualAddress = pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
				DWORD dwExportSize = pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
				DWORD dwExportPointerToRawData = 0;
				DWORD dwImportVirtualAddress = pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
				DWORD dwImportSize = pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
				DWORD dwImportPointerToRawData = 0;
				DWORD dwResourceVirtualAddress = pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
				DWORD dwResourceSize = pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size;
				DWORD dwResourcePointerToRawData = 0;
				ULONG nOfs = pIDh->e_lfanew + sizeof(IMAGE_NT_HEADERS32);
				BOOL bSecurom = FALSE;

				for (INT i = 0; i < pINH->FileHeader.NumberOfSections; i++) {
					PIMAGE_SECTION_HEADER pISH = (PIMAGE_SECTION_HEADER)&lpBuf[nOfs];
					OutputFsImageSectionHeader(pExtArg, pDisc, pISH, &bSecurom);
					nOfs += sizeof(IMAGE_SECTION_HEADER);
					if (pISH->VirtualAddress <= dwExportVirtualAddress && dwExportVirtualAddress <= pISH->VirtualAddress + pISH->Misc.VirtualSize) {
						dwExportPointerToRawData = pISH->PointerToRawData + dwExportVirtualAddress - pISH->VirtualAddress;
					}
					if (pISH->VirtualAddress <= dwImportVirtualAddress && dwImportVirtualAddress <= pISH->VirtualAddress + pISH->Misc.VirtualSize) {
						dwImportPointerToRawData = pISH->PointerToRawData + dwImportVirtualAddress - pISH->VirtualAddress;
					}
					if (pISH->VirtualAddress <= dwResourceVirtualAddress && dwResourceVirtualAddress <= pISH->VirtualAddress + pISH->Misc.VirtualSize) {
						dwResourcePointerToRawData = pISH->PointerToRawData + dwResourceVirtualAddress - pISH->VirtualAddress;
					}
				}

				if (dwExportSize > 0) {
					if (bufsize < dwExportSize) {
						bufsize = dwExportSize;
						LPBYTE lpBuf2 = NULL;
						if (NULL == (lpBuf2 = (LPBYTE)realloc(lpBuf, dwExportSize))) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							FreeAndNull(lpBuf);
							FcloseAndNull(fp);
							return FALSE;
						}
						lpBuf = lpBuf2;
					}
					fseek(fp, (LONG)dwExportPointerToRawData, SEEK_SET);
					fread(lpBuf, sizeof(BYTE), bufsize, fp);
					OutputMainInfoLog("dwExportVirtualAddress: 0x%lx, dwExportSize: 0x%lx\n", dwExportVirtualAddress, dwExportSize);
					OutputMainChannel(fileMainInfo, lpBuf, NULL, 0, bufsize + bufsize % 16);
					OutputExportDirectory(lpBuf, bufsize, dwExportVirtualAddress, 0);
				}

				if (dwImportSize > 0) {
					if (bufsize < dwImportSize) {
						bufsize = dwImportSize;
						LPBYTE lpBuf2 = NULL;
						if (NULL == (lpBuf2 = (LPBYTE)realloc(lpBuf, dwImportSize))) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							FreeAndNull(lpBuf);
							FcloseAndNull(fp);
							return FALSE;
						}
						lpBuf = lpBuf2;
					}
					fseek(fp, (LONG)dwImportPointerToRawData, SEEK_SET);
					fread(lpBuf, sizeof(BYTE), bufsize, fp);
					OutputMainInfoLog("dwImportVirtualAddress: 0x%lx, dwImportSize: 0x%lx\n", dwImportVirtualAddress, dwImportSize);
					OutputMainChannel(fileMainInfo, lpBuf, NULL, 0, bufsize + bufsize % 16);
					OutputImportDirectory(lpBuf, bufsize, dwImportVirtualAddress, 0);
				}

				if (dwResourceSize > 0) {
					if (bufsize < dwResourceSize) {
						bufsize = dwResourceSize;
						LPBYTE lpBuf2 = NULL;
						if (NULL == (lpBuf2 = (LPBYTE)realloc(lpBuf, dwResourceSize))) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							FreeAndNull(lpBuf);
							FcloseAndNull(fp);
							return FALSE;
						}
						lpBuf = lpBuf2;
					}
					fseek(fp, (LONG)dwResourcePointerToRawData, SEEK_SET);
					fread(lpBuf, sizeof(BYTE), bufsize, fp);
					OutputMainInfoLog("dwResourceVirtualAddress: 0x%lx, dwResourceSize: 0x%lx\n", dwResourceVirtualAddress, dwResourceSize);
					OutputMainChannel(fileMainInfo, lpBuf, NULL, 0, bufsize + bufsize % 16);
					_TCHAR szTab[256] = {};
					szTab[0] = _T('\t');
					WCHAR wszFileVer[FILE_VERSION_SIZE] = { 0 };
					OutputResourceDirectory(lpBuf, bufsize, dwResourceVirtualAddress, 0, 0, wszFileVer, szTab);
					if (wszFileVer[0] != 0 && strcasestr(szFileName, ".EXE")) {
						OutputLog(standardOut | fileDisc, " %s: File Version %ls\n", szFileName, wszFileVer);
					}
				}

				if (bSecurom) {
					UINT uiSecuromReadSize = DISC_MAIN_DATA_SIZE * 2;
					fseek(fp, -4, SEEK_END);
					UINT uiOfsOfSecuRomDll = 0;
					fread(&uiOfsOfSecuRomDll, sizeof(UINT), 1, fp);
					if (uiOfsOfSecuRomDll) {
						fseek(fp, (LONG)uiOfsOfSecuRomDll, SEEK_SET);
						fread(lpBuf, sizeof(BYTE), (size_t)uiSecuromReadSize, fp);
						if (!strncmp((LPCH)lpBuf, "AddD", 4)) {
							OutputMainChannel(fileMainInfo, lpBuf, "SecuROM Header", 0, uiSecuromReadSize);

							UINT uiOfsOf16 = 0;
							UINT uiOfsOf32 = 0;
							UINT uiOfsOfNT = 0;
							UINT uiSizeOf16 = 0;
							UINT uiSizeOf32 = 0;
							UINT uiSizeOfNT = 0;
							OutputSecuRomDllHeader(lpBuf, &uiOfsOf16, &uiOfsOf32, &uiOfsOfNT, &uiSizeOf16, &uiSizeOf32, &uiSizeOfNT);

							fseek(fp, (LONG)uiOfsOf16, SEEK_SET);
							fread(lpBuf, sizeof(BYTE), (size_t)uiSizeOf16, fp);
							OutputMainInfoLog(OUTPUT_DHYPHEN_PLUS_STR("Sintf16.dll [F:%s]"), __FUNCTION__);
							OutputMainChannel(fileMainInfo, lpBuf, NULL, 0, uiSizeOf16);
							OutputSint16(lpBuf, 0);

							fseek(fp, (LONG)uiOfsOf32, SEEK_SET);
							fread(lpBuf, sizeof(BYTE), (size_t)uiSizeOf32, fp);
							OutputMainInfoLog(OUTPUT_DHYPHEN_PLUS_STR("Sintf32.dll [F:%s]"), __FUNCTION__);
							OutputMainChannel(fileMainInfo, lpBuf, NULL, 0, uiSizeOf32);
							OutputSint32(lpBuf, 0, uiSizeOf32, FALSE);

							fseek(fp, (LONG)uiOfsOfNT, SEEK_SET);
							fread(lpBuf, sizeof(BYTE), (size_t)uiSizeOfNT, fp);
							OutputMainInfoLog(OUTPUT_DHYPHEN_PLUS_STR("SintfNT.dll [F:%s]"), __FUNCTION__);
							OutputMainChannel(fileMainInfo, lpBuf, NULL, 0, uiSizeOfNT);
							OutputSintNT(lpBuf, 0, uiSizeOfNT, FALSE);
						}
					}
					else if (pExtArg->byIntentionalSub) {
						rewind(fp);
						fread(lpBuf, sizeof(BYTE), (size_t)uiSecuromReadSize, fp);
						BOOL bFound = FALSE;
						while (!feof(fp) && !ferror(fp)) {
							for (UINT i = 0; i < uiSecuromReadSize - 8; i++) {
								if (IsSecuromDllSig(lpBuf, i)) {
									LONG lSigPos = (LONG)(ftell(fp) - uiSecuromReadSize + i);
									LONG lSigOfs = GetOfsOfSecuromDllSig(lpBuf, i);
									if (lSigPos == lSigOfs) {
										OutputSecuRomDll4_87Header(lpBuf, i);
										OutputMainChannel(fileMainInfo, lpBuf, NULL, 0, uiSecuromReadSize);
										bFound = TRUE;
										break;
									}
								}
							}
							if (bFound) {
								break;
							}
							else {
								fread(lpBuf, sizeof(BYTE), (size_t)uiSecuromReadSize, fp);
							}
						}
					}
				}
			}
			else if (IsImageSig(&lpBuf[pIDh->e_lfanew], IMAGE_OS2_SIGNATURE)) {
				OutputFsImageOS2Header((PIMAGE_OS2_HEADER)&lpBuf[pIDh->e_lfanew]);
			}
			else if (IsImageSig(&lpBuf[pIDh->e_lfanew], IMAGE_OS2_SIGNATURE_LE)) {
				// TODO
			}
			else {
				OutputVolDescLog(
					"%s: ImageNT,NE,LEHeader doesn't exist\n", szFileName);
			}
		}
	}
	FreeAndNull(lpBuf);
	FcloseAndNull(fp);
	return TRUE;
}

#define FILE_DELETE 1
#define FILE_SEARCH 2
#define FILE_CREATE 3

BOOL ProcessDirectory(
	PEXT_ARG pExtArg, 
	PDISC pDisc,
	LPTSTR szExtractdir,
	INT nOperate
) {
	_TCHAR szExtractdirFind[_MAX_PATH];
	memcpy(szExtractdirFind, szExtractdir, _MAX_PATH);
	_tcscat(szExtractdirFind, _T("*"));

	if (PathFileExists(szExtractdir)) {
		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFile(szExtractdirFind, &fd);
		if (INVALID_HANDLE_VALUE == hFind) {
			return FALSE;
		}
		do {
			if (_tcscmp(fd.cFileName, _T(".")) &&
				_tcscmp(fd.cFileName, _T(".."))) {
				_TCHAR szFoundFilePathName[_MAX_PATH];
				_tcsncpy(szFoundFilePathName, szExtractdir, sizeof(szFoundFilePathName) / sizeof(szFoundFilePathName[0]));
				_tcsncat(szFoundFilePathName, fd.cFileName, sizeof(szFoundFilePathName) / sizeof(szFoundFilePathName[0]) - _tcslen(szFoundFilePathName));

				if (!(FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes)) {
					if (FILE_ATTRIBUTE_READONLY & fd.dwFileAttributes) {
						SetFileAttributes(szFoundFilePathName, fd.dwFileAttributes - FILE_ATTRIBUTE_READONLY);
					}
					if (nOperate == FILE_DELETE || nOperate == FILE_CREATE) {
						if (!DeleteFile(szFoundFilePathName)) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							OutputErrorString("Failed to DeleteFile %s\n", fd.cFileName);
							FindClose(hFind);
							return FALSE;
						}
					}
					else if (nOperate == FILE_SEARCH) {
						if (!ReadExeFromFile(pExtArg, pDisc, szFoundFilePathName, fd.cFileName)) {
							return FALSE;
						}
					}
				}
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	if (nOperate == FILE_CREATE) {
#ifdef UNICODE
		if (SHCreateDirectory(NULL, szExtractdir)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			OutputErrorString("Failed to SHCreateDirectory: %s\n", szExtractdir);
			return FALSE;
		}
#else
		if (!MakeSureDirectoryPathExists(szExtractdir)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			OutputErrorString("Failed to MakeSureDirectoryPathExists: %s\n", szExtractdir);
			return FALSE;
		}
#endif
	}
	else if (nOperate == FILE_DELETE) {
		if (!RemoveDirectory(szExtractdir)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			OutputErrorString("Failed to RemoveDirectory: %s\n", szExtractdir);
			return FALSE;
		}
	}
	return TRUE;
}
#endif
VOID GetFullPathWithDrive(
	PDEVICE pDevice,
	PDISC pDisc,
	INT nIdx,
	_TCHAR* FullPathWithDrive,
	size_t FullPathWithDriveLen
) {
	CHAR FullPathTmp[_MAX_PATH] = {};
	CHAR drive[_MAX_DRIVE] = {};
	CHAR dir[_MAX_DIR] = {};
	CHAR filename[_MAX_FNAME] = {};
	CHAR ext[_MAX_EXT] = {};
#ifdef _WIN32
	_snprintf(drive, sizeof(drive), "%c:", pDevice->byDriveLetter);
#else
	_snprintf(drive, sizeof(drive), "%s", pDevice->drivepath);
#endif
	_splitpath(pDisc->PROTECT.pFullNameForExe[nIdx], NULL, dir, filename, ext);
	_makepath(FullPathTmp, drive, dir, filename, ext);

#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0,
		FullPathTmp, sizeof(FullPathTmp), FullPathWithDrive, (INT)FullPathWithDriveLen);
#else
	strncpy(FullPathWithDrive, FullPathTmp, FullPathWithDriveLen);
#endif
}

BOOL ReadCDForCheckingExe(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE pCdb,
	LPBYTE lpBuf
) {
	BOOL bRet = TRUE;
	DWORD dwSize = DISC_MAIN_DATA_SIZE;
	BYTE byTransferLen = 1;
	BYTE byRoopLen = byTransferLen;
	SetCommandForTransferLength(pExecType, pDevice, pCdb, dwSize, &byTransferLen, &byRoopLen);

#ifdef _WIN32
	BOOL bIscCab = FALSE;
#endif
	for (INT n = 0; pDisc->PROTECT.pExtentPosForExe[n] != 0; n++) {
		if (!ExecReadCD(pExtArg, pDevice, pCdb, pDisc->PROTECT.pExtentPosForExe[n],
			lpBuf, dwSize, _T(__FUNCTION__), __LINE__)) {
			continue;
		}
		BOOL bCab = FALSE;
		_TCHAR FullPathWithDrive[_MAX_PATH] = {};
		GetFullPathWithDrive(pDevice, pDisc, n, FullPathWithDrive, sizeof(FullPathWithDrive));
#ifdef _WIN32
		if (strcasestrW(FullPathWithDrive, _T("directx"))) {
			continue;
		}
#endif
		if (strcasestr(pDisc->PROTECT.pNameForExe[n], ".CAB") ||
			strcasestr(pDisc->PROTECT.pNameForExe[n], ".HDR")) {

			_TCHAR szTmpPath[_MAX_PATH] = {};
			if (!GetTempPath(sizeof(szTmpPath) / sizeof(szTmpPath[0]), szTmpPath)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			
			if (!strncmp((LPCCH)&lpBuf[0], "MSCF", 4)) {
				OutputString(
					"\nDetected Microsoft Cabinet File: %" CHARWIDTH "s\n"
					, pDisc->PROTECT.pFullNameForExe[n]
				);
				if (pExtArg->byMicroSoftCabFile) {
#ifdef _WIN32
					OutputString(
						"Please wait until all files are extracted. This is needed to search protection\n"
					);
					_tcscat(szTmpPath, _T("!extracted\\"));
					ProcessDirectory(pExtArg, pDisc, szTmpPath, FILE_CREATE);
					if (!SetupIterateCabinet(FullPathWithDrive, 0, (PSP_FILE_CALLBACK)CabinetCallback, szTmpPath)) {
						// 
					}
					// Search exe, dll from extracted file
					ProcessDirectory(pExtArg, pDisc, szTmpPath, FILE_SEARCH);
					ProcessDirectory(pExtArg, pDisc, szTmpPath, FILE_DELETE);
					bCab = TRUE;
#else
					// TODO: linux can use cabextract
#endif
				}
				else {
					OutputString("/mscf is needed to extract it\n");
				}
			}
			else if (!strncmp((LPCCH)&lpBuf[0], "ISc(", 4)) {
#ifdef _WIN32
				OutputString(
					"\nDetected InstallShield Cabinet File: %" CHARWIDTH "s\n"
					, pDisc->PROTECT.pFullNameForExe[n]
				);
				if (!bIscCab) {
					OutputString(
						"Please wait until all files are extracted. This is needed to search protection\n"
					);
				}
				else {
					continue;
				}
				_TCHAR szPathIsc[_MAX_PATH] = {};
				bRet = GetCmd(szPathIsc, _T("i6comp"), _T("exe"));

				if (bRet && PathFileExists(szPathIsc)) {
					_TCHAR szTmpFullPath[_MAX_PATH] = {};
					_tcsncpy(szTmpFullPath, szTmpPath, sizeof(szTmpFullPath) / sizeof(szTmpFullPath[0]));
					_tcscat(szTmpPath, _T("!exelist.txt"));

					CONST INT nStrSize = _MAX_PATH * 2;
					_TCHAR str[nStrSize] = {};
					LPCTSTR szSearchList[] = { _T(".exe"), _T(".dll") };

					for (INT i = 0; i < 2; i++) {
						// Get all .exe or .dll list from .cab
						// i6comp.exe output the file list like this
						// 07-12-1999 12:45          0 A___          0    5 iKernel.exe
						// 09-16-2002 19:12    2379215 A___    1125708   40 Lithtech.exe
						// 09-16-2002 15:45    1658880 A___     718535   46 SierraUp.exe
						_sntprintf(str, nStrSize,
							_T("\"\"%s\" l -o \"%s\"\" 2> NUL | findstr /e /i %s > %s")
							, szPathIsc, FullPathWithDrive, szSearchList[i], szTmpPath);
#ifdef _DEBUG
						OutputString("%s\n", str);
#endif
						_tsystem(str);

						FILE* fp = _tfopen(szTmpPath, _T("r"));
						if (!fp) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							continue;
						}

						_TCHAR buf[512] = {};
						CONST INT nMaxTrimSize = 16;
						INT nTrimSize = 0;
						_fgetts(buf, sizeof(buf), fp);

						while (!feof(fp) && !ferror(fp)) {
							LPTCH pTrimBuf[nMaxTrimSize] = {};
							pTrimBuf[0] = _tcstok(buf, _T(" ")); // space

							for (INT nRoop = 1; nRoop < nMaxTrimSize; nRoop++) {
								pTrimBuf[nRoop] = _tcstok(NULL, _T(" ")); // space

								if (pTrimBuf[nRoop] == NULL) {
									break;
								}
								nTrimSize++;
							}
							// File size is over 0
							if (_tcscmp(pTrimBuf[4], _T("0"))) {
								OutputVolDescLog("Extracted from %" CHARWIDTH "s\n", pDisc->PROTECT.pFullNameForExe[n]);
								// extract .exe or .dll from .cab
								_sntprintf(str, nStrSize,
									_T("\"\"%s\" e -o \"%s\" %s\" 2> NUL"), szPathIsc, FullPathWithDrive, pTrimBuf[5]);
								_tsystem(str);

								if (!GetCurrentDirectory(sizeof(szTmpFullPath) / sizeof(szTmpFullPath[0]), szTmpFullPath)) {
									OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
									return FALSE;
								}
								_TCHAR fname[_MAX_PATH] = {};
								size_t len = 0;

//#define DEBUGTEST4
#ifdef DEBUGTEST1
								memcpy(pTrimBuf[6], "TCP", sizeof("TCP"));
								if (pTrimBuf[7] == NULL) {
									pTrimBuf[7] =(LPTCH) malloc(32);
									memcpy(pTrimBuf[7], "Protocol.dll\n", sizeof("Protocol.dll\n"));
								}
								nTrimSize = 7;
#endif
#ifdef DEBUGTEST2
								memcpy(pTrimBuf[6], "TCP", sizeof("TCP"));
								if (pTrimBuf[7] == NULL) {
									pTrimBuf[7] = (LPTCH)malloc(32);
									memcpy(pTrimBuf[7], "version\\Protocol.dll\n", sizeof("version\\Protocol.dll\n"));
								}
								nTrimSize = 7;
#endif
#ifdef DEBUGTEST3
								memcpy(pTrimBuf[6], "TCP", sizeof("TCP"));
								if (pTrimBuf[7] == NULL) {
									pTrimBuf[7] = (LPTCH)malloc(32);
									memcpy(pTrimBuf[7], "version\\TCP", sizeof("version\\TCP"));
								}
								if (pTrimBuf[8] == NULL) {
									pTrimBuf[8] = (LPTCH)malloc(32);
									memcpy(pTrimBuf[8], "Protocol.dll\n", sizeof("Protocol.dll\n"));
							}
								nTrimSize = 8;
#endif
#ifdef DEBUGTEST4
								memcpy(pTrimBuf[6], "utils\\clcompile.exe\n", sizeof("utils\\clcompile.exe\n"));
#endif
								// check 4 types
								if (nTrimSize == 6) {
									// 1: with path and no space => nTrimSize is 6, needs to search '\\'
									_TCHAR* p = _tcsrchr(pTrimBuf[6], '\\');
									if (p) {
										// exclude path
										// 03-27-2003 11:07     274432 A___     128354   56 utils\clcompile.exe
										len = _tcslen(p + sizeof(_TCHAR));
										_tcsncpy(fname, p + sizeof(_TCHAR), len);
									}
									else {
										// 2: no path and no space => nTrimSize is 6
										len = _tcslen(pTrimBuf[6]);
										_tcsncpy(fname, pTrimBuf[6], len);
									}
								}
								else {
									// 3: with path and with space => nTrimSize is 7 or larger, needs to search '\\'
									INT idx = 6;
									for (INT j = 6; j <= nTrimSize; j++) {
										_TCHAR* p = _tcsrchr(pTrimBuf[j], '\\');
										if (p) {
											pTrimBuf[j] = p + sizeof(_TCHAR);
											idx++;
										}
									}

									// 4: no path and with space => nTrimSize is 7 or larger
									for (; idx <= nTrimSize; idx++) {
										// 08-16-2002 18:06      36957 A___      12034   41 TCP Protocol.dll
										_tcscat(fname, pTrimBuf[idx]);
										len += _tcslen(pTrimBuf[idx]);
										if (idx < nTrimSize) {
											_tcscat(fname, _T(" "));
											len++;
										}
									}
								}
								// Delete '\n'
								fname[len - sizeof(_TCHAR)] = '\0';
								_tcscat(szTmpFullPath, _T("\\"));
								_tcscat(szTmpFullPath, fname);

								if (!ReadExeFromFile(pExtArg, pDisc, szTmpFullPath, fname)) {
									continue;
								}
								DWORD attr = GetFileAttributes(szTmpFullPath);
								if (FILE_ATTRIBUTE_READONLY & attr) {
									SetFileAttributes(szTmpFullPath, attr - FILE_ATTRIBUTE_READONLY);
								}
								if (!DeleteFile(szTmpFullPath)) {
									OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
									OutputErrorString("Failed to DeleteFile %s\n", fname);
								}
								ZeroMemory(szTmpFullPath, sizeof(szTmpFullPath));
							}
							_fgetts(buf, sizeof(buf), fp);
							nTrimSize = 0;
						}
						FcloseAndNull(fp);
					}
					if (!DeleteFile(szTmpPath)) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						OutputErrorString("Failed to DeleteFile %s\n", szTmpPath);
					}
					bIscCab = TRUE;
					bCab = TRUE;
				}
#else
				// TODO: linux doesn't support yet
#endif
			}
		}

		if (!bCab) {
			if (IsImageSig(lpBuf, IMAGE_DOS_SIGNATURE)) {
				PIMAGE_DOS_HEADER pIDh = (PIMAGE_DOS_HEADER)&lpBuf[0];
				if (dwSize < (DWORD)pIDh->e_lfanew) {
					if (pDevice->dwMaxTransferLength < (DWORD)pIDh->e_lfanew) {
						OutputVolDescLog("%" CHARWIDTH "s: offset is very big (%ld). read skip [TODO]\n"
							, pDisc->PROTECT.pNameForExe[n], pIDh->e_lfanew);
					}
					else {
						SetCommandForTransferLength(pExecType, pDevice, pCdb, (DWORD)pIDh->e_lfanew, &byTransferLen, &byRoopLen);
						dwSize = DWORD(DISC_MAIN_DATA_SIZE) * byTransferLen;
						n--;
					}
					continue;
				}
				if (dwSize > DISC_MAIN_DATA_SIZE) {
					SetCommandForTransferLength(pExecType, pDevice, pCdb, DISC_MAIN_DATA_SIZE, &byTransferLen, &byRoopLen);
					dwSize = DISC_MAIN_DATA_SIZE;
				}
				OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA
					, pDisc->PROTECT.pExtentPosForExe[n], (UINT)pDisc->PROTECT.pExtentPosForExe[n], pDisc->PROTECT.pNameForExe[n]);
				OutputFsImageDosHeader(pIDh);

				if (IsImageSig(&lpBuf[pIDh->e_lfanew], IMAGE_NT_SIGNATURE)) {
					PIMAGE_NT_HEADERS32 pINH = (PIMAGE_NT_HEADERS32)&lpBuf[pIDh->e_lfanew];
					OutputFsImageNtHeader(pINH);

					DWORD dwExportVirtualAddress = pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
					DWORD dwExportSize = pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
					DWORD dwExportPointerToRawData = 0;
					DWORD dwExportDataOfs = 0;
					DWORD dwExportSectorSize = 0;
					DWORD dwImportVirtualAddress = pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
					DWORD dwImportSize = pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
					DWORD dwImportPointerToRawData = 0;
					DWORD dwImportDataOfs = 0;
					DWORD dwImportSectorSize = 0;
					DWORD dwResourceVirtualAddress = pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
					DWORD dwResourceSize = pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size;
					DWORD dwResourcePointerToRawData = 0;
					DWORD dwResourceDataOfs = 0;
					DWORD dwResourceSectorSize = 0;
					ULONG nOfs = pIDh->e_lfanew + sizeof(IMAGE_NT_HEADERS32);
					BOOL bSecurom = FALSE;
					BOOL bCompress = FALSE;

					for (INT i = 0; i < pINH->FileHeader.NumberOfSections; i++) {
						PIMAGE_SECTION_HEADER pISH = (PIMAGE_SECTION_HEADER)&lpBuf[nOfs];
						OutputFsImageSectionHeader(pExtArg, pDisc, pISH, &bSecurom);
						if (!strncmp((PCHAR)pISH->Name, ".petite", 7)) {
							bCompress = TRUE;
						}
						nOfs += sizeof(IMAGE_SECTION_HEADER);

						if (pISH->VirtualAddress <= dwExportVirtualAddress && dwExportVirtualAddress <= pISH->VirtualAddress + pISH->Misc.VirtualSize) {
							dwExportPointerToRawData = pISH->PointerToRawData + dwExportVirtualAddress - pISH->VirtualAddress;
							dwExportDataOfs = dwExportPointerToRawData % DISC_MAIN_DATA_SIZE;
							DWORD dwTmpSize = dwExportDataOfs + pISH->SizeOfRawData;
							dwExportSectorSize = DISC_MAIN_DATA_SIZE * (dwTmpSize / DISC_MAIN_DATA_SIZE + 1);
							if (dwExportSectorSize > pDevice->dwMaxTransferLength) {
								dwExportSectorSize = pDevice->dwMaxTransferLength;
							}
						}
						if (pISH->VirtualAddress <= dwImportVirtualAddress && dwImportVirtualAddress <= pISH->VirtualAddress + pISH->Misc.VirtualSize) {
							dwImportPointerToRawData = pISH->PointerToRawData + dwImportVirtualAddress - pISH->VirtualAddress;
							dwImportDataOfs = dwImportPointerToRawData % DISC_MAIN_DATA_SIZE;
							DWORD dwTmpSize = dwImportDataOfs + pISH->SizeOfRawData;
							dwImportSectorSize = DISC_MAIN_DATA_SIZE * (dwTmpSize / DISC_MAIN_DATA_SIZE + 1);
							if (dwImportSectorSize > pDevice->dwMaxTransferLength) {
								dwImportSectorSize = pDevice->dwMaxTransferLength;
							}
						}
						if (pISH->VirtualAddress <= dwResourceVirtualAddress && dwResourceVirtualAddress <= pISH->VirtualAddress + pISH->Misc.VirtualSize) {
							dwResourcePointerToRawData = pISH->PointerToRawData + dwResourceVirtualAddress - pISH->VirtualAddress;
							dwResourceDataOfs = dwResourcePointerToRawData % DISC_MAIN_DATA_SIZE;
							DWORD dwTmpSize = dwResourceDataOfs + pISH->SizeOfRawData;
							dwResourceSectorSize = DISC_MAIN_DATA_SIZE * (dwTmpSize / DISC_MAIN_DATA_SIZE + 1);
							if (dwResourceSectorSize > pDevice->dwMaxTransferLength) {
								dwResourceSectorSize = pDevice->dwMaxTransferLength;
							}
						}
					}

					if (bCompress) {
						OutputVolDescLog("Skipped reading Export, Import, Resource data due to the compressed data\n");
					}
					else {
						if (dwExportSize > 0) {
							INT nExpSection = pDisc->PROTECT.pExtentPosForExe[n] + (INT)dwExportPointerToRawData / DISC_MAIN_DATA_SIZE;
							SetCommandForTransferLength(pExecType, pDevice, pCdb, dwExportSectorSize, &byTransferLen, &byRoopLen);
							if (!ExecReadCD(pExtArg, pDevice, pCdb, nExpSection, lpBuf, dwExportSectorSize, _T(__FUNCTION__), __LINE__)) {
								continue;
							}
							OutputMainInfoLog("dwExportVirtualAddress: 0x%lx, dwExportSize: 0x%lx, dwExportDataOfs: 0x%lx\n"
								, dwExportVirtualAddress, dwExportSize, dwExportDataOfs);
							OutputMainChannel(fileMainInfo, lpBuf, NULL, nExpSection, dwExportSectorSize);
							OutputExportDirectory(lpBuf, dwExportSectorSize, dwExportVirtualAddress, dwExportDataOfs);
						}
						if (dwImportSize > 0) {
							INT nImpSection = pDisc->PROTECT.pExtentPosForExe[n] + (INT)dwImportPointerToRawData / DISC_MAIN_DATA_SIZE;
							SetCommandForTransferLength(pExecType, pDevice, pCdb, dwImportSectorSize, &byTransferLen, &byRoopLen);
							if (!ExecReadCD(pExtArg, pDevice, pCdb, nImpSection, lpBuf, dwImportSectorSize, _T(__FUNCTION__), __LINE__)) {
								continue;
							}
							OutputMainInfoLog("dwImportVirtualAddress: 0x%lx, dwImportSize: 0x%lx, dwImportDataOfs: 0x%lx\n"
								, dwImportVirtualAddress, dwImportSize, dwImportDataOfs);
							OutputMainChannel(fileMainInfo, lpBuf, NULL, nImpSection, dwImportSectorSize);
							OutputImportDirectory(lpBuf, dwImportSectorSize, dwImportVirtualAddress, dwImportDataOfs);
						}
						if (dwResourceSize > 0) {
							INT nResSection = pDisc->PROTECT.pExtentPosForExe[n] + (INT)dwResourcePointerToRawData / DISC_MAIN_DATA_SIZE;
							SetCommandForTransferLength(pExecType, pDevice, pCdb, dwResourceSectorSize, &byTransferLen, &byRoopLen);
							if (!ExecReadCD(pExtArg, pDevice, pCdb, nResSection, lpBuf, dwResourceSectorSize, _T(__FUNCTION__), __LINE__)) {
								continue;
							}
							OutputMainInfoLog("dwResourceVirtualAddress: 0x%lx, dwResourceSize: 0x%lx, dwResourceDataOfs: 0x%lx\n"
								, dwResourceVirtualAddress, dwResourceSize, dwResourceDataOfs);
							OutputMainChannel(fileMainInfo, lpBuf, NULL, nResSection, dwResourceSectorSize);
							_TCHAR szTab[256] = {};
							szTab[0] = _T('\t');
							WCHAR wszFileVer[FILE_VERSION_SIZE] = { 0 };
							OutputResourceDirectory(lpBuf, dwResourceSectorSize, dwResourceVirtualAddress, dwResourceDataOfs, 0, wszFileVer, szTab);
							if (wszFileVer[0] != 0 && strcasestr(pDisc->PROTECT.pNameForExe[n], ".EXE")) {
								OutputLog(standardOut | fileDisc, " %s: File Version %ls\n", pDisc->PROTECT.pNameForExe[n], wszFileVer);
							}
						}
					}
					dwSize = DISC_MAIN_DATA_SIZE;
					SetCommandForTransferLength(pExecType, pDevice, pCdb, dwSize, &byTransferLen, &byRoopLen);

					INT nLastSector = pDisc->PROTECT.pExtentPosForExe[n] + pDisc->PROTECT.pSectorSizeForExe[n] - 1;
					if (bSecurom) {
						if (!ExecReadCD(pExtArg, pDevice, pCdb, nLastSector, lpBuf, dwSize, _T(__FUNCTION__), __LINE__)) {
							continue;
						}
						OutputMainChannel(fileMainInfo, lpBuf, NULL, nLastSector, DISC_MAIN_DATA_SIZE);
						INT nMod = pDisc->PROTECT.pDataLenForExe[n] % DISC_MAIN_DATA_SIZE;
						UINT uiOfsOfSecuRomDll = 0;
						if (nMod) {
							uiOfsOfSecuRomDll = MAKEUINT(MAKEWORD(lpBuf[nMod - 4], lpBuf[nMod - 3]), MAKEWORD(lpBuf[nMod - 2], lpBuf[nMod - 1]));
						}
						else {
							uiOfsOfSecuRomDll = MAKEUINT(MAKEWORD(lpBuf[DISC_MAIN_DATA_SIZE - 4], lpBuf[DISC_MAIN_DATA_SIZE - 3])
								, MAKEWORD(lpBuf[DISC_MAIN_DATA_SIZE - 2], lpBuf[DISC_MAIN_DATA_SIZE - 1]));
						}
						if (uiOfsOfSecuRomDll && uiOfsOfSecuRomDll < (UINT)pDisc->PROTECT.pDataLenForExe[n]) {
							UINT uiPosOfSecuRomDll = uiOfsOfSecuRomDll / DISC_MAIN_DATA_SIZE;

							INT n1stSector = (INT)(pDisc->PROTECT.pExtentPosForExe[n] + uiPosOfSecuRomDll);
							if (!ExecReadCD(pExtArg, pDevice, pCdb, n1stSector, lpBuf, dwSize, _T(__FUNCTION__), __LINE__)) {
								continue;
							}
							OutputMainChannel(fileMainInfo, lpBuf, NULL, n1stSector, DISC_MAIN_DATA_SIZE);

							UINT uiOfsOf16 = 0;
							UINT uiOfsOf32 = 0;
							UINT uiOfsOfNT = 0;
							UINT uiSizeOf16 = 0;
							UINT uiSizeOf32 = 0;
							UINT uiSizeOfNT = 0;
							if (!strncmp((LPCCH)&lpBuf[0], "AddD", 4)) {
								OutputSecuRomDllHeader(lpBuf, &uiOfsOf16, &uiOfsOf32, &uiOfsOfNT, &uiSizeOf16, &uiSizeOf32, &uiSizeOfNT);

								INT nOfsOf16dll = (INT)(uiOfsOf16 - uiOfsOfSecuRomDll) % DISC_MAIN_DATA_SIZE;
								OutputSint16(lpBuf, nOfsOf16dll);

								dwSize = uiSizeOf32 + (DISC_MAIN_DATA_SIZE - uiSizeOf32 % DISC_MAIN_DATA_SIZE);
								SetCommandForTransferLength(pExecType, pDevice, pCdb, dwSize, &byTransferLen, &byRoopLen);
								UINT tmp = uiOfsOf32 / DISC_MAIN_DATA_SIZE;
								INT n2ndSector = (INT)(pDisc->PROTECT.pExtentPosForExe[n] + tmp);
								if (!ExecReadCD(pExtArg, pDevice, pCdb, n2ndSector, lpBuf, dwSize, _T(__FUNCTION__), __LINE__)) {
									continue;
								}

								OutputMainInfoLog(OUTPUT_DHYPHEN_PLUS_STR("Sintf32.dll [F:%s]"), __FUNCTION__);
								OutputMainChannel(fileMainInfo, lpBuf, NULL, n2ndSector, dwSize);
								INT nOfsOf32dll = (INT)(uiOfsOf32 - uiOfsOfSecuRomDll) % DISC_MAIN_DATA_SIZE;
								OutputSint32(lpBuf, nOfsOf32dll, dwSize, FALSE);

								dwSize = uiSizeOfNT + (DISC_MAIN_DATA_SIZE - uiSizeOfNT % DISC_MAIN_DATA_SIZE);
								SetCommandForTransferLength(pExecType, pDevice, pCdb, dwSize, &byTransferLen, &byRoopLen);
								UINT tmp2 = uiOfsOfNT / DISC_MAIN_DATA_SIZE;
								INT n3rdSector = (INT)(pDisc->PROTECT.pExtentPosForExe[n] + tmp2);
								if (!ExecReadCD(pExtArg, pDevice, pCdb, n3rdSector, lpBuf, dwSize, _T(__FUNCTION__), __LINE__)) {
									continue;
								}

								OutputMainInfoLog(OUTPUT_DHYPHEN_PLUS_STR("SintfNT.dll [F:%s]"), __FUNCTION__);
								OutputMainChannel(fileMainInfo, lpBuf, NULL, n3rdSector, dwSize);
								INT nOfsOfNTdll = (INT)(uiOfsOfNT - uiOfsOfSecuRomDll) % DISC_MAIN_DATA_SIZE;
								OutputSintNT(lpBuf, nOfsOfNTdll, dwSize, FALSE);

								dwSize = DISC_MAIN_DATA_SIZE;
								SetCommandForTransferLength(pExecType, pDevice, pCdb, DISC_MAIN_DATA_SIZE, &byTransferLen, &byRoopLen);
							}
						}
						else if (pExtArg->byIntentionalSub) {
							BOOL bFound = FALSE;
							for (INT j = 0; j < pDisc->PROTECT.pSectorSizeForExe[n]; j++) {
								if (!ExecReadCD(pExtArg, pDevice, pCdb
									, pDisc->PROTECT.pExtentPosForExe[n] + j, lpBuf, dwSize, _T(__FUNCTION__), __LINE__)) {
									continue;
								}
								for (UINT i = 0; i < dwSize - 8; i++) {
									if (IsSecuromDllSig(lpBuf, i)) {
										LONG lSigPos = (LONG)(dwSize * j + i);
										LONG lSigOfs = GetOfsOfSecuromDllSig(lpBuf, i);
										if (lSigPos == lSigOfs) {
											OutputSecuRomDll4_87Header(lpBuf, i);
											OutputMainChannel(fileMainInfo, lpBuf, NULL, pDisc->PROTECT.pExtentPosForExe[n] + j, dwSize);
											bFound = TRUE;
											break;
										}
									}
								}
								if (bFound) {
									break;
								}
							}
						}
					}
					else if (strcasestr(pDisc->PROTECT.pNameForExe[n], "SETUP.EXE")) {
#ifdef _WIN32
						for (INT i = pDisc->PROTECT.pExtentPosForExe[n] + 1; i < nLastSector; i++) {
							if (!ExecReadCD(pExtArg, pDevice, pCdb, i, lpBuf, dwSize, _T(__FUNCTION__), __LINE__)) {
								continue;
							}
							
							BOOL bFound = FALSE;
							for (DWORD j = 0; j < dwSize - 8; j++) {
								if (!strncmp((LPCCH)&lpBuf[j], "WiseMain", 8)) {
									OutputString(
										"\nDetected Wise Installation: %" CHARWIDTH "s\n"
										"Please wait until all files are extracted. This is needed to search protection\n"
										, pDisc->PROTECT.pFullNameForExe[n]
									);
									OutputMainChannel(fileMainInfo, lpBuf, NULL, i, dwSize);

									_TCHAR szTmpPath[_MAX_PATH] = {};
									if (!GetCurrentDirectory(sizeof(szTmpPath) / sizeof(szTmpPath[0]), szTmpPath)) {
										OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
										return FALSE;
									}
									_tcscat(szTmpPath, _T("\\!extracted\\"));
									GetFullPathWithDrive(pDevice, pDisc, n, FullPathWithDrive, sizeof(FullPathWithDrive));
									ProcessDirectory(pExtArg, pDisc, szTmpPath, FILE_CREATE);

									_TCHAR szPathWise[_MAX_PATH] = {};
									bRet = GetCmd(szPathWise, _T("E_WISE_W"), _T("EXE"));

									if (bRet && PathFileExists(szPathWise)) {
										CONST INT nStrSize = _MAX_PATH * 2;
										_TCHAR str[nStrSize] = {};
										_sntprintf(str, nStrSize,
											_T("\"\"%s\" \"%s\" \"%s\"\" > NUL")
											, szPathWise, FullPathWithDrive, szTmpPath);
										_tsystem(str);
										ProcessDirectory(pExtArg, pDisc, szTmpPath, FILE_SEARCH);
										ProcessDirectory(pExtArg, pDisc, szTmpPath, FILE_DELETE);
										bFound = TRUE;
										break;
									}
								}
							}
							if (bFound) {
								break;
							}
						}
#else
					// TODO: linux doesn't support yet
#endif
					}
				}
				else if (IsImageSig(&lpBuf[pIDh->e_lfanew], IMAGE_OS2_SIGNATURE)) {
					OutputFsImageOS2Header((PIMAGE_OS2_HEADER)&lpBuf[pIDh->e_lfanew]);
				}
				else if (IsImageSig(&lpBuf[pIDh->e_lfanew], IMAGE_OS2_SIGNATURE_LE)) {
					// TODO
				}
				else {
					OutputVolDescLog(
						"%" CHARWIDTH "s: ImageNT,NE,LEHeader doesn't exist\n", pDisc->PROTECT.pNameForExe[n]);
				}
			}
			else {
				OutputVolDescLog(
					"LBA: %d, %" CHARWIDTH "s: ImageDosHeader doesn't exist\n"
					, pDisc->PROTECT.pExtentPosForExe[n], pDisc->PROTECT.pNameForExe[n]);
			}
		}
		OutputString("\rChecking EXE %4d", n + 1);
	}
	OutputString("\n");
	return bRet;
}

BOOL ReadCDForSegaDisc(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	BYTE buf[DISC_MAIN_DATA_SIZE] = {};
	CDB::_READ12 cdb = {};
	cdb.OperationCode = SCSIOP_READ12;
	cdb.TransferLength[3] = 1;

	if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, 0, buf,
		DISC_MAIN_DATA_SIZE, _T(__FUNCTION__), __LINE__)) {
	}
	if (!memcmp(buf, "SEGA", 4)) {
		OutputMainChannel(fileMainInfo, buf, "Sega Header", 0, DISC_MAIN_DATA_SIZE);
	}
	return TRUE;
}

BOOL ReadCDForCheckingPsxRegion(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	BYTE buf[DISC_MAIN_DATA_SIZE] = {};
	CONST CHAR regionPal[] =
		"          Licensed  by          Sony Computer Entertainment Euro pe   ";
	CONST CHAR regionPal2[] =
		"          Licensed  by          Sony Computer Entertainment(Europe)";
	CDB::_READ12 cdb = {};
	cdb.OperationCode = SCSIOP_READ12;
	cdb.TransferLength[3] = 1;

	if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, 4, buf,
		DISC_MAIN_DATA_SIZE, _T(__FUNCTION__), __LINE__)) {
		return TRUE;
	}
	OutputMainChannel(fileMainInfo, buf, "Check PSX Region", 4, 80);
	if (!memcmp(buf, regionPal, sizeof(regionPal))) {
		return TRUE;
	}
	else if (!memcmp(buf, regionPal2, sizeof(regionPal2))) {
		return TRUE;
	}
	OutputString("[INFO] This disc isn't PSX PAL. /nl is ignored.\n");
	return FALSE;
}

VOID ReadCDForScanningPsxAntiMod(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	BOOL bRet = FALSE;
	BYTE buf[DISC_MAIN_DATA_SIZE * 2] = {};
	CONST CHAR antiModStrEn[] =
		"     SOFTWARE TERMINATED\nCONSOLE MAY HAVE BEEN MODIFIED\n     CALL 1-888-780-7690";
	CONST CHAR antiModStrJp[] =
		"強制終了しました。\n本体が改造されている\nおそれがあります。";
	CDB::_READ12 cdb = {};
	cdb.OperationCode = SCSIOP_READ12;
	cdb.TransferLength[3] = 2;

	for (INT nLBA = 18; nLBA < pDisc->SCSI.nLastLBAofDataTrkOnToc - 150; nLBA++) {
		if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, buf,
			DISC_MAIN_DATA_SIZE * 2, _T(__FUNCTION__), __LINE__)) {
			return;
		}
		for (INT i = 0; i < DISC_MAIN_DATA_SIZE; i++) {
			if (!memcmp(&buf[i], antiModStrEn, sizeof(antiModStrEn))) {
				OutputLog(fileDisc | standardOut
					, "\nDetected anti-mod string (en): LBA %d, offset %d(0x%03x)", nLBA, i, (UINT)i);
				OutputMainChannel(fileMainInfo, buf, NULL, nLBA, DISC_MAIN_DATA_SIZE);
				bRet += TRUE;
			}
			if (!memcmp(&buf[i], antiModStrJp, sizeof(antiModStrJp))) {
				OutputLog(fileDisc | standardOut
					, "\nDetected anti-mod string (jp): LBA %d, offset %d(0x%03x)\n", nLBA, i, (UINT)i);
				if (!bRet) {
					OutputMainChannel(fileMainInfo, buf, NULL, nLBA, DISC_MAIN_DATA_SIZE);
				}
				bRet += TRUE;
			}
			if (bRet == 2) {
				break;
			}
		}
		if (bRet == 2) {
			break;
		}
		OutputString("\rScanning sector for anti-mod string (LBA) %6d/%6d", nLBA, pDisc->SCSI.nLastLBAofDataTrkOnToc - 150 - 1);
	}
	if (!bRet) {
		OutputLog(fileDisc | standardOut, "\nNo anti-mod string\n");
	}
	return;
}

BOOL ReadCDForScanningProtectViaSector(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
	DWORD dwBufLen = CD_RAW_SECTOR_SIZE + CD_RAW_READ_SUBCODE_SIZE;
	CDFLAG::_READ_CD::_ERROR_FLAGS c2 = CDFLAG::_READ_CD::NoC2;
	CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION flg = CDFLAG::_READ_CD::Pack;
	if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		dwBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
		c2 = CDFLAG::_READ_CD::byte294;
		flg = CDFLAG::_READ_CD::Raw;
	}
	SetReadDiscCommand(NULL, pExtArg, pDevice, 1, c2, flg, lpCmd, FALSE);

	BYTE aBuf[CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE] = {};
	BYTE byScsiStatus = 0;
	for (INT nLBA = 0; nLBA < pDisc->SCSI.nAllLength; nLBA++) {
		if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, aBuf,
			dwBufLen, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
		INT nOfs = 0;
		if (pDisc->MAIN.nCombinedOffset > 0) {
			nOfs = pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE;
		}
		else if (pDisc->MAIN.nCombinedOffset < 0) {
			nOfs = CD_RAW_SECTOR_SIZE + pDisc->MAIN.nCombinedOffset;
		}
		if (aBuf[nOfs] == 0 && aBuf[nOfs + 1] == 0xff && aBuf[nOfs + 2] == 0 && aBuf[nOfs + 3] == 0xff &&
			aBuf[nOfs + 4] == 0 && aBuf[nOfs + 5] == 0xff && aBuf[nOfs + 6] == 0 && aBuf[nOfs + 7] == 0xff &&
			aBuf[nOfs + 8] == 0 && aBuf[nOfs + 9] == 0xff && aBuf[nOfs + 10] == 0 && aBuf[nOfs + 11] == 0xff) {
			OutputLog(standardOut | fileDisc, "\nDetected ProtectCD VOB. It begins from %d sector", nLBA);
			pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = nLBA;
			pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] = pDisc->SCSI.nAllLength - nLBA - 1;
			pDisc->PROTECT.byExist = protectCDVOB;
			pExtArg->byScanProtectViaFile = pExtArg->byScanProtectViaSector;
			break;
		}
		OutputString("\rScanning sector (LBA) %6d/%6d", nLBA, pDisc->SCSI.nAllLength - 1);
	}
	OutputLog(standardOut | fileDisc, "\n");

	return TRUE;
}

BOOL ReadCDForCheckingSecuROM(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	LPBYTE lpCmd
) {
#ifdef _DEBUG
	WORD w = (WORD)GetCrc16CCITT(10, &pDiscPerSector->subcode.current[12]);
	OutputSubInfoWithLBALog(
		"CRC-16 is original:[%02x%02x], recalc:[%04x] and XORed with 0x8001:[%02x%02x]\n"
		, -1, 0, pDiscPerSector->subcode.current[22], pDiscPerSector->subcode.current[23]
		, w, pDiscPerSector->subcode.current[22] ^ 0x80, pDiscPerSector->subcode.current[23] ^ 0x01);
#endif
	if (pExtArg->byIntentionalSub && pDisc->PROTECT.byExist != securomV1 &&
		(pDiscPerSector->subcode.current[12] == 0x41 || pDiscPerSector->subcode.current[12] == 0x61)) {

		INT nRLBA = MSFtoLBA(BcdToDec(pDiscPerSector->subcode.current[15])
			, BcdToDec(pDiscPerSector->subcode.current[16]), BcdToDec(pDiscPerSector->subcode.current[17]));
		INT nALBA = MSFtoLBA(BcdToDec(pDiscPerSector->subcode.current[19])
			, BcdToDec(pDiscPerSector->subcode.current[20]), BcdToDec(pDiscPerSector->subcode.current[21]));

		// All SecuROM titles http://redump.org/discs/quicksearch/securom/protection/only
		if ((nRLBA == 3000 || nRLBA == 3001) && nALBA == 299) { // 3001(00:40:01), 299(00:03:74)
			// Aliens Versus Predator 2 (Disc 1) http://redump.org/disc/35861/
			// Unreal Tournament 2003 (Disc 1) (Play Disc) http://redump.org/disc/21469/
			// Unreal Tournament 2004 (Play Disc) http://redump.org/disc/61749/
			OutputSubInfoWithLBALog(
				"Detected intentional error. CRC-16 is original:[%02x%02x] and XORed with 0x8001:[%02x%02x] "
				, -1, 0, pDiscPerSector->subcode.current[22] ^ 0x80, pDiscPerSector->subcode.current[23] ^ 0x01
				, pDiscPerSector->subcode.current[22], pDiscPerSector->subcode.current[23]);
			OutputSubInfoLog(
				"RMSF[%02x:%02x:%02x] AMSF[%02x:%02x:%02x]\n"
				, pDiscPerSector->subcode.current[15], pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
				, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);

			OutputLog(standardOut | fileDisc, "Detected intentional subchannel in LBA -1 => SecuROM 4th version (a.k.a. NEW)\n");
			OutputIntentionalSubchannel(-1, &pDiscPerSector->subcode.current[12]);
			pDisc->PROTECT.byExist = securomV4;
			pDiscPerSector->subch.prev.nRelativeTime = -1;
			pDiscPerSector->subch.prev.nAbsoluteTime = 149;
		}
		else if ((nRLBA == 167295 || nRLBA == 0 || pDiscPerSector->subcode.current[17] == 0xff) && nALBA == 150) { // 167295(37:10:45), 150(00:02:00)
			OutputSubInfoWithLBALog(
				"Detected shifted sub. RMSF[%02x:%02x:%02x] AMSF[%02x:%02x:%02x]\n"
				, -1, 0, pDiscPerSector->subcode.current[15], pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
				, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);

			if (nRLBA == 167295) {
				// Colin McRae Rally 2.0 (Europe) http://redump.org/disc/31587/
				OutputLog(standardOut | fileDisc, "Detected intentional subchannel in LBA -1 => SecuROM 3rd version type 1 (a.k.a. NEW)\n");
				pDisc->PROTECT.byExist = securomV3_1;
			}
			else if (nRLBA == 0) {
				// Empire Earth (USA) http://redump.org/disc/45559/
				// Diablo II: Lord of Destruction (Expansion Set) (USA) http://redump.org/disc/58232/
				OutputLog(standardOut | fileDisc, "Detected intentional subchannel in LBA -1 => SecuROM 3rd version type 2 (a.k.a. NEW)\n");
				pDisc->PROTECT.byExist = securomV3_2;
			}
			else if (pDiscPerSector->subcode.current[17] == 0xff) {
				OutputLog(standardOut | fileDisc, "Detected intentional subchannel in LBA -1 => SecuROM 3rd version type 3 (a.k.a. NEW)\n");
				pDisc->PROTECT.byExist = securomV3_3;
			}
			OutputIntentionalSubchannel(-1, &pDiscPerSector->subcode.current[12]);
			if (pDisc->SUB.nSubChannelOffset) {
				pDisc->SUB.nSubChannelOffset -= 1;
			}
			pDiscPerSector->subch.prev.nRelativeTime = -1;
			pDiscPerSector->subch.prev.nAbsoluteTime = 149;
		}
		else if (pDisc->SCSI.nAllLength > 5000) {
			BYTE byTransferLen = 2;
			if (lpCmd[0] == 0xd8) {
				byTransferLen = lpCmd[9];
				lpCmd[9] = 1;
			}
			else {
				byTransferLen = lpCmd[8];
				lpCmd[8] = 1;
			}

			if (!ExecReadCD(pExtArg, pDevice, lpCmd, 5000, pDiscPerSector->data.current,
				pDevice->TRANSFER.uiBufLen, _T(__FUNCTION__), __LINE__)) {
				return FALSE;
			}
			AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
			nRLBA = MSFtoLBA(BcdToDec(pDiscPerSector->subcode.current[15])
				, BcdToDec(pDiscPerSector->subcode.current[16]), BcdToDec(pDiscPerSector->subcode.current[17]));
			nALBA = MSFtoLBA(BcdToDec(pDiscPerSector->subcode.current[19])
				, BcdToDec(pDiscPerSector->subcode.current[20]), BcdToDec(pDiscPerSector->subcode.current[21]));

			if (nRLBA == 5001 && nALBA == 5151) { // 5001(01:06:51), 5151(01:08:51)
				// Supreme Snowboarding (Europe) http://redump.org/disc/32182/
				OutputLog(standardOut | fileDisc, "Detected intentional subchannel in LBA 5000 => SecuROM 2nd version (a.k.a. NEW)\n");
				pDisc->PROTECT.byExist = securomV2;
			}
			else if (pDisc->PROTECT.byExist == securomTmp) {
				for (INT nTmpLBA = 40000; nTmpLBA < 45800; nTmpLBA++) {
					if (pDisc->SCSI.nAllLength > nTmpLBA) {
						if (!ExecReadCD(pExtArg, pDevice, lpCmd, nTmpLBA, pDiscPerSector->data.current,
							pDevice->TRANSFER.uiBufLen, _T(__FUNCTION__), __LINE__)) {
							return FALSE;
						}
						AlignRowSubcode(pDiscPerSector->subcode.current, pDiscPerSector->data.current + pDevice->TRANSFER.uiBufSubOffset);
						WORD reCalcCrc16 = (WORD)GetCrc16CCITT(10, &pDiscPerSector->subcode.current[12]);
						WORD reCalcXorCrc16 = (WORD)(reCalcCrc16 ^ 0x0080);
						if (pDiscPerSector->subcode.current[22] == HIBYTE(reCalcXorCrc16) &&
							pDiscPerSector->subcode.current[23] == LOBYTE(reCalcXorCrc16)) {
							// FIFA 99 (Europe) http://redump.org/disc/23791/
							OutputLog(standardOut | fileDisc
								, "Detected intentional subchannel in LBA %d => SecuROM 1st version (a.k.a. OLD)\n", nTmpLBA);
							pDisc->PROTECT.byExist = securomV1;
							break;
						}
					}
				}
				if (pDisc->PROTECT.byExist != securomV1) {
					OutputLog(standardOut | fileDisc, "[INFO] SecuROM sector not found \n");
				}
			}
			if (lpCmd[0] == 0xd8) {
				lpCmd[9] = byTransferLen;
			}
			else {
				lpCmd[8] = byTransferLen;
			}
		}
		else {
			OutputLog(standardOut | fileDisc, "[INFO] SecuROM sector not found \n");
		}
	}
	return TRUE;
}

BOOL ExecCheckingByteOrder(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	CDFLAG::_READ_CD::_ERROR_FLAGS c2,
	CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION sub
) {
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
//	pExtArg->byBe = TRUE;
	SetReadDiscCommand(NULL, pExtArg, pDevice, 1, c2, sub, lpCmd, FALSE);
//	pExtArg->byBe = FALSE;

	BOOL bRet = TRUE;
	if (!ExecReadCD(pExtArg, pDevice, lpCmd, 0, lpBuf
		, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
		OutputLog(standardError | fileDrive,
			"This drive doesn't support [OpCode: %#02x, C2flag: %d, SubCode: %x]\n"
			, lpCmd[0], (lpCmd[9] & 0x6) >> 1, lpCmd[10]);
		bRet = FALSE;
	}
	else {
		OutputDriveLog(OUTPUT_DHYPHEN_PLUS_STR("Check main + c2 + sub"));
		OutputCDC2Error296(fileDrive, lpBuf + CD_RAW_SECTOR_SIZE, 0);
		OutputCDSub96Raw(fileDrive, lpBuf + CD_RAW_SECTOR_WITH_C2_294_SIZE, 0);

		OutputDriveLog(OUTPUT_DHYPHEN_PLUS_STR("Check main + sub + c2"));
		OutputCDSub96Raw(fileDrive, lpBuf + CD_RAW_SECTOR_SIZE, 0);
		OutputCDC2Error296(fileDrive, lpBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE, 0);

		BYTE subcode[CD_RAW_READ_SUBCODE_SIZE] = {};
		memcpy(subcode, lpBuf + CD_RAW_SECTOR_WITH_C2_294_SIZE, CD_RAW_READ_SUBCODE_SIZE);
		// check main + c2 + sub order
		BOOL bMainSubC2 = TRUE;
		for (INT i = 0; i < CD_RAW_READ_SUBCODE_SIZE; i++) {
			if (subcode[i]) {
				bMainSubC2 = FALSE;
				break;
			}
		}
		if (bMainSubC2) {
			pDevice->driveOrder = DRIVE_DATA_ORDER::MainSubC2;
		}
	}
	FreeAndNull(pBuf);
	return bRet;
}

VOID ReadCDForCheckingByteOrder(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	CDFLAG::_READ_CD::_ERROR_FLAGS* c2
) {
	SetBufferSizeForReadCD(pExecType, pDevice, DRIVE_DATA_ORDER::NoC2);
	if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		*c2 = CDFLAG::_READ_CD::byte294;
		SetBufferSizeForReadCD(pExecType, pDevice, DRIVE_DATA_ORDER::MainC2Sub);
		pDevice->driveOrder = DRIVE_DATA_ORDER::MainC2Sub;
		CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION sub = CDFLAG::_READ_CD::Raw;

		if (!ExecCheckingByteOrder(pExtArg, pDevice, *c2, sub)) {
			BOOL bRet = FALSE;
			if (!pExtArg->byD8 && !pDevice->byPlxtrDrive) {
				bRet = TRUE;
				sub = CDFLAG::_READ_CD::Q;
				if (!ExecCheckingByteOrder(pExtArg, pDevice, *c2, sub)) {
					// not return FALSE
				}
				sub = CDFLAG::_READ_CD::Pack;
				if (!ExecCheckingByteOrder(pExtArg, pDevice, *c2, sub)) {
					// not return FALSE
				}
				*c2 = CDFLAG::_READ_CD::byte296;
				sub = CDFLAG::_READ_CD::Raw;
				if (!ExecCheckingByteOrder(pExtArg, pDevice, *c2, sub)) {
					bRet = FALSE;
				}
				sub = CDFLAG::_READ_CD::Q;
				if (!ExecCheckingByteOrder(pExtArg, pDevice, *c2, sub)) {
					// not return FALSE
				}
				sub = CDFLAG::_READ_CD::Pack;
				if (!ExecCheckingByteOrder(pExtArg, pDevice, *c2, sub)) {
					// not return FALSE
				}
			}
			if (!bRet) {
				OutputLog(standardError | fileDrive,
					"[WARNING] This drive doesn't support reporting C2 error. Disabled /c2\n");
				*c2 = CDFLAG::_READ_CD::NoC2;
				pDevice->driveOrder = DRIVE_DATA_ORDER::NoC2;
				pDevice->FEATURE.byC2ErrorData = FALSE;
				SetBufferSizeForReadCD(pExecType, pDevice, DRIVE_DATA_ORDER::NoC2);
			}
		}
		if (pDevice->driveOrder == DRIVE_DATA_ORDER::MainSubC2) {
			OutputDriveLog(
				"\tByte order of this drive is main + sub + c2\n");
			SetBufferSizeForReadCD(pExecType, pDevice, DRIVE_DATA_ORDER::MainSubC2);
		}
		else if (pDevice->driveOrder == DRIVE_DATA_ORDER::MainC2Sub) {
			OutputDriveLog(
				"\tByte order of this drive is main + c2 + sub\n");
		}
	}
#ifdef _DEBUG
	OutputString(
		"BufLen %ubyte, BufC2Offset %ubyte, BufSubOffset %ubyte\n",
		pDevice->TRANSFER.uiBufLen, pDevice->TRANSFER.uiBufC2Offset, pDevice->TRANSFER.uiBufSubOffset);
#endif
}

BOOL ReadCDCheck(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	// needs to call ReadTOCFull
	if (!pDisc->SCSI.bMultiSession && pExtArg->byMultiSession) {
		OutputLog(standardOut | fileDisc,
			"[INFO] This disc isn't Multi-Session. /ms is ignored.\n");
		pExtArg->byMultiSession = FALSE;
	}
	else if (pDisc->SCSI.bMultiSession && !pExtArg->byMultiSession) {
		OutputLog(standardOut | fileDisc,
			"[INFO] This disc is Multi-Session. /ms is set.\n");
		pExtArg->byMultiSession = TRUE;
	}

	if (!pExtArg->byReverse) {
		if (pExtArg->byMultiSession && pDevice->byPlxtrDrive && !IsPrextorDVDDrive(pDevice)) {
			OutputLog(standardOut | fileDisc,
				"[ERROR] This program doesn't support to dump the multi-session disc by the plextor CD Drive\n");
			return FALSE;
		}
		// Typically, CD+G data is included in audio only disc
		// But exceptionally, WonderMega Collection (SCD)(mixed disc) exists CD+G data.
		if (!ReadCDForCheckingSubRtoW(pExecType, pExtArg, pDevice, pDisc)) {
			return FALSE;
		}
		if (pDisc->SCSI.trkType != TRACK_TYPE::audioOnly) {
			if (*pExecType == gd) {
				if (!ReadGDForFileSystem(pExecType, pExtArg, pDevice, pDisc)) {
					return FALSE;
				}
			}
			else {
				if (pDisc->SCSI.by1stDataTrkNum == 1) {
					ReadCDForSegaDisc(pExtArg, pDevice);
					if (IsPregapOfTrack1ReadableDrive(pDevice)) {
						ReadCDForCheckingPregapSync(pExecType, pExtArg, pDevice);
					}
				}
				if (pExtArg->byLibCrypt) {
					// PSX PAL only
					if (!ReadCDForCheckingPsxRegion(pExtArg, pDevice)) {
						pExtArg->byLibCrypt = FALSE;
					}
				}
				if (pExtArg->byScanAntiModStr) {
					// PSX only
					ReadCDForScanningPsxAntiMod(pExtArg, pDevice, pDisc);
				}
				if (pExtArg->byScanProtectViaSector) {
					// Now ProtectCD VOB can be detected
					if (!ReadCDForScanningProtectViaSector(pExtArg, pDevice, pDisc)) {
						return FALSE;
					}
				}
				if (!ReadCDForFileSystem(pExecType, pExtArg, pDevice, pDisc)) {
					return FALSE;
				}
				if ((pExtArg->byScanProtectViaFile || pExtArg->byScanProtectViaSector) &&
					pDisc->PROTECT.byExist == PROTECT_TYPE_CD::no) {
					OutputString(
						"[INFO] Protection can't be detected. /sf, /ss is ignored.\n");
					if (pExtArg->byScanProtectViaFile && !pExtArg->byIntentionalSub) {
						TerminateProtectData(&pDisc);
					}
					pExtArg->byScanProtectViaFile = FALSE;
					pExtArg->byScanProtectViaSector = FALSE;
				}
			}
		}
		else {
			if (pExtArg->byVerifyAudioCDOfs && (pExtArg->byAtari || pExtArg->byVideoNow || pExtArg->byVideoNowColor || pExtArg->byVideoNowXp)) {
				OutputString("[INFO] /aj, /vn, /vnc and /vnx ignore /vrfy\n");
				pExtArg->byVerifyAudioCDOfs = FALSE;
			}
			else {
				if (/*!pExtArg->byVerifyAudioCDOfs ||*/ (pExtArg->byVerifyAudioCDOfs && pExtArg->uiVerifyAudio != READ_AUDIO_DISC_WITHOUT_OFFSET)) {
					ReadAudioCDForCheckingReadInOut(pExecType, pExtArg, pDevice, pDisc);
				}
			}
		}
	}
	return TRUE;
}

BOOL ReadGDForCheckingSubQAdr(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector
) {
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
	INT nOfs = 0;
	BYTE byMode = DATA_BLOCK_MODE0;
	UINT uiBufLen = CD_RAW_SECTOR_SIZE + CD_RAW_READ_SUBCODE_SIZE;

	if (!ReadCDForCheckingSubQAdrFirst(pExecType, pExtArg
		, pDevice, pDisc, &pBuf, &lpBuf, lpCmd, &uiBufLen, &nOfs)) {
		return FALSE;
	}
	for (BYTE i = (BYTE)(pDisc->SCSI.toc.FirstTrack - 1); i < pDisc->SCSI.toc.LastTrack; i++) {
		if (!ReadCDForCheckingSubQAdr(pExtArg, pDevice, pDisc
			, pDiscPerSector, lpCmd, lpBuf, uiBufLen, nOfs, i, &byMode, 1, NULL)) {
			return FALSE;
		}
		OutputString(
			"\rChecking SubQ adr (Track) %2d/%2u", i + 1, pDisc->SCSI.toc.LastTrack);
	}
	OutputString("\n");
	return TRUE;
}
