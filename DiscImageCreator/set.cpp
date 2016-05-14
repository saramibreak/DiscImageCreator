/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "get.h"
#include "output.h"
#include "outputScsiCmdLog.h"
#include "_external\crc16ccitt.h"

VOID SetReadCDCommand(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	CDB::_READ_CD* cdb,
	CDFLAG::_READ_CD::_EXPECTED_SECTOR_TYPE type,
	DWORD dwTransferLen,
	CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION Sub,
	BOOL bCheckReading
	)
{
	cdb->OperationCode = SCSIOP_READ_CD;
	cdb->ExpectedSectorType = type;
	cdb->Lun = pDevice->address.Lun;
	cdb->TransferBlocks[0] = LOBYTE(HIWORD(dwTransferLen));
	cdb->TransferBlocks[1] = HIBYTE(LOWORD(dwTransferLen));
	cdb->TransferBlocks[2] = LOBYTE(LOWORD(dwTransferLen));
	if (!bCheckReading && pDevice->FEATURE.byC2ErrorData &&
		(pExtArg && pExtArg->byC2)) {
		cdb->ErrorFlags = CDFLAG::_READ_CD::byte294;
	}
	cdb->IncludeEDC = TRUE;
	cdb->IncludeUserData = TRUE;
	cdb->HeaderCode = CDFLAG::_READ_CD::BothHeader;
	cdb->IncludeSyncData = TRUE;
	cdb->SubChannelSelection = Sub;
}

VOID SetReadD8Command(
	PDEVICE pDevice,
	CDB::_PLXTR_READ_CDDA* cdb,
	DWORD dwTransferLen,
	CDFLAG::_PLXTR_READ_CDDA::_SUB_CHANNEL_SELECTION Sub
	)
{
	cdb->OperationCode = SCSIOP_PLXTR_READ_CDDA;
	cdb->LogicalUnitNumber = pDevice->address.Lun;
	cdb->TransferBlockByte0 = HIBYTE(HIWORD(dwTransferLen));
	cdb->TransferBlockByte1 = LOBYTE(HIWORD(dwTransferLen));
	cdb->TransferBlockByte2 = HIBYTE(LOWORD(dwTransferLen));
	cdb->TransferBlockByte3 = LOBYTE(LOWORD(dwTransferLen));
	cdb->SubCode = (UCHAR)Sub;
}

VOID SetCommandForTransferLength(
	CDB::_READ12* pCdb,
	DWORD dwSize,
	LPBYTE lpTransferLen
	)
{
	*lpTransferLen = (BYTE)(dwSize / DISC_RAW_READ_SIZE);
	// Generally, directory size is per 2048 byte
	// Exception:
	//  Codename - Outbreak (Europe) (Sold Out Software)
	//  Commandos - Behind Enemy Lines (Europe) (Sold Out Software)
	// and more
	if (dwSize % DISC_RAW_READ_SIZE != 0) {
		(*lpTransferLen)++;
	}
	pCdb->TransferLength[3] = *lpTransferLen;
}

VOID SetBufferSizeForReadCD(
	PDEVICE pDevice,
	DRIVE_DATA_ORDER order
	)
{
	pDevice->TRANSFER.dwTransferLen = 1;
	if (order == DRIVE_DATA_ORDER::NoC2) {
		pDevice->TRANSFER.dwBufLen = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
		pDevice->TRANSFER.dwAllBufLen =
			pDevice->TRANSFER.dwBufLen * pDevice->TRANSFER.dwTransferLen;
		pDevice->TRANSFER.dwAdditionalBufLen = 0;
		pDevice->TRANSFER.dwBufC2Offset = 0;
		pDevice->TRANSFER.dwBufSubOffset = CD_RAW_SECTOR_SIZE;
	}
	else if (order == DRIVE_DATA_ORDER::MainC2Sub) {
		pDevice->TRANSFER.dwBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
		if (pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXW4824A) {
			pDevice->TRANSFER.dwAdditionalBufLen = 34; // unknown size
		}
		else {
			pDevice->TRANSFER.dwAdditionalBufLen = 0;
		}
		pDevice->TRANSFER.dwAllBufLen =
			(pDevice->TRANSFER.dwBufLen + pDevice->TRANSFER.dwAdditionalBufLen) * pDevice->TRANSFER.dwTransferLen;
		pDevice->TRANSFER.dwBufC2Offset = CD_RAW_SECTOR_SIZE;
		pDevice->TRANSFER.dwBufSubOffset = CD_RAW_SECTOR_WITH_C2_294_SIZE;
	}
	else if (order == DRIVE_DATA_ORDER::MainSubC2) {
		pDevice->TRANSFER.dwBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
		pDevice->TRANSFER.dwAdditionalBufLen = 0;
		pDevice->TRANSFER.dwAllBufLen =
			pDevice->TRANSFER.dwBufLen * pDevice->TRANSFER.dwTransferLen;
		pDevice->TRANSFER.dwBufSubOffset = CD_RAW_SECTOR_SIZE;
		pDevice->TRANSFER.dwBufC2Offset = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
	}
}

VOID SetFeatureCdRead(
	PFEATURE_DATA_CD_READ pCDRead,
	PDEVICE pDevice
	)
{
	pDevice->FEATURE.byCanCDText = pCDRead->CDText;
	pDevice->FEATURE.byC2ErrorData = pCDRead->C2ErrorData;
}

VOID SetFeatureRealTimeStreaming(
	PFEATURE_DATA_REAL_TIME_STREAMING pRTS,
	PDEVICE pDevice
	)
{
	pDevice->FEATURE.byModePage2a = pRTS->WriteSpeedInMP2A;
	pDevice->FEATURE.bySetCDSpeed = pRTS->SetCDSpeed;
	pDevice->FEATURE.byReadBufCapa = pRTS->ReadBufferCapacityBlock;
}

VOID SetAndOutputToc(
	PEXEC_TYPE pExecType,
	PDISC pDisc
	)
{
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(TOC));
	CONST INT typeSize = 7;
	CHAR strType[typeSize] = { 0 };
	BOOL bFirstData = TRUE;
	BYTE byAudioOnly = TRUE;
	for (BYTE i = pDisc->SCSI.toc.FirstTrack; i <= pDisc->SCSI.toc.LastTrack; i++) {
		for (INT j = 0, k = 24; j < 4; j++, k -= 8) {
			pDisc->SCSI.lpFirstLBAListOnToc[i - 1] |= 
				pDisc->SCSI.toc.TrackData[i - 1].Address[j] << k;
			pDisc->SCSI.lpLastLBAListOnToc[i - 1] |= 
				pDisc->SCSI.toc.TrackData[i].Address[j] << k;
		}
		pDisc->SCSI.lpLastLBAListOnToc[i - 1] -= 1;
		pDisc->SCSI.nAllLength += 
			pDisc->SCSI.lpLastLBAListOnToc[i - 1] - pDisc->SCSI.lpFirstLBAListOnToc[i - 1] + 1;

		if ((pDisc->SCSI.toc.TrackData[i - 1].Control & AUDIO_DATA_TRACK) == 0) {
			strncpy(strType, " Audio", typeSize);
		}
		else if ((pDisc->SCSI.toc.TrackData[i - 1].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			strncpy(strType, "  Data", typeSize);
			if (bFirstData) {
				pDisc->SCSI.nFirstLBAofDataTrack = 
					pDisc->SCSI.lpFirstLBAListOnToc[i - 1];
				pDisc->SCSI.byFirstDataTrackNum = i;
				bFirstData = FALSE;
				byAudioOnly = FALSE;
			}
			pDisc->SCSI.nLastLBAofDataTrack = 
				pDisc->SCSI.lpLastLBAListOnToc[i - 1];
			pDisc->SCSI.byLastDataTrackNum = i;
		}
		if (i == pDisc->SCSI.toc.FirstTrack && 
			pDisc->SCSI.lpFirstLBAListOnToc[i - 1] > 0) {
			pDisc->SCSI.nAllLength += pDisc->SCSI.lpFirstLBAListOnToc[i - 1];
			OutputDiscLogA(
				"\tPregap Track   , LBA %8u-%8u, Length %8u\n",
				0, pDisc->SCSI.lpFirstLBAListOnToc[i - 1] - 1, 
				pDisc->SCSI.lpFirstLBAListOnToc[i - 1]);
		}
		OutputDiscLogA(
			"\t%s Track %2u, LBA %8u-%8u, Length %8u\n", strType, i,
			pDisc->SCSI.lpFirstLBAListOnToc[i - 1], pDisc->SCSI.lpLastLBAListOnToc[i - 1],
			pDisc->SCSI.lpLastLBAListOnToc[i - 1] - pDisc->SCSI.lpFirstLBAListOnToc[i - 1] + 1);
	}
	OutputDiscLogA(
		"\t                                        Total  %8u\n", pDisc->SCSI.nAllLength);
	if (*pExecType != gd) {
		pDisc->SCSI.byAudioOnly = byAudioOnly;
	}
}

