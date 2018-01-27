/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "execScsiCmd.h"
#include "execScsiCmdforCD.h"
#include "execIoctl.h"
#include "get.h"
#include "init.h"
#include "output.h"
#include "outputScsiCmdLog.h"
#include "outputScsiCmdLogforCD.h"
#include "set.h"
#include "calcHash.h"

// These global variable is set at prngcd.cpp
extern unsigned char scrambled_table[2352];

BOOL GetLBAForSubChannelOffset(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpCmd,
	INT nLBA,
	LPBYTE lpBuf,
	DWORD dwBufLen
) {
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	LPBYTE lpBufTmp = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		dwBufLen, &lpBufTmp, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	memcpy(lpBufTmp, lpBuf, dwBufLen);
	BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
	for (;;) {
		if (dwBufLen == CD_RAW_READ_SUBCODE_SIZE) {
			AlignRowSubcode(lpBufTmp, lpSubcode);
		}
		else if (dwBufLen == CD_RAW_SECTOR_WITH_SUBCODE_SIZE) {
			AlignRowSubcode(lpBufTmp + CD_RAW_SECTOR_SIZE, lpSubcode);
		}
		else if (dwBufLen == CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE) {
			AlignRowSubcode(lpBufTmp + CD_RAW_SECTOR_SIZE + CD_RAW_READ_C2_294_SIZE, lpSubcode);
		}
		OutputCDSub96Align(lpSubcode, nLBA);

		if ((lpSubcode[12] & 0x0f) == ADR_ENCODES_CURRENT_POSITION) {
			pDisc->SUB.nSubChannelOffset = MSFtoLBA(BcdToDec(lpSubcode[19]),
				BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21])) - 150 - nLBA;
			break;
		}
		else {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, ++nLBA
				, lpBufTmp, dwBufLen, _T(__FUNCTION__), __LINE__)) {
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
	DWORD dwBufSize,
	BOOL bGetDriveOffset,
	INT nDriveSampleOffset,
	INT nDriveOffset
) {
	BOOL bRet = ExecReadCD(pExtArg, pDevice, lpCmd
		, nLBA, lpBuf, dwBufSize, _T(__FUNCTION__), __LINE__);

	if (!bRet) {
		if (*pExecType == gd) {
			OutputErrorString(
				_T("Couldn't read a data sector at scrambled mode [OpCode: %#02x, C2flag: %x, SubCode: %x]\n")
					, lpCmd[0], (lpCmd[9] & 0x6) >> 1, lpCmd[10]);
		}
		else {
			if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
				OutputLogA(standardError | fileDrive,
					"This drive doesn't support [OpCode: %#02x, SubCode: %x]\n", lpCmd[0], lpCmd[10]);
			}
			else {
				OutputErrorString(
					_T("This drive can't read a data sector at scrambled mode [OpCode: %#02x, C2flag: %x, SubCode: %x]\n")
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
					"This drive can read a data sector at scrambled mode [OpCode: %#02x, C2flag: %x, SubCode: %x]\n"
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
	else if (!pExtArg->byD8 && !pDevice->byPlxtrDrive || pExtArg->byBe) {
		if (lpCmd[10] == CDFLAG::_READ_CD::Q) {
			// because check only
			return TRUE;
		}
		OutputDiscLogA(
			OUTPUT_DHYPHEN_PLUS_STR_WITH_C2_SUBCH_F(Check Drive + CD offset), lpCmd[0], (lpCmd[9] & 0x6) >> 1, lpCmd[10]);
	}

	if (pDisc->SCSI.trackType != TRACK_TYPE::audioOnly) {
//		if (pExtArg->byD8 || pDevice->byPlxtrDrive || *pExecType == gd) {
		if (*pExecType != data) {
			OutputCDMain(fileDisc, lpBuf, nLBA, CD_RAW_SECTOR_SIZE);
		}
	}
	if (dwBufSize == CD_RAW_SECTOR_WITH_SUBCODE_SIZE ||
		dwBufSize == CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE) {
		if (!GetLBAForSubChannelOffset(pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf, dwBufSize)) {
			return FALSE;
		}
	}
//	if (!pExtArg->byD8 && !pDevice->byPlxtrDrive && *pExecType != gd) {
	if (*pExecType == data) {
		if (pDisc->SUB.nSubChannelOffset != 0xff) {
			OutputDiscLogA("\tSubChannel Offset: %d\n", pDisc->SUB.nSubChannelOffset);
		}
	}
	else {
		if (pDisc->SCSI.trackType != TRACK_TYPE::audioOnly || *pExecType == gd) {
			BYTE aBuf[CD_RAW_SECTOR_SIZE * 2] = { 0 };
			memcpy(aBuf, lpBuf, CD_RAW_SECTOR_SIZE);

			if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA + 1
				, lpBuf, dwBufSize, _T(__FUNCTION__), __LINE__)) {
				return FALSE;
			}
			OutputCDMain(fileDisc, lpBuf, nLBA + 1, CD_RAW_SECTOR_SIZE);

			memcpy(aBuf + CD_RAW_SECTOR_SIZE, lpBuf, CD_RAW_SECTOR_SIZE);
			if (!GetWriteOffset(pDisc, aBuf)) {
				if (pDisc->SCSI.trackType == TRACK_TYPE::dataExist) {
					OutputErrorString(_T("Failed to get write-offset\n"));
					return FALSE;
				}
				// There isn't some data sector in pregap sector of track 1.
				pDisc->SCSI.trackType = TRACK_TYPE::audioOnly;
			}
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
	BOOL bGetDriveOffset = GetDriveOffset(pDevice->szProductId, &nDriveSampleOffset);
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
		_TCHAR aBuf[6] = { 0 };
		OutputString(
			_T("This drive doesn't define in driveOffset.txt\n")
			_T("Please input drive offset(Samples): "));
		INT b = _tscanf(_T("%6[^\n]%*[^\n]"), aBuf);
		b = _gettchar();
		nDriveSampleOffset = _ttoi(aBuf);
	}

	INT nDriveOffset = nDriveSampleOffset * 4; // byte size * 4 = sample size
	if (pDisc->SCSI.trackType != TRACK_TYPE::dataExist) {
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
	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
		CDB::_PLXTR_READ_CDDA cdb = { 0 };
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
				DWORD milliseconds = 10000;
				OutputErrorString(_T("Retry %d/10 after %ld milliseconds\n"), n, milliseconds);
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
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(NULL, pDevice, &cdb, flg
			, 1, CDFLAG::_READ_CD::NoC2, CDFLAG::_READ_CD::Raw, FALSE);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);

		INT nLBA = pDisc->SCSI.nFirstLBAofDataTrack;
		ZeroMemory(lpBuf, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE);

		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			SetReadCDCommand(pExtArg, pDevice, &cdb, flg
				, 1, CDFLAG::_READ_CD::byte294, CDFLAG::_READ_CD::NoSub, FALSE);
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
			if (pDisc->SCSI.trackType != TRACK_TYPE::audioOnly) {
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
			if (!bRet) {
				bRet = TRUE;
				SetReadCDCommand(pExtArg, pDevice, &cdb, flg
					, 1, CDFLAG::_READ_CD::byte296, CDFLAG::_READ_CD::NoSub, FALSE);
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
			}
		}
		else {
			if (*pExecType != data && pDisc->SCSI.trackType != TRACK_TYPE::audioOnly) {
				lpCmd[10] = (BYTE)CDFLAG::_READ_CD::NoSub;
				if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_SIZE, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
					// not return FALSE
				}
			}
			lpCmd[10] = (BYTE)CDFLAG::_READ_CD::Raw;
			for(INT n = 1; n <= 10; n++) {
				if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_WITH_SUBCODE_SIZE, bGetDriveOffset, nDriveSampleOffset, nDriveOffset)) {
					if (n == 10) {
						bRet = FALSE;
						break;
					}
					StartStopUnit(pExtArg, pDevice, STOP_UNIT_CODE, STOP_UNIT_CODE);
					DWORD milliseconds = 10000;
					OutputErrorString(_T("Retry %d/10 after %ld milliseconds\n"), n, milliseconds);
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
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	BOOL bRet = TRUE;
	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
		CDB::_PLXTR_READ_CDDA cdb = { 0 };
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::MainC2Raw);
		}
		else {
			SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::MainPack);
		}
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	else {
		// non plextor && support scrambled ripping
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(pExtArg, pDevice, &cdb, CDFLAG::_READ_CD::CDDA
			, 1, CDFLAG::_READ_CD::byte294, CDFLAG::_READ_CD::NoSub, TRUE);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	INT nLBA = 0;
	if (pDisc->MAIN.nCombinedOffset < 0) {
		OutputLogA(standardOut | fileDrive, "Checking reading lead-in -> ");
		nLBA = -1;
	}
	else if (0 < pDisc->MAIN.nCombinedOffset) {
		OutputLogA(standardOut | fileDrive, "Checking reading lead-out -> ");
		nLBA = pDisc->SCSI.nAllLength;
	}
	// buffer is unused but buf null and size zero is semaphore error...
	BYTE aBuf[CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE] = { 0 };
	BYTE byScsiStatus = 0;
	if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, aBuf,
		CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputLogA(standardOut | fileDrive, "OK\n");
#if 0
	OutputCDMain(fileMainInfo, aBuf, nLBA, CD_RAW_SECTOR_SIZE);
#endif
	return bRet;
}

BOOL ReadCDForCheckingSubQAdrFirst(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE* ppBuf,
	LPBYTE* lpBuf,
	LPBYTE lpCmd,
	LPINT nOfs
) {
	if (!GetAlignedCallocatedBuffer(pDevice, ppBuf,
		CD_RAW_SECTOR_WITH_SUBCODE_SIZE, lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
		CDB::_PLXTR_READ_CDDA cdb = { 0 };
		SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::MainPack);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		*nOfs = pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE;
		if (pDisc->MAIN.nCombinedOffset < 0) {
			*nOfs = CD_RAW_SECTOR_SIZE + *nOfs;
		}
	}
	else {
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(NULL, pDevice, &cdb, CDFLAG::_READ_CD::All
			, 1, CDFLAG::_READ_CD::byte294, CDFLAG::_READ_CD::Raw, TRUE);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
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
	INT nOfs,
	BYTE byIdxOfTrack,
	LPBYTE byMode,
	BYTE bySessionNum,
	FILE* fpCcd
) {
	BOOL bCheckMCN = FALSE;
	BOOL bCheckISRC = FALSE;
	CHAR szTmpCatalog[META_CATALOG_SIZE] = { 0 };
	CHAR szTmpISRC[META_ISRC_SIZE] = { 0 };
	INT nMCNIdx = 0;
	INT nISRCIdx = 0;
	INT nTmpMCNLBAList[9] = { -1 };
	INT nTmpISRCLBAList[9] = { -1 };
	INT nTmpLBA = pDisc->SCSI.lpFirstLBAListOnToc[byIdxOfTrack];
	INT nTmpNextLBA = 0;
	if (byIdxOfTrack + 1 < pDisc->SCSI.byLastDataTrackNum) {
		nTmpNextLBA = pDisc->SCSI.lpFirstLBAListOnToc[byIdxOfTrack + 1] - nTmpLBA;
	}
	else {
		nTmpNextLBA = pDisc->SCSI.nAllLength - nTmpLBA;
	}

	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_TRACK_F(Check MCN and/or ISRC), lpCmd[0], lpCmd[10], byIdxOfTrack + 1);
	for (INT nLBA = nTmpLBA; nLBA < nTmpLBA + 400; nLBA++) {
		if (400 > nTmpNextLBA) {
			bCheckMCN = FALSE;
			bCheckISRC = FALSE;
			break;
		}
		if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, lpBuf,
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
			// skip checking
			return TRUE;
		}
		LPBYTE lpBuf2 = lpBuf;
		if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
			lpBuf2 = lpBuf + nOfs;
		}
		BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
#if 0
		OutputCDMain(lpBuf2, nLBA, CD_RAW_SECTOR_SIZE);
		OutputCDSub96Align(lpSubcode, nLBA);
