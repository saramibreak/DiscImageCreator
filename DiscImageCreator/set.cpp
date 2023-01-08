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
#include "check.h"
#include "convert.h"
#include "get.h"
#include "output.h"
#include "outputScsiCmdLog.h"
#include "calcHash.h"

VOID SetReadCDCommand(
	PDEVICE pDevice,
	CDB::_READ_CD* cdb,
	CDFLAG::_READ_CD::_EXPECTED_SECTOR_TYPE type,
	DWORD dwTransferLen,
	CDFLAG::_READ_CD::_ERROR_FLAGS c2,
	CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION Sub
) {
	cdb->OperationCode = SCSIOP_READ_CD;
	cdb->ExpectedSectorType = (UCHAR)(type & 0x07);
	cdb->Lun = (UCHAR)(pDevice->address.Lun & 0x07);
	cdb->TransferBlocks[0] = LOBYTE(HIWORD(dwTransferLen));
	cdb->TransferBlocks[1] = HIBYTE(LOWORD(dwTransferLen));
	cdb->TransferBlocks[2] = LOBYTE(LOWORD(dwTransferLen));
	cdb->ErrorFlags = (UCHAR)(c2 & 0x03);
	cdb->IncludeEDC = TRUE;
	cdb->IncludeUserData = TRUE;
	cdb->HeaderCode = CDFLAG::_READ_CD::BothHeader;
	cdb->IncludeSyncData = TRUE;
	cdb->SubChannelSelection = (UCHAR)(Sub & 0x07);
}

VOID SetReadD8Command(
	PDEVICE pDevice,
	CDB::_PLXTR_READ_CDDA* cdb,
	DWORD dwTransferLen,
	CDFLAG::_PLXTR_READ_CDDA::_SUB_CHANNEL_SELECTION Sub
) {
	cdb->OperationCode = SCSIOP_PLXTR_READ_CDDA;
	cdb->LogicalUnitNumber = (UCHAR)(pDevice->address.Lun & 0x07);
	cdb->TransferBlockByte0 = HIBYTE(HIWORD(dwTransferLen));
	cdb->TransferBlockByte1 = LOBYTE(HIWORD(dwTransferLen));
	cdb->TransferBlockByte2 = HIBYTE(LOWORD(dwTransferLen));
	cdb->TransferBlockByte3 = LOBYTE(LOWORD(dwTransferLen));
	cdb->SubCode = (UCHAR)Sub;
}

VOID SetReadDiscCommand(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BYTE byTransferLen,
	CDFLAG::_READ_CD::_ERROR_FLAGS c2,
	CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION tmpsub,
	LPBYTE lpCmd,
	BOOL bOutputLog
) {
	_TCHAR szSubCode[5] = {};
	if ((pExtArg->byD8 || pDevice->byPlxtrDrive) && !pExtArg->byBe && !pExtArg->byReverse) {
		CDB::_PLXTR_READ_CDDA cdb = {};
		if (pDevice->sub == CDFLAG::_READ_CD::NoSub || tmpsub == CDFLAG::_READ_CD::NoSub) {
			SetReadD8Command(pDevice, &cdb, byTransferLen, CDFLAG::_PLXTR_READ_CDDA::NoSub);
		}
		else if (pDevice->sub == CDFLAG::_READ_CD::Pack || tmpsub == CDFLAG::_READ_CD::Pack) {
			_tcsncpy(szSubCode, _T("Pack"), sizeof(szSubCode) / sizeof(szSubCode[0]));
			SetReadD8Command(pDevice, &cdb, byTransferLen, CDFLAG::_PLXTR_READ_CDDA::MainPack);
		}
		else if (pDevice->sub == CDFLAG::_READ_CD::Raw || tmpsub == CDFLAG::_READ_CD::Raw) {
			_tcsncpy(szSubCode, _T("Raw"), sizeof(szSubCode) / sizeof(szSubCode[0]));
			SetReadD8Command(pDevice, &cdb, byTransferLen, CDFLAG::_PLXTR_READ_CDDA::MainC2Raw);
		}
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	else {
		// non plextor && support scrambled ripping
		CDB::_READ_CD cdb = {};
		CDFLAG::_READ_CD::_EXPECTED_SECTOR_TYPE type = CDFLAG::_READ_CD::CDDA;
		if (pExtArg->byBe || (pExecType != NULL && *pExecType == data)) {
			type = CDFLAG::_READ_CD::All;
		}
		CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION sub = tmpsub;
		if (pExtArg->byPack || tmpsub == CDFLAG::_READ_CD::Pack) {
			_tcsncpy(szSubCode, _T("Pack"), sizeof(szSubCode) / sizeof(szSubCode[0]));
		}
		else if (pExtArg->byRaw || tmpsub == CDFLAG::_READ_CD::Raw) {
			_tcsncpy(szSubCode, _T("Raw"), sizeof(szSubCode) / sizeof(szSubCode[0]));
		}
		SetReadCDCommand(pDevice, &cdb, type, byTransferLen, c2, sub);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	if (bOutputLog) {
#ifdef _DEBUG
		OutputLog(standardOut | fileDisc,
			"Set Cmd: %#02x, %#02x, %#02x, %#02x, %#02x, %#02x, %#02x, %#02x, %#02x, %#02x, %#02x, %#02x\n"
			, lpCmd[0], lpCmd[1], lpCmd[2], lpCmd[3], lpCmd[4], lpCmd[5]
			, lpCmd[6], lpCmd[7], lpCmd[8], lpCmd[9], lpCmd[10], lpCmd[11]
		);
#else
		OutputLog(standardOut | fileDisc,
			"Set OpCode: %#02x, SubCode: %x(%s)\n", lpCmd[0], lpCmd[10], szSubCode);
#endif
	}
}

VOID SetCommandForTransferLength(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	LPBYTE pCdb,
	DWORD dwSize,
	LPBYTE lpTransferLen,
	LPBYTE lpRoopLen
) {
	if (*pExecType == gd) {
		if (dwSize == pDevice->dwMaxTransferLength * DISC_MAIN_DATA_SIZE / CD_RAW_SECTOR_SIZE) {
			*lpTransferLen = (BYTE)(pDevice->dwMaxTransferLength / CD_RAW_SECTOR_SIZE);
		}
		else {
			*lpTransferLen = (BYTE)(dwSize / DISC_MAIN_DATA_SIZE + 1);
		}
	}
	else {
		*lpTransferLen = (BYTE)(dwSize / DISC_MAIN_DATA_SIZE);
	}
	// Generally, directory size is per 2048 bytes
	// Exception:
	//  Codename - Outbreak (Europe) (Sold Out Software)
	//  Commandos - Behind Enemy Lines (Europe) (Sold Out Software)
	// and more
	if (dwSize % DISC_MAIN_DATA_SIZE != 0) {
		if (!(*pExecType == gd && dwSize == pDevice->dwMaxTransferLength * DISC_MAIN_DATA_SIZE / CD_RAW_SECTOR_SIZE)) {
			(*lpTransferLen)++;
		}
	}
	if (*pExecType == gd) {
		// 0xbe
		pCdb[8] = *lpTransferLen;
		*lpRoopLen = (BYTE)(*lpTransferLen - 1);
	}
	else {
		// 0xa8
		pCdb[9] = *lpTransferLen;
		*lpRoopLen = *lpTransferLen;
	}
}

VOID SetBufferSizeForReadCD(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	DRIVE_DATA_ORDER order
) {
	if (order == DRIVE_DATA_ORDER::NoC2) {
		pDevice->TRANSFER.uiBufLen = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
		pDevice->TRANSFER.uiBufC2Offset = 0;
		pDevice->TRANSFER.uiBufSubOffset = CD_RAW_SECTOR_SIZE;
		if (*pExecType == gd) {
			pDevice->sub = CDFLAG::_READ_CD::Raw;
		}
		else {
			pDevice->sub = CDFLAG::_READ_CD::Pack;
		}
	}
	else if (order == DRIVE_DATA_ORDER::MainC2Sub) {
		pDevice->TRANSFER.uiBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
		pDevice->TRANSFER.uiBufC2Offset = CD_RAW_SECTOR_SIZE;
		pDevice->TRANSFER.uiBufSubOffset = CD_RAW_SECTOR_WITH_C2_294_SIZE;
		pDevice->sub = CDFLAG::_READ_CD::Raw;
	}
	else if (order == DRIVE_DATA_ORDER::MainSubC2) {
		pDevice->TRANSFER.uiBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
		pDevice->TRANSFER.uiBufSubOffset = CD_RAW_SECTOR_SIZE;
		pDevice->TRANSFER.uiBufC2Offset = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
		pDevice->sub = CDFLAG::_READ_CD::Raw;
	}
}

VOID SetFeatureCdRead(
	PFEATURE_DATA_CD_READ pCDRead,
	PDEVICE pDevice
) {
	pDevice->FEATURE.byCanCDText = pCDRead->CDText;
	pDevice->FEATURE.byC2ErrorData = pCDRead->C2ErrorData;
}

VOID SetFeatureRealTimeStreaming(
	PFEATURE_DATA_REAL_TIME_STREAMING pRTS,
	PDEVICE pDevice
) {
	pDevice->FEATURE.byModePage2a = pRTS->WriteSpeedInMP2A;
	pDevice->FEATURE.bySetCDSpeed = pRTS->SetCDSpeed;
	pDevice->FEATURE.byReadBufCapa = pRTS->ReadBufferCapacityBlock;
}

VOID SetAndOutputToc(
	PDISC pDisc
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("TOC"));
	CONST INT typeSize = 7 * sizeof(_TCHAR);
	_TCHAR strType[typeSize] = {};
	BOOL bFirstData = TRUE;
	TRACK_TYPE trkType = TRACK_TYPE::audioOnly;
	// for swap command
	pDisc->SCSI.nAllLength = 0;

	for (INT i = pDisc->SCSI.toc.FirstTrack - 1; i < pDisc->SCSI.toc.LastTrack; i++) {
		// corrupt toc
		// [CD-i] Op Jacht Naar Vernuft (Netherlands)
		if (pDisc->SCSI.toc.TrackData[i - 1].TrackNumber == 0xff) {
			OutputLog(standardOut | fileDisc
				, "Detected corrupt TOC in Track%d. Fixed it\n", pDisc->SCSI.toc.TrackData[i].TrackNumber - 1);
			for (INT k = i; k <= pDisc->SCSI.toc.LastTrack; k++) {
				memcpy(&pDisc->SCSI.toc.TrackData[k - 1], &pDisc->SCSI.toc.TrackData[k], sizeof(TRACK_DATA));
			}
		}
	}

	for (BYTE i = pDisc->SCSI.toc.FirstTrack; i <= pDisc->SCSI.toc.LastTrack; i++) {
		INT tIdx = i - 1;
		if (pDisc->SCSI.byFormat == DISK_TYPE_CDI && pDisc->SCSI.toc.LastTrack > 1) {
			if (i == pDisc->SCSI.toc.LastTrack) {
				break;
			}
			else {
				tIdx = i;
			}
		}
		// for swap command
		pDisc->SCSI.lp1stLBAListOnToc[tIdx] = 0;
		pDisc->SCSI.lpLastLBAListOnToc[tIdx] = 0;

		for (INT j = 0, k = 24; j < 4; j++, k -= 8) {
			pDisc->SCSI.lp1stLBAListOnToc[tIdx] |= pDisc->SCSI.toc.TrackData[i - 1].Address[j] << k;
			pDisc->SCSI.lpLastLBAListOnToc[tIdx] |= pDisc->SCSI.toc.TrackData[i].Address[j] << k;
		}
		if (pDisc->SCSI.toc.TrackData[i].TrackNumber == 0x00) {
			// corrupt toc (not found 0xAA (lead-out)
			// [CD-i] op jacht naar vernuft
			pDisc->SCSI.toc.TrackData[i].Adr = 1;
			pDisc->SCSI.toc.TrackData[i].TrackNumber = 0xaa;
			pDisc->SCSI.lpLastLBAListOnToc[tIdx] = pDisc->SCSI.n1stLBAofLeadout;
		}
		pDisc->SCSI.lpLastLBAListOnToc[tIdx] -= 1;
		pDisc->SCSI.nAllLength += 
			pDisc->SCSI.lpLastLBAListOnToc[tIdx] - pDisc->SCSI.lp1stLBAListOnToc[tIdx] + 1;

		if ((pDisc->SCSI.toc.TrackData[i - 1].Control & AUDIO_DATA_TRACK) == 0) {
			_tcsncpy(strType, _T(" Audio"), typeSize);
		}
		else if ((pDisc->SCSI.toc.TrackData[i - 1].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			_tcsncpy(strType, _T("  Data"), typeSize);
			if (bFirstData) {
				pDisc->SCSI.n1stLBAofDataTrk = pDisc->SCSI.lp1stLBAListOnToc[tIdx];
				pDisc->SCSI.by1stDataTrkNum = i;
				bFirstData = FALSE;
				trkType = TRACK_TYPE::dataExist;
			}
			pDisc->SCSI.nLastLBAofDataTrkOnToc = pDisc->SCSI.lpLastLBAListOnToc[tIdx];
			pDisc->SCSI.byLastDataTrkNum = i;
		}

		if (i == pDisc->SCSI.toc.FirstTrack && pDisc->SCSI.lp1stLBAListOnToc[tIdx] > 0) {
			if (pDisc->SCSI.byFormat != DISK_TYPE_CDI) {
				// [CD-i Ready] Dimo's Quest (Europe) http://redump.org/disc/35804/
				pDisc->SCSI.nAllLength += pDisc->SCSI.lp1stLBAListOnToc[tIdx];
				OutputDiscLog("\tPregap Track   , LBA        0 - %8d, Length %8d\n"
					, pDisc->SCSI.lp1stLBAListOnToc[tIdx] - 1, pDisc->SCSI.lp1stLBAListOnToc[tIdx]);
				if ((pDisc->SUB.byCtlOfLBA0 & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					trkType = TRACK_TYPE::pregapDataIn1stTrack;
					pDisc->SCSI.n1stLBAofDataTrk = 0;
					pDisc->SCSI.by1stDataTrkNum = 1;
					pDisc->SCSI.byLastDataTrkNum = 1;
				}
				else {
					trkType = TRACK_TYPE::pregapAudioIn1stTrack;
				}
			}
		}
		INT idx = i;
		if (pDisc->SCSI.byFormat == DISK_TYPE_CDI && pDisc->SCSI.toc.LastTrack > 1) {
			if (i == pDisc->SCSI.toc.FirstTrack) {
				if (pDisc->SUB.byIdxOfLBA0 == 0) {
					// [CD-i Ready] Dimo's Quest (USA)
					OutputDiscLog(
						"\tPregap Track   , LBA        0 -        ?, Length        ?\n"
						"\t Audio Track %2u, LBA        ? - %8d, Length        ?\n"
						, i, pDisc->SCSI.lp1stLBAListOnToc[tIdx] - 1);
					trkType = TRACK_TYPE::pregapDataIn1stTrack;
				}
				else {
					OutputDiscLog(
						"\t  Data Track %2u, LBA        0 - %8d, Length %8d\n"
						, i, pDisc->SCSI.lp1stLBAListOnToc[tIdx] - 1, pDisc->SCSI.lp1stLBAListOnToc[tIdx]);
					trkType = TRACK_TYPE::dataExist;
				}
				pDisc->SCSI.n1stLBAofDataTrk = 0;
				pDisc->SCSI.by1stDataTrkNum = 1;
				pDisc->SCSI.byLastDataTrkNum = 1;
			}
			idx = i + 1;
		}
		OutputDiscLog(
			"\t%s Track %2d, LBA %8d - %8d, Length %8d\n", strType, idx,
			pDisc->SCSI.lp1stLBAListOnToc[tIdx], pDisc->SCSI.lpLastLBAListOnToc[tIdx],
			pDisc->SCSI.lpLastLBAListOnToc[tIdx] - pDisc->SCSI.lp1stLBAListOnToc[tIdx] + 1);
	}
	if (pDisc->SCSI.byFormat == DISK_TYPE_CDI && pDisc->SCSI.toc.LastTrack > 1) {
		pDisc->SCSI.lpLastLBAListOnToc[0] = pDisc->SCSI.lp1stLBAListOnToc[1] - 1;
		pDisc->SCSI.nAllLength += pDisc->SCSI.lp1stLBAListOnToc[1];
	}
	OutputDiscLog(
		"\t                                          Total  %8d\n", pDisc->SCSI.nAllLength);

	pDisc->SCSI.trkType = trkType;
	OutputDiscLog("Set the TRACK_TYPE to ");

	switch (pDisc->SCSI.trkType) {
	case TRACK_TYPE::audioOnly:
		OutputDiscLog("audioOnly\n");
		break;
	case TRACK_TYPE::dataExist:
		OutputDiscLog("dataExist\n");
		break;
	case TRACK_TYPE::pregapAudioIn1stTrack:
		OutputDiscLog("pregapAudioIn1stTrack\n");
		break;
	case TRACK_TYPE::pregapDataIn1stTrack:
		OutputDiscLog("pregapDataIn1stTrack\n");
		break;
	default:
		OutputDiscLog("unknown\n");
		break;
	}
}

VOID SetAndOutputTocForGD(
	PDISC pDisc,
	LPBYTE bufDec
) {
	// update the toc of audio trap disc to the toc of gd-rom
	pDisc->SCSI.toc.FirstTrack = bufDec[0x29a];
	pDisc->SCSI.toc.LastTrack = bufDec[0x29e];
	pDisc->SCSI.nAllLength = (INT)(MAKELONG(
		MAKEWORD(bufDec[0x2a0], bufDec[0x2a1]), MAKEWORD(bufDec[0x2a2], 0)) - 150);

	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("TOC For GD(HD Area)"));
	CONST INT typeSize = 7 * sizeof(_TCHAR);
	_TCHAR strType[typeSize] = {};
	BOOL bFirstData = TRUE;
	TRACK_TYPE trkType = TRACK_TYPE::audioOnly;

	INT j = 0;
	for (BYTE i = pDisc->SCSI.toc.FirstTrack; i <= pDisc->SCSI.toc.LastTrack; i++, j += 4) {
		INT tIdx = i - 1;
		// update the toc of audio trap disc to the toc of gd-rom
		pDisc->SCSI.lp1stLBAListOnToc[tIdx] = (INT)(MAKELONG(
			MAKEWORD(bufDec[0x114 + j], bufDec[0x115 + j]), MAKEWORD(bufDec[0x116 + j], 0)) - 150);
		if (i == pDisc->SCSI.toc.LastTrack) {
			pDisc->SCSI.lpLastLBAListOnToc[tIdx] = pDisc->SCSI.nAllLength - 1;
		}
		else {
			pDisc->SCSI.lpLastLBAListOnToc[tIdx] = (INT)(MAKELONG(
				MAKEWORD(bufDec[0x118 + j], bufDec[0x119 + j]), MAKEWORD(bufDec[0x11a + j], 0)) - 150 - 1);
		}
		pDisc->SCSI.toc.TrackData[tIdx].Adr = (BYTE)((bufDec[0x117 + j]) & 0x0f);
		pDisc->SCSI.toc.TrackData[tIdx].Control = (BYTE)((BYTE)(bufDec[0x117 + j] >> 4) & 0x0f);
		pDisc->SCSI.toc.TrackData[tIdx].TrackNumber = i;

		if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == 0) {
			_tcsncpy(strType, _T(" Audio"), typeSize);
		}
		else if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			_tcsncpy(strType, _T("  Data"), typeSize);
			if (bFirstData) {
				pDisc->SCSI.n1stLBAofDataTrk = pDisc->SCSI.lp1stLBAListOnToc[tIdx];
				pDisc->SCSI.by1stDataTrkNum = i;
				bFirstData = FALSE;
				trkType = TRACK_TYPE::dataExist;
			}
			pDisc->SCSI.nLastLBAofDataTrkOnToc = pDisc->SCSI.lpLastLBAListOnToc[tIdx];
			pDisc->SCSI.byLastDataTrkNum = i;
		}
		OutputDiscLog(
			"\t%s Track %2u, LBA %6d - %6d, Length %6d\n", strType, i,
			pDisc->SCSI.lp1stLBAListOnToc[tIdx], pDisc->SCSI.lpLastLBAListOnToc[tIdx],
			pDisc->SCSI.lpLastLBAListOnToc[tIdx] - pDisc->SCSI.lp1stLBAListOnToc[tIdx] + 1);
	}
	OutputDiscLog("                                           Total %6d\n"
		, pDisc->SCSI.nAllLength - FIRST_LBA_FOR_GD);
	pDisc->SCSI.trkType = trkType;
}

