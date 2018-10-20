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

VOID SetCommandForTransferLength(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	LPBYTE pCdb,
	DWORD dwSize,
	LPBYTE lpTransferLen,
	LPBYTE lpRoopLen
) {
	if (*pExecType == gd) {
		if (dwSize == pDevice->dwMaxTransferLength) {
			*lpTransferLen = (BYTE)(dwSize / CD_RAW_SECTOR_SIZE);
		}
		else {
			*lpTransferLen = (BYTE)(dwSize / DISC_RAW_READ_SIZE + 1);
		}
	}
	else {
		*lpTransferLen = (BYTE)(dwSize / DISC_RAW_READ_SIZE);
	}
	// Generally, directory size is per 2048 byte
	// Exception:
	//  Codename - Outbreak (Europe) (Sold Out Software)
	//  Commandos - Behind Enemy Lines (Europe) (Sold Out Software)
	// and more
	if (dwSize % DISC_RAW_READ_SIZE != 0) {
		(*lpTransferLen)++;
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
	PDEVICE pDevice,
	DRIVE_DATA_ORDER order
) {
	if (order == DRIVE_DATA_ORDER::NoC2) {
		pDevice->TRANSFER.dwBufLen = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
		pDevice->TRANSFER.dwBufC2Offset = 0;
		pDevice->TRANSFER.dwBufSubOffset = CD_RAW_SECTOR_SIZE;
	}
	else if (order == DRIVE_DATA_ORDER::MainC2Sub) {
		pDevice->TRANSFER.dwBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
		pDevice->TRANSFER.dwBufC2Offset = CD_RAW_SECTOR_SIZE;
		pDevice->TRANSFER.dwBufSubOffset = CD_RAW_SECTOR_WITH_C2_294_SIZE;
	}
	else if (order == DRIVE_DATA_ORDER::MainSubC2) {
		pDevice->TRANSFER.dwBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
		pDevice->TRANSFER.dwBufSubOffset = CD_RAW_SECTOR_SIZE;
		pDevice->TRANSFER.dwBufC2Offset = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
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
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(TOC));
	CONST INT typeSize = 7;
	CHAR strType[typeSize] = {};
	BOOL bFirstData = TRUE;
	TRACK_TYPE trkType = TRACK_TYPE::audioOnly;
	// for swap command
	pDisc->SCSI.nAllLength = 0;

	for (BYTE i = pDisc->SCSI.toc.FirstTrack; i <= pDisc->SCSI.toc.LastTrack; i++) {
		INT tIdx = i - 1;
		// for swap command
		pDisc->SCSI.lpFirstLBAListOnToc[tIdx] = 0;
		pDisc->SCSI.lpLastLBAListOnToc[tIdx] = 0;

		for (INT j = 0, k = 24; j < 4; j++, k -= 8) {
			pDisc->SCSI.lpFirstLBAListOnToc[tIdx] |= pDisc->SCSI.toc.TrackData[tIdx].Address[j] << k;
			pDisc->SCSI.lpLastLBAListOnToc[tIdx] |= pDisc->SCSI.toc.TrackData[i].Address[j] << k;
		}
		pDisc->SCSI.lpLastLBAListOnToc[tIdx] -= 1;
		pDisc->SCSI.nAllLength += 
			pDisc->SCSI.lpLastLBAListOnToc[tIdx] - pDisc->SCSI.lpFirstLBAListOnToc[tIdx] + 1;

		if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == 0) {
			strncpy(strType, " Audio", typeSize);
		}
		else if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			strncpy(strType, "  Data", typeSize);
			if (bFirstData) {
				pDisc->SCSI.nFirstLBAofDataTrack = pDisc->SCSI.lpFirstLBAListOnToc[tIdx];
				pDisc->SCSI.byFirstDataTrackNum = i;
				bFirstData = FALSE;
				trkType = TRACK_TYPE::dataExist;
			}
			pDisc->SCSI.nLastLBAofDataTrack = pDisc->SCSI.lpLastLBAListOnToc[tIdx];
			pDisc->SCSI.byLastDataTrackNum = i;
		}
		if (i == pDisc->SCSI.toc.FirstTrack && pDisc->SCSI.lpFirstLBAListOnToc[tIdx] > 0) {
			pDisc->SCSI.nAllLength += pDisc->SCSI.lpFirstLBAListOnToc[tIdx];
			OutputDiscLogA("\tPregap Track   , LBA %8u - %8u, Length %8u\n",
				0, pDisc->SCSI.lpFirstLBAListOnToc[tIdx] - 1, pDisc->SCSI.lpFirstLBAListOnToc[tIdx]);
			// This flag exists for The Apprentice (CD-i), Dimo's Quest (CD-i), that is, CD-I ready disc.
			// These discs have only audio track on TOC, but there are
			// actually some data sector in pregap of first sector.
//			pDisc->SCSI.byFirstDataTrackNum = i;
//			pDisc->SCSI.byLastDataTrackNum = i;
			trkType = TRACK_TYPE::pregapIn1stTrack;
		}
		OutputDiscLogA(
			"\t%s Track %2u, LBA %8u - %8u, Length %8u\n", strType, i,
			pDisc->SCSI.lpFirstLBAListOnToc[tIdx], pDisc->SCSI.lpLastLBAListOnToc[tIdx],
			pDisc->SCSI.lpLastLBAListOnToc[tIdx] - pDisc->SCSI.lpFirstLBAListOnToc[tIdx] + 1);
	}
	OutputDiscLogA(
		"\t                                          Total  %8u\n", pDisc->SCSI.nAllLength);
	pDisc->SCSI.trackType = trkType;
}

VOID SetAndOutputTocForGD(
	PDISC pDisc,
	LPBYTE bufDec
) {
	// update the toc of audio trap disc to the toc of gd-rom
	pDisc->SCSI.toc.FirstTrack = bufDec[0x29a];
	pDisc->SCSI.toc.LastTrack = bufDec[0x29e];
	pDisc->SCSI.nAllLength = MAKELONG(
		MAKEWORD(bufDec[0x2a0], bufDec[0x2a1]), MAKEWORD(bufDec[0x2a2], 0)) - 150;

	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(TOC For GD(HD Area)));
	CONST INT typeSize = 7;
	CHAR strType[typeSize] = {};
	BOOL bFirstData = TRUE;
	TRACK_TYPE trkType = TRACK_TYPE::audioOnly;

	for (BYTE i = pDisc->SCSI.toc.FirstTrack, j = 0; i <= pDisc->SCSI.toc.LastTrack; i++, j = (BYTE)(j + 4)) {
		INT tIdx = i - 1;
		// update the toc of audio trap disc to the toc of gd-rom
		pDisc->SCSI.lpFirstLBAListOnToc[tIdx] = MAKELONG(
			MAKEWORD(bufDec[0x114 + j], bufDec[0x115 + j]), MAKEWORD(bufDec[0x116 + j], 0)) - 150;
		if (i == pDisc->SCSI.toc.LastTrack) {
			pDisc->SCSI.lpLastLBAListOnToc[tIdx] = pDisc->SCSI.nAllLength - 1;
		}
		else {
			pDisc->SCSI.lpLastLBAListOnToc[tIdx] = MAKELONG(
				MAKEWORD(bufDec[0x118 + j], bufDec[0x119 + j]), MAKEWORD(bufDec[0x11a + j], 0)) - 150 - 1;
		}
		pDisc->SCSI.toc.TrackData[tIdx].Adr = BYTE((bufDec[0x117 + j]) & 0x0f);
		pDisc->SCSI.toc.TrackData[tIdx].Control = BYTE((bufDec[0x117 + j] >> 4) & 0x0f);
		pDisc->SCSI.toc.TrackData[tIdx].TrackNumber = i;

		if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == 0) {
			strncpy(strType, " Audio", typeSize);
		}
		else if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			strncpy(strType, "  Data", typeSize);
			if (bFirstData) {
				pDisc->SCSI.nFirstLBAofDataTrack = pDisc->SCSI.lpFirstLBAListOnToc[tIdx];
				pDisc->SCSI.byFirstDataTrackNum = i;
				bFirstData = FALSE;
				trkType = TRACK_TYPE::dataExist;
			}
			pDisc->SCSI.nLastLBAofDataTrack = pDisc->SCSI.lpLastLBAListOnToc[tIdx];
			pDisc->SCSI.byLastDataTrackNum = i;
		}
		OutputDiscLogA(
			"\t%s Track %2u, LBA %6u - %6u, Length %6u\n", strType, i,
			pDisc->SCSI.lpFirstLBAListOnToc[tIdx], pDisc->SCSI.lpLastLBAListOnToc[tIdx],
			pDisc->SCSI.lpLastLBAListOnToc[tIdx] - pDisc->SCSI.lpFirstLBAListOnToc[tIdx] + 1);
	}
	OutputDiscLogA("                                           Total %6d\n"
		, pDisc->SCSI.nAllLength - FIRST_LBA_FOR_GD);
	pDisc->SCSI.trackType = trkType;
}

