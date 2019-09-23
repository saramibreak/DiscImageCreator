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
#include "struct.h"
#include "calcHash.h"
#include "check.h"
#include "convert.h"
#include "execScsiCmd.h"
#include "execScsiCmdforCD.h"
#include "execScsiCmdforCDCheck.h"
#include "execScsiCmdforFileSystem.h"
#include "get.h"
#include "output.h"
#include "outputScsiCmdLogforCD.h"
#include "set.h"

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
			OutputDiscLogA("SubQ is all zero... (BufLen: %d)\n", uiBufLen);
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
	INT nDriveSampleOffset,
	INT nDriveOffset
) {
	BOOL bRet = ExecReadCD(pExtArg, pDevice, lpCmd
		, nLBA, lpBuf, uiBufSize, _T(__FUNCTION__), __LINE__);

	if (!bRet) {
		if (*pExecType == gd) {
			OutputErrorString(
				_T("Couldn't read data sectors at scrambled state [OpCode: %#02x, C2flag: %x, SubCode: %x]\n")
				, lpCmd[0], (lpCmd[9] & 0x6) >> 1, lpCmd[10]);
		}
		else {
			if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
				OutputLogA(standardError | fileDrive,
					"This drive doesn't support [OpCode: %#02x, SubCode: %x]\n", lpCmd[0], lpCmd[10]);
			}
			else {
				OutputErrorString(
					_T("This drive can't read data sectors at scrambled state [OpCode: %#02x, C2flag: %x, SubCode: %x]\n")
					, lpCmd[0], (lpCmd[9] & 0x6) >> 1, lpCmd[10]);
			}
		}
		return FALSE;
	}
	else {
		if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
			OutputLogA(standardOut | fileDrive,
				"This drive supports [OpCode: %#02x, SubCode: %x]\n", lpCmd[0], lpCmd[10]);
		}
		else {
			if (*pExecType != data) {
				OutputLogA(standardOut | fileDrive,
					"This drive can read data sectors at scrambled state [OpCode: %#02x, C2flag: %x, SubCode: %x]\n"
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
		OutputDiscLogA(
			OUTPUT_DHYPHEN_PLUS_STR_WITH_SUBCH_F(Check Drive + CD offset), lpCmd[0], lpCmd[10]);
	}
	else if ((!pExtArg->byD8 && !pDevice->byPlxtrDrive) || pExtArg->byBe) {
		if (lpCmd[10] == CDFLAG::_READ_CD::Q) {
			// because check only
			return TRUE;
		}
		OutputDiscLogA(
			OUTPUT_DHYPHEN_PLUS_STR_WITH_C2_SUBCH_F(Check Drive + CD offset), lpCmd[0], (lpCmd[9] & 0x6) >> 1, lpCmd[10]);
	}

	if (pDisc->SCSI.trackType != TRACK_TYPE::audioOnly || *pExecType == swap) {
		if (*pExecType != data) {
			OutputCDMain(fileDisc, lpBuf, nLBA, CD_RAW_SECTOR_SIZE);
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
			OutputDiscLogA("\tSubChannel Offset: %d\n", pDisc->SUB.nSubChannelOffset);
		}
	}
	else {
		if (pDisc->SCSI.trackType != TRACK_TYPE::audioOnly || *pExecType == swap) {
			BYTE aBuf[CD_RAW_SECTOR_SIZE * 2] = {};
			memcpy(aBuf, lpBuf, CD_RAW_SECTOR_SIZE);

			if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA + 1
				, lpBuf, uiBufSize, _T(__FUNCTION__), __LINE__)) {
				return FALSE;
			}
			OutputCDMain(fileDisc, lpBuf, nLBA + 1, CD_RAW_SECTOR_SIZE);

			memcpy(aBuf + CD_RAW_SECTOR_SIZE, lpBuf, CD_RAW_SECTOR_SIZE);
			if (pDisc->SCSI.trackType != TRACK_TYPE::audioOnly || *pExecType == swap) {
				if (!GetWriteOffset(pDisc, aBuf)) {
					if (pDisc->SCSI.trackType == TRACK_TYPE::dataExist ||
						pDisc->SCSI.trackType == TRACK_TYPE::pregapDataIn1stTrack) {
						OutputLogA(standardError | fileDisc, _T("Failed to get write-offset\n"));
						return FALSE;
					}
					OutputLogA(standardOut | fileDisc,
						"There isn't data sector in pregap sector of track 1\n");
					pDisc->SCSI.trackType = TRACK_TYPE::audioOnly;
				}
			}
		}
		else if (pDisc->SCSI.trackType == TRACK_TYPE::audioOnly &&
			(pExtArg->byVideoNow || pExtArg->byVideoNowColor)) {
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
			// VideoNow Color Jr. XP
			CONST BYTE aVideoNowBytes[] = {
				0x81, 0xe3, 0xe3, 0xc7, 0xc7, 0x81, 0x81, 0xe3,
			};
			// VideoNow B&W
			CONST BYTE aVideoNowBytesOrg[] = {
				0xe1, 0xe1, 0xe1, 0x01, 0xe1, 0xe1, 0xe1, 0x00,
			};
			INT nSector = 1;

			for (INT i = 0; i < CD_RAW_SECTOR_SIZE * 15; i++) {
				for (size_t c = 0; c < sizeof(aVideoNowBytes); c++) {
					if (lpBuf2[i + c] != aVideoNowBytes[c]) {
						bRet = FALSE;
						break;
					}
					if (c == sizeof(aVideoNowBytes) - 1) {
						OutputLogA(standardOut | fileDisc, "Detected VideoNow Color or Jr. or XP\n");
						bRet = TRUE;

						if (pExtArg->byVideoNowColor) {
							OutputLogA(standardOut | fileDisc, "Search incomplete frame of track 01\n");
							LPBYTE pBuf3 = NULL;
							LPBYTE lpBuf3 = NULL;
							if (!GetAlignedCallocatedBuffer(pDevice, &pBuf3,
								CD_RAW_SECTOR_SIZE * 9, &lpBuf3, _T(__FUNCTION__), __LINE__)) {
								return FALSE;
							}
							INT tmpLBA = 883;
							nSector--;
							for (INT j = nSector; j < 9 + nSector; j++) {
								if (!ExecReadCD(pExtArg, pDevice, lpCmd, tmpLBA + j
									, aBuf, uiBufSize, _T(__FUNCTION__), __LINE__)) {
									return FALSE;
								}
								memcpy(lpBuf3 + CD_RAW_SECTOR_SIZE * (j - nSector), aBuf, CD_RAW_SECTOR_SIZE);
								OutputCDMain(fileMainInfo, lpBuf3 + CD_RAW_SECTOR_SIZE * (j - nSector), tmpLBA + j, CD_RAW_SECTOR_SIZE);
							}
							INT n1stHeaderOfs = 0;
							INT n2ndHeaderOfs = 0;
							for (INT m = 0; m < CD_RAW_SECTOR_SIZE * 9; m++) {
								for (size_t d = 0; d < sizeof(aVideoNowBytes); d++) {
									if (lpBuf3[m + d] != aVideoNowBytes[d]) {
										bRet = FALSE;
										break;
									}
									if (d == sizeof(aVideoNowBytes) - 1) {
										if (n1stHeaderOfs == 0) {
											n1stHeaderOfs = m;
											OutputLogA(standardOut | fileDisc, "1stHeaderOfs: %d (0x%x)\n"
												, n1stHeaderOfs, n1stHeaderOfs);
											m += 400;
											break;
										}
										else {
											n2ndHeaderOfs = m;
											OutputLogA(standardOut | fileDisc, "2ndHeaderOfs: %d (0x%x)\n"
												, n2ndHeaderOfs, n2ndHeaderOfs);
											OutputLogA(standardOut | fileDisc
												, "Empty bytes which are needed in this disc: %d [18032 - (%d - %d)]\n"
												, 18032 - (n2ndHeaderOfs - n1stHeaderOfs)
												, n2ndHeaderOfs, n1stHeaderOfs
											);
											pExtArg->nAudioCDOffsetNum = 18032 - (n2ndHeaderOfs - n1stHeaderOfs);
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
						}
					}
				}
				if (!bRet) {
					for (size_t c = 0; c < sizeof(aVideoNowBytesOrg); c++) {
						if (lpBuf2[i + c] != aVideoNowBytesOrg[c]) {
							bRet = FALSE;
							break;
						}
						if (c == sizeof(aVideoNowBytesOrg) - 1) {
							OutputLogA(standardOut | fileDisc, "Detected VideoNow B&W\n");
							bRet = TRUE;
						}
					}
				}
				if (bRet) {
					pDisc->MAIN.nCombinedOffset = i - pExtArg->nAudioCDOffsetNum;
					if (!pExtArg->byVideoNowColor) {
						nSector--;
						OutputCDMain(fileDisc, lpBuf2 + CD_RAW_SECTOR_SIZE * nSector, nLBA + nSector, CD_RAW_SECTOR_SIZE);
					}
					break;
				}
				else if (i == CD_RAW_SECTOR_SIZE * nSector - 1) {
					nSector++;
				}
			}
			FreeAndNull(pBuf2);
		}
		else if (pDisc->SCSI.trackType == TRACK_TYPE::audioOnly && pExtArg->byAtari) {
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
			nLBA = pDisc->SCSI.nFirstLBAof2ndSession;
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
							OutputLogA(standardOut | fileDisc, "Detected Atari Jaguar CD Header\n");
							bRet = TRUE;
						}
					}
					if (bRet) {
						pDisc->MAIN.nCombinedOffset = i - 2 + CD_RAW_SECTOR_SIZE * nSector;
						OutputCDMain(fileDisc, lpBuf, nLBA, CD_RAW_SECTOR_SIZE);
						break;
					}
				}
				if (!bRet) {
					nSector--;
					nLBA += nSector;
				}
			} while (!bRet);
		}
		OutputCDOffset(pExtArg, pDisc, bGetDriveOffset
			, nDriveSampleOffset, nDriveOffset, pDisc->SUB.nSubChannelOffset);
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
	INT nDriveSampleOffset = 0;
	BOOL bGetDriveOffset = GetDriveOffsetAuto(pDevice->szProductId, &nDriveSampleOffset);
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
		nDriveSampleOffset = 30;
		bGetDriveOffset = TRUE;
	}
	else if (
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXW4824A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXW4012A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXW4012S
		) {
		nDriveSampleOffset = 98;
		bGetDriveOffset = TRUE;
	}
	else if (!strncmp(pDevice->szProductId, "DVD-ROM TS-H353A", 16)) {
		nDriveSampleOffset = 6;
		bGetDriveOffset = TRUE;
	}
