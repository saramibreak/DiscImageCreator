/**
 * Copyright 2011-2020 sarami
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
#include "execIoctl.h"
#include "execScsiCmd.h"
#include "execScsiCmdforCD.h"
#include "execScsiCmdforCDCheck.h"
#include "init.h"
#include "get.h"
#include "output.h"
#include "outputScsiCmdLog.h"
#include "outputScsiCmdLogforCD.h"
#include "set.h"

BOOL TestUnitReady(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	CDB::_CDB6GENERIC cdb = {};
	cdb.OperationCode = SCSIOP_TEST_UNIT_READY;

#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_NONE;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB6GENERIC_LENGTH,
		NULL, direction, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

BOOL Inquiry(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	CDB::_CDB6INQUIRY3 cdb = {};
	cdb.OperationCode = SCSIOP_INQUIRY;
	cdb.AllocationLength = sizeof(INQUIRYDATA);
#ifdef _WIN32
	_declspec(align(4)) INQUIRYDATA inquiryData = {};
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	__attribute__((aligned(4))) INQUIRYDATA inquiryData = {};
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB6GENERIC_LENGTH, &inquiryData, 
		direction, sizeof(INQUIRYDATA), &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	strncpy(pDevice->szVendorId,
		(LPCCH)&inquiryData.VendorId, sizeof(pDevice->szVendorId));
	strncpy(pDevice->szProductId,
		(LPCCH)&inquiryData.ProductId, sizeof(pDevice->szProductId));
	strncpy(pDevice->szProductRevisionLevel,
		(LPCCH)&inquiryData.ProductRevisionLevel, sizeof(pDevice->szProductRevisionLevel));

	if (*pExecType != drivespeed) {
		OutputInquiry(&inquiryData);
	}
	return TRUE;
}

#define MODE_SENSE_SIZE 256
BOOL ModeSense(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	if (pDevice->FEATURE.byModePage2a) {
		CDB::_MODE_SENSE cdb = {};
		cdb.OperationCode = SCSIOP_MODE_SENSE;
		cdb.PageCode = MODE_SENSE_RETURN_ALL;
		cdb.Pc = 2;
		BYTE bySize = MODE_SENSE_SIZE - 1;
		cdb.AllocationLength = bySize;
		BYTE modesense[MODE_SENSE_SIZE] = {};
#ifdef _WIN32
		INT direction = SCSI_IOCTL_DATA_IN;
#else
		INT direction = SG_DXFER_FROM_DEV;
#endif
		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB6GENERIC_LENGTH, &modesense,
			direction, bySize, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
			byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			// not false.
		}
		else {
			OutputModeSense(pExecType, pDevice, pDisc, modesense);
		}
	}
	return TRUE;
}

BOOL StartStopUnit(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BYTE Start,
	BYTE LoadEject
) {
	CDB::_START_STOP cdb = {};
	cdb.OperationCode = SCSIOP_START_STOP_UNIT;
	cdb.Start = (UCHAR)(Start & 0x01);
	cdb.LoadEject = (UCHAR)(LoadEject & 0x01);
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_NONE;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB6GENERIC_LENGTH
		, NULL, direction, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

BOOL SynchronizeCache(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	CDB::_SYNCHRONIZE_CACHE10 cdb = {};
	cdb.OperationCode = SCSIOP_SYNCHRONIZE_CACHE;
	cdb.Immediate = TRUE;
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, NULL, direction, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

BOOL ReadTOC(
	PEXT_ARG pExtArg,
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC pDisc
) {
	CDB::_READ_TOC cdb = {};
	cdb.OperationCode = SCSIOP_READ_TOC;
	cdb.Format2 = CDROM_READ_TOC_EX_FORMAT_TOC;
	cdb.StartingTrack = 1;
	TWO_BYTE size;
	size.AsUShort = CDROM_TOC_SIZE;
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);

#ifdef _DEBUG
	OutputString("pDisc->SCSI.toc address: %p\n", &pDisc->SCSI.toc);
#endif
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, &pDisc->SCSI.toc, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		if (pDisc->SCSI.wCurrentMedia != ProfileDvdRom) {
			return FALSE;
		}
	}
	OutputMainInfoLog(OUTPUT_DHYPHEN_PLUS_STR("TOC (Binary)"));
	OutputCDMain(fileMainInfo, (LPBYTE)&pDisc->SCSI.toc, 0, size.AsUShort);

	if (pDisc->SCSI.toc.FirstTrack < 1 || 99 < pDisc->SCSI.toc.FirstTrack ||
		pDisc->SCSI.toc.LastTrack < 1 || 99 < pDisc->SCSI.toc.LastTrack) {
		OutputErrorString("Invalid TOC\n");
		return FALSE;
	}
	if (!InitLBAPerTrack(pExecType, &pDisc)) {
		return FALSE;
	}
	if (byScsiStatus == SCSISTAT_GOOD) {
		pDevice->bySuccessReadToc = TRUE;
		SetAndOutputToc(pDisc);
		if (*pExecType == gd) {
			pDisc->SCSI.trkType = TRACK_TYPE::dataExist;
			OutputDiscLog("This is the TOC of audio trap disc\n");
		}
	}
	return TRUE;
}

BOOL ReadTOCFull(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PCDROM_TOC_FULL_TOC_DATA pFullTocData,
	PCDROM_TOC_FULL_TOC_DATA_BLOCK* pTocData,
	LPWORD wTocEntries,
	LPBYTE* pPFullToc
) {
	CDB::_READ_TOC cdb = {};
	cdb.OperationCode = SCSIOP_READ_TOC;
	cdb.Format2 = CDROM_READ_TOC_EX_FORMAT_FULL_TOC;
	cdb.StartingTrack = 1;
	TWO_BYTE size;
	size.AsUShort = sizeof(CDROM_TOC_FULL_TOC_DATA);
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);

#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, pFullTocData, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
#if 0
		UINT uiBufLen = CD_RAW_SECTOR_SIZE + CD_RAW_READ_SUBCODE_SIZE;
		if (!ReadCDForCheckingSubQAdrFirst(pExtArg, pDevice
			, pDisc, &pBuf, &lpBuf, lpCmd, &uiBufLen, &nOfs, CDFLAG::_READ_CD::All)) {
			return FALSE;
		}
		for (BYTE i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
			if (!ReadCDForCheckingSubQAdr(pExtArg, pDevice, pDisc
				, pDiscPerSector, lpCmd, lpBuf, uiBufLen, nOfs, i, &byMode, 1, fpCcd)) {
				return FALSE;
			}
			if (bySessionNum < 1) {
				WriteCcdForSession(1, byMode, fpCcd);
				bySessionNum++;
			}
			OutputString(
				"\rChecking SubQ adr (Track) %2u/%2u", i + 1, pDisc->SCSI.toc.LastTrack);
		}
		OutputString("\n");
#endif
		pDevice->bySuccessReadTocFull = FALSE;
		return TRUE;
	}
	WORD wFullTocLen = MAKEWORD(pFullTocData->Length[1], pFullTocData->Length[0]);
	WORD wTocEntriesAll = (WORD)(wFullTocLen - 2); // 2 is the size of FirstCompleteSession and LastCompleteSession
	*wTocEntries = (WORD)(wTocEntriesAll / sizeof(CDROM_TOC_FULL_TOC_DATA_BLOCK));

	pDisc->SCSI.bMultiSession = pFullTocData->LastCompleteSession > 1 ? TRUE : FALSE;
	pDisc->SCSI.n1stLBAof2ndSession = -1;

	WORD wFullTocLenFix = (WORD)(wTocEntriesAll + sizeof(CDROM_TOC_FULL_TOC_DATA));
#ifdef _DEBUG
	OutputString(
		"FullTocLen: %u, TocEntriesAll: %u, TocEntries: %u, FullTocLenFix (before padding): %u\n",
		wFullTocLen, wTocEntriesAll, *wTocEntries, wFullTocLenFix);
#endif
	// 4 byte padding
	if (wFullTocLenFix % 4) {
		wFullTocLenFix = (WORD)((wFullTocLenFix / 4 + 1) * 4);
	}
	LPBYTE pFullToc = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, pPFullToc,
		wFullTocLenFix, &pFullToc, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
#ifdef _DEBUG
	OutputString(
		"FullTocLenFix (after padding): %u, pPFullToc address: %p, pFullToc address: %p\n"
		, wFullTocLenFix, &pPFullToc, &pFullToc);
#endif
	size.AsUShort = wFullTocLenFix;
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);
	BOOL bRet = TRUE;
	try {
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
			, pFullToc, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
		*pTocData = ((PCDROM_TOC_FULL_TOC_DATA)(pFullToc))->Descriptors;
		INT nTmpLBAExt = 0;
		for (WORD a = 0; a < *wTocEntries; a++) {
			switch ((*pTocData + a)->Point) {
			case 0xa0:
				pDisc->SCSI.byFormat = (*pTocData + a)->Msf[1];
				break;
			case 0xa2:
				if ((*pTocData + a)->SessionNumber == 1) {
					pDisc->SCSI.n1stLBAofLeadout = 
						MSFtoLBA((*pTocData + a)->Msf[0], (*pTocData + a)->Msf[1], (*pTocData + a)->Msf[2]) - 150;
				}
				break;
			case 0xb0: // (multi-session disc)
				/* single-session disc, but 0xb0 exists
				FirstCompleteSession: 1
				 LastCompleteSession: 1
				Session 1, Ctl 4, Adr 1, Point 0xa0, FirstTrack  1, Format: CD-DA or CD-ROM
				Session 1, Ctl 4, Adr 1, Point 0xa1,  LastTrack  1
				Session 1, Ctl 4, Adr 1, Point 0xa2,      Lead-out, AMSF 62:10:20 (LBA[279620, 0x44444])
				Session 1, Ctl 4, Adr 1, Point 0x01,      Track  1, AMSF 00:02:00 (LBA[000000, 0000000])
				Session 1, Ctl 4, Adr 5, Point 0xb0,   NextSession, AMSF 64:40:20 (LBA[290870, 0x47036])
									Outermost Lead-out of the disc, AMSF 74:05:11 (LBA[333236, 0x515b4])
										 Num of pointers in Mode 5, 02
				Session 1, Ctl 4, Adr 5, Point 0xc0, Optimum recording power, 128
										 First Lead-in of the disc, AMSF 97:27:56 (LBA[438431, 0x6b09f])
				 */
				if (pDisc->SCSI.bMultiSession) {
					nTmpLBAExt =
						MSFtoLBA((*pTocData + a)->MsfExtra[0], (*pTocData + a)->MsfExtra[1], (*pTocData + a)->MsfExtra[2]) - 150;
					pDisc->SCSI.n1stLBAof2ndSession = nTmpLBAExt + 150;
				}
				break;
			default:
				break;
			}
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	pDevice->bySuccessReadTocFull = TRUE;
	return bRet;
}