VOID SetAndOutputTocFull(
	PDISC pDisc,
	PCDROM_TOC_FULL_TOC_DATA fullToc,
	PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData,
	WORD wTocEntries,
	FILE* fpCcd
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(FULL TOC)
		"\tFirstCompleteSession: %u\n"
		"\t LastCompleteSession: %u\n",
		fullToc->FirstCompleteSession,
		fullToc->LastCompleteSession);
	BOOL bFirst2ndSession = TRUE;
	UCHAR ucTmpLastTrack = 0;

	for (WORD a = 0; a < wTocEntries; a++) {
		INT nTmpLBAExt = 0;
		INT nTmpLBA = 0;
		if (fpCcd) {
			WriteCcdForEntry(pTocData, a, fpCcd);
		}
		OutputDiscLogA("\tSession %u, Ctl %u, Adr %u, Point 0x%02x,"
			, pTocData[a].SessionNumber, pTocData[a].Control, pTocData[a].Adr, pTocData[a].Point);
		switch (pTocData[a].Point) {
		case 0xa0:
			OutputDiscLogA(" FirstTrack %2u, ", pTocData[a].Msf[0]);
			switch (pTocData[a].Msf[1]) {
			case DISK_TYPE_CDDA:
				OutputDiscLogA("Format: CD-DA or CD-ROM\n");
				break;
			case DISK_TYPE_CDI:
				OutputDiscLogA("Format: CD-I\n");
				break;
			case DISK_TYPE_XA:
				OutputDiscLogA("Format: CD-ROM-XA\n");
				break;
			default:
				OutputDiscLogA("Format: Other\n");
				break;
			}
			pDisc->SCSI.byFormat = pTocData[a].Msf[1];
			break;
		case 0xa1:
			OutputDiscLogA("  LastTrack %2u\n", pTocData[a].Msf[0]);
			ucTmpLastTrack = pTocData[a].Msf[0];
			break;
		case 0xa2:
			nTmpLBA = 
				MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			OutputDiscLogA(
				"      Lead-out, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
				, pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2], nTmpLBA, nTmpLBA);
			if (fullToc->LastCompleteSession > 1) {
				// Rayman (USA) [SS], Wolfchild (Europe) [MCD]
				// Last LBA is corrupt, so this doesn't use in single session disc.
				pDisc->SCSI.lpLastLBAListOnToc[ucTmpLastTrack - 1] = nTmpLBA - 150 - 1;
			}
			if (pTocData[a].SessionNumber == 1) {
				pDisc->SCSI.nFirstLBAofLeadout = nTmpLBA - 150;
			}
			break;
		case 0xb0: // (multi-session disc)
			nTmpLBAExt =
				MSFtoLBA(pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2]);
			nTmpLBA =
				MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			OutputDiscLogA(
				"   NextSession, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
				"\t                    Outermost Lead-out of the disc, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n" 
				"\t                         Num of pointers in Mode 5, %02u\n", 
				pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2], 
				nTmpLBAExt, nTmpLBAExt,
				pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2],
				nTmpLBA, nTmpLBA, pTocData[a].Zero);
			break;
		case 0xb1: // (Audio only: This identifies the presence of skip intervals)
			OutputDiscLogA(
				"\t                     Num of skip interval pointers, %02u\n"
				"\t                        Num of skip track pointers, %02u\n",
				pTocData[a].Msf[0], pTocData[a].Msf[1]);
			break;
		case 0xb2: // (Audio only: This identifies tracks that should be skipped during playback)
		case 0xb3:
		case 0xb4:
			OutputDiscLogA(
				"                         Skip num, %02u %02u %02u %02u %02u %02u\n"
				, pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2],
				pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			break;
		case 0xc0: // (Together with POINT=B0h, this is used to identify a multi-session disc)
			nTmpLBAExt =
				MSFtoLBA(pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2]);
			nTmpLBA =
				MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			OutputDiscLogA(
				" Optimum recording power, %02u\n"
				"\t                         First Lead-in of the disc, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n", 
				pTocData[a].MsfExtra[0], 
				pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2],
				nTmpLBA, nTmpLBA);
			break;
		case 0xc1:
			OutputDiscLogA(
				"  Copy of info from A1 point in ATIP, %02u %02u %02u %02u %02u %02u\n"
				, pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2],
				pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			break;
		default:
			if (pTocData[a].Adr == 1) {
				nTmpLBA =
					MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
				OutputDiscLogA(
					"      Track %2u, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
					, pTocData[a].Point, pTocData[a].Msf[0],
					pTocData[a].Msf[1], pTocData[a].Msf[2], nTmpLBA, nTmpLBA);
				if (fullToc->LastCompleteSession > 1) {
#if 0
					// Rayman (USA) [SS], Wolfchild (Europe) [MCD]
					// Last LBA is corrupt, so this doesn't use in single session disc.
					if (pTocData[a].Point == 1) {
						pDisc->SCSI.lpFirstLBAListOnToc[pTocData[a].Point - 1] = nTmpLBA - 150;
					}
					else if (pTocData[a].Point >= 2 && pTocData[a].Point <= 100) {
						pDisc->SCSI.lpLastLBAListOnToc[pTocData[a].Point - 2] = nTmpLBA - 150 - 1;
						pDisc->SCSI.lpFirstLBAListOnToc[pTocData[a].Point - 1] = nTmpLBA - 150;
					}
#endif
					if (pTocData[a].SessionNumber == 2 && bFirst2ndSession) {
						pDisc->SCSI.nFirstLBAof2ndSession = nTmpLBA - 150;
						bFirst2ndSession = FALSE;
					}
				}
				pDisc->SCSI.lpSessionNumList[pTocData[a].Point - 1] = pTocData[a].SessionNumber;
			}
			else if (pTocData[a].Adr == 5) {
				nTmpLBAExt =
					MSFtoLBA(pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2]);
				nTmpLBA =
					MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
				OutputDiscLogA(
					"\t              Skipped interval end time, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
					"\t              Skipped interval start time on playback, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n",
					pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2],
					nTmpLBAExt, nTmpLBAExt,
					pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2],
					nTmpLBA, nTmpLBA);
			}
			break;
		}
	}
}

