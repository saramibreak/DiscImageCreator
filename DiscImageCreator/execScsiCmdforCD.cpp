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
#include "_external/crc16ccitt.h"

// These global variable is set at prngcd.cpp
extern unsigned char scrambled_table[2352];

BOOL ExecReadCD(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPBYTE lpCmd,
	INT nLBA,
	LPBYTE lpBuf,
	DWORD dwBufSize,
	LPCTSTR pszFuncName,
	LONG lLineNum
	)
{
	REVERSE_BYTES(&lpCmd[2], &nLBA);
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB12GENERIC_LENGTH
		, lpBuf, dwBufSize, &byScsiStatus, pszFuncName, lLineNum)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputErrorString(
			_T("lpCmd: %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x\n")
			_T("dwBufSize: %lu\n")
			, lpCmd[0], lpCmd[1], lpCmd[2], lpCmd[3], lpCmd[4], lpCmd[5]
			, lpCmd[6], lpCmd[7], lpCmd[8], lpCmd[9], lpCmd[10], lpCmd[11]
			, dwBufSize
			);
		return FALSE;
	}
	return TRUE;
}

BOOL ExecSearchingOffset(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpCmd,
	LPBYTE lpBuf,
	DWORD dwBufSize,
	BOOL bGetDriveOfs,
	INT nDriveSampleOffset,
	INT nDriveOffset
	)
{
	if (!ExecReadCD(pExtArg, pDevice, lpCmd, pDisc->SCSI.nFirstLBAofDataTrack
		, lpBuf, dwBufSize, _T(__FUNCTION__), __LINE__)) {
		if (*pExecType == gd) {
			OutputErrorString(
				_T("Couldn't read a data sector at scrambled mode\n")
				_T("Please start it again from 1st step or after waiting a little\n"));
		}
		else {
			if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
				OutputLogA(standardErr | fileDrive,
					"This drive doesn't support [command: %#02x, subch: %#02x]\n"
					, lpCmd[0], lpCmd[10]);
			}
			else {
				OutputErrorString(
					_T("This drive can't read a data sector at scrambled mode\n")
					_T("Please start it again at plextor drive (PX-708, 712, 716, 755, 760 etc)\n"));
			}
		}
		return FALSE;
	}
	else {
		if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
			OutputLogA(standardOut | fileDrive,
				"This drive support [command: %#02x, subch: %#02x]\n"
				, lpCmd[0], lpCmd[10]);
		}
		else {
			OutputLogA(standardOut | fileDrive,
				"This drive can read a data sector at scrambled mode\n");
		}
	}
	if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
		if (lpCmd[10] == 0x01 || lpCmd[10] == 0x03) {
			// check only
			return TRUE;
		}
		OutputDiscLogA(
			OUTPUT_DHYPHEN_PLUS_STR_WITH_SUBCH_F(Check Drive + CD offset), lpCmd[10]);
	}
	else {
		OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(Check Drive + CD offset));
	}
	OutputCDMain(fileDisc, lpBuf, pDisc->SCSI.nFirstLBAofDataTrack, CD_RAW_SECTOR_SIZE);
	if (dwBufSize != CD_RAW_SECTOR_SIZE) {
		BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		if (dwBufSize == CD_RAW_SECTOR_WITH_SUBCODE_SIZE) {
			AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
		}
		else if (dwBufSize == CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE) {
			AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE + CD_RAW_READ_C2_294_SIZE, lpSubcode);
		}
		OutputCDSub96Align(lpSubcode, pDisc->SCSI.nFirstLBAofDataTrack);
	}

	BYTE aBuf[CD_RAW_SECTOR_SIZE * 2] = { 0 };
	memcpy(aBuf, lpBuf, CD_RAW_SECTOR_SIZE);

	if (!ExecReadCD(pExtArg, pDevice, lpCmd, pDisc->SCSI.nFirstLBAofDataTrack + 1
		, lpBuf, dwBufSize, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	memcpy(aBuf + CD_RAW_SECTOR_SIZE, lpBuf, CD_RAW_SECTOR_SIZE);
	if (!GetWriteOffset(pDisc, aBuf)) {
		OutputErrorString(_T("Failed to get write-offset\n"));
		return FALSE;
	}
	OutputCDOffset(pExtArg, pDisc, bGetDriveOfs, nDriveSampleOffset, nDriveOffset);
	return TRUE;
}

BOOL ReadCDForSearchingOffset(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	BOOL bRet = TRUE;
	INT nDriveSampleOffset = 0;
	BOOL bGetDriveOfs = GetDriveOffset(pDevice->szProductId, &nDriveSampleOffset);
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
		bGetDriveOfs = TRUE;
	}
	else if (
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXW4824A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXW4012A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXW4012S
		) {
		nDriveSampleOffset = 98;
		bGetDriveOfs = TRUE;
	}
#endif
	if (!bGetDriveOfs) {
		_TCHAR aBuf[6] = { 0 };
		OutputString(
			_T("This drive doesn't define in driveOffset.txt\n")
			_T("Please input drive offset(Samples): "));
		INT b = _tscanf(_T("%6[^\n]%*[^\n]"), aBuf);
		b = _gettchar();
		nDriveSampleOffset = _ttoi(aBuf);
	}

	INT nDriveOffset = nDriveSampleOffset * 4; // byte size * 4 = sample size
	if (pDisc->SCSI.byAudioOnly) {
		pDisc->MAIN.nCombinedOffset = nDriveOffset;
		OutputCDOffset(pExtArg, pDisc, bGetDriveOfs, nDriveSampleOffset, nDriveOffset);
	}
	else {
		LPBYTE pBuf = NULL;
		LPBYTE lpBuf = NULL;
		if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
			CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::NoSub);
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		}
		else {
			CDB::_READ_CD cdb = { 0 };
			SetReadCDCommand(NULL, pDevice, &cdb,
				CDFLAG::_READ_CD::CDDA, 1, CDFLAG::_READ_CD::NoSub, TRUE);
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		}
		if (*pExecType == gd) {
			pDisc->SCSI.nFirstLBAofDataTrack = FIRST_LBA_FOR_GD;
		}
		if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, lpBuf
			, CD_RAW_SECTOR_SIZE, bGetDriveOfs, nDriveSampleOffset, nDriveOffset)) {
			bRet = FALSE;
		}
		if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
			lpCmd[10] = (BYTE)CDFLAG::_PLXTR_READ_CDDA::MainQ;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, lpBuf
				, CD_RAW_SECTOR_SIZE + 16, bGetDriveOfs, nDriveSampleOffset, nDriveOffset)) {
			}
			lpCmd[10] = (BYTE)CDFLAG::_PLXTR_READ_CDDA::MainPack;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, lpBuf
				, CD_RAW_SECTOR_WITH_SUBCODE_SIZE, bGetDriveOfs, nDriveSampleOffset, nDriveOffset)) {
				bRet = FALSE;
			}
			lpCmd[10] = (BYTE)CDFLAG::_PLXTR_READ_CDDA::Raw;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, lpBuf
				, CD_RAW_READ_SUBCODE_SIZE, bGetDriveOfs, nDriveSampleOffset, nDriveOffset)) {
			}
			lpCmd[10] = (BYTE)CDFLAG::_PLXTR_READ_CDDA::MainC2Raw;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd, lpBuf
				, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, bGetDriveOfs, nDriveSampleOffset, nDriveOffset)) {
				pExtArg->byC2 = FALSE;
				pDevice->FEATURE.byC2ErrorData = FALSE;
				// not return FALSE
			}
		}
		FreeAndNull(pBuf);
	}
	return bRet;
}