BOOL ReadTOCPma(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	CDB::_READ_TOC cdb = {};
	cdb.OperationCode = SCSIOP_READ_TOC;
	cdb.Format2 = CDROM_READ_TOC_EX_FORMAT_PMA;
	TWO_BYTE size;
	size.AsUShort= sizeof(CDROM_TOC_PMA_DATA);
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);

#ifdef _WIN32
	_declspec(align(4)) CDROM_TOC_PMA_DATA pma = { 0 };
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	__attribute__((aligned(4))) CDROM_TOC_PMA_DATA pma = {};
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, &pma, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputDriveNoSupportLog("READ_TOC_PMA");
		return TRUE;
	}
	WORD wTocPmaLen = MAKEWORD(pma.Length[1], pma.Length[0]);
	WORD wTocPmaAll = (WORD)(wTocPmaLen - sizeof(pma.Length));

	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("TOC_PMA"));
	if (wTocPmaAll) {
		LPBYTE pPTocPma = NULL;
		LPBYTE pTocPma = NULL;
		if (!GetAlignedCallocatedBuffer(pDevice, &pPTocPma,
			wTocPmaAll, &pTocPma, _T(__FUNCTION__), __LINE__)) {
			OutputDriveNoSupportLog("READ_TOC_PMA");
			return TRUE;
		}
		size.AsUShort = wTocPmaAll;
		REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
			, pTocPma, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputDriveNoSupportLog("READ_TOC_PMA");
			return TRUE;
		}
		PCDROM_TOC_FULL_TOC_DATA_BLOCK pDesc =
			((PCDROM_TOC_PMA_DATA)pTocPma)->Descriptors;
		for (UINT i = 0; i < wTocPmaAll / sizeof(CDROM_TOC_FULL_TOC_DATA_BLOCK); i++) {
			OutputDiscLog(
				"\tAdr: %u, Control: %u\n"
				"\t          Point: %u\n"
				"\t            Msf: %02u:%02u:%02u\n"
				"\t            Msf: %02u:%02u:%02u\n"
				, pDesc[i].Adr, pDesc[i].Control
				, pDesc[i].Point
				, pDesc[i].MsfExtra[0], pDesc[i].MsfExtra[1], pDesc[i].MsfExtra[2]
				, pDesc[i].Msf[0], pDesc[i].Msf[1], pDesc[i].Msf[2]
			);
		}
		FreeAndNull(pPTocPma);
	}
	else {
		OutputDiscLog("\tNothing\n");
	}
	return TRUE;
}