VOID SetAndOutputTocCDText(
	PDISC pDisc,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	LPCH pTmpText,
	WORD wTocTextEntries,
	WORD wAllTextSize
) {
	UINT uiAlbumCnt = 0/*, byAlbumIdx = 0*/;
	UINT uiPerformerCnt = 0/*, byPerformerIdx = 0*/;
	UINT uiSongwriterCnt = 0/*, bySongwriterIdx = 0*/;
	UINT uiComposerCnt = 0/*, byComposerIdx = 0*/;
	UINT uiArrangerCnt = 0/*, byArrangerIdx = 0*/;
	UINT uiMessagesCnt = 0/*, byMessagesIdx = 0*/;
	UINT uiDiscIdCnt = 0/*, byDiscIdIdx = 0*/;
	UINT uiGenreCnt = 0/*, byGenreIdx = 0*/;
	UINT uiTocInfoCnt = 0/*, byTocInfoIdx = 0*/;
	UINT uiTocInfo2Cnt = 0/*, byTocInfo2Idx = 0*/;
	UINT uiUpcEanCnt = 0/*, byUpcEanIdx = 0*/;
	UINT uiSizeInfoCnt = 0; BYTE bySizeInfoIdx = 0;

	for (size_t t = 0; t < wTocTextEntries; t++) {
		BYTE bRet = 0;
		BYTE bCnt = 0;
		for (INT k = 0; k < 12; k++) {
			if (pDesc[t].Text[k] == 0) {
				bRet++;
				if (k < 11 && pDesc[t].Text[k+1] == 0) {
					bRet--;
					bCnt++;
				}
				if (k == 11 && bCnt == 11 && pDesc[t].CharacterPosition == 0) {
					bRet--;
				}
			}
		}
		if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_ALBUM_NAME) {
			if (bRet) {
				uiAlbumCnt += bRet;
			}
//			byAlbumIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_PERFORMER) {
			if (bRet) {
				uiPerformerCnt += bRet;
			}
//			byPerformerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_SONGWRITER) {
			if (bRet) {
				uiSongwriterCnt += bRet;
			}
//			bySongwriterIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_COMPOSER) {
			if (bRet) {
				uiComposerCnt += bRet;
			}
//			byComposerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_ARRANGER) {
			if (bRet) {
				uiArrangerCnt += bRet;
			}
//			byArrangerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_MESSAGES) {
			if (bRet) {
				uiMessagesCnt += bRet;
			}
//			byMessagesIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_DISC_ID) {
			if (bRet) {
				uiDiscIdCnt += bRet;
			}
//			byDiscIdIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_GENRE) {
			if (bRet) {
				uiGenreCnt += bRet;
			}
//			byGenreIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_TOC_INFO) {
			uiTocInfoCnt++;
//			byTocInfoIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_TOC_INFO2) {
			uiTocInfo2Cnt++;
//			byTocInfo2Idx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_UPC_EAN) {
			if (bRet) {
				uiUpcEanCnt += bRet;
			}
//			byUpcEanIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_SIZE_INFO) {
			uiSizeInfoCnt++;
			bySizeInfoIdx = pDesc[t].SequenceNumber;
		}
		if (pDesc[t].PackType != CDROM_CD_TEXT_PACK_TOC_INFO &&
			pDesc[t].PackType != CDROM_CD_TEXT_PACK_TOC_INFO2 &&
			pDesc[t].PackType != CDROM_CD_TEXT_PACK_SIZE_INFO) {
			memcpy(pTmpText + 12 * t, (pDesc[t].Text), 12);
		}
	}
	size_t uiIdx = 0;
	INT nTitleCnt = 0;
	INT nPerformerCnt = 0;
	INT nSongwriterCnt = 0;
	for (size_t z = 0; z < wTocTextEntries; z++) {
		if (uiIdx == wAllTextSize) {
			break;
		}
		size_t len1 = strlen(pTmpText + uiIdx);
		if (len1 == 0 || len1 >= META_CDTEXT_SIZE) {
			z--;
		}
		else {
			CHAR tmp[META_CDTEXT_SIZE] = {};
			strncpy(tmp, pTmpText + uiIdx, len1);

			if (uiAlbumCnt != 0 && z < uiAlbumCnt) {
				strncpy(pDisc->SCSI.pszTitle[nTitleCnt], tmp, strlen(tmp));
				if (nTitleCnt == 0) {
					OutputDiscLogA("\tAlbum Name: %s\n", pDisc->SCSI.pszTitle[nTitleCnt]);
				}
				else {
					OutputDiscLogA("\t Song Name: %s\n", pDisc->SCSI.pszTitle[nTitleCnt]);
				}
				nTitleCnt++;
			}
			else if (uiPerformerCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt)) {
				strncpy(pDisc->SCSI.pszPerformer[nPerformerCnt], tmp, strlen(tmp));
				if (nPerformerCnt == 0) {
					OutputDiscLogA("\tAlbum Performer: %s\n", pDisc->SCSI.pszPerformer[nPerformerCnt]);
				}
				else {
					OutputDiscLogA("\t Song Performer: %s\n", pDisc->SCSI.pszPerformer[nPerformerCnt]);
				}
				nPerformerCnt++;
			}
			else if (uiSongwriterCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt + uiSongwriterCnt)) {
				strncpy(pDisc->SCSI.pszSongWriter[nSongwriterCnt], tmp, strlen(tmp));
				if (nSongwriterCnt == 0) {
					OutputDiscLogA("\tAlbum SongWriter: %s\n", pDisc->SCSI.pszSongWriter[nSongwriterCnt]);
				}
				else {
					OutputDiscLogA("\t      SongWriter: %s\n", pDisc->SCSI.pszSongWriter[nSongwriterCnt]);
				}
				nSongwriterCnt++;
			}
			else if (uiComposerCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt + uiSongwriterCnt + uiComposerCnt)) {
				OutputDiscLogA("\tComposer: %s\n", tmp);
			}
			else if (uiArrangerCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt + uiSongwriterCnt + uiComposerCnt + uiArrangerCnt)) {
				OutputDiscLogA("\tArranger: %s\n", tmp);
			}
			else if (uiMessagesCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt + uiSongwriterCnt +
				uiComposerCnt + uiArrangerCnt + uiMessagesCnt)) {
				OutputDiscLogA("\tMessages: %s\n", tmp);
			}
			else if (uiDiscIdCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt + uiSongwriterCnt +
				uiComposerCnt + uiArrangerCnt + uiMessagesCnt + uiDiscIdCnt)) {
				OutputDiscLogA("\tDiscId: %s\n", tmp);
			}
			else if (uiGenreCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt + uiSongwriterCnt +
				uiComposerCnt + uiArrangerCnt + uiMessagesCnt + uiDiscIdCnt + uiGenreCnt)) {
				OutputDiscLogA("\tGenre: %s\n", tmp);
			}
			else if (uiUpcEanCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt + uiSongwriterCnt +
				uiComposerCnt + uiArrangerCnt + uiMessagesCnt + uiDiscIdCnt +
				uiGenreCnt + uiTocInfoCnt + uiTocInfo2Cnt + uiUpcEanCnt)) {
				OutputDiscLogA("\tUpcEan: %s\n", tmp);
			}
		}
		uiIdx += len1 + 1;
	}
	OutputCDTextOther(pDesc, wTocTextEntries, bySizeInfoIdx, uiSizeInfoCnt);
}