BOOL ReadCDForCheckingReadInOut(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	BOOL bRet = TRUE;
	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pDisc->SCSI.byAudioOnly) {
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
		SetReadCDCommand(pExtArg, pDevice, &cdb, 
			CDFLAG::_READ_CD::CDDA, 1, CDFLAG::_READ_CD::NoSub, TRUE);
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
	)
{
	if (!GetAlignedCallocatedBuffer(pDevice, ppBuf,
		CD_RAW_SECTOR_WITH_SUBCODE_SIZE, lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
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
		SetReadCDCommand(NULL, pDevice, &cdb,
			CDFLAG::_READ_CD::All, 1, CDFLAG::_READ_CD::Raw, TRUE);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	return TRUE;
}

BOOL ReadCDForCheckingSubQAdr(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpCmd,
	LPBYTE lpBuf,
	INT nOfs,
	BYTE byIdxOfTrack,
	LPBYTE byMode,
	BYTE bySessionNum,
	FILE* fpCcd
	)
{
	BOOL bCheckMCN = FALSE;
	BOOL bCheckISRC = FALSE;
	CHAR szTmpCatalog[META_CATALOG_SIZE] = { 0 };
	CHAR szTmpISRC[META_ISRC_SIZE] = { 0 };
	INT nMCNIdx = 0;
	INT nISRCIdx = 0;
	INT nTmpMCNLBAList[9] = { -1 };
	INT nTmpISRCLBAList[9] = { -1 };
	INT nTmpLBA = pDisc->SCSI.lpFirstLBAListOnToc[byIdxOfTrack];

	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_TRACK_F(Check MCN and/or ISRC), byIdxOfTrack + 1);
	for (INT nLBA = nTmpLBA; nLBA < nTmpLBA + 400; nLBA++) {
		if (!ExecReadCD(pExtArg, pDevice, lpCmd, nLBA, lpBuf,
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		LPBYTE lpBuf2 = lpBuf;
		if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
			lpBuf2 = lpBuf + nOfs;
		}
		BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
#if 0
		OutputCDMain(lpBuf2, nLBA, CD_RAW_SECTOR_SIZE);
		OutputCDSub96Align(lpSubcode, nLBA);
#endif
		if (nLBA == nTmpLBA) {
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
	)
{
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
		CDB::_PLXTR_READ_CDDA cdb = { 0 };
		SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::MainPack);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	else {
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(NULL, pDevice, &cdb,
			CDFLAG::_READ_CD::All, 1, CDFLAG::_READ_CD::Raw, TRUE);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}

	for (BYTE i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
		if ((pDisc->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) == 0) {
			try {
				INT nTmpLBA = pDisc->SCSI.lpFirstLBAListOnToc[i] + 100;
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, nTmpLBA, lpBuf,
					CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
				BYTE lpSubcodeOrg[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
				AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
				memcpy(lpSubcodeOrg, lpBuf + CD_RAW_SECTOR_SIZE, CD_RAW_READ_SUBCODE_SIZE);
				OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_TRACK_F(Check CD+G), i + 1);
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
					OutputDiscLogA("\tRtoW is 0xff\n");
				}
				else {
					BOOL bFull = FALSE;
					// 0xff * 12 = 0xbf4
					if (nR == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::RFull;
						OutputDiscLogA("\tAll R is 0xff\n");
						bFull = TRUE;
					}
					if (nS == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::SFull;
						OutputDiscLogA("\tAll S is 0xff\n");
						bFull = TRUE;
					}
					if (nT == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::TFull;
						OutputDiscLogA("\tAll T is 0xff\n");
						bFull = TRUE;
					}
					if (nU == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::UFull;
						OutputDiscLogA("\tAll U is 0xff\n");
						bFull = TRUE;
					}
					if (nV == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::VFull;
						OutputDiscLogA("\tAll V is 0xff\n");
						bFull = TRUE;
					}
					if (nW == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::WFull;
						OutputDiscLogA("\tAll W is 0xff\n");
						bFull = TRUE;
					}
					if (!bFull) {
						if (bCDG && nRtoW > 0 && nRtoW != 0x200) {
							pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::CDG;
							OutputDiscLogA("\tCD+G\n");
						}
						else if (bCDEG && nRtoW > 0 && nRtoW != 0x200) {
							pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::CDG;
							OutputDiscLogA("\tCD+EG\n");
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
		}
		else {
			pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::Zero;
		}
		OutputString(
			_T("\rChecking SubRtoW (Track) %2u/%2u"), i + 1, pDisc->SCSI.toc.LastTrack);
	}
	OutputString(_T("\n"));
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadCDForCheckingExe(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb,
	LPBYTE lpBuf
	)
{
	BOOL bRet = TRUE;
	BYTE byTransferLen = 1;
	DWORD dwSize = DISC_RAW_READ_SIZE;
	SetCommandForTransferLength(pCdb, dwSize, &byTransferLen);
	for (INT n = 0; pDisc->PROTECT.pExtentPosForExe[n] != 0; n++) {
		if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)pCdb, pDisc->PROTECT.pExtentPosForExe[n],
			lpBuf, dwSize, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		WORD wMagic = MAKEWORD(lpBuf[0], lpBuf[1]);
		if (wMagic == IMAGE_DOS_SIGNATURE) {
			PIMAGE_DOS_HEADER pIDh = (PIMAGE_DOS_HEADER)&lpBuf[0];
			if (dwSize < (DWORD)pIDh->e_lfanew) {
				if (pDevice->dwMaxTransferLength < (DWORD)pIDh->e_lfanew) {
					OutputVolDescLogA("%s: offset is very big (%lu). read skip [TODO]\n"
						, pDisc->PROTECT.pNameForExe[n], pIDh->e_lfanew);
				}
				else {
					SetCommandForTransferLength(pCdb, (DWORD)pIDh->e_lfanew, &byTransferLen);
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
					OutputFsImageSectionHeader(pDisc, (PIMAGE_SECTION_HEADER)&lpBuf[nOfs], n);
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

BOOL ReadCDForDirectoryRecordDetail(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb,
	LPBYTE lpBuf,
	BYTE byTransferLen,
	INT nDirPosNum,
	PDIRECTORY_RECORD pDirRec,
	INT nDirRecIdx,
	LPINT pDirCnt
	)
{
	if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)pCdb, (INT)pDirRec[nDirRecIdx].uiPosOfDir, lpBuf,
		(DWORD)(DISC_RAW_READ_SIZE * byTransferLen), _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
#if 1
	OutputLogA(fileMainInfo, "byTransferLen: %d\n", byTransferLen);
	for (BYTE i = 0; i < byTransferLen; i++) {
		OutputCDMain(fileMainInfo, lpBuf + DISC_RAW_READ_SIZE * i
			, (INT)pDirRec[nDirRecIdx].uiPosOfDir + i, DISC_RAW_READ_SIZE);
	}
#endif
	UINT nOfs = 0;
	for (INT nSectorNum = 0; nSectorNum < byTransferLen;) {
		if (*(lpBuf + nOfs) == 0) {
			break;
		}
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Directory Record)
			, pDirRec[nDirRecIdx].uiPosOfDir + nSectorNum, pDirRec[nDirRecIdx].uiPosOfDir + nSectorNum);
			for (;;) {
			CHAR szCurDirName[MAX_FNAME_FOR_VOLUME] = { 0 };
			LPBYTE lpDirRec = lpBuf + nOfs;
			if (lpDirRec[0] >= 0x22) {
				DWORD dwExtentPos = GetSizeOrDwordForVolDesc(lpDirRec + 2);
				DWORD dwDataLen = GetSizeOrDwordForVolDesc(lpDirRec + 10);
				OutputFsDirectoryRecord(
					pExtArg, pDisc, lpDirRec, dwExtentPos, dwDataLen, szCurDirName);
				OutputVolDescLogA("\n");
				nOfs += lpDirRec[0];

				if (lpDirRec[25] & 0x02) {
					for (INT b = *pDirCnt; b < nDirPosNum; b++) {
						if (dwExtentPos == pDirRec[*pDirCnt].uiPosOfDir &&
							!_strnicmp(szCurDirName, pDirRec[*pDirCnt].szDirName, MAX_FNAME_FOR_VOLUME)) {
							pDirRec[*pDirCnt].uiDirSize = PadSizeForVolDesc(dwDataLen);
							*pDirCnt = *pDirCnt + 1;
							break;
						}
					}
				}
				if (nOfs == (UINT)(DISC_RAW_READ_SIZE * (nSectorNum + 1))) {
					nSectorNum++;
					break;
				}
			}
			else {
				UINT zeroPaddingNum = DISC_RAW_READ_SIZE * (nSectorNum + 1) - nOfs;
				if (nSectorNum < byTransferLen) {
					UINT j = 0;
					for (; j < zeroPaddingNum; j++) {
						if (lpDirRec[j] != 0) {
							break;
						}
					}
					if (j == zeroPaddingNum) {
						nOfs += zeroPaddingNum;
						nSectorNum++;
						break;
					}
				}
				else {
					break;
				}
			}
		}
	}
	return TRUE;
}

BOOL ReadCDForDirectoryRecord(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb,
	LPBYTE lpBuf,
	DWORD dwRootDataLen,
	BYTE byTransferLen,
	PDIRECTORY_RECORD pDirRec,
	INT nDirPosNum
	)
{
	INT nDirRecIdx = 0;
	INT nDirIdx = 1;
	BYTE byMaxTransferLen = 0;
	DWORD dwAdditionalTransferLen = 0;
	DWORD dwLastTblSize = 0;
	DWORD dwMostLargeSize = dwRootDataLen;
	do {
		if (!ReadCDForDirectoryRecordDetail(pExtArg, pDevice, pDisc, pCdb
			, lpBuf, byTransferLen, nDirPosNum, pDirRec, nDirRecIdx, &nDirIdx)) {
			return FALSE;
		}
		for (DWORD u = 1; u <= dwAdditionalTransferLen; u++) {
			if (u == dwAdditionalTransferLen) {
				SetCommandForTransferLength(pCdb, dwLastTblSize, &byTransferLen);
			}
			else {
				byTransferLen = byMaxTransferLen;
				pCdb->TransferLength[3] = byTransferLen;
			}
			if (!ReadCDForDirectoryRecordDetail(pExtArg, pDevice, pDisc, pCdb
				, lpBuf, byTransferLen, nDirPosNum, pDirRec, nDirRecIdx, &nDirIdx)) {
				return FALSE;
			}
		}
		dwAdditionalTransferLen = 0;
		nDirRecIdx++;
		if (pDirRec[nDirRecIdx].uiDirSize > dwMostLargeSize) {
			// [FMT] Psychic Detective Series Vol. 4 - Orgel (Japan) (v1.0)
			// [FMT] Psychic Detective Series Vol. 5 - Nightmare (Japan)
			// [IBM - PC compatible] Maria 2 - Jutai Kokuchi no Nazo (Japan) (Disc 1)
			// [IBM - PC compatible] PC Game Best Series Vol. 42 - J.B. Harold Series - Kiss of Murder - Satsui no Kuchizuke (Japan)
			// and more
			if (pDirRec[nDirRecIdx].uiDirSize > pDevice->dwMaxTransferLength) {
				dwMostLargeSize = pDevice->dwMaxTransferLength;
				SetCommandForTransferLength(
					pCdb, pDevice->dwMaxTransferLength, &byMaxTransferLen);
				byTransferLen = byMaxTransferLen;
				dwAdditionalTransferLen =
					pDirRec[nDirRecIdx].uiDirSize / pDevice->dwMaxTransferLength;
				dwLastTblSize =
					pDirRec[nDirRecIdx].uiDirSize % pDevice->dwMaxTransferLength;
			}
			else {
				dwMostLargeSize = pDirRec[nDirRecIdx].uiDirSize;
				SetCommandForTransferLength(pCdb, dwMostLargeSize, &byTransferLen);
			}
		}
		else {
			if (pDirRec[nDirRecIdx].uiDirSize != 0) {
				SetCommandForTransferLength(pCdb, pDirRec[nDirRecIdx].uiDirSize, &byTransferLen);
			}
		}
		OutputString(_T("\rReading DirectoryRecord %4d/%4d"), nDirRecIdx, nDirPosNum);
	} while (nDirRecIdx < nDirPosNum);
	OutputString(_T("\n"));

	return TRUE;
}

BOOL ReadCDForPathTableRecord(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb,
	LPBYTE lpBuf,
	DWORD dwPathTblSize,
	DWORD dwPathTblPos,
	DWORD dwRootDataLen,
	LPBYTE byTransferLen,
	PDIRECTORY_RECORD pDirRec,
	LPINT nDirPosNum
	)
{
	if (dwPathTblSize > DISC_RAW_READ_SIZE) {
		SetCommandForTransferLength(pCdb, dwPathTblSize, byTransferLen);
	}
	if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)pCdb, (INT)dwPathTblPos, lpBuf,
		(DWORD)(DISC_RAW_READ_SIZE * *byTransferLen), _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	OutputFsPathTableRecord(pDisc, lpBuf, dwPathTblPos,
		dwPathTblSize, pDirRec, nDirPosNum);
	OutputVolDescLogA("Dir Num: %u\n", *nDirPosNum);
	// for CD-I
	if (dwRootDataLen == 0) {
		if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)pCdb, (INT)pDirRec[0].uiPosOfDir, lpBuf,
			(DWORD)(DISC_RAW_READ_SIZE * *byTransferLen), _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		dwRootDataLen = PadSizeForVolDesc(GetSizeOrDwordForVolDesc(lpBuf + 10));
	}
	if (dwRootDataLen > 0 && dwRootDataLen != (DWORD)(DISC_RAW_READ_SIZE * *byTransferLen)) {
		SetCommandForTransferLength(pCdb, dwRootDataLen, byTransferLen);
	}
	pDirRec[0].uiDirSize = dwRootDataLen;

	return TRUE;
}

BOOL ReadCDForVolumeDescriptor(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	BYTE byIdx,
	CDB::_READ12* pCdb,
	LPBYTE lpBuf,
	LPBOOL pPVD,
	LPDWORD dwPathTblSize,
	LPDWORD dwPathTblPos,
	LPDWORD dwRootDataLen
	)
{
	INT nPVD = pDisc->SCSI.lpFirstLBAListOnToc[byIdx] + 16;
	INT nTmpLBA = nPVD;
	for (;;) {
		if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)pCdb, nTmpLBA, lpBuf,
			DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		if (!strncmp((LPCH)&lpBuf[1], "CD001", 5) ||
			(pDisc->SCSI.byCdi && !strncmp((LPCH)&lpBuf[1], "CD-I ", 5))) {
			if (nTmpLBA == nPVD) {
				*dwPathTblSize = GetSizeOrDwordForVolDesc(lpBuf + 132);
				*dwPathTblPos = MAKEDWORD(MAKEWORD(lpBuf[140], lpBuf[141]),
					MAKEWORD(lpBuf[142], lpBuf[143]));
				if (*dwPathTblPos == 0) {
					*dwPathTblPos = MAKEDWORD(MAKEWORD(lpBuf[151], lpBuf[150]),
						MAKEWORD(lpBuf[149], lpBuf[148]));
				}
				*dwRootDataLen = GetSizeOrDwordForVolDesc(lpBuf + 166);
				if (*dwRootDataLen > 0) {
					*dwRootDataLen = PadSizeForVolDesc(*dwRootDataLen);
				}
				*pPVD = TRUE;
			}
			OutputFsVolumeDescriptor(pExtArg, pDisc, lpBuf, nTmpLBA++);
		}
		else {
			break;
		}
	}
	return TRUE;
}

BOOL ReadCDFor3DODirectory(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb,
	LPCH pPath,
	INT nLBA
	)
{
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
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	)
{
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
			BOOL bPVD = FALSE;
			PDIRECTORY_RECORD pDirRec = NULL;
			try {
				// general data track disc
				DWORD dwPathTblSize, dwPathTblPos, dwRootDataLen = 0;
				if (!ReadCDForVolumeDescriptor(pExtArg, pDevice, pDisc, i, &cdb
					, lpBuf, &bPVD, &dwPathTblSize, &dwPathTblPos, &dwRootDataLen)) {
					throw FALSE;
				}
				if (bPVD) {
					// TODO: buf size
					pDirRec = (PDIRECTORY_RECORD)calloc(4096, sizeof(DIRECTORY_RECORD));
					if (!pDirRec) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
					INT nDirPosNum = 0;
					BYTE byTransferLen = 1;
					if (!ReadCDForPathTableRecord(pExtArg, pDevice, pDisc, &cdb, lpBuf
						, dwPathTblSize, dwPathTblPos, dwRootDataLen, &byTransferLen, pDirRec, &nDirPosNum)) {
						throw FALSE;
					}
					if (!ReadCDForDirectoryRecord(pExtArg, pDevice, pDisc, &cdb
						, lpBuf, dwRootDataLen, byTransferLen, pDirRec, nDirPosNum)) {
						throw FALSE;
					}
					if (!ReadCDForCheckingExe(pExtArg, pDevice, pDisc, &cdb, lpBuf)) {
						throw FALSE;
					}
					if (pDisc->PROTECT.byExist) {
						OutputLogA(standardErr | fileDisc, "Detected [%s], Skip error from %d to %d\n"
							, pDisc->PROTECT.name, pDisc->PROTECT.ERROR_SECTOR.nExtentPos
							, pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize);
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

BOOL ReadCDForCheckingByteOrder(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDRIVE_DATA_ORDER pOrder
	)
{
	DWORD dwBufLen =
		CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE + pDevice->TRANSFER.dwAdditionalBufLen;
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		dwBufLen, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	if (pExtArg->byD8 || pDevice->byPlxtrDrive) {
		CDB::_PLXTR_READ_CDDA cdb = { 0 };
		SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::MainC2Raw);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	else {
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(pExtArg, pDevice, &cdb
			, CDFLAG::_READ_CD::All, 1, CDFLAG::_READ_CD::Raw, FALSE);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	BOOL bRet = TRUE;
	if (!ExecReadCD(pExtArg, pDevice, lpCmd, 0, lpBuf, dwBufLen, _T(__FUNCTION__), __LINE__)) {
		OutputLogA(standardErr | fileDrive,
			"This drive doesn't support [command: %#02x, subch: %#02x]\n", lpCmd[0], lpCmd[10]);
		*pOrder = DRIVE_DATA_ORDER::NoC2;
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
			*pOrder = DRIVE_DATA_ORDER::MainSubC2;
		}
	}
	FreeAndNull(pBuf);
	return bRet;
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
	)
{
	REVERSE_BYTES(&lpCmd[2], &nLBA);
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB12GENERIC_LENGTH, lpBuf,
		pDevice->TRANSFER.dwBufLen + pDevice->TRANSFER.dwAdditionalBufLen,
		&byScsiStatus, pszFuncName, lLineNum)) {
		if (pExtArg->byReadContinue) {
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
	)
{
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
	return TRUE;
}

BOOL ProcessReadCD(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	UINT uiC2ErrorLBACnt,
	LPBYTE lpCmd,
	INT nLBA,
	PDISC_PER_SECTOR pDiscPerSector
	)
{
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
#if 0
	if (0 <= nLBA && nLBA <= 10) {
		OutputCDMain(fileMainInfo, pDiscPerSector->data.present, nLBA, CD_RAW_SECTOR_SIZE);
	}
#endif
	if (bRet == RETURNED_NO_C2_ERROR_1ST) {
		AlignRowSubcode(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.present);
		if (pC2ErrorPerSector && pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (pExtArg->byReadContinue && pDisc->PROTECT.byExist &&
				(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
				nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize)) {
				// skip check c2 error
				ZeroMemory(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufC2Offset, CD_RAW_READ_C2_294_SIZE);
			}
			else {
				bRet = ContainsC2Error(
					pC2ErrorPerSector, pDevice, pDisc, pDiscPerSector->data.present, uiC2ErrorLBACnt);
			}
		}
		if (!(pExtArg->byReadContinue && pDisc->PROTECT.byExist &&
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
	if (pExtArg->byReadContinue && pDisc->PROTECT.byExist &&
		(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
		nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize)) {
		if (bRet == RETURNED_CONTINUE) {
			if (pC2ErrorPerSector && pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
				// skip check c2 error
				ZeroMemory(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufC2Offset, CD_RAW_READ_C2_294_SIZE);
			}
		}
		// replace sub to sub of prev
		ZeroMemory(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, CD_RAW_READ_SUBCODE_SIZE);
		ZeroMemory(pDiscPerSector->subcode.present, CD_RAW_READ_SUBCODE_SIZE);
		SetBufferFromTmpSubQData(pDiscPerSector->subQ.prev, pDiscPerSector->subcode.present, 0);
	}
	return bRet;
}

BOOL ReadCDForRereadingSector(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	UINT uiC2ErrorLBACnt,
	LPBYTE lpCmd,
	PDISC_PER_SECTOR pDiscPerSector,
	FILE* fpImg
	)
{
	BOOL bProcessRet = RETURNED_NO_C2_ERROR_1ST;
	UINT uiCnt = 0;
	UINT uiContinueCnt = 0;
	if (uiC2ErrorLBACnt > 0) {
		if (pDevice->FEATURE.bySetCDSpeed) {
			OutputString(
				_T("\nChanged reading speed: %lux\n"), pExtArg->dwRereadSpeedNum);
			SetCDSpeed(pExtArg, pDevice, pExtArg->dwRereadSpeedNum);
		}
		// forced fua ripping
		if (!pExtArg->byFua) {
			OutputString(_T("Set the force unit access\n"));
			pExtArg->byFua = TRUE;
		}
	}

	while (uiC2ErrorLBACnt > 0) {
		if (uiCnt == pExtArg->dwMaxRereadNum) {
			OutputString(_T("\nReread reached max: %u"), uiCnt);
			bProcessRet = RETURNED_FALSE;
			break;
		}
		UINT uiC2ErrorLBACntBackup = uiC2ErrorLBACnt;
		uiC2ErrorLBACnt = 0;
		SetC2ErrorBackup(pC2ErrorPerSector, 
			uiC2ErrorLBACntBackup, pDevice->TRANSFER.dwAllBufLen);

		UINT i = 0;
		for (INT nLBA = pC2ErrorPerSector[0].nErrorLBANumBackup; i < uiC2ErrorLBACntBackup; i++) {
			OutputString(
				_T("\rReread times: %4u, Error sector num: %4u/%4u"), 
				uiCnt + 1, i + 1, uiC2ErrorLBACntBackup);
			nLBA = pC2ErrorPerSector[i].nErrorLBANumBackup;
			bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, 
				pC2ErrorPerSector, uiC2ErrorLBACnt, lpCmd, nLBA, pDiscPerSector);

//#define C2TEST
#if defined C2TEST
			if (nLBA == 100 && uiCnt == 1) {
				memset(pDiscPerSector->data.present, 0xff, 2352);
				bProcessRet = RETURNED_EXIST_C2_ERROR;
			}
#endif
			if (bProcessRet == RETURNED_EXIST_C2_ERROR) {
				SetC2ErrorData(pC2ErrorPerSector, nLBA, &uiC2ErrorLBACnt, FALSE);
			}
			else if (bProcessRet == RETURNED_NO_C2_ERROR_1ST) {
				if (pC2ErrorPerSector[i].byErrorFlagBackup == RETURNED_NO_C2_ERROR_1ST) {
					if (ContainsDiffByte(pC2ErrorPerSector, pDiscPerSector->data.present, i)) {
						SetNoC2ErrorExistsByteErrorData(pC2ErrorPerSector, nLBA, &uiC2ErrorLBACnt);
					}
					else {
						LONG lPos = LONG(CD_RAW_SECTOR_SIZE * nLBA - pDisc->MAIN.nCombinedOffset);
						LONG lEndPos = lPos + CD_RAW_SECTOR_SIZE - 1;
						INT nPosOrg = lPos + pDisc->MAIN.nCombinedOffset;
						INT nEndPosOrg = lPos + pDisc->MAIN.nCombinedOffset + CD_RAW_SECTOR_SIZE - 1;
						OutputC2ErrorWithLBALogA(
							"BytePos[%d-%d, %#x-%#x] Reread data matched. Rewrote from [%ld, %#lx] to [%ld, %#lx]\n",
							nLBA, nPosOrg, nEndPosOrg, nPosOrg, nEndPosOrg, lPos, lPos, lEndPos, lEndPos);
						fseek(fpImg, lPos, SEEK_SET);
						// Write track to scrambled again
						WriteMainChannel(pExtArg, pDisc, pDiscPerSector->data.present, nLBA, fpImg);
					}
				}
				else {
					SetNoC2ErrorData(pC2ErrorPerSector, pDiscPerSector->data.present,
						nLBA, pDevice->TRANSFER.dwAllBufLen, &uiC2ErrorLBACnt);
				}
			}
			else if (bProcessRet == RETURNED_CONTINUE) {
				uiContinueCnt++;
			}
			else if (bProcessRet == RETURNED_FALSE) {
				break;
			}
		}
		uiCnt++;
	}
	OutputString(_T("\n"));
	if (uiCnt == 0 && uiC2ErrorLBACnt == 0) {
		OutputLogA(standardOut | fileC2Error, "No C2 errors\n");
	}
	else if (uiCnt > 0 && uiC2ErrorLBACnt == 0 && uiContinueCnt == 0) {
		OutputLogA(standardOut | fileC2Error, 
			"C2 errors was fixed at all\n"
			"But please dump at least twice (if possible, using different drives)\n");
	}
	else if (uiC2ErrorLBACnt > 0 || uiContinueCnt > 0) {
		OutputLogA(standardErr | fileC2Error, 
			"There are unrecoverable errors: %d\n", uiC2ErrorLBACnt);
	}
	return bProcessRet;
}

VOID ExecEccEdc(
	BYTE byReadContinue,
	LPCTSTR pszImgPath,
	_DISC::_PROTECT::_ERROR_SECTOR errorSector
	)
{
	CONST INT nCmdSize = 6;
	CONST INT nStrSize = _MAX_PATH * 2 + nCmdSize;
	_TCHAR str[nStrSize] = { 0 };
	_TCHAR cmd[nCmdSize] = { _T("check") };
	INT nStartLBA = errorSector.nExtentPos;
	INT nEndLBA = errorSector.nExtentPos + errorSector.nSectorSize;
	if (byReadContinue) {
		_tcsncpy(cmd, _T("fix"), sizeof(cmd) / sizeof(cmd[0]));
	}
	if (GetEccEdcCmd(str, nStrSize, cmd, pszImgPath, nStartLBA, nEndLBA)) {
		OutputString(_T("Exec %s\n"), str);
		_tsystem(str);
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
	FILE* fpImg,
	FILE* fpSub,
	FILE* fpC2
	)
{
#if 1
	if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		OutputCDMain(fileMainError,
			pDiscPerSector->mainHeader.present, nLBA, MAINHEADER_MODE1_SIZE);
	}
#endif
	UpdateTmpMainHeader(&pDiscPerSector->mainHeader,
		(LPBYTE)&pDiscPerSector->mainHeader.prev, pDiscPerSector->subQ.prev.byCtl, nMainDataType);
	WriteErrorBuffer(pExecType, pExtArg, pDevice, pDisc, pDiscPerSector,
		scrambled_table, nLBA, fpImg, fpSub, fpC2);
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

BOOL ReadCDAll(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PMAIN_HEADER pMain,
	LPCTSTR pszPath,
	FILE* fpCcd
	)
{
	if (pDisc->SCSI.toc.FirstTrack < 1 || 99 < pDisc->SCSI.toc.FirstTrack ||
		pDisc->SCSI.toc.LastTrack < 1 || 99 < pDisc->SCSI.toc.LastTrack) {
		return FALSE;
	}
	FILE* fpImg = NULL;
	_TCHAR pszOutReverseScmFile[_MAX_PATH] = { 0 };
	_TCHAR pszOutScmFile[_MAX_PATH] = { 0 };
	if (pExtArg->byReverse) {
		if (NULL == (fpImg = CreateOrOpenFile(pszPath, _T("_reverse"),
			pszOutReverseScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	else {
		if (NULL == (fpImg = CreateOrOpenFile(pszPath, NULL,
			pszOutScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	BOOL bRet = TRUE;
	FILE* fpC2 = NULL;
	FILE* fpCue = NULL;
	FILE* fpCueForImg = NULL;
	FILE* fpParse = NULL;
	FILE* fpSub = NULL;
	LPBYTE pBuf = NULL;
	LPBYTE pNextBuf = NULL;
	LPBYTE pNextNextBuf = NULL;
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector = NULL;
	DISC_PER_SECTOR discPerSector = { 0 };
	INT nMainDataType = scrambled;
	if (pExtArg->byBe) {
		nMainDataType = unscrambled;
	}
	memcpy(&discPerSector.mainHeader, pMain, sizeof(MAIN_HEADER));

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
			pszPath, _T("_sub"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpSub = CreateOrOpenFile(
			pszPath, NULL, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		SetBufferSizeForReadCD(pDevice, DRIVE_DATA_ORDER::NoC2);
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (NULL == (fpC2 = CreateOrOpenFile(
				pszPath, NULL, NULL, NULL, NULL, _T(".c2"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			SetBufferSizeForReadCD(pDevice, DRIVE_DATA_ORDER::MainC2Sub);
			DRIVE_DATA_ORDER dataOrder = DRIVE_DATA_ORDER::MainC2Sub;

			if (!InitC2ErrorData(pExtArg, 
				&pC2ErrorPerSector, pDevice->TRANSFER.dwAllBufLen)) {
				throw FALSE;
			}
			if (!ReadCDForCheckingByteOrder(pExtArg, pDevice, &dataOrder)) {
				TerminateC2ErrorData(pExtArg, pDevice, &pC2ErrorPerSector);
				pDevice->FEATURE.byC2ErrorData = FALSE;
				SetBufferSizeForReadCD(pDevice, DRIVE_DATA_ORDER::NoC2);
			}
			if (dataOrder == DRIVE_DATA_ORDER::MainSubC2) {
				OutputDriveLogA(
					"=> Byte order of this drive is main + sub + c2\n");
				SetBufferSizeForReadCD(pDevice, DRIVE_DATA_ORDER::MainSubC2);
			}
			else if (dataOrder == DRIVE_DATA_ORDER::MainC2Sub) {
				OutputDriveLogA(
					"=> Byte order of this drive is main + c2 + sub\n");
			}
		}
#ifdef _DEBUG
		OutputString(
			_T("TransferLen %lu, BufLen %lubyte, AdditionalBufLen %lubyte, AllBufLen %lubyte, BufC2Offset %lubyte, BufSubOffset %lubyte\n"),
			pDevice->TRANSFER.dwTransferLen, pDevice->TRANSFER.dwBufLen,
			pDevice->TRANSFER.dwAdditionalBufLen, pDevice->TRANSFER.dwAllBufLen, 
			pDevice->TRANSFER.dwBufC2Offset, pDevice->TRANSFER.dwBufSubOffset);
#endif
		// store main + (c2) + sub data all
		if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
			pDevice->TRANSFER.dwAllBufLen, &discPerSector.data.present, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		if (1 <= pExtArg->dwSubAddionalNum) {
			if (!GetAlignedCallocatedBuffer(pDevice, &pNextBuf,
				pDevice->TRANSFER.dwAllBufLen, &discPerSector.data.next, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			if (2 <= pExtArg->dwSubAddionalNum) {
				if (!GetAlignedCallocatedBuffer(pDevice, &pNextNextBuf,
					pDevice->TRANSFER.dwAllBufLen, &discPerSector.data.nextNext, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
			}
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		_TCHAR szSubCode[5] = { 0 };
		if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe && !pDisc->SCSI.byAudioOnly) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
				_tcsncpy(szSubCode, _T("Raw"), sizeof(szSubCode) / sizeof(szSubCode[0]));
				SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::MainC2Raw);
			}
			else {
				_tcsncpy(szSubCode, _T("Pack"), sizeof(szSubCode) / sizeof(szSubCode[0]));
				SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::MainPack);
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
			SetReadCDCommand(pExtArg, pDevice, &cdb, type, 1, sub, FALSE);
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		}
		OutputLog(standardOut | fileDisc,
			_T("Set read command: %#x, subcode reading mode: %s\n"), lpCmd[0], szSubCode);

		BYTE lpPrevSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 }; // only use PX-S88T
		// to get prevSubQ
		if (pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXS88T) {
			// Somehow PX-S88T is sliding subchannel +1;
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, -2, discPerSector.data.present,
				pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(discPerSector.data.present + pDevice->TRANSFER.dwBufSubOffset, discPerSector.subcode.present);
			SetTmpSubQDataFromBuffer(&discPerSector.subQ.prev, discPerSector.subcode.present);

			if (!ExecReadCD(pExtArg, pDevice, lpCmd, -1, discPerSector.data.present,
				pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(discPerSector.data.present + pDevice->TRANSFER.dwBufSubOffset, discPerSector.subcode.present);
			SetTmpSubQDataFromBuffer(&discPerSector.subQ.present, discPerSector.subcode.present);
			memcpy(lpPrevSubcode, discPerSector.subcode.present, CD_RAW_READ_SUBCODE_SIZE);
		}
		else {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, -1, discPerSector.data.present,
				pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(discPerSector.data.present + pDevice->TRANSFER.dwBufSubOffset, discPerSector.subcode.present);
			SetTmpSubQDataFromBuffer(&discPerSector.subQ.prev, discPerSector.subcode.present);
		}
		if (discPerSector.subQ.prev.byAdr != ADR_ENCODES_CURRENT_POSITION) {
			// 1552 Tenka Tairan
			discPerSector.subQ.prev.byAdr = ADR_ENCODES_CURRENT_POSITION;
			discPerSector.subQ.prev.byTrackNum = 1;
			discPerSector.subQ.prev.nAbsoluteTime = 149;
		}

		for (UINT p = 0; p < pDisc->SCSI.toc.LastTrack; p++) {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, pDisc->SCSI.lpFirstLBAListOnToc[p]
				, discPerSector.data.present, pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(discPerSector.data.present + pDevice->TRANSFER.dwBufSubOffset, discPerSector.subcode.present);
			pDisc->SUB.lpEndCtlList[p] = (BYTE)((discPerSector.subcode.present[12] >> 4) & 0x0f);
			OutputString(_T("\rChecking SubQ ctl (Track) %2u/%2u"), p + 1, pDisc->SCSI.toc.LastTrack);
		}
		OutputString(_T("\n"));
		SetCDOffset(pExtArg->byBe, pDisc, 0, pDisc->SCSI.nAllLength);

		BYTE byCurrentTrackNum = pDisc->SCSI.toc.FirstTrack;
		INT nFirstLBAForSub = 0;
		INT nFirstLBA = pDisc->MAIN.nOffsetStart;
		INT nLastLBA = 0;
		if (pExtArg->byBe) {
			nLastLBA = pDisc->SCSI.nAllLength;
		}
		else {
			nLastLBA = pDisc->SCSI.nAllLength + pDisc->MAIN.nOffsetEnd;
		}
		INT nLBA = nFirstLBA;	// This value switches by /r option.

		if (pExtArg->byReverse) {
			SetCDOffset(pExtArg->byBe, pDisc, nLastLBA, pDisc->SCSI.nFirstLBAofDataTrack);
			byCurrentTrackNum = pDisc->SCSI.byLastDataTrackNum;
			nFirstLBA = pDisc->SCSI.nFirstLBAofDataTrack;
			nLastLBA = pDisc->SCSI.nLastLBAofDataTrack + 1;
			nLBA = nLastLBA;
			if (pDisc->MAIN.nCombinedOffset > 0) {
				discPerSector.subQ.prev.nAbsoluteTime = 149 + nLastLBA;
			}
			else if (pDisc->MAIN.nCombinedOffset < 0) {
				discPerSector.subQ.prev.nAbsoluteTime = 150 + nLastLBA + pDisc->MAIN.nAdjustSectorNum;
			}
			else {
				discPerSector.subQ.prev.nAbsoluteTime = 149 + nLastLBA;
			}
			discPerSector.subQ.prev.byCtl = pDisc->SUB.lpEndCtlList[pDisc->SCSI.byLastDataTrackNum - 1];
			discPerSector.subQ.prev.byAdr = ADR_ENCODES_CURRENT_POSITION;
			discPerSector.subQ.prev.byTrackNum = pDisc->SCSI.byLastDataTrackNum;
			discPerSector.subQ.prev.byIndex = pDisc->MAIN.nOffsetStart < 0 ? (BYTE)0 : (BYTE)1;
		}
		else if (pDisc->SUB.byIndex0InTrack1) {
			nFirstLBAForSub = PREGAP_START_LBA;
			nFirstLBA = PREGAP_START_LBA;
			nLBA = nFirstLBA;
			pDisc->MAIN.nOffsetStart = PREGAP_START_LBA;
		}
		// init end
		FlushLog();

		UINT uiC2ErrorLBACnt = 0;
		BOOL bReadOK = pDisc->SUB.byIndex0InTrack1 ? FALSE : TRUE;

		while (nFirstLBA < nLastLBA) {
			BOOL bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc,
				pC2ErrorPerSector, uiC2ErrorLBACnt, lpCmd, nLBA, &discPerSector);
//#define C2TEST
#if defined C2TEST
			if (nLBA == 100 || nLBA == 200 || nLBA == 300) {
				memset(discPerSector.data.present, 0xff, 2352);
				bProcessRet = RETURNED_EXIST_C2_ERROR;
			}
#endif
			if (pC2ErrorPerSector && bProcessRet == RETURNED_EXIST_C2_ERROR) {
				OutputErrorString(
					_T("\rLBA[%06d, %#07x] Detected C2 error    \n"), nLBA, nLBA);
				SetC2ErrorData(pC2ErrorPerSector, nLBA, &uiC2ErrorLBACnt, TRUE);
				if (uiC2ErrorLBACnt == pExtArg->dwMaxC2ErrorNum) {
					OutputErrorString(_T("C2 error Max: %u\n"), uiC2ErrorLBACnt);
					throw FALSE;
				}
#ifdef _DEBUG
				OutputCDMain(fileMainError,
					discPerSector.data.present, nLBA, CD_RAW_SECTOR_SIZE);
#endif
			}
			else if (bProcessRet == RETURNED_SKIP_LBA) {
				if (pExtArg->byReverse) {
					nLBA = pDisc->SCSI.nFirstLBAof2ndSession - SESSION_TO_SESSION_SKIP_LBA;
				}
				else {
					nLBA = pDisc->MAIN.nFixFirstLBAof2ndSession - 1;
					nFirstLBA = nLBA;
				}
			}
			else if (bProcessRet == RETURNED_CONTINUE) {
				ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
					, &discPerSector, nLBA, nMainDataType, fpImg, fpSub, fpC2);
			}
			else if (bProcessRet == RETURNED_FALSE) {
				throw FALSE;
			}
			if (bProcessRet != RETURNED_CONTINUE &&
				bProcessRet != RETURNED_SKIP_LBA) {
				if (pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXS88T) {
					memcpy(discPerSector.subcode.nextNext, discPerSector.subcode.next, CD_RAW_READ_SUBCODE_SIZE);
					memcpy(discPerSector.subcode.next, discPerSector.subcode.present, CD_RAW_READ_SUBCODE_SIZE);
					memcpy(discPerSector.subcode.present, lpPrevSubcode, CD_RAW_READ_SUBCODE_SIZE);
				}
				SetTmpSubQDataFromBuffer(&discPerSector.subQ.present, discPerSector.subcode.present);

				if (pDisc->SUB.byIndex0InTrack1 && PREGAP_START_LBA <= nLBA && nLBA <= -76) {
					if (discPerSector.subQ.present.byTrackNum == 1 && discPerSector.subQ.present.nAbsoluteTime == 0) {
						discPerSector.subQ.prev.nRelativeTime = discPerSector.subQ.present.nRelativeTime + 1;
						discPerSector.subQ.prev.nAbsoluteTime = -1;
						pDisc->MAIN.nFixStartLBA = nLBA;
						bReadOK = TRUE;
						if (pDisc->MAIN.nAdjustSectorNum < 0 ||
							1 < pDisc->MAIN.nAdjustSectorNum) {
							for (INT i = 0; i < abs(pDisc->MAIN.nAdjustSectorNum) * CD_RAW_SECTOR_SIZE; i++) {
								fputc(0, fpImg);
							}
						}
					}
					if (bReadOK) {
						if (discPerSector.subQ.present.byTrackNum == 1 && discPerSector.subQ.present.nAbsoluteTime == 74) {
							nFirstLBA = -76;
						}
					}
				}
				if (bReadOK) {
					if (nFirstLBAForSub <= nLBA && nLBA < pDisc->SCSI.nAllLength) {
						if (ExecCheckingSubchannnel(pExtArg, pDisc, nLBA)) {
							BOOL bLibCrypt = IsValidLibCryptSector(pExtArg->byLibCrypt, nLBA);
							BOOL bSecuRom = IsValidIntentionalSubErrorSector(pExtArg->byIntentionalSub, pDisc, nLBA);
							if (!pExtArg->byReverse) {
								CheckAndFixSubChannel(pExecType, pExtArg, pDevice, pDisc
									, &discPerSector, byCurrentTrackNum, nLBA, bLibCrypt, bSecuRom);
								BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
								// fix raw subchannel
								AlignColumnSubcode(discPerSector.subcode.present, lpSubcodeRaw);
#if 0
								OutputCDSub96Align(discPerSector.subcode.present, nLBA);
#endif
								WriteSubChannel(pDisc, lpSubcodeRaw,
									discPerSector.subcode.present, nLBA, byCurrentTrackNum, fpSub, fpParse);
							}
							CheckAndFixMainHeader(pExtArg, pDisc
								, &discPerSector, nLBA, byCurrentTrackNum, nMainDataType);
							SetTrackAttribution(pExtArg, pDisc, nLBA,
								&byCurrentTrackNum, &discPerSector.mainHeader, &discPerSector.subQ);
							UpdateTmpSubQData(&discPerSector.subQ, bLibCrypt, bSecuRom);
						}
					}
					// Write track to scrambled
					WriteMainChannel(pExtArg, pDisc, discPerSector.data.present, nLBA, fpImg);
					if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
						WriteC2(pExtArg, pDisc, discPerSector.data.present + pDevice->TRANSFER.dwBufC2Offset, nLBA, fpC2);
					}
				}
				// for DEBUG begin
				else {
					BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
					AlignColumnSubcode(discPerSector.subcode.present, lpSubcodeRaw);
					OutputCDSubToLog(pDisc, discPerSector.subcode.present, lpSubcodeRaw, nLBA, byCurrentTrackNum, fpParse);
				}
				// for DEBUG end
				if (pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXS88T) {
					memcpy(lpPrevSubcode, discPerSector.subcode.next, CD_RAW_READ_SUBCODE_SIZE);
				}
			}

			if (pExtArg->byReverse) {
				OutputString(_T("\rCreated img (LBA) %6d/%6d"),
					nLBA, pDisc->SCSI.nFirstLBAofDataTrack);
				nLBA--;
			}
			else {
				OutputString(_T("\rCreated img (LBA) %6d/%6d"),
					nLBA, pDisc->SCSI.nAllLength - 1);
				if (nFirstLBA == -76) {
					nLBA = nFirstLBA;
					if (!bReadOK) {
						bReadOK = TRUE;
					}
				}
				nLBA++;
			}
			nFirstLBA++;
		}
		OutputString(_T("\n"));
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			FcloseAndNull(fpC2);
		}
		FcloseAndNull(fpParse);
		FcloseAndNull(fpSub);
		FlushLog();

		if (!pExtArg->byReverse) {
			if (pDisc->SCSI.toc.FirstTrack == pDisc->SCSI.toc.LastTrack) {
				pDisc->SUB.lpFirstLBAListOnSub[0][1] = pDisc->SCSI.lpLastLBAListOnToc[0];
			}
			for (INT i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
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
		}
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (!ReadCDForRereadingSector(pExecType, pExtArg, pDevice, pDisc,
				pC2ErrorPerSector, uiC2ErrorLBACnt, lpCmd, &discPerSector, fpImg)) {
				throw FALSE;
			}
		}
		FcloseAndNull(fpImg);

		if (!pExtArg->byReverse) {
			OutputTocWithPregap(pDisc);
		}
		else {
			FILE* fpImg_r = NULL;
			if (NULL == (fpImg_r = CreateOrOpenFile(
				pszPath, _T("_reverse"), NULL, NULL, NULL, _T(".scm"), _T("rb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (fpImg = CreateOrOpenFile(
				pszPath, NULL, pszOutScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			OutputString(_T("Reversing _reverse.scm to .scm\n"));
			BYTE rBuf[CD_RAW_SECTOR_SIZE] = { 0 };
			DWORD dwRoop = GetFileSize(0, fpImg_r) - CD_RAW_SECTOR_SIZE * 2;
			LONG lSeek = CD_RAW_SECTOR_SIZE - (LONG)pDisc->MAIN.uiMainDataSlideSize;
			fseek(fpImg_r, -lSeek, SEEK_END);
			fread(rBuf, sizeof(BYTE), (size_t)lSeek, fpImg_r);
			fwrite(rBuf, sizeof(BYTE), (size_t)lSeek, fpImg);
			fseek(fpImg_r, -lSeek, SEEK_CUR);
			for (DWORD i = 0; i < dwRoop; i += CD_RAW_SECTOR_SIZE) {
				fseek(fpImg_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
				fread(rBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg_r);
				fwrite(rBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
				fseek(fpImg_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
			}
			rewind(fpImg_r);
			fread(rBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpImg_r);
			fwrite(rBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpImg);
			FcloseAndNull(fpImg);
			FcloseAndNull(fpImg_r);
		}

		_TCHAR pszNewPath[_MAX_PATH] = { 0 };
		_tcsncpy(pszNewPath, pszPath, sizeof(pszNewPath) / sizeof(pszNewPath[0]));
		pszNewPath[_MAX_PATH - 1] = 0;
		if (!PathRenameExtension(pszNewPath, _T(".img"))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		// audio only -> from .scm to .img. other descramble img.
		if (pExtArg->byBe || pDisc->SCSI.byAudioOnly) {
			OutputString(_T("Moving .scm to .img\n"));
			if (!MoveFileEx(pszOutScmFile, pszNewPath, MOVEFILE_REPLACE_EXISTING)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (pExtArg->byBe) {
				ExecEccEdc(pExtArg->byReadContinue, pszNewPath, pDisc->PROTECT.ERROR_SECTOR);
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
			DescrambleMainChannel(pExtArg, pDisc, scrambled_table, fpImg);
			FcloseAndNull(fpImg);
			ExecEccEdc(pExtArg->byReadContinue, pszImgPath, pDisc->PROTECT.ERROR_SECTOR);
		}

		if (pExtArg->byReverse) {
			_TCHAR pszNewPath2[_MAX_PATH] = { 0 };
			FILE* fpBin = CreateOrOpenFile(pszPath, NULL, pszNewPath2, NULL, NULL, _T(".bin"),
				_T("wb"), pDisc->SCSI.byFirstDataTrackNum, pDisc->SCSI.byFirstDataTrackNum);
			if (!fpBin) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			OutputString(_T("Copying .img to %s\n"), pszNewPath2);
			if (!CopyFile(pszNewPath, pszNewPath2, FALSE)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FcloseAndNull(fpBin);
		}
		else {
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
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpImg);
	if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		FcloseAndNull(fpC2);
	}
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
	TerminateC2ErrorData(pExtArg, pDevice, &pC2ErrorPerSector);

	return bRet;
}

BOOL ReadCDPartial(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PMAIN_HEADER pMain,
	LPCTSTR pszPath,
	INT nStart,
	INT nEnd,
	CDFLAG::_READ_CD::_EXPECTED_SECTOR_TYPE flg
	)
{
	CONST INT size = 8;
	_TCHAR szPlusFnameSub[size] = { 0 };
	_TCHAR szPlusFnameTxt[size] = { 0 };
	_TCHAR szExt[size] = { 0 };

	if (*pExecType == gd) {
		_tcsncpy(szPlusFnameSub, _T("_dc"), size);
		_tcsncpy(szPlusFnameTxt, _T("_dc_sub"), size);
		_tcsncpy(szExt, _T(".scm2"), size);
	}
	else {
		_tcsncpy(szPlusFnameTxt, _T("_sub"), size);
		_tcsncpy(szExt, _T(".bin"), size);
	}
	FILE* fpBin = 
		CreateOrOpenFile(pszPath, NULL, NULL, NULL, NULL, szExt, _T("wb"), 0, 0);
	if (!fpBin) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	FILE* fpC2 = NULL;
	FILE* fpParse = NULL;
	FILE* fpSub = NULL;
	LPBYTE pBuf = NULL;
	LPBYTE pNextBuf = NULL;
	LPBYTE pNextNextBuf = NULL;
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector = NULL;
	DISC_PER_SECTOR discPerSector = { 0 };
	INT nMainDataType = scrambled;
	if (*pExecType == data || pExtArg->byBe) {
		nMainDataType = unscrambled;
	}
	memcpy(&discPerSector.mainHeader, pMain, sizeof(MAIN_HEADER));

	try {
		// init start
		if (NULL == (fpParse = CreateOrOpenFile(
			pszPath, szPlusFnameTxt, NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpSub = CreateOrOpenFile(
			pszPath, szPlusFnameSub, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		SetBufferSizeForReadCD(pDevice, DRIVE_DATA_ORDER::NoC2);
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (NULL == (fpC2 = CreateOrOpenFile(
				pszPath, NULL, NULL, NULL, NULL, _T(".c2"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			SetBufferSizeForReadCD(pDevice, DRIVE_DATA_ORDER::MainC2Sub);
			DRIVE_DATA_ORDER dataOrder = DRIVE_DATA_ORDER::MainC2Sub;

			if (!InitC2ErrorData(pExtArg,
				&pC2ErrorPerSector, pDevice->TRANSFER.dwAllBufLen)) {
				throw FALSE;
			}
			if (!ReadCDForCheckingByteOrder(pExtArg, pDevice, &dataOrder)) {
				TerminateC2ErrorData(pExtArg, pDevice, &pC2ErrorPerSector);
				pDevice->FEATURE.byC2ErrorData = FALSE;
				SetBufferSizeForReadCD(pDevice, DRIVE_DATA_ORDER::NoC2);
			}
			if (dataOrder == DRIVE_DATA_ORDER::MainSubC2) {
				OutputDriveLogA(
					"=> Byte order of this drive is main + sub + c2\n");
				SetBufferSizeForReadCD(pDevice, DRIVE_DATA_ORDER::MainSubC2);
			}
			else if (dataOrder == DRIVE_DATA_ORDER::MainC2Sub) {
				OutputDriveLogA(
					"=> Byte order of this drive is main + c2 + sub\n");
			}
		}
		// store main+(c2)+sub data
		if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
			pDevice->TRANSFER.dwAllBufLen, &discPerSector.data.present, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		if (1 <= pExtArg->dwSubAddionalNum) {
			if (!GetAlignedCallocatedBuffer(pDevice, &pNextBuf,
				pDevice->TRANSFER.dwAllBufLen, &discPerSector.data.next, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			if (2 <= pExtArg->dwSubAddionalNum) {
				if (!GetAlignedCallocatedBuffer(pDevice, &pNextNextBuf,
					pDevice->TRANSFER.dwAllBufLen, &discPerSector.data.nextNext, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
			}
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		_TCHAR szSubCode[5] = { 0 };
		if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe && !pDisc->SCSI.byAudioOnly) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
				_tcsncpy(szSubCode, _T("Raw"), sizeof(szSubCode) / sizeof(szSubCode[0]));
				SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::MainC2Raw);
			}
			else {
				_tcsncpy(szSubCode, _T("Pack"), sizeof(szSubCode) / sizeof(szSubCode[0]));
				SetReadD8Command(pDevice, &cdb, 1, CDFLAG::_PLXTR_READ_CDDA::MainPack);
			}
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		}
		else {
			// non plextor && support scrambled ripping
			CDB::_READ_CD cdb = { 0 };
			_tcsncpy(szSubCode, _T("Raw"), sizeof(szSubCode) / sizeof(szSubCode[0]));
			SetReadCDCommand(pExtArg, pDevice, &cdb, flg, 1, CDFLAG::_READ_CD::Raw, FALSE);
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		}
		OutputLog(standardOut | fileDisc,
			_T("Set read command: %#x, subcode reading mode: %s\n"), lpCmd[0], szSubCode);

		BYTE lpPrevSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 }; // only use PX-S88T
		if (pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXS88T) {
			// Somehow PX-S88T is sliding subchannel +1;
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, nStart - 2, discPerSector.data.present,
				pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(discPerSector.data.present + pDevice->TRANSFER.dwBufSubOffset, discPerSector.subcode.present);
			SetTmpSubQDataFromBuffer(&discPerSector.subQ.prev, discPerSector.subcode.present);

			if (!ExecReadCD(pExtArg, pDevice, lpCmd, nStart - 1, discPerSector.data.present,
				pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(discPerSector.data.present + pDevice->TRANSFER.dwBufSubOffset, discPerSector.subcode.present);
			SetTmpSubQDataFromBuffer(&discPerSector.subQ.present, discPerSector.subcode.present);
			memcpy(lpPrevSubcode, discPerSector.subcode.present, CD_RAW_READ_SUBCODE_SIZE);
		}
		else {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, nStart - 1, discPerSector.data.present,
				pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(discPerSector.data.present + pDevice->TRANSFER.dwBufSubOffset, discPerSector.subcode.present);
			SetTmpSubQDataFromBuffer(&discPerSector.subQ.prev, discPerSector.subcode.present);
		}

		if (*pExecType == gd) {
			for (INT p = pDisc->GDROM_TOC.FirstTrack - 1; p < pDisc->GDROM_TOC.LastTrack; p++) {
				pDisc->SUB.lpEndCtlList[p] = pDisc->GDROM_TOC.TrackData[p].Control;
			}
		}
		else {
			for (UINT p = 0; p < pDisc->SCSI.toc.LastTrack; p++) {
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, pDisc->SCSI.lpFirstLBAListOnToc[p]
					, discPerSector.data.present, pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				AlignRowSubcode(discPerSector.data.present + pDevice->TRANSFER.dwBufSubOffset, discPerSector.subcode.present);
				pDisc->SUB.lpEndCtlList[p] = (BYTE)((discPerSector.subcode.present[12] >> 4) & 0x0f);
				OutputString(_T("\rChecking SubQ ctl (Track) %2u/%2u"), p + 1, pDisc->SCSI.toc.LastTrack);
			}
			OutputString(_T("\n"));
		}
		SetCDOffset(pExtArg->byBe, pDisc, nStart, nEnd);

		BYTE byCurrentTrackNum = discPerSector.subQ.prev.byTrackNum;
		if (*pExecType == gd) {
			byCurrentTrackNum = pDisc->GDROM_TOC.FirstTrack;
			// because address out of range
			pDisc->MAIN.nOffsetEnd = 0;
		}
		else if (byCurrentTrackNum < pDisc->SCSI.toc.FirstTrack || pDisc->SCSI.toc.LastTrack < byCurrentTrackNum) {
			byCurrentTrackNum = pDisc->SCSI.toc.FirstTrack;
		}
		INT nLastLBA = nEnd + pDisc->MAIN.nOffsetEnd;
		INT nLBA = nStart + pDisc->MAIN.nOffsetStart;
		// init end
		FlushLog();

		UINT uiC2ErrorLBACnt = 0;
		while (nLBA < nLastLBA) {
			BOOL bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, 
				pC2ErrorPerSector, uiC2ErrorLBACnt, lpCmd, nLBA, &discPerSector);
			if (pC2ErrorPerSector && bProcessRet == RETURNED_EXIST_C2_ERROR) {
				OutputErrorString(
					_T("\rLBA[%06d, %#07x] Detected C2 error    \n"), nLBA, nLBA);
				SetC2ErrorData(pC2ErrorPerSector, nLBA, &uiC2ErrorLBACnt, TRUE);
				if (uiC2ErrorLBACnt == pExtArg->dwMaxC2ErrorNum) {
					OutputErrorString(_T("C2 error Max: %u\n"), uiC2ErrorLBACnt);
					throw FALSE;
				}
			}
			else if (bProcessRet == RETURNED_SKIP_LBA) {
				nLBA = pDisc->MAIN.nFixFirstLBAof2ndSession - 1;
			}
			else if (bProcessRet == RETURNED_CONTINUE) {
				ProcessReturnedContinue(pExecType, pExtArg, pDevice, pDisc
					, &discPerSector, nLBA, nMainDataType, fpBin, fpSub, fpC2);
			}
			else if (bProcessRet == RETURNED_FALSE) {
				throw FALSE;
			}
			if (bProcessRet != RETURNED_CONTINUE &&
				bProcessRet != RETURNED_SKIP_LBA) {
				if (pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXS88T) {
					memcpy(discPerSector.subcode.nextNext, discPerSector.subcode.next, CD_RAW_READ_SUBCODE_SIZE);
					memcpy(discPerSector.subcode.next, discPerSector.subcode.present, CD_RAW_READ_SUBCODE_SIZE);
					memcpy(discPerSector.subcode.present, lpPrevSubcode, CD_RAW_READ_SUBCODE_SIZE);
				}
				SetTmpSubQDataFromBuffer(&discPerSector.subQ.present, discPerSector.subcode.present);

				if (nStart <= nLBA && nLBA < nEnd) {
					if (ExecCheckingSubchannnel(pExtArg, pDisc, nLBA)) {
						CheckAndFixSubChannel(pExecType, pExtArg, pDevice, pDisc
							, &discPerSector, byCurrentTrackNum, nLBA, FALSE, FALSE);
						BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
						// fix raw subchannel
						AlignColumnSubcode(discPerSector.subcode.present, lpSubcodeRaw);
#if 0
						OutputCDSub96Align(discPerSector.subcode.present, nLBA);
#endif
						WriteSubChannel(pDisc, lpSubcodeRaw,
							discPerSector.subcode.present, nLBA, byCurrentTrackNum, fpSub, fpParse);
						CheckAndFixMainHeader(pExtArg, pDisc
							, &discPerSector, nLBA, byCurrentTrackNum, nMainDataType);
						if (*pExecType == gd) {
							byCurrentTrackNum = discPerSector.subQ.present.byTrackNum;
						}
						else {
							SetTrackAttribution(pExtArg, pDisc, nLBA,
								&byCurrentTrackNum, &discPerSector.mainHeader, &discPerSector.subQ);
						}
						UpdateTmpSubQData(&discPerSector.subQ, FALSE, FALSE);
					}
				}
				// Write track to scrambled
				WriteMainChannel(pExtArg, pDisc, discPerSector.data.present, nLBA, fpBin);
				if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					WriteC2(pExtArg, pDisc, discPerSector.data.present + pDevice->TRANSFER.dwBufC2Offset, nLBA, fpC2);
				}
				if (pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXS88T) {
					memcpy(lpPrevSubcode, discPerSector.subcode.next, CD_RAW_READ_SUBCODE_SIZE);
				}
			}
			OutputString(_T("\rCreating bin from %d to %d (LBA) %6d"), 
				nStart + pDisc->MAIN.nOffsetStart, nEnd + pDisc->MAIN.nOffsetEnd, nLBA);
			nLBA++;
		}
		OutputString(_T("\n"));
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			FcloseAndNull(fpC2);
		}
		FcloseAndNull(fpParse);
		FcloseAndNull(fpSub);
		FlushLog();

		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (!ReadCDForRereadingSector(pExecType, pExtArg, pDevice, pDisc,
				pC2ErrorPerSector, uiC2ErrorLBACnt, lpCmd, &discPerSector, fpBin)) {
				throw FALSE;
			}
		}
		FcloseAndNull(fpBin);
		if (*pExecType == data) {
			ExecEccEdc(FALSE, pszPath, pDisc->PROTECT.ERROR_SECTOR);
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpBin);
	if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		FcloseAndNull(fpC2);
	}
	FcloseAndNull(fpParse);
	FcloseAndNull(fpSub);
	FreeAndNull(pBuf);
	if (1 <= pExtArg->dwSubAddionalNum) {
		FreeAndNull(pNextBuf);
		if (2 <= pExtArg->dwSubAddionalNum) {
			FreeAndNull(pNextNextBuf);
		}
	}
	TerminateC2ErrorData(pExtArg, pDevice, &pC2ErrorPerSector);

	if (bRet && *pExecType == gd) {
		if (!DescrambleMainChannelForGD(pszPath)) {
			return FALSE;
		}
		if (!SplitFileForGD(pszPath)) {
			return FALSE;
		}
	}
	return bRet;
}

BOOL ReadCDForGDTOC(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	CDB::_READ_CD cdb = { 0 };
	SetReadCDCommand(NULL, pDevice, &cdb,
		CDFLAG::_READ_CD::All, 1, CDFLAG::_READ_CD::NoSub, TRUE);
	BYTE aToc[CD_RAW_SECTOR_SIZE] = { 0 };
	if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, FIRST_LBA_FOR_GD, aToc,
		CD_RAW_SECTOR_SIZE, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}

	BYTE bufDec[CD_RAW_SECTOR_SIZE] = { 0 };
	INT idx = pDisc->MAIN.nCombinedOffset;
	for (INT j = 0; j < CD_RAW_SECTOR_SIZE; j++) {
		bufDec[j] = (BYTE)(aToc[idx + j] ^ scrambled_table[j]);
	}
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
	return TRUE;
}