BOOL ReadTOCAtip(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	CDB::_READ_TOC cdb = {};
	cdb.OperationCode = SCSIOP_READ_TOC;
	cdb.Format2 = CDROM_READ_TOC_EX_FORMAT_ATIP;
	TWO_BYTE size;
	size.AsUShort = sizeof(CDROM_TOC_ATIP_DATA);
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);

#ifdef _WIN32
	_declspec(align(4)) CDROM_TOC_ATIP_DATA atip = { 0 };
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	__attribute__((aligned(4))) CDROM_TOC_ATIP_DATA atip = {};
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, &atip, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputDriveNoSupportLog("READ_TOC_ATIP");
		return TRUE;
	}
	WORD wTocAtipLen = MAKEWORD(atip.Length[1], atip.Length[0]);
	WORD wTocAtipAll = (WORD)(wTocAtipLen - sizeof(atip.Length));

	LPBYTE pPTocAtip = NULL;
	LPBYTE pTocAtip = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pPTocAtip,
		wTocAtipAll, &pTocAtip, _T(__FUNCTION__), __LINE__)) {
		OutputDriveNoSupportLog("READ_TOC_ATIP");
		return TRUE;
	}
	size.AsUShort = wTocAtipAll;
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, pTocAtip, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputDriveNoSupportLog("READ_TOC_ATIP");
		return TRUE;
	}
	PCDROM_TOC_ATIP_DATA_BLOCK pDesc =
		((PCDROM_TOC_ATIP_DATA)pTocAtip)->Descriptors;
	OutputCDAtip(pDesc);
	FreeAndNull(pPTocAtip);
	return TRUE;
}

BOOL ReadTOCText(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	FILE* fpCcd
) {
	CDB::_READ_TOC cdb = {};
	cdb.OperationCode = SCSIOP_READ_TOC;
	cdb.Format2 = CDROM_READ_TOC_EX_FORMAT_CDTEXT;
	TWO_BYTE size;
	size.AsUShort = sizeof(CDROM_TOC_CD_TEXT_DATA);
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);

#ifdef _WIN32
	_declspec(align(4)) CDROM_TOC_CD_TEXT_DATA tocText = { 0 };
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	__attribute__((aligned(4))) CDROM_TOC_CD_TEXT_DATA tocText = {}; 
	INT direction = SG_DXFER_FROM_DEV;
#endif
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("CDTEXT"));
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, &tocText, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		// not false. because undefined mmc1..
		OutputDriveNoSupportLog("Nothing CDTEXT or READ_TOC_EX_FORMAT_CDTEXT");
		return TRUE;
	}
	WORD wTocTextLen = MAKEWORD(tocText.Length[1], tocText.Length[0]);
	WORD wTocTextEntriesAll = (WORD)(wTocTextLen - sizeof(tocText.Length));

	WriteCcdForDiscCDTextLength(wTocTextEntriesAll, fpCcd);
	if (!wTocTextEntriesAll) {
		OutputDiscLog("\tNothing\n");
		// many CD is no text
		return TRUE;
	}

	WORD wTocTextEntries = (WORD)(wTocTextEntriesAll / sizeof(CDROM_TOC_CD_TEXT_DATA_BLOCK));
#ifndef _WIN32
//	OutputDiscLog("sizeof(CDROM_TOC_CD_TEXT_DATA_BLOCK): %u\n", sizeof(CDROM_TOC_CD_TEXT_DATA_BLOCK));
#endif
	WriteCcdForCDText(wTocTextEntries, fpCcd);

	WORD wTocTextLenFix = (WORD)(wTocTextEntriesAll + sizeof(CDROM_TOC_CD_TEXT_DATA));
	// 4 byte padding
	if (wTocTextLenFix % 4) {
		wTocTextLenFix = (WORD)((wTocTextLenFix / 4 + 1) * 4);
	}
	LPBYTE pPTocText = NULL;
	LPBYTE pTocText = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pPTocText,
		wTocTextLenFix, &pTocText, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
#ifdef _DEBUG
	OutputDiscLog(
		"TocTextLen: %u, TocTextEntriesAll: %u, TocTextEntries: %u, TocTextLenFix: %u\n"
		, wTocTextLen, wTocTextEntriesAll, wTocTextEntries, wTocTextLenFix);