#endif
	if (!bGetDriveOffset) {
		GetDriveOffsetManually(&nDriveSampleOffset);
	}

	INT nDriveOffset = nDriveSampleOffset * 4; // byte size * 4 = sample size
	if (pDisc->SCSI.trackType != TRACK_TYPE::dataExist &&
		pDisc->SCSI.trackType != TRACK_TYPE::pregapDataIn1stTrack) {
		pDisc->MAIN.nCombinedOffset = nDriveOffset;
	}
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	if (*pExecType == gd) {
		pDisc->SCSI.nFirstLBAofDataTrack = FIRST_LBA_FOR_GD;
	}
	FlushLog();
	BYTE lpCmd[CDB12GENERIC_LENGTH] = {};
	if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
		CDB::_PLXTR_READ_CDDA cdb = {};
		SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::NoSub);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);

		INT nLBA = pDisc->SCSI.nFirstLBAofDataTrack;
		ZeroMemory(lpBuf, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE);

		if (pDisc->SCSI.trackType != TRACK_TYPE::audioOnly) {
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
				, CD_RAW_SECTOR_SIZE, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
				bRet = FALSE;
			}
		}
		lpCmd[10] = (BYTE)CDFLAG::_PLXTR_READ_CDDA::MainQ;
		if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
			, CD_RAW_SECTOR_SIZE + 16, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
			// not return FALSE
		}
		lpCmd[10] = (BYTE)CDFLAG::_PLXTR_READ_CDDA::MainPack;
		for (INT n = 1; n <= 10; n++) {
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
				, CD_RAW_SECTOR_WITH_SUBCODE_SIZE, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
				if (n == 10) {
					bRet = FALSE;
					break;
				}
				StartStopUnit(pExtArg, pDevice, STOP_UNIT_CODE, STOP_UNIT_CODE);
				UINT milliseconds = 10000;
				OutputErrorString(_T("Retry %d/10 after %d milliseconds\n"), n, milliseconds);
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
			, CD_RAW_READ_SUBCODE_SIZE, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
			// not return FALSE
		}