VOID SetAndOutputTocFull(
	PDISC pDisc,
	PCDROM_TOC_FULL_TOC_DATA fullToc,
	PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData,
	WORD wTocEntries,
	FILE* fpCcd
	)
{
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(FULL TOC)
		"\tFirstCompleteSession: %u\n"
		"\t LastCompleteSession: %u\n",
		fullToc->FirstCompleteSession,
		fullToc->LastCompleteSession);
	BOOL bFirst2ndSession = TRUE;
	pDisc->SCSI.nFirstLBAofLeadout = -1;
	pDisc->SCSI.nFirstLBAof2ndSession = -1;

	for (WORD a = 0; a < wTocEntries; a++) {
		INT nTmpLBAExt = 0;
		INT nTmpLBA = 0;
		if (fpCcd) {
			WriteCcdForEntry(pTocData, a, fpCcd);
		}
		switch (pTocData[a].Point) {
		case 0xa0:
			OutputDiscLogA("\tSession %u, FirstTrack %2u, ", 
				pTocData[a].SessionNumber, pTocData[a].Msf[0]);
			switch (pTocData[a].Msf[1]) {
				case 0x00:
					OutputDiscLogA("Format: CD-DA or CD-ROM\n");
					break;
				case 0x10:
					OutputDiscLogA("Format: CD-I\n");
					pDisc->SCSI.byCdi = TRUE;
					break;
				case 0x20:
					OutputDiscLogA("Format: CD-ROM-XA\n");
					break;
				default:
					OutputDiscLogA("Format: Other\n");
					break;
			}
			break;
		case 0xa1:
			OutputDiscLogA("\tSession %u,  LastTrack %2u\n", 
				pTocData[a].SessionNumber, pTocData[a].Msf[0]);
			break;
		case 0xa2:
			nTmpLBA = 
				MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			OutputDiscLogA(
				"\tSession %u,      Leadout, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n", 
				pTocData[a].SessionNumber, pTocData[a].Msf[0], pTocData[a].Msf[1],
				pTocData[a].Msf[2], nTmpLBA, nTmpLBA);
			pDisc->SCSI.lpLastLBAListOnToc[pDisc->SCSI.toc.LastTrack - 1] = nTmpLBA - 150 - 1;
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
				"\tSession %u,  NextSession, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
				"\t     Outermost Lead-out, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n" 
				"\tThe number of different Mode-5 pointers present %02u\n", 
				pTocData[a].SessionNumber, 
				pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2], 
				nTmpLBAExt, nTmpLBAExt,
				pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2],
				nTmpLBA, nTmpLBA, pTocData[a].Zero);
			break;
		case 0xb1: // (Audio only: This identifies the presence of skip intervals)
			OutputDiscLogA(
				"\tThe number of skip interval pointers %2u\n"
				"\tThe number of skip track assignments %2u\n",
				pTocData[a].Msf[0], pTocData[a].Msf[1]);
			break;
		case 0xb2: // (Audio only: This identifies tracks that should be skipped during playback)
		case 0xb3:
		case 0xb4:
			OutputDiscLogA(
				"\tTrack number to skip upon playback %2u %2u %2u %2u %2u %2u\n"
				, pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2],
				pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			break;
		case 0xc0: // (Together with POINT=B0h, this is used to identify a multi-session disc)
			nTmpLBAExt =
				MSFtoLBA(pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2]);
			nTmpLBA =
				MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			OutputDiscLogA(
				"\tSession %u,  ATIP values, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
				"\t          First Lead-in, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n", 
				pTocData[a].SessionNumber, 
				pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2], 
				nTmpLBAExt, nTmpLBAExt,
				pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2],
				nTmpLBA, nTmpLBA);
			break;
		default:
			nTmpLBA = 
				MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			OutputDiscLogA(
				"\tSession %u,     Track %2u, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n", 
				pTocData[a].SessionNumber, pTocData[a].Point, pTocData[a].Msf[0], 
				pTocData[a].Msf[1], pTocData[a].Msf[2], nTmpLBA, nTmpLBA);
			if (pTocData[a].Point == 1) {
				pDisc->SCSI.lpFirstLBAListOnToc[pTocData[a].Point - 1] = nTmpLBA - 150;
			}
			else if (pTocData[a].Point >= 2 && pTocData[a].Point <= 100) {
				pDisc->SCSI.lpLastLBAListOnToc[pTocData[a].Point - 2] = nTmpLBA - 150 - 1;
				pDisc->SCSI.lpFirstLBAListOnToc[pTocData[a].Point - 1] = nTmpLBA - 150;
			}
			if (pTocData[a].SessionNumber == 2 && bFirst2ndSession) {
				pDisc->SCSI.nFirstLBAof2ndSession = nTmpLBA - 150;
				bFirst2ndSession = FALSE;
			}
			pDisc->SCSI.lpSessionNumList[pTocData[a].Point - 1] = pTocData[a].SessionNumber;
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
	)
{
	BYTE byAlbumCnt = 0, byAlbumIdx = 0;
	BYTE byPerformerCnt = 0, byPerformerIdx = 0;
	BYTE bySongwriterCnt = 0, bySongwriterIdx = 0;
	BYTE byComposerCnt = 0, byComposerIdx = 0;
	BYTE byArrangerCnt = 0, byArrangerIdx = 0;
	BYTE byMessagesCnt = 0, byMessagesIdx = 0;
	BYTE byDiscIdCnt = 0, byDiscIdIdx = 0;
	BYTE byGenreCnt = 0, byGenreIdx = 0;
	BYTE byTocInfoCnt = 0, byTocInfoIdx = 0;
	BYTE byTocInfo2Cnt = 0, byTocInfo2Idx = 0;
	BYTE byUpcEanCnt = 0, byUpcEanIdx = 0;
	BYTE bySizeInfoCnt = 0, bySizeInfoIdx = 0;

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
				byAlbumCnt += bRet;
			}
			byAlbumIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_PERFORMER) {
			if (bRet) {
				byPerformerCnt += bRet;
			}
			byPerformerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_SONGWRITER) {
			if (bRet) {
				bySongwriterCnt += bRet;
			}
			bySongwriterIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_COMPOSER) {
			if (bRet) {
				byComposerCnt += bRet;
			}
			byComposerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_ARRANGER) {
			if (bRet) {
				byArrangerCnt += bRet;
			}
			byArrangerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_MESSAGES) {
			if (bRet) {
				byMessagesCnt += bRet;
			}
			byMessagesIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_DISC_ID) {
			if (bRet) {
				byDiscIdCnt += bRet;
			}
			byDiscIdIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_GENRE) {
			if (bRet) {
				byGenreCnt += bRet;
			}
			byGenreIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_TOC_INFO) {
			byTocInfoCnt++;
			byTocInfoIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_TOC_INFO2) {
			byTocInfo2Cnt++;
			byTocInfo2Idx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_UPC_EAN) {
			if (bRet) {
				byUpcEanCnt += bRet;
			}
			byUpcEanIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_SIZE_INFO) {
			bySizeInfoCnt++;
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
			CHAR tmp[META_CDTEXT_SIZE] = { 0 };
			strncpy(tmp, pTmpText + uiIdx, len1);

			if (byAlbumCnt != 0 && z < byAlbumCnt) {
				strncpy(pDisc->SCSI.pszTitle[nTitleCnt], tmp, strlen(tmp));
				if (nTitleCnt == 0) {
					OutputDiscLogA(
						"\tAlbum Name: %s\n", 
						pDisc->SCSI.pszTitle[nTitleCnt]);
				}
				else {
					OutputDiscLogA(
						"\t Song Name: %s\n", 
						pDisc->SCSI.pszTitle[nTitleCnt]);
				}
				nTitleCnt++;
			}
			else if (byPerformerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt)) {
				strncpy(pDisc->SCSI.pszPerformer[nPerformerCnt], tmp, strlen(tmp));
				if (nPerformerCnt == 0) {
					OutputDiscLogA(
						"\tAlbum Performer: %s\n", 
						pDisc->SCSI.pszPerformer[nPerformerCnt]);
				}
				else {
					OutputDiscLogA(
						"\t Song Performer: %s\n", 
						pDisc->SCSI.pszPerformer[nPerformerCnt]);
				}
				nPerformerCnt++;
			}
			else if (bySongwriterCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt)) {
				strncpy(pDisc->SCSI.pszSongWriter[nSongwriterCnt], tmp, strlen(tmp));
				if (nSongwriterCnt == 0) {
					OutputDiscLogA(
						"\tAlbum SongWriter: %s\n", 
						pDisc->SCSI.pszSongWriter[nSongwriterCnt]);
				}
				else {
					OutputDiscLogA(
						"\t      SongWriter: %s\n", 
						pDisc->SCSI.pszSongWriter[nSongwriterCnt]);
				}
				nSongwriterCnt++;
			}
			else if (byComposerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt + byComposerCnt)) {
				OutputDiscLogA("\tComposer: %s\n", tmp);
			}
			else if (byArrangerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt + byComposerCnt + byArrangerCnt)) {
				OutputDiscLogA("\tArranger: %s\n", tmp);
			}
			else if (byMessagesCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt)) {
				OutputDiscLogA("\tMessages: %s\n", tmp);
			}
			else if (byDiscIdCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt)) {
				OutputDiscLogA("\tDiscId: %s\n", tmp);
			}
			else if (byGenreCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt + byGenreCnt)) {
				OutputDiscLogA("\tGenre: %s\n", tmp);
			}
			else if (byUpcEanCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt +
				byGenreCnt + byTocInfoCnt + byTocInfo2Cnt + byUpcEanCnt)) {
				OutputDiscLogA("\tUpcEan: %s\n", tmp);
			}
		}
		uiIdx += len1 + 1;
	}
	OutputCDTextOther(pDesc, wTocTextEntries, bySizeInfoIdx, bySizeInfoCnt);
}