VOID SetAndOutputTocFull(
	PDISC pDisc,
	PCDROM_TOC_FULL_TOC_DATA fullToc,
	PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData,
	WORD wTocEntries,
	FILE* fpCcd
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("FULL TOC")
		"\tFirstCompleteSession: %u\n"
		"\t LastCompleteSession: %u\n"
		, fullToc->FirstCompleteSession
		, fullToc->LastCompleteSession);
	UCHAR ucTmpLastTrack = 0;

	for (WORD a = 0; a < wTocEntries; a++) {
		INT nTmpLBAExt = 0;
		INT nTmpLBA = 0;
		if (fpCcd) {
			WriteCcdForEntry(pTocData, a, fpCcd);
		}
		OutputDiscLog("\tSession %u, Ctl %u, Adr %u, Point 0x%02x,"
			, pTocData[a].SessionNumber, pTocData[a].Control, pTocData[a].Adr, pTocData[a].Point);
		switch (pTocData[a].Point) {
		case 0xa0:
			OutputDiscLog(" FirstTrack %2u, ", pTocData[a].Msf[0]);
			switch (pTocData[a].Msf[1]) {
			case DISK_TYPE_CDDA:
				OutputDiscLog("Format: CD-DA or CD-ROM\n");
				break;
			case DISK_TYPE_CDI:
				OutputDiscLog("Format: CD-I\n");
				break;
			case DISK_TYPE_XA:
				OutputDiscLog("Format: CD-ROM-XA\n");
				break;
			default:
				OutputDiscLog("Format: Other [0x%02x]\n", pTocData[a].Msf[1]);
				break;
			}
			// set this by ReadTOCFull
//			pDisc->SCSI.byFormat = pTocData[a].Msf[1];
			if (fullToc->LastCompleteSession > 1 && pTocData[a].Msf[0] > 1) {
				pDisc->SCSI.by1stMultiSessionTrkNum = pTocData[a].Msf[0];
			}
			break;
		case 0xa1:
			OutputDiscLog("  LastTrack %2u\n", pTocData[a].Msf[0]);
			ucTmpLastTrack = pTocData[a].Msf[0];
			break;
		case 0xa2:
			nTmpLBA = 
				MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]) - 150;
			OutputDiscLog(
				"      Lead-out, AMSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
				, pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2], nTmpLBA, (UINT)nTmpLBA);
			if (fullToc->LastCompleteSession > 1) {
				// Rayman (USA) [SS], Wolfchild (Europe) [MCD]
				// Last LBA is corrupt, so this doesn't use in single session disc.
				pDisc->SCSI.lpLastLBAListOnToc[ucTmpLastTrack - 1] = nTmpLBA - 1;
			}
			// set this by ReadTOCFull
//			if (pTocData[a].SessionNumber == 1) {
//				pDisc->SCSI.n1stLBAofLeadout = nTmpLBA;
//			}
			break;
		case 0xb0: // (multi-session disc)
			nTmpLBAExt =
				MSFtoLBA(pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2]) - 150;
			nTmpLBA =
				MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]) - 150;
			OutputDiscLog(
				"   NextSession, AMSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
				"\t                    Outermost Lead-out of the disc, AMSF %02u:%02u:%02u (LBA[%06d, %#07x])\n" 
				"\t                         Num of pointers in Mode 5, %02u\n"
				, pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2]
				, nTmpLBAExt, (UINT)nTmpLBAExt
				, pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]
				, nTmpLBA, (UINT)nTmpLBA, pTocData[a].Zero);
			// set this by ReadTOCFull
//			pDisc->SCSI.n1stLBAof2ndSession = nTmpLBAExt + 150;
			break;
		case 0xb1: // (Audio only: This identifies the presence of skip intervals)
			OutputDiscLog(
				"\t                     Num of skip interval pointers, %02u\n"
				"\t                        Num of skip track pointers, %02u\n"
				, pTocData[a].Msf[0], pTocData[a].Msf[1]);
			break;
		case 0xb2: // (Audio only: This identifies tracks that should be skipped during playback)
		case 0xb3:
		case 0xb4:
			OutputDiscLog(
				"                         Skip num, %02u %02u %02u %02u %02u %02u\n"
				, pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2]
				, pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			break;
		case 0xc0: // (Together with POINT=B0h, this is used to identify a multi-session disc)
			nTmpLBA =
				MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]) - 150;
			if (pDisc->SCSI.wCurrentMedia == ProfileCdRecordable ||
				pDisc->SCSI.wCurrentMedia == ProfileCdRewritable) {
				OutputDiscLog(
					"              ,  MSF %02u:%02u:%02u\n"
					"\t                         First Lead-in of the disc, AMSF %02u:%02u:%02u\n"
					, pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2]
					, pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
				// Ecma 394
				// 4.4.1 Special Information 1 : M1,S1,F1 = 101
				pTocData[a].MsfExtra[0] = (UCHAR)(pTocData[a].MsfExtra[0] >> 1 | 0x80);
				pTocData[a].MsfExtra[1] = (UCHAR)(pTocData[a].MsfExtra[1] >> 1);
				pTocData[a].MsfExtra[2] = (UCHAR)(pTocData[a].MsfExtra[2] >> 1 | 0x80);
#if 0
				PCDROM_TOC_ATIP_DATA_BLOCK pAtip = (PCDROM_TOC_ATIP_DATA_BLOCK)&pTocData[a].MsfExtra[0];
				OutputCDAtipSI1(pAtip);
				OutputCDAtipSI2(pAtip);
#endif
			}
			else {
				OutputDiscLog(
					"              ,  MSF %02u:%02u:%02u\n"
					"\t                         First Lead-in of the disc, AMSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
					, pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2]
					, pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]
					, nTmpLBA - 450150, (UINT)nTmpLBA - 450150);
			}
			break;
		case 0xc1:
			OutputDiscLog(
				"\n     Copy of info from Additional Information1 in ATIP,  MSF %02u:%02u:%02u, AMSF %02u:%02u:%02u\n"
				, pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2]
				, pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			// Ecma 394
			// 4.4.4 Additional Information 1 : M1,S1,F1 = 001
#if 0
			CDROM_TOC_ATIP_DATA_BLOCK atip;
			if (pDisc->SCSI.wCurrentMedia == ProfileCdRewritable) {
				atip.IsCdrw = TRUE;
			}
			atip.A1Valid = TRUE;
			atip.A1Values[0] = (UCHAR)(pTocData[a].MsfExtra[0] >> 1);
			atip.A1Values[1] = (UCHAR)(pTocData[a].MsfExtra[1] >> 1);
			atip.A1Values[2] = (UCHAR)(pTocData[a].MsfExtra[2] >> 1 | 0x80);
			OutputCDAtipAI1(&atip);
#endif
			break;
		default:
			if (pTocData[a].Adr == 1) {
				nTmpLBA =
					MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]) - 150;
				OutputDiscLog(
					"      Track %2u, AMSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
					, pTocData[a].Point, pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2], nTmpLBA, (UINT)nTmpLBA);
				if (fullToc->LastCompleteSession > 1) {
#if 0
					// Rayman (USA) [SS], Wolfchild (Europe) [MCD]
					// Last LBA is corrupt, so this doesn't use in single session disc.
					if (pTocData[a].Point == 1) {
						pDisc->SCSI.lp1stLBAListOnToc[pTocData[a].Point - 1] = nTmpLBA - 150;
					}
					else if (pTocData[a].Point >= 2 && pTocData[a].Point <= 100) {
						pDisc->SCSI.lpLastLBAListOnToc[pTocData[a].Point - 2] = nTmpLBA - 150 - 1;
						pDisc->SCSI.lp1stLBAListOnToc[pTocData[a].Point - 1] = nTmpLBA - 150;
					}
					// Track 2 is incorrect...
					//	Session 1, Ctl 0, Adr 5, Point 0xb0,   NextSession, AMSF 02:36:02 (LBA[011702, 0x02db6])
					//	                    Outermost Lead-out of the disc, AMSF 16:19:71 (LBA[073496, 0x11f18])
					//	                         Num of pointers in Mode 5, 02
					//	Session 1, Ctl 0, Adr 5, Point 0xc0, Optimum recording power, 00
					//	                         First Lead-in of the disc, AMSF 95:00:00 (LBA[427500, 0x685ec])
					//	Session 2, Ctl 4, Adr 1, Point 0xa0, FirstTrack  2, Format: CD-ROM-XA
					//	Session 2, Ctl 4, Adr 1, Point 0xa1,  LastTrack  2
					//	Session 2, Ctl 4, Adr 1, Point 0xa2,      Lead-out, AMSF 16:19:71 (LBA[073496, 0x11f18])
					//	Session 2, Ctl 4, Adr 1, Point 0x02,      Track  2, AMSF 02:00:00 (LBA[009000, 0x02328])
					if (pTocData[a].SessionNumber == 2 && bFirst2ndSession) {
						pDisc->SCSI.n1stLBAof2ndSession = nTmpLBA - 150;
						bFirst2ndSession = FALSE;
					}
#endif
				}
				if (pDisc->SCSI.byFormat == DISK_TYPE_CDI && pTocData[a].Point == 0x02) {
					pDisc->SCSI.lpSessionNumList[0] = pTocData[a].SessionNumber;
				}
				pDisc->SCSI.lpSessionNumList[pTocData[a].Point - 1] = pTocData[a].SessionNumber;
			}
			else if (pTocData[a].Adr == 5) {
				nTmpLBAExt =
					MSFtoLBA(pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2]) - 150;
				nTmpLBA =
					MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]) - 150;
				OutputDiscLog(
					"\t              Skipped interval end time, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
					"\t              Skipped interval start time on playback, AMSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
					, pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2], nTmpLBAExt, (UINT)nTmpLBAExt
					, pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2], nTmpLBA, (UINT)nTmpLBA);
			}
			break;
		}
	}
}