#endif
	size.AsUShort = wTocTextLenFix;
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);
	LPCH pTmpText = NULL;
	BOOL bRet = TRUE;
	try {
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
			, pTocText, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
		PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc =
			((PCDROM_TOC_CD_TEXT_DATA)pTocText)->Descriptors;
		WriteCcdForCDTextEntry(pDesc, wTocTextEntries, fpCcd);

		WORD wAllTextSize = (WORD)(wTocTextEntries * sizeof(pDesc->Text));
		if (NULL == (pTmpText = (LPCH)calloc(wAllTextSize, sizeof(_TCHAR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		UINT uiLastSeqNumOfBlock[9] = {};
		for (INT i = 1, j = 8; i <= 4; i++, j++) {
			uiLastSeqNumOfBlock[i] = pDesc[wTocTextEntries - 2].Text[j];
			if (1 < i && uiLastSeqNumOfBlock[i]) {
				uiLastSeqNumOfBlock[i] += uiLastSeqNumOfBlock[i - 1];
				uiLastSeqNumOfBlock[i]++;
			}
		}
		if (uiLastSeqNumOfBlock[4]) {
			for (INT k = 5, m = 0; k <= 8; k++, m++) {
				uiLastSeqNumOfBlock[k] = pDesc[wTocTextEntries - 2].Text[m];
				if (5 < k && uiLastSeqNumOfBlock[k]) {
					uiLastSeqNumOfBlock[k] += uiLastSeqNumOfBlock[k - 1];
					uiLastSeqNumOfBlock[k]++;
				}
			}
		}

		for (INT n = 0; n < MAX_CDTEXT_LANG; n++) {
			if (uiLastSeqNumOfBlock[n + 1]) {
				pDisc->SCSI.CDTEXT[n].bExist = TRUE;
				if (n == 0) {
					SetAndOutputTocCDText(pDisc, pDesc, pTmpText, (WORD)(uiLastSeqNumOfBlock[n + 1] + 1)
						, 0, pDesc[uiLastSeqNumOfBlock[n + 1]].Unicode, n);
				}
				else {
					SetAndOutputTocCDText(pDisc, pDesc, pTmpText, (WORD)(uiLastSeqNumOfBlock[n + 1] + 1)
						, uiLastSeqNumOfBlock[n] + 1, pDesc[uiLastSeqNumOfBlock[n + 1]].Unicode, n);
				}
			}
			else {
				break;
			}
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FreeAndNull(pPTocText);
	FreeAndNull(pTmpText);
	return bRet;
}

BOOL GetConfiguration(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	CDB::_GET_CONFIGURATION cdb = {};
	cdb.OperationCode = SCSIOP_GET_CONFIGURATION;
	cdb.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_CURRENT;
	cdb.StartingFeature[1] = FeatureProfileList;
	TWO_BYTE size;
	size.AsUShort = sizeof(GET_CONFIGURATION_HEADER);
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);

#ifdef _WIN32
	_declspec(align(4)) GET_CONFIGURATION_HEADER configHeader = { 0 };
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	__attribute__((aligned(4))) GET_CONFIGURATION_HEADER configHeader = {};
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, &configHeader, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		pDisc->SCSI.wCurrentMedia = ProfileCdrom;
		// not false. because undefined mmc1..
		OutputDriveNoSupportLog("GET_CONFIGURATION");
		if (pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX40TS ||
			pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX32TS ||
			pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX20TS
			) {
			pDevice->FEATURE.byCanCDText = TRUE;
			pDevice->FEATURE.byC2ErrorData = TRUE;
			OutputLog(standardOut | fileDrive, "But this drive supports to read CDText and C2 Error\n");
		}
	}
	else {
		pDisc->SCSI.wCurrentMedia =
			MAKEWORD(configHeader.CurrentProfile[1], configHeader.CurrentProfile[0]);
		if (*pExecType != drivespeed) {
			if (pDisc->SCSI.wCurrentMedia == ProfileInvalid) {
				OutputDriveLog(
					"SCSIOP_GET_CONFIGURATION didn't fail. But it couldn't get CurrentMedia on this drive\n");
				if (*pExecType == dvd) {
					OutputDriveLog("\tSet CurrentMedia to DVD-ROM\n");
					configHeader.CurrentProfile[1] = ProfileDvdRom;
					pDisc->SCSI.wCurrentMedia = ProfileDvdRom;
				}
				else {
					OutputDriveLog("\tSet CurrentMedia to CD-ROM\n");
					configHeader.CurrentProfile[1] = ProfileCdrom;
					pDisc->SCSI.wCurrentMedia = ProfileCdrom;
				}
			}
			OutputGetConfigurationHeader(&configHeader);

			DWORD dwAllLen =
				MAKEDWORD(MAKEWORD(configHeader.DataLength[3], configHeader.DataLength[2]),
					MAKEWORD(configHeader.DataLength[1], configHeader.DataLength[0])) -
				sizeof(configHeader.DataLength) + sizeof(GET_CONFIGURATION_HEADER);
			LPBYTE pPConf = NULL;
			LPBYTE lpConf = NULL;
			if (!GetAlignedCallocatedBuffer(pDevice, &pPConf,
				(UINT)dwAllLen, &lpConf, _T(__FUNCTION__), __LINE__)) {
				return FALSE;
			}
			size.AsUShort = (USHORT)dwAllLen;
			REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);

			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
				, lpConf, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				// not false. because undefined mmc1..
				OutputDriveNoSupportLog("GET_CONFIGURATION");
			}
			else {
				OutputGetConfigurationFeatureNumber(pDevice,
					lpConf + sizeof(GET_CONFIGURATION_HEADER), dwAllLen - sizeof(GET_CONFIGURATION_HEADER));
				if (pDevice->byPlxtrDrive == (BYTE)PLXTR_DRIVE_TYPE::PXW1210A ||
					pDevice->byPlxtrDrive == (BYTE)PLXTR_DRIVE_TYPE::PXW1210S) {
					// Somehow SetDiscSpeed fails in PX-W1210...
					pDevice->FEATURE.bySetCDSpeed = FALSE;
				}
			}
			FreeAndNull(pPConf);
		}
	}
	return TRUE;
}

BOOL ReadDiscInformation(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	CDB::_READ_DISK_INFORMATION cdb = {};
	cdb.OperationCode = SCSIOP_READ_DISC_INFORMATION;
	TWO_BYTE size;
	size.AsUShort = sizeof(DISC_INFORMATION);
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);

#ifdef _WIN32
	_declspec(align(4)) DISC_INFORMATION discInformation = {};
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	__attribute__((aligned(4))) DISC_INFORMATION discInformation = {};
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, &discInformation, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		// not false.
		OutputDriveNoSupportLog("READ_DISC_INFORMATION");
	}
	else {
		OutputDiscInformation(&discInformation);
	}
	return TRUE;
}