VOID SetAndOutputTocCDWText(
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	LPCH pTmpText,
	WORD wFirstEntries,
	WORD wTocTextEntries,
	WORD wAllTextSize
	)
{
	BYTE byAlbumCnt = 0, byAlbumIdx = 0;
	BYTE byPerformerCnt = 0, byPerformerIdx = 0;
	BYTE bySongwriterCnt = 0, bySongwriterIdx = 0;
	BYTE byComposerCnt = 0, byComposerIdx = 0;
	BYTE byArrangerCnt = 0, byArrangerIdx = 0;
	BYTE byMessagesCnt = 0, byMessagesIdx = 0;
	BYTE byDiscIdCnt = 0, byDiscIdIdx = 0;
	BYTE byGenreCnt = 0, byGenreIdx = 0;
	BYTE byTocInfoCnt = 0, byTocInfoIdx = 0;
	BYTE byTocInfo2Cnt = 0, byTocInfo2Idx = 0;
	BYTE byUpcEanCnt = 0, byUpcEanIdx = 0;
	BYTE bySizeInfoCnt = 0, bySizeInfoIdx = 0;

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
				byAlbumCnt++;
			}
			byAlbumIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_PERFORMER) {
			if (bRet) {
				byPerformerCnt++;
			}
			byPerformerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_SONGWRITER) {
			if (bRet) {
				bySongwriterCnt++;
			}
			bySongwriterIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_COMPOSER) {
			if (bRet) {
				byComposerCnt++;
			}
			byComposerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_ARRANGER) {
			if (bRet) {
				byArrangerCnt++;
			}
			byArrangerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_MESSAGES) {
			if (bRet) {
				byMessagesCnt++;
			}
			byMessagesIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_DISC_ID) {
			if (bRet) {
				byDiscIdCnt++;
			}
			byDiscIdIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_GENRE) {
			if (bRet) {
				byGenreCnt++;
			}
			byGenreIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_TOC_INFO) {
			byTocInfoCnt++;
			byTocInfoIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_TOC_INFO2) {
			byTocInfo2Cnt++;
			byTocInfo2Idx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_UPC_EAN) {
			if (bRet) {
				byUpcEanCnt++;
			}
			byUpcEanIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_SIZE_INFO) {
			bySizeInfoCnt++;
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
			CHAR tmp[META_CDTEXT_SIZE] = { 0 };
			strncpy(tmp, pTmpText + uiIdx, len1);

			if (byAlbumCnt != 0 && z < byAlbumCnt) {
				if (nTitleCnt == 0) {
					OutputDiscLogA("\tAlbum Name: %s\n", tmp);
				}
				else {
					OutputDiscLogA("\t Song Name: %s\n", tmp);
				}
				nTitleCnt++;
			}
			else if (byPerformerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt)) {
				if (nPerformerCnt == 0) {
					OutputDiscLogA("\tAlbum Performer: %s\n", tmp);
				}
				else {
					OutputDiscLogA("\t Song Performer: %s\n", tmp);
				}
				nPerformerCnt++;
			}
			else if (bySongwriterCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt)) {
				if (nSongwriterCnt == 0) {
					OutputDiscLogA("\tAlbum SongWriter: %s\n", tmp);
				}
				else {
					OutputDiscLogA("\t      SongWriter: %s\n", tmp);
				}
				nSongwriterCnt++;
			}
			else if (byComposerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt + byComposerCnt)) {
				OutputDiscLogA("\tComposer: %s\n", tmp);
			}
			else if (byArrangerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt + byComposerCnt + byArrangerCnt)) {
				OutputDiscLogA("\tArranger: %s\n", tmp);
			}
			else if (byMessagesCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt)) {
				OutputDiscLogA("\tMessages: %s\n", tmp);
			}
			else if (byDiscIdCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt)) {
				OutputDiscLogA("\tDiscId: %s\n", tmp);
			}
			else if (byGenreCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt + byGenreCnt)) {
				OutputDiscLogA("\tGenre: %s\n", tmp);
			}
			else if (byUpcEanCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt +
				byGenreCnt + byTocInfoCnt + byTocInfo2Cnt + byUpcEanCnt)) {
				OutputDiscLogA("\tUpcEan: %s\n", tmp);
			}
		}
		uiIdx += len1 + 1;
	}
	OutputCDTextOther(pDesc, wTocTextEntries, bySizeInfoIdx, bySizeInfoCnt);
}