VOID SetAndOutputTocCDText(
	PDISC pDisc,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	LPCH pTmpText,
	WORD wTocTextEntries
) {
	UINT uiLastTrackNum = 0;
	UINT uiPacksOfAlbum[MAX_CDTEXT_LANG] = {};
	UINT uiPacksOfPerformer[MAX_CDTEXT_LANG] = {};
	UINT uiPacksOfSongwriter[MAX_CDTEXT_LANG] = {};
	UINT uiPacksOfComposer[MAX_CDTEXT_LANG] = {};
	UINT uiPacksOfArranger[MAX_CDTEXT_LANG] = {};
	UINT uiPacksOfMessages[MAX_CDTEXT_LANG] = {};
	UINT uiPacksOfDiscId[MAX_CDTEXT_LANG] = {};
	UINT uiPacksOfGenre[MAX_CDTEXT_LANG] = {};
	UINT uiPacksOfUpcEan[MAX_CDTEXT_LANG] = {};

	for (INT i = 0; i < wTocTextEntries; i++) {
		if (pDesc[i].PackType == CDROM_CD_TEXT_PACK_SIZE_INFO) {
			uiLastTrackNum = pDesc[i].Text[2];
			uiPacksOfAlbum[pDesc[i].BlockNumber] = pDesc[i].Text[4];
			uiPacksOfPerformer[pDesc[i].BlockNumber] = pDesc[i].Text[5];
			uiPacksOfSongwriter[pDesc[i].BlockNumber] = pDesc[i].Text[6];
			uiPacksOfComposer[pDesc[i].BlockNumber] = pDesc[i].Text[7];
			uiPacksOfArranger[pDesc[i].BlockNumber] = pDesc[i].Text[8];
			uiPacksOfMessages[pDesc[i].BlockNumber] = pDesc[i].Text[9];
			uiPacksOfDiscId[pDesc[i].BlockNumber] = pDesc[i].Text[10];
			uiPacksOfGenre[pDesc[i].BlockNumber] = pDesc[i].Text[11];
			uiPacksOfUpcEan[pDesc[i].BlockNumber] = pDesc[i + 1].Text[6];
			i += 2;
		}
	}
	UCHAR ucLastTrkNum = 0;

	for (UINT j = 0; j < wTocTextEntries; j++) {
		if (pDesc[j].PackType == CDROM_CD_TEXT_PACK_ALBUM_NAME) {
			for (UINT m = 0; m < uiPacksOfAlbum[pDesc[j].BlockNumber]; m++) {
				memcpy(pTmpText + 12 * m, pDesc[m + j].Text, 12);
			}
			size_t stTxtIdx = 0;
			size_t stTmpTextLen = 0;
			LPCH bufShiftJis = NULL;
			for (UINT k = 0; k < uiLastTrackNum + 1; k++) {
				INT nConvSize = 0;
				if (pDesc[j].Unicode) {
					stTmpTextLen = strlen(pTmpText + stTxtIdx);
					if (stTmpTextLen) {
						if (!IsSjis(pTmpText, stTxtIdx, stTmpTextLen)) {
							stTmpTextLen = wcslen((LPWCH)(pTmpText + stTxtIdx));
							if (stTmpTextLen == 0) {
								stTxtIdx += 2;
								continue;
							}
							nConvSize = ConvertUnicodeToSjis(pTmpText, &bufShiftJis, stTmpTextLen, stTxtIdx);
						}
					}
				}
				if (nConvSize) {
					strncpy(pDisc->SCSI.CDTEXT[pDesc[j].BlockNumber].pszTitle[k], bufShiftJis, META_CDTEXT_SIZE - 1);
				}
				else {
					stTmpTextLen = strlen(pTmpText + stTxtIdx);
					if (stTmpTextLen) {
						strncpy(pDisc->SCSI.CDTEXT[pDesc[j].BlockNumber].pszTitle[k], pTmpText + stTxtIdx, META_CDTEXT_SIZE - 1);
					}
				}
				if (stTmpTextLen) {
					if (k == 0) {
						OutputDiscLog("\tAlbum Name: %" CHARWIDTH "s\n", pDisc->SCSI.CDTEXT[pDesc[j].BlockNumber].pszTitle[k]);
					}
					else {
						OutputDiscLog("\t Song Name[%u]: %" CHARWIDTH "s\n", k, pDisc->SCSI.CDTEXT[pDesc[j].BlockNumber].pszTitle[k]);
					}
					if (nConvSize) {
						stTxtIdx += stTmpTextLen * 2;
					}
					else {
						stTxtIdx += stTmpTextLen;
					}
				}
				if (pDesc[j].Unicode) {
					stTxtIdx += 2;
				}
				else {
					stTxtIdx++;
				}
				FreeAndNull(bufShiftJis);
			}
			j += uiPacksOfAlbum[pDesc[j].BlockNumber] - 1;
			pDisc->SCSI.CDTEXT[pDesc[j].BlockNumber].bExist = TRUE;
		}
		else if (pDesc[j].PackType == CDROM_CD_TEXT_PACK_PERFORMER) {
			for (UINT m = 0; m < uiPacksOfPerformer[pDesc[j].BlockNumber]; m++) {
				memcpy(pTmpText + 12 * m, pDesc[m + j].Text, 12);
			}
			size_t stTxtIdx = 0;
			size_t stTmpTextLen = 0;
			LPCH bufShiftJis = NULL;
			for (UINT k = 0; k < uiLastTrackNum + 1; k++) {
				INT nConvSize = 0;
				if (pDesc[j].Unicode) {
					stTmpTextLen = strlen(pTmpText + stTxtIdx);
					if (stTmpTextLen) {
						if (!IsSjis(pTmpText, stTxtIdx, stTmpTextLen)) {
							stTmpTextLen = wcslen((LPWCH)(pTmpText + stTxtIdx));
							if (stTmpTextLen == 0) {
								stTxtIdx += 2;
								continue;
							}
							nConvSize = ConvertUnicodeToSjis(pTmpText, &bufShiftJis, stTmpTextLen, stTxtIdx);
						}
					}
				}
				if (nConvSize) {
					strncpy(pDisc->SCSI.CDTEXT[pDesc[j].BlockNumber].pszPerformer[k], bufShiftJis, META_CDTEXT_SIZE - 1);
				}
				else {
					stTmpTextLen = strlen(pTmpText + stTxtIdx);
					if (stTmpTextLen) {
						strncpy(pDisc->SCSI.CDTEXT[pDesc[j].BlockNumber].pszPerformer[k], pTmpText + stTxtIdx, META_CDTEXT_SIZE - 1);
					}
				}
				if (stTmpTextLen) {
					if (k == 0) {
						OutputDiscLog("\tAlbum Performer: %" CHARWIDTH "s\n", pDisc->SCSI.CDTEXT[pDesc[j].BlockNumber].pszPerformer[k]);
					}
					else {
						OutputDiscLog("\t Song Performer[%u]: %" CHARWIDTH "s\n", k, pDisc->SCSI.CDTEXT[pDesc[j].BlockNumber].pszPerformer[k]);
					}
					if (nConvSize) {
						stTxtIdx += stTmpTextLen * 2;
					}
					else {
						stTxtIdx += stTmpTextLen;
					}
				}
				if (pDesc[j].Unicode) {
					stTxtIdx += 2;
				}
				else {
					stTxtIdx++;
				}
				FreeAndNull(bufShiftJis);
			}
			j += uiPacksOfPerformer[pDesc[j].BlockNumber] - 1;
		}
		else if (pDesc[j].PackType == CDROM_CD_TEXT_PACK_SONGWRITER) {
			for (UINT m = 0; m < uiPacksOfSongwriter[pDesc[j].BlockNumber]; m++) {
				memcpy(pTmpText + 12 * m, pDesc[m + j].Text, 12);
			}
			size_t stTxtIdx = 0;
			size_t stTmpTextLen = 0;
			LPCH bufShiftJis = NULL;
			for (UINT k = 0; k < uiLastTrackNum + 1; k++) {
				INT nConvSize = 0;
				if (pDesc[j].Unicode) {
					stTmpTextLen = strlen(pTmpText + stTxtIdx);
					if (stTmpTextLen) {
						if (!IsSjis(pTmpText, stTxtIdx, stTmpTextLen)) {
							stTmpTextLen = wcslen((LPWCH)(pTmpText + stTxtIdx));
							if (stTmpTextLen == 0) {
								stTxtIdx += 2;
								continue;
							}
							nConvSize = ConvertUnicodeToSjis(pTmpText, &bufShiftJis, stTmpTextLen, stTxtIdx);
						}
					}
				}
				if (nConvSize) {
					strncpy(pDisc->SCSI.CDTEXT[pDesc[j].BlockNumber].pszSongWriter[k], bufShiftJis, META_CDTEXT_SIZE - 1);
				}
				else {
					stTmpTextLen = strlen(pTmpText + stTxtIdx);
					if (stTmpTextLen) {
						strncpy(pDisc->SCSI.CDTEXT[pDesc[j].BlockNumber].pszSongWriter[k], pTmpText + stTxtIdx, META_CDTEXT_SIZE - 1);
					}
				}
				if (stTmpTextLen) {
					if (k == 0) {
						OutputDiscLog("\tAlbum SongWriter: %" CHARWIDTH "s\n", pDisc->SCSI.CDTEXT[pDesc[j].BlockNumber].pszSongWriter[k]);
					}
					else {
						OutputDiscLog("\t      SongWriter[%u]: %" CHARWIDTH "s\n", k, pDisc->SCSI.CDTEXT[pDesc[j].BlockNumber].pszSongWriter[k]);
					}
					if (nConvSize) {
						stTxtIdx += stTmpTextLen * 2;
					}
					else {
						stTxtIdx += stTmpTextLen;
					}
				}
				if (pDesc[j].Unicode) {
					stTxtIdx += 2;
				}
				else {
					stTxtIdx++;
				}
				FreeAndNull(bufShiftJis);
			}
			j += uiPacksOfSongwriter[pDesc[j].BlockNumber] - 1;
		}
		else if (pDesc[j].PackType == CDROM_CD_TEXT_PACK_COMPOSER) {
			for (UINT m = 0; m < uiPacksOfComposer[pDesc[j].BlockNumber]; m++) {
				memcpy(pTmpText + 12 * m, pDesc[m + j].Text, 12);
			}
			size_t stTxtIdx = 0;
			for (UINT k = 0; k < uiLastTrackNum + 1; k++) {
				size_t len = strlen(pTmpText + stTxtIdx);
				if (len) {
					if (k == 0) {
						OutputDiscLog("\tAlbum Composer: %" CHARWIDTH "s\n", pTmpText + stTxtIdx);
					}
					else {
						OutputDiscLog("\t      Composer[%u]: %" CHARWIDTH "s\n", k, pTmpText + stTxtIdx);
					}
					stTxtIdx += len;
				}
				if (pDesc[j].Unicode) {
					stTxtIdx += 2;
				}
				else {
					stTxtIdx++;
				}
			}
			j += uiPacksOfComposer[pDesc[j].BlockNumber] - 1;
		}
		else if (pDesc[j].PackType == CDROM_CD_TEXT_PACK_ARRANGER) {
			for (UINT m = 0; m < uiPacksOfArranger[pDesc[j].BlockNumber]; m++) {
				memcpy(pTmpText + 12 * m, pDesc[m + j].Text, 12);
			}
			size_t stTxtIdx = 0;
			for (UINT k = 0; k < uiLastTrackNum + 1; k++) {
				size_t len = strlen(pTmpText + stTxtIdx);
				if (len) {
					if (k == 0) {
						OutputDiscLog("\tAlbum Arranger: %" CHARWIDTH "s\n", pTmpText + stTxtIdx);
					}
					else {
						OutputDiscLog("\t      Arranger[%u]: %" CHARWIDTH "s\n", k, pTmpText + stTxtIdx);
					}
					stTxtIdx += len;
				}
				if (pDesc[j].Unicode) {
					stTxtIdx += 2;
				}
				else {
					stTxtIdx++;
				}
			}
			j += uiPacksOfArranger[pDesc[j].BlockNumber] - 1;
		}
		else if (pDesc[j].PackType == CDROM_CD_TEXT_PACK_MESSAGES) {
			for (UINT m = 0; m < uiPacksOfMessages[pDesc[j].BlockNumber]; m++) {
				memcpy(pTmpText + 12 * m, pDesc[m + j].Text, 12);
			}
			size_t stTxtIdx = 0;
			for (UINT k = 0; k < uiLastTrackNum + 1; k++) {
				size_t len = strlen(pTmpText + stTxtIdx);
				if (len) {
					if (k == 0) {
						OutputDiscLog("\tAlbum Messages: %" CHARWIDTH "s\n", pTmpText + stTxtIdx);
					}
					else {
						OutputDiscLog("\t      Messages[%u]: %" CHARWIDTH "s\n", k, pTmpText + stTxtIdx);
					}
					stTxtIdx += len;
				}
				if (pDesc[j].Unicode) {
					stTxtIdx += 2;
				}
				else {
					stTxtIdx++;
				}
			}
			j += uiPacksOfMessages[pDesc[j].BlockNumber] - 1;
		}
		else if (pDesc[j].PackType == CDROM_CD_TEXT_PACK_DISC_ID) {
			for (UINT m = 0; m < uiPacksOfDiscId[pDesc[j].BlockNumber]; m++) {
				memcpy(pTmpText + 12 * m, pDesc[m + j].Text, 12);
			}
			OutputDiscLog("\tDisc ID: %" CHARWIDTH "s\n", pTmpText);
		}
		else if (pDesc[j].PackType == CDROM_CD_TEXT_PACK_GENRE) {
			for (UINT m = 0; m < uiPacksOfGenre[pDesc[j].BlockNumber]; m++) {
				memcpy(pTmpText + 12 * m, pDesc[m + j].Text, 12);
			}
			OutputDiscLog("\tGenre Code: 0x%02x%02x", (UINT)*(pTmpText), (UINT)*(pTmpText + 1));
			if (*(pTmpText + 1) != 0) {
				OutputDiscLog(" %" CHARWIDTH "s", pTmpText + 2);
			}
			OutputDiscLog("\n");
		}
		else if (pDesc[j].PackType == CDROM_CD_TEXT_PACK_TOC_INFO) {
			if (pDesc[j].TrackNumber == 0) {
				OutputDiscLog(
					"\tFirst track number: %u\n"
					"\t Last track number: %u\n"
					"\t     Lead-out(msf): %02u:%02u:%02u\n"
					, pDesc[j].Text[0], pDesc[j].Text[1], pDesc[j].Text[3]
					, pDesc[j].Text[4], pDesc[j].Text[5]);
				ucLastTrkNum = pDesc[j].Text[1];
			}
			else {
				for (INT i = pDesc[j].TrackNumber, k = 0; i < pDesc[j].TrackNumber + 4; i++, k += 3) {
					if (ucLastTrkNum < i) {
						break;
					}
					OutputDiscLog(
						"\t      Track %d(msf): %02u:%02u:%02u\n"
						, i, pDesc[j].Text[k], pDesc[j].Text[k + 1], pDesc[j].Text[k + 2]);
				}
			}
		}
		else if (pDesc[j].PackType == CDROM_CD_TEXT_PACK_TOC_INFO2) {
			OutputDiscLog(
				"\t     Priority number: %u\n"
				"\t Number of intervals: %u\n"
				"\tStart point(minutes): %u\n"
				"\tStart point(seconds): %u\n"
				"\t Start point(frames): %u\n"
				"\t  End point(minutes): %u\n"
				"\t  End point(seconds): %u\n"
				"\t   End point(frames): %u\n"
				, pDesc[j].Text[0], pDesc[j].Text[1], pDesc[j].Text[6], pDesc[j].Text[7]
				, pDesc[j].Text[8], pDesc[j].Text[9], pDesc[j].Text[10], pDesc[j].Text[11]);
		}
		else if (pDesc[j].PackType == CDROM_CD_TEXT_PACK_UPC_EAN) {
			for (UINT m = 0; m < uiPacksOfUpcEan[pDesc[j].BlockNumber]; m++) {
				memcpy(pTmpText + 12 * m, pDesc[m + j].Text, 12);
			}
			size_t stTxtIdx = 0;
			for (UINT i = 0; i < uiLastTrackNum + 1; i++) {
				size_t len = strlen(pTmpText + stTxtIdx);
				if (len) {
					if (i == 0) {
						OutputDiscLog("\tAlbum UpcEan: %" CHARWIDTH "s\n", pTmpText + stTxtIdx);
					}
					else {
						OutputDiscLog("\t      UpcEan[%u]: %" CHARWIDTH "s\n", i, pTmpText + stTxtIdx);
					}
					stTxtIdx += len;
				}
				stTxtIdx++;
			}
			j += uiPacksOfUpcEan[pDesc[j].BlockNumber] - 1;
		}
		else if (pDesc[j].PackType == CDROM_CD_TEXT_PACK_SIZE_INFO) {
			if (pDesc[j].TrackNumber == 0) {
				BYTE ch = pDesc[j].Text[0];
				OutputDiscLog(
					"\t         Character code for this BLOCK: 0x%02x ", ch);
				switch (ch) {
				case 0x00:
					OutputDiscLog("(ISO/IEC 8859-1 [Latin-1])\n");
					break;
				case 0x01:
					OutputDiscLog("(ISO/IEC 646 [ASCII])\n");
					break;
				case 0x80:
					OutputDiscLog("(Shift-JIS)\n");
					break;
				case 0x81:
					OutputDiscLog("(Korean character code)\n");
					break;
				case 0x82:
					OutputDiscLog("(Mandarin Chinese character code)\n");
					break;
				default:
					OutputDiscLog("(Reserved)\n");
					break;
				}
				OutputDiscLog(
					"\t                    First track Number: %u\n"
					"\t                     Last track Number: %u\n"
					"\t                         Mode2 PACKETs: %s\n"
					"\t          Program area copy protection: %s\n"
					"\t            Copyright asserted for $85: %s\n"
					"\t        Copyright asserted for $81-$84: %s\n"
					"\t            Copyright asserted for $80: %s\n"
					"\tNumber of PACKS with $80 (ALBUM_NAME) : %u\n"
					"\tNumber of PACKS with $81 (PERFORMER)  : %u\n"
					"\tNumber of PACKS with $82 (SONGWRITER) : %u\n"
					"\tNumber of PACKS with $83 (COMPOSER)   : %u\n"
					"\tNumber of PACKS with $84 (ARRANGER)   : %u\n"
					"\tNumber of PACKS with $85 (MESSAGES)   : %u\n"
					"\tNumber of PACKS with $86 (DISC_ID)    : %u\n"
					"\tNumber of PACKS with $87 (GENRE)      : %u\n"
					, pDesc[j].Text[1], pDesc[j].Text[2]
					, BOOLEAN_TO_STRING_YES_NO(pDesc[j].Text[3] & 0x80)
					, BOOLEAN_TO_STRING_YES_NO(pDesc[j].Text[3] & 0x40)
					, BOOLEAN_TO_STRING_YES_NO(pDesc[j].Text[3] & 0x04)
					, BOOLEAN_TO_STRING_YES_NO(pDesc[j].Text[3] & 0x02)
					, BOOLEAN_TO_STRING_YES_NO(pDesc[j].Text[3] & 0x01)
					, pDesc[j].Text[4], pDesc[j].Text[5], pDesc[j].Text[6]
					, pDesc[j].Text[7], pDesc[j].Text[8], pDesc[j].Text[9]
					, pDesc[j].Text[10], pDesc[j].Text[11]
				);
			}
			else if (pDesc[j].TrackNumber == 1) {
				OutputDiscLog(
					"\tNumber of PACKS with $88 (TOC_INFO)   : %u\n"
					"\tNumber of PACKS with $89 (TOC_INFO2)  : %u\n"
					"\tNumber of PACKS with $8a              : %u\n"
					"\tNumber of PACKS with $8b              : %u\n"
					"\tNumber of PACKS with $8c              : %u\n"
					"\tNumber of PACKS with $8d (CLOSED_INFO): %u\n"
					"\tNumber of PACKS with $8e (UPC_EAN)    : %u\n"
					"\tNumber of PACKS with $8f (SIZE_INFO)  : %u\n"
					"\t       Last Sequence number of BLOCK 0: %u\n"
					"\t       Last Sequence number of BLOCK 1: %u\n"
					"\t       Last Sequence number of BLOCK 2: %u\n"
					"\t       Last Sequence number of BLOCK 3: %u\n"
					, pDesc[j].Text[0], pDesc[j].Text[1], pDesc[j].Text[2]
					, pDesc[j].Text[3], pDesc[j].Text[4], pDesc[j].Text[5]
					, pDesc[j].Text[6], pDesc[j].Text[7], pDesc[j].Text[8]
					, pDesc[j].Text[9], pDesc[j].Text[10], pDesc[j].Text[11]
				);
			}
			else if (pDesc[j].TrackNumber == 2) {
				OutputDiscLog(
					"\t       Last Sequence number of BLOCK 4: %u\n"
					"\t       Last Sequence number of BLOCK 5: %u\n"
					"\t       Last Sequence number of BLOCK 6: %u\n"
					"\t       Last Sequence number of BLOCK 7: %u\n"
					, pDesc[j].Text[0], pDesc[j].Text[1], pDesc[j].Text[2], pDesc[j].Text[3]
				);
				// https://tech.ebu.ch/docs/tech/tech3264.pdf
				_TCHAR szLangCode[][15] = {
					_T("not applicable"), _T("Albanian"), _T("Breton"), _T("Catalan"), _T("Croatian"), _T("Welsh"), _T("Czech"), _T("Danish"),
					_T("German"), _T("English"), _T("Spanish"), _T("Esperanto"), _T("Estonian"), _T("Basque"), _T("Faroese"), _T("French"),
					_T("Frisian"), _T("Irish"), _T("Gaelic"), _T("Galician"), _T("Icelandic"), _T("Italian"), _T("Lappish"), _T("Latin"),
					_T("Latvian"), _T("Luxembourgian"), _T("Lithuanian"), _T("Hungarian"), _T("Maltese"), _T("Dutch"), _T("Norwegian"), _T("Occitan"),
					_T("Polish"), _T("Portugese"), _T("Romanian"), _T("Romansh"), _T("Serbian"), _T("Slovak"), _T("Slovenian"), _T("Finnish"),
					_T("Swedish"), _T("Turkish"), _T("Flemish"), _T("Wallon"), _T(""), _T(""), _T(""), _T(""),
					_T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""),
					_T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""),
					_T(""), _T(""), _T(""), _T(""), _T(""), _T("Zulu"), _T("Vietnamese"), _T("Uzbek"),
					_T("Urdu"), _T("Ukrainian"), _T("Thai"), _T("Telugu"), _T("Tatar"), _T("Tamil"), _T("Tadzhik"), _T("Swahili"),
					_T("Sranan Tongo"), _T("Somali"), _T("Sinhalese"), _T("Shona"), _T("Serbo-croat"), _T("Ruthenian"), _T("Russian"), _T("Quechua"),
					_T("Pushtu"), _T("Punjabi"), _T("Persian"), _T("Papamiento"), _T("Oriya"), _T("Nepali"), _T("Ndebele"), _T("Marathi"),
					_T("Moldavian"), _T("Malaysian"), _T("Malagasay"), _T("Macedonian"), _T("Laotian"), _T("Korean"), _T("Khmer"), _T("Kazakh"),
					_T("Kannada"), _T("Japanese"), _T("Indonesian"), _T("Hindi"), _T("Hebrew"), _T("Hausa"), _T("Gurani"), _T("Gujurati"),
					_T("Greek"), _T("Georgian"), _T("Fulani"), _T("Dari"), _T("Churash"), _T("Chinese"), _T("Burmese"), _T("Bulgarian"),
					_T("Bengali"), _T("Bielorussian"), _T("Bambora"), _T("Azerbaijani"), _T("Assamese"), _T("Armenian"), _T("Arabic"), _T("Amharic")
				};
				for (INT i = 0, m = 4; i < 8; i++, m++) {
					UCHAR lang = pDesc[j].Text[m];
					OutputDiscLog(
						"\t                 Language code BLOCK %d: 0x%02x (%s)\n", i, lang, szLangCode[lang]);
				}
			}
		}
	}
}