VOID SetAndOutputTocCDWText(
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	LPCH pTmpText,
	WORD wFirstEntries,
	WORD wTocTextEntries,
	WORD wAllTextSize
) {
	BYTE uiAlbumCnt = 0/*, byAlbumIdx = 0*/;
	BYTE uiPerformerCnt = 0/*, byPerformerIdx = 0*/;
	BYTE uiSongwriterCnt = 0/*, bySongwriterIdx = 0*/;
	BYTE uiComposerCnt = 0/*, byComposerIdx = 0*/;
	BYTE uiArrangerCnt = 0/*, byArrangerIdx = 0*/;
	BYTE uiMessagesCnt = 0/*, byMessagesIdx = 0*/;
	BYTE uiDiscIdCnt = 0/*, byDiscIdIdx = 0*/;
	BYTE uiGenreCnt = 0/*, byGenreIdx = 0*/;
	BYTE uiTocInfoCnt = 0/*, byTocInfoIdx = 0*/;
	BYTE uiTocInfo2Cnt = 0/*, byTocInfo2Idx = 0*/;
	BYTE uiUpcEanCnt = 0/*, byUpcEanIdx = 0*/;
	BYTE uiSizeInfoCnt = 0, bySizeInfoIdx = 0;

	for (size_t t = wFirstEntries; t < wTocTextEntries; t++) {
		BOOL bRet = FALSE;
		for (INT k = 0; k < 6; k++) {
			if (pDesc[t].WText[k] == 0) {
				bRet++;
				if (k < 5 && pDesc[t].WText[k + 1] == 0) {
					bRet--;
				}
			}
		}
		if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_ALBUM_NAME) {
			if (bRet) {
				uiAlbumCnt++;
			}
//			byAlbumIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_PERFORMER) {
			if (bRet) {
				uiPerformerCnt++;
			}
//			byPerformerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_SONGWRITER) {
			if (bRet) {
				uiSongwriterCnt++;
			}
//			bySongwriterIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_COMPOSER) {
			if (bRet) {
				uiComposerCnt++;
			}
//			byComposerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_ARRANGER) {
			if (bRet) {
				uiArrangerCnt++;
			}
//			byArrangerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_MESSAGES) {
			if (bRet) {
				uiMessagesCnt++;
			}
//			byMessagesIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_DISC_ID) {
			if (bRet) {
				uiDiscIdCnt++;
			}
//			byDiscIdIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_GENRE) {
			if (bRet) {
				uiGenreCnt++;
			}
//			byGenreIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_TOC_INFO) {
			uiTocInfoCnt++;
//			byTocInfoIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_TOC_INFO2) {
			uiTocInfo2Cnt++;
//			byTocInfo2Idx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_UPC_EAN) {
			if (bRet) {
				uiUpcEanCnt++;
			}
//			byUpcEanIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_SIZE_INFO) {
			uiSizeInfoCnt++;
			bySizeInfoIdx = pDesc[t].SequenceNumber;
		}
		if (pDesc[t].PackType != CDROM_CD_TEXT_PACK_TOC_INFO &&
			pDesc[t].PackType != CDROM_CD_TEXT_PACK_TOC_INFO2 &&
			pDesc[t].PackType != CDROM_CD_TEXT_PACK_SIZE_INFO) {
			memcpy(pTmpText + 12 * (t - wFirstEntries), (LPWCH)(pDesc[t].Text), 12);
		}
	}
	size_t uiIdx = 0;
	INT nTitleCnt = 0;
	INT nPerformerCnt = 0;
	INT nSongwriterCnt = 0;
	for (size_t z = 0; z < wTocTextEntries; z++) {
		if (uiIdx == wAllTextSize) {
			break;
		}
		size_t len1 = strlen(pTmpText + uiIdx);
		if (len1 == 0 || len1 >= META_CDTEXT_SIZE) {
			z--;
		}
		else {
			CHAR tmp[META_CDTEXT_SIZE] = {};
			strncpy(tmp, pTmpText + uiIdx, len1);

			if (uiAlbumCnt != 0 && z < uiAlbumCnt) {
				if (nTitleCnt == 0) {
					OutputDiscLogA("\tAlbum Name: %s\n", tmp);
				}
				else {
					OutputDiscLogA("\t Song Name: %s\n", tmp);
				}
				nTitleCnt++;
			}
			else if (uiPerformerCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt)) {
				if (nPerformerCnt == 0) {
					OutputDiscLogA("\tAlbum Performer: %s\n", tmp);
				}
				else {
					OutputDiscLogA("\t Song Performer: %s\n", tmp);
				}
				nPerformerCnt++;
			}
			else if (uiSongwriterCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt + uiSongwriterCnt)) {
				if (nSongwriterCnt == 0) {
					OutputDiscLogA("\tAlbum SongWriter: %s\n", tmp);
				}
				else {
					OutputDiscLogA("\t      SongWriter: %s\n", tmp);
				}
				nSongwriterCnt++;
			}
			else if (uiComposerCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt + uiSongwriterCnt + uiComposerCnt)) {
				OutputDiscLogA("\tComposer: %s\n", tmp);
			}
			else if (uiArrangerCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt + uiSongwriterCnt + uiComposerCnt + uiArrangerCnt)) {
				OutputDiscLogA("\tArranger: %s\n", tmp);
			}
			else if (uiMessagesCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt + uiSongwriterCnt +
				uiComposerCnt + uiArrangerCnt + uiMessagesCnt)) {
				OutputDiscLogA("\tMessages: %s\n", tmp);
			}
			else if (uiDiscIdCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt + uiSongwriterCnt +
				uiComposerCnt + uiArrangerCnt + uiMessagesCnt + uiDiscIdCnt)) {
				OutputDiscLogA("\tDiscId: %s\n", tmp);
			}
			else if (uiGenreCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt + uiSongwriterCnt +
				uiComposerCnt + uiArrangerCnt + uiMessagesCnt + uiDiscIdCnt + uiGenreCnt)) {
				OutputDiscLogA("\tGenre: %s\n", tmp);
			}
			else if (uiUpcEanCnt != 0 &&
				z < (size_t)(uiAlbumCnt + uiPerformerCnt + uiSongwriterCnt +
				uiComposerCnt + uiArrangerCnt + uiMessagesCnt + uiDiscIdCnt +
				uiGenreCnt + uiTocInfoCnt + uiTocInfo2Cnt + uiUpcEanCnt)) {
				OutputDiscLogA("\tUpcEan: %s\n", tmp);
			}
		}
		uiIdx += len1 + 1;
	}
	OutputCDTextOther(pDesc, wTocTextEntries, bySizeInfoIdx, uiSizeInfoCnt);
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
		if (*pExecType != gd && byBe && pDisc->SCSI.trackType != TRACK_TYPE::audioOnly) {
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
		if (pDisc->SCSI.nFirstLBAof2ndSession != -1) {
			pDisc->MAIN.nFixFirstLBAofLeadout =
				pDisc->SCSI.nFirstLBAofLeadout + pDisc->MAIN.nAdjustSectorNum;
			pDisc->MAIN.nFixFirstLBAof2ndSession = 
				pDisc->SCSI.nFirstLBAof2ndSession + pDisc->MAIN.nAdjustSectorNum - 1;
		}
	}
	else if (pDisc->MAIN.nCombinedOffset < 0) {
		if (*pExecType != gd && byBe && pDisc->SCSI.trackType != TRACK_TYPE::audioOnly) {
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
		if (pDisc->SCSI.nFirstLBAof2ndSession != -1) {
			pDisc->MAIN.nFixFirstLBAofLeadout = 
				pDisc->SCSI.nFirstLBAofLeadout + pDisc->MAIN.nAdjustSectorNum + 1;
			pDisc->MAIN.nFixFirstLBAof2ndSession =
				pDisc->SCSI.nFirstLBAof2ndSession + pDisc->MAIN.nAdjustSectorNum;
		}
	}
	else {
		pDisc->MAIN.uiMainDataSlideSize = 0;
		pDisc->MAIN.nOffsetStart = 0;
		pDisc->MAIN.nOffsetEnd = 0;
		pDisc->MAIN.nFixStartLBA = nStartLBA;
		pDisc->MAIN.nFixEndLBA = nEndLBA;
		if (pDisc->SCSI.nFirstLBAof2ndSession != -1) {
			pDisc->MAIN.nFixFirstLBAofLeadout =	pDisc->SCSI.nFirstLBAofLeadout;
			pDisc->MAIN.nFixFirstLBAof2ndSession = pDisc->SCSI.nFirstLBAof2ndSession;
		}
	}
	UNREFERENCED_PARAMETER(byPlxtrDrive);
#if 0
	if (*pExecType == data &&
		((byBe && byPlxtrDrive) || !byPlxtrDrive) &&
		pDisc->SCSI.toc.FirstTrack == pDisc->SCSI.toc.LastTrack &&
		pDisc->SCSI.toc.FirstTrack == pDisc->SCSI.byFirstDataTrackNum) {
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
	BYTE tmpCurrentTrackNum = pDiscPerSector->subQ.current.byTrackNum;
	BYTE tmpCurrentIndex = pDiscPerSector->subQ.current.byIndex;
	if (pDiscPerSector->subQ.current.byAdr != ADR_ENCODES_CURRENT_POSITION) {
		if ((IsValidPregapSector(pDisc, &pDiscPerSector->subQ, nLBA) && pExtArg->byMCN) ||
			nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->byTrackNum - 1]) {
			tmpCurrentTrackNum = pDiscPerSector->subQ.next.byTrackNum;
			tmpCurrentIndex = pDiscPerSector->subQ.next.byIndex;
		}
		else {
			tmpCurrentTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
			tmpCurrentIndex = pDiscPerSector->subQ.prev.byIndex;
		}
	}
	if (0 <= nLBA && nLBA < pDisc->SCSI.nAllLength &&
		0 < tmpCurrentTrackNum && tmpCurrentTrackNum <= pDiscPerSector->byTrackNum + 1) {
		INT tIdx = pDiscPerSector->byTrackNum - 1;
		BYTE tmpPrevTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
		BYTE tmpPrevIndex = pDiscPerSector->subQ.prev.byIndex;
		if (pDiscPerSector->subQ.prev.byAdr != ADR_ENCODES_CURRENT_POSITION && !pExtArg->byMCN) {
			tmpPrevTrackNum = pDiscPerSector->subQ.prevPrev.byTrackNum;
			tmpPrevIndex = pDiscPerSector->subQ.prevPrev.byIndex;
		}
		// preserve the 1st LBA of the changed trackNum
		if (tmpPrevTrackNum + 1 == tmpCurrentTrackNum) {
			pDiscPerSector->byTrackNum = tmpCurrentTrackNum;
			tIdx = pDiscPerSector->byTrackNum - 1;

			if (tmpCurrentIndex > 0) {
				if (pDisc->SCSI.nFirstLBAof2ndSession == -1 &&
					tmpCurrentIndex == 1 &&
					nLBA != pDisc->SCSI.lpFirstLBAListOnToc[tIdx]) {
					OutputSubInfoWithLBALogA(
						"Subchannel & TOC doesn't sync. LBA on TOC[%d, %#x], index[%02u]\n",
						nLBA, tmpCurrentTrackNum, pDisc->SCSI.lpFirstLBAListOnToc[tIdx],
						pDisc->SCSI.lpFirstLBAListOnToc[tIdx], tmpCurrentIndex);

					pDisc->SUB.lpFirstLBAListOnSub[tIdx][1] = pDisc->SCSI.lpFirstLBAListOnToc[tIdx];
					if (pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSub[tIdx][1]) {
						pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] = -1;
					}
					pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][1] = nLBA;
					if (pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][1]) {
						pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] = -1;
					}
					if (*pExecType != gd && *pExecType != swap) {
						pDisc->SUB.byDesync = TRUE;
					}
				}
				else {
					OutputSubInfoWithLBALogA(
						"There isn't the pregap on this track\n", nLBA, tmpCurrentTrackNum);
					pDisc->SUB.lpFirstLBAListOnSub[tIdx][tmpCurrentIndex] = nLBA;
					pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][tmpCurrentIndex] = nLBA;
				}
			}
			// preserve last LBA per data track
			if (tmpPrevTrackNum > 0) {
				if (pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[tmpPrevTrackNum - 1] != -1 &&
					pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tmpPrevTrackNum - 1] == -1 &&
					(pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {

					INT nTmpLastLBA = nLBA - 1;
					if (!pExtArg->byMultiSession &&
						pDisc->SCSI.lpSessionNumList[tmpCurrentTrackNum - 1] > pDisc->SCSI.lpSessionNumList[tmpPrevTrackNum - 1]) {
						nTmpLastLBA -= SESSION_TO_SESSION_SKIP_LBA;
					}
					OutputSubInfoWithLBALogA(
						"Last LBA of this data track [L:%d]\n", nTmpLastLBA, tmpPrevTrackNum, (INT)__LINE__);
					pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tmpPrevTrackNum - 1] = nTmpLastLBA;

					OutputSubInfoWithLBALogA(
						"TrackNum is changed [L:%d]\n", nLBA, tmpCurrentTrackNum, (INT)__LINE__);
				}
				else if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == 0) {
					OutputSubInfoWithLBALogA(
						"TrackNum is changed [L:%d]\n", nLBA, tmpCurrentTrackNum, (INT)__LINE__);
				}
			}
		}
		// preserve mode, ctl
		if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[tIdx]) {
			pDisc->SUB.lpCtlList[tIdx] = pDiscPerSector->subQ.current.byCtl;
			pDisc->MAIN.lpModeList[tIdx] = GetMode(pDiscPerSector, unscrambled);
		}
		// preserve the 1st LBA of the changed index 
		if (pDiscPerSector->byTrackNum >= 1 && tmpPrevIndex + 1 == tmpCurrentIndex) {
			if (tmpCurrentIndex != 1) {
				if (pDisc->SUB.lpFirstLBAListOnSub[tIdx][tmpCurrentIndex] == -1) {
					OutputSubInfoWithLBALogA("Index is changed from [%02d] to [%02d] [L:%d]\n", nLBA
						, tmpCurrentTrackNum, tmpPrevIndex, tmpCurrentIndex, (INT)__LINE__);
					pDisc->SUB.lpFirstLBAListOnSub[tIdx][tmpCurrentIndex] = nLBA;
					pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][tmpCurrentIndex] = nLBA;
				}
			}
			else {
				if (nLBA != pDisc->SCSI.lpFirstLBAListOnToc[tIdx]) {
					OutputSubInfoWithLBALogA(
						"Subchannel & TOC doesn't sync. LBA on TOC[%d, %#x], prevIndex[%02u]\n",
						nLBA, tmpCurrentTrackNum, pDisc->SCSI.lpFirstLBAListOnToc[tIdx],
						pDisc->SCSI.lpFirstLBAListOnToc[tIdx], tmpPrevIndex);
				}
				OutputSubInfoWithLBALogA("Index is changed from [%02d] to [%02d] [L:%d]\n", nLBA
					, tmpCurrentTrackNum, tmpPrevIndex, tmpCurrentIndex, (INT)__LINE__);

				pDisc->SUB.lpFirstLBAListOnSub[tIdx][1] = pDisc->SCSI.lpFirstLBAListOnToc[tIdx];
				if (pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSub[tIdx][1]) {
					// Crow, The - Original Motion Picture Soundtrack (82519-2)
					// LBA 108975, Track[06], Subchannel & TOC isn't sync. LBA on TOC: 108972, prevIndex[00]
					pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] = -1;
				}

				pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][1] = pDisc->SCSI.lpFirstLBAListOnToc[tIdx];
				if (pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][1]) {
					pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] = -1;
				}
			}
		}
		else if (tmpPrevIndex >= 1 && tmpCurrentIndex == 0) {
#if 0
			for (INT i = pDisc->SCSI.toc.FirstTrack - 1; i < pDisc->SCSI.toc.LastTrack; i++) {
				OutputString(_T("pDisc->SUB.lpFirstLBAListOnSub[%d][%d]: %d, %p, LBA: %d\n")
					, i, tmpCurrentIndex, pDisc->SUB.lpFirstLBAListOnSub[i][tmpCurrentIndex], &pDisc->SUB.lpFirstLBAListOnSub[i][tmpCurrentIndex], nLBA);
			}
#endif
			if (pDisc->SUB.lpFirstLBAListOnSub[tIdx][tmpCurrentIndex] == -1) {
				OutputSubInfoWithLBALogA("Index is changed from [%02d] to [%02d] [L:%d]\n", nLBA
					, tmpCurrentTrackNum, tmpPrevIndex, tmpCurrentIndex, (INT)__LINE__);
				pDisc->SUB.lpFirstLBAListOnSub[tIdx][tmpCurrentIndex] = nLBA;
				pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][tmpCurrentIndex] = nLBA;
			}