VOID SetCDOffset(
	BYTE byBe,
	PDISC pDisc,
	INT nStartLBA,
	INT nEndLBA
	)
{
	if (pDisc->MAIN.nCombinedOffset > 0) {
		if (byBe && !pDisc->SCSI.byAudioOnly) {
			pDisc->MAIN.uiMainDataSlideSize = 0;
			pDisc->MAIN.nOffsetStart = 0;
			pDisc->MAIN.nOffsetEnd = 0;
			pDisc->MAIN.nFixStartLBA = 0;
			pDisc->MAIN.nFixEndLBA = pDisc->SCSI.nAllLength;
		}
		else {
			pDisc->MAIN.uiMainDataSlideSize =
				(size_t)pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE;
			pDisc->MAIN.nOffsetStart = 0;
			pDisc->MAIN.nOffsetEnd =
				pDisc->MAIN.nAdjustSectorNum;
			pDisc->MAIN.nFixStartLBA =
				nStartLBA + pDisc->MAIN.nAdjustSectorNum - 1;
			pDisc->MAIN.nFixEndLBA =
				nEndLBA + pDisc->MAIN.nAdjustSectorNum;
		}
		if (pDisc->SCSI.nFirstLBAof2ndSession != -1) {
			pDisc->MAIN.nFixFirstLBAofLeadout =
				pDisc->SCSI.nFirstLBAofLeadout + pDisc->MAIN.nAdjustSectorNum;
			pDisc->MAIN.nFixFirstLBAof2ndSession = 
				pDisc->SCSI.nFirstLBAof2ndSession + pDisc->MAIN.nAdjustSectorNum - 1;
		}
	}
	else if (pDisc->MAIN.nCombinedOffset < 0) {
		if (byBe && !pDisc->SCSI.byAudioOnly) {
			pDisc->MAIN.uiMainDataSlideSize = 0;
			pDisc->MAIN.nOffsetStart = 0;
			pDisc->MAIN.nOffsetEnd = 0;
			pDisc->MAIN.nFixStartLBA = 0;
			pDisc->MAIN.nFixEndLBA = pDisc->SCSI.nAllLength;
		}
		else {
			pDisc->MAIN.uiMainDataSlideSize =
				(size_t)CD_RAW_SECTOR_SIZE + (pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE);
			pDisc->MAIN.nOffsetStart =
				pDisc->MAIN.nAdjustSectorNum;
			pDisc->MAIN.nOffsetEnd = 0;
			pDisc->MAIN.nFixStartLBA =
				nStartLBA + pDisc->MAIN.nAdjustSectorNum;
			pDisc->MAIN.nFixEndLBA =
				nEndLBA + pDisc->MAIN.nAdjustSectorNum + 1;
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
		pDisc->MAIN.nFixStartLBA = 0;
		pDisc->MAIN.nFixEndLBA = pDisc->SCSI.nAllLength;
	}
}

VOID SetTrackAttribution(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nLBA,
	LPBYTE lpCurrentTrackNum,
	PMAIN_HEADER pMain,
	PSUB_Q pSubQ
	)
{
	if (0 <= nLBA && nLBA < pDisc->SCSI.nAllLength &&
		0 < pSubQ->present.byTrackNum && pSubQ->present.byTrackNum <= *lpCurrentTrackNum + 1) {
		INT tIdx = *lpCurrentTrackNum - 1;
		// preserve the 1st LBA of the changed trackNum
		if (pSubQ->prev.byTrackNum + 1 == pSubQ->present.byTrackNum) {
			*lpCurrentTrackNum = pSubQ->present.byTrackNum;
			tIdx = *lpCurrentTrackNum - 1;
			if (pSubQ->present.byIndex > 0) {
				if (pDisc->SCSI.nFirstLBAof2ndSession == -1 &&
					pSubQ->present.byIndex == 1 &&
					nLBA != pDisc->SCSI.lpFirstLBAListOnToc[tIdx]) {
					OutputSubInfoWithLBALogA(
						"Subchannel & TOC doesn't sync. LBA on TOC[%d, %#x], index[%02u]\n",
						nLBA, pSubQ->present.byTrackNum, pDisc->SCSI.lpFirstLBAListOnToc[tIdx],
						pDisc->SCSI.lpFirstLBAListOnToc[tIdx], pSubQ->present.byIndex);
					pDisc->SUB.lpFirstLBAListOnSub[tIdx][1] = pDisc->SCSI.lpFirstLBAListOnToc[tIdx];
					if (pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSub[tIdx][1]) {
						pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] = -1;
					}
					pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][1] = nLBA;
					if (pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][1]) {
						pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] = -1;
					}
					pDisc->SUB.byDesync = TRUE;
					OutputSubInfoWithLBALogA("TrackNum is changed [L:%d]\n", nLBA, pSubQ->present.byTrackNum, __LINE__);
				}
				else {
					pDisc->SUB.lpFirstLBAListOnSub[tIdx][pSubQ->present.byIndex] = nLBA;
					pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][pSubQ->present.byIndex] = nLBA;
					OutputSubInfoWithLBALogA("TrackNum is changed [L:%d]\n", nLBA, pSubQ->present.byTrackNum, __LINE__);
				}
			}
			// preserve last LBA per data track
			if (pSubQ->prev.byTrackNum > 0) {
				if (pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[pSubQ->prev.byTrackNum - 1] != -1 &&
					pDisc->SUB.lpLastLBAListOfDataTrackOnSub[pSubQ->prev.byTrackNum - 1] == -1 &&
					(pSubQ->prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					pDisc->SUB.lpLastLBAListOfDataTrackOnSub[pSubQ->prev.byTrackNum - 1] = nLBA - 1;
					OutputSubInfoWithLBALogA("TrackNum is changed [L:%d]\n", nLBA, pSubQ->present.byTrackNum, __LINE__);
				}
			}
		}
		// preserve mode, ctl
		if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[tIdx]) {
			pDisc->SUB.lpCtlList[tIdx] = pSubQ->present.byCtl;
			pDisc->MAIN.lpModeList[tIdx] = GetMode((LPBYTE)pMain->present, pMain->prev[15], pSubQ->present.byCtl, unscrambled);
		}
		// preserve the 1st LBA of the changed index 
		if (pSubQ->prev.byIndex + 1 == pSubQ->present.byIndex && *lpCurrentTrackNum >= 0) {
			if (pSubQ->present.byIndex != 1) {
				pDisc->SUB.lpFirstLBAListOnSub[tIdx][pSubQ->present.byIndex] = nLBA;
				pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][pSubQ->present.byIndex] = nLBA;
				OutputSubInfoWithLBALogA("Index is changed to [%02d][L:%d]\n"
					, nLBA, pSubQ->present.byTrackNum, pSubQ->present.byIndex, __LINE__);
			}
			else {
				if (nLBA != pDisc->SCSI.lpFirstLBAListOnToc[tIdx]) {
					OutputSubInfoWithLBALogA(
						"Subchannel & TOC doesn't sync. LBA on TOC[%d, %#x], prevIndex[%02u]\n",
						nLBA, pSubQ->present.byTrackNum, pDisc->SCSI.lpFirstLBAListOnToc[tIdx],
						pDisc->SCSI.lpFirstLBAListOnToc[tIdx], pSubQ->prev.byIndex);
				}
				pDisc->SUB.lpFirstLBAListOnSub[tIdx][1] = pDisc->SCSI.lpFirstLBAListOnToc[tIdx];
				if (pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSub[tIdx][1]) {
					// Crow, The - Original Motion Picture Soundtrack (82519-2)
					// LBA 108975, Track[06], Subchannel & TOC isn't sync. LBA on TOC: 108972, prevIndex[00]
					pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] = -1;
				}
				pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][pSubQ->present.byIndex] = nLBA;
				if (pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][1]) {
					pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] = -1;
				}
				OutputSubInfoWithLBALogA("Index is changed to [%02d][L:%d]\n"
					, nLBA, pSubQ->present.byTrackNum, pSubQ->present.byIndex, __LINE__);
			}
		}
		else if (pSubQ->prev.byIndex >= 1 && pSubQ->present.byIndex == 0) {
			pDisc->SUB.lpFirstLBAListOnSub[tIdx][pSubQ->present.byIndex] = nLBA;
			pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][pSubQ->present.byIndex] = nLBA;
			OutputSubInfoWithLBALogA("Index is changed to [%02d][L:%d]\n"
				, nLBA, pSubQ->present.byTrackNum, pSubQ->present.byIndex, __LINE__);
		}

		if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
			(pSubQ->present.byCtl & AUDIO_DATA_TRACK) == 0) {
			OutputMainInfoWithLBALogA(
				"Data track, but this sector is audio\n", nLBA, pSubQ->present.byTrackNum);
		}
		else if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == 0 &&
			(pSubQ->present.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			OutputMainInfoWithLBALogA(
				"Audio track, but this sector is data\n", nLBA, pSubQ->present.byTrackNum);
		}

		if (pExtArg->byReverse) {
			// preserve last LBA per data track
			if (nLBA == pDisc->SCSI.nLastLBAofDataTrack) {
				if (pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] == -1 &&
					(pSubQ->present.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
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
				pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[tIdx] = nLBA;
				OutputSubInfoWithLBALogA("1st LBA of this track [L:%d]\n", nLBA, pSubQ->present.byTrackNum, __LINE__);
			}
			// preserve last LBA per data track
			else if (nLBA == pDisc->SCSI.nAllLength - 1) {
				if (pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] == -1) {
					if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
						pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] = pDisc->SCSI.nAllLength - 1;
						OutputSubInfoWithLBALogA("Last LBA of this disc [L:%d]\n", nLBA, pSubQ->present.byTrackNum, __LINE__);
					}
				}
			}
		}
	}
	else if (pSubQ->present.byTrackNum == 110) {
		pDisc->SUB.lpLastLBAListOfDataTrackOnSub[*lpCurrentTrackNum - 1] = nLBA;
	}
}