VOID SetAndOutputCDOffset(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	BOOL bGetDriveOffset,
	INT nDriveSampleOffset,
	INT nDriveOffset,
	INT nSubChannelOffset
) {
	OutputDiscLog(STR_DOUBLE_HYPHEN_B "Offset ");
	if (bGetDriveOffset) {
		OutputDiscLog("(Drive offset refers to http://www.accuraterip.com)");
	}
	OutputDiscLog(STR_DOUBLE_HYPHEN_E);

	if (pExtArg->byAdd && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
		pDisc->MAIN.nCombinedOffset += pExtArg->nAudioCDOffsetNum * 4;
		pExtArg->nAudioCDOffsetNum = 0; // If it is possible, I want to repair it by a better method...
		OutputDiscLog(
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
		OutputDiscLog(
			"\t Combined Offset(Byte) %6d, (Samples) %5d\n"
			"\t-   Drive Offset(Byte) %6d, (Samples) %5d\n"
			"\t----------------------------------------------\n"
			"\t       CD Offset(Byte) %6d, (Samples) %5d\n",
			pDisc->MAIN.nCombinedOffset, pDisc->MAIN.nCombinedOffset / 4,
			nDriveOffset, nDriveSampleOffset,
			pDisc->MAIN.nCombinedOffset - nDriveOffset,
			(pDisc->MAIN.nCombinedOffset - nDriveOffset) / 4);
	}

	if (pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE == 0) {
		pDisc->MAIN.nAdjustSectorNum =
			pDisc->MAIN.nCombinedOffset / CD_RAW_SECTOR_SIZE;
	}
	else if (0 < pDisc->MAIN.nCombinedOffset) {
		pDisc->MAIN.nAdjustSectorNum =
			pDisc->MAIN.nCombinedOffset / CD_RAW_SECTOR_SIZE + 1;
	}
	else if (pDisc->MAIN.nCombinedOffset < 0) {
		pDisc->MAIN.nAdjustSectorNum =
			pDisc->MAIN.nCombinedOffset / CD_RAW_SECTOR_SIZE - 1;
	}
	OutputDiscLog("\tOverread sector: %d\n", pDisc->MAIN.nAdjustSectorNum);
	if (nSubChannelOffset != 0xff) {
		OutputDiscLog("\tSubChannel Offset: %d\n", nSubChannelOffset);
	}
}

VOID ResetAndOutputCDOffset(
	PDEVICE pDevice,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nSample
) {
	pDisc->MAIN.nCombinedOffsetOrg = pDisc->MAIN.nCombinedOffset;
	pDisc->MAIN.nCombinedOffset += nSample * 4;
	SetAndOutputCDOffset(pExtArg, pDisc, TRUE, pDevice->nDriveSampleOffset
		, pDevice->nDriveSampleOffset * 4, pDisc->SUB.nSubChannelOffset);
	pDisc->MAIN.bResetOffset = TRUE;
}

VOID SetCDOffset(
	PEXEC_TYPE pExecType,
	BYTE byBe,
	BYTE byPlxtrDrive,
	PDISC pDisc,
	INT nStartLBA,
	INT nEndLBA
) {
	if (pDisc->MAIN.nCombinedOffset > 0) {
		if (*pExecType != gd && byBe && pDisc->SCSI.trkType != TRACK_TYPE::audioOnly) {
			pDisc->MAIN.uiMainDataSlideSize = 0;
			pDisc->MAIN.nOffsetStart = 0;
			pDisc->MAIN.nOffsetEnd = 0;
			pDisc->MAIN.nFixStartLBA = 0;
			pDisc->MAIN.nFixEndLBA = nEndLBA;
		}
		else {
			pDisc->MAIN.uiMainDataSlideSize =
				(UINT)pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE;
			pDisc->MAIN.nOffsetStart = 0;
			pDisc->MAIN.nOffsetEnd = pDisc->MAIN.nAdjustSectorNum;
			pDisc->MAIN.nFixStartLBA = nStartLBA + pDisc->MAIN.nAdjustSectorNum - 1;
			pDisc->MAIN.nFixEndLBA = nEndLBA + pDisc->MAIN.nAdjustSectorNum;
		}
		if (pDisc->SCSI.n1stLBAof2ndSession != -1) {
			pDisc->MAIN.nFix1stLBAofLeadout =
				pDisc->SCSI.n1stLBAofLeadout + pDisc->MAIN.nAdjustSectorNum;
			pDisc->MAIN.nFix1stLBAof2ndSession = 
				pDisc->SCSI.n1stLBAof2ndSession + pDisc->MAIN.nAdjustSectorNum - 1;
		}
	}
	else if (pDisc->MAIN.nCombinedOffset < 0) {
		if (*pExecType != gd && byBe && pDisc->SCSI.trkType != TRACK_TYPE::audioOnly) {
			pDisc->MAIN.uiMainDataSlideSize = 0;
			pDisc->MAIN.nOffsetStart = 0;
			pDisc->MAIN.nOffsetEnd = 0;
			pDisc->MAIN.nFixStartLBA = 0;
			pDisc->MAIN.nFixEndLBA = nEndLBA;
		}
		else {
			INT nTmp = pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE;
			if (nTmp == 0) {
				pDisc->MAIN.uiMainDataSlideSize = 0;
			}
			else {
				pDisc->MAIN.uiMainDataSlideSize = (UINT)CD_RAW_SECTOR_SIZE + nTmp;
			}
			pDisc->MAIN.nOffsetStart = pDisc->MAIN.nAdjustSectorNum;
			pDisc->MAIN.nOffsetEnd = 0;
			pDisc->MAIN.nFixStartLBA = nStartLBA + pDisc->MAIN.nAdjustSectorNum;
			pDisc->MAIN.nFixEndLBA = nEndLBA + pDisc->MAIN.nAdjustSectorNum + 1;
		}
		if (pDisc->SCSI.n1stLBAof2ndSession != -1) {
			pDisc->MAIN.nFix1stLBAofLeadout = 
				pDisc->SCSI.n1stLBAofLeadout + pDisc->MAIN.nAdjustSectorNum + 1;
			pDisc->MAIN.nFix1stLBAof2ndSession =
				pDisc->SCSI.n1stLBAof2ndSession + pDisc->MAIN.nAdjustSectorNum;
		}
	}
	else {
		pDisc->MAIN.uiMainDataSlideSize = 0;
		pDisc->MAIN.nOffsetStart = 0;
		pDisc->MAIN.nOffsetEnd = 0;
		pDisc->MAIN.nFixStartLBA = nStartLBA;
		pDisc->MAIN.nFixEndLBA = nEndLBA;
		if (pDisc->SCSI.n1stLBAof2ndSession != -1) {
			pDisc->MAIN.nFix1stLBAofLeadout =	pDisc->SCSI.n1stLBAofLeadout;
			pDisc->MAIN.nFix1stLBAof2ndSession = pDisc->SCSI.n1stLBAof2ndSession;
		}
	}
	UNREFERENCED_PARAMETER(byPlxtrDrive);
#if 0
	if (*pExecType == data &&
		((byBe && byPlxtrDrive) || !byPlxtrDrive) &&
		pDisc->SCSI.toc.FirstTrack == pDisc->SCSI.toc.LastTrack &&
		pDisc->SCSI.toc.FirstTrack == pDisc->SCSI.by1stDataTrkNum) {
		pDisc->MAIN.uiMainDataSlideSize = 0;
		pDisc->MAIN.nOffsetStart = 0;
		pDisc->MAIN.nOffsetEnd = 0;
		pDisc->MAIN.nFixStartLBA = 0;
	}
#endif
}

VOID SetTrackAttribution(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA
) {
	BYTE tmpCurrentTrackNum = pDiscPerSector->subch.current.byTrackNum;
	BYTE tmpCurrentIndex = pDiscPerSector->subch.current.byIndex;

	if ((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		if ((pDiscPerSector->subch.next.byCtl & AUDIO_DATA_TRACK) == 0 && pDisc->SCSI.trkType == TRACK_TYPE::pregapDataIn1stTrack) {
			pDisc->SUB.lp1stLBAListOfDataTrackOnSub[0] = 0;
			pDisc->SUB.lpLastLBAListOfDataTrackOnSub[0] = nLBA;
			OutputSubInfoWithLBALog("Index[%02d], Last data sector of this track\n"
				, nLBA, tmpCurrentTrackNum, pDiscPerSector->subch.next.byIndex);
		}
	}

	if (pDiscPerSector->subch.current.byAdr != ADR_ENCODES_CURRENT_POSITION) {
		if (nLBA == 0) {
			// [CD-i] Jazz Giants - From Big Band to Bossa Nova (Europe)
			// LBA[000000, 0000000]: P[ff], Q[46cf3cf0cff0fff000001202]{ Data,      Copy NG,                  Unknown Data    [cf3cf0cff0fff000], AMSF[     :00]}, RtoW[0, 0, 0, 0]
			// LBA[000001, 0x00001]: P[00], Q[410101000001000002019242]{ Data,      Copy NG,                  Track[01], Idx[01], RMSF[00:00:01], AMSF[00:02:01]}, RtoW[0, 0, 0, 0]
			tmpCurrentTrackNum = pDiscPerSector->subch.next.byTrackNum;
			tmpCurrentIndex = pDiscPerSector->subch.next.byIndex;
			OutputSubInfoWithLBALog("Set Track[%02d], Index[%02d] using next subchannel\n"
				, nLBA, tmpCurrentTrackNum, pDiscPerSector->subch.next.byTrackNum, pDiscPerSector->subch.next.byIndex);
			pDiscPerSector->bNextTrk = TRUE;
		}
		else if (pDiscPerSector->subch.current.byP == 0x00 && pDiscPerSector->subch.next.byP == 0xff &&
			pDiscPerSector->subch.prev.byTrackNum + 1 == pDiscPerSector->subch.next.byTrackNum) {
			if (pDisc->SUB.n1stRmsfOfTrk == 149 || pDisc->SUB.n1stRmsfOfTrk == 150 ||
				pDisc->SUB.n1stRmsfOfTrk == 224 || pDiscPerSector->subch.next.byTrackNum == 2) {
				// [FMT] Winning Post (Japan)
				// LBA[007949, 0x01f0d]: P[00], Q[01030100207400014774c6dd]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[03], Idx[01], RMSF[00:20:74], AMSF[01:47:74]}, RtoW[0, 0, 0, 0]
				// LBA[007950, 0x01f0e]: P[00], Q[020000000000000000002175]{Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :00]}, RtoW[0, 0, 0, 0]
				// LBA[007951, 0x01f0f]: P[ff], Q[010400000174000148017e01]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[04], Idx[00], RMSF[00:01:74], AMSF[01:48:01]}, RtoW[0, 0, 0, 0]
				// [PCE] Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
				// LBA[261585, 0x3fdd1]: P[00], Q[01950100136900580960087c]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[95], Idx[01], RMSF[00:13:69], AMSF[58:09:60]}, RtoW[0, 0, 0, 0]
				// LBA[261586, 0x3fdd2]: P[00], Q[020000000000000000615df2]{Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :61]}, RtoW[0, 0, 0, 0]
				// LBA[261587, 0x3fdd3]: P[ff], Q[019600000273005809625f79]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[96], Idx[00], RMSF[00:02:73], AMSF[58:09:62]}, RtoW[0, 0, 0, 0]
				tmpCurrentTrackNum = pDiscPerSector->subch.next.byTrackNum;
				tmpCurrentIndex = pDiscPerSector->subch.next.byIndex;
				OutputSubInfoWithLBALog("Set Track[%02d], Index[%02d] using next subchannel\n"
					, nLBA, tmpCurrentTrackNum, pDiscPerSector->subch.next.byTrackNum, pDiscPerSector->subch.next.byIndex);
				pDiscPerSector->bNextTrk = TRUE;
			}
			else {
				// [IBM PC] Quake (Canada)
				// LBA[039728, 0x09b30]: P[00], Q[110201050644000851531709]{Audio, 2ch, Copy NG, Pre-emphasis Yes, Track[02], Idx[01], RMSF[05:06:44], AMSF[08:51:53]}, RtoW[0, 0, 0, 0]
				// LBA[039729, 0x09b31]: P[00], Q[11020105064500085154cdbf]{Audio, 2ch, Copy NG, Pre-emphasis Yes, Track[02], Idx[01], RMSF[05:06:45], AMSF[08:51:54]}, RtoW[0, 0, 0, 0]
				// LBA[039730, 0x09b32]: P[ff], Q[1103000001730008515501e3]{Audio, 2ch, Copy NG, Pre-emphasis Yes, Track[03], Idx[00], RMSF[00:01:73], AMSF[08:51:55]}, RtoW[0, 0, 0, 0]
				// LBA[039731, 0x09b33]: P[ff], Q[110300000172000851569bd1]{Audio, 2ch, Copy NG, Pre-emphasis Yes, Track[03], Idx[00], RMSF[00:01:72], AMSF[08:51:56]}, RtoW[0, 0, 0, 0]
				//
				// LBA[050678, 0x0c5f6]: P[00], Q[11030102237400111753e511]{Audio, 2ch, Copy NG, Pre-emphasis Yes, Track[03], Idx[01], RMSF[02:23:74], AMSF[11:17:53]}, RtoW[0, 0, 0, 0]
				// LBA[050679, 0x0c5f7]: P[00], Q[11030102240000111754c937]{Audio, 2ch, Copy NG, Pre-emphasis Yes, Track[03], Idx[01], RMSF[02:24:00], AMSF[11:17:54]}, RtoW[0, 0, 0, 0]				
				// LBA[050680, 0x0c5f8]: P[00], Q[12000000000000000055bfb3]{Audio, 2ch, Copy NG, Pre-emphasis Yes, MediaCatalogNumber [0000000000000], AMSF[     :55]}, RtoW[0, 0, 0, 0]
				// LBA[050681, 0x0c5f9]: P[ff], Q[11040000017300111756fab3]{Audio, 2ch, Copy NG, Pre-emphasis Yes, Track[04], Idx[00], RMSF[00:01:73], AMSF[11:17:56]}, RtoW[0, 0, 0, 0]
				// LBA[050682, 0x0c5fa]: P[ff], Q[1104000001720011175740c3]{Audio, 2ch, Copy NG, Pre-emphasis Yes, Track[04], Idx[00], RMSF[00:01:72], AMSF[11:17:57]}, RtoW[0, 0, 0, 0]
				tmpCurrentTrackNum = pDiscPerSector->subch.prev.byTrackNum;
				tmpCurrentIndex = pDiscPerSector->subch.prev.byIndex;
				OutputSubInfoWithLBALog("Set Track[%02d], Index[%02d] using prev subchannel\n"
					, nLBA, tmpCurrentTrackNum, pDiscPerSector->subch.prev.byTrackNum, pDiscPerSector->subch.prev.byIndex);
			}
		}
		else if (pDiscPerSector->subch.current.byP == 0xff && pDiscPerSector->subch.next.byP == 0x00) {
			if (pDiscPerSector->subch.prev.byTrackNum + 1 == pDiscPerSector->subch.next.byTrackNum) {
				// [PCE] Madou Monogatari I - Honoo no Sotsuenji (Japan)
				// LBA[183031, 0x2caf7]: P[ff], Q[01210100317000404231bc6d]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[21], Idx[01], RMSF[00:31:70], AMSF[40:42:31]}, RtoW[0, 0, 0, 0]
				// LBA[183032, 0x2caf8]: P[ff], Q[020000000000000000323764]{Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :32]}, RtoW[0, 0, 0, 0]
				// LBA[183033, 0x2caf9]: P[00], Q[012201000001004042336c90]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[22], Idx[01], RMSF[00:00:01], AMSF[40:42:33]}, RtoW[0, 0, 0, 0]
				tmpCurrentTrackNum = pDiscPerSector->subch.next.byTrackNum;
				OutputSubInfoWithLBALog("Set Track[%02d] using next subchannel\n"
					, nLBA, tmpCurrentTrackNum, pDiscPerSector->subch.next.byTrackNum);
				pDiscPerSector->bNextTrk = TRUE;
			}
			else if (pDiscPerSector->subch.prev.byIndex + 1 == pDiscPerSector->subch.next.byIndex) {
				// [PCE] Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
				// LBA[142873, 0x22e19]: P[ff], Q[41370000000000314673bc02]{ Data,      Copy NG,                  Track[37], Idx[00], RMSF[00:00:00], AMSF[31:46:73]}, RtoW[0, 0, 0, 0]
				// LBA[142874, 0x22e1a]: P[ff], Q[420000000000000000746d7c]{ Data,      Copy NG,                  MediaCatalogNumber [0000000000000], AMSF[     :74]}, RtoW[0, 0, 0, 0]
				// LBA[142875, 0x22e1b]: P[00], Q[413701000001003147002c45]{ Data,      Copy NG,                  Track[37], Idx[01], RMSF[00:00:01], AMSF[31:47:00]}, RtoW[0, 0, 0, 0]
				tmpCurrentIndex = pDiscPerSector->subch.next.byIndex;
				OutputSubInfoWithLBALog("Set Index[%02d] using next subchannel\n"
					, nLBA, tmpCurrentTrackNum, pDiscPerSector->subch.next.byIndex);
				pDiscPerSector->bNextTrk = TRUE;
			}
		}
		else {
			tmpCurrentTrackNum = pDiscPerSector->subch.prev.byTrackNum;
			tmpCurrentIndex = pDiscPerSector->subch.prev.byIndex;
			OutputSubInfoWithLBALog("Set Track[%02d], Index[%02d] using prev subchannel\n"
				, nLBA, tmpCurrentTrackNum, pDiscPerSector->subch.prev.byTrackNum, pDiscPerSector->subch.prev.byIndex);
		}
	}
	if (0 <= nLBA && nLBA < pDisc->SCSI.nAllLength &&
		0 < tmpCurrentTrackNum && tmpCurrentTrackNum <= pDiscPerSector->byTrackNum + 1) {
		pDiscPerSector->byTrackNum = tmpCurrentTrackNum;
		INT tIdx = pDiscPerSector->byTrackNum - 1;
		BYTE tmpPrevTrackNum = pDiscPerSector->subch.prev.byTrackNum;
		BYTE tmpPrevIndex = pDiscPerSector->subch.prev.byIndex;

		if (pDiscPerSector->subch.prev.byAdr != ADR_ENCODES_CURRENT_POSITION) {
			if (nLBA == 1) {
				// LBA[000000, 0000000]: P[ff], Q[42000000000000000000536f]{ Data,      Copy NG,                  MediaCatalogNumber [0000000000000], AMSF[     :00]}, RtoW[Unknown, Unknown, Unknown, Unknown]
				// LBA[000001, 0x00001]: P[00], Q[410101000001000002019242]{ Data,      Copy NG,                  Track[01], Idx[01], RMSF[00:00:01], AMSF[00:02:01]}, RtoW[Unknown, Unknown, Unknown, Unknown]
				tmpPrevTrackNum = 1;
				tmpPrevIndex = 1;
			}
			else if (pDiscPerSector->bNextTrk) {
				tmpPrevTrackNum = pDiscPerSector->subch.current.byTrackNum;
				tmpPrevIndex = pDiscPerSector->subch.current.byIndex;
				pDiscPerSector->bNextTrk = FALSE;
				OutputSubInfoWithLBALog("Set PrevTrack[%02d], PrevIndex[%02d] using current subchannel\n"
					, nLBA, tmpCurrentTrackNum, pDiscPerSector->subch.prevPrev.byTrackNum, pDiscPerSector->subch.prevPrev.byIndex);
			}
			else {
				tmpPrevTrackNum = pDiscPerSector->subch.prevPrev.byTrackNum;
				tmpPrevIndex = pDiscPerSector->subch.prevPrev.byIndex;
				OutputSubInfoWithLBALog("Set PrevTrack[%02d], PrevIndex[%02d] using prevPrev subchannel\n"
					, nLBA, tmpCurrentTrackNum, pDiscPerSector->subch.prevPrev.byTrackNum, pDiscPerSector->subch.prevPrev.byIndex);
			}
		}

		if (pDiscPerSector->subch.current.byP == 0x00 && pDiscPerSector->subch.next.byP == 0xff) {
			OutputSubInfoWithLBALog("P-channel is changed from 0x00 to 0xff. Adr[%02d]\n"
				, nLBA, tmpCurrentTrackNum, pDiscPerSector->subch.current.byAdr);
			pDisc->SUB.n1stPchannelOfTrk = nLBA;
		}
		else if (pDiscPerSector->subch.current.byP == 0xff && pDiscPerSector->subch.next.byP == 0x00) {
			BYTE m, s, f;
			LBAtoMSF(nLBA - pDisc->SUB.n1stPchannelOfTrk, &m, &s, &f);
			OutputSubInfoWithLBALog("P-channel pregap size (%02d:%02d:%02d)", nLBA, tmpCurrentTrackNum, m, s, f);
			if (0 < tIdx && pDisc->SUB.lp1stLBAListOnSub[tIdx][0] != -1) {
				LBAtoMSF(nLBA - pDisc->SUB.lp1stLBAListOnSub[tIdx][0], &m, &s, &f);
				OutputSubInfoLog(" vs Q-channel pregap size (%02d:%02d:%02d)", m, s, f);
				if (pDisc->SUB.n1stPchannelOfTrk != pDisc->SUB.lp1stLBAListOnSub[tIdx][0]) {
					OutputSubInfoLog(" mismatch [%d sector(s)]\n"
						, pDisc->SUB.lp1stLBAListOnSub[tIdx][0] - pDisc->SUB.n1stPchannelOfTrk);
				}
				else {
					OutputSubInfoLog("\n");
				}
			}
			else {
				OutputSubInfoLog("\n");
			}
		}
		// preserve the 1st LBA of the changed trackNum
		if (tmpPrevTrackNum + 1 == tmpCurrentTrackNum) {
			pDiscPerSector->byTrackNum = tmpCurrentTrackNum;
//			tIdx = pDiscPerSector->byTrackNum - 1;

			if (pDisc->SCSI.byFormat == DISK_TYPE_CDI && pDisc->SCSI.toc.LastTrack > 1) {
				if ((pDiscPerSector->subch.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					pDisc->SUB.lp1stLBAListOfDataTrackOnSub[tIdx - 1] = pDisc->SCSI.lp1stLBAListOnToc[tIdx - 1];
					pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx - 1] = nLBA - 1;
				}
			}

			if (pDiscPerSector->subch.current.nRelativeTime != 0) {
				OutputSubInfoWithLBALog("1st RMSF of this track[%d]\n"
					, nLBA, tmpCurrentTrackNum, pDiscPerSector->subch.current.nRelativeTime);
				pDisc->SUB.n1stRmsfOfTrk = pDiscPerSector->subch.current.nRelativeTime;
			}

			if (0 < tmpCurrentIndex && tmpCurrentIndex < MAXIMUM_NUMBER_INDEXES) {
				if (pDisc->SCSI.n1stLBAof2ndSession == -1 &&
					tmpCurrentIndex == 1 &&
					nLBA != pDisc->SCSI.lp1stLBAListOnToc[tIdx]) {
					OutputSubInfoWithLBALog(
						"Subchannel & TOC doesn't sync. LBA on TOC[%d, %#x], index[%02u]\n",
						nLBA, tmpCurrentTrackNum, pDisc->SCSI.lp1stLBAListOnToc[tIdx],
						(UINT)pDisc->SCSI.lp1stLBAListOnToc[tIdx], tmpCurrentIndex);

					pDisc->SUB.lp1stLBAListOnSub[tIdx][1] = pDisc->SCSI.lp1stLBAListOnToc[tIdx];
					if (pDisc->SUB.lp1stLBAListOnSub[tIdx][0] >= pDisc->SUB.lp1stLBAListOnSub[tIdx][1]) {
						OutputSubInfoWithLBALog("Reset pDisc->SUB.lp1stLBAListOnSub[tIdx][0] (changed track sector)\n", nLBA, tmpCurrentTrackNum);
						pDisc->SUB.lp1stLBAListOnSub[tIdx][0] = -1;
					}
					pDisc->SUB.lp1stLBAListOnSubSync[tIdx][1] = nLBA;
					if (pDisc->SUB.lp1stLBAListOnSubSync[tIdx][0] >= pDisc->SUB.lp1stLBAListOnSubSync[tIdx][1]) {
						OutputSubInfoWithLBALog("Reset pDisc->SUB.lp1stLBAListOnSubSync[tIdx][0] (changed track sector)\n", nLBA, tmpCurrentTrackNum);
						pDisc->SUB.lp1stLBAListOnSubSync[tIdx][0] = -1;
					}
					if (*pExecType != gd && *pExecType != swap) {
						pDisc->SUB.byDesync = TRUE;
					}
				}
				else {
					OutputSubInfoWithLBALog(
						"There isn't the pregap on this track\n", nLBA, tmpCurrentTrackNum);
					pDisc->SUB.lp1stLBAListOnSub[tIdx][tmpCurrentIndex] = nLBA;
					pDisc->SUB.lp1stLBAListOnSubSync[tIdx][tmpCurrentIndex] = nLBA;
				}
			}
			// preserve last LBA per data track
			if (0 < tmpPrevTrackNum && tmpPrevTrackNum <= tmpCurrentTrackNum + 1) {
				if (pDisc->SUB.lp1stLBAListOfDataTrackOnSub[tmpPrevTrackNum - 1] != -1 &&
					pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tmpPrevTrackNum - 1] == -1 &&
					(pDiscPerSector->subch.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {

					INT nTmpLastLBA = nLBA - 1;
					if (!pExtArg->byMultiSession &&
						pDisc->SCSI.lpSessionNumList[tmpCurrentTrackNum - 1] > pDisc->SCSI.lpSessionNumList[tmpPrevTrackNum - 1]) {
						nTmpLastLBA -= SESSION_TO_SESSION_SKIP_LBA;
					}
					OutputSubInfoWithLBALog(
						"Last LBA of this data track\n", nTmpLastLBA, tmpPrevTrackNum);
					pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tmpPrevTrackNum - 1] = nTmpLastLBA;

					OutputSubInfoWithLBALog(
						"TrackNum is changed (Prev track is data)\n", nLBA, tmpCurrentTrackNum);
				}
				else if ((pDiscPerSector->subch.prev.byCtl & AUDIO_DATA_TRACK) == 0) {
					OutputSubInfoWithLBALog(
						"TrackNum is changed (Prev track is audio)\n", nLBA, tmpCurrentTrackNum);
				}
			}
		}
		// preserve mode, ctl
		// +11 => some discs have subs and toc desync http://forum.redump.org/post/82442/#p82442
		//        and double offset [FMT] Sangokushi IV, Lip 3: Lipstick Adventure, Gulf War: Soukouden
		// +10 is weird sector https://github.com/saramibreak/DiscImageCreator/issues/173
		if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[tIdx] + 11) {
			pDisc->SUB.lpCtlList[tIdx] = pDiscPerSector->subch.current.byCtl;
			pDisc->MAIN.lpModeList[tIdx] = GetMode(pDiscPerSector, unscrambled);
			OutputSubInfoWithLBALog("Set Ctl[%02d], Mode[%02d]\n"
				, nLBA, tmpCurrentTrackNum, pDisc->SUB.lpCtlList[tIdx], pDisc->MAIN.lpModeList[tIdx]);
		}
		// preserve the 1st LBA of the changed index 
		if (pDiscPerSector->byTrackNum >= 1 && tmpPrevIndex + 1 == tmpCurrentIndex) {
			if (tmpCurrentIndex != 1 && tmpCurrentIndex < MAXIMUM_NUMBER_INDEXES) {
				if (pDisc->SUB.lp1stLBAListOnSub[tIdx][tmpCurrentIndex] == -1) {
					OutputSubInfoWithLBALog("Index (larger than 1) is changed from [%02d] to [%02d]\n"
						, nLBA, tmpCurrentTrackNum, tmpPrevIndex, tmpCurrentIndex);
					pDisc->SUB.lp1stLBAListOnSub[tIdx][tmpCurrentIndex] = nLBA;
					pDisc->SUB.lp1stLBAListOnSubSync[tIdx][tmpCurrentIndex] = nLBA;
				}
			}
			else {
				if (nLBA != pDisc->SCSI.lp1stLBAListOnToc[tIdx]) {
					if (pDisc->SCSI.byFormat == DISK_TYPE_CDI && pDisc->SCSI.toc.LastTrack > 1) {
						// [CD-i Ready] Dimo's Quest (USA)
						// LBA[075149, 0x1258d]: P[ff], Q[41010000000100164374349d]{ Data,      Copy NG,                  Track[01], Idx[00], RMSF[00:00:01], AMSF[16:43:74]}, RtoW[0, 0, 0, 0]
						// LBA[075150, 0x1258e]: P[ff], Q[010101000000001644000c81]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:00:00], AMSF[16:44:00]}, RtoW[0, 0, 0, 0]
						pDisc->SCSI.lp1stLBAListOnToc[tIdx] = nLBA;
						pDisc->SUB.lp1stLBAListOfDataTrackOnSub[tIdx] = 0;
						pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] = nLBA - 1;
					}
					else {
						OutputSubInfoWithLBALog(
							"Subchannel & TOC doesn't sync. LBA on TOC[%d, %#x], prevIndex[%02u]\n",
							nLBA, tmpCurrentTrackNum, pDisc->SCSI.lp1stLBAListOnToc[tIdx],
							(UINT)pDisc->SCSI.lp1stLBAListOnToc[tIdx], tmpPrevIndex);
						if (*pExecType != gd && *pExecType != swap) {
							pDisc->SUB.byDesync = TRUE;
						}
					}
				}
				OutputSubInfoWithLBALog("Index is changed from [%02d] to [%02d]\n"
					, nLBA, tmpCurrentTrackNum, tmpPrevIndex, tmpCurrentIndex);

				pDisc->SUB.lp1stLBAListOnSub[tIdx][1] = pDisc->SCSI.lp1stLBAListOnToc[tIdx];
				if (pDisc->SUB.lp1stLBAListOnSub[tIdx][0] >= pDisc->SUB.lp1stLBAListOnSub[tIdx][1]) {
					// Crow, The - Original Motion Picture Soundtrack (82519-2)
					// LBA 108975, Track[06], Subchannel & TOC isn't sync. LBA on TOC: 108972, prevIndex[00]
					OutputSubInfoWithLBALog("Reset pDisc->SUB.lp1stLBAListOnSub[tIdx][0] (changed index sector)\n", nLBA, tmpCurrentTrackNum);
					pDisc->SUB.lp1stLBAListOnSub[tIdx][0] = -1;
				}

				pDisc->SUB.lp1stLBAListOnSubSync[tIdx][1] = nLBA;
				if (pDisc->SUB.lp1stLBAListOnSubSync[tIdx][0] >= pDisc->SUB.lp1stLBAListOnSubSync[tIdx][1]) {
					OutputSubInfoWithLBALog("Reset pDisc->SUB.lp1stLBAListOnSubSync[tIdx][0] (changed index sector)\n", nLBA, tmpCurrentTrackNum);
					pDisc->SUB.lp1stLBAListOnSubSync[tIdx][0] = -1;
				}
			}
		}
		else if (tmpPrevIndex >= 1 && tmpCurrentIndex == 0) {
#if 0
			for (INT i = pDisc->SCSI.toc.FirstTrack - 1; i < pDisc->SCSI.toc.LastTrack; i++) {
				OutputString("pDisc->SUB.lp1stLBAListOnSub[%d][%d]: %d, %p, LBA: %d\n"
					, i, tmpCurrentIndex, pDisc->SUB.lp1stLBAListOnSub[i][tmpCurrentIndex]
					, &pDisc->SUB.lp1stLBAListOnSub[i][tmpCurrentIndex], nLBA);
			}
#endif
			if (pDisc->SUB.lp1stLBAListOnSub[tIdx][tmpCurrentIndex] == -1) {
				OutputSubInfoWithLBALog(
					"Index is changed from [%02d] to [00]\n", nLBA, tmpCurrentTrackNum, tmpPrevIndex);
				pDisc->SUB.lp1stLBAListOnSub[tIdx][tmpCurrentIndex] = nLBA;
				pDisc->SUB.lp1stLBAListOnSubSync[tIdx][tmpCurrentIndex] = nLBA;
			}
#if 0
			for (INT i = pDisc->SCSI.toc.FirstTrack - 1; i < pDisc->SCSI.toc.LastTrack; i++) {
				OutputString("pDisc->SUB.lp1stLBAListOnSub[%d][%d]: %d, %p, LBA: %d\n"
					, i, tmpCurrentIndex, pDisc->SUB.lp1stLBAListOnSub[i][tmpCurrentIndex]
					, &pDisc->SUB.lp1stLBAListOnSub[i][tmpCurrentIndex], nLBA);
			}
#endif
		}

		INT tIdx2 = tIdx;
		if (pDisc->SCSI.byFormat == DISK_TYPE_CDI && pDisc->SCSI.toc.LastTrack > 1) {
			tIdx2 -= 1;
			if (tIdx2 < 0) {
				return;
			}
		}
		if (!(pDisc->SCSI.byFormat == DISK_TYPE_CDI && pDisc->SCSI.toc.LastTrack > 1)) {
			if ((pDisc->SCSI.toc.TrackData[tIdx2].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
				(pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == 0) {
				OutputMainInfoWithLBALog(
					"Data track, but this sector is audio\n", nLBA, tmpCurrentTrackNum);
			}
			else if ((pDisc->SCSI.toc.TrackData[tIdx2].Control & AUDIO_DATA_TRACK) == 0 &&
				(pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				OutputMainInfoWithLBALog(
					"Audio track, but this sector is data\n", nLBA, tmpCurrentTrackNum);
			}
		}

		if (pExtArg->byReverse) {
			// preserve last LBA per data track
			if (nLBA == pDisc->SCSI.nLastLBAofDataTrkOnToc) {
				if (pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] == -1 &&
					(pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] = pDisc->SCSI.nLastLBAofDataTrkOnToc;
				}
			}
			else if ((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				pDisc->SUB.lp1stLBAListOfDataTrackOnSub[tIdx] = nLBA;
			}
		}
		else {
			// preserve first LBA per data track
			if (pDisc->SUB.lp1stLBAListOfDataTrackOnSub[tIdx] == -1 &&
				(pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				OutputSubInfoWithLBALog("1st LBA of this data track\n", nLBA, tmpCurrentTrackNum);
				pDisc->SUB.lp1stLBAListOfDataTrackOnSub[tIdx] = nLBA;
			}
			else if (nLBA == pDisc->SCSI.nAllLength - 1) {
				if (pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] == -1) {
					// preserve last LBA per data track
					if ((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
						OutputSubInfoWithLBALog(
							"Last LBA of this data track (and this disc)\n", nLBA, tmpCurrentTrackNum);
						pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] = pDisc->SCSI.nAllLength - 1;
					}
				}
				OutputSubInfoWithLBALog("Last LBA of this disc\n", nLBA, tmpCurrentTrackNum);
			}
		}
	}
	else if (*pExecType != swap && (pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		// '110' is Lead-out
		if (tmpCurrentTrackNum == 110 ||
			(tmpCurrentTrackNum == 0 &&
				// '100'&'101' is Lead-in
				(tmpCurrentIndex == 100 || tmpCurrentIndex == 101 ||
					(tmpCurrentIndex == 0 &&
						pDiscPerSector->subch.current.nAbsoluteTime == 0) ||
					// MAQIUPAI [unlicensed DC]
					// LBA[088955, 0x15b7b]: P[00], Q[01aa01012974001948057886]{Audio, 2ch, Copy NG, Pre-emphasis No, LeadOut  , Idx[01], RMSF[01:29:74], AMSF[19:48:05]}, RtoW[0, 0, 0, 0]
					// LBA[088956, 0x15b7c]: P[00], Q[410012194806002050060e6d]{ Data,      Copy NG,                  Point[12], AMSF[19:48:06], StartTimeOfTrack[20:50:06]}, RtoW[0, 0, 0, 0]
					// LBA[088957, 0x15b7d]: P[00], Q[41001219480700205006a43c]{ Data,      Copy NG,                  Point[12], AMSF[19:48:07], StartTimeOfTrack[20:50:06]}, RtoW[0, 0, 0, 0]
					// LBA[088958, 0x15b7e]: P[00], Q[41001219480800205006c1c5]{ Data,      Copy NG,                  Point[12], AMSF[19:48:08], StartTimeOfTrack[20:50:06]}, RtoW[0, 0, 0, 0]
					// LBA[088959, 0x15b7f]: P[00], Q[4100a0194809001220009c35]{ Data,      Copy NG,                  Point[a0], AMSF[19:48:09], TrackNumOf1stTrack[12], ProgramAreaFormat[20]}, RtoW[0, 0, 0, 0]
					(tmpCurrentIndex == pDisc->SCSI.by1stMultiSessionTrkNum &&
						pDiscPerSector->subch.current.nRelativeTime == pDiscPerSector->subch.prev.nAbsoluteTime + 1)
					)
				)
			) {
			if (pDisc->SUB.lp1stLBAListOfDataTrackOnSub[pDiscPerSector->byTrackNum - 1] == -1) {
				OutputSubInfoWithLBALog("1st LBA of Lead-out or Lead-in\n", nLBA, pDiscPerSector->byTrackNum);
				pDisc->SUB.lp1stLBAListOfDataTrackOnSub[pDiscPerSector->byTrackNum - 1] = nLBA;
				if (pDiscPerSector->byTrackNum < pDisc->SCSI.by1stDataTrkNum) {
					pDisc->SCSI.by1stDataTrkNum = pDiscPerSector->byTrackNum;
				}
			}
			else if (nLBA != pDisc->SCSI.n1stLBAof2ndSession - 150) {
				OutputSubInfoWithLBALog(
					"Set the last LBA of data track [%d]->[%d]\n", nLBA, pDiscPerSector->byTrackNum,
					pDisc->SUB.lpLastLBAListOfDataTrackOnSub[pDiscPerSector->byTrackNum - 1], nLBA);
				pDisc->SUB.lpLastLBAListOfDataTrackOnSub[pDiscPerSector->byTrackNum - 1] = nLBA;
			}
		}
	}
}

VOID SetAdr6ToString(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPSTR pszOutString,
	BOOL bCopy
) {
#ifdef _WIN32
	size_t size = META_ADR6_SIZE - 1;
#else
	size_t size = META_ADR6_SIZE;
#endif
	_snprintf(pszOutString, size, "%02x%02x%02x%02x%02x%02x%02x%02x"
		, lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16]
		, lpSubcode[17], lpSubcode[18], lpSubcode[19], lpSubcode[20]);
	pszOutString[META_ADR6_SIZE - 1] = 0;
	if (bCopy) {
		strncpy(pDisc->SUB.szAdr6, pszOutString, sizeof(pDisc->SUB.szAdr6) - 1);
	}
}

VOID SetISRCToString(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	LPSTR pszOutString,
	BOOL bCopy
) {
	/*
	BYTE＼bit |	7	 6	  5	  4	  3	  2	  1	  0
	0	      |      Ctrl	    |		ADR
	1		  |		I01				    |(MSB) I02
	2		  |		I02　(LSB)  |(MSB)　　I03
	3		  |I03 (LSB)|			I04
	4		  |I05						|ZERO
	5		  |I06			    |I07
	6		  |I08			    |I09
	7		  |I10			    |I11
	8		  |I12			    |ZERO
	9	フレーム (=1/75秒) (CD全体の経過時間)(BCD)
	10	
	(MSB)
	CRC　P(x)=x16+x12+x5+x1
	(LSB)
	11

	I01 ～ I02 : 国名コード (6ビット)
	I03 ～ I05 : 登録者コード (6ビット)
	I06 ～ I07 : 記録年 (4ビット)
	I08 ～ I12 : シリアルナンバー (4ビット)

	これをASCIIコードに変換するには、それぞれに 0x30 を足してやります。
	*/
#ifdef _WIN32
	size_t size = META_ISRC_SIZE - 1;
#else
	size_t size = META_ISRC_SIZE;
#endif
	_snprintf(pszOutString, size, "%c%c%c%c%c%c%c%c%c%c%c%c",
		((pDiscPerSector->subcode.current[13] >> 2) & 0x3f) + 0x30,
		(((pDiscPerSector->subcode.current[13] << 4) & 0x30) | ((pDiscPerSector->subcode.current[14] >> 4) & 0x0f)) + 0x30, 
		(((pDiscPerSector->subcode.current[14] << 2) & 0x3c) | ((pDiscPerSector->subcode.current[15] >> 6) & 0x03)) + 0x30, 
		(pDiscPerSector->subcode.current[15] & 0x3f) + 0x30, 
		((pDiscPerSector->subcode.current[16] >> 2) & 0x3f) + 0x30, 
		((pDiscPerSector->subcode.current[17] >> 4) & 0x0f) + 0x30, (pDiscPerSector->subcode.current[17] & 0x0f) + 0x30, 
		((pDiscPerSector->subcode.current[18] >> 4) & 0x0f) + 0x30, (pDiscPerSector->subcode.current[18] & 0x0f) + 0x30, 
		((pDiscPerSector->subcode.current[19] >> 4) & 0x0f) + 0x30, (pDiscPerSector->subcode.current[19] & 0x0f) + 0x30,
		((pDiscPerSector->subcode.current[20] >> 4) & 0x0f) + 0x30);
	pszOutString[META_ISRC_SIZE - 1] = 0;
	if (bCopy) {
		strncpy(pDisc->SUB.pszISRC[pDiscPerSector->byTrackNum - 1], pszOutString, META_ISRC_SIZE - 1);
	}
}

VOID SetMCNToString(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPSTR pszOutString,
	BOOL bCopy
) {
#ifdef _WIN32
	size_t size = META_CATALOG_SIZE - 1;
#else
	size_t size = META_CATALOG_SIZE;
#endif
	_snprintf(pszOutString, size, "%c%c%c%c%c%c%c%c%c%c%c%c%c",
		((lpSubcode[13] >> 4) & 0x0f) + 0x30, (lpSubcode[13] & 0x0f) + 0x30, 
		((lpSubcode[14] >> 4) & 0x0f) + 0x30, (lpSubcode[14] & 0x0f) + 0x30, 
		((lpSubcode[15] >> 4) & 0x0f) + 0x30, (lpSubcode[15] & 0x0f) + 0x30, 
		((lpSubcode[16] >> 4) & 0x0f) + 0x30, (lpSubcode[16] & 0x0f) + 0x30, 
		((lpSubcode[17] >> 4) & 0x0f) + 0x30, (lpSubcode[17] & 0x0f) + 0x30, 
		((lpSubcode[18] >> 4) & 0x0f) + 0x30, (lpSubcode[18] & 0x0f) + 0x30, 
		((lpSubcode[19] >> 4) & 0x0f) + 0x30);
#if 0
	printf("mcn1: %s\n", pszOutString);
	printf("mcn2: %c%c%c%c%c%c%c%c%c%c%c%c%c\n",
		pszOutString[0], pszOutString[1],
		pszOutString[2], pszOutString[3],
		pszOutString[4], pszOutString[5],
		pszOutString[6], pszOutString[7],
		pszOutString[8], pszOutString[9],
		pszOutString[10], pszOutString[11],
		pszOutString[12]);
	printf("mcn3: %c%c%c%c%c%c%c%c%c%c%c%c%c\n",
		((lpSubcode[13] >> 4) & 0x0f) + 0x30, (lpSubcode[13] & 0x0f) + 0x30,
		((lpSubcode[14] >> 4) & 0x0f) + 0x30, (lpSubcode[14] & 0x0f) + 0x30,
		((lpSubcode[15] >> 4) & 0x0f) + 0x30, (lpSubcode[15] & 0x0f) + 0x30,
		((lpSubcode[16] >> 4) & 0x0f) + 0x30, (lpSubcode[16] & 0x0f) + 0x30,
		((lpSubcode[17] >> 4) & 0x0f) + 0x30, (lpSubcode[17] & 0x0f) + 0x30,
		((lpSubcode[18] >> 4) & 0x0f) + 0x30, (lpSubcode[18] & 0x0f) + 0x30,
		((lpSubcode[19] >> 4) & 0x0f) + 0x30);
#endif
	pszOutString[META_CATALOG_SIZE - 1] = 0;
	if (bCopy) {
		strncpy(pDisc->SUB.szCatalog, pszOutString, sizeof(pDisc->SUB.szCatalog) - 1);
	}
}

VOID SetLBAForFirstAdr(
	INT n1stLBA[][2],
	INT nRangeLBA[][2],
	LPCTSTR strAdr,
	LPINT nAdrLBAList,
	BYTE byIdxOfSession,
	BYTE byPlxtrDrive
) {
	INT first = nAdrLBAList[1] - nAdrLBAList[0];
	INT second = nAdrLBAList[2] - nAdrLBAList[1];
	INT third = nAdrLBAList[3] - nAdrLBAList[2];
	INT fourth = nAdrLBAList[4] - nAdrLBAList[3];
	INT betweenThirdOne = nAdrLBAList[2] - nAdrLBAList[0];
	INT betweenTourthTwo = nAdrLBAList[3] - nAdrLBAList[1];
	INT betweenFifthThird = nAdrLBAList[4] - nAdrLBAList[2];
	if (first == second && first == third) {
		if (n1stLBA[0][byIdxOfSession] == -1) {
			n1stLBA[0][byIdxOfSession] = nAdrLBAList[0];
			nRangeLBA[0][byIdxOfSession] = nAdrLBAList[1] - nAdrLBAList[0];
			if (byPlxtrDrive == PLXTR_DRIVE_TYPE::PXS88T) {
				// Somehow PX-S88T is sliding subchannel +1;
				n1stLBA[0][byIdxOfSession]++;
			}
		}
		OutputDiscLog(
			"\tSession %d, 1st %s sector is %d, %s sector exists per %d byte\n"
			, byIdxOfSession + 1, strAdr, nAdrLBAList[0]
			, strAdr, nAdrLBAList[1] - nAdrLBAList[0]);
	}
	else if (second == third && third == fourth) {
		// Originally, MCN sector exists per same frame number, but in case of 1st sector or next idx of the track, MCN sector slides at the next sector
		//
		// Shake the fake [Kyosuke Himuro] [First MCN Sector: 1, MCN sector exists per 91 byte]
		// LBA[000000, 0000000], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:00:00], AMSF[00:02:00], RtoW[0, 0, 0, 0]
		// LBA[000001, 0x00001], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [4988006116269], AMSF[     :01], RtoW[0, 0, 0, 0]
		// LBA[000002, 0x00002], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:00:02], AMSF[00:02:02], RtoW[0, 0, 0, 0]
		//  :
		// LBA[000090, 0x0005a], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:01:15], AMSF[00:03:15], RtoW[0, 0, 0, 0]
		// LBA[000091, 0x0005b], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [4988006116269], AMSF[     :16], RtoW[0, 0, 0, 0]
		// LBA[000092, 0x0005c], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:01:17], AMSF[00:03:17], RtoW[0, 0, 0, 0]
		//  :
		// LBA[000181, 0x000b5], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:02:31], AMSF[00:04:31], RtoW[0, 0, 0, 0]
		// LBA[000182, 0x000b6], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [4988006116269], AMSF[     :32], RtoW[0, 0, 0, 0]
		// LBA[000183, 0x000b7], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:02:33], AMSF[00:04:33], RtoW[0, 0, 0, 0]
		if (n1stLBA[0][byIdxOfSession] == -1) {
			n1stLBA[0][byIdxOfSession] = nAdrLBAList[0] - 1;
			nRangeLBA[0][byIdxOfSession] = nAdrLBAList[1] - nAdrLBAList[0] + 1;
			if (byPlxtrDrive == PLXTR_DRIVE_TYPE::PXS88T) {
				// Somehow PX-S88T is sliding subchannel +1;
				n1stLBA[0][byIdxOfSession]++;
			}
		}
		OutputDiscLog(
			"\tSession %d, 1st %s sector is %d, %s sector exists per %d byte\n"
			, byIdxOfSession + 1, strAdr, nAdrLBAList[0] - 1
			, strAdr, nAdrLBAList[1] - nAdrLBAList[0] + 1);
	}
	else if (betweenThirdOne == betweenTourthTwo && betweenThirdOne == betweenFifthThird) {
		if (n1stLBA[0][byIdxOfSession] == -1) {
			n1stLBA[0][byIdxOfSession] = nAdrLBAList[0];
			nRangeLBA[0][byIdxOfSession] = betweenThirdOne;
			n1stLBA[1][byIdxOfSession] = nAdrLBAList[1];
			nRangeLBA[1][byIdxOfSession] = betweenTourthTwo;
			if (byPlxtrDrive == PLXTR_DRIVE_TYPE::PXS88T) {
				// Somehow PX-S88T is sliding subchannel +1;
				n1stLBA[0][byIdxOfSession]++;
				n1stLBA[1][byIdxOfSession]++;
			}
		}
		OutputDiscLog(
			"\tSession %d, 1st %s sector is %d, %s sector exists per %d\n"
			"\t            2nd %s sector is %d, %s sector exists per %d\n"
			, byIdxOfSession + 1
			, strAdr, nAdrLBAList[0], strAdr, betweenThirdOne
			, strAdr, nAdrLBAList[1], strAdr, betweenTourthTwo);
	}
	else if (first == second || first == third || second == third) {
		if (n1stLBA[0][byIdxOfSession] == -1) {
			n1stLBA[0][byIdxOfSession] = nAdrLBAList[0];
			nRangeLBA[0][byIdxOfSession] = nAdrLBAList[1] - nAdrLBAList[0];
			n1stLBA[1][byIdxOfSession] = nAdrLBAList[0];
			nRangeLBA[1][byIdxOfSession] = nAdrLBAList[2] - nAdrLBAList[1];
			n1stLBA[2][byIdxOfSession] = nAdrLBAList[0];
			nRangeLBA[2][byIdxOfSession] = nAdrLBAList[3] - nAdrLBAList[2];
			if (byPlxtrDrive == PLXTR_DRIVE_TYPE::PXS88T) {
				// Somehow PX-S88T is sliding subchannel +1;
				n1stLBA[0][byIdxOfSession]++;
				n1stLBA[1][byIdxOfSession]++;
				n1stLBA[2][byIdxOfSession]++;
			}
		}
		OutputDiscLog(
			"\tSession %d, 1st %s sector is %d, %s sector exists per %d, %d, %d\n"
			, byIdxOfSession + 1
			, strAdr, nAdrLBAList[0]
			, strAdr, nAdrLBAList[1] - nAdrLBAList[0]
			, nAdrLBAList[2] - nAdrLBAList[1]
			, nAdrLBAList[3] - nAdrLBAList[2]);
	}
}

VOID SetBufferFromTmpSubch(
	LPBYTE lpSubcode,
	SUBCH_PER_SECTOR subch,
	BOOL bCurrent,
	BOOL bUpdateCrc
) {
	if (subch.byTrackNum == 110) {
		if (subch.byAdr == 3) {
			subch.byAdr = 1;
		}
		lpSubcode[12] = BYTE(subch.byCtl << 4 | subch.byAdr);
		lpSubcode[13] = 0xaa;
	}
	else {
		lpSubcode[12] = BYTE(subch.byCtl << 4 | subch.byAdr);
		lpSubcode[13] = DecToBcd(subch.byTrackNum);
	}
	lpSubcode[14] = DecToBcd(subch.byIndex);
	BYTE m, s, f;
	if (bCurrent) {
		LBAtoMSF(subch.nRelativeTime, &m, &s, &f);
	}
	else {
		LBAtoMSF(subch.nRelativeTime + 1, &m, &s, &f);
	}
	lpSubcode[15] = DecToBcd(m);
	lpSubcode[16] = DecToBcd(s);
	lpSubcode[17] = DecToBcd(f);
	if (bCurrent) {
		LBAtoMSF(subch.nAbsoluteTime, &m, &s, &f);
	}
	else {
		LBAtoMSF(subch.nAbsoluteTime + 1, &m, &s, &f);
	}
	lpSubcode[19] = DecToBcd(m);
	lpSubcode[20] = DecToBcd(s);
	lpSubcode[21] = DecToBcd(f);
	if (bUpdateCrc) {
		WORD crc16 = (WORD)GetCrc16CCITT(10, &lpSubcode[12]);
		lpSubcode[22] = HIBYTE(crc16);
		lpSubcode[23] = LOBYTE(crc16);
	}
}

VOID SetBufferFromMCN(
	PDISC pDisc,
	LPBYTE lpSubcode
) {
	for (INT i = 13, j = 0; i < 19; i++, j += 2) {
		lpSubcode[i] = (BYTE)(pDisc->SUB.szCatalog[j] - 0x30);
		lpSubcode[i] = (BYTE)(lpSubcode[i] << 4);
		lpSubcode[i] |= (BYTE)(pDisc->SUB.szCatalog[j + 1] - 0x30);
	}
	lpSubcode[19] = (BYTE)(pDisc->SUB.szCatalog[12] - 0x30);
	lpSubcode[19] = (BYTE)(lpSubcode[19] << 4);
	lpSubcode[20] = 0;
}

VOID SetTmpSubchFromBuffer(
	PSUBCH_PER_SECTOR pSubch,
	LPBYTE lpSubcode
) {
	INT nFFCnt = 0;
	INT n00Cnt = 0;
	for (INT i = 0; i < 12; i++) {
		if (lpSubcode[i] == 0xff) {
			nFFCnt++;
		}
		else if (lpSubcode[i] == 0x00) {
			n00Cnt++;
		}
	}
	if (nFFCnt >= n00Cnt) {
		pSubch->byP = 0xff;
	}
	else {
		pSubch->byP = 0x00;
	}
	pSubch->byCtl = (BYTE)((BYTE)(lpSubcode[12] >> 4) & 0x0f);
	pSubch->byAdr = (BYTE)(lpSubcode[12] & 0x0f);
	pSubch->byTrackNum = BcdToDec(lpSubcode[13]);
	pSubch->byIndex = BcdToDec(lpSubcode[14]);
	pSubch->nRelativeTime = MSFtoLBA(BcdToDec(lpSubcode[15]),
		BcdToDec(lpSubcode[16]), BcdToDec(lpSubcode[17]));
	pSubch->nAbsoluteTime = MSFtoLBA(BcdToDec(lpSubcode[19]),
		BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21]));
}

VOID UpdateTmpSubchForMCN(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA
) {
	// Subchannel pattern on MCN Sector
	// Tomb Raider III - Adventures of Lara Croft (Europe)(Mac)
	if (pDisc->SCSI.toc.LastTrack == 1) {
		pDiscPerSector->subch.current.byTrackNum = 1;
		pDiscPerSector->subch.current.byIndex = 1;
		if (nLBA > 0) {
			pDiscPerSector->subch.current.nRelativeTime += 1;
		}
	}
	// pattern 1: pregap sector.
	else if (IsValidPregapSector(pDisc, &pDiscPerSector->subch, nLBA)) {
		BOOL bValidPre = FALSE;
		// pattern 1-1: prev sector is audio.
		if ((pDiscPerSector->subch.prev.byCtl & AUDIO_DATA_TRACK) == 0) {
			// pattern 1-1-1: current sector is audio.
			if ((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == 0) {
				if (pDiscPerSector->subch.prev.byTrackNum > 0) {
					if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] - 225) {
						// Atlas, The - Renaissance Voyager (Japan)
						// LBA[003364, 0x00d24], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:44:64], AMSF[00:46:64], RtoW[0, 0, 0, 0]
						// LBA[003365, 0x00d25], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :65], RtoW[0, 0, 0, 0]
						// LBA[003366, 0x00d26], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:02:73], AMSF[00:46:66], RtoW[0, 0, 0, 0]
						// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
						// LBA[261585, 0x3fDD1], Audio, 2ch, Copy NG, Pre-emphasis No, Track[95], Idx[01], RMSF[00:13:69], AMSF[58:09:60], RtoW[0, 0, 0, 0]
						// LBA[261586, 0x3fDD2], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :61], RtoW[0, 0, 0, 0]
						// LBA[261587, 0x3fDD3], Audio, 2ch, Copy NG, Pre-emphasis No, Track[96], Idx[00], RMSF[00:02:73], AMSF[58:09:62], RtoW[0, 0, 0, 0]
						pDiscPerSector->subch.current.nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] - 150) {
						pDiscPerSector->subch.current.nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] - 149) {
						pDiscPerSector->subch.current.nRelativeTime = 148;
						bValidPre = TRUE;
					}
				}
				if (bValidPre) {
					// pattern 1-1-1-1: change track.
					pDiscPerSector->subch.current.byTrackNum = (BYTE)(pDiscPerSector->subch.prev.byTrackNum + 1);
					// pattern 1-1-1-1-1: change index. (pregap sector is 0)
					pDiscPerSector->subch.current.byIndex = 0;
					OutputSubInfoWithLBALog(
						"The 1st pregap sector of Audio Track %02d is replaced with EAN sector (in this case it's NOT the last sector of Audio Track %02d)\n"
						, nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subch.current.byTrackNum, pDiscPerSector->subch.prev.byTrackNum);
				}
				else {
					// pattern 1-1-1-2: not change track.
					pDiscPerSector->subch.current.byTrackNum = pDiscPerSector->subch.prev.byTrackNum;
					// pattern 1-1-1-2-2: not change index.
					pDiscPerSector->subch.current.byIndex = pDiscPerSector->subch.prev.byIndex;
				}
			}
			// pattern 1-1-2: current sector is data.
			else if ((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				if (pDiscPerSector->subch.prev.byTrackNum > 0) {
					if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] - 225) {
						pDiscPerSector->subch.current.nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] - 150) {
						pDiscPerSector->subch.current.nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] - 149) {
						pDiscPerSector->subch.current.nRelativeTime = 148;
						bValidPre = TRUE;
					}
				}
				if (bValidPre) {
					// pattern 1-1-2-1: change track.
					pDiscPerSector->subch.current.byTrackNum = (BYTE)(pDiscPerSector->subch.prev.byTrackNum + 1);
					// pattern 1-1-2-1-1: change index. (pregap sector is 0)
					pDiscPerSector->subch.current.byIndex = 0;
					OutputSubInfoWithLBALog(
						"The 1st pregap sector of Data Track %02d is replaced with EAN sector (in this case it's NOT the last sector of Audio Track %02d)\n"
						, nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subch.current.byTrackNum, pDiscPerSector->subch.prev.byTrackNum);
				}
				else {
					// pattern 1-1-2-2: not change track.
					pDiscPerSector->subch.current.byTrackNum = pDiscPerSector->subch.prev.byTrackNum;
					// pattern 1-1-2-2-2: not change index.
					pDiscPerSector->subch.current.byIndex = pDiscPerSector->subch.prev.byIndex;
				}
			}
		}
		// pattern 1-2: prev sector is data.
		else if ((pDiscPerSector->subch.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			// pattern 1-2-1: current sector is audio.
			if ((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == 0) {
				if (pDiscPerSector->subch.prev.byTrackNum > 0) {
					if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] - 225) {
						pDiscPerSector->subch.current.nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] - 150) {
						// Valis IV (Japan)
						// LBA[157830, 0x26886],  Data,      Copy NG,                  Track[44], Idx[01], RMSF[00:06:27], AMSF[35:06:30], RtoW[0, 0, 0, 0]
						// LBA[157831, 0x26887], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :31], RtoW[0, 0, 0, 0]
						// LBA[157832, 0x26888], Audio, 2ch, Copy NG, Pre-emphasis No, Track[45], Idx[00], RMSF[00:01:73], AMSF[35:06:32], RtoW[0, 0, 0, 0]
						// Cosmic Fantasy 2
						// LBA[202749, 0x317FD],  Data,      Copy NG,                  Track[80], Idx[01], RMSF[00:06:63], AMSF[45:05:24], RtoW[0, 0, 0, 0]
						// LBA[202750, 0x317FE], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :25], RtoW[0, 0, 0, 0]
						// LBA[202751, 0x317FF], Audio, 2ch, Copy NG, Pre-emphasis No, Track[81], Idx[00], RMSF[00:01:73], AMSF[45:05:26], RtoW[0, 0, 0, 0]
						pDiscPerSector->subch.current.nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] - 149) {
						pDiscPerSector->subch.current.nRelativeTime = 148;
						bValidPre = TRUE;
					}
				}
				if (bValidPre) {
					// pattern 1-2-1-1: change track.
					pDiscPerSector->subch.current.byTrackNum = (BYTE)(pDiscPerSector->subch.prev.byTrackNum + 1);
					// pattern 1-2-1-1-1: change index. (pregap sector is 0)
					pDiscPerSector->subch.current.byIndex = 0;
					OutputSubInfoWithLBALog(
						"The 1st pregap sector of Audio Track %02d is replaced with EAN sector (in this case it's NOT the last sector of Data Track %02d)\n"
						, nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subch.current.byTrackNum, pDiscPerSector->subch.prev.byTrackNum);
				}
				else {
					// pattern 1-2-1-2: not change track.
					pDiscPerSector->subch.current.byTrackNum = pDiscPerSector->subch.prev.byTrackNum;
					// pattern 1-2-1-2-2: not change index.
					pDiscPerSector->subch.current.byIndex = pDiscPerSector->subch.prev.byIndex;
				}
			}
			// pattern 1-2-2: current sector is data.
			else if ((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				if (pDiscPerSector->subch.prev.byTrackNum > 0) {
					if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] - 225) {
						pDiscPerSector->subch.current.nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] - 150) {
						// [IBM-PC] Hexen: Beyond Heretic (USA)
						// LBA[021026, 0x05222]: P[00], Q[41010104402600044226d37d]{ Data,      Copy NG,                  Track[01], Idx[01], RMSF[04:40:26], AMSF[04:42:26]}, RtoW[0, 0, 0, 0]
						// LBA[021027, 0x05223]: P[00], Q[4200000000000000002707ea]{ Data,      Copy NG,                  MediaCatalogNumber [0000000000000], AMSF[     :27]}, RtoW[0, 0, 0, 0]
						// LBA[021028, 0x05224]: P[ff], Q[0102000001740004422895f9]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:01:74], AMSF[04:42:28]}, RtoW[0, 0, 0, 0]
						//  :
						// LBA[021176, 0x052b8]: P[ff], Q[01020000000100044426a5e0]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:00:01], AMSF[04:44:26]}, RtoW[0, 0, 0, 0]
						// LBA[021177, 0x052b9]: P[ff], Q[010201000000000444275843]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[01], RMSF[00:00:00], AMSF[04:44:27]}, RtoW[0, 0, 0, 0]
						pDiscPerSector->subch.current.nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] - 149) {
						pDiscPerSector->subch.current.nRelativeTime = 148;
						bValidPre = TRUE;
					}
				}
				if (bValidPre) {
					// pattern 1-2-2-1: change track.
					pDiscPerSector->subch.current.byTrackNum = (BYTE)(pDiscPerSector->subch.prev.byTrackNum + 1);
					// pattern 1-2-2-1-1: change index. (pregap sector is 0)
					pDiscPerSector->subch.current.byIndex = 0;
					OutputSubInfoWithLBALog(
						"The 1st pregap sector of Data Track %02d is replaced with EAN sector (in this case it's NOT the last sector of Data Track %02d)\n"
						, nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subch.current.byTrackNum, pDiscPerSector->subch.prev.byTrackNum);
				}
				else {
					// pattern 1-2-2-2: not change track.
					pDiscPerSector->subch.current.byTrackNum = pDiscPerSector->subch.prev.byTrackNum;
					// pattern 1-2-2-2-2: not change index.
					pDiscPerSector->subch.current.byIndex = pDiscPerSector->subch.prev.byIndex;
				}
			}
		}
	}
	// pattern 2: not pregap sector.
	else {
		// pattern 2-1: prev sector is audio.
		if ((pDiscPerSector->subch.prev.byCtl & AUDIO_DATA_TRACK) == 0) {
			// pattern 2-1-1: current sector is audio.
			if ((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == 0) {
				// 1st sector of tracks
				if (pDiscPerSector->subch.prev.byTrackNum > 0 &&
					nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum]) {
					// [PCE] Madou Monogatari I - Honoo no Sotsuenji (Japan)
					// LBA[183031, 0x2caf7]: P[ff], Q[01210100317000404231bc6d]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[21], Idx[01], RMSF[00:31:70], AMSF[40:42:31]}, RtoW[0, 0, 0, 0]
					// LBA[183032, 0x2caf8]: P[ff], Q[020000000000000000323764]{Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :32]}, RtoW[0, 0, 0, 0]
					// LBA[183033, 0x2caf9]: P[00], Q[012201000001004042336c90]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[22], Idx[01], RMSF[00:00:01], AMSF[40:42:33]}, RtoW[0, 0, 0, 0]
					pDiscPerSector->subch.current.nRelativeTime = 0;
					// pattern 2-1-1-1: change track.
					pDiscPerSector->subch.current.byTrackNum = (BYTE)(pDiscPerSector->subch.prev.byTrackNum + 1);
					// pattern 2-1-1-1-2: not change index.
					pDiscPerSector->subch.current.byIndex = pDiscPerSector->subch.prev.byIndex;
					OutputSubInfoWithLBALog(
						"The 1st sector of Audio Track %02d is replaced with EAN sector (in this case it's NOT the last sector of Audio Track %02d)\n"
						, nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subch.current.byTrackNum, pDiscPerSector->subch.prev.byTrackNum);
				}
				// 1st next index of same tracks
				else if (pDiscPerSector->subch.prev.byIndex == 0 && pDiscPerSector->subch.prev.nRelativeTime == 0) {
					// Psychic Detective Series Vol. 5 - Nightmare (Japan)
					// LBA[080999, 0x13C67], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:00:00], AMSF[18:01:74], RtoW[0, 0, 0, 0]
					// LBA[081000, 0x13C68], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [3010911111863], AMSF[     :00], RtoW[0, 0, 0, 0]
					// LBA[081001, 0x13C69], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[01], RMSF[00:00:01], AMSF[18:02:01], RtoW[0, 0, 0, 0]
					pDiscPerSector->subch.current.nRelativeTime = 0;
					// pattern 2-1-1-2: not change track.
					pDiscPerSector->subch.current.byTrackNum = pDiscPerSector->subch.prev.byTrackNum;
					// pattern 2-1-1-2-1: change index.
					pDiscPerSector->subch.current.byIndex = (BYTE)(pDiscPerSector->subch.prev.byIndex + 1);
				}
				// 1st index of same tracks
				else if (pDiscPerSector->subch.prev.byIndex > 1 && pDiscPerSector->subch.prev.byIndex != pDiscPerSector->subch.next.byIndex) {
					if (pDiscPerSector->subch.prev.byIndex + 1 == pDiscPerSector->subch.next.byIndex) {
						// Space Jam (Japan)
						// LBA[056262, 0x0dbc6], Audio, 2ch, Copy NG, Pre-emphasis No, Track[04], Idx[53], RMSF[01:38:65], AMSF[12:32:12], RtoW[0, 0, 0, 0]
						// LBA[056263, 0x0dbc7], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :13], RtoW[0, 0, 0, 0]
						// LBA[056264, 0x0dbc8], Audio, 2ch, Copy NG, Pre-emphasis No, Track[04], Idx[54], RMSF[01:38:67], AMSF[12:32:14], RtoW[0, 0, 0, 0]
						// Space Jam (Japan)
						// LBA[086838, 0x15336], Audio, 2ch, Copy NG, Pre-emphasis No, Track[06], Idx[82], RMSF[02:31:05], AMSF[19:19:63], RtoW[0, 0, 0, 0]
						// LBA[086839, 0x15337], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :64], RtoW[0, 0, 0, 0]
						// LBA[086840, 0x15338], Audio, 2ch, Copy NG, Pre-emphasis No, Track[06], Idx[83], RMSF[02:31:07], AMSF[19:19:65], RtoW[0, 0, 0, 0]
						pDiscPerSector->subch.current.nRelativeTime = pDiscPerSector->subch.prev.nRelativeTime + 1;
						// pattern 2-1-1-2: not change track.
						pDiscPerSector->subch.current.byTrackNum = pDiscPerSector->subch.prev.byTrackNum;
						// pattern 2-1-1-2-1: change index.
						pDiscPerSector->subch.current.byIndex = (BYTE)(pDiscPerSector->subch.prev.byIndex + 1);
					}
				}
				// same index of same tracks
				else {
					if (pDiscPerSector->subch.prev.byIndex == 0) {
						// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
						// LBA[003413, 0x00D55], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:02:26], AMSF[00:47:38], RtoW[0, 0, 0, 0]
						// LBA[003414, 0x00D56], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :39], RtoW[0, 0, 0, 0]
						// LBA[003415, 0x00D57], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:02:24], AMSF[00:47:40], RtoW[0, 0, 0, 0]
						pDiscPerSector->subch.current.nRelativeTime = pDiscPerSector->subch.prev.nRelativeTime - 1;
					}
					else if (pDiscPerSector->subch.prev.byIndex > 0) {
						// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
						// LBA[081541, 0x13E85], Audio, 2ch, Copy NG, Pre-emphasis No, Track[18], Idx[01], RMSF[00:12:57], AMSF[18:09:16], RtoW[0, 0, 0, 0]
						// LBA[081542, 0x13E86], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :17], RtoW[0, 0, 0, 0]
						// LBA[081543, 0x13E87], Audio, 2ch, Copy NG, Pre-emphasis No, Track[18], Idx[01], RMSF[00:12:59], AMSF[18:09:18], RtoW[0, 0, 0, 0]
						pDiscPerSector->subch.current.nRelativeTime = pDiscPerSector->subch.prev.nRelativeTime + 1;
					}
					// pattern 2-1-1-2: not change track.
					pDiscPerSector->subch.current.byTrackNum = pDiscPerSector->subch.prev.byTrackNum;
					// pattern 2-1-1-2-2: not change index.
					pDiscPerSector->subch.current.byIndex = pDiscPerSector->subch.prev.byIndex;
				}
			}
			// pattern 2-1-2: current sector is data.
			else if ((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				// 1st sector of tracks
				if (pDiscPerSector->subch.prev.byTrackNum > 0 &&
					nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum]) {
					pDiscPerSector->subch.current.nRelativeTime = 0;
					// pattern 2-1-2-1: change track.
					pDiscPerSector->subch.current.byTrackNum = (BYTE)(pDiscPerSector->subch.prev.byTrackNum + 1);
					// pattern 2-1-2-1-2: not change index.
					pDiscPerSector->subch.current.byIndex = pDiscPerSector->subch.prev.byIndex;
					OutputSubInfoWithLBALog(
						"The 1st sector of Data Track %02d is replaced with EAN sector (in this case it's NOT the last sector of Audio Track %02d)\n"
						, nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subch.current.byTrackNum, pDiscPerSector->subch.prev.byTrackNum);
				}
				// 1st next index of same tracks
				else if (pDiscPerSector->subch.prev.byIndex == 0 && pDiscPerSector->subch.prev.nRelativeTime == 0) {
					pDiscPerSector->subch.current.nRelativeTime = 0;
					// pattern 2-1-2-2: not change track.
					pDiscPerSector->subch.current.byTrackNum = pDiscPerSector->subch.prev.byTrackNum;
					// pattern 2-1-2-2-1: change index.
					pDiscPerSector->subch.current.byIndex = (BYTE)(pDiscPerSector->subch.prev.byIndex + 1);
				}
				// same index of same tracks
				else {
					if (pDiscPerSector->subch.prev.byIndex == 0) {
						pDiscPerSector->subch.current.nRelativeTime = pDiscPerSector->subch.prev.nRelativeTime - 1;
					}
					else if (pDiscPerSector->subch.prev.byIndex > 0) {
						pDiscPerSector->subch.current.nRelativeTime = pDiscPerSector->subch.prev.nRelativeTime + 1;
					}
					// pattern 2-1-2-2: not change track.
					pDiscPerSector->subch.current.byTrackNum = pDiscPerSector->subch.prev.byTrackNum;
					// pattern 2-1-2-2-2: not change index.
					pDiscPerSector->subch.current.byIndex = pDiscPerSector->subch.prev.byIndex;
				}
			}
		}
		// pattern 2-2: prev sector is data.
		else if ((pDiscPerSector->subch.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			// pattern 2-2-1: current sector is audio.
			if ((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == 0) {
				// 1st sector of tracks
				if (pDiscPerSector->subch.prev.byTrackNum > 0 &&
					nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum]) {
					pDiscPerSector->subch.current.nRelativeTime = 0;
					// pattern 2-2-1-1: change track.
					pDiscPerSector->subch.current.byTrackNum = (BYTE)(pDiscPerSector->subch.prev.byTrackNum + 1);
					// pattern 2-2-1-1-2: not change index.
					pDiscPerSector->subch.current.byIndex = pDiscPerSector->subch.prev.byIndex;
					OutputSubInfoWithLBALog(
						"The 1st sector of Audio Track %02d is replaced with EAN sector (in this case it's NOT the last sector of Data Track %02d)\n"
						, nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subch.current.byTrackNum, pDiscPerSector->subch.prev.byTrackNum);
				}
				// 1st next index of same tracks
				else if (pDiscPerSector->subch.prev.byIndex == 0 && pDiscPerSector->subch.prev.nRelativeTime == 0) {
					pDiscPerSector->subch.current.nRelativeTime = 0;
					// pattern 2-2-1-2: not change track.
					pDiscPerSector->subch.current.byTrackNum = pDiscPerSector->subch.prev.byTrackNum;
					// pattern 2-2-1-2-1: change index.
					pDiscPerSector->subch.current.byIndex = (BYTE)(pDiscPerSector->subch.prev.byIndex + 1);
				}
				// same index of same tracks
				else {
					if (pDiscPerSector->subch.prev.byIndex == 0) {
						pDiscPerSector->subch.current.nRelativeTime = pDiscPerSector->subch.prev.nRelativeTime - 1;
					}
					else if (pDiscPerSector->subch.prev.byIndex > 0) {
						// EVE - burst error (Disc 3) (Terror Disc)
						// LBA[188021, 0x2DE75],  Data,      Copy NG,                  Track[01], Idx[01], RMSF[41:46:71], AMSF[41:48:71], RtoW[0, 0, 0, 0]
						// LBA[188022, 0x2DE76], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :72], RtoW[0, 0, 0, 0]
						// LBA[188023, 0x2DE77],  Data,      Copy NG,                  Track[01], Idx[01], RMSF[41:46:73], AMSF[41:48:73], RtoW[0, 0, 0, 0]
						pDiscPerSector->subch.current.nRelativeTime = pDiscPerSector->subch.prev.nRelativeTime + 1;
					}
					// pattern 2-2-1-2: not change track.
					pDiscPerSector->subch.current.byTrackNum = pDiscPerSector->subch.prev.byTrackNum;
					// pattern 2-2-1-2-2: not change index.
					pDiscPerSector->subch.current.byIndex = pDiscPerSector->subch.prev.byIndex;
				}
			}
			// pattern 2-2-2: current sector is data.
			else if ((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				// 1st sector of tracks
				if (pDiscPerSector->subch.prev.byTrackNum > 0 &&
					nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum]) {
					pDiscPerSector->subch.current.nRelativeTime = 0;
					// pattern 2-2-2-1: change track.
					pDiscPerSector->subch.current.byTrackNum = (BYTE)(pDiscPerSector->subch.prev.byTrackNum + 1);
					// pattern 2-2-2-1-2: not change index.
					pDiscPerSector->subch.current.byIndex = pDiscPerSector->subch.prev.byIndex;
					OutputSubInfoWithLBALog(
						"The 1st sector of Data Track %02d is replaced with EAN sector (in this case it's NOT the last sector of Data Track %02d)\n"
						, nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subch.current.byTrackNum, pDiscPerSector->subch.prev.byTrackNum);
				}
				// 1st next index of same tracks
				else if (pDiscPerSector->subch.prev.byIndex == 0 && pDiscPerSector->subch.prev.nRelativeTime == 0) {
					// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
					// LBA[142873, 0x22E19], Data, Copy NG, Track[37], Idx[00], RMSF[00:00:00], AMSF[31:46:73], RtoW[0, 0, 0, 0]
					// LBA[142874, 0x22E1A], Data, Copy NG, MediaCatalogNumber [0000000000000], AMSF[     :74], RtoW[0, 0, 0, 0]
					// LBA[142875, 0x22E1B], Data, Copy NG, Track[37], Idx[01], RMSF[00:00:01], AMSF[31:47:00], RtoW[0, 0, 0, 0]
					pDiscPerSector->subch.current.nRelativeTime = 0;
					// pattern 2-2-2-2: not change track.
					pDiscPerSector->subch.current.byTrackNum = pDiscPerSector->subch.prev.byTrackNum;
					// pattern 2-2-2-2-1: change index.
					pDiscPerSector->subch.current.byIndex = (BYTE)(pDiscPerSector->subch.prev.byIndex + 1);
				}
				// same index of same tracks
				else {
					if (pDiscPerSector->subch.prev.byIndex == 0) {
						pDiscPerSector->subch.current.nRelativeTime = pDiscPerSector->subch.prev.nRelativeTime - 1;
					}
					else if (pDiscPerSector->subch.prev.byIndex > 0) {
						// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
						// LBA[174261, 0x2A8B5], Data, Copy NG, Track[60], Idx[01], RMSF[00:06:19], AMSF[38:45:36], RtoW[0, 0, 0, 0]
						// LBA[174262, 0x2A8B6], Data, Copy NG, MediaCatalogNumber [0000000000000], AMSF[     :37], RtoW[0, 0, 0, 0]
						// LBA[174263, 0x2A8B7], Data, Copy NG, Track[60], Idx[01], RMSF[00:06:21], AMSF[38:45:38], RtoW[0, 0, 0, 0]
						pDiscPerSector->subch.current.nRelativeTime = pDiscPerSector->subch.prev.nRelativeTime + 1;
					}
					// pattern 2-2-2-2: not change track.
					pDiscPerSector->subch.current.byTrackNum = pDiscPerSector->subch.prev.byTrackNum;
					// pattern 2-2-2-2-2: not change index.
					pDiscPerSector->subch.current.byIndex = pDiscPerSector->subch.prev.byIndex;
				}
			}
		}
	}
}