#if 0
			for (INT i = pDisc->SCSI.toc.FirstTrack - 1; i < pDisc->SCSI.toc.LastTrack; i++) {
				OutputString(_T("pDisc->SUB.lpFirstLBAListOnSub[%d][%d]: %d, %p, LBA: %d\n")
					, i, tmpCurrentIndex, pDisc->SUB.lpFirstLBAListOnSub[i][tmpCurrentIndex], &pDisc->SUB.lpFirstLBAListOnSub[i][tmpCurrentIndex], nLBA);
			}
#endif
		}

		if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
			(pDiscPerSector->subQ.current.byCtl & AUDIO_DATA_TRACK) == 0) {
			OutputMainInfoWithLBALogA(
				"Data track, but this sector is audio\n", nLBA, tmpCurrentTrackNum);
		}
		else if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == 0 &&
			(pDiscPerSector->subQ.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			OutputMainInfoWithLBALogA(
				"Audio track, but this sector is data\n", nLBA, tmpCurrentTrackNum);
		}

		if (pExtArg->byReverse) {
			// preserve last LBA per data track
			if (nLBA == pDisc->SCSI.nLastLBAofDataTrack) {
				if (pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] == -1 &&
					(pDiscPerSector->subQ.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] = pDisc->SCSI.nLastLBAofDataTrack;
				}
			}
			else if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[tIdx] = nLBA;
			}
		}
		else {
			// preserve first LBA per data track
			if (pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[tIdx] == -1 &&
				(pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				OutputSubInfoWithLBALogA("1st LBA of this data track\n", nLBA, tmpCurrentTrackNum);
				pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[tIdx] = nLBA;
			}
			else if (nLBA == pDisc->SCSI.nAllLength - 1) {
				if (pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] == -1) {
					// preserve last LBA per data track
					if ((pDiscPerSector->subQ.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
						OutputSubInfoWithLBALogA(
							"Last LBA of this data track [L:%d]\n", nLBA, tmpCurrentTrackNum, (INT)__LINE__);
						pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] = pDisc->SCSI.nAllLength - 1;
					}
				}
				OutputSubInfoWithLBALogA("Last LBA of this disc\n", nLBA, tmpCurrentTrackNum);
			}
		}
	}
	else if (*pExecType != swap && tmpCurrentTrackNum == 110) {
		pDisc->SUB.lpLastLBAListOfDataTrackOnSub[pDiscPerSector->byTrackNum - 1] = nLBA;
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
		strncpy(pDisc->SUB.pszISRC[pDiscPerSector->byTrackNum - 1], pszOutString, META_ISRC_SIZE);
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
		strncpy(pDisc->SUB.szCatalog, pszOutString, sizeof(pDisc->SUB.szCatalog));
	}
}

VOID SetLBAForFirstAdr(
	INT nFirstLBA[][2],
	INT nRangeLBA[][2],
	LPCSTR strAdr,
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
		if (nFirstLBA[0][byIdxOfSession] == -1) {
			nFirstLBA[0][byIdxOfSession] = nAdrLBAList[0];
			nRangeLBA[0][byIdxOfSession] = nAdrLBAList[1] - nAdrLBAList[0];
			if (byPlxtrDrive == PLXTR_DRIVE_TYPE::PXS88T) {
				// Somehow PX-S88T is sliding subchannel +1;
				nFirstLBA[0][byIdxOfSession]++;
			}
		}
		OutputDiscLogA(
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
		if (nFirstLBA[0][byIdxOfSession] == -1) {
			nFirstLBA[0][byIdxOfSession] = nAdrLBAList[0] - 1;
			nRangeLBA[0][byIdxOfSession] = nAdrLBAList[1] - nAdrLBAList[0] + 1;
			if (byPlxtrDrive == PLXTR_DRIVE_TYPE::PXS88T) {
				// Somehow PX-S88T is sliding subchannel +1;
				nFirstLBA[0][byIdxOfSession]++;
			}
		}
		OutputDiscLogA(
			"\tSession %d, 1st %s sector is %d, %s sector exists per %d byte\n"
			, byIdxOfSession + 1, strAdr, nAdrLBAList[0] - 1
			, strAdr, nAdrLBAList[1] - nAdrLBAList[0] + 1);
	}
	else if (betweenThirdOne == betweenTourthTwo && betweenThirdOne == betweenFifthThird) {
		if (nFirstLBA[0][byIdxOfSession] == -1) {
			nFirstLBA[0][byIdxOfSession] = nAdrLBAList[0];
			nRangeLBA[0][byIdxOfSession] = betweenThirdOne;
			nFirstLBA[1][byIdxOfSession] = nAdrLBAList[1];
			nRangeLBA[1][byIdxOfSession] = betweenTourthTwo;
			if (byPlxtrDrive == PLXTR_DRIVE_TYPE::PXS88T) {
				// Somehow PX-S88T is sliding subchannel +1;
				nFirstLBA[0][byIdxOfSession]++;
				nFirstLBA[1][byIdxOfSession]++;
			}
		}
		OutputDiscLogA(
			"\tSession %d, 1st %s sector is %d, %s sector exists per %d\n"
			"\t            2nd %s sector is %d, %s sector exists per %d\n"
			, byIdxOfSession + 1
			, strAdr, nAdrLBAList[0], strAdr, betweenThirdOne
			, strAdr, nAdrLBAList[1], strAdr, betweenTourthTwo);
	}
	else if (first == second || first == third || second == third) {
		if (nFirstLBA[0][byIdxOfSession] == -1) {
			nFirstLBA[0][byIdxOfSession] = nAdrLBAList[0];
			nRangeLBA[0][byIdxOfSession] = nAdrLBAList[1] - nAdrLBAList[0];
			nFirstLBA[1][byIdxOfSession] = nAdrLBAList[0];
			nRangeLBA[1][byIdxOfSession] = nAdrLBAList[2] - nAdrLBAList[1];
			nFirstLBA[2][byIdxOfSession] = nAdrLBAList[0];
			nRangeLBA[2][byIdxOfSession] = nAdrLBAList[3] - nAdrLBAList[2];
			if (byPlxtrDrive == PLXTR_DRIVE_TYPE::PXS88T) {
				// Somehow PX-S88T is sliding subchannel +1;
				nFirstLBA[0][byIdxOfSession]++;
				nFirstLBA[1][byIdxOfSession]++;
				nFirstLBA[2][byIdxOfSession]++;
			}
		}
		OutputDiscLogA(
			"\tSession %d, 1st %s sector is %d, %s sector exists per %d, %d, %d\n"
			, byIdxOfSession + 1
			, strAdr, nAdrLBAList[0]
			, strAdr, nAdrLBAList[1] - nAdrLBAList[0]
			, nAdrLBAList[2] - nAdrLBAList[1]
			, nAdrLBAList[3] - nAdrLBAList[2]);
	}
}