VOID SetISRCToString(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPSTR pszOutString,
	BYTE byIdxOfTrack,
	BOOL bCopy
	)
{
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
	_snprintf(pszOutString, META_ISRC_SIZE - 1, "%c%c%c%c%c%c%c%c%c%c%c%c",
		((lpSubcode[13] >> 2) & 0x3f) + 0x30, 
		(((lpSubcode[13] << 4) & 0x30) | ((lpSubcode[14] >> 4) & 0x0f)) + 0x30, 
		(((lpSubcode[14] << 2) & 0x3c) | ((lpSubcode[15] >> 6) & 0x03)) + 0x30, 
		(lpSubcode[15] & 0x3f) + 0x30, 
		((lpSubcode[16] >> 2) & 0x3f) + 0x30, 
		((lpSubcode[17] >> 4) & 0x0f) + 0x30, (lpSubcode[17] & 0x0f) + 0x30, 
		((lpSubcode[18] >> 4) & 0x0f) + 0x30, (lpSubcode[18] & 0x0f) + 0x30, 
		((lpSubcode[19] >> 4) & 0x0f) + 0x30, (lpSubcode[19] & 0x0f) + 0x30,
		((lpSubcode[20] >> 4) & 0x0f) + 0x30);
	pszOutString[META_ISRC_SIZE - 1] = 0;
	if (bCopy) {
		strncpy(pDisc->SUB.pszISRC[byIdxOfTrack], pszOutString, META_ISRC_SIZE);
	}
}

VOID SetMCNToString(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPSTR pszOutString,
	BOOL bCopy
	)
{
	_snprintf(pszOutString, META_CATALOG_SIZE - 1, "%c%c%c%c%c%c%c%c%c%c%c%c%c",
		((lpSubcode[13] >> 4) & 0x0f) + 0x30, (lpSubcode[13] & 0x0f) + 0x30, 
		((lpSubcode[14] >> 4) & 0x0f) + 0x30, (lpSubcode[14] & 0x0f) + 0x30, 
		((lpSubcode[15] >> 4) & 0x0f) + 0x30, (lpSubcode[15] & 0x0f) + 0x30, 
		((lpSubcode[16] >> 4) & 0x0f) + 0x30, (lpSubcode[16] & 0x0f) + 0x30, 
		((lpSubcode[17] >> 4) & 0x0f) + 0x30, (lpSubcode[17] & 0x0f) + 0x30, 
		((lpSubcode[18] >> 4) & 0x0f) + 0x30, (lpSubcode[18] & 0x0f) + 0x30, 
		((lpSubcode[19] >> 4) & 0x0f) + 0x30);
	pszOutString[META_CATALOG_SIZE - 1] = 0;
	if (bCopy) {
		strncpy(pDisc->SUB.szCatalog, pszOutString, sizeof(pDisc->SUB.szCatalog));
	}
}

VOID SetLBAForFirstAdr(
	INT nFirstLBA[][2],
	INT nRangeLBA[][2],
	LPSTR strAdr,
	LPINT nAdrLBAList,
	BYTE byIdxOfSession,
	BYTE byPlxtrDrive
	)
{
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
	SUB_Q_PER_SECTOR subQ,
	LPBYTE lpSubcode,
	BYTE byPresent
	)
{
	lpSubcode[12] = BYTE(subQ.byCtl << 4 | subQ.byAdr);
	lpSubcode[13] = DecToBcd(subQ.byTrackNum);
	lpSubcode[14] = DecToBcd(subQ.byIndex);
	BYTE m, s, f;
	if (byPresent) {
		LBAtoMSF(subQ.nRelativeTime, &m, &s, &f);
	}
	else {
		LBAtoMSF(subQ.nRelativeTime + 1, &m, &s, &f);
	}
	lpSubcode[15] = DecToBcd(m);
	lpSubcode[16] = DecToBcd(s);
	lpSubcode[17] = DecToBcd(f);
	if (byPresent) {
		LBAtoMSF(subQ.nAbsoluteTime, &m, &s, &f);
	}
	else {
		LBAtoMSF(subQ.nAbsoluteTime + 1, &m, &s, &f);
	}
	lpSubcode[19] = DecToBcd(m);
	lpSubcode[20] = DecToBcd(s);
	lpSubcode[21] = DecToBcd(f);
	WORD crc16 = (WORD)GetCrc16CCITT(10, &lpSubcode[12]);
	lpSubcode[22] = HIBYTE(crc16);
	lpSubcode[23] = LOBYTE(crc16);
}

VOID SetBufferFromMCN(
	PDISC pDisc,
	LPBYTE lpSubcode
	)
{
	for (INT i = 13, j = 0; i < 19; i++, j += 2) {
		lpSubcode[i] = (BYTE)(pDisc->SUB.szCatalog[j] - 0x30);
		lpSubcode[i] <<= 4;
		lpSubcode[i] |= (BYTE)(pDisc->SUB.szCatalog[j + 1] - 0x30);
	}
	lpSubcode[19] = (BYTE)(pDisc->SUB.szCatalog[12] - 0x30);
	lpSubcode[19] <<= 4;
	lpSubcode[20] = 0;
}