BOOL ModeSense10(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	CDB::_MODE_SENSE10 cdb = {};
	cdb.OperationCode = SCSIOP_MODE_SENSE10;
	cdb.PageCode = MODE_SENSE_RETURN_ALL;
	cdb.Pc = 0; // 0: Current Values, 1:Changeable Values, 2:Default Values, 3:Saved Values
	TWO_BYTE size;
	size.AsUShort = MODE_SENSE_SIZE;
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);
	BYTE modesense[MODE_SENSE_SIZE] = {};
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, &modesense, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		// not false.
		OutputDriveNoSupportLog("MODE_SENSE10");
		ModeSense(pExecType, pExtArg, pDevice, pDisc);
	}
	else {
		OutputModeSense(pExecType, pDevice, pDisc, modesense);
	}
	return TRUE;
}

BOOL ReadBufferCapacity(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	if (pDevice->FEATURE.byReadBufCapa) {
		CDB::_READ_BUFFER_CAPACITY cdb = {};
		cdb.OperationCode = SCSIOP_READ_BUFFER_CAPACITY;
		TWO_BYTE size;
		size.AsUShort = sizeof(READ_BUFFER_CAPACITY_DATA);
		REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);

#ifdef _WIN32
		_declspec(align(4)) READ_BUFFER_CAPACITY_DATA readBufCapaData = {};
		INT direction = SCSI_IOCTL_DATA_IN;
#else
		__attribute__((aligned(4))) READ_BUFFER_CAPACITY_DATA readBufCapaData = {};
		INT direction = SG_DXFER_FROM_DEV;
#endif
		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH
			, &readBufCapaData, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			// not false.
			OutputDriveNoSupportLog("READ_BUFFER_CAPACITY");
		}
		else {
			OutputReadBufferCapacity(&readBufCapaData);
		}
	}
	return TRUE;
}

BOOL SendKey(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BYTE agid,
	BYTE keyFormat,
	LPBYTE key,
	WORD keyLength
) {
	CDB::_SEND_KEY cdb = {};
	cdb.OperationCode = SCSIOP_SEND_KEY;
	TWO_BYTE size;
	size.AsUShort = keyLength;
	REVERSE_BYTES_SHORT(&cdb.ParameterListLength, &size);
	cdb.KeyFormat = (BYTE)(keyFormat & 0x3f);
	cdb.AGID = (BYTE)(agid & 0x3);

#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_TO_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH,
		key, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}

	return TRUE;
}

BOOL ReportKey(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BYTE agid,
	BYTE keyFormat,
	LPBYTE key,
	WORD keyLength
) {
	CDB::_REPORT_KEY cdb = {};
	cdb.OperationCode = SCSIOP_REPORT_KEY;
	TWO_BYTE size;
	size.AsUShort = keyLength;
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &size);
	cdb.KeyFormat = (BYTE)(keyFormat & 0x3f);
	cdb.AGID = (BYTE)(agid & 0x3);

#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH,
		key, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
 		return FALSE;
	}
	return TRUE;
}

BOOL SetDiscSpeed(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	UINT uiDiscSpeedNum
) {
//	if ((*pExecType == cd || *pExecType == gd || *pExecType == audio || *pExecType == data)
//		&& (pDevice->FEATURE.bySetCDSpeed || *pExecType == dvd)) {
		WORD wSpeed = 0;
		// https://msdn.microsoft.com/en-us/library/windows/hardware/ff551368(v=vs.85).aspx
		// https://msdn.microsoft.com/ja-jp/library/ff551396(v=vs.85).aspx
#ifdef _WIN32
		_declspec(align(4)) CDROM_SET_SPEED setspeed = { CdromSetSpeed, 0, 0, CdromDefaultRotation };
		INT direction = SCSI_IOCTL_DATA_IN;
#else
		__attribute__((aligned(4))) CDROM_SET_SPEED setspeed = { CdromSetSpeed, 0, 0, CdromDefaultRotation };
		INT direction = SG_DXFER_TO_DEV;
#endif
		if ((*pExecType == cd || *pExecType == swap || *pExecType == gd || *pExecType == audio || *pExecType == data) &&
			0 < uiDiscSpeedNum && uiDiscSpeedNum <= CD_DRIVE_MAX_SPEED) {
			// http://senbee.seesaa.net/article/26247159.html
			// 2048 x 75 = 153600 B -> 150 KiB
			// 2352 x 75 = 176400 B -> 172,265625 KiB
			wSpeed = (WORD)(CD_RAW_SECTOR_SIZE * 75 * uiDiscSpeedNum / 1000);
			setspeed.ReadSpeed = wSpeed;
		}
		else if ((*pExecType == dvd || IsXbox(pExecType)) &&
			0 < uiDiscSpeedNum && uiDiscSpeedNum <= DVD_DRIVE_MAX_SPEED) {
			// Read and write speeds for the first DVD drives and players were of
			// 1,385 kB/s (1,353 KiB/s); this speed is usually called "1x".
			// 2048 x 75 x 9 = 1384448 B -> 1352 KiB
			wSpeed = (WORD)(1385 * uiDiscSpeedNum);
			setspeed.ReadSpeed = wSpeed;
		}
		else if ((*pExecType == bd) &&
			0 < uiDiscSpeedNum && uiDiscSpeedNum <= BD_DRIVE_MAX_SPEED) {
			wSpeed = (WORD)(4496 * uiDiscSpeedNum);
			setspeed.ReadSpeed = wSpeed;
		}
		else {
			wSpeed = 0xffff;
			setspeed.ReadSpeed = pDevice->wMaxReadSpeed;
		}
		CDB::_SET_CD_SPEED cdb = {};
		cdb.OperationCode = SCSIOP_SET_CD_SPEED;
		TWO_BYTE size;
		size.AsUShort = wSpeed;
		REVERSE_BYTES_SHORT(&cdb.ReadSpeed, &size);
		// https://msdn.microsoft.com/en-us/library/windows/hardware/ff551370(v=vs.85).aspx
		setspeed.RequestType = CdromSetSpeed;

		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, &setspeed, 
			direction, sizeof(CDROM_SET_SPEED), &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			// Somehow PX-W1210S fails...
			OutputDriveNoSupportLog("SET_CD_SPEED");
			OutputDriveLog("Or if you use the SATA/IDE to USB adapter, doesn't support this command\n");
		}
		else {
			OutputSetSpeed(&setspeed);
			OutputString("Set the drive speed: %uKB/sec\n", setspeed.ReadSpeed);
		}