VOID UpdateTmpSubchForISRC(
	PSUBCH pSubch
) {
	pSubch->current.byTrackNum = pSubch->prev.byTrackNum;
	pSubch->current.byIndex = pSubch->prev.byIndex;
	if (pSubch->current.byIndex == 0) {
		pSubch->current.nRelativeTime = pSubch->prev.nRelativeTime - 1;
	}
	else if (pSubch->current.byIndex > 0) {
		pSubch->current.nRelativeTime = pSubch->prev.nRelativeTime + 1;
	}
}

VOID UpdateTmpSubchForAdr6(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA
) {
	if (IsValidPregapSector(pDisc, &pDiscPerSector->subch, nLBA)) {
		pDiscPerSector->subch.current.byTrackNum = (BYTE)(pDiscPerSector->subch.prev.byTrackNum + 1);
		pDiscPerSector->subch.current.byIndex = 0;
		if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] - 225) {
			pDiscPerSector->subch.current.nRelativeTime = 224;
		}
		else if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] - 150) {
			pDiscPerSector->subch.current.nRelativeTime = 149;
		}
		else if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] - 149) {
			pDiscPerSector->subch.current.nRelativeTime = 148;
		}
	}
	else if (pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->byTrackNum] == nLBA) {
		pDiscPerSector->subch.current.byTrackNum = (BYTE)(pDiscPerSector->subch.prev.byTrackNum + 1);
		pDiscPerSector->subch.current.byIndex = 1;
		pDiscPerSector->subch.current.nRelativeTime = pDiscPerSector->subch.prev.nRelativeTime;
	}
}