#endif
		lpCmd[10] = (BYTE)CDFLAG::_PLXTR_READ_CDDA::MainC2Raw;
		if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
			, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
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

		INT nLBA = pDisc->SCSI.nFirstLBAofDataTrack;
		ZeroMemory(lpBuf, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE);

		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			SetReadCDCommand(pDevice, &cdb, flg
				, 1, CDFLAG::_READ_CD::byte294, CDFLAG::_READ_CD::NoSub);
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
			if (pDisc->SCSI.trackType != TRACK_TYPE::audioOnly) {
				// Audio only disc doesn't call this because of NoSub mode 
				if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_WITH_C2_294_SIZE, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
					// not return FALSE
				}
			}
			lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Raw;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
				, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
				bRet = FALSE;
			}
			lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Q;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
				, CD_RAW_SECTOR_WITH_C2_294_SIZE + 16, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
				// not return FALSE
			}
			lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Pack;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
				, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
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
				if (pDisc->SCSI.trackType != TRACK_TYPE::audioOnly) {
					if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
						, CD_RAW_SECTOR_WITH_C2_SIZE, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
						// not return FALSE
					}
				}
				lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Raw;
				if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
					bRet = FALSE;
				}
				lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Q;
				if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_WITH_C2_SIZE + 16, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
					// not return FALSE
				}
				lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Pack;
				if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
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
			if (*pExecType != data && pDisc->SCSI.trackType != TRACK_TYPE::audioOnly) {
				lpCmd[10] = (BYTE)CDFLAG::_READ_CD::NoSub;
				// Audio only disc doesn't call this because of NoSub mode 
				if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_SIZE, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
					// not return FALSE
				}
			}
			lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Raw;
			for (INT n = 1; n <= 10; n++) {
				if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_WITH_SUBCODE_SIZE, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
					if (n == 10) {
						bRet = FALSE;
						break;
					}
					StartStopUnit(pExtArg, pDevice, STOP_UNIT_CODE, STOP_UNIT_CODE);
					UINT milliseconds = 10000;
					OutputErrorString(_T("Retry %d/10 after %d milliseconds\n"), n, milliseconds);
					Sleep(milliseconds);
					continue;
				}
				else {
					break;
				}
			}
			lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Q;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
				, CD_RAW_SECTOR_SIZE + 16, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
				// not return FALSE
			}
			lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Pack;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
				, CD_RAW_SECTOR_WITH_SUBCODE_SIZE, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
				// not return FALSE
			}
		}
	}
	FreeAndNull(pBuf);
	return bRet;
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
		OutputLogA(standardOut | fileDrive, "Checking reading lead-in -> ");
		nLBA = -1;
	}
	else if (0 < pDisc->MAIN.nCombinedOffset && *pExecType == cd) {
		OutputLogA(standardOut | fileDrive, "Checking reading lead-out -> ");
		nLBA = pDisc->SCSI.nAllLength;
	}
	// buffer is unused but buf null and size zero is semaphore error...
	BYTE aBuf[CD_RAW_SECTOR_SIZE] = {};
	BYTE byScsiStatus = 0;
	if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, aBuf,
		CD_RAW_SECTOR_SIZE, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		if (pDisc->MAIN.nCombinedOffset < 0) {
			OutputLogA(standardOut | fileDrive, "This drive can't read the lead-in\n");
		}
		else if (0 < pDisc->MAIN.nCombinedOffset) {
			OutputLogA(standardOut | fileDrive, "This drive can't read the lead-out\n");
			if (IsValidAsusDrive(pDevice)) {
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA - 1, aBuf,
					CD_RAW_SECTOR_SIZE, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				}
//				OutputCDMain(fileMainInfo, aBuf, nLBA - 1, CD_RAW_SECTOR_SIZE);
				if (ReadCacheForLgAsus(pExtArg, pDevice, pDisc, 1)) {
					OutputLogA(standardOut | fileDrive, "But 0xF1 is supported\n");
					return TRUE;
				}
			}
		}
		return FALSE;
	}
	else {
		if (nLBA != 0) {
			OutputLogA(standardOut | fileDrive, "OK\n");
		}
	}
#if 0
	OutputCDMain(fileMainInfo, aBuf, nLBA, CD_RAW_SECTOR_SIZE);
#endif
	return TRUE;
}