VOID SetBufferFromTmpSubQData(
	LPBYTE lpSubcode,
	SUB_Q_PER_SECTOR subQ,
	BOOL bCurrent,
	BOOL bUpdateCrc
) {
	if (subQ.byTrackNum == 110) {
		if (subQ.byAdr == 3) {
			subQ.byAdr = 1;
		}
		lpSubcode[12] = BYTE(subQ.byCtl << 4 | subQ.byAdr);
		lpSubcode[13] = 0xaa;
	}
	else {
		lpSubcode[12] = BYTE(subQ.byCtl << 4 | subQ.byAdr);
		lpSubcode[13] = DecToBcd(subQ.byTrackNum);
	}
	lpSubcode[14] = DecToBcd(subQ.byIndex);
	BYTE m, s, f;
	if (bCurrent) {
		LBAtoMSF(subQ.nRelativeTime, &m, &s, &f);
	}
	else {
		LBAtoMSF(subQ.nRelativeTime + 1, &m, &s, &f);
	}
	lpSubcode[15] = DecToBcd(m);
	lpSubcode[16] = DecToBcd(s);
	lpSubcode[17] = DecToBcd(f);
	if (bCurrent) {
		LBAtoMSF(subQ.nAbsoluteTime, &m, &s, &f);
	}
	else {
		LBAtoMSF(subQ.nAbsoluteTime + 1, &m, &s, &f);
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

VOID SetTmpSubQDataFromBuffer(
	PSUB_Q_PER_SECTOR pSubQ,
	LPBYTE lpSubcode
) {
	pSubQ->byCtl = (BYTE)((lpSubcode[12] >> 4) & 0x0f);
	pSubQ->byAdr = (BYTE)(lpSubcode[12] & 0x0f);
	pSubQ->byTrackNum = BcdToDec(lpSubcode[13]);
	pSubQ->byIndex = BcdToDec(lpSubcode[14]);
	pSubQ->nRelativeTime = MSFtoLBA(BcdToDec(lpSubcode[15]),
		BcdToDec(lpSubcode[16]), BcdToDec(lpSubcode[17]));
	pSubQ->nAbsoluteTime = MSFtoLBA(BcdToDec(lpSubcode[19]),
		BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21]));
}