#endif
		if (nLBA == nTmpLBA) {
			// this func is used to get a SubChannel Offset
			SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.present, lpSubcode);
			BYTE byCtl = (BYTE)((lpSubcode[12] >> 4) & 0x0f);
			*byMode = GetMode(lpBuf2, 0, byCtl, unscrambled);
		}
		BOOL bCRC = FALSE;
		WORD crc16 = (WORD)GetCrc16CCITT(10, &lpSubcode[12]);
		BYTE tmp1 = HIBYTE(crc16);
		BYTE tmp2 = LOBYTE(crc16);
		if (lpSubcode[22] == tmp1 && lpSubcode[23] == tmp2) {
			bCRC = TRUE;
		}
		BYTE byAdr = (BYTE)(lpSubcode[12] & 0x0f);
		if (byAdr == ADR_ENCODES_MEDIA_CATALOG) {
			if (!bCRC) {
				SetBufferFromMCN(pDisc, lpSubcode);
				bCRC = TRUE;
			}
			BOOL bMCN = IsValidSubQMCN(lpSubcode);
			if (!bMCN && bCRC) {
				// force a invalid MCN to valid MCN
				bMCN = bCRC;
			}
			if (bMCN && bCRC) {
				nTmpMCNLBAList[nMCNIdx++] = nLBA;
				CHAR szCatalog[META_CATALOG_SIZE] = { 0 };
				if (!bCheckMCN) {
					SetMCNToString(pDisc, lpSubcode, szCatalog, FALSE);
					strncpy(szTmpCatalog, szCatalog, sizeof(szTmpCatalog) / sizeof(szTmpCatalog[0]));
					szTmpCatalog[META_CATALOG_SIZE - 1] = 0;
					bCheckMCN = bMCN;
				}
				else if (!pDisc->SUB.byCatalog) {
					SetMCNToString(pDisc, lpSubcode, szCatalog, FALSE);
					if (!strncmp(szTmpCatalog, szCatalog, sizeof(szTmpCatalog) / sizeof(szTmpCatalog[0]))) {
						strncpy(pDisc->SUB.szCatalog, szCatalog, sizeof(pDisc->SUB.szCatalog) / sizeof(pDisc->SUB.szCatalog[0]));
						pDisc->SUB.byCatalog = (BYTE)bMCN;
						OutputCDSub96Align(lpSubcode, nLBA);
						OutputDiscLogA("\tMCN: [%s]\n", szCatalog);
						WriteCcdForDiscCatalog(pDisc, fpCcd);
					}
				}
			}
		}
		else if (byAdr == ADR_ENCODES_ISRC) {
			BOOL bISRC = IsValidSubQISRC(lpSubcode);
			if (!bISRC && bCRC) {
				// force a invalid ISRC to valid ISRC
				bISRC = bCRC;
			}
			if (bISRC && bCRC) {
				nTmpISRCLBAList[nISRCIdx++] = nLBA;
				CHAR szISRC[META_ISRC_SIZE] = { 0 };
				if (!bCheckISRC) {
					SetISRCToString(pDisc, lpSubcode, szISRC, byIdxOfTrack, FALSE);
					strncpy(szTmpISRC, szISRC, sizeof(szTmpISRC) / sizeof(szTmpISRC[0]));
					szTmpISRC[META_ISRC_SIZE - 1] = 0;
					bCheckISRC = bISRC;
				}
				else if (!pDisc->SUB.lpISRCList[byIdxOfTrack]) {
					SetISRCToString(pDisc, lpSubcode, szISRC, byIdxOfTrack, FALSE);
					if (!strncmp(szTmpISRC, szISRC, sizeof(szISRC) / sizeof(szISRC[0]))) {
						strncpy(pDisc->SUB.pszISRC[byIdxOfTrack], szISRC, META_ISRC_SIZE);
						pDisc->SUB.lpISRCList[byIdxOfTrack] = bISRC;
						OutputCDSub96Align(lpSubcode, nLBA);
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
		CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	BOOL bC2 = FALSE;
	if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
		CDB::_PLXTR_READ_CDDA cdb = { 0 };
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::MainC2Raw);
			bC2 = TRUE;
		}
		else {
			SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::MainPack);
		}
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	else {
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(NULL, pDevice, &cdb, CDFLAG::_READ_CD::All, 
			1, CDFLAG::_READ_CD::NoC2, CDFLAG::_READ_CD::Raw, TRUE);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}

	for (BYTE i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
		try {
			INT nTmpLBA = pDisc->SCSI.lpFirstLBAListOnToc[i] + 100;
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, nTmpLBA, lpBuf,
				CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
			BYTE lpSubcodeOrg[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
			if (bC2) {
				AlignRowSubcode(lpBuf + CD_RAW_SECTOR_WITH_C2_294_SIZE, lpSubcode);
				memcpy(lpSubcodeOrg, lpBuf + CD_RAW_SECTOR_WITH_C2_294_SIZE, CD_RAW_READ_SUBCODE_SIZE);
			}
			else {
				AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
				memcpy(lpSubcodeOrg, lpBuf + CD_RAW_SECTOR_SIZE, CD_RAW_READ_SUBCODE_SIZE);
			}
			OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_TRACK_F(Check CD+G), lpCmd[0], lpCmd[10], i + 1);
			OutputCDSub96Align(lpSubcode, nTmpLBA);

			SUB_R_TO_W scRW[4] = { 0 };
			BYTE tmpCode[24] = { 0 };
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
	BYTE byTransferLen = 1;
	DWORD dwSize = DISC_RAW_READ_SIZE;
	SetCommandForTransferLength(pExecType, pDevice, pCdb, dwSize, &byTransferLen);
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
					SetCommandForTransferLength(pExecType, pDevice, pCdb, (DWORD)pIDh->e_lfanew, &byTransferLen);
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

BOOL ReadCDFor3DODirectory(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb,
	LPCH pPath,
	INT nLBA
) {
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		DISC_RAW_READ_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	try {
		if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)pCdb, nLBA, lpBuf,
			DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		LONG lOfs = THREEDO_DIR_HEADER_SIZE;
		LONG lDirSize =
			MAKELONG(MAKEWORD(lpBuf[15], lpBuf[14]), MAKEWORD(lpBuf[13], lpBuf[12]));
		OutputFs3doDirectoryRecord(lpBuf, nLBA, pPath, lDirSize);

		// next dir
		CHAR szNewPath[_MAX_PATH] = { 0 };
		CHAR fname[32] = { 0 };
		while (lOfs < lDirSize) {
			LPBYTE lpDirEnt = lpBuf + lOfs;
			LONG lFlags = MAKELONG(
				MAKEWORD(lpDirEnt[3], lpDirEnt[2]), MAKEWORD(lpDirEnt[1], lpDirEnt[0]));
			strncpy(fname, (LPCH)&lpDirEnt[32], sizeof(fname));
			LONG lastCopy = MAKELONG(
				MAKEWORD(lpDirEnt[67], lpDirEnt[66]), MAKEWORD(lpDirEnt[65], lpDirEnt[64]));
			lOfs += THREEDO_DIR_ENTRY_SIZE;

			if ((lFlags & 0xff) == 7) {
				sprintf(szNewPath, "%s%s/", pPath, fname);
				if (!ReadCDFor3DODirectory(pExtArg, pDevice, pDisc, pCdb, szNewPath,
					MAKELONG(MAKEWORD(lpDirEnt[71], lpDirEnt[70]), MAKEWORD(lpDirEnt[69], lpDirEnt[68])))) {
					throw FALSE;
				}
			}
			for (LONG i = 0; i < lastCopy; i++) {
				lOfs += sizeof(LONG);
			}
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadCDForFileSystem(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	BOOL bRet = TRUE;
	for (BYTE i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
		if ((pDisc->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			// for Label Gate CD, XCP
			if (i > 1 && pDisc->SCSI.lpLastLBAListOnToc[i] - pDisc->SCSI.lpFirstLBAListOnToc[i] + 1 <= 750) {
				return TRUE;
			}
			LPBYTE pBuf = NULL;
			LPBYTE lpBuf = NULL;
			if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
				pDevice->dwMaxTransferLength, &lpBuf, _T(__FUNCTION__), __LINE__)) {
				return FALSE;
			}
			CDB::_READ12 cdb = { 0 };
			cdb.OperationCode = SCSIOP_READ12;
			cdb.LogicalUnitNumber = pDevice->address.Lun;
			cdb.TransferLength[3] = 1;
			BOOL bVD = FALSE;
			PDIRECTORY_RECORD pDirRec = NULL;
			try {
				// general data track disc
				VOLUME_DESCRIPTOR volDesc;
				if (!ReadVolumeDescriptor(pExecType, pExtArg, pDevice, pDisc, i, (LPBYTE)&cdb, lpBuf, 16, &bVD, &volDesc)) {
					throw FALSE;
				}
				if (bVD) {
					pDirRec = (PDIRECTORY_RECORD)calloc(DIRECTORY_RECORD_SIZE, sizeof(DIRECTORY_RECORD));
					if (!pDirRec) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
					INT nDirPosNum = 0;
					if (!ReadPathTableRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb
						, volDesc.ISO_9660.dwLogicalBlkCoef, volDesc.ISO_9660.dwPathTblSize
						, volDesc.ISO_9660.dwPathTblPos, pDirRec, &nDirPosNum)) {
						throw FALSE;
					}
					if (!ReadDirectoryRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb, lpBuf
						, volDesc.ISO_9660.dwLogicalBlkCoef, volDesc.ISO_9660.dwRootDataLen, pDirRec, nDirPosNum)) {
						OutputVolDescLogA("Failed to read ISO9660\n");
						nDirPosNum = 0;
						if (!ReadPathTableRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb
							, volDesc.JOLIET.dwLogicalBlkCoef, volDesc.JOLIET.dwPathTblSize
							, volDesc.JOLIET.dwPathTblPos, pDirRec, &nDirPosNum)) {
							throw FALSE;
						}
						if (!ReadDirectoryRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb, lpBuf
							, volDesc.JOLIET.dwLogicalBlkCoef, volDesc.JOLIET.dwRootDataLen, pDirRec, nDirPosNum)) {
							throw FALSE;
						}
					}
					if (!ReadCDForCheckingExe(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb, lpBuf)) {
						throw FALSE;
					}
					if (pDisc->PROTECT.byExist) {
						OutputLogA(standardOut | fileDisc, "Detected [%s], Skip error from %d to %d"
							, pDisc->PROTECT.name, pDisc->PROTECT.ERROR_SECTOR.nExtentPos
							, pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize);
						if (pDisc->PROTECT.byExist == microids) {
							OutputLogA(standardOut | fileDisc, ", %d to %d\n"
								, pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd
								, pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd + pDisc->PROTECT.ERROR_SECTOR.nSectorSize2nd);
						}
						else {
							OutputLogA(standardOut | fileDisc, "\n");
						}
					}
				}
				else {
					BOOL bOtherHeader = FALSE;
					// for pce, pc-fx
					INT nLBA = pDisc->SCSI.nFirstLBAofDataTrack;
					if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
						DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
						throw FALSE;
					}
					if (IsValidPceSector(lpBuf)) {
						OutputFsPceStuff(lpBuf, nLBA);
						nLBA = pDisc->SCSI.nFirstLBAofDataTrack + 1;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						OutputFsPceBootSector(lpBuf, nLBA);
						bOtherHeader = TRUE;
					}
					else if (IsValidPcfxSector(lpBuf)) {
						OutputFsPcfxHeader(lpBuf, nLBA);
						nLBA = pDisc->SCSI.nFirstLBAofDataTrack + 1;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						OutputFsPcfxSector(lpBuf, nLBA);
						bOtherHeader = TRUE;
					}

					if (!bOtherHeader) {
						// for 3DO
						nLBA = 0;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						if (IsValid3doDataHeader(lpBuf)) {
							OutputFs3doHeader(lpBuf, nLBA);
							if (!ReadCDFor3DODirectory(pExtArg, pDevice, pDisc, &cdb, "/",
								MAKELONG(MAKEWORD(lpBuf[103], lpBuf[102]),
								MAKEWORD(lpBuf[101], lpBuf[100])))) {
								throw FALSE;
							}
							bOtherHeader = TRUE;
						}
					}
					if (!bOtherHeader) {
						// for MAC pattern 1
						nLBA = 1;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						if (IsValidMacDataHeader(lpBuf + 1024)) {
							OutputFsMasterDirectoryBlocks(lpBuf + 1024, nLBA);
							bOtherHeader = TRUE;
						}
						else if (IsValidMacDataHeader(lpBuf + 512)) {
							OutputFsMasterDirectoryBlocks(lpBuf + 512, nLBA);
							bOtherHeader = TRUE;
						}
						// for MAC pattern 2
						nLBA = 16;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						if (IsValidMacDataHeader(lpBuf + 1024)) {
							OutputFsMasterDirectoryBlocks(lpBuf + 1024, nLBA);
							bOtherHeader = TRUE;
						}
					}
					if (bOtherHeader) {
						FreeAndNull(pBuf);
						break;
					}
				}
			}
			catch (BOOL bErr) {
				bRet = bErr;
			}
			FreeAndNull(pDirRec);
			FreeAndNull(pBuf);
		}
	}
	return bRet;
}