//	}
	return TRUE;
}

BOOL SetStreaming(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	DWORD dwDiscSpeedNum
) {
	UNREFERENCED_PARAMETER(dwDiscSpeedNum);
	CDB::_SET_STREAMING cdb = {};
	cdb.OperationCode = SCSIOP_SET_STREAMING;
#ifdef _WIN32
	_declspec(align(4)) PERFORMANCE_DESCRIPTOR pd = {};
#else
	__attribute__((aligned(4))) PERFORMANCE_DESCRIPTOR pd = {};
#endif
	TWO_BYTE size;
	size.AsUShort = sizeof(PERFORMANCE_DESCRIPTOR);
	REVERSE_BYTES_SHORT(&cdb.ParameterListLength, &size);
#if 1
	FOUR_BYTE LBA;
	LBA.AsULong = 0x7FFFFFFF;
	REVERSE_BYTES(&pd.EndLba, &LBA);
	FOUR_BYTE RSize;
	RSize.AsULong = 0;
	REVERSE_BYTES(&pd.ReadSize, &RSize);
	FOUR_BYTE WSize;
	WSize.AsULong = 0;
	REVERSE_BYTES(&pd.WriteSize, &WSize);

//	INT nTime = 0x3e8;
//	REVERSE_BYTES(&pd.ReadTime, &nTime);
//	REVERSE_BYTES(&pd.WriteTime, &nTime);
#endif
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_OUT;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH
		, &pd, direction, size.AsUShort, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("SetStreaming")
		"\t        RandomAccess: %s\n"
		"\t               Exact: %s\n"
		"\t     RestoreDefaults: %s\n"
		"\tWriteRotationControl: %s\n"
		"\t            StartLba: %u (%x)\n"
		"\t              EndLba: %u (%x)\n"
		"\t            ReadSize: %u (%x)\n"
		"\t            ReadTime: %u (%x)\n"
		"\t           WriteSize: %u (%x)\n"
		"\t           WriteTime: %u (%x)\n"
		, BOOLEAN_TO_STRING_YES_NO(pd.RandomAccess)
		, BOOLEAN_TO_STRING_YES_NO(pd.Exact)
		, BOOLEAN_TO_STRING_YES_NO(pd.RestoreDefaults)
		, BOOLEAN_TO_STRING_YES_NO(pd.WriteRotationControl)
		, MAKEUINT(MAKEWORD(pd.StartLba[3], pd.StartLba[2]), MAKEWORD(pd.StartLba[1], pd.StartLba[0]))
		, MAKEUINT(MAKEWORD(pd.StartLba[3], pd.StartLba[2]), MAKEWORD(pd.StartLba[1], pd.StartLba[0]))
		, MAKEUINT(MAKEWORD(pd.EndLba[3], pd.EndLba[2]), MAKEWORD(pd.EndLba[1], pd.EndLba[0]))
		, MAKEUINT(MAKEWORD(pd.EndLba[3], pd.EndLba[2]), MAKEWORD(pd.EndLba[1], pd.EndLba[0]))
		, MAKEUINT(MAKEWORD(pd.ReadSize[3], pd.ReadSize[2]), MAKEWORD(pd.ReadSize[1], pd.ReadSize[0]))
		, MAKEUINT(MAKEWORD(pd.ReadSize[3], pd.ReadSize[2]), MAKEWORD(pd.ReadSize[1], pd.ReadSize[0]))
		, MAKEUINT(MAKEWORD(pd.ReadTime[3], pd.ReadTime[2]), MAKEWORD(pd.ReadTime[1], pd.ReadTime[0]))
		, MAKEUINT(MAKEWORD(pd.ReadTime[3], pd.ReadTime[2]), MAKEWORD(pd.ReadTime[1], pd.ReadTime[0]))
		, MAKEUINT(MAKEWORD(pd.WriteSize[3], pd.WriteSize[2]), MAKEWORD(pd.WriteSize[1], pd.WriteSize[0]))
		, MAKEUINT(MAKEWORD(pd.WriteSize[3], pd.WriteSize[2]), MAKEWORD(pd.WriteSize[1], pd.WriteSize[0]))
		, MAKEUINT(MAKEWORD(pd.WriteTime[3], pd.WriteTime[2]), MAKEWORD(pd.WriteTime[1], pd.WriteTime[0]))
		, MAKEUINT(MAKEWORD(pd.WriteTime[3], pd.WriteTime[2]), MAKEWORD(pd.WriteTime[1], pd.WriteTime[0]))
	);
	return TRUE;
}