VOID UpdateTmpSubQDataForMCN(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA
) {
	// Subchannel pattern on MCN Sector
	// Tomb Raider III - Adventures of Lara Croft (Europe)(Mac)
	if (pDisc->SCSI.toc.LastTrack == 1) {
		pDiscPerSector->subQ.current.byTrackNum = 1;
		pDiscPerSector->subQ.current.byIndex = 1;
		if (nLBA > 0) {
			pDiscPerSector->subQ.current.nRelativeTime += 1;
		}
	}
	// pattern 1: pregap sector.
	else if (IsValidPregapSector(pDisc, &pDiscPerSector->subQ, nLBA)) {
		BOOL bValidPre = FALSE;
		// pattern 1-1: prev sector is audio.
		if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == 0) {
			// pattern 1-1-1: current sector is audio.
			if ((pDiscPerSector->subQ.current.byCtl & AUDIO_DATA_TRACK) == 0) {
				if (pDiscPerSector->subQ.prev.byTrackNum > 0) {
					if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum] - 225) {
						// Atlas, The - Renaissance Voyager (Japan)
						// LBA[003364, 0x00d24], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:44:64], AMSF[00:46:64], RtoW[0, 0, 0, 0]
						// LBA[003365, 0x00d25], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :65], RtoW[0, 0, 0, 0]
						// LBA[003366, 0x00d26], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:02:73], AMSF[00:46:66], RtoW[0, 0, 0, 0]
						// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
						// LBA[261585, 0x3fDD1], Audio, 2ch, Copy NG, Pre-emphasis No, Track[95], Idx[01], RMSF[00:13:69], AMSF[58:09:60], RtoW[0, 0, 0, 0]
						// LBA[261586, 0x3fDD2], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :61], RtoW[0, 0, 0, 0]
						// LBA[261587, 0x3fDD3], Audio, 2ch, Copy NG, Pre-emphasis No, Track[96], Idx[00], RMSF[00:02:73], AMSF[58:09:62], RtoW[0, 0, 0, 0]
						pDiscPerSector->subQ.current.nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum] - 150) {
						pDiscPerSector->subQ.current.nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum] - 149) {
						pDiscPerSector->subQ.current.nRelativeTime = 148;
						bValidPre = TRUE;
					}
				}
				if (bValidPre && pExtArg->byMCN) {
					// pattern 1-1-1-1: change track.
					pDiscPerSector->subQ.current.byTrackNum = (BYTE)(pDiscPerSector->subQ.prev.byTrackNum + 1);
					// pattern 1-1-1-1-1: change index. (pregap sector is 0)
					pDiscPerSector->subQ.current.byIndex = 0;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subQ.current.byTrackNum, pDiscPerSector->subQ.prev.byTrackNum);
				}
				else {
					// pattern 1-1-1-2: not change track.
					pDiscPerSector->subQ.current.byTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
					// pattern 1-1-1-2-2: not change index.
					pDiscPerSector->subQ.current.byIndex = pDiscPerSector->subQ.prev.byIndex;
				}
			}
			// pattern 1-1-2: current sector is data.
			else if ((pDiscPerSector->subQ.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				if (pDiscPerSector->subQ.prev.byTrackNum > 0) {
					if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum] - 225) {
						pDiscPerSector->subQ.current.nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum] - 150) {
						pDiscPerSector->subQ.current.nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum] - 149) {
						pDiscPerSector->subQ.current.nRelativeTime = 148;
						bValidPre = TRUE;
					}
				}
				if (bValidPre && pExtArg->byMCN) {
					// pattern 1-1-2-1: change track.
					pDiscPerSector->subQ.current.byTrackNum = (BYTE)(pDiscPerSector->subQ.prev.byTrackNum + 1);
					// pattern 1-1-2-1-1: change index. (pregap sector is 0)
					pDiscPerSector->subQ.current.byIndex = 0;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subQ.current.byTrackNum, pDiscPerSector->subQ.prev.byTrackNum);
				}
				else {
					// pattern 1-1-2-2: not change track.
					pDiscPerSector->subQ.current.byTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
					// pattern 1-1-2-2-2: not change index.
					pDiscPerSector->subQ.current.byIndex = pDiscPerSector->subQ.prev.byIndex;
				}
			}
		}
		// pattern 1-2: prev sector is data.
		else if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			// pattern 1-2-1: current sector is audio.
			if ((pDiscPerSector->subQ.current.byCtl & AUDIO_DATA_TRACK) == 0) {
				if (pDiscPerSector->subQ.prev.byTrackNum > 0) {
					if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum] - 225) {
						pDiscPerSector->subQ.current.nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum] - 150) {
						// Valis IV (Japan)
						// LBA[157830, 0x26886],  Data,      Copy NG,                  Track[44], Idx[01], RMSF[00:06:27], AMSF[35:06:30], RtoW[0, 0, 0, 0]
						// LBA[157831, 0x26887], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :31], RtoW[0, 0, 0, 0]
						// LBA[157832, 0x26888], Audio, 2ch, Copy NG, Pre-emphasis No, Track[45], Idx[00], RMSF[00:01:73], AMSF[35:06:32], RtoW[0, 0, 0, 0]
						// Cosmic Fantasy 2
						// LBA[202749, 0x317FD],  Data,      Copy NG,                  Track[80], Idx[01], RMSF[00:06:63], AMSF[45:05:24], RtoW[0, 0, 0, 0]
						// LBA[202750, 0x317FE], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :25], RtoW[0, 0, 0, 0]
						// LBA[202751, 0x317FF], Audio, 2ch, Copy NG, Pre-emphasis No, Track[81], Idx[00], RMSF[00:01:73], AMSF[45:05:26], RtoW[0, 0, 0, 0]
						pDiscPerSector->subQ.current.nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum] - 149) {
						pDiscPerSector->subQ.current.nRelativeTime = 148;
						bValidPre = TRUE;
					}
				}
				if (bValidPre && pExtArg->byMCN) {
					// pattern 1-2-1-1: change track.
					pDiscPerSector->subQ.current.byTrackNum = (BYTE)(pDiscPerSector->subQ.prev.byTrackNum + 1);
					// pattern 1-2-1-1-1: change index. (pregap sector is 0)
					pDiscPerSector->subQ.current.byIndex = 0;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subQ.current.byTrackNum, pDiscPerSector->subQ.prev.byTrackNum);
				}
				else {
					// pattern 1-2-1-2: not change track.
					pDiscPerSector->subQ.current.byTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
					// pattern 1-2-1-2-2: not change index.
					pDiscPerSector->subQ.current.byIndex = pDiscPerSector->subQ.prev.byIndex;
				}
			}
			// pattern 1-2-2: current sector is data.
			else if ((pDiscPerSector->subQ.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				if (pDiscPerSector->subQ.prev.byTrackNum > 0) {
					if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum] - 225) {
						pDiscPerSector->subQ.current.nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum] - 150) {
						pDiscPerSector->subQ.current.nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum] - 149) {
						pDiscPerSector->subQ.current.nRelativeTime = 148;
						bValidPre = TRUE;
					}
				}
				if (bValidPre && pExtArg->byMCN) {
					// pattern 1-2-2-1: change track.
					pDiscPerSector->subQ.current.byTrackNum = (BYTE)(pDiscPerSector->subQ.prev.byTrackNum + 1);
					// pattern 1-2-2-1-1: change index. (pregap sector is 0)
					pDiscPerSector->subQ.current.byIndex = 0;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subQ.current.byTrackNum, pDiscPerSector->subQ.prev.byTrackNum);
				}
				else {
					// pattern 1-2-2-2: not change track.
					pDiscPerSector->subQ.current.byTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
					// pattern 1-2-2-2-2: not change index.
					pDiscPerSector->subQ.current.byIndex = pDiscPerSector->subQ.prev.byIndex;
				}
			}
		}
	}
	// pattern 2: not pregap sector.
	else {
		// pattern 2-1: prev sector is audio.
		if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == 0) {
			// pattern 2-1-1: current sector is audio.
			if ((pDiscPerSector->subQ.current.byCtl & AUDIO_DATA_TRACK) == 0) {
				// 1st sector of tracks
				if (pDiscPerSector->subQ.prev.byTrackNum > 0 &&
					nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum]) {
					// Madou Monogatari I - Honoo no Sotsuenji (Japan)
					// LBA[183031, 0x2CAF7], Audio, 2ch, Copy NG, Pre-emphasis No, Track[21], Idx[01], RMSF[00:31:70], AMSF[40:42:31], RtoW[0, 0, 0, 0]
					// LBA[183032, 0x2CAF8], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :32], RtoW[0, 0, 0, 0]
					// LBA[183033, 0x2CAF9], Audio, 2ch, Copy NG, Pre-emphasis No, Track[22], Idx[01], RMSF[00:00:01], AMSF[40:42:33], RtoW[0, 0, 0, 0]
					pDiscPerSector->subQ.current.nRelativeTime = 0;
					// pattern 2-1-1-1: change track.
					pDiscPerSector->subQ.current.byTrackNum = (BYTE)(pDiscPerSector->subQ.prev.byTrackNum + 1);
					// pattern 2-1-1-1-2: not change index.
					pDiscPerSector->subQ.current.byIndex = pDiscPerSector->subQ.prev.byIndex;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subQ.current.byTrackNum, pDiscPerSector->subQ.prev.byTrackNum);
				}
				// 1st next index of same tracks
				else if (pDiscPerSector->subQ.prev.byIndex == 0 && pDiscPerSector->subQ.prev.nRelativeTime == 0) {
					// Psychic Detective Series Vol. 5 - Nightmare (Japan)
					// LBA[080999, 0x13C67], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:00:00], AMSF[18:01:74], RtoW[0, 0, 0, 0]
					// LBA[081000, 0x13C68], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [3010911111863], AMSF[     :00], RtoW[0, 0, 0, 0]
					// LBA[081001, 0x13C69], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[01], RMSF[00:00:01], AMSF[18:02:01], RtoW[0, 0, 0, 0]
					pDiscPerSector->subQ.current.nRelativeTime = 0;
					// pattern 2-1-1-2: not change track.
					pDiscPerSector->subQ.current.byTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
					// pattern 2-1-1-2-1: change index.
					pDiscPerSector->subQ.current.byIndex = (BYTE)(pDiscPerSector->subQ.prev.byIndex + 1);
				}
				// 1st index of same tracks
				else if (pDiscPerSector->subQ.prev.byIndex > 1 && pDiscPerSector->subQ.prev.byIndex != pDiscPerSector->subQ.next.byIndex) {
					if (pDiscPerSector->subQ.prev.byIndex + 1 == pDiscPerSector->subQ.next.byIndex) {
						// Space Jam (Japan)
						// LBA[056262, 0x0dbc6], Audio, 2ch, Copy NG, Pre-emphasis No, Track[04], Idx[53], RMSF[01:38:65], AMSF[12:32:12], RtoW[0, 0, 0, 0]
						// LBA[056263, 0x0dbc7], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :13], RtoW[0, 0, 0, 0]
						// LBA[056264, 0x0dbc8], Audio, 2ch, Copy NG, Pre-emphasis No, Track[04], Idx[54], RMSF[01:38:67], AMSF[12:32:14], RtoW[0, 0, 0, 0]
						// Space Jam (Japan)
						// LBA[086838, 0x15336], Audio, 2ch, Copy NG, Pre-emphasis No, Track[06], Idx[82], RMSF[02:31:05], AMSF[19:19:63], RtoW[0, 0, 0, 0]
						// LBA[086839, 0x15337], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :64], RtoW[0, 0, 0, 0]
						// LBA[086840, 0x15338], Audio, 2ch, Copy NG, Pre-emphasis No, Track[06], Idx[83], RMSF[02:31:07], AMSF[19:19:65], RtoW[0, 0, 0, 0]
						pDiscPerSector->subQ.current.nRelativeTime = pDiscPerSector->subQ.prev.nRelativeTime + 1;
						// pattern 2-1-1-2: not change track.
						pDiscPerSector->subQ.current.byTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
						// pattern 2-1-1-2-1: change index.
						pDiscPerSector->subQ.current.byIndex = (BYTE)(pDiscPerSector->subQ.prev.byIndex + 1);
					}
				}
				// same index of same tracks
				else {
					if (pDiscPerSector->subQ.prev.byIndex == 0) {
						// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
						// LBA[003413, 0x00D55], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:02:26], AMSF[00:47:38], RtoW[0, 0, 0, 0]
						// LBA[003414, 0x00D56], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :39], RtoW[0, 0, 0, 0]
						// LBA[003415, 0x00D57], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:02:24], AMSF[00:47:40], RtoW[0, 0, 0, 0]
						pDiscPerSector->subQ.current.nRelativeTime = pDiscPerSector->subQ.prev.nRelativeTime - 1;
					}
					else if (pDiscPerSector->subQ.prev.byIndex > 0) {
						// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
						// LBA[081541, 0x13E85], Audio, 2ch, Copy NG, Pre-emphasis No, Track[18], Idx[01], RMSF[00:12:57], AMSF[18:09:16], RtoW[0, 0, 0, 0]
						// LBA[081542, 0x13E86], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :17], RtoW[0, 0, 0, 0]
						// LBA[081543, 0x13E87], Audio, 2ch, Copy NG, Pre-emphasis No, Track[18], Idx[01], RMSF[00:12:59], AMSF[18:09:18], RtoW[0, 0, 0, 0]
						pDiscPerSector->subQ.current.nRelativeTime = pDiscPerSector->subQ.prev.nRelativeTime + 1;
					}
					// pattern 2-1-1-2: not change track.
					pDiscPerSector->subQ.current.byTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
					// pattern 2-1-1-2-2: not change index.
					pDiscPerSector->subQ.current.byIndex = pDiscPerSector->subQ.prev.byIndex;
				}
			}
			// pattern 2-1-2: current sector is data.
			else if ((pDiscPerSector->subQ.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				// 1st sector of tracks
				if (pDiscPerSector->subQ.prev.byTrackNum > 0 &&
					nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum]) {
					pDiscPerSector->subQ.current.nRelativeTime = 0;
					// pattern 2-1-2-1: change track.
					pDiscPerSector->subQ.current.byTrackNum = (BYTE)(pDiscPerSector->subQ.prev.byTrackNum + 1);
					// pattern 2-1-2-1-2: not change index.
					pDiscPerSector->subQ.current.byIndex = pDiscPerSector->subQ.prev.byIndex;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subQ.current.byTrackNum, pDiscPerSector->subQ.prev.byTrackNum);
				}
				// 1st next index of same tracks
				else if (pDiscPerSector->subQ.prev.byIndex == 0 && pDiscPerSector->subQ.prev.nRelativeTime == 0) {
					pDiscPerSector->subQ.current.nRelativeTime = 0;
					// pattern 2-1-2-2: not change track.
					pDiscPerSector->subQ.current.byTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
					// pattern 2-1-2-2-1: change index.
					pDiscPerSector->subQ.current.byIndex = (BYTE)(pDiscPerSector->subQ.prev.byIndex + 1);
				}
				// same index of same tracks
				else {
					if (pDiscPerSector->subQ.prev.byIndex == 0) {
						pDiscPerSector->subQ.current.nRelativeTime = pDiscPerSector->subQ.prev.nRelativeTime - 1;
					}
					else if (pDiscPerSector->subQ.prev.byIndex > 0) {
						pDiscPerSector->subQ.current.nRelativeTime = pDiscPerSector->subQ.prev.nRelativeTime + 1;
					}
					// pattern 2-1-2-2: not change track.
					pDiscPerSector->subQ.current.byTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
					// pattern 2-1-2-2-2: not change index.
					pDiscPerSector->subQ.current.byIndex = pDiscPerSector->subQ.prev.byIndex;
				}
			}
		}
		// pattern 2-2: prev sector is data.
		else if ((pDiscPerSector->subQ.prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			// pattern 2-2-1: current sector is audio.
			if ((pDiscPerSector->subQ.current.byCtl & AUDIO_DATA_TRACK) == 0) {
				// 1st sector of tracks
				if (pDiscPerSector->subQ.prev.byTrackNum > 0 &&
					nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum]) {
					pDiscPerSector->subQ.current.nRelativeTime = 0;
					// pattern 2-2-1-1: change track.
					pDiscPerSector->subQ.current.byTrackNum = (BYTE)(pDiscPerSector->subQ.prev.byTrackNum + 1);
					// pattern 2-2-1-1-2: not change index.
					pDiscPerSector->subQ.current.byIndex = pDiscPerSector->subQ.prev.byIndex;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subQ.current.byTrackNum, pDiscPerSector->subQ.prev.byTrackNum);
				}
				// 1st next index of same tracks
				else if (pDiscPerSector->subQ.prev.byIndex == 0 && pDiscPerSector->subQ.prev.nRelativeTime == 0) {
					pDiscPerSector->subQ.current.nRelativeTime = 0;
					// pattern 2-2-1-2: not change track.
					pDiscPerSector->subQ.current.byTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
					// pattern 2-2-1-2-1: change index.
					pDiscPerSector->subQ.current.byIndex = (BYTE)(pDiscPerSector->subQ.prev.byIndex + 1);
				}
				// same index of same tracks
				else {
					if (pDiscPerSector->subQ.prev.byIndex == 0) {
						pDiscPerSector->subQ.current.nRelativeTime = pDiscPerSector->subQ.prev.nRelativeTime - 1;
					}
					else if (pDiscPerSector->subQ.prev.byIndex > 0) {
						// EVE - burst error (Disc 3) (Terror Disc)
						// LBA[188021, 0x2DE75],  Data,      Copy NG,                  Track[01], Idx[01], RMSF[41:46:71], AMSF[41:48:71], RtoW[0, 0, 0, 0]
						// LBA[188022, 0x2DE76], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :72], RtoW[0, 0, 0, 0]
						// LBA[188023, 0x2DE77],  Data,      Copy NG,                  Track[01], Idx[01], RMSF[41:46:73], AMSF[41:48:73], RtoW[0, 0, 0, 0]
						pDiscPerSector->subQ.current.nRelativeTime = pDiscPerSector->subQ.prev.nRelativeTime + 1;
					}
					// pattern 2-2-1-2: not change track.
					pDiscPerSector->subQ.current.byTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
					// pattern 2-2-1-2-2: not change index.
					pDiscPerSector->subQ.current.byIndex = pDiscPerSector->subQ.prev.byIndex;
				}
			}
			// pattern 2-2-2: current sector is data.
			else if ((pDiscPerSector->subQ.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				// 1st sector of tracks
				if (pDiscPerSector->subQ.prev.byTrackNum > 0 &&
					nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum]) {
					pDiscPerSector->subQ.current.nRelativeTime = 0;
					// pattern 2-2-2-1: change track.
					pDiscPerSector->subQ.current.byTrackNum = (BYTE)(pDiscPerSector->subQ.prev.byTrackNum + 1);
					// pattern 2-2-2-1-2: not change index.
					pDiscPerSector->subQ.current.byIndex = pDiscPerSector->subQ.prev.byIndex;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, pDiscPerSector->byTrackNum + 1, pDiscPerSector->subQ.current.byTrackNum, pDiscPerSector->subQ.prev.byTrackNum);
				}
				// 1st next index of same tracks
				else if (pDiscPerSector->subQ.prev.byIndex == 0 && pDiscPerSector->subQ.prev.nRelativeTime == 0) {
					// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
					// LBA[142873, 0x22E19], Data, Copy NG, Track[37], Idx[00], RMSF[00:00:00], AMSF[31:46:73], RtoW[0, 0, 0, 0]
					// LBA[142874, 0x22E1A], Data, Copy NG, MediaCatalogNumber [0000000000000], AMSF[     :74], RtoW[0, 0, 0, 0]
					// LBA[142875, 0x22E1B], Data, Copy NG, Track[37], Idx[01], RMSF[00:00:01], AMSF[31:47:00], RtoW[0, 0, 0, 0]
					pDiscPerSector->subQ.current.nRelativeTime = 0;
					// pattern 2-2-2-2: not change track.
					pDiscPerSector->subQ.current.byTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
					// pattern 2-2-2-2-1: change index.
					pDiscPerSector->subQ.current.byIndex = (BYTE)(pDiscPerSector->subQ.prev.byIndex + 1);
				}
				// same index of same tracks
				else {
					if (pDiscPerSector->subQ.prev.byIndex == 0) {
						pDiscPerSector->subQ.current.nRelativeTime = pDiscPerSector->subQ.prev.nRelativeTime - 1;
					}
					else if (pDiscPerSector->subQ.prev.byIndex > 0) {
						// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
						// LBA[174261, 0x2A8B5], Data, Copy NG, Track[60], Idx[01], RMSF[00:06:19], AMSF[38:45:36], RtoW[0, 0, 0, 0]
						// LBA[174262, 0x2A8B6], Data, Copy NG, MediaCatalogNumber [0000000000000], AMSF[     :37], RtoW[0, 0, 0, 0]
						// LBA[174263, 0x2A8B7], Data, Copy NG, Track[60], Idx[01], RMSF[00:06:21], AMSF[38:45:38], RtoW[0, 0, 0, 0]
						pDiscPerSector->subQ.current.nRelativeTime = pDiscPerSector->subQ.prev.nRelativeTime + 1;
					}
					// pattern 2-2-2-2: not change track.
					pDiscPerSector->subQ.current.byTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
					// pattern 2-2-2-2-2: not change index.
					pDiscPerSector->subQ.current.byIndex = pDiscPerSector->subQ.prev.byIndex;
				}
			}
		}
	}
}