BOOL ReadCDForScanningProtectViaSector(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
		CDB::_PLXTR_READ_CDDA cdb = { 0 };
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::MainC2Raw);
		}
		else {
			SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::MainPack);
		}
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	else {
		// non plextor && support scrambled ripping
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(pExtArg, pDevice, &cdb, CDFLAG::_READ_CD::CDDA
			, 1, CDFLAG::_READ_CD::byte294, CDFLAG::_READ_CD::NoSub, TRUE);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	
	BYTE aBuf[CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE] = { 0 };
	BYTE byScsiStatus = 0;
	for (INT nLBA = 0; nLBA < pDisc->SCSI.nAllLength; nLBA++) {
		if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, aBuf,
			CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
		INT nOfs = pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE;
		if (aBuf[nOfs] == 0 && aBuf[nOfs + 1] == 0xff && aBuf[nOfs + 2] == 0 && aBuf[nOfs + 3] == 0xff &&
			aBuf[nOfs + 4] == 0 && aBuf[nOfs + 5] == 0xff && aBuf[nOfs + 6] == 0 && aBuf[nOfs + 7] == 0xff &&
			aBuf[nOfs + 8] == 0 && aBuf[nOfs + 9] == 0xff && aBuf[nOfs + 10] == 0 && aBuf[nOfs + 11] == 0xff) {
			OutputLogA(standardOut | fileDisc, "\nDetected ProtectCD VOB");
			pDisc->PROTECT.byExist = protectCDVOB;
			pExtArg->byScanProtectViaFile = pExtArg->byScanProtectViaSector;
			break;
		}
		OutputString(_T("\rScanning sector (LBA) %6d/%6d"), nLBA, pDisc->SCSI.nAllLength - 1);
	}
	OutputString(_T("\n"));

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
	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
		CDB::_PLXTR_READ_CDDA cdb = { 0 };
		SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::MainC2Raw);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	else {
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(pExtArg, pDevice, &cdb
			, CDFLAG::_READ_CD::All, 1, c2, sub, FALSE);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	BOOL bRet = TRUE;
	if (!ExecReadCD(pExtArg, pDevice, lpCmd, 0, lpBuf
		, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
		OutputLogA(standardError | fileDrive,
			"This drive doesn't support [OpCode: %#02x, C2flag: %x, SubCode: %x]\n"
			, lpCmd[0], (lpCmd[9] & 0x6) >> 1, lpCmd[10]);
		bRet = FALSE;
	}
	else {
		OutputDriveLogA(OUTPUT_DHYPHEN_PLUS_STR(Check main+c2+sub));
		OutputCDC2Error296(fileDrive, lpBuf + CD_RAW_SECTOR_SIZE, 0);
		OutputCDSub96Raw(fileDrive, lpBuf + CD_RAW_SECTOR_WITH_C2_294_SIZE, 0);

		OutputDriveLogA(OUTPUT_DHYPHEN_PLUS_STR(Check main+sub+c2));
		OutputCDSub96Raw(fileDrive, lpBuf + CD_RAW_SECTOR_SIZE, 0);
		OutputCDC2Error296(fileDrive, lpBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE, 0);

		BYTE subcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
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
		_T("BufLen %lubyte, BufC2Offset %lubyte, BufSubOffset %lubyte\n"),
		pDevice->TRANSFER.dwBufLen, pDevice->TRANSFER.dwBufC2Offset, pDevice->TRANSFER.dwBufSubOffset);
#endif
}

BOOL ExecReadCDForC2(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPBYTE lpCmd,
	INT nLBA,
	LPBYTE lpBuf,
	LPCTSTR pszFuncName,
	LONG lLineNum
) {
	REVERSE_BYTES(&lpCmd[2], &nLBA);
	BYTE byTransferLen = lpCmd[9];
	if (lpCmd[0] != 0xd8) {
		byTransferLen = lpCmd[8];
	}
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB12GENERIC_LENGTH, lpBuf,
		pDevice->TRANSFER.dwBufLen * byTransferLen, &byScsiStatus, pszFuncName, lLineNum)) {
		if (pExtArg->byScanProtectViaFile) {
			return RETURNED_CONTINUE;
		}
		else {
			return RETURNED_FALSE;
		}
	}
	if (byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		if (*pExecType != gd) {
			return RETURNED_CONTINUE;
		}
		else {
			return RETURNED_FALSE;
		}
	}
	return RETURNED_NO_C2_ERROR_1ST;
}

// http://tmkk.undo.jp/xld/secure_ripping.html
// https://forum.dbpoweramp.com/showthread.php?33676
BOOL FlushDriveCache(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	INT nLBA
) {
	if (nLBA < 450000 && nLBA % pExtArg->dwCacheDelNum == 0) {
		CDB::_READ12 cdb = { 0 };
		cdb.OperationCode = SCSIOP_READ12;
		cdb.ForceUnitAccess = TRUE;
		cdb.LogicalUnitNumber = pDevice->address.Lun;
		INT NextLBAAddress = nLBA + 1;
		REVERSE_BYTES(&cdb.LogicalBlock, &NextLBAAddress);
		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pExtArg, pDevice, (LPBYTE)&cdb, CDB12GENERIC_LENGTH,
			NULL, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL ProcessReadCD(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	LPBYTE lpCmd,
	INT nLBA
) {
	BOOL bRet = RETURNED_NO_C2_ERROR_1ST;
	if (*pExecType != gd && !pExtArg->byRawDump && pDevice->bySuccessReadTocFull) {
		if (pDisc->SCSI.nFirstLBAof2ndSession != -1) {
			if (pExtArg->byReverse) {
				if (pDisc->SCSI.nFirstLBAof2ndSession == nLBA + 1) {
					OutputMainInfoLogA(
						"Skip from Leadout of Session 1 [%d, %#x] to Leadin of Session 2 [%d, %#x]\n",
						pDisc->SCSI.nFirstLBAofLeadout, pDisc->SCSI.nFirstLBAofLeadout,
						pDisc->SCSI.nFirstLBAof2ndSession - 1, pDisc->SCSI.nFirstLBAof2ndSession - 1);
					pDiscPerSector->subQ.prev.nAbsoluteTime = nLBA - SESSION_TO_SESSION_SKIP_LBA - 150;
					return RETURNED_SKIP_LBA;
				}
			}
			else {
				if (pDisc->MAIN.nFixFirstLBAofLeadout == nLBA) {
					OutputMainInfoLogA(
						"Skip from Leadout of Session 1 [%d, %#x] to Leadin of Session 2 [%d, %#x]\n",
						pDisc->SCSI.nFirstLBAofLeadout, pDisc->SCSI.nFirstLBAofLeadout,
						pDisc->SCSI.nFirstLBAof2ndSession - 1, pDisc->SCSI.nFirstLBAof2ndSession - 1);
					if (pDisc->MAIN.nCombinedOffset > 0) {
						pDiscPerSector->subQ.prev.nAbsoluteTime =
							nLBA + SESSION_TO_SESSION_SKIP_LBA + 150 - pDisc->MAIN.nAdjustSectorNum - 1;
					}
					else if (pDisc->MAIN.nCombinedOffset < 0) {
						pDiscPerSector->subQ.prev.nAbsoluteTime =
							nLBA + SESSION_TO_SESSION_SKIP_LBA + 150 + pDisc->MAIN.nAdjustSectorNum;
					}
					return RETURNED_SKIP_LBA;
				}
			}
		}
	}
	if (pExtArg->byFua) {
		FlushDriveCache(pExtArg, pDevice, nLBA);
	}
	bRet = ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd, nLBA,
		pDiscPerSector->data.present, _T(__FUNCTION__), __LINE__);

	if (pDevice->byPlxtrDrive) {
		if (bRet == RETURNED_NO_C2_ERROR_1ST) {
			AlignRowSubcode(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.present);
			if (!(pExtArg->byScanProtectViaFile && pDisc->PROTECT.byExist &&
				(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
					nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize) ||
					(pDisc->PROTECT.byExist == microids && pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd <= nLBA &&
					nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd + pDisc->PROTECT.ERROR_SECTOR.nSectorSize2nd))) {

				if (pDiscPerSector->data.next != NULL && 1 <= pExtArg->dwSubAddionalNum) {
					memcpy(pDiscPerSector->data.next, pDiscPerSector->data.present + pDevice->TRANSFER.dwBufLen, pDevice->TRANSFER.dwBufLen);
					AlignRowSubcode(pDiscPerSector->data.next + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.next);
					
					if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
						bRet = ContainsC2Error(pDevice, pDiscPerSector->data.next);
					}
					if (pDiscPerSector->data.nextNext != NULL && 2 <= pExtArg->dwSubAddionalNum) {
						ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd,
							nLBA + 2, pDiscPerSector->data.nextNext, _T(__FUNCTION__), __LINE__);
						AlignRowSubcode(pDiscPerSector->data.nextNext + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.nextNext);
					}
				}
			}
			else {
				// replace sub to sub of prev
				ZeroMemory(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, CD_RAW_READ_SUBCODE_SIZE);
				ZeroMemory(pDiscPerSector->subcode.present, CD_RAW_READ_SUBCODE_SIZE);
				SetBufferFromTmpSubQData(pDiscPerSector->subQ.prev, pDiscPerSector->subcode.present, 0);
				if (pDiscPerSector->data.next != NULL && 1 <= pExtArg->dwSubAddionalNum) {
					SetBufferFromTmpSubQData(pDiscPerSector->subQ.present, pDiscPerSector->subcode.next, 0);
					if (pDiscPerSector->data.nextNext != NULL && 2 <= pExtArg->dwSubAddionalNum) {
						SetBufferFromTmpSubQData(pDiscPerSector->subQ.next, pDiscPerSector->subcode.nextNext, 0);
					}
				}
			}
		}
	}
	else {
		if (bRet == RETURNED_NO_C2_ERROR_1ST) {
			AlignRowSubcode(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.present);
#if 1
			if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
				if (pExtArg->byScanProtectViaFile && pDisc->PROTECT.byExist &&
					(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
						nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize)) {
					// skip check c2 error
					ZeroMemory(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufC2Offset, CD_RAW_READ_C2_294_SIZE);
				}
				else {
					bRet = ContainsC2Error(pDevice, pDiscPerSector->data.present);
				}
			}
#endif
			if (!(pExtArg->byScanProtectViaFile && pDisc->PROTECT.byExist &&
				(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
					nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize))) {
				if (pDiscPerSector->data.next != NULL && 1 <= pExtArg->dwSubAddionalNum) {
					ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd,
						nLBA + 1, pDiscPerSector->data.next, _T(__FUNCTION__), __LINE__);
					AlignRowSubcode(pDiscPerSector->data.next + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.next);

					if (pDiscPerSector->data.nextNext != NULL && 2 <= pExtArg->dwSubAddionalNum) {
						ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd,
							nLBA + 2, pDiscPerSector->data.nextNext, _T(__FUNCTION__), __LINE__);
						AlignRowSubcode(pDiscPerSector->data.nextNext + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.nextNext);
					}
				}
			}
		}
		if (pExtArg->byScanProtectViaFile && pDisc->PROTECT.byExist &&
			(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
				nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize)) {
#if 1
			if (bRet == RETURNED_CONTINUE) {
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					// skip check c2 error
					ZeroMemory(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufC2Offset, CD_RAW_READ_C2_294_SIZE);
				}
			}
#endif
			// replace sub to sub of prev
			ZeroMemory(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, CD_RAW_READ_SUBCODE_SIZE);
			ZeroMemory(pDiscPerSector->subcode.present, CD_RAW_READ_SUBCODE_SIZE);
			SetBufferFromTmpSubQData(pDiscPerSector->subQ.prev, pDiscPerSector->subcode.present, 0);
			if (pDiscPerSector->data.next != NULL && 1 <= pExtArg->dwSubAddionalNum) {
				SetBufferFromTmpSubQData(pDiscPerSector->subQ.present, pDiscPerSector->subcode.next, 0);
				if (pDiscPerSector->data.nextNext != NULL && 2 <= pExtArg->dwSubAddionalNum) {
					SetBufferFromTmpSubQData(pDiscPerSector->subQ.next, pDiscPerSector->subcode.nextNext, 0);
				}
			}
		}
	}
	return bRet;
}