VOID SetTmpSubQDataFromBuffer(
	PSUB_Q_PER_SECTOR pSubQ,
	LPBYTE lpSubcode
	)
{
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
	PSUB_Q pSubQ,
	INT nLBA,
	BYTE byCurrentTrackNum
	)
{
	// Subchannel pattern on MCN Sector
	// pattern 1: pregap sector.
	if (IsValidPregapSector(pDisc, pSubQ, nLBA)) {
		BOOL bValidPre = FALSE;
		// pattern 1-1: prev sector is audio.
		if ((pSubQ->prev.byCtl & AUDIO_DATA_TRACK) == 0) {
			// pattern 1-1-1: present sector is audio.
			if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == 0) {
				if (pSubQ->prev.byTrackNum > 0) {
					if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 225) {
						// Atlas, The - Renaissance Voyager (Japan)
						// LBA[003364, 0x00d24], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:44:64], AMSF[00:46:64], RtoW[0, 0, 0, 0]
						// LBA[003365, 0x00d25], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :65], RtoW[0, 0, 0, 0]
						// LBA[003366, 0x00d26], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:02:73], AMSF[00:46:66], RtoW[0, 0, 0, 0]
						// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
						// LBA[261585, 0x3fDD1], Audio, 2ch, Copy NG, Pre-emphasis No, Track[95], Idx[01], RMSF[00:13:69], AMSF[58:09:60], RtoW[0, 0, 0, 0]
						// LBA[261586, 0x3fDD2], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :61], RtoW[0, 0, 0, 0]
						// LBA[261587, 0x3fDD3], Audio, 2ch, Copy NG, Pre-emphasis No, Track[96], Idx[00], RMSF[00:02:73], AMSF[58:09:62], RtoW[0, 0, 0, 0]
						pSubQ->present.nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 150) {
						pSubQ->present.nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 149) {
						pSubQ->present.nRelativeTime = 148;
						bValidPre = TRUE;
					}
				}
				if (bValidPre && pExtArg->byMCN) {
					// pattern 1-1-1-1: change track.
					pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
					// pattern 1-1-1-1-1: change index. (pregap sector is 0)
					pSubQ->present.byIndex = 0;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
				}
				else {
					// pattern 1-1-1-2: not change track.
					pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
					// pattern 1-1-1-2-2: not change index.
					pSubQ->present.byIndex = pSubQ->prev.byIndex;
				}
			}
			// pattern 1-1-2: present sector is data.
			else if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				if (pSubQ->prev.byTrackNum > 0) {
					if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 225) {
						pSubQ->present.nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 150) {
						pSubQ->present.nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 149) {
						pSubQ->present.nRelativeTime = 148;
						bValidPre = TRUE;
					}
				}
				if (bValidPre && pExtArg->byMCN) {
					// pattern 1-1-2-1: change track.
					pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
					// pattern 1-1-2-1-1: change index. (pregap sector is 0)
					pSubQ->present.byIndex = 0;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
				}
				else {
					// pattern 1-1-2-2: not change track.
					pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
					// pattern 1-1-2-2-2: not change index.
					pSubQ->present.byIndex = pSubQ->prev.byIndex;
				}
			}
		}
		// pattern 1-2: prev sector is data.
		else if ((pSubQ->prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			// pattern 1-2-1: present sector is audio.
			if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == 0) {
				if (pSubQ->prev.byTrackNum > 0) {
					if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 225) {
						pSubQ->present.nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 150) {
						// Valis IV (Japan)
						// LBA[157830, 0x26886],  Data,      Copy NG,                  Track[44], Idx[01], RMSF[00:06:27], AMSF[35:06:30], RtoW[0, 0, 0, 0]
						// LBA[157831, 0x26887], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :31], RtoW[0, 0, 0, 0]
						// LBA[157832, 0x26888], Audio, 2ch, Copy NG, Pre-emphasis No, Track[45], Idx[00], RMSF[00:01:73], AMSF[35:06:32], RtoW[0, 0, 0, 0]
						// Cosmic Fantasy 2
						// LBA[202749, 0x317FD],  Data,      Copy NG,                  Track[80], Idx[01], RMSF[00:06:63], AMSF[45:05:24], RtoW[0, 0, 0, 0]
						// LBA[202750, 0x317FE], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :25], RtoW[0, 0, 0, 0]
						// LBA[202751, 0x317FF], Audio, 2ch, Copy NG, Pre-emphasis No, Track[81], Idx[00], RMSF[00:01:73], AMSF[45:05:26], RtoW[0, 0, 0, 0]
						pSubQ->present.nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 149) {
						pSubQ->present.nRelativeTime = 148;
						bValidPre = TRUE;
					}
				}
				if (bValidPre && pExtArg->byMCN) {
					// pattern 1-2-1-1: change track.
					pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
					// pattern 1-2-1-1-1: change index. (pregap sector is 0)
					pSubQ->present.byIndex = 0;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
				}
				else {
					// pattern 1-2-1-2: not change track.
					pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
					// pattern 1-2-1-2-2: not change index.
					pSubQ->present.byIndex = pSubQ->prev.byIndex;
				}
			}
			// pattern 1-2-2: present sector is data.
			else if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				if (pSubQ->prev.byTrackNum > 0) {
					if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 225) {
						pSubQ->present.nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 150) {
						pSubQ->present.nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 149) {
						pSubQ->present.nRelativeTime = 148;
						bValidPre = TRUE;
					}
				}
				if (bValidPre && pExtArg->byMCN) {
					// pattern 1-2-2-1: change track.
					pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
					// pattern 1-2-2-1-1: change index. (pregap sector is 0)
					pSubQ->present.byIndex = 0;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
				}
				else {
					// pattern 1-2-2-2: not change track.
					pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
					// pattern 1-2-2-2-2: not change index.
					pSubQ->present.byIndex = pSubQ->prev.byIndex;
				}
			}
		}
	}
	// pattern 2: not pregap sector.
	else {
		// pattern 2-1: prev sector is audio.
		if ((pSubQ->prev.byCtl & AUDIO_DATA_TRACK) == 0) {
			// pattern 2-1-1: present sector is audio.
			if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == 0) {
				// 1st sector of tracks
				if (pSubQ->prev.byTrackNum > 0 &&
					nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum]) {
					// Madou Monogatari I - Honoo no Sotsuenji (Japan)
					// LBA[183031, 0x2CAF7], Audio, 2ch, Copy NG, Pre-emphasis No, Track[21], Idx[01], RMSF[00:31:70], AMSF[40:42:31], RtoW[0, 0, 0, 0]
					// LBA[183032, 0x2CAF8], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :32], RtoW[0, 0, 0, 0]
					// LBA[183033, 0x2CAF9], Audio, 2ch, Copy NG, Pre-emphasis No, Track[22], Idx[01], RMSF[00:00:01], AMSF[40:42:33], RtoW[0, 0, 0, 0]
					pSubQ->present.nRelativeTime = 0;
					// pattern 2-1-1-1: change track.
					pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
					// pattern 2-1-1-1-2: not change index.
					pSubQ->present.byIndex = pSubQ->prev.byIndex;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
				}
				// 1st next index of same tracks
				else if (pSubQ->prev.byIndex == 0 && pSubQ->prev.nRelativeTime == 0) {
					// Psychic Detective Series Vol. 5 - Nightmare (Japan)
					// LBA[080999, 0x13C67], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:00:00], AMSF[18:01:74], RtoW[0, 0, 0, 0]
					// LBA[081000, 0x13C68], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [3010911111863], AMSF[     :00], RtoW[0, 0, 0, 0]
					// LBA[081001, 0x13C69], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[01], RMSF[00:00:01], AMSF[18:02:01], RtoW[0, 0, 0, 0]
					pSubQ->present.nRelativeTime = 0;
					// pattern 2-1-1-2: not change track.
					pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
					// pattern 2-1-1-2-1: change index.
					pSubQ->present.byIndex = (BYTE)(pSubQ->prev.byIndex + 1);
				}
				// 1st index of same tracks
				else if (pSubQ->prev.byIndex > 1 && pSubQ->prev.byIndex != pSubQ->next.byIndex) {
					if (pSubQ->prev.byIndex + 1 == pSubQ->next.byIndex) {
						// Space Jam (Japan)
						// LBA[056262, 0x0dbc6], Audio, 2ch, Copy NG, Pre-emphasis No, Track[04], Idx[53], RMSF[01:38:65], AMSF[12:32:12], RtoW[0, 0, 0, 0]
						// LBA[056263, 0x0dbc7], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :13], RtoW[0, 0, 0, 0]
						// LBA[056264, 0x0dbc8], Audio, 2ch, Copy NG, Pre-emphasis No, Track[04], Idx[54], RMSF[01:38:67], AMSF[12:32:14], RtoW[0, 0, 0, 0]
						// Space Jam (Japan)
						// LBA[086838, 0x15336], Audio, 2ch, Copy NG, Pre-emphasis No, Track[06], Idx[82], RMSF[02:31:05], AMSF[19:19:63], RtoW[0, 0, 0, 0]
						// LBA[086839, 0x15337], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :64], RtoW[0, 0, 0, 0]
						// LBA[086840, 0x15338], Audio, 2ch, Copy NG, Pre-emphasis No, Track[06], Idx[83], RMSF[02:31:07], AMSF[19:19:65], RtoW[0, 0, 0, 0]
						pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime + 1;
						// pattern 2-1-1-2: not change track.
						pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
						// pattern 2-1-1-2-1: change index.
						pSubQ->present.byIndex = (BYTE)(pSubQ->prev.byIndex + 1);
					}
				}
				// same index of same tracks
				else {
					if (pSubQ->prev.byIndex == 0) {
						// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
						// LBA[003413, 0x00D55], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:02:26], AMSF[00:47:38], RtoW[0, 0, 0, 0]
						// LBA[003414, 0x00D56], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :39], RtoW[0, 0, 0, 0]
						// LBA[003415, 0x00D57], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:02:24], AMSF[00:47:40], RtoW[0, 0, 0, 0]
						pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime - 1;
					}
					else if (pSubQ->prev.byIndex > 0) {
						// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
						// LBA[081541, 0x13E85], Audio, 2ch, Copy NG, Pre-emphasis No, Track[18], Idx[01], RMSF[00:12:57], AMSF[18:09:16], RtoW[0, 0, 0, 0]
						// LBA[081542, 0x13E86], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :17], RtoW[0, 0, 0, 0]
						// LBA[081543, 0x13E87], Audio, 2ch, Copy NG, Pre-emphasis No, Track[18], Idx[01], RMSF[00:12:59], AMSF[18:09:18], RtoW[0, 0, 0, 0]
						pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime + 1;
					}
					// pattern 2-1-1-2: not change track.
					pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
					// pattern 2-1-1-2-2: not change index.
					pSubQ->present.byIndex = pSubQ->prev.byIndex;
				}
			}
			// pattern 2-1-2: present sector is data.
			else if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				// 1st sector of tracks
				if (pSubQ->prev.byTrackNum > 0 &&
					nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum]) {
					pSubQ->present.nRelativeTime = 0;
					// pattern 2-1-2-1: change track.
					pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
					// pattern 2-1-2-1-2: not change index.
					pSubQ->present.byIndex = pSubQ->prev.byIndex;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
				}
				// 1st next index of same tracks
				else if (pSubQ->prev.byIndex == 0 && pSubQ->prev.nRelativeTime == 0) {
					pSubQ->present.nRelativeTime = 0;
					// pattern 2-1-2-2: not change track.
					pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
					// pattern 2-1-2-2-1: change index.
					pSubQ->present.byIndex = (BYTE)(pSubQ->prev.byIndex + 1);
				}
				// same index of same tracks
				else {
					if (pSubQ->prev.byIndex == 0) {
						pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime - 1;
					}
					else if (pSubQ->prev.byIndex > 0) {
						pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime + 1;
					}
					// pattern 2-1-2-2: not change track.
					pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
					// pattern 2-1-2-2-2: not change index.
					pSubQ->present.byIndex = pSubQ->prev.byIndex;
				}
			}
		}
		// pattern 2-2: prev sector is data.
		else if ((pSubQ->prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			// pattern 2-2-1: present sector is audio.
			if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == 0) {
				// 1st sector of tracks
				if (pSubQ->prev.byTrackNum > 0 &&
					nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum]) {
					pSubQ->present.nRelativeTime = 0;
					// pattern 2-2-1-1: change track.
					pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
					// pattern 2-2-1-1-2: not change index.
					pSubQ->present.byIndex = pSubQ->prev.byIndex;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
				}
				// 1st next index of same tracks
				else if (pSubQ->prev.byIndex == 0 && pSubQ->prev.nRelativeTime == 0) {
					pSubQ->present.nRelativeTime = 0;
					// pattern 2-2-1-2: not change track.
					pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
					// pattern 2-2-1-2-1: change index.
					pSubQ->present.byIndex = (BYTE)(pSubQ->prev.byIndex + 1);
				}
				// same index of same tracks
				else {
					if (pSubQ->prev.byIndex == 0) {
						pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime - 1;
					}
					else if (pSubQ->prev.byIndex > 0) {
						// EVE - burst error (Disc 3) (Terror Disc)
						// LBA[188021, 0x2DE75],  Data,      Copy NG,                  Track[01], Idx[01], RMSF[41:46:71], AMSF[41:48:71], RtoW[0, 0, 0, 0]
						// LBA[188022, 0x2DE76], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :72], RtoW[0, 0, 0, 0]
						// LBA[188023, 0x2DE77],  Data,      Copy NG,                  Track[01], Idx[01], RMSF[41:46:73], AMSF[41:48:73], RtoW[0, 0, 0, 0]
						pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime + 1;
					}
					// pattern 2-2-1-2: not change track.
					pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
					// pattern 2-2-1-2-2: not change index.
					pSubQ->present.byIndex = pSubQ->prev.byIndex;
				}
			}
			// pattern 2-2-2: present sector is data.
			else if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				// 1st sector of tracks
				if (pSubQ->prev.byTrackNum > 0 &&
					nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum]) {
					pSubQ->present.nRelativeTime = 0;
					// pattern 2-2-2-1: change track.
					pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
					// pattern 2-2-2-1-2: not change index.
					pSubQ->present.byIndex = pSubQ->prev.byIndex;
					OutputMainInfoWithLBALogA(
						"The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
						nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
				}
				// 1st next index of same tracks
				else if (pSubQ->prev.byIndex == 0 && pSubQ->prev.nRelativeTime == 0) {
					// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
					// LBA[142873, 0x22E19], Data, Copy NG, Track[37], Idx[00], RMSF[00:00:00], AMSF[31:46:73], RtoW[0, 0, 0, 0]
					// LBA[142874, 0x22E1A], Data, Copy NG, MediaCatalogNumber [0000000000000], AMSF[     :74], RtoW[0, 0, 0, 0]
					// LBA[142875, 0x22E1B], Data, Copy NG, Track[37], Idx[01], RMSF[00:00:01], AMSF[31:47:00], RtoW[0, 0, 0, 0]
					pSubQ->present.nRelativeTime = 0;
					// pattern 2-2-2-2: not change track.
					pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
					// pattern 2-2-2-2-1: change index.
					pSubQ->present.byIndex = (BYTE)(pSubQ->prev.byIndex + 1);
				}
				// same index of same tracks
				else {
					if (pSubQ->prev.byIndex == 0) {
						pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime - 1;
					}
					else if (pSubQ->prev.byIndex > 0) {
						// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
						// LBA[174261, 0x2A8B5], Data, Copy NG, Track[60], Idx[01], RMSF[00:06:19], AMSF[38:45:36], RtoW[0, 0, 0, 0]
						// LBA[174262, 0x2A8B6], Data, Copy NG, MediaCatalogNumber [0000000000000], AMSF[     :37], RtoW[0, 0, 0, 0]
						// LBA[174263, 0x2A8B7], Data, Copy NG, Track[60], Idx[01], RMSF[00:06:21], AMSF[38:45:38], RtoW[0, 0, 0, 0]
						pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime + 1;
					}
					// pattern 2-2-2-2: not change track.
					pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
					// pattern 2-2-2-2-2: not change index.
					pSubQ->present.byIndex = pSubQ->prev.byIndex;
				}
			}
		}
	}
}