// feature PLEXTOR drive
BOOL SetSpeedRead(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BOOL bState
) {
	// PX-708, PXW4012 or older doesn't support SpeedRead
	if (pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX760A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX755A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX716AL ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX716A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX714A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX712A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PREMIUM2 ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PREMIUM ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXW5224A ||
		pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PXW4824A
		) {
		CONST WORD size = 8;
		BYTE buf[size] = {};

		CDB::_CDB12 cdb = {};
		cdb.OperationCode = SCSIOP_PLXTR_EXTEND;
		cdb.DisablePageOut = TRUE;
		cdb.LogicalBlock[0] = PLXTR_FLAG_SPEED_READ;
		cdb.LogicalBlock[1] = (BYTE)bState;
		cdb.Reserved2 = 0x08;
#ifdef _WIN32
		INT direction = SCSI_IOCTL_DATA_IN;
#else
		INT direction = SG_DXFER_FROM_DEV;
#endif
		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH
			, buf, direction, sizeof(buf), &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
#if 0
		for (INT i = 0; i < size; i++) {
			OutputString("%02x "), buf[i]);
		}
		OutputString("\n");
#endif
	}
	return TRUE;
}

BOOL Reset(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	CDB::_CDB6GENERIC cdb = {};
	cdb.OperationCode = SCSIOP_PLXTR_RESET;
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB6GENERIC_LENGTH
		, NULL, direction, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

BOOL ReadEeprom(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	UINT tLen = 128;
	BOOL bHigh = FALSE;
	switch (pDevice->byPlxtrDrive) {
	case PLXTR_DRIVE_TYPE::PXW5224A:
	case PLXTR_DRIVE_TYPE::PREMIUM:
	case PLXTR_DRIVE_TYPE::PREMIUM2:
		tLen = 160;
		break;
	case PLXTR_DRIVE_TYPE::PX320A:
	case PLXTR_DRIVE_TYPE::PX704A:
	case PLXTR_DRIVE_TYPE::PX708A:
	case PLXTR_DRIVE_TYPE::PX708A2:
		tLen = 256;
		break;
	case PLXTR_DRIVE_TYPE::PX712A:
		tLen = 512;
		break;
	case PLXTR_DRIVE_TYPE::PX714A:
	case PLXTR_DRIVE_TYPE::PX716A:
	case PLXTR_DRIVE_TYPE::PX716AL:
	case PLXTR_DRIVE_TYPE::PX755A:
	case PLXTR_DRIVE_TYPE::PX760A:
		tLen = 256;
		bHigh = TRUE;
		break;
	default:
		break;
	}
	UINT BufLen = tLen;
	LPBYTE pPBuf = NULL;
	LPBYTE pBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pPBuf,
		BufLen, &pBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	CDB::_CDB12 cdb = {};
	cdb.OperationCode = SCSIOP_PLXTR_READ_EEPROM;
	cdb.RelativeAddress = (UCHAR)(bHigh & 0x01);

	BOOL bRet = TRUE;
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	OutputDriveLog(OUTPUT_DHYPHEN_PLUS_STR("EEPROM"));
	try {
		FOUR_BYTE size;
		for (BYTE idx = 0; idx < 4; idx++) {
			size.AsULong = tLen;
			REVERSE_BYTES(&cdb.TransferLength, &size);
			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH
				, pBuf, direction, BufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			OutputEeprom(pBuf, idx, pDevice->byPlxtrDrive);
			if (bHigh) {
				tLen += 0x10000;
			}
			else {
				break;
			}
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pPBuf);
	return bRet;
}

BOOL ReadCacheForLgAsus(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpOutBuf,
	INT nLineNum,
	INT nLBA,
	LPBOOL lpbCached,
	LPINT lpbLeadOutCnt
) {
	CONST DWORD dwBufSize = F1_BUFFER_SIZE * F1_READ_SECTOR_SIZE;
	BYTE aBuf[dwBufSize] = {};
	// http://forum.redump.org/post/72629/#p72629
	// F1 06 xx xx xx xx yy yy yy yy
	// xx - address to read
	// yy - length of data to return
	//
	// 0x000-0x92F - Main Channel (Scrambled)
	// 0x930-0x98F - P-W Subchannel
	// 0x990-0x99F - Q Subchannel
	// 0x9A0-0x9A3 - (unknown)
	// 0x9A4-0xAC9 - C2 Error Bits
	// 0xACA-0xB00 - (unknown)
	CDB::_CDB10 cdb = {};
	cdb.OperationCode = 0xf1;
	cdb.Reserved1 = 3;
	cdb.LogicalBlockByte0 = BYTE((0xb00 * nLineNum >> 24) & 0xff);
	cdb.LogicalBlockByte1 = BYTE((0xb00 * nLineNum >> 16) & 0xff);
	cdb.LogicalBlockByte2 = BYTE((0xb00 * nLineNum >> 8) & 0xff);
	cdb.LogicalBlockByte3 = BYTE(0xb00 * nLineNum & 0xff);
	cdb.Reserved2 = BYTE((dwBufSize >> 24) & 0xff);
	cdb.TransferBlocksMsb = BYTE((dwBufSize >> 16) & 0xff);
	cdb.TransferBlocksLsb = BYTE((dwBufSize >> 8) & 0xff);
	cdb.Control = BYTE(dwBufSize & 0xff);
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, aBuf, direction, dwBufSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}

	BYTE aMainBuf[CD_RAW_SECTOR_SIZE * F1_READ_SECTOR_SIZE] = {};
	BYTE aSubBuf[CD_RAW_READ_SUBCODE_SIZE * F1_READ_SECTOR_SIZE] = {};
	for (INT i = 0; i < F1_READ_SECTOR_SIZE; i++) {
		memcpy(aMainBuf + CD_RAW_SECTOR_SIZE * i, aBuf + F1_BUFFER_SIZE * i, CD_RAW_SECTOR_SIZE);
		AlignRowSubcode(aSubBuf + CD_RAW_READ_SUBCODE_SIZE * i, aBuf + F1_BUFFER_SIZE * i + CD_RAW_SECTOR_SIZE);
	}
	OutputMainInfoLog(OUTPUT_DHYPHEN_PLUS_STR("Cached Main Channel"));
	OutputCDMain(fileMainInfo, aMainBuf, nLBA, CD_RAW_SECTOR_SIZE);

	if (nLBA >= pDisc->SCSI.nAllLength) {
		for (INT x = 0; x < CD_RAW_SECTOR_SIZE * pDisc->MAIN.nAdjustSectorNum - 16; ++x) {
			if (IsValidMainDataHeader(aMainBuf + x)) {
				BYTE m = (BYTE)(aMainBuf[x + 0x0c] ^ 0x01);
				BYTE s = (BYTE)(aMainBuf[x + 0x0d] ^ 0x80);
				BYTE f = (BYTE)(aMainBuf[x + 0x0e]);

				BYTE md = BcdToDec(m);
				BYTE sd = BcdToDec(s);
				BYTE fd = BcdToDec(f);
				INT tmpLBA = MSFtoLBA(md, sd, fd) - 150 + x / CD_RAW_SECTOR_SIZE;
				if (tmpLBA == nLineNum ||
					(pDisc->SCSI.toc.TrackData[0].Control & AUDIO_DATA_TRACK) == 0) {
					OutputLog(standardOut | fileDisc,
						"-----------------------------------------------------\n"
						"Cache SIZE: %u (This size is different every running)\n"
						"-----------------------------------------------------\n"
						, nLineNum
					);
					*lpbCached = TRUE;
				}
				break;
			}
			else if (aSubBuf[13] != 0xaa) {
				OutputLog(standardOut | fileDisc,
					"-----------------------------------------------------\n"
					"Cache SIZE: %u (This size is different every running)\n"
					"-----------------------------------------------------\n"
					, nLineNum
				);
				*lpbCached = TRUE;
			}
		}
		if (!*lpbCached) {
			OutputLog(standardOut | fileDisc, "%02u Cache LBA %06u, SubQ Trk %02x, AMSF %02x:%02x:%02x [Lead-out]\n"
				, nLineNum + 1, nLBA, aSubBuf[13], aSubBuf[19], aSubBuf[20], aSubBuf[21]);
			memcpy(lpOutBuf + F1_BUFFER_SIZE * (nLBA - pDisc->SCSI.nAllLength), aBuf, F1_BUFFER_SIZE);
			(*lpbLeadOutCnt)++;
		}
	}
	else {
		OutputLog(standardOut | fileDisc, "%02u Cache LBA %06u, SubQ Trk %02x, AMSF %02x:%02x:%02x\n"
			, nLineNum + 1, nLBA, aSubBuf[13], aSubBuf[19], aSubBuf[20], aSubBuf[21]);
	}
	return TRUE;
}

BOOL ReadDriveInformation(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	UINT uiCDSpeed
) {
#ifdef _WIN32
	if (*pExecType != drivespeed) {
		BOOL bBusTypeUSB = FALSE;
		if (!StorageQueryProperty(pDevice, &bBusTypeUSB)) {
			return FALSE;
		}
		if (!bBusTypeUSB) {
			if (!ScsiGetAddress(pDevice)) {
				return FALSE;
			}
		}
	}
#else
	pDevice->dwMaxTransferLength = 65536;
#endif
	// 3rd: get drive vendor, product id here (because use IsValidPlextorDrive)
	if (!Inquiry(pExecType, pExtArg, pDevice)) {
		return FALSE;
	}
	if (*pExecType != fd && *pExecType != disk) {
		// 4th: check PLEXTOR or not here (because use modesense and from there)
		if (IsValidPlextorDrive(pDevice)) {
			if ((PLXTR_DRIVE_TYPE)pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::NotLatest) {
				OutputErrorString(
					"[ERROR] This drive isn't latest firmware. Please update\n"
					" -> https://web.archive.org/web/20200128060702/http://www.skcj.co.jp/discon/download/index.html \n"
				);
				return FALSE;
			}
			if ((PLXTR_DRIVE_TYPE)pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::Other) {
				if (*pExecType != drivespeed) {
					if (pExtArg->byPre) {
						SupportIndex0InTrack1(pExtArg, pDevice);
					}
					ReadEeprom(pExtArg, pDevice);
				}
				SetSpeedRead(pExtArg, pDevice, TRUE);
			}
		}
		else {
//			if (pExtArg->byBe) {
//				OutputString("[INFO] /be can be used by plextor drive. /be is ignored\n");
//				pExtArg->byBe = FALSE;
//			}
		}
		// 5th: get currentMedia, if use CD-Text, C2 error, modesense, readbuffercapacity, SetDiscSpeed or not here.
		if (!GetConfiguration(pExecType, pExtArg, pDevice, pDisc)) {
			return FALSE;
		}
		if (*pExecType == drivespeed) {
			pDevice->FEATURE.byModePage2a = TRUE;
		}
		if (*pExecType != drivespeed) {
#if 0
			if (*pExecType == dvd) {
				SetStreaming(pExtArg, pDevice, uiCDSpeed);
			}
			else {
#endif
				if (uiCDSpeed != 0) {
					SetDiscSpeed(pExecType, pExtArg, pDevice, uiCDSpeed);
				}
#if 0
			}
#endif
			ReadBufferCapacity(pExtArg, pDevice);
		}
	}
	ModeSense10(pExecType, pExtArg, pDevice, pDisc);
	return TRUE;
}

BOOL ReadGDForTOC(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	CDB::_READ_CD cdb = {};
	SetReadCDCommand(pDevice, &cdb,
		CDFLAG::_READ_CD::CDDA, 2, CDFLAG::_READ_CD::NoC2, CDFLAG::_READ_CD::NoSub);
	BYTE aToc[CD_RAW_SECTOR_SIZE * 4] = {};
	BYTE bufDec[CD_RAW_SECTOR_SIZE * 2] = {};
	INT nOffset = pDisc->MAIN.nAdjustSectorNum - 1;
	if (pDisc->MAIN.nCombinedOffset < 0) {
		nOffset = pDisc->MAIN.nAdjustSectorNum;
	}
	if (!ExecReadGD(pExtArg, pDevice, pDisc, (LPBYTE)&cdb, FIRST_LBA_FOR_GD + nOffset, 2, aToc, bufDec)) {
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
		OutputErrorString("No GD-ROM data\n");
		return FALSE;
	}
	SetAndOutputTocForGD(pDisc, bufDec);
	return TRUE;
}