VOID UpdateTmpSubQDataForISRC(
	PSUB_Q pSubQ
) {
	pSubQ->current.byTrackNum = pSubQ->prev.byTrackNum;
	pSubQ->current.byIndex = pSubQ->prev.byIndex;
	if (pSubQ->current.byIndex == 0) {
		pSubQ->current.nRelativeTime = pSubQ->prev.nRelativeTime - 1;
	}
	else if (pSubQ->current.byIndex > 0) {
		pSubQ->current.nRelativeTime = pSubQ->prev.nRelativeTime + 1;
	}
}

VOID UpdateTmpSubQDataForCDTV(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA
) {
	if (IsValidPregapSector(pDisc, &pDiscPerSector->subQ, nLBA)) {
		pDiscPerSector->subQ.current.byTrackNum = (BYTE)(pDiscPerSector->subQ.prev.byTrackNum + 1);
		pDiscPerSector->subQ.current.byIndex = 0;
		if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum] - 225) {
			pDiscPerSector->subQ.current.nRelativeTime = 224;
		}
		else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum] - 150) {
			pDiscPerSector->subQ.current.nRelativeTime = 149;
		}
		else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->subQ.prev.byTrackNum] - 149) {
			pDiscPerSector->subQ.current.nRelativeTime = 148;
		}
	}
	else if (pDisc->SCSI.lpFirstLBAListOnToc[pDiscPerSector->byTrackNum] == nLBA) {
		pDiscPerSector->subQ.current.byTrackNum = (BYTE)(pDiscPerSector->subQ.prev.byTrackNum + 1);
		pDiscPerSector->subQ.current.byIndex = 1;
		pDiscPerSector->subQ.current.nRelativeTime = pDiscPerSector->subQ.prev.nRelativeTime;
	}
}

VOID UpdateTmpSubQData(
	PDISC_PER_SECTOR pDiscPerSector
) {
	// TODO: Doesn't need?
	if (pDiscPerSector->subQ.prev.byIndex == 0 && pDiscPerSector->subQ.prev.nRelativeTime == 0) {
		pDiscPerSector->subQ.prev.byIndex = 1;
	}
	pDiscPerSector->subQ.prevPrev.byCtl = pDiscPerSector->subQ.prev.byCtl;
	if (pDiscPerSector->subQ.prev.byAdr != ADR_ENCODES_MEDIA_CATALOG &&
		pDiscPerSector->subQ.prev.byAdr != ADR_ENCODES_ISRC) {
		pDiscPerSector->subQ.prevPrev.byAdr = pDiscPerSector->subQ.prev.byAdr;
		pDiscPerSector->subQ.prevPrev.nRelativeTime = pDiscPerSector->subQ.prev.nRelativeTime;
	}
	pDiscPerSector->subQ.prevPrev.byTrackNum = pDiscPerSector->subQ.prev.byTrackNum;
	pDiscPerSector->subQ.prevPrev.byIndex = pDiscPerSector->subQ.prev.byIndex;
	pDiscPerSector->subQ.prevPrev.nAbsoluteTime = pDiscPerSector->subQ.prev.nAbsoluteTime;

	pDiscPerSector->subQ.prev.byCtl = pDiscPerSector->subQ.current.byCtl;
	pDiscPerSector->subQ.prev.byAdr = pDiscPerSector->subQ.current.byAdr;
	pDiscPerSector->subQ.prev.byTrackNum = pDiscPerSector->subQ.current.byTrackNum;
	pDiscPerSector->subQ.prev.byIndex = pDiscPerSector->subQ.current.byIndex;
	if (pDiscPerSector->bLibCrypt || pDiscPerSector->bSecuRom) {
		pDiscPerSector->subQ.prev.nRelativeTime++;
	}
	else {
		pDiscPerSector->subQ.prev.nRelativeTime = pDiscPerSector->subQ.current.nRelativeTime;
	}
	pDiscPerSector->subQ.prev.nAbsoluteTime++;
}

VOID UpdateTmpMainHeader(
	PDISC_PER_SECTOR pDiscPerSector,
	INT nMainDataType
) {
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