VOID UpdateTmpSubQDataForISRC(
	PSUB_Q pSubQ
	)
{
	pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
	pSubQ->present.byIndex = pSubQ->prev.byIndex;
	if (pSubQ->present.byIndex == 0) {
		pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime - 1;
	}
	else if (pSubQ->present.byIndex > 0) {
		pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime + 1;
	}
}

VOID UpdateTmpSubQData(
	PSUB_Q pSubQ,
	BOOL bLibCrypt,
	BOOL bSecuRom
	)
{
	// TODO: Doesn't need?
	if (pSubQ->prev.byIndex == 0 && pSubQ->prev.nRelativeTime == 0) {
		pSubQ->prev.byIndex = 1;
	}
	pSubQ->prevPrev.byCtl = pSubQ->prev.byCtl;
	if (pSubQ->prev.byAdr != ADR_ENCODES_MEDIA_CATALOG &&
		pSubQ->prev.byAdr != ADR_ENCODES_ISRC) {
		pSubQ->prevPrev.byAdr = pSubQ->prev.byAdr;
		pSubQ->prevPrev.nRelativeTime = pSubQ->prev.nRelativeTime;
	}
	pSubQ->prevPrev.byTrackNum = pSubQ->prev.byTrackNum;
	pSubQ->prevPrev.byIndex = pSubQ->prev.byIndex;
	pSubQ->prevPrev.nAbsoluteTime = pSubQ->prev.nAbsoluteTime;

	pSubQ->prev.byCtl = pSubQ->present.byCtl;
	pSubQ->prev.byAdr = pSubQ->present.byAdr;
	pSubQ->prev.byTrackNum = pSubQ->present.byTrackNum;
	pSubQ->prev.byIndex = pSubQ->present.byIndex;
	if (bLibCrypt || bSecuRom) {
		pSubQ->prev.nRelativeTime++;
	}
	else {
		pSubQ->prev.nRelativeTime = pSubQ->present.nRelativeTime;
	}
	pSubQ->prev.nAbsoluteTime++;
}