BOOL ReadCDForCheckingSubQAdrFirst(
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
	if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		c2 = CDFLAG::_READ_CD::byte294;
		*uiBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * 2;
	}
	SetReadDiscCommand(NULL, pExtArg, pDevice, 2, c2, CDFLAG::_READ_CD::Raw, lpCmd, FALSE);
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
	CHAR szTmpCatalog[META_CATALOG_SIZE] = {};
	CHAR szTmpISRC[META_ISRC_SIZE] = {};
	INT nMCNIdx = 0;
	INT nISRCIdx = 0;
	INT nTmpMCNLBAList[25] = { -1 };
	INT nTmpISRCLBAList[25] = { -1 };
	INT nTmpLBA = pDisc->SCSI.lpFirstLBAListOnToc[byIdxOfTrack];
	INT nTmpNextLBA = 0;
	if (byIdxOfTrack + 1 < pDisc->SCSI.byLastDataTrackNum) {
		nTmpNextLBA = pDisc->SCSI.lpFirstLBAListOnToc[byIdxOfTrack + 1] - nTmpLBA;
	}
	else {
		nTmpNextLBA = pDisc->SCSI.nAllLength - nTmpLBA;
	}
	pDiscPerSector->byTrackNum = BYTE(byIdxOfTrack + 1);
	INT nSubOfs = CD_RAW_SECTOR_SIZE;
	if (pDevice->driveOrder == DRIVE_DATA_ORDER::MainC2Sub) {
		nSubOfs = CD_RAW_SECTOR_WITH_C2_294_SIZE;
	}

	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_TRACK_F(Check MCN and/or ISRC), lpCmd[0], lpCmd[10], byIdxOfTrack + 1);
	for (INT nLBA = nTmpLBA; nLBA < nTmpLBA + 400; nLBA++) {
		if (400 > nTmpNextLBA) {
			bCheckMCN = FALSE;
			bCheckISRC = FALSE;
			break;
		}
		if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, lpBuf,
			uiBufLen, _T(__FUNCTION__), __LINE__)) {
			// skip checking
			return TRUE;
		}
		AlignRowSubcode(pDiscPerSector->subcode.current, lpBuf + nSubOfs);
#if 0
		OutputCDMain(lpBuf2, nLBA, CD_RAW_SECTOR_SIZE);
		OutputCDSub96Align(pDiscPerSector->subcode.current, nLBA);
#endif
		if (nLBA == nTmpLBA) {
			memcpy(pDiscPerSector->mainHeader.current, lpBuf + nOfs, MAINHEADER_MODE1_SIZE);
			// this func is used to get a SubChannel Offset
			SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.current, pDiscPerSector->subcode.current);
			pDiscPerSector->subQ.current.byCtl = (BYTE)((BYTE)(pDiscPerSector->subcode.current[12] >> 4) & 0x0f);
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
						OutputDiscLogA("\tMCN: [%s]\n", szCatalog);
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
						OutputDiscLogA("\tISRC: [%s]\n", szISRC);
					}
				}
			}
		}
	}
	if (bCheckMCN) {
		SetLBAForFirstAdr(pDisc->SUB.nFirstLBAForMCN, pDisc->SUB.nRangeLBAForMCN,
			"MCN", nTmpMCNLBAList, (BYTE)(bySessionNum - 1), pDevice->byPlxtrDrive);
	}
	if (bCheckISRC) {
		SetLBAForFirstAdr(pDisc->SUB.nFirstLBAForISRC, pDisc->SUB.nRangeLBAForISRC,
			"ISRC", nTmpISRCLBAList, (BYTE)(bySessionNum - 1), pDevice->byPlxtrDrive);
	}
	if (!bCheckMCN && !bCheckISRC) {
		OutputDiscLogA("\tNothing\n");
	}
	return TRUE;
}

BOOL ReadCDForCheckingSubRtoW(
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
	if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		c2 = CDFLAG::_READ_CD::byte294;
		uiBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
	}
	SetReadDiscCommand(NULL, pExtArg, pDevice, 1, c2, CDFLAG::_READ_CD::Raw, lpCmd, FALSE);

	for (BYTE i = (BYTE)(pDisc->SCSI.toc.FirstTrack - 1); i < pDisc->SCSI.toc.LastTrack; i++) {
		try {
			OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_TRACK_F(Check CD + G), lpCmd[0], lpCmd[10], i + 1);
			INT nTmpLBA = pDisc->SCSI.lpFirstLBAListOnToc[i] + 100;
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
				case 20: // MODE 2, ITEM 4
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
				OutputDiscLogA("\tAll RtoW is 0xff\n");
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
				OutputDiscLogA("\tAll RtoW is 0x57, 0x33, 0x13\n");
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
				OutputDiscLogA("\tAll RtoW is 0x33\n");
			}
			else {
				BOOL bAnyFull = FALSE;
				// 0xff * 12 = 0xbf4
				if (nR == 0xbf4) {
					OutputDiscLogA("\tAll R is 0xff\n");
					bAnyFull = TRUE;
				}
				if (nS == 0xbf4) {
					OutputDiscLogA("\tAll S is 0xff\n");
					bAnyFull = TRUE;
				}
				if (nT == 0xbf4) {
					OutputDiscLogA("\tAll T is 0xff\n");
					bAnyFull = TRUE;
				}
				if (nU == 0xbf4) {
					OutputDiscLogA("\tAll U is 0xff\n");
					bAnyFull = TRUE;
				}
				if (nV == 0xbf4) {
					OutputDiscLogA("\tAll V is 0xff\n");
					bAnyFull = TRUE;
				}
				if (nW == 0xbf4) {
					OutputDiscLogA("\tAll W is 0xff\n");
					bAnyFull = TRUE;
				}
				if (bAnyFull) {
					pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::AnyFull;
				}
				else {
					if (bCDG && nRtoW > 0 && nRtoW != 0x200) {
						pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::CDG;
						OutputDiscLogA("\tCD+G\n");
					}
					else if (bCDEG && nRtoW > 0 && nRtoW != 0x200) {
						pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::CDG;
						OutputDiscLogA("\tCD+EG\n");
					}
					else if ((0 <= nR && nR <= 0x03) && (0 <= nS && nS <= 0x03) &&
						(0 <= nT && nT <= 0x03) && (0 <= nU && nU <= 0x03) &&
						(0 <= nV && nV <= 0x03) && (0 <= nW && nW <= 0x03) && nRtoW != 0) {
						pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::PSXSpecific;
						OutputDiscLogA("\tRandom data exists (PSX)\n");
					}
					else {
						pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::Zero;
						OutputDiscLogA("\tNothing\n");
					}
				}
			}
		}
		catch (BOOL bErr) {
			bRet = bErr;
		}
		OutputString(
			_T("\rChecking SubRtoW (Track) %2u/%2u"), i + 1, pDisc->SCSI.toc.LastTrack);
	}
	OutputString(_T("\n"));
	FreeAndNull(pBuf);
	return bRet;
}
#if 0
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
		printf("Extracted %s\n", pFilePaths->Target);
		break;
	case SPFILENOTIFY_FILEOPDELAYED:
		break;
	}
	return lRetVal;
}