VOID UpdateTmpSubch(
	PDISC_PER_SECTOR pDiscPerSector
) {
	pDiscPerSector->subch.prevPrev.byP = pDiscPerSector->subch.prev.byP;
	pDiscPerSector->subch.prevPrev.byCtl = pDiscPerSector->subch.prev.byCtl;

	if (pDiscPerSector->subch.prev.byAdr != ADR_ENCODES_MEDIA_CATALOG &&
		pDiscPerSector->subch.prev.byAdr != ADR_ENCODES_ISRC) {
		pDiscPerSector->subch.prevPrev.byAdr = pDiscPerSector->subch.prev.byAdr;
		pDiscPerSector->subch.prevPrev.nRelativeTime = pDiscPerSector->subch.prev.nRelativeTime;
	}
	pDiscPerSector->subch.prevPrev.byTrackNum = pDiscPerSector->subch.prev.byTrackNum;
	pDiscPerSector->subch.prevPrev.byIndex = pDiscPerSector->subch.prev.byIndex;
	pDiscPerSector->subch.prevPrev.nAbsoluteTime = pDiscPerSector->subch.prev.nAbsoluteTime;

	pDiscPerSector->subch.prev.byP = pDiscPerSector->subch.current.byP;
	pDiscPerSector->subch.prev.byCtl = pDiscPerSector->subch.current.byCtl;
	pDiscPerSector->subch.prev.byAdr = pDiscPerSector->subch.current.byAdr;
	pDiscPerSector->subch.prev.byTrackNum = pDiscPerSector->subch.current.byTrackNum;
	pDiscPerSector->subch.prev.byIndex = pDiscPerSector->subch.current.byIndex;

	if (pDiscPerSector->bLibCrypt || pDiscPerSector->bSecuRom) {
		pDiscPerSector->subch.prev.nRelativeTime++;
	}
	else {
		pDiscPerSector->subch.prev.nRelativeTime = pDiscPerSector->subch.current.nRelativeTime;
	}
	pDiscPerSector->subch.prev.nAbsoluteTime++;
}