VOID UpdateTmpMainHeader(
	PMAIN_HEADER pMain,
	LPBYTE lpBuf,
	BYTE byCtl,
	INT nMainDataType
	)
{
	memcpy(pMain->prev, pMain->present, MAINHEADER_MODE1_SIZE);
	BYTE tmp = (BYTE)(pMain->present[14] + 1);
	if ((tmp & 0x0f) == 0x0a) {
		tmp += 6;
	}
	if (tmp == 0x75) {
		pMain->present[14] = 0;
		if (nMainDataType == scrambled) {
			tmp = (BYTE)((pMain->present[13] ^ 0x80) + 1);
		}
		else {
			tmp = (BYTE)(pMain->present[13] + 1);
		}
		if ((tmp & 0x0f) == 0x0a) {
			tmp += 6;
		}
		if (tmp == 0x60) {
			if (nMainDataType == scrambled) {
				pMain->present[13] = 0x80;
				tmp = (BYTE)((pMain->present[12] ^ 0x01) + 1);
			}
			else {
				pMain->present[13] = 0;
				tmp = (BYTE)(pMain->present[12] + 1);
			}
			if ((tmp & 0x0f) == 0x0a) {
				tmp += 6;
			}
			if (nMainDataType == scrambled) {
				pMain->present[12] = (BYTE)(tmp ^ 0x01);
			}
			else {
				pMain->present[12] = tmp;
			}
		}
		else {
			if (nMainDataType == scrambled) {
				pMain->present[13] = (BYTE)(tmp ^ 0x80);
			}
			else {
				pMain->present[13] = tmp;
			}
		}
	}
	else {
		pMain->present[14] = tmp;
	}
	pMain->present[15] = GetMode(lpBuf, pMain->prev[15], byCtl, nMainDataType);
}

VOID SetC2ErrorDataDetail(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	INT nLBA,
	PUINT puiC2ErrorLBACnt
	)
{
	BOOL bSame = FALSE;
	for (UINT i = 0; i < *puiC2ErrorLBACnt; i++) {
		if (pC2ErrorPerSector[i].nErrorLBANum == nLBA) {
			bSame = TRUE;
		}
	}
	if (!bSame) {
		pC2ErrorPerSector[*puiC2ErrorLBACnt].byErrorFlag = RETURNED_EXIST_C2_ERROR;
		pC2ErrorPerSector[*puiC2ErrorLBACnt].nErrorLBANum = nLBA;
		(*puiC2ErrorLBACnt)++;
	}
}

VOID SetC2ErrorData(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	INT nLBA,
	PUINT puiC2ErrorLBACnt,
	BOOL b1stRead
	)
{
	INT nBytePos = nLBA * CD_RAW_SECTOR_SIZE;
	INT nBytePosEnd = nBytePos + CD_RAW_SECTOR_SIZE - 1;
#ifdef _DEBUG
	OutputC2ErrorWithLBALogA(
		"BytePos[%d-%d, %#x-%#x] C2 error exists. Error byte num: %u\n"
		"                      [ErrorOfs:BytePos(dec), ErrorOfs:BytePos(hex)]"
		, nLBA, nBytePos, nBytePosEnd, nBytePos, nBytePosEnd,
		pC2ErrorPerSector[*puiC2ErrorLBACnt].uiErrorBytePosCnt);
	UINT n = 0;
	for (; n < pC2ErrorPerSector[*puiC2ErrorLBACnt].uiErrorBytePosCnt; n++) {
		INT nPos = nBytePos + pC2ErrorPerSector[*puiC2ErrorLBACnt].lpErrorBytePos[n];
		OutputC2ErrorLogA(", [%d:%d, %#x:%#x]",
			pC2ErrorPerSector[*puiC2ErrorLBACnt].lpErrorBytePos[n], nPos,
			pC2ErrorPerSector[*puiC2ErrorLBACnt].lpErrorBytePos[n], nPos);
	}
	OutputC2ErrorLogA("\n");
#else
	OutputC2ErrorWithLBALogA(
		"BytePos[%d-%d, %#x-%#x] C2 error exists. Error byte num: %u\n"
		, nLBA, nBytePos, nBytePosEnd, nBytePos, nBytePosEnd,
		pC2ErrorPerSector[*puiC2ErrorLBACnt].uiErrorBytePosCnt);
#endif
	if (b1stRead) {
		SetC2ErrorDataDetail(pC2ErrorPerSector, nLBA - 1, puiC2ErrorLBACnt);
		SetC2ErrorDataDetail(pC2ErrorPerSector, nLBA, puiC2ErrorLBACnt);
	}
	else {
		SetC2ErrorDataDetail(pC2ErrorPerSector, nLBA, puiC2ErrorLBACnt);
	}
}

VOID SetNoC2ErrorData(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwAllBufLen,
	PUINT puiC2ErrorLBACnt
	)
{
	OutputC2ErrorLogA(STR_LBA
		"C2 error doesn't exist. Next check 2352 byte.\n", nLBA, nLBA);
	memcpy(pC2ErrorPerSector[*puiC2ErrorLBACnt].lpBufNoC2Sector, lpBuf, dwAllBufLen);
	pC2ErrorPerSector[*puiC2ErrorLBACnt].byErrorFlag = RETURNED_NO_C2_ERROR_1ST;
	pC2ErrorPerSector[*puiC2ErrorLBACnt].nErrorLBANum = nLBA;
	(*puiC2ErrorLBACnt)++;
}

VOID SetNoC2ErrorExistsByteErrorData(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	INT nLBA,
	PUINT puiC2ErrorLBACnt
	)
{
	OutputC2ErrorLogA(STR_LBA
		"C2 error doesn't exist. But byte doesn't match\n", nLBA, nLBA);
	pC2ErrorPerSector[*puiC2ErrorLBACnt].byErrorFlag = RETURNED_NO_C2_ERROR_BUT_BYTE_ERROR;
	pC2ErrorPerSector[*puiC2ErrorLBACnt].nErrorLBANum = nLBA;
	(*puiC2ErrorLBACnt)++;
}

VOID SetC2ErrorBackup(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	UINT uiC2ErrorLBACntBackup,
	DWORD dwAllBufLen
	)
{
	for (UINT c = 0; c < uiC2ErrorLBACntBackup; c++) {
		pC2ErrorPerSector[c].byErrorFlagBackup = pC2ErrorPerSector[c].byErrorFlag;
		pC2ErrorPerSector[c].byErrorFlag = RETURNED_NO_C2_ERROR_1ST;
		pC2ErrorPerSector[c].nErrorLBANumBackup = pC2ErrorPerSector[c].nErrorLBANum;
		pC2ErrorPerSector[c].nErrorLBANum = 0;
		for (UINT d = 0; d < pC2ErrorPerSector[c].uiErrorBytePosCnt; d++) {
			pC2ErrorPerSector[c].lpErrorBytePosBackup[d] = pC2ErrorPerSector[c].lpErrorBytePos[d];
			pC2ErrorPerSector[c].lpErrorBytePos[d] = 0;
		}
		pC2ErrorPerSector[c].uiErrorBytePosCntBackup = pC2ErrorPerSector[c].uiErrorBytePosCnt;
		pC2ErrorPerSector[c].uiErrorBytePosCnt = 0;
		memcpy(pC2ErrorPerSector[c].lpBufNoC2SectorBackup,
			pC2ErrorPerSector[c].lpBufNoC2Sector, dwAllBufLen);
		ZeroMemory(pC2ErrorPerSector[c].lpBufNoC2Sector, dwAllBufLen);
	}
}