BOOL IterateCabinet(
	PTSTR pszCabFile
) {
	_TCHAR szExtractdir[_MAX_PATH];
	if (!GetCurrentDirectory(sizeof(szExtractdir) / sizeof(szExtractdir[0]), szExtractdir)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	lstrcat(szExtractdir, "\\extract_cab\\");
	_TCHAR szExtractdirFind[_MAX_PATH];
	memcpy(szExtractdirFind, szExtractdir, _MAX_PATH);
	lstrcat(szExtractdirFind, "*");

	if (PathFileExists(szExtractdir)) {
		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFile(szExtractdirFind, &fd);
		if (INVALID_HANDLE_VALUE == hFind) {
			return FALSE;
		}
		do {
			if (0 != _tcscmp(fd.cFileName, _T("."))
				&& 0 != _tcscmp(fd.cFileName, _T(".."))) {
				TCHAR szFoundFilePathName[_MAX_PATH];
				_tcsncpy(szFoundFilePathName, szExtractdir, _MAX_PATH);
				_tcsncat(szFoundFilePathName, fd.cFileName, _MAX_PATH);

				if (!(FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes)) {
					if (!DeleteFile(szFoundFilePathName)) {
						FindClose(hFind);
						return FALSE;
					}
				}
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	if (!MakeSureDirectoryPathExists(szExtractdir)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (!SetupIterateCabinet(pszCabFile,
		0, (PSP_FILE_CALLBACK)CabinetCallback, szExtractdir)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	return TRUE;
}
#endif
BOOL ReadCDForCheckingExe(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE pCdb,
	LPBYTE lpBuf
) {
	BOOL bRet = TRUE;
	DWORD dwSize = DISC_RAW_READ_SIZE;
	BYTE byTransferLen = 1;
	BYTE byRoopLen = byTransferLen;
	SetCommandForTransferLength(pExecType, pDevice, pCdb, dwSize, &byTransferLen, &byRoopLen);
	for (INT n = 0; pDisc->PROTECT.pExtentPosForExe[n] != 0; n++) {
#if 0
		if (strstr(pDisc->PROTECT.pNameForExe[n], ".CAB") || strstr(pDisc->PROTECT.pNameForExe[n], ".cab")) {
			// Get the absPath of cab file from path table
			IterateCabinet(pDisc->PROTECT.pNameForExe[n]);
			IterateCabinet("C:\\test\\disk1\\1.cab");
			// Search exe, dll from extracted file
			// Open exe, dll
			// Read
		}
		else {
#endif
			if (!ExecReadCD(pExtArg, pDevice, pCdb, pDisc->PROTECT.pExtentPosForExe[n],
				lpBuf, dwSize, _T(__FUNCTION__), __LINE__)) {
				//				return FALSE;
				// FIFA 99 (Europe) on PX-5224A
				// LBA[000000, 0000000], [F:ReadCDForCheckingExe][L:734]
				//		OperationCode: 0xa8
				//		ScsiStatus: 0x02 = CHECK_CONDITION
				//		SenseData Key-Asc-Ascq: 03-02-83 = MEDIUM_ERROR - OTHER
				//  =>  The reason is unknown...
				continue;
			}
#if 0
		}
#endif
		WORD wMagic = MAKEWORD(lpBuf[0], lpBuf[1]);
		if (wMagic == IMAGE_DOS_SIGNATURE) {
			PIMAGE_DOS_HEADER pIDh = (PIMAGE_DOS_HEADER)&lpBuf[0];
			if (dwSize < (DWORD)pIDh->e_lfanew) {
				if (pDevice->dwMaxTransferLength < (DWORD)pIDh->e_lfanew) {
					OutputVolDescLogA("%s: offset is very big (%lu). read skip [TODO]\n"
						, pDisc->PROTECT.pNameForExe[n], pIDh->e_lfanew);
				}
				else {
					SetCommandForTransferLength(pExecType, pDevice, pCdb, (DWORD)pIDh->e_lfanew, &byTransferLen, &byRoopLen);
					dwSize = DWORD(DISC_RAW_READ_SIZE) * byTransferLen;
					n--;
				}
				continue;
			}
			OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA
				, pDisc->PROTECT.pExtentPosForExe[n], pDisc->PROTECT.pExtentPosForExe[n], pDisc->PROTECT.pNameForExe[n]);
			OutputFsImageDosHeader(pIDh);
			WORD wMagic2 = MAKEWORD(lpBuf[pIDh->e_lfanew], lpBuf[pIDh->e_lfanew + 1]);
			if (wMagic2 == IMAGE_NT_SIGNATURE) {
				PIMAGE_NT_HEADERS32 pINH = (PIMAGE_NT_HEADERS32)&lpBuf[pIDh->e_lfanew];
				OutputFsImageNtHeader(pINH);
				ULONG nOfs = pIDh->e_lfanew + sizeof(IMAGE_NT_HEADERS32);
				for (INT i = 0; i < pINH->FileHeader.NumberOfSections; i++) {
					OutputFsImageSectionHeader(pDisc, (PIMAGE_SECTION_HEADER)&lpBuf[nOfs]);
					nOfs += sizeof(IMAGE_SECTION_HEADER);
				}
			}
			else if (wMagic2 == IMAGE_OS2_SIGNATURE) {
				OutputFsImageOS2Header((PIMAGE_OS2_HEADER)&lpBuf[pIDh->e_lfanew]);
			}
			else if (wMagic2 == IMAGE_OS2_SIGNATURE_LE) {
				// TODO
			}
			else {
				OutputVolDescLogA(
					"%s: ImageNT,NE,LEHeader doesn't exist\n", pDisc->PROTECT.pNameForExe[n]);
			}
		}
		else {
			OutputVolDescLogA(
				"%s: ImageDosHeader doesn't exist\n", pDisc->PROTECT.pNameForExe[n]);
		}
		OutputString(_T("\rChecking EXE %4d"), n + 1);
	}
	OutputString(_T("\n"));
	return bRet;
}

BOOL ReadCDForSegaDisc(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	BYTE buf[DISC_RAW_READ_SIZE] = {};
	CDB::_READ12 cdb = {};
	cdb.OperationCode = SCSIOP_READ12;
	cdb.TransferLength[3] = 1;

	if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, 0, buf,
		DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
	}
	if (!memcmp(buf, "SEGA", 4)) {
		OutputCDMain(fileMainInfo, buf, 0, DISC_RAW_READ_SIZE);
	}
	return TRUE;
}

BOOL ReadCDForCheckingPsxRegion(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	BYTE buf[DISC_RAW_READ_SIZE] = {};
	CONST CHAR regionPal[] =
		"          Licensed  by          Sony Computer Entertainment Euro pe   ";
	CONST CHAR regionPal2[] =
		"          Licensed  by          Sony Computer Entertainment(Europe)";
	CDB::_READ12 cdb = {};
	cdb.OperationCode = SCSIOP_READ12;
	cdb.TransferLength[3] = 1;

	if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, 4, buf,
		DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
		return TRUE;
	}
	OutputCDMain(fileMainInfo, buf, 4, 80);
	if (!memcmp(buf, regionPal, sizeof(regionPal))) {
		return TRUE;
	}
	else if (!memcmp(buf, regionPal2, sizeof(regionPal2))) {
		return TRUE;
	}
	OutputString(
		_T("[INFO] This disc isn't PSX PAL. /nl is ignored.\n"));
	return FALSE;
}

VOID ReadCDForScanningPsxAntiMod(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	BOOL bRet = FALSE;
	BYTE buf[DISC_RAW_READ_SIZE] = {};
	CONST CHAR antiModStrEn[] =
		"     SOFTWARE TERMINATED\nCONSOLE MAY HAVE BEEN MODIFIED\n     CALL 1-888-780-7690";
	CONST CHAR antiModStrJp[] =
		"強制終了しました。\n本体が改造されている\nおそれがあります。";
	CDB::_READ12 cdb = {};
	cdb.OperationCode = SCSIOP_READ12;
	cdb.TransferLength[3] = 1;

	for (INT nLBA = 18; nLBA < pDisc->SCSI.nLastLBAofDataTrack - 150; nLBA++) {
		if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, buf,
			DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
			return;
		}
		for (INT i = 0; i < DISC_RAW_READ_SIZE; i++) {
			if (!memcmp(&buf[i], antiModStrEn, sizeof(antiModStrEn))) {
				OutputLogA(fileDisc | standardOut, "\nDetected anti-mod string (en): LBA %d", nLBA);
				OutputCDMain(fileMainInfo, buf, nLBA, DISC_RAW_READ_SIZE);
				bRet += TRUE;
			}
			if (!memcmp(&buf[i], antiModStrJp, sizeof(antiModStrJp))) {
				OutputLogA(fileDisc | standardOut, "\nDetected anti-mod string (jp): LBA %d\n", nLBA);
				if (!bRet) {
					OutputCDMain(fileMainInfo, buf, nLBA, DISC_RAW_READ_SIZE);
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
		OutputString(_T("\rScanning sector for anti-mod string (LBA) %6d/%6d"), nLBA, pDisc->SCSI.nLastLBAofDataTrack - 150 - 1);
	}
	if (!bRet) {
		OutputLogA(fileDisc | standardOut, "\nNo anti-mod string\n");
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
	if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		dwBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
		c2 = CDFLAG::_READ_CD::byte294;
	}
	SetReadDiscCommand(NULL, pExtArg, pDevice, 1, c2, CDFLAG::_READ_CD::Raw, lpCmd, FALSE);

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
			OutputLogA(standardOut | fileDisc, "\nDetected ProtectCD VOB. It begins from %d sector", nLBA);
			pDisc->PROTECT.ERROR_SECTOR.nExtentPos = nLBA;
			pDisc->PROTECT.ERROR_SECTOR.nSectorSize = pDisc->SCSI.nAllLength - nLBA - 1;
			pDisc->PROTECT.byExist = protectCDVOB;
			pExtArg->byScanProtectViaFile = pExtArg->byScanProtectViaSector;
			break;
		}
		OutputString(_T("\rScanning sector (LBA) %6d/%6d"), nLBA, pDisc->SCSI.nAllLength - 1);
	}
	OutputLogA(standardOut | fileDisc, "\n");

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
	OutputSubInfoWithLBALogA(
		"CRC-16 is original:[%02x%02x], recalc:[%04x] and XORed with 0x8001:[%02x%02x]\n"
		, -1, 0, pDiscPerSector->subcode.current[22], pDiscPerSector->subcode.current[23]
		, w, pDiscPerSector->subcode.current[22] ^ 0x80, pDiscPerSector->subcode.current[23] ^ 0x01);
#endif
	if (pExtArg->byIntentionalSub && pDisc->PROTECT.byExist != securomV1 &&
		(pDiscPerSector->subcode.current[12] == 0x41 || pDiscPerSector->subcode.current[12] == 0x61)) {
//		WORD crc16 = (WORD)GetCrc16CCITT(10, &pDiscPerSector->subcode.current[12]);
//		WORD bufcrc = MAKEWORD(pDiscPerSector->subcode.current[23], pDiscPerSector->subcode.current[22]);
		INT nRLBA = MSFtoLBA(BcdToDec(pDiscPerSector->subcode.current[15])
			, BcdToDec(pDiscPerSector->subcode.current[16]), BcdToDec(pDiscPerSector->subcode.current[17]));
		INT nALBA = MSFtoLBA(BcdToDec(pDiscPerSector->subcode.current[19])
			, BcdToDec(pDiscPerSector->subcode.current[20]), BcdToDec(pDiscPerSector->subcode.current[21]));

//		if (crc16 != bufcrc) {
		if ((nRLBA == 3000 || nRLBA == 3001) && nALBA == 299) { // 3001(00:40:01), 299(00:03:74)
			OutputSubInfoWithLBALogA(
				"Detected intentional error. CRC-16 is original:[%02x%02x] and XORed with 0x8001:[%02x%02x] "
				, -1, 0, pDiscPerSector->subcode.current[22] ^ 0x80, pDiscPerSector->subcode.current[23] ^ 0x01
				, pDiscPerSector->subcode.current[22], pDiscPerSector->subcode.current[23]);
			OutputSubInfoLogA(
				"RMSF[%02x:%02x:%02x] AMSF[%02x:%02x:%02x]\n"
				, pDiscPerSector->subcode.current[15], pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
				, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);

			OutputLogA(standardOut | fileDisc, "Detected intentional subchannel in LBA -1 => SecuROM Type 4 (a.k.a. NEW)\n");
			OutputIntentionalSubchannel(-1, &pDiscPerSector->subcode.current[12]);
			pDisc->PROTECT.byExist = securomV4;
			pDiscPerSector->subQ.prev.nRelativeTime = -1;
			pDiscPerSector->subQ.prev.nAbsoluteTime = 149;
		}
		else if ((nRLBA == 167295 || nRLBA == 0) && nALBA == 150) { // 167295(37:10:45), 150(00:02:00)
			OutputSubInfoWithLBALogA(
				"Detected shifted sub. RMSF[%02x:%02x:%02x] AMSF[%02x:%02x:%02x]\n"
				, -1, 0, pDiscPerSector->subcode.current[15], pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
				, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);

			// Colin McRae Rally 2.0 http://redump.org/disc/31587/
			if (nRLBA == 167295) {
				OutputLogA(standardOut | fileDisc, "Detected intentional subchannel in LBA -1 => SecuROM Type 3_1 (a.k.a. NEW)\n");
				pDisc->PROTECT.byExist = securomV3_1;
			}
			// Empire Earth http://redump.org/disc/45559/ Diablo II: Lord of Destruction (Expansion Set) http://redump.org/disc/58232/
			else if (nRLBA == 0) {
				OutputLogA(standardOut | fileDisc, "Detected intentional subchannel in LBA -1 => SecuROM Type 3_2 (a.k.a. NEW)\n");
				pDisc->PROTECT.byExist = securomV3_2;
			}
			OutputIntentionalSubchannel(-1, &pDiscPerSector->subcode.current[12]);
			if (pDisc->SUB.nSubChannelOffset) {
				pDisc->SUB.nSubChannelOffset -= 1;
			}
			pDiscPerSector->subQ.prev.nRelativeTime = -1;
			pDiscPerSector->subQ.prev.nAbsoluteTime = 149;
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
				OutputLogA(standardOut | fileDisc, "Detected intentional subchannel in LBA 5000 => SecuROM Type 2 (a.k.a. NEW)\n");
				pDisc->PROTECT.byExist = securomV2;
			}
			else if (pDisc->PROTECT.byExist == securomTmp) {
				pDisc->PROTECT.byExist = securomV1;
			}
			else {
				for (INT nTmpLBA = 40000; nTmpLBA < 45800; nTmpLBA++) {
					if (pDisc->SCSI.nAllLength > nTmpLBA) {
						if (!ExecReadCD(pExtArg, pDevice, lpCmd, nTmpLBA, pDiscPerSector->data.current,
							pDevice->TRANSFER.uiBufLen, _T(__FUNCTION__), __LINE__)) {
							return FALSE;
						}
						WORD reCalcCrc16 = (WORD)GetCrc16CCITT(10, &pDiscPerSector->subcode.current[12]);
						WORD reCalcXorCrc16 = (WORD)(reCalcCrc16 ^ 0x0080);
						if (pDiscPerSector->subcode.current[22] == HIBYTE(reCalcXorCrc16) &&
							pDiscPerSector->subcode.current[23] == LOBYTE(reCalcXorCrc16)) {
							OutputLogA(standardOut | fileDisc
								, "Detected intentional subchannel in LBA %d => SecuROM Type 1 (a.k.a. OLD)\n", nTmpLBA);
							pDisc->PROTECT.byExist = securomV1;
							break;
						}
					}
				}
				if (pDisc->PROTECT.byExist != securomV1) {
					OutputLogA(standardOut | fileDisc, "[INFO] SecuROM sector not found \n");
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
			OutputLogA(standardOut | fileDisc, "[INFO] SecuROM sector not found \n");
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
	pExtArg->byBe = TRUE;
	SetReadDiscCommand(NULL, pExtArg, pDevice, 1, c2, sub, lpCmd, FALSE);
	pExtArg->byBe = FALSE;

	BOOL bRet = TRUE;
	if (!ExecReadCD(pExtArg, pDevice, lpCmd, 0, lpBuf
		, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
		OutputLogA(standardError | fileDrive,
			"This drive doesn't support [OpCode: %#02x, C2flag: %x, SubCode: %x]\n"
			, lpCmd[0], (lpCmd[9] & 0x6) >> 1, lpCmd[10]);
		bRet = FALSE;
	}
	else {
		OutputDriveLogA(OUTPUT_DHYPHEN_PLUS_STR(Check main + c2 + sub));
		OutputCDC2Error296(fileDrive, lpBuf + CD_RAW_SECTOR_SIZE, 0);
		OutputCDSub96Raw(fileDrive, lpBuf + CD_RAW_SECTOR_WITH_C2_294_SIZE, 0);

		OutputDriveLogA(OUTPUT_DHYPHEN_PLUS_STR(Check main + sub + c2));
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
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	CDFLAG::_READ_CD::_ERROR_FLAGS* c2
) {
	SetBufferSizeForReadCD(pDevice, DRIVE_DATA_ORDER::NoC2);
	if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		*c2 = CDFLAG::_READ_CD::byte294;
		SetBufferSizeForReadCD(pDevice, DRIVE_DATA_ORDER::MainC2Sub);
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
				OutputLogA(standardError | fileDrive,
					"[WARNING] This drive doesn't support reporting C2 error. Disabled /c2\n");
				*c2 = CDFLAG::_READ_CD::NoC2;
				pDevice->driveOrder = DRIVE_DATA_ORDER::NoC2;
				pDevice->FEATURE.byC2ErrorData = FALSE;
				SetBufferSizeForReadCD(pDevice, DRIVE_DATA_ORDER::NoC2);
			}
		}
		if (pDevice->driveOrder == DRIVE_DATA_ORDER::MainSubC2) {
			OutputDriveLogA(
				"\tByte order of this drive is main + sub + c2\n");
			SetBufferSizeForReadCD(pDevice, DRIVE_DATA_ORDER::MainSubC2);
		}
		else if (pDevice->driveOrder == DRIVE_DATA_ORDER::MainC2Sub) {
			OutputDriveLogA(
				"\tByte order of this drive is main + c2 + sub\n");
		}
	}
#ifdef _DEBUG
	OutputString(
		_T("BufLen %ubyte, BufC2Offset %ubyte, BufSubOffset %ubyte\n"),
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
		OutputLogA(standardOut | fileDisc,
			_T("[INFO] This disc isn't Multi-Session. /ms is ignored.\n"));
		pExtArg->byMultiSession = FALSE;
	}
	else if (pDisc->SCSI.bMultiSession && !pExtArg->byMultiSession) {
		OutputLogA(standardOut | fileDisc,
			_T("[INFO] This disc is Multi-Session. /ms is set.\n"));
		pExtArg->byMultiSession = TRUE;
	}

	if (!pExtArg->byReverse) {
		// Typically, CD+G data is included in audio only disc
		// But exceptionally, WonderMega Collection (SCD)(mixed disc) exists CD+G data.
		if (!ReadCDForCheckingSubRtoW(pExtArg, pDevice, pDisc)) {
			return FALSE;
		}
		if (pDisc->SCSI.trackType != TRACK_TYPE::audioOnly) {
			if (*pExecType == gd) {
				if (!ReadGDForFileSystem(pExecType, pExtArg, pDevice, pDisc)) {
					return FALSE;
				}
			}
			else {
				if (pDisc->SCSI.byFirstDataTrackNum == 1) {
					ReadCDForSegaDisc(pExtArg, pDevice);
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
						_T("[INFO] Protection can't be detected. /sf, /ss is ignored.\n"));
					pExtArg->byScanProtectViaFile = FALSE;
					pExtArg->byScanProtectViaSector = FALSE;
				}
			}
		}
	}
	return TRUE;
}

BOOL ReadGDForCheckingSubQAdr(
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

	if (!ReadCDForCheckingSubQAdrFirst(pExtArg
		, pDevice, pDisc, &pBuf, &lpBuf, lpCmd, &uiBufLen, &nOfs)) {
		return FALSE;
	}
	for (BYTE i = (BYTE)(pDisc->SCSI.toc.FirstTrack - 1); i < pDisc->SCSI.toc.LastTrack; i++) {
		if (!ReadCDForCheckingSubQAdr(pExtArg, pDevice, pDisc
			, pDiscPerSector, lpCmd, lpBuf, uiBufLen, nOfs, i, &byMode, 1, NULL)) {
			return FALSE;
		}
		OutputString(
			_T("\rChecking SubQ adr (Track) %2u/%2u"), i + 1, pDisc->SCSI.toc.LastTrack);
	}
	OutputString(_T("\n"));
	return TRUE;
}