VOID UpdateTmpMainHeader(
	PDISC_PER_SECTOR pDiscPerSector,
	INT nMainDataType
) {
	if (IsValidMainDataHeader(pDiscPerSector->mainHeader.current)) {
		memcpy(pDiscPerSector->mainHeader.prev, pDiscPerSector->mainHeader.current, MAINHEADER_MODE1_SIZE);
		BYTE tmp = (BYTE)(pDiscPerSector->mainHeader.current[14] + 1);
		if ((tmp & 0x0f) == 0x0a) {
			tmp = (BYTE)(tmp + 6);
		}
		if (tmp == 0x75) {
			pDiscPerSector->mainHeader.current[14] = 0;
			if (nMainDataType == scrambled) {
				tmp = (BYTE)((pDiscPerSector->mainHeader.current[13] ^ 0x80) + 1);
			}
			else {
				tmp = (BYTE)(pDiscPerSector->mainHeader.current[13] + 1);
			}
			if ((tmp & 0x0f) == 0x0a) {
				tmp = (BYTE)(tmp + 6);
			}
			if (tmp == 0x60) {
				if (nMainDataType == scrambled) {
					pDiscPerSector->mainHeader.current[13] = 0x80;
					tmp = (BYTE)((pDiscPerSector->mainHeader.current[12] ^ 0x01) + 1);
				}
				else {
					pDiscPerSector->mainHeader.current[13] = 0;
					tmp = (BYTE)(pDiscPerSector->mainHeader.current[12] + 1);
				}
				if ((tmp & 0x0f) == 0x0a) {
					tmp = (BYTE)(tmp + 6);
				}
				if (nMainDataType == scrambled) {
					pDiscPerSector->mainHeader.current[12] = (BYTE)(tmp ^ 0x01);
				}
				else {
					pDiscPerSector->mainHeader.current[12] = tmp;
				}
			}
			else {
				if (nMainDataType == scrambled) {
					pDiscPerSector->mainHeader.current[13] = (BYTE)(tmp ^ 0x80);
				}
				else {
					pDiscPerSector->mainHeader.current[13] = tmp;
				}
			}
		}
		else {
			pDiscPerSector->mainHeader.current[14] = tmp;
		}
		pDiscPerSector->mainHeader.current[15] = GetMode(pDiscPerSector, nMainDataType);
	}
}