BOOL ReadCDForRereadingSectorType1(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpCmd,
	FILE* fpImg,
	FILE* fpC2
) {
	LPBYTE lpBuf = NULL;
	if (NULL == (lpBuf = (LPBYTE)calloc(CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * 2, sizeof(BYTE)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	try {
		CDB::_PLXTR_READ_CDDA cdb = { 0 };
		SetReadD8Command(pDevice, &cdb, 2, CDFLAG::_PLXTR_READ_CDDA::MainC2Raw);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);

		for (INT m = 0; m < pDisc->MAIN.nC2ErrorCnt; m++) {
			INT nLBA = pDisc->MAIN.lpAllLBAOfC2Error[m];
			for (DWORD i = 0; i < pExtArg->dwMaxRereadNum; i++) {
				OutputString(_T("\rNeed to reread sector: %6d rereading times: %4ld/%4ld")
					, nLBA, i + 1, pExtArg->dwMaxRereadNum);
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, lpBuf
					, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * 2, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				DWORD dwTmpCrc32 = 0;
				GetCrc32(&dwTmpCrc32, lpBuf, CD_RAW_SECTOR_SIZE);
//				OutputC2ErrorWithLBALogA("to [%06d] crc32[%03ld]: 0x%08lx "
//					, nLBA - pDisc->MAIN.nOffsetStart - 1, nLBA - pDisc->MAIN.nOffsetStart, i, dwTmpCrc32);
				OutputC2ErrorWithLBALogA("crc32[%03ld]: 0x%08lx ", nLBA, i, dwTmpCrc32);

				LPBYTE lpNextBuf = lpBuf + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
				if (ContainsC2Error(pDevice, lpNextBuf) == RETURNED_NO_C2_ERROR_1ST) {
					LONG lSeekMain = CD_RAW_SECTOR_SIZE * (LONG)nLBA - pDisc->MAIN.nCombinedOffset;
					fseek(fpImg, lSeekMain, SEEK_SET);
					// Write track to scrambled again
					WriteMainChannel(pExtArg, pDisc, lpBuf, nLBA, fpImg);
					LONG lSeekC2 = CD_RAW_READ_C2_294_SIZE * (LONG)nLBA - (pDisc->MAIN.nCombinedOffset / 8);
					fseek(fpC2, lSeekC2, SEEK_SET);
					WriteC2(pExtArg, pDisc, lpNextBuf + pDevice->TRANSFER.dwBufC2Offset, nLBA, fpC2);
					OutputC2ErrorLogA("good. Rewrote .scm[%ld-%ld(%lx-%lx)] .c2[%ld-%ld(%lx-%lx)]\n"
						, lSeekMain, lSeekMain + 2351, lSeekMain, lSeekMain + 2351
						, lSeekC2, lSeekC2 + 293, lSeekC2, lSeekC2 + 293);
					break;
				}
				else {
					if (i == pExtArg->dwMaxRereadNum - 1) {
						OutputLogA(standardError | fileC2Error, "\nbad all. need to reread more\n");
						throw FALSE;
					}
					else {
						OutputC2ErrorLogA("bad\n");
					}
				}
				if (!FlushDriveCache(pExtArg, pDevice, nLBA)) {
					throw FALSE;
				}
			}
		}
		OutputString("\nDone. See _c2Error.txt\n");
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(lpBuf);
	return bRet;
}

BOOL ReadCDForRereadingSectorType2(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpCmd,
	FILE* fpImg,
	FILE* fpC2,
	INT nStartLBA,
	INT nEndLBA
) {
	BOOL bRet = TRUE;
	DWORD dwTransferLen = pDevice->dwMaxTransferLength / CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
	DWORD dwTransferLenBak = dwTransferLen;
	LPBYTE lpBufMain = NULL;
	if (NULL == (lpBufMain = (LPBYTE)calloc(dwTransferLen * CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * pExtArg->dwMaxRereadNum, sizeof(BYTE)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE lpBufC2 = NULL;
	LPBYTE* lpRereadSector = NULL;
	LPDWORD* lpCrc32RereadSector = NULL;
	LPDWORD* lpRepeatedNum = NULL;
	LPDWORD* lpContainsC2 = NULL;
	try {
		if (NULL == (lpBufC2 = (LPBYTE)calloc(dwTransferLen * CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * pExtArg->dwMaxRereadNum, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpRereadSector = (LPBYTE*)calloc(dwTransferLen, sizeof(ULONG_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpCrc32RereadSector = (LPDWORD*)calloc(dwTransferLen, sizeof(ULONG_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpRepeatedNum = (LPDWORD*)calloc(dwTransferLen, sizeof(ULONG_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpContainsC2 = (LPDWORD*)calloc(dwTransferLen, sizeof(ULONG_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		for (DWORD a = 0; a < dwTransferLen; a++) {
			if (NULL == (lpRereadSector[a] = (LPBYTE)calloc(CD_RAW_SECTOR_SIZE * pExtArg->dwMaxRereadNum, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (lpCrc32RereadSector[a] = (LPDWORD)calloc(pExtArg->dwMaxRereadNum, sizeof(DWORD)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (lpRepeatedNum[a] = (LPDWORD)calloc(pExtArg->dwMaxRereadNum, sizeof(DWORD)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (lpContainsC2[a] = (LPDWORD)calloc(pExtArg->dwMaxRereadNum, sizeof(DWORD)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}

		CDB::_PLXTR_READ_CDDA cdb = { 0 };
		SetReadD8Command(pDevice, &cdb, dwTransferLen, CDFLAG::_PLXTR_READ_CDDA::MainC2Raw);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);

		INT nLBA = nStartLBA;
		INT nLastLBA = nEndLBA;

		while (nLBA < nLastLBA) {
			OutputString(_T("\rRewrited img (LBA) %6d/%6d"), nLBA, pDisc->SCSI.nAllLength - 1);
			INT idx = 0;
			if (nLastLBA - nLBA < (INT)dwTransferLen) {
				dwTransferLen = (DWORD)(nLastLBA - nLBA);
				lpCmd[9] = (BYTE)dwTransferLen;
			}
			for (DWORD i = 0; i < pExtArg->dwMaxRereadNum; i++) {
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, lpBufMain
					, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * dwTransferLen, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA + 1, lpBufC2
					, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * dwTransferLen, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				for (DWORD k = 0; k < dwTransferLen; k++) {
					OutputC2ErrorWithLBALogA("crc32", nLBA - pDisc->MAIN.nOffsetStart + (INT)k);
					DWORD dwTmpCrc32 = 0;
					GetCrc32(&dwTmpCrc32, lpBufMain + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * k, CD_RAW_SECTOR_SIZE);

					BOOL bMatch = FALSE;
					BOOL bC2 = RETURNED_NO_C2_ERROR_1ST;
					for (WORD wC2ErrorPos = 0; wC2ErrorPos < CD_RAW_READ_C2_294_SIZE; wC2ErrorPos++) {
						INT nBit = 0x80;
						DWORD dwPos = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * k + pDevice->TRANSFER.dwBufC2Offset + wC2ErrorPos;
						for (INT n = 0; n < CHAR_BIT; n++) {
							// exist C2 error
							if (lpBufC2[dwPos] & nBit) {
								bC2 = RETURNED_EXIST_C2_ERROR;
								break;
							}
							nBit >>= 1;
						}
						if (bC2 == RETURNED_EXIST_C2_ERROR) {
							break;
						}
					}
					for (DWORD j = 0; j <= i; j++) {
						if (dwTmpCrc32 == lpCrc32RereadSector[k][j]) {
							OutputC2ErrorLogA("[%03ld]:0x%08lx, %d ", i, dwTmpCrc32, bC2);
							lpRepeatedNum[k][j] += 1;
							lpContainsC2[k][j] += bC2;
							bMatch = TRUE;
							break;
						}
					}
					if (!bMatch) {
						lpCrc32RereadSector[k][idx] = dwTmpCrc32;
						memcpy(&lpRereadSector[k][CD_RAW_SECTOR_SIZE * idx]
							, lpBufMain + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * k, CD_RAW_SECTOR_SIZE);
						OutputC2ErrorLogA("[%03d]:0x%08lx, %d ", idx, dwTmpCrc32, bC2);
#if 0
						if (idx > 0) {
							OutputCDMain(fileC2Error, &lpRereadSector[k][CD_RAW_SECTOR_SIZE * idx]
								, nLBA - pDisc->MAIN.nOffsetStart + (INT)k, CD_RAW_SECTOR_SIZE);
						}
#endif
					}
				}
				OutputC2ErrorLogA("\n");
				idx++;
				if (!FlushDriveCache(pExtArg, pDevice, nLBA)) {
					throw FALSE;
				}
			}

			for (DWORD q = 0; q < dwTransferLen; q++) {
				DWORD dwMaxNum = lpRepeatedNum[q][0];
				DWORD dwMaxC2 = lpContainsC2[q][0];
				for (INT k = 0; k < idx - 1; k++) {
					dwMaxNum = max(dwMaxNum, lpRepeatedNum[q][k + 1]);
					dwMaxC2 = max(dwMaxC2, lpContainsC2[q][k + 1]);
				}

				INT nTmpLBA = nLBA - pDisc->MAIN.nOffsetStart + (INT)q;
				if (dwMaxC2 == 0) {
					OutputC2ErrorWithLBALogA(
						"to[%06d] All crc32 is probably bad. No rewrite\n", nTmpLBA - 1, nTmpLBA);
				}
				else if (dwMaxNum + 1 == pExtArg->dwMaxRereadNum &&
					lpCrc32RereadSector[q][0] == pDisc->MAIN.lpAllSectorCrc32[nTmpLBA]) {
					OutputC2ErrorWithLBALogA(
						"to[%06d] All same crc32. No rewrite\n", nTmpLBA - 1, nTmpLBA);
				}
				else {
					for (INT l = 0; l < idx; l++) {
						if (dwMaxC2 == lpContainsC2[q][l]) {
							OutputC2ErrorWithLBALogA(
								"to[%06d] crc32[%d]:0x%08lx, no c2 %lu times. Rewrite\n"
								, nTmpLBA - 1, nTmpLBA, l, lpCrc32RereadSector[q][l], dwMaxC2 + 1);
							fseek(fpImg, CD_RAW_SECTOR_SIZE * (LONG)(nLBA + q) - pDisc->MAIN.nCombinedOffset, SEEK_SET);
							// Write track to scrambled again
							WriteMainChannel(pExtArg, pDisc, &lpRereadSector[q][CD_RAW_SECTOR_SIZE * l], nLBA, fpImg);
							fseek(fpC2, CD_RAW_READ_C2_294_SIZE * (LONG)nLBA - (pDisc->MAIN.nCombinedOffset / 8), SEEK_SET);
							if (q - 1 < dwTransferLen) {
								WriteC2(pExtArg, pDisc, &lpRereadSector[q + 1][CD_RAW_SECTOR_SIZE * l] + pDevice->TRANSFER.dwBufC2Offset, nLBA, fpC2);
							}
#if 0
							OutputC2ErrorLogA("Seek to %ld (0x%08lx)\n"
								, CD_RAW_SECTOR_SIZE * (LONG)(nLBA + q) - pDisc->MAIN.nCombinedOffset
								, CD_RAW_SECTOR_SIZE * (LONG)(nLBA + q) - pDisc->MAIN.nCombinedOffset);
							OutputCDMain(fileC2Error, &lpRereadSector[q][CD_RAW_SECTOR_SIZE * l], nTmpLBA, CD_RAW_SECTOR_SIZE);
							OutputC2ErrorLogA("\n");
#endif
							break;
						}
					}
				}
			}
			nLBA += dwTransferLen;
			for (DWORD r = 0; r < dwTransferLen; r++) {
				for (INT p = 0; p < idx; p++) {
					lpRereadSector[r][p] = 0;
					lpCrc32RereadSector[r][p] = 0;
					lpRepeatedNum[r][p] = 0;
					lpContainsC2[r][p] = 0;
				}
			}
		}
		OutputLogA(standardOut | fileC2Error, "\n");
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(lpBufMain);
	FreeAndNull(lpBufC2);
	for (DWORD r = 0; r < dwTransferLenBak; r++) {
		FreeAndNull(lpRereadSector[r]);
		FreeAndNull(lpCrc32RereadSector[r]);
		FreeAndNull(lpRepeatedNum[r]);
		FreeAndNull(lpContainsC2[r]);
	}
	FreeAndNull(lpRereadSector);
	FreeAndNull(lpCrc32RereadSector);
	FreeAndNull(lpRepeatedNum);
	FreeAndNull(lpContainsC2);

	return bRet;
}

VOID ExecEccEdc(
	BYTE byScanProtectViaFile,
	_DISC::_PROTECT protect,
	LPCTSTR pszImgPath,
	_DISC::_PROTECT::_ERROR_SECTOR errorSector
) {
	CONST INT nCmdSize = 6;
	CONST INT nStrSize = _MAX_PATH * 2 + nCmdSize;
	_TCHAR str[nStrSize] = { 0 };
	_TCHAR cmd[nCmdSize] = { _T("check") };
	INT nStartLBA = errorSector.nExtentPos;
	INT nEndLBA = errorSector.nExtentPos + errorSector.nSectorSize;
	if (byScanProtectViaFile) {
		_tcsncpy(cmd, _T("fix"), sizeof(cmd) / sizeof(cmd[0]));
	}
	if (GetEccEdcCmd(str, nStrSize, cmd, pszImgPath, nStartLBA, nEndLBA)) {
		OutputString(_T("Exec %s\n"), str);
		_tsystem(str);
	}
	if (protect.byExist == microids) {
		nStartLBA = errorSector.nExtentPos2nd;
		nEndLBA = errorSector.nExtentPos2nd + errorSector.nSectorSize2nd;
		if (GetEccEdcCmd(str, nStrSize, cmd, pszImgPath, nStartLBA, nEndLBA)) {
			OutputString(_T("Exec %s\n"), str);
			_tsystem(str);
		}
	}
}

VOID ProcessReturnedContinue(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA,
	INT nMainDataType,
	BYTE byCurrentTrackNum,
	FILE* fpImg,
	FILE* fpSub,
	FILE* fpC2,
	FILE* fpParse
) {
#if 1
	if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		OutputCDMain(fileMainError,
			pDiscPerSector->mainHeader.present, nLBA, MAINHEADER_MODE1_SIZE);
	}
#endif
	UpdateTmpMainHeader(&pDiscPerSector->mainHeader,
		(LPBYTE)&pDiscPerSector->mainHeader.prev, pDiscPerSector->subQ.prev.byCtl, nMainDataType);
	WriteErrorBuffer(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector,
		scrambled_table, nLBA, byCurrentTrackNum, fpImg, fpSub, fpC2, fpParse);
#if 1
	if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		if (pExtArg->byBe) {
			OutputCDMain(fileMainError,
				pDiscPerSector->mainHeader.present, nLBA, MAINHEADER_MODE1_SIZE);
		}
		else {
			OutputCDMain(fileMainError,
				pDiscPerSector->data.present + pDisc->MAIN.uiMainDataSlideSize, nLBA, MAINHEADER_MODE1_SIZE);
		}
	}
#endif
	if (pDiscPerSector->subQ.prev.byIndex == 0) {
		pDiscPerSector->subQ.prev.nRelativeTime--;
	}
	else {
		pDiscPerSector->subQ.prev.nRelativeTime++;
	}
	pDiscPerSector->subQ.prev.nAbsoluteTime++;
}

BOOL ReadCDForCheckingSecuROM(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	LPBYTE lpCmd
) {
#ifdef _DEBUG
	WORD w = (WORD)GetCrc16CCITT(10, &pDiscPerSector->subcode.present[12]);
	OutputSubInfoWithLBALogA(
		"CRC-16 is original:[%02x%02x], recalc:[%04x] and XORed with 0x8001:[%02x%02x]\n"
		, -1, 0, pDiscPerSector->subcode.present[22], pDiscPerSector->subcode.present[23]
		, w, pDiscPerSector->subcode.present[22] ^ 0x80, pDiscPerSector->subcode.present[23] ^ 0x01);
#endif
	if (pExtArg->byIntentionalSub && pDisc->PROTECT.byExist != securomV1 &&
		(pDiscPerSector->subcode.present[12] == 0x41 || pDiscPerSector->subcode.present[12] == 0x61)) {
		WORD crc16 = (WORD)GetCrc16CCITT(10, &pDiscPerSector->subcode.present[12]);
		WORD bufcrc = MAKEWORD(pDiscPerSector->subcode.present[23], pDiscPerSector->subcode.present[22]);
		INT nRLBA = MSFtoLBA(BcdToDec(pDiscPerSector->subcode.present[15])
			, BcdToDec(pDiscPerSector->subcode.present[16]), BcdToDec(pDiscPerSector->subcode.present[17]));
		INT nALBA = MSFtoLBA(BcdToDec(pDiscPerSector->subcode.present[19])
			, BcdToDec(pDiscPerSector->subcode.present[20]), BcdToDec(pDiscPerSector->subcode.present[21]));

		if (crc16 != bufcrc) {
			OutputSubInfoWithLBALogA(
				"Detected intentional error. CRC-16 is original:[%02x%02x] and XORed with 0x8001:[%02x%02x] "
				, -1, 0, pDiscPerSector->subcode.present[22] ^ 0x80, pDiscPerSector->subcode.present[23] ^ 0x01
				, pDiscPerSector->subcode.present[22], pDiscPerSector->subcode.present[23]);
			OutputSubInfoLogA(
				"RMSF[%02x:%02x:%02x] AMSF[%02x:%02x:%02x]\n"
				, pDiscPerSector->subcode.present[15], pDiscPerSector->subcode.present[16], pDiscPerSector->subcode.present[17]
				, pDiscPerSector->subcode.present[19], pDiscPerSector->subcode.present[20], pDiscPerSector->subcode.present[21]);

			OutputLogA(standardOut | fileDisc, "Detected intentional subchannel in LBA -1 => SecuROM Type 4 (a.k.a. NEW)\n");
			OutputIntentionalSubchannel(-1, &pDiscPerSector->subcode.present[12]);
			pDisc->PROTECT.byExist = securomV4;
			pDiscPerSector->subQ.prev.nRelativeTime = -1;
			pDiscPerSector->subQ.prev.nAbsoluteTime = 149;
		}
		else if ((nRLBA == 167295 || nRLBA == 0) && nALBA == 150) {
			OutputSubInfoWithLBALogA(
				"Detected shifted sub. RMSF[%02x:%02x:%02x] AMSF[%02x:%02x:%02x]\n"
				, -1, 0, pDiscPerSector->subcode.present[15], pDiscPerSector->subcode.present[16], pDiscPerSector->subcode.present[17]
				, pDiscPerSector->subcode.present[19], pDiscPerSector->subcode.present[20], pDiscPerSector->subcode.present[21]);

			OutputLogA(standardOut | fileDisc, "Detected intentional subchannel in LBA -1 => SecuROM Type 3 (a.k.a. NEW)\n");
			OutputIntentionalSubchannel(-1, &pDiscPerSector->subcode.present[12]);
			pDisc->PROTECT.byExist = securomV3;
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
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, 5000, pDiscPerSector->data.present,
				pDevice->TRANSFER.dwBufLen, _T(__FUNCTION__), __LINE__)) {
				return FALSE;
			}
			AlignRowSubcode(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.present);
			nRLBA = MSFtoLBA(BcdToDec(pDiscPerSector->subcode.present[15])
				, BcdToDec(pDiscPerSector->subcode.present[16]), BcdToDec(pDiscPerSector->subcode.present[17]));
			nALBA = MSFtoLBA(BcdToDec(pDiscPerSector->subcode.present[19])
				, BcdToDec(pDiscPerSector->subcode.present[20]), BcdToDec(pDiscPerSector->subcode.present[21]));
			if (nRLBA == 5001 && nALBA == 5151) {
				OutputLogA(standardOut | fileDisc, "Detected intentional subchannel in LBA 5000 => SecuROM Type 2 (a.k.a. NEW)\n");
				pDisc->PROTECT.byExist = securomV2;
			}
			else if (pDisc->PROTECT.byExist == securomTmp) {
				pDisc->PROTECT.byExist = securomV1;
			}
			else {
				for (INT nTmpLBA = 40000; nTmpLBA < 45800; nTmpLBA++) {
					if (pDisc->SCSI.nAllLength > nTmpLBA) {
						if (!ExecReadCD(pExtArg, pDevice, lpCmd, nTmpLBA, pDiscPerSector->data.present,
							pDevice->TRANSFER.dwBufLen, _T(__FUNCTION__), __LINE__)) {
							return FALSE;
						}
						WORD reCalcCrc16 = (WORD)GetCrc16CCITT(10, &pDiscPerSector->subcode.present[12]);
						WORD reCalcXorCrc16 = (WORD)(reCalcCrc16 ^ 0x0080);
						if (pDiscPerSector->subcode.present[22] == HIBYTE(reCalcXorCrc16) &&
							pDiscPerSector->subcode.present[23] == LOBYTE(reCalcXorCrc16)) {
							OutputLogA(standardOut | fileDisc
								, "Detected intentional subchannel in LBA %d => SecuROM Type 1 (a.k.a. OLD)\n", nTmpLBA);
							pDisc->PROTECT.byExist = securomV1;
							break;
						}
					}
				}
				if (pDisc->PROTECT.byExist != securomV1) {
					OutputLogA(standardOut | fileDisc, "SecuROM sector not found \n");
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
			OutputLogA(standardOut | fileDisc, "SecuROM sector not found \n");
		}
	}
	return TRUE;
}

BOOL ReadCDAll(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	CDFLAG::_READ_CD::_ERROR_FLAGS c2,
	LPCTSTR pszPath,
	FILE* fpCcd,
	FILE* fpC2
) {
	FILE* fpImg = NULL;
	_TCHAR pszOutScmFile[_MAX_PATH] = { 0 };
	if (NULL == (fpImg = CreateOrOpenFile(pszPath, NULL,
		pszOutScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	FILE* fpCue = NULL;
	FILE* fpCueForImg = NULL;
	FILE* fpParse = NULL;
	FILE* fpSub = NULL;
	LPBYTE pBuf = NULL;
	LPBYTE pNextBuf = NULL;
	LPBYTE pNextNextBuf = NULL;
	INT nMainDataType = scrambled;
	if (pExtArg->byBe) {
		nMainDataType = unscrambled;
	}

	try {
		// init start
		if (NULL == (fpCue = CreateOrOpenFile(
			pszPath, NULL, NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpCueForImg = CreateOrOpenFile(
			pszPath, _T("_img"), NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpParse = CreateOrOpenFile(
			pszPath, _T("_subReadable"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpSub = CreateOrOpenFile(
			pszPath, NULL, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		BYTE byTransferLen = 1;
		if (pDevice->byPlxtrDrive) {
			byTransferLen = 2;
		}
		// store main + (c2) + sub data all
		if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
			pDevice->TRANSFER.dwBufLen * byTransferLen, &pDiscPerSector->data.present, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		if (1 <= pExtArg->dwSubAddionalNum) {
			if (!GetAlignedCallocatedBuffer(pDevice, &pNextBuf,
				pDevice->TRANSFER.dwBufLen * byTransferLen, &pDiscPerSector->data.next, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			if (2 <= pExtArg->dwSubAddionalNum) {
				if (!GetAlignedCallocatedBuffer(pDevice, &pNextNextBuf,
					pDevice->TRANSFER.dwBufLen * byTransferLen, &pDiscPerSector->data.nextNext, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
			}
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		_TCHAR szSubCode[5] = { 0 };
		if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
				_tcsncpy(szSubCode, _T("Raw"), sizeof(szSubCode) / sizeof(szSubCode[0]));
				SetReadD8Command(pDevice, &cdb, byTransferLen, CDFLAG::_PLXTR_READ_CDDA::MainC2Raw);
			}
			else {
				_tcsncpy(szSubCode, _T("Pack"), sizeof(szSubCode) / sizeof(szSubCode[0]));
				SetReadD8Command(pDevice, &cdb, byTransferLen, CDFLAG::_PLXTR_READ_CDDA::MainPack);
			}
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		}
		else {
			// non plextor && support scrambled ripping
			CDB::_READ_CD cdb = { 0 };
			CDFLAG::_READ_CD::_EXPECTED_SECTOR_TYPE type = CDFLAG::_READ_CD::CDDA;
			if (pExtArg->byBe) {
				type = CDFLAG::_READ_CD::All;
			}
			CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION sub = CDFLAG::_READ_CD::Raw;
			_tcsncpy(szSubCode, _T("Raw"), sizeof(szSubCode) / sizeof(szSubCode[0]));
			if (pExtArg->byPack) {
				sub = CDFLAG::_READ_CD::Pack;
				_tcsncpy(szSubCode, _T("Pack"), sizeof(szSubCode) / sizeof(szSubCode[0]));
			}
			SetReadCDCommand(pExtArg, pDevice, &cdb, type, byTransferLen, c2, sub, FALSE);
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		}
		OutputLog(standardOut | fileDisc,
			_T("Set OpCode: %#02x, SubCode: %x(%s)\n"), lpCmd[0], lpCmd[10], szSubCode);

		BYTE lpPrevSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		// to get prevSubQ
		if (pDisc->SUB.nSubChannelOffset) {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, -2, pDiscPerSector->data.present,
				pDevice->TRANSFER.dwBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.present);
			SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.prev, pDiscPerSector->subcode.present);

			if (!ExecReadCD(pExtArg, pDevice, lpCmd, -1, pDiscPerSector->data.present,
				pDevice->TRANSFER.dwBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.present);
			SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.present, pDiscPerSector->subcode.present);
			memcpy(lpPrevSubcode, pDiscPerSector->subcode.present, CD_RAW_READ_SUBCODE_SIZE);
		}
		else {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, -1, pDiscPerSector->data.present,
				pDevice->TRANSFER.dwBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.present);
			SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.prev, pDiscPerSector->subcode.present);
		}
		// special fix begin
		if (pDiscPerSector->subQ.prev.byAdr != ADR_ENCODES_CURRENT_POSITION) {
			// [PCE] 1552 Tenka Tairan
			pDiscPerSector->subQ.prev.byAdr = ADR_ENCODES_CURRENT_POSITION;
			pDiscPerSector->subQ.prev.byTrackNum = 1;
			pDiscPerSector->subQ.prev.nAbsoluteTime = 149;
		}
		if (!ReadCDForCheckingSecuROM(pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd)) {
			throw FALSE;
		}
		// special fix end

		for (UINT p = 0; p < pDisc->SCSI.toc.LastTrack; p++) {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, pDisc->SCSI.lpFirstLBAListOnToc[p]
				, pDiscPerSector->data.present, pDevice->TRANSFER.dwBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.present);
			pDisc->SUB.lpEndCtlList[p] = (BYTE)((pDiscPerSector->subcode.present[12] >> 4) & 0x0f);
			OutputString(_T("\rChecking SubQ ctl (Track) %2u/%2u"), p + 1, pDisc->SCSI.toc.LastTrack);
		}
		OutputString(_T("\n"));
		SetCDOffset(pExecType, pExtArg->byBe, pDevice->byPlxtrDrive, pDisc, 0, pDisc->SCSI.nAllLength);

		BYTE byCurrentTrackNum = pDisc->SCSI.toc.FirstTrack;
		INT nFirstLBAForSub = 0;
		INT nFirstLBA = pDisc->MAIN.nOffsetStart;
		INT nLastLBA = 0;
		/*
		if (pExtArg->byBe) {
			nLastLBA = pDisc->SCSI.nAllLength;
		}
		else {
		*/
			nLastLBA = pDisc->SCSI.nAllLength + pDisc->MAIN.nOffsetEnd;
			/*
		}
		*/
		INT nLBA = nFirstLBA;

		if (pExtArg->byPre) {
			nFirstLBAForSub = PREGAP_START_LBA;
			nFirstLBA = PREGAP_START_LBA;
			nLBA = nFirstLBA;
			pDisc->MAIN.nOffsetStart = PREGAP_START_LBA;
		}
		// init end
		FlushLog();

		BOOL bReadOK = pExtArg->byPre ? FALSE : TRUE;
		BOOL bC2Error = FALSE;

		while (nFirstLBA < nLastLBA) {
			BOOL bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd, nLBA);
			if (bProcessRet == RETURNED_EXIST_C2_ERROR) {
				bC2Error = TRUE;
				// C2 error points the current LBA - 1 (offset?)
				OutputLogA(standardError | fileC2Error,
					"\rLBA[%06d, %#07x] Detected C2 error    \n", nLBA, nLBA);
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					pDisc->MAIN.lpAllLBAOfC2Error[pDisc->MAIN.nC2ErrorCnt++] = nLBA;
				}
			}
			else if (bProcessRet == RETURNED_SKIP_LBA) {
				nLBA = pDisc->MAIN.nFixFirstLBAof2ndSession - 1;
				nFirstLBA = nLBA;
			}
			else if (bProcessRet == RETURNED_CONTINUE &&
				!(pExtArg->byPre && -1149 <= nLBA && nLBA <= -76)) {
				ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector
					, nLBA, nMainDataType, byCurrentTrackNum, fpImg, fpSub, fpC2, fpParse);
			}
			else if (bProcessRet == RETURNED_FALSE) {
				throw FALSE;
			}
			if (bProcessRet != RETURNED_CONTINUE &&
				bProcessRet != RETURNED_SKIP_LBA) {
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData
					&& pExtArg->nC2RereadingType == 1 && nFirstLBA - pDisc->MAIN.nOffsetStart >= 0) {
					GetCrc32(&pDisc->MAIN.lpAllSectorCrc32[nFirstLBA - pDisc->MAIN.nOffsetStart]
						, pDiscPerSector->data.present, CD_RAW_SECTOR_SIZE);
				}
				if (pDisc->SUB.nSubChannelOffset) {
					if (!(pExtArg->byScanProtectViaFile && pDisc->PROTECT.byExist &&
						(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
							nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize))) {
						if (2 <= pExtArg->dwSubAddionalNum) {
							memcpy(pDiscPerSector->subcode.nextNext
								, pDiscPerSector->subcode.next, CD_RAW_READ_SUBCODE_SIZE);
						}
						if (1 <= pExtArg->dwSubAddionalNum) {
							memcpy(pDiscPerSector->subcode.next
								, pDiscPerSector->subcode.present, CD_RAW_READ_SUBCODE_SIZE);
						}
						memcpy(pDiscPerSector->subcode.present, lpPrevSubcode, CD_RAW_READ_SUBCODE_SIZE);
					}
				}
				SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.present, pDiscPerSector->subcode.present);

				if (pExtArg->byPre && PREGAP_START_LBA <= nLBA && nLBA <= -76) {
					if (pDiscPerSector->subQ.present.byTrackNum == 1 &&
						pDiscPerSector->subQ.present.nAbsoluteTime == 0) {
						pDiscPerSector->subQ.prev.nRelativeTime = pDiscPerSector->subQ.present.nRelativeTime + 1;
						pDiscPerSector->subQ.prev.nAbsoluteTime = -1;
						pDisc->MAIN.nFixStartLBA = nLBA;
						bReadOK = TRUE;
#if 0
						if (pDisc->MAIN.nAdjustSectorNum < 0 ||
							1 < pDisc->MAIN.nAdjustSectorNum) {
							for (INT i = 0; i < abs(pDisc->MAIN.nAdjustSectorNum) * CD_RAW_SECTOR_SIZE; i++) {
								fputc(0, fpImg);
							}
						}
#endif
					}
					if (bReadOK) {
						if (pDiscPerSector->subQ.present.byTrackNum == 1 &&
							pDiscPerSector->subQ.present.nAbsoluteTime == 74) {
							nFirstLBA = -76;
						}
					}
				}
#if 0
				if (pExtArg->byPre && PREGAP_START_LBA <= nLBA && nLBA <= -76) {
					OutputCDSub96Align(pDiscPerSector->subcode.present, nLBA);
					OutputCDMain(fileDisc, pDiscPerSector->data.present, nLBA, CD_RAW_SECTOR_SIZE);
				}
#endif
				if (bReadOK) {
					if (nFirstLBAForSub <= nLBA && nLBA < pDisc->SCSI.nAllLength) {
						if (IsCheckingSubChannel(pExtArg, pDisc, nLBA)) {
							BOOL bLibCrypt = IsValidLibCryptSector(pExtArg->byLibCrypt, nLBA);
							BOOL bSecuRom = IsValidIntentionalSubSector(pExtArg->byIntentionalSub, pDisc, nLBA);
							CheckAndFixSubChannel(pExecType, pExtArg, pDevice, pDisc
								, pDiscPerSector, byCurrentTrackNum, nLBA, bLibCrypt, bSecuRom);
							BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
							// fix raw subchannel
							AlignColumnSubcode(pDiscPerSector->subcode.present, lpSubcodeRaw);
#if 0
							OutputCDSub96Align(pDiscPerSector->subcode.present, nLBA);
							OutputCDSub96Raw(standardOut, lpSubcodeRaw, nLBA);
#endif
							WriteSubChannel(pDisc, lpSubcodeRaw,
								pDiscPerSector->subcode.present, nLBA, byCurrentTrackNum, fpSub, fpParse);
							CheckAndFixMainHeader(pExtArg, pDisc
								, pDiscPerSector, nLBA, byCurrentTrackNum, nMainDataType);
							SetTrackAttribution(pExtArg, pDisc, nLBA,
								&byCurrentTrackNum, &pDiscPerSector->mainHeader, &pDiscPerSector->subQ);
							UpdateTmpSubQData(&pDiscPerSector->subQ, bLibCrypt, bSecuRom);
						}
					}
					// Write track to scrambled
					WriteMainChannel(pExtArg, pDisc, pDiscPerSector->data.present, nLBA, fpImg);
					if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
						if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
							WriteC2(pExtArg, pDisc, pDiscPerSector->data.next + pDevice->TRANSFER.dwBufC2Offset, nLBA, fpC2);
						}
						else {
							WriteC2(pExtArg, pDisc, pDiscPerSector->data.present + pDevice->TRANSFER.dwBufC2Offset, nLBA, fpC2);
						}
					}
				}
#if 0
				else {
					BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
					AlignColumnSubcode(pDiscPerSector->subcode.present, lpSubcodeRaw);
					OutputCDSubToLog(pDisc, pDiscPerSector->subcode.present, lpSubcodeRaw, nLBA, byCurrentTrackNum, fpParse);
				}
#endif
				if (pDisc->SUB.nSubChannelOffset) {
					memcpy(lpPrevSubcode, pDiscPerSector->subcode.next, CD_RAW_READ_SUBCODE_SIZE);
				}
			}

			OutputString(_T("\rCreated img (LBA) %6d/%6d"),	nLBA, pDisc->SCSI.nAllLength - 1);
			if (nFirstLBA == -76) {
				nLBA = nFirstLBA;
				if (!bReadOK) {
					bReadOK = TRUE;
				}
			}
			nLBA++;
			nFirstLBA++;
		}
		OutputString(_T("\n"));
		FcloseAndNull(fpParse);
		FcloseAndNull(fpSub);
		FlushLog();

		if (pDisc->SCSI.toc.FirstTrack == pDisc->SCSI.toc.LastTrack) {
			// [3DO] Jurassic Park Interactive (Japan)
			if (pDisc->SUB.lpFirstLBAListOnSub[0][2] == -1) {
				pDisc->SUB.lpFirstLBAListOnSub[0][1] = pDisc->SCSI.lpLastLBAListOnToc[0];
			}
		}
		for (INT i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
			if (pDisc->PROTECT.byExist == cds300 && i == pDisc->SCSI.toc.LastTrack - 1) {
				break;
			}
			BOOL bErr = FALSE;
			LONG lLine = 0;
			if (pDisc->SUB.lpFirstLBAListOnSub[i][1] == -1) {
				bErr = TRUE;
				lLine = __LINE__;
			}
			else if ((pDisc->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				if (pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[i] == -1) {
					bErr = TRUE;
					lLine = __LINE__;
				}
				else if (pDisc->SUB.lpLastLBAListOfDataTrackOnSub[i] == -1) {
					bErr = TRUE;
					lLine = __LINE__;
				}
			}
			if (bErr) {
				OutputErrorString(
					_T("[L:%ld] Internal error. Failed to analyze the subchannel. Track[%02u]/[%02u]\n"),
					lLine, i + 1, pDisc->SCSI.toc.LastTrack);
				throw FALSE;
			}
		}
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (bC2Error) {
				if (pExtArg->nC2RereadingType == 0) {
					if (!ReadCDForRereadingSectorType1(pExtArg, pDevice, pDisc, lpCmd, fpImg, fpC2)) {
						throw FALSE;
					}
				}
				else {
					INT nStartLBA = pExtArg->nStartLBAForC2;
					INT nEndLBA = pExtArg->nEndLBAForC2;
					if (nStartLBA == 0 && nEndLBA == 0) {
						nStartLBA = pDisc->MAIN.nOffsetStart;
						nEndLBA = pDisc->SCSI.nAllLength + pDisc->MAIN.nOffsetEnd;
					}
					if (!ReadCDForRereadingSectorType2(pExtArg, pDevice, pDisc, lpCmd, fpImg, fpC2, nStartLBA, nEndLBA)) {
						throw FALSE;
					}
				}
			}
			else {
				OutputLogA(standardOut | fileC2Error, "No C2 errors\n");
			}
			FcloseAndNull(fpC2);
		}
		FcloseAndNull(fpImg);
		OutputTocWithPregap(pDisc);

		_TCHAR pszNewPath[_MAX_PATH] = { 0 };
		_tcsncpy(pszNewPath, pszOutScmFile, sizeof(pszNewPath) / sizeof(pszNewPath[0]));
		pszNewPath[_MAX_PATH - 1] = 0;
		// "PathRenameExtension" fails to rename if space is included in extension.
		// e.g.
		//  no label. 2017-02-14_ 9-41-31 => no label. 2017-02-14_ 9-41-31.img
		//  no label.2017-02-14_9-41-31   => no label.img
		if (!PathRenameExtension(pszNewPath, _T(".img"))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		// audio only -> from .scm to .img. other descramble img.
		if (pExtArg->byBe || pDisc->SCSI.trackType == TRACK_TYPE::audioOnly) {
			OutputString(_T("Moving .scm to .img\n"));
			if (!MoveFileEx(pszOutScmFile, pszNewPath, MOVEFILE_REPLACE_EXISTING)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (pExtArg->byBe) {
				ExecEccEdc(pExtArg->byScanProtectViaFile, pDisc->PROTECT, pszNewPath, pDisc->PROTECT.ERROR_SECTOR);
			}
		}
		else {
			OutputString(_T("Copying .scm to .img\n"));
			if (!CopyFile(pszOutScmFile, pszNewPath, FALSE)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			_TCHAR pszImgPath[_MAX_PATH] = { 0 };
			if (NULL == (fpImg = CreateOrOpenFile(
				pszPath, NULL, pszImgPath, NULL, NULL, _T(".img"), _T("rb+"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			DescrambleMainChannelAll(pExtArg, pDisc, scrambled_table, fpImg);
			FcloseAndNull(fpImg);
			ExecEccEdc(pExtArg->byScanProtectViaFile, pDisc->PROTECT, pszImgPath, pDisc->PROTECT.ERROR_SECTOR);
		}

		_TCHAR pszImgName[_MAX_FNAME] = { 0 };
		if (NULL == (fpImg = CreateOrOpenFile(
			pszPath, NULL, NULL, pszImgName, NULL, _T(".img"), _T("rb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (!CreateBinCueCcd(pExtArg, pDisc, pszPath, pszImgName,
			pDevice->FEATURE.byCanCDText, fpImg, fpCue, fpCueForImg, fpCcd)) {
			throw FALSE;
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpImg);
	FcloseAndNull(fpCueForImg);
	FcloseAndNull(fpCue);
	FcloseAndNull(fpParse);
	FcloseAndNull(fpSub);
	FreeAndNull(pBuf);
	if (1 <= pExtArg->dwSubAddionalNum) {
		FreeAndNull(pNextBuf);
		if (2 <= pExtArg->dwSubAddionalNum) {
			FreeAndNull(pNextNextBuf);
		}
	}

	return bRet;
}

BOOL ReadCDPartial(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	CDFLAG::_READ_CD::_ERROR_FLAGS c2,
	LPCTSTR pszPath,
	INT nStart,
	INT nEnd,
	CDFLAG::_READ_CD::_EXPECTED_SECTOR_TYPE flg,
	FILE* fpC2
) {
	CONST INT size = 5;
	_TCHAR szExt[size] = { 0 };

	if (*pExecType == gd) {
		_tcsncpy(szExt, _T(".scm"), size);
	}
	else {
		_tcsncpy(szExt, _T(".bin"), size);
	}
	_TCHAR pszBinPath[_MAX_PATH] = { 0 };
	FILE* fpBin = NULL;
	if (pExtArg->byReverse) {
		fpBin = CreateOrOpenFile(pszPath, _T("_reverse"), pszBinPath, NULL, NULL, szExt, _T("wb"), 0, 0);
	}
	else {
		fpBin = CreateOrOpenFile(pszPath, NULL, pszBinPath, NULL, NULL, szExt, _T("wb"), 0, 0);
	}
	if (!fpBin) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	FILE* fpParse = NULL;
	FILE* fpSub = NULL;
	LPBYTE pBuf = NULL;
	LPBYTE pNextBuf = NULL;
	LPBYTE pNextNextBuf = NULL;
	INT nMainDataType = scrambled;
	if (*pExecType == data || pExtArg->byBe) {
		nMainDataType = unscrambled;
	}

	try {
		// init start
		if (!pExtArg->byReverse) {
			if (NULL == (fpParse = CreateOrOpenFile(
				pszPath, _T("_subReadable"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (fpSub = CreateOrOpenFile(
				pszPath, NULL, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}
		BYTE byTransferLen = 1;
		if (pDevice->byPlxtrDrive) {
			byTransferLen = 2;
		}
		// store main+(c2)+sub data
		if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
			pDevice->TRANSFER.dwBufLen * byTransferLen, &pDiscPerSector->data.present, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		if (1 <= pExtArg->dwSubAddionalNum) {
			if (!GetAlignedCallocatedBuffer(pDevice, &pNextBuf,
				pDevice->TRANSFER.dwBufLen * byTransferLen, &pDiscPerSector->data.next, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			if (2 <= pExtArg->dwSubAddionalNum) {
				if (!GetAlignedCallocatedBuffer(pDevice, &pNextNextBuf,
					pDevice->TRANSFER.dwBufLen * byTransferLen, &pDiscPerSector->data.nextNext, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
			}
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		_TCHAR szSubCode[5] = { 0 };
		if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
				_tcsncpy(szSubCode, _T("Raw"), sizeof(szSubCode) / sizeof(szSubCode[0]));
				SetReadD8Command(pDevice, &cdb, byTransferLen, CDFLAG::_PLXTR_READ_CDDA::MainC2Raw);
			}
			else {
				_tcsncpy(szSubCode, _T("Pack"), sizeof(szSubCode) / sizeof(szSubCode[0]));
				SetReadD8Command(pDevice, &cdb, byTransferLen, CDFLAG::_PLXTR_READ_CDDA::MainPack);
			}
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		}
		else {
			// non plextor && support scrambled ripping
			CDB::_READ_CD cdb = { 0 };
			_tcsncpy(szSubCode, _T("Raw"), sizeof(szSubCode) / sizeof(szSubCode[0]));
			SetReadCDCommand(pExtArg, pDevice, &cdb, flg, byTransferLen, c2, CDFLAG::_READ_CD::Raw, FALSE);
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		}
		OutputLog(standardOut | fileDisc,
			_T("Set OpCode: %#02x, SubCode: %x(%s)\n"), lpCmd[0], lpCmd[10], szSubCode);

		BYTE lpPrevSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		if (pDisc->SUB.nSubChannelOffset) { // confirmed PXS88T, TS-H353A
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, nStart - 2, pDiscPerSector->data.present,
				pDevice->TRANSFER.dwBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				if (nStart == 0) {
					pDiscPerSector->subQ.present.nRelativeTime = 0;
					pDiscPerSector->subQ.present.nAbsoluteTime = 149;
					SetBufferFromTmpSubQData(pDiscPerSector->subQ.present, pDiscPerSector->subcode.present, 1);
					SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.prev, pDiscPerSector->subcode.present);
				}
				else {
					throw FALSE;
				}
			}
			else {
				AlignRowSubcode(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.present);
				SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.prev, pDiscPerSector->subcode.present);
			}
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, nStart - 1, pDiscPerSector->data.present,
				pDevice->TRANSFER.dwBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
				if (nStart == 0) {
					pDiscPerSector->subQ.present.nRelativeTime = 0;
					pDiscPerSector->subQ.present.nAbsoluteTime = 150;
					SetBufferFromTmpSubQData(pDiscPerSector->subQ.present, pDiscPerSector->subcode.present, 1);
					for (INT i = 0; i < 12; i++) {
						pDiscPerSector->subcode.present[i] = 0xff;
					}
				}
				else {
					throw FALSE;
				}
			}
			else {
				AlignRowSubcode(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.present);
				SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.present, pDiscPerSector->subcode.present);
			}
			memcpy(lpPrevSubcode, pDiscPerSector->subcode.present, CD_RAW_READ_SUBCODE_SIZE);
		}
		else {
			if (*pExecType != gd) {
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, nStart - 1, pDiscPerSector->data.present,
					pDevice->TRANSFER.dwBufLen, _T(__FUNCTION__), __LINE__)) {
				}
				else {
					AlignRowSubcode(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.present);
					SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.prev, pDiscPerSector->subcode.present);
				}
			}
		}
		if (*pExecType == gd) {
			for (INT p = pDisc->GDROM_TOC.FirstTrack - 1; p < pDisc->GDROM_TOC.LastTrack; p++) {
				pDisc->SUB.lpEndCtlList[p] = pDisc->GDROM_TOC.TrackData[p].Control;
			}
		}
		else {
			for (UINT p = 0; p < pDisc->SCSI.toc.LastTrack; p++) {
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, pDisc->SCSI.lpFirstLBAListOnToc[p]
					, pDiscPerSector->data.present, pDevice->TRANSFER.dwBufLen * byTransferLen, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				AlignRowSubcode(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.present);
				pDisc->SUB.lpEndCtlList[p] = (BYTE)((pDiscPerSector->subcode.present[12] >> 4) & 0x0f);
				OutputString(_T("\rChecking SubQ ctl (Track) %2u/%2u"), p + 1, pDisc->SCSI.toc.LastTrack);
			}
			OutputString(_T("\n"));
		}
		SetCDOffset(pExecType, pExtArg->byBe, pDevice->byPlxtrDrive, pDisc, nStart, nEnd);
#ifdef _DEBUG
		OutputString(
			_T("byBe: %d, nCombinedOffset: %d, uiMainDataSlideSize: %u, nOffsetStart: %u, nOffsetEnd: %u, nFixStartLBA: %u, nFixEndLBA: %u\n")
			, pExtArg->byBe, pDisc->MAIN.nCombinedOffset, pDisc->MAIN.uiMainDataSlideSize
			, pDisc->MAIN.nOffsetStart, pDisc->MAIN.nOffsetEnd, pDisc->MAIN.nFixStartLBA, pDisc->MAIN.nFixEndLBA);
#endif
		BYTE byCurrentTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
		if (*pExecType == gd) {
			byCurrentTrackNum = pDisc->GDROM_TOC.FirstTrack;
		}
		else if (byCurrentTrackNum < pDisc->SCSI.toc.FirstTrack || pDisc->SCSI.toc.LastTrack < byCurrentTrackNum) {
			byCurrentTrackNum = pDisc->SCSI.toc.FirstTrack;
		}
#ifdef _DEBUG
		OutputString(
			_T("byBe: %d, nCombinedOffset: %d, uiMainDataSlideSize: %u, nOffsetStart: %u, nOffsetEnd: %u, nFixStartLBA: %u, nFixEndLBA: %u\n")
			, pExtArg->byBe, pDisc->MAIN.nCombinedOffset, pDisc->MAIN.uiMainDataSlideSize
			, pDisc->MAIN.nOffsetStart, pDisc->MAIN.nOffsetEnd, pDisc->MAIN.nFixStartLBA, pDisc->MAIN.nFixEndLBA);
#endif
		INT nLastLBA = nEnd + pDisc->MAIN.nOffsetEnd;
		INT nFirstLBA = nStart + pDisc->MAIN.nOffsetStart - 1;
		if (*pExecType == gd) {
			if (nFirstLBA < 0) {
				nFirstLBA = 0;
			}
			else if (nFirstLBA == 44849) {
				nFirstLBA = 44850;
			}
		}
		INT nLBA = nFirstLBA;
		if (pExtArg->byReverse) {
			nLBA = nLastLBA;
		}
		// init end
		FlushLog();

		INT nStoreLBA = 0;
		INT nRetryCnt = 1;
		BOOL bC2Error = FALSE;

		while (nFirstLBA < nLastLBA) {
			BOOL bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd, nLBA);
			if (bProcessRet == RETURNED_EXIST_C2_ERROR) {
				bC2Error = TRUE;
				// C2 error points the current LBA - 1 (offset?)
				OutputLogA(standardError | fileC2Error,
					"\rLBA[%06d, %#07x] Detected C2 error    \n", nLBA, nLBA);
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					pDisc->MAIN.lpAllLBAOfC2Error[pDisc->MAIN.nC2ErrorCnt++] = nLBA;
				}
			}
			else if (bProcessRet == RETURNED_SKIP_LBA) {
				if (pExtArg->byReverse) {
					nLBA = pDisc->SCSI.nFirstLBAof2ndSession - SESSION_TO_SESSION_SKIP_LBA;
				}
				else {
					nLBA = pDisc->MAIN.nFixFirstLBAof2ndSession - 1;
				}
			}
			else if (bProcessRet == RETURNED_CONTINUE) {
				ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector
					, nLBA, nMainDataType, byCurrentTrackNum, fpBin, fpSub, fpC2, fpParse);
			}
			else if (bProcessRet == RETURNED_FALSE) {
				if (*pExecType == gd && nRetryCnt <= 10 && nLBA > 65000) {
					OutputLog(standardError | fileMainError, _T("Retry %d/10\n"), nRetryCnt);
					INT nTmpLBA = 0;
					for (nTmpLBA = nLBA - 20000; 449849 <= nTmpLBA; nTmpLBA -= 20000) {
						OutputString(_T("Reread %d sector\n"), nTmpLBA);
						if (RETURNED_FALSE == ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd, nTmpLBA,
							pDiscPerSector->data.present, _T(__FUNCTION__), __LINE__)) {
							if (nTmpLBA < 20000) {
								break;
							}
						}
						else {
							break;
						}
					}
					if (nStoreLBA == 0 && nRetryCnt == 1) {
						nStoreLBA = nLBA;
					}
					nLBA = nTmpLBA;
					nRetryCnt++;
					continue;
				}
				else {
					throw FALSE;
				}
			}
			if (nRetryCnt > 1) {
				if (nStoreLBA == nLBA) {
					// init
					nStoreLBA = 0;
					nRetryCnt = 1;
					OutputString(_T("\n"));
				}
				else {
					OutputString(_T("\rReread %d sector"), nLBA);
					nLBA++;
					continue;
				}
			}
			if (bProcessRet != RETURNED_CONTINUE &&
				bProcessRet != RETURNED_SKIP_LBA) {
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData
					&& pExtArg->nC2RereadingType == 1 && nFirstLBA - pDisc->MAIN.nOffsetStart >= 0) {
					GetCrc32(&pDisc->MAIN.lpAllSectorCrc32[nFirstLBA - pDisc->MAIN.nOffsetStart]
						, pDiscPerSector->data.present, CD_RAW_SECTOR_SIZE);
				}
				if (pDisc->SUB.nSubChannelOffset) {
					if (!(pExtArg->byScanProtectViaFile && pDisc->PROTECT.byExist &&
						(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
							nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize))) {
						if (2 <= pExtArg->dwSubAddionalNum) {
							memcpy(pDiscPerSector->subcode.nextNext, pDiscPerSector->subcode.next, CD_RAW_READ_SUBCODE_SIZE);
						}
						if (1 <= pExtArg->dwSubAddionalNum) {
							memcpy(pDiscPerSector->subcode.next, pDiscPerSector->subcode.present, CD_RAW_READ_SUBCODE_SIZE);
						}
						memcpy(pDiscPerSector->subcode.present, lpPrevSubcode, CD_RAW_READ_SUBCODE_SIZE);
					}
				}
				SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.present, pDiscPerSector->subcode.present);

				if (nStart <= nLBA && nLBA < nEnd) {
					BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
					if (!pExtArg->byReverse) {
						if (IsCheckingSubChannel(pExtArg, pDisc, nLBA)) {
							CheckAndFixSubChannel(pExecType, pExtArg, pDevice, pDisc
								, pDiscPerSector, byCurrentTrackNum, nLBA, FALSE, FALSE);
							// fix raw subchannel
							AlignColumnSubcode(pDiscPerSector->subcode.present, lpSubcodeRaw);
#if 0
							OutputCDSub96Align(pDiscPerSector->subcode.present, nLBA);
#endif
							WriteSubChannel(pDisc, lpSubcodeRaw,
								pDiscPerSector->subcode.present, nLBA, byCurrentTrackNum, fpSub, fpParse);
							CheckAndFixMainHeader(pExtArg, pDisc
								, pDiscPerSector, nLBA, byCurrentTrackNum, nMainDataType);
						}
						if (*pExecType == gd) {
							byCurrentTrackNum = pDiscPerSector->subQ.present.byTrackNum;
						}
						else {
							SetTrackAttribution(pExtArg, pDisc, nLBA,
								&byCurrentTrackNum, &pDiscPerSector->mainHeader, &pDiscPerSector->subQ);
						}
						UpdateTmpSubQData(&pDiscPerSector->subQ, FALSE, FALSE);
					}
				}
				// Write track to scrambled
				WriteMainChannel(pExtArg, pDisc, pDiscPerSector->data.present, nLBA, fpBin);
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
						WriteC2(pExtArg, pDisc, pDiscPerSector->data.next + pDevice->TRANSFER.dwBufC2Offset, nLBA, fpC2);
					}
					else {
						WriteC2(pExtArg, pDisc, pDiscPerSector->data.present + pDevice->TRANSFER.dwBufC2Offset, nLBA, fpC2);
					}
				}
				if (pDisc->SUB.nSubChannelOffset) {
					memcpy(lpPrevSubcode, pDiscPerSector->subcode.next, CD_RAW_READ_SUBCODE_SIZE);
				}
			}
			if (pExtArg->byReverse) {
				OutputString(_T("\rCreating bin from %d to %d (LBA) %6d")
					, nEnd + pDisc->MAIN.nOffsetEnd, nStart + pDisc->MAIN.nOffsetStart - 1, nLBA);
				nLBA--;
			}
			else {
				OutputString(_T("\rCreating bin from %d to %d (LBA) %6d"),
					nStart + pDisc->MAIN.nOffsetStart, nEnd + pDisc->MAIN.nOffsetEnd, nLBA);
				nLBA++;
			}
			nFirstLBA++;
		}
		OutputString(_T("\n"));
		FcloseAndNull(fpParse);
		FcloseAndNull(fpSub);
		FlushLog();

		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (bC2Error) {
				if (pExtArg->nC2RereadingType == 0) {
					if (!ReadCDForRereadingSectorType1(pExtArg, pDevice, pDisc, lpCmd, fpBin, fpC2)) {
						throw FALSE;
					}
				}
				else {
					INT nStartLBA = pExtArg->nStartLBAForC2;
					INT nEndLBA = pExtArg->nEndLBAForC2;
					if (nStartLBA == 0 && nEndLBA == 0) {
						nStartLBA = nStart;
						nEndLBA = nEnd;
					}
					if (!ReadCDForRereadingSectorType2(pExtArg, pDevice, pDisc, lpCmd, fpBin, fpC2, nStartLBA, nEndLBA)) {
						throw FALSE;
					}
				}
			}
			else {
				OutputLogA(standardOut | fileC2Error, "No C2 errors\n");
			}
			FcloseAndNull(fpC2);
		}
		FcloseAndNull(fpBin);

		if (pExtArg->byReverse) {
			FILE* fpBin_r = NULL;
			if (NULL == (fpBin_r = CreateOrOpenFile(
				pszPath, _T("_reverse"), NULL, NULL, NULL, szExt, _T("rb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (fpBin = CreateOrOpenFile(
				pszPath, NULL, NULL, NULL, NULL, szExt, _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			OutputString(_T("Reversing _reverse%s to %s\n"), szExt, szExt);
			BYTE rBuf[CD_RAW_SECTOR_SIZE] = { 0 };
			DWORD dwRoop = GetFileSize(0, fpBin_r) - CD_RAW_SECTOR_SIZE * 3;
			LONG lSeek = CD_RAW_SECTOR_SIZE - (LONG)pDisc->MAIN.uiMainDataSlideSize;
			fseek(fpBin_r, -lSeek, SEEK_END);
			fread(rBuf, sizeof(BYTE), (size_t)lSeek, fpBin_r);
			fwrite(rBuf, sizeof(BYTE), (size_t)lSeek, fpBin);
			fseek(fpBin_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);

			for (DWORD i = 0; i < dwRoop; i += CD_RAW_SECTOR_SIZE) {
				fseek(fpBin_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
				fread(rBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpBin_r);
				fwrite(rBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpBin);
				fseek(fpBin_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
			}
			fseek(fpBin_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
			fread(rBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpBin_r);
			fwrite(rBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpBin);

			FcloseAndNull(fpBin);
			FcloseAndNull(fpBin_r);
		}

		if (*pExecType == data) {
			if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe) {
				if (NULL == (fpBin = CreateOrOpenFile(
					pszPath, NULL, NULL, NULL, NULL, _T(".bin"), _T("rb+"), 0, 0))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				DescrambleMainChannelPartial(nStart, nEnd - 1, scrambled_table, fpBin);
				FcloseAndNull(fpBin);
			}
			ExecEccEdc(pExtArg->byScanProtectViaFile, pDisc->PROTECT, pszPath, pDisc->PROTECT.ERROR_SECTOR);
		}
		else if (*pExecType == gd) {
			_TCHAR pszImgPath[_MAX_PATH] = { 0 };
			if (!DescrambleMainChannelForGD(pszPath, pszImgPath)) {
				throw FALSE;
			}
			ExecEccEdc(pExtArg->byScanProtectViaFile, pDisc->PROTECT, pszImgPath, pDisc->PROTECT.ERROR_SECTOR);
			if (!SplitFileForGD(pszPath)) {
				throw FALSE;
			}
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpBin);
	FcloseAndNull(fpParse);
	FcloseAndNull(fpSub);
	FcloseAndNull(fpC2);
	FreeAndNull(pBuf);
	if (1 <= pExtArg->dwSubAddionalNum) {
		FreeAndNull(pNextBuf);
		if (2 <= pExtArg->dwSubAddionalNum) {
			FreeAndNull(pNextNextBuf);
		}
	}
	return bRet;
}

BOOL ReadGDForFileSystem(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	CDB::_READ_CD cdb = { 0 };
	SetReadCDCommand(NULL, pDevice, &cdb,
		CDFLAG::_READ_CD::CDDA, 1, CDFLAG::_READ_CD::NoC2, CDFLAG::_READ_CD::NoSub, TRUE);
	BYTE aToc[CD_RAW_SECTOR_SIZE * 2] = { 0 };
	BYTE bufDec[CD_RAW_SECTOR_SIZE] = { 0 };
	INT nOffset = pDisc->MAIN.nAdjustSectorNum - 1;
	if (pDisc->MAIN.nCombinedOffset < 0) {
		nOffset = pDisc->MAIN.nAdjustSectorNum;
	}
	if (!ExecReadGD(pExtArg, pDevice, pDisc, (LPBYTE)&cdb, FIRST_LBA_FOR_GD + nOffset, 1, aToc, bufDec)) {
		return FALSE;
	}

	// http://hwdocs.webs.com/dreamcast
	/*
	0x110 - 0x113: TOC1
	0x114 - 0x116: LBA(little) |
	0x117        : Ctl/Adr     |
	  :                        |-> 100 track
	  :                        |
	0x294 - 0x296: LBA(little) |
	0x297        : Ctl/Adr     |
	0x298 - 0x299: Zero
	0x29a        : First track |-> alway "3"
	0x29b        : Ctl/Adr     |-> alway "41"
	0x29c - 0x29d: Zero
	0x29e        : Last track
	0x29f        : Ctl/Adr
	0x2a0 - 0x2a2: Max LBA     |-> alway "b4 61 08" (549300)
	0x2a3        : Ctl/Adr     |-> alway "41"
	*/
	OutputCDMain(fileMainInfo, bufDec, FIRST_LBA_FOR_GD + nOffset, CD_RAW_SECTOR_SIZE);
	if (bufDec[0x110] != 'T' || bufDec[0x111] != 'O' ||
		bufDec[0x112] != 'C' || bufDec[0x113] != '1') {
		OutputErrorString(_T("No GD-ROM data\n"));
		return FALSE;
	}
	pDisc->GDROM_TOC.FirstTrack = bufDec[0x29a];
	pDisc->GDROM_TOC.LastTrack = bufDec[0x29e];
	pDisc->GDROM_TOC.Length = MAKELONG(
		MAKEWORD(bufDec[0x2a0], bufDec[0x2a1]), MAKEWORD(bufDec[0x2a2], 0));

	for (INT i = pDisc->GDROM_TOC.FirstTrack - 1, j = 0; i < pDisc->GDROM_TOC.LastTrack; i++, j += 4) {
		pDisc->GDROM_TOC.TrackData[i].Address = MAKELONG(
			MAKEWORD(bufDec[0x114 + j], bufDec[0x115 + j]), MAKEWORD(bufDec[0x116 + j], 0));
		pDisc->GDROM_TOC.TrackData[i].Control = BYTE((bufDec[0x117 + j]) >> 4 & 0x0f);
		pDisc->GDROM_TOC.TrackData[i].Adr = BYTE((bufDec[0x117 + j]) & 0x0f);
		pDisc->GDROM_TOC.TrackData[i].TrackNumber = (BYTE)(i + 1);
	}
	OutputTocForGD(pDisc);

	VOLUME_DESCRIPTOR volDesc;
	PDIRECTORY_RECORD pDirRec = NULL;
	BOOL bVD = FALSE;
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		pDevice->dwMaxTransferLength, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	if (!ReadVolumeDescriptor(pExecType, pExtArg, pDevice, pDisc, 0
		, (LPBYTE)&cdb, lpBuf, FIRST_LBA_FOR_GD + 16 + nOffset, &bVD, &volDesc)) {
		return FALSE;
	}
	if (bVD) {
		pDirRec = (PDIRECTORY_RECORD)calloc(DIRECTORY_RECORD_SIZE, sizeof(DIRECTORY_RECORD));
		if (!pDirRec) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			FreeAndNull(pBuf);
			return FALSE;
		}
		INT nDirPosNum = 0;
		if (!ReadPathTableRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb
			, volDesc.ISO_9660.dwLogicalBlkCoef, volDesc.ISO_9660.dwPathTblSize
			, volDesc.ISO_9660.dwPathTblPos + nOffset, pDirRec, &nDirPosNum)) {
			FreeAndNull(pDirRec);
			FreeAndNull(pBuf);
			return FALSE;
		}
		if (!ReadDirectoryRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb, lpBuf
			, volDesc.ISO_9660.dwLogicalBlkCoef, volDesc.ISO_9660.dwRootDataLen, pDirRec, nDirPosNum)) {
			FreeAndNull(pDirRec);
			FreeAndNull(pBuf);
			return FALSE;
		}
		FreeAndNull(pDirRec);
	}
	FreeAndNull(pBuf);
	return TRUE;
}
