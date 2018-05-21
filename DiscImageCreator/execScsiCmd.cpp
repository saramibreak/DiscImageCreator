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
#include "execIoctl.h"
#include "execScsiCmd.h"
#include "execScsiCmdforCD.h"
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
	CDB::_CDB6GENERIC cdb = { 0 };
	cdb.OperationCode = SCSIOP_TEST_UNIT_READY;
	cdb.LogicalUnitNumber = pDevice->address.Lun;

	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB6GENERIC_LENGTH,
		NULL, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__)
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
	CDB::_CDB6INQUIRY3 cdb = { 0 };
	cdb.OperationCode = SCSIOP_INQUIRY;
	cdb.AllocationLength = sizeof(INQUIRYDATA);

	_declspec(align(4)) INQUIRYDATA inquiryData = { 0 };
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB6GENERIC_LENGTH, &inquiryData, 
		sizeof(INQUIRYDATA), &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	strncpy(pDevice->szVendorId,
		(LPCH)&inquiryData.VendorId, sizeof(pDevice->szVendorId));
	strncpy(pDevice->szProductId,
		(LPCH)&inquiryData.ProductId, sizeof(pDevice->szProductId));
	if (*pExecType != drivespeed) {
		OutputInquiry(&inquiryData);
	}
	return TRUE;
}

BOOL ModeSense(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	if (pDevice->FEATURE.byModePage2a) {
		CDB::_MODE_SENSE cdb = { 0 };
		cdb.OperationCode = SCSIOP_MODE_SENSE;
#if (NTDDI_VERSION <= NTDDI_WIN7)
		cdb.LogicalUnitNumber = pDevice->address.Lun;
#endif
		cdb.PageCode = MODE_PAGE_CAPABILITIES;
		cdb.Pc = 2;// MODE_SENSE_CURRENT_VALUES;
		cdb.AllocationLength = sizeof(CDVD_CAPABILITIES_PAGE_WITH_HEADER);

		_declspec(align(4)) CDVD_CAPABILITIES_PAGE_WITH_HEADER modesense = { 0 };
		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB6GENERIC_LENGTH, &modesense,
			sizeof(CDVD_CAPABILITIES_PAGE_WITH_HEADER), &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
			byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			// not false.
		}
		else {
			if (modesense.cdvd.PageCode == MODE_PAGE_CAPABILITIES) {
				WORD rsm = MAKEWORD(modesense.cdvd.ReadSpeedMaximum[1],
					modesense.cdvd.ReadSpeedMaximum[0]);
				INT perKb = 176;
				if (pDisc->SCSI.wCurrentMedia == ProfileDvdRom) {
					perKb = 1385;
				}
				else if (pDisc->SCSI.wCurrentMedia == ProfileBDRom) {
					perKb = 4496;
				}
				if (*pExecType == drivespeed) {
					OutputString("ReadSpeedMaximum: %uKB/sec (%ux)\n", rsm, rsm / perKb);
				}
				else {
					pDevice->wMaxReadSpeed = rsm;
					OutputModeParmeterHeader(&modesense.header);
					OutputCDVDCapabilitiesPage(&modesense.cdvd, perKb);
				}
			}
			else {
				OutputDriveLogA(
					"SCSIOP_MODE_SENSE didn't fail. But it couldn't get PageCode on this drive\n");
			}
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
	CDB::_START_STOP cdb = { 0 };
	cdb.OperationCode = SCSIOP_START_STOP_UNIT;
	cdb.LogicalUnitNumber = pDevice->address.Lun;
	cdb.Start = Start;
	cdb.LoadEject = LoadEject;

	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB6GENERIC_LENGTH,
		NULL, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
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
	CDB::_READ_TOC cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_TOC;
	cdb.LogicalUnitNumber = pDevice->address.Lun;
	cdb.Format2 = CDROM_READ_TOC_EX_FORMAT_TOC;
	cdb.StartingTrack = 1;
	WORD wSize = CDROM_TOC_SIZE;
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wSize);

#ifdef _DEBUG
	OutputString(_T("pDisc->SCSI.toc address: %p\n"), &pDisc->SCSI.toc);
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, &pDisc->SCSI.toc,	wSize, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		if (pDisc->SCSI.wCurrentMedia != ProfileDvdRom) {
			return FALSE;
		}
	}
	if (pDisc->SCSI.toc.FirstTrack < 1 || 99 < pDisc->SCSI.toc.FirstTrack ||
		pDisc->SCSI.toc.LastTrack < 1 || 99 < pDisc->SCSI.toc.LastTrack) {
		OutputErrorString(_T("Invalid TOC\n"));
		return FALSE;
	}
	if (!InitLBAPerTrack(pExecType, &pDisc)) {
		return FALSE;
	}
	if (byScsiStatus == SCSISTAT_GOOD) {
		pDevice->bySuccessReadToc = TRUE;
		SetAndOutputToc(pDisc);
		if (*pExecType == gd) {
			pDisc->SCSI.trackType = TRACK_TYPE::dataExist;
			OutputDiscLogA("This is the TOC of audio trap disc\n");
		}
	}
	return TRUE;
}

BOOL ReadTOCFull(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	FILE* fpCcd
) {
	CDB::_READ_TOC cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_TOC;
	cdb.LogicalUnitNumber = pDevice->address.Lun;
	cdb.Format2 = CDROM_READ_TOC_EX_FORMAT_FULL_TOC;
	cdb.StartingTrack = 1;
	WORD wSize = sizeof(CDROM_TOC_FULL_TOC_DATA);
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wSize);

	_declspec(align(4)) CDROM_TOC_FULL_TOC_DATA fullToc = { 0 };
#ifdef _DEBUG
	OutputString(_T("fullToc address: %p\n"), &fullToc);
#endif
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	INT nOfs = 0;
	BYTE byMode = DATA_BLOCK_MODE0;
	BYTE bySessionNum = 0;
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, &fullToc,	wSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		DWORD dwBufLen = CD_RAW_SECTOR_SIZE + CD_RAW_READ_SUBCODE_SIZE;
		if (!ReadCDForCheckingSubQAdrFirst(pExtArg, pDevice
			, pDisc, &pBuf, &lpBuf, lpCmd, &dwBufLen, &nOfs, CDFLAG::_READ_CD::All)) {
			return FALSE;
		}
		for (BYTE i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
			if (!ReadCDForCheckingSubQAdr(pExtArg, pDevice, pDisc
				, pDiscPerSector, lpCmd, lpBuf, dwBufLen, nOfs, i, &byMode, 1, fpCcd)) {
				return FALSE;
			}
			if (bySessionNum < 1) {
				WriteCcdForSession(1, byMode, fpCcd);
				bySessionNum++;
			}
			OutputString(
				_T("\rChecking SubQ adr (Track) %2u/%2u"), i + 1, pDisc->SCSI.toc.LastTrack);
		}
		OutputString(_T("\n"));
		pDevice->bySuccessReadTocFull = FALSE;
		return TRUE;
	}
	WORD wFullTocLen = MAKEWORD(fullToc.Length[1], fullToc.Length[0]);
	WORD wTocEntriesAll = wFullTocLen - sizeof(fullToc.Length);
	WORD wTocEntries = wTocEntriesAll / sizeof(CDROM_TOC_FULL_TOC_DATA_BLOCK);

	if (fpCcd) {
		WriteCcdForDisc(wTocEntries, fullToc.LastCompleteSession, fpCcd);
		if (pDevice->FEATURE.byCanCDText) {
			ReadTOCText(pExtArg, pDevice, pDisc, fpCcd);
		}
	}
	pDisc->SCSI.bMultiSession = fullToc.LastCompleteSession > 1 ? TRUE : FALSE;

	WORD wFullTocLenFix = wTocEntriesAll + sizeof(CDROM_TOC_FULL_TOC_DATA);
	// 4 byte padding
	if (wFullTocLenFix % 4) {
		wFullTocLenFix = (WORD)((wFullTocLenFix / 4 + 1) * 4);
	}
	LPBYTE pPFullToc = NULL;
	LPBYTE pFullToc = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pPFullToc,
		wFullTocLenFix, &pFullToc, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
#ifdef _DEBUG
	OutputDiscLogA(
		"FullTocLen: %u, TocEntriesAll: %u, TocEntries: %u, FullTocLenFix: %u\n",
		wFullTocLen, wTocEntriesAll, wTocEntries, wFullTocLenFix);
#endif
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wFullTocLenFix);
	BOOL bRet = TRUE;
	try {
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
			, pFullToc,	wFullTocLenFix, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
		PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData =
			((PCDROM_TOC_FULL_TOC_DATA)pFullToc)->Descriptors;
		DWORD dwBufLen = CD_RAW_SECTOR_SIZE + CD_RAW_READ_SUBCODE_SIZE;
		if (!ReadCDForCheckingSubQAdrFirst(pExtArg
			, pDevice, pDisc, &pBuf, &lpBuf, lpCmd, &dwBufLen, &nOfs, CDFLAG::_READ_CD::All)) {
			throw FALSE;
		}
		for (WORD i = 0; i < wTocEntries; i++) {
			if (pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX40TS) {
				// Somehow Ultraplex seems to get the fulltoc data as "hexadecimal"
				pTocData[i].Msf[0] = BcdToDec(pTocData[i].Msf[0]);
				pTocData[i].Msf[1] = BcdToDec(pTocData[i].Msf[1]);
				pTocData[i].Msf[2] = BcdToDec(pTocData[i].Msf[2]);
				pTocData[i].MsfExtra[0] = BcdToDec(pTocData[i].MsfExtra[0]);
				pTocData[i].MsfExtra[1] = BcdToDec(pTocData[i].MsfExtra[1]);
				pTocData[i].MsfExtra[2] = BcdToDec(pTocData[i].MsfExtra[2]);
				if (pTocData[i].Point < 0xa0) {
					pTocData[i].Point = BcdToDec(pTocData[i].Point);
				}
			}
			if (pTocData[i].Point < 100) {
				if (!ReadCDForCheckingSubQAdr(pExtArg, pDevice, pDisc, pDiscPerSector, lpCmd, lpBuf
					, dwBufLen, nOfs, (BYTE)(pTocData[i].Point - 1), &byMode, pTocData[i].SessionNumber, fpCcd)) {
					throw FALSE;
				}
				if (bySessionNum < pTocData[i].SessionNumber) {
					WriteCcdForSession(pTocData[i].SessionNumber, byMode, fpCcd);
					bySessionNum++;
				}
				OutputString(
					_T("\rChecking SubQ adr (Track) %2u/%2u"), pTocData[i].Point, pDisc->SCSI.toc.LastTrack);
			}
		}
		OutputString(_T("\n"));
		SetAndOutputTocFull(pDisc, &fullToc, pTocData, wTocEntries, fpCcd);
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	pDevice->bySuccessReadTocFull = TRUE;
	FreeAndNull(pPFullToc);
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadTOCAtip(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	CDB::_READ_TOC cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_TOC;
	cdb.LogicalUnitNumber = pDevice->address.Lun;
	cdb.Format2 = CDROM_READ_TOC_EX_FORMAT_ATIP;
	WORD wSize = sizeof(CDROM_TOC_ATIP_DATA);
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wSize);

	_declspec(align(4)) CDROM_TOC_ATIP_DATA atip = { 0 };
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, &atip, wSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputDriveNoSupportLogA(READ_TOC_ATIP);
		return TRUE;
	}
	WORD wTocAtipLen = MAKEWORD(atip.Length[1], atip.Length[0]);
	WORD wTocAtipAll = wTocAtipLen - sizeof(atip.Length);

	LPBYTE pPTocAtip = NULL;
	LPBYTE pTocAtip = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pPTocAtip,
		wTocAtipAll, &pTocAtip, _T(__FUNCTION__), __LINE__)) {
		OutputDriveNoSupportLogA(READ_TOC_ATIP);
		return TRUE;
	}
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wTocAtipAll);
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, pTocAtip, wTocAtipAll, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputDriveNoSupportLogA(READ_TOC_ATIP);
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
	CDB::_READ_TOC cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_TOC;
	cdb.LogicalUnitNumber = pDevice->address.Lun;
	cdb.Format2 = CDROM_READ_TOC_EX_FORMAT_CDTEXT;
	WORD wSize = sizeof(CDROM_TOC_CD_TEXT_DATA);
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wSize);

	_declspec(align(4)) CDROM_TOC_CD_TEXT_DATA tocText = { 0 };
#ifdef _DEBUG
	OutputString(_T("tocText address: %p\n"), &tocText);
#endif
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(CDTEXT));
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, &tocText,	wSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		// not false. because undefined mmc1..
		OutputDriveNoSupportLogA(Nothing CDTEXT or READ_TOC_EX_FORMAT_CDTEXT);
		return TRUE;
	}
	WORD wTocTextLen = MAKEWORD(tocText.Length[1], tocText.Length[0]);
	WORD wTocTextEntriesAll = wTocTextLen - sizeof(tocText.Length);

	WriteCcdForDiscCDTextLength(wTocTextEntriesAll, fpCcd);
	if (!wTocTextEntriesAll) {
		OutputDiscLogA("\tNothing\n");
		// many CD is no text
		return TRUE;
	}

	WORD wTocTextEntries = wTocTextEntriesAll / sizeof(CDROM_TOC_CD_TEXT_DATA_BLOCK);
	WriteCcdForCDText(wTocTextEntries, fpCcd);

	WORD wTocTextLenFix = wTocTextEntriesAll + sizeof(CDROM_TOC_CD_TEXT_DATA);
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
	OutputDiscLogA(
		"TocTextLen: %u, TocTextEntriesAll: %u, TocTextEntries: %u, TocTextLenFix: %u\n",
		wTocTextLen, wTocTextEntriesAll, wTocTextEntries, wTocTextLenFix);
	OutputString(_T("pPTocText address: %p\n"), &pPTocText);
	OutputString(_T("pTocText address: %p\n"), &pTocText);
#endif
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wTocTextLenFix);
	LPCH pTmpText = NULL;
	BOOL bRet = TRUE;
	try {
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
			, pTocText,	wTocTextLenFix, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
		PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc =
			((PCDROM_TOC_CD_TEXT_DATA)pTocText)->Descriptors;
		WriteCcdForCDTextEntry(pDesc, wTocTextEntries, fpCcd);

		WORD wAllTextSize = wTocTextEntries * sizeof(pDesc->Text);
		if (NULL == (pTmpText = (LPCH)calloc(wAllTextSize, sizeof(_TCHAR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		WORD wEntrySize = 0;
		BOOL bUnicode = FALSE;
		while (wEntrySize < wTocTextEntries) {
			if (pDesc[wEntrySize].Unicode == 1) {
				bUnicode = TRUE;
				break;
			}
			wEntrySize++;
		}
		SetAndOutputTocCDText(pDisc, pDesc, pTmpText, wEntrySize, wAllTextSize);
		if (bUnicode) {
			PWCHAR pTmpWText = NULL;
			if (NULL == (pTmpWText = (PWCHAR)calloc(wAllTextSize, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			SetAndOutputTocCDWText(pDesc,
				pTmpText, wEntrySize, wTocTextEntries, wAllTextSize);
			FreeAndNull(pTmpWText);
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
	CDB::_GET_CONFIGURATION cdb = { 0 };
	cdb.OperationCode = SCSIOP_GET_CONFIGURATION;
	cdb.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_CURRENT;
	cdb.StartingFeature[1] = FeatureProfileList;
	WORD wSize = sizeof(GET_CONFIGURATION_HEADER);
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wSize);

	_declspec(align(4)) GET_CONFIGURATION_HEADER configHeader = { 0 };
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, &configHeader, wSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		pDisc->SCSI.wCurrentMedia = ProfileCdrom;
		// not false. because undefined mmc1..
		if (pDevice->byPlxtrDrive == PLXTR_DRIVE_TYPE::PX40TS) {
			pDevice->FEATURE.byCanCDText = TRUE;
			pDevice->FEATURE.byC2ErrorData = TRUE;
		}
		OutputDriveNoSupportLogA(GET_CONFIGURATION);
	}
	else {
		pDisc->SCSI.wCurrentMedia =
			MAKEWORD(configHeader.CurrentProfile[1], configHeader.CurrentProfile[0]);
		if (*pExecType != drivespeed) {
			if (pDisc->SCSI.wCurrentMedia == ProfileInvalid) {
				OutputDriveLogA(
					"SCSIOP_GET_CONFIGURATION didn't fail. But it couldn't get CurrentMedia on this drive\n");
				if (*pExecType == dvd) {
					OutputDriveLogA("\tSet CurrentMedia to DVD-ROM\n");
					configHeader.CurrentProfile[1] = ProfileDvdRom;
					pDisc->SCSI.wCurrentMedia = ProfileDvdRom;
				}
				else {
					OutputDriveLogA("\tSet CurrentMedia to CD-ROM\n");
					configHeader.CurrentProfile[1] = ProfileCdrom;
					pDisc->SCSI.wCurrentMedia = ProfileCdrom;
				}
			}
			OutputGetConfigurationHeader(&configHeader);

			DWORD dwAllLen =
				MAKELONG(MAKEWORD(configHeader.DataLength[3], configHeader.DataLength[2]),
					MAKEWORD(configHeader.DataLength[1], configHeader.DataLength[0])) -
				sizeof(configHeader.DataLength) + sizeof(GET_CONFIGURATION_HEADER);
			LPBYTE pPConf = NULL;
			LPBYTE lpConf = NULL;
			if (!GetAlignedCallocatedBuffer(pDevice, &pPConf,
				dwAllLen, &lpConf, _T(__FUNCTION__), __LINE__)) {
				return FALSE;
			}
			REVERSE_BYTES_SHORT(&cdb.AllocationLength, &dwAllLen);

			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
				, lpConf, dwAllLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				// not false. because undefined mmc1..
				OutputDriveNoSupportLogA(GET_CONFIGURATION);
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
	CDB::_READ_DISK_INFORMATION cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_DISC_INFORMATION;
	cdb.Lun = pDevice->address.Lun;
	WORD wSize = sizeof(DISC_INFORMATION);
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wSize);

	_declspec(align(4)) DISC_INFORMATION discInformation = { 0 };
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
		, &discInformation,	wSize, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		// not false.
		OutputDriveNoSupportLogA(READ_DISC_INFORMATION);
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
//	if (pDevice->FEATURE.byModePage2a) {
		CDB::_MODE_SENSE10 cdb = { 0 };
		cdb.OperationCode = SCSIOP_MODE_SENSE10;
#if (NTDDI_VERSION <= NTDDI_WIN7)
		cdb.LogicalUnitNumber = pDevice->address.Lun;
#endif
		cdb.PageCode = MODE_PAGE_CAPABILITIES;
		cdb.Pc = 2;// MODE_SENSE_CURRENT_VALUES;
		WORD wSize = sizeof(CDVD_CAPABILITIES_PAGE_WITH_HEADER10);
		REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wSize);

		_declspec(align(4)) CDVD_CAPABILITIES_PAGE_WITH_HEADER10 modesense = { 0 };
		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB10GENERIC_LENGTH
			, &modesense, wSize, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
			byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			// not false.
			OutputDriveNoSupportLogA(MODE_SENSE10);
			ModeSense(pExecType, pExtArg, pDevice, pDisc);
		}
		else {
			if (modesense.cdvd.PageCode == MODE_PAGE_CAPABILITIES) {
				WORD rsm = MAKEWORD(modesense.cdvd.ReadSpeedMaximum[1],
					modesense.cdvd.ReadSpeedMaximum[0]);
				INT perKb = 176;
				if (pDisc->SCSI.wCurrentMedia == ProfileDvdRom) {
					perKb = 1385;
				}
				else if (pDisc->SCSI.wCurrentMedia == ProfileBDRom) {
					perKb = 4496;
				}
				if (*pExecType == drivespeed) {
					OutputString("ReadSpeedMaximum: %uKB/sec (%ux)\n", rsm, rsm / perKb);
				}
				else {
					pDevice->wMaxReadSpeed = rsm;
					OutputModeParmeterHeader10(&modesense.header);
					OutputCDVDCapabilitiesPage(&modesense.cdvd, perKb);
				}
			}
			else {
				OutputDriveLogA(
					"SCSIOP_MODE_SENSE10 didn't fail. But it couldn't get PageCode on this drive\n");
				ModeSense(pExecType, pExtArg, pDevice, pDisc);
			}
		}
//	}
//	else {
//		OutputDriveNoSupportLogA(MODE_SENSE10);
//	}
	return TRUE;
}

BOOL ReadBufferCapacity(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	if (pDevice->FEATURE.byReadBufCapa) {
		CDB::_READ_BUFFER_CAPACITY cdb = { 0 };
		cdb.OperationCode = SCSIOP_READ_BUFFER_CAPACITY;
		WORD wSize = sizeof(READ_BUFFER_CAPACITY_DATA);
		REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wSize);

		_declspec(align(4)) READ_BUFFER_CAPACITY_DATA readBufCapaData = { 0 };
		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH
			, &readBufCapaData,	wSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			// not false.
			OutputDriveNoSupportLogA(READ_BUFFER_CAPACITY);
		}
		else {
			OutputReadBufferCapacity(&readBufCapaData);
		}
	}
	return TRUE;
}

BOOL SetDiscSpeed(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	DWORD dwDiscSpeedNum
) {
	if ((*pExecType == cd || *pExecType == gd || *pExecType == audio || *pExecType == data)
		&& pDevice->FEATURE.bySetCDSpeed || *pExecType == dvd) {
		WORD wSpeed = 0;
		// https://msdn.microsoft.com/en-us/library/windows/hardware/ff551368(v=vs.85).aspx
		// https://msdn.microsoft.com/ja-jp/library/ff551396(v=vs.85).aspx
		_declspec(align(4)) CDROM_SET_SPEED setspeed = { CdromSetSpeed, 0, 0, CdromDefaultRotation };
		if ((*pExecType == cd || *pExecType == gd || *pExecType == audio || *pExecType == data) &&
			0 < dwDiscSpeedNum && dwDiscSpeedNum <= CD_DRIVE_MAX_SPEED) {
			// http://senbee.seesaa.net/article/26247159.html
			// 2048 x 75 = 153600 B -> 150 KiB
			// 2352 x 75 = 176400 B -> 172,265625 KiB
			wSpeed = (WORD)(CD_RAW_SECTOR_SIZE * 75 * dwDiscSpeedNum / 1000);
			setspeed.ReadSpeed = wSpeed;
		}
#if 1
		else if (*pExecType == dvd &&
			0 < dwDiscSpeedNum && dwDiscSpeedNum <= DVD_DRIVE_MAX_SPEED) {
			// Read and write speeds for the first DVD drives and players were of
			// 1,385 kB/s (1,353 KiB/s); this speed is usually called "1x".
			// 2048 x 75 x 9 = 1384448 B -> 1352 KiB
			wSpeed = (WORD)(DISC_RAW_READ_SIZE * 676 * dwDiscSpeedNum / 1000);
			setspeed.ReadSpeed = wSpeed;
		}
#endif
		else {
			wSpeed = 0xffff;
			setspeed.ReadSpeed = pDevice->wMaxReadSpeed;
		}
		CDB::_SET_CD_SPEED cdb = { 0 };
		cdb.OperationCode = SCSIOP_SET_CD_SPEED;
		REVERSE_BYTES_SHORT(&cdb.ReadSpeed, &wSpeed);
		// https://msdn.microsoft.com/en-us/library/windows/hardware/ff551370(v=vs.85).aspx
		setspeed.RequestType = CdromSetSpeed;

		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, &setspeed, 
			sizeof(CDROM_SET_SPEED), &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			// Somehow PX-W1210S fails...
			OutputDriveNoSupportLogA(SET_CD_SPEED);
			OutputDriveLogA("Or if you use the SATA/IDE to USB adapter, doesn't support this command\n");
		}
		else {
			OutputSetSpeed(&setspeed);
			OutputString(_T("Set the drive speed: %uKB/sec\n"), setspeed.ReadSpeed);
		}
	}
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
		BYTE buf[size] = { 0 };

		CDB::_CDB12 cdb = { 0 };
		cdb.OperationCode = SCSIOP_PLXTR_EXTEND;
		cdb.DisablePageOut = TRUE;
		cdb.LogicalBlock[0] = PLXTR_FLAG_SPEED_READ;
		cdb.LogicalBlock[1] = (BYTE)bState;
		cdb.Reserved2 = 0x08;

		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH
			, buf, sizeof(buf), &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
#if 0
		for (INT i = 0; i < size; i++) {
			OutputString(_T("%02x "), buf[i]);
		}
		OutputString(_T("\n"));
#endif
	}
	return TRUE;
}

BOOL Reset(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	CDB::_CDB6GENERIC cdb = { 0 };
	cdb.OperationCode = SCSIOP_PLXTR_RESET;

	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB6GENERIC_LENGTH
		, NULL, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

BOOL ReadEeprom(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	DWORD tLen = 128;
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
	}
	DWORD BufLen = tLen;
	LPBYTE pPBuf = NULL;
	LPBYTE pBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pPBuf,
		BufLen, &pBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	CDB::_CDB12 cdb = { 0 };
	cdb.OperationCode = SCSIOP_PLXTR_READ_EEPROM;
	cdb.RelativeAddress = (BYTE)bHigh;

	BOOL bRet = TRUE;
	BYTE byScsiStatus = 0;
	OutputDriveLogA(OUTPUT_DHYPHEN_PLUS_STR(EEPROM));
	try {
		for (BYTE idx = 0; idx < 4; idx++) {
			REVERSE_BYTES(&cdb.TransferLength, &tLen);
			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH
				, pBuf, BufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
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

BOOL ReadDriveInformation(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	DWORD dwCDSpeed
) {
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
	// 3rd: get drive vendor, product id here (because use IsValidPlextorDrive)
	if (!Inquiry(pExecType, pExtArg, pDevice)) {
		return FALSE;
	}
	// 4th: check PLEXTOR or not here (because use modesense and from there)
	IsValidPlextorDrive(pDevice);
	if ((PLXTR_DRIVE_TYPE)pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::No) {
		if (*pExecType != drivespeed) {
			if (pExtArg->byPre) {
				SupportIndex0InTrack1(pExtArg, pDevice);
			}
			ReadEeprom(pExtArg, pDevice);
		}
		SetSpeedRead(pExtArg, pDevice, TRUE);
	}
	// 5th: get currentMedia, if use CD-Text, C2 error, modesense, readbuffercapacity, SetDiscSpeed or not here.
	if (!GetConfiguration(pExecType, pExtArg, pDevice, pDisc)) {
		return FALSE;
	}
	if (*pExecType == drivespeed) {
		pDevice->FEATURE.byModePage2a = TRUE;
	}
	ModeSense10(pExecType, pExtArg, pDevice, pDisc);
	if (*pExecType != drivespeed) {
#if 0
		if (*pExecType == dvd) {
			SetStreaming(pDevice, dwCDSpeed);
		}
		else {
#endif
			if (dwCDSpeed != 0) {
				SetDiscSpeed(pExecType, pExtArg, pDevice, dwCDSpeed);
			}
#if 0
		}
#endif
		ReadBufferCapacity(pExtArg, pDevice);
	}
	return TRUE;
}

BOOL ExecReadCD(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPBYTE lpCmd,
	INT nLBA,
	LPBYTE lpBuf,
	DWORD dwBufSize,
	LPCTSTR pszFuncName,
	LONG lLineNum
) {
	REVERSE_BYTES(&lpCmd[2], &nLBA);
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB12GENERIC_LENGTH
		, lpBuf, dwBufSize, &byScsiStatus, pszFuncName, lLineNum)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputLogA(standardError | fileMainError,
			"lpCmd: %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x\n"
			"dwBufSize: %lu\n"
			, lpCmd[0], lpCmd[1], lpCmd[2], lpCmd[3], lpCmd[4], lpCmd[5]
			, lpCmd[6], lpCmd[7], lpCmd[8], lpCmd[9], lpCmd[10], lpCmd[11]
			, dwBufSize
		);
		return FALSE;
	}
	return TRUE;
}

VOID ManageEndOfDirectoryRecord(
	LPINT nSectorNum,
	BYTE byTransferLen,
	UINT uiZeroPaddingNum,
	LPBYTE lpDirRec,
	LPUINT nOfs
) {
	if (*nSectorNum < byTransferLen) {
		UINT j = 0;
		for (; j < uiZeroPaddingNum; j++) {
			if (lpDirRec[j] != 0) {
				break;
			}
		}
		if (j == uiZeroPaddingNum) {
			*nOfs += uiZeroPaddingNum;
			(*nSectorNum)++;
			return;
		}
	}
	else {
		return;
	}
}

BOOL ExecReadDisc(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE pCdb,
	INT nLBA,
	LPBYTE lpBuf,
	LPBYTE bufDec,
	BYTE byTransferLen
) {
	if (*pExecType == gd) {
		if (!ExecReadGD(pExtArg, pDevice, pDisc, pCdb, nLBA, byTransferLen, lpBuf, bufDec)) {
			return FALSE;
		}
		for (BYTE i = 0; i < byTransferLen; i++) {
			memcpy(lpBuf + DISC_RAW_READ_SIZE * i, bufDec + CD_RAW_SECTOR_SIZE * i + 16, DISC_RAW_READ_SIZE);
		}
	}
	else {
		if (!ExecReadCD(pExtArg, pDevice, pCdb, nLBA, lpBuf,
			(DWORD)(DISC_RAW_READ_SIZE * byTransferLen), _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL ReadDirectoryRecordDetail(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE pCdb,
	INT nLBA,
	LPBYTE lpBuf,
	LPBYTE bufDec,
	BYTE byTransferLen,
	INT nDirPosNum,
	DWORD dwLogicalBlkCoef,
	INT nOffset,
	PDIRECTORY_RECORD pDirRec
) {
	if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc
		, pCdb, nLBA + nOffset, lpBuf, bufDec, byTransferLen)) {
		return FALSE;
	}
	for (BYTE i = 0; i < byTransferLen; i++) {
		OutputCDMain(fileMainInfo, lpBuf + DISC_RAW_READ_SIZE * i, nLBA + i, DISC_RAW_READ_SIZE);
	}

	UINT uiOfs = 0;
	for (INT nSectorNum = 0; nSectorNum < byTransferLen;) {
		if (*(lpBuf + uiOfs) == 0) {
			break;
		}
		OutputVolDescLogA(
			OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Directory Record), nLBA + nSectorNum, nLBA + nSectorNum);
		for (;;) {
			CHAR szCurDirName[MAX_FNAME_FOR_VOLUME] = { 0 };
			LPBYTE lpDirRec = lpBuf + uiOfs;
			if (lpDirRec[0] >= MIN_LEN_DR) {
				if (lpDirRec[0] == MIN_LEN_DR && uiOfs > 0 && uiOfs % DISC_RAW_READ_SIZE == 0) {
					// SimCity 3000 (USA)
					OutputVolDescLogA(
						"Direcory record size of the %d sector maybe incorrect. Skip the reading of this sector\n", nLBA);
					nSectorNum++;
					break;
				}
				DWORD dwMaxByte = DWORD(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE);
				if (pDisc->SCSI.nAllLength >= 0x200000) {
					dwMaxByte = 0xffffffff;
				}
				DWORD dwExtentPos = GetSizeOrDwordForVolDesc(lpDirRec + 2, dwMaxByte) / dwLogicalBlkCoef;
				DWORD dwDataLen = GetSizeOrDwordForVolDesc(lpDirRec + 10, dwMaxByte);
				if (dwDataLen >= dwMaxByte) {
					OutputVolDescLogA(
						"Data length is incorrect. Skip the reading of this sector\n");
					nSectorNum++;
					break;
				}
				OutputFsDirectoryRecord(
					pExtArg, pDisc, lpDirRec, dwExtentPos, dwDataLen, szCurDirName);
				OutputVolDescLogA("\n");
				uiOfs += lpDirRec[0];

				if ((lpDirRec[25] & 0x02 || (pDisc->SCSI.byCdi && lpDirRec[25] == 0))
					&& !(lpDirRec[32] == 1 && szCurDirName[0] == 0)
					&& !(lpDirRec[32] == 1 && szCurDirName[0] == 1)) {
					// not upper and current directory 
					for (INT i = 1; i < nDirPosNum; i++) {
						if (dwExtentPos == pDirRec[i].uiPosOfDir &&
							!_strnicmp(szCurDirName, pDirRec[i].szDirName, MAX_FNAME_FOR_VOLUME)) {
							pDirRec[i].uiDirSize = PadSizeForVolDesc(dwDataLen);
							break;
						}
					}
				}
				if (uiOfs == (UINT)(DISC_RAW_READ_SIZE * (nSectorNum + 1))) {
					nSectorNum++;
					break;
				}
			}
			else {
				UINT uiZeroPaddingNum = DISC_RAW_READ_SIZE * (nSectorNum + 1) - uiOfs;
				if (uiZeroPaddingNum > MIN_LEN_DR) {
					BYTE byNextLenDR = lpDirRec[MIN_LEN_DR];
					if (byNextLenDR >= MIN_LEN_DR) {
						// Amiga Tools 4 : The second of Direcory Record (0x22 - 0x43) is corrupt...
						// ========== LBA[040915, 0x09fd3]: Main Channel ==========
						//        +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F
						// 0000 : 22 00 D3 9F 00 00 00 00  9F D3 00 08 00 00 00 00   "...............
						// 0010 : 08 00 60 02 1D 17 18 2C  00 02 00 00 01 00 00 01   ..`....,........
						// 0020 : 01 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00   ................
						// 0030 : 00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00   ................
						// 0040 : 00 00 01 01 2E 00 09 A0  00 00 00 00 A0 09 D8 01   ................
						OutputMainErrorWithLBALogA(
							"Direcory Record is corrupt. Skip reading from %d to %d byte. See _mainInfo.txt\n"
							, nLBA, 0, uiOfs, uiOfs + MIN_LEN_DR - 1);
						uiOfs += MIN_LEN_DR;
						break;
					}
					else {
						ManageEndOfDirectoryRecord(&nSectorNum, byTransferLen, uiZeroPaddingNum, lpDirRec, &uiOfs);
						break;
					}
				}
				else {
					ManageEndOfDirectoryRecord(&nSectorNum, byTransferLen, uiZeroPaddingNum, lpDirRec, &uiOfs);
					break;
				}
			}
		}
	}
	return TRUE;
}

BOOL ReadDirectoryRecord(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE pCdb,
	LPBYTE lpBuf,
	DWORD dwLogicalBlkCoef,
	DWORD dwRootDataLen,
	INT nSectorOfs,
	PDIRECTORY_RECORD pDirRec,
	INT nDirPosNum
) {
	LPBYTE bufDec = NULL;
	if (*pExecType == gd) {
		bufDec = (LPBYTE)calloc(pDevice->dwMaxTransferLength, sizeof(BYTE));
		if (!bufDec) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	BYTE byTransferLen = 1;
	// for CD-I
	if (dwRootDataLen == 0) {
		if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc, pCdb
			, (INT)pDirRec[0].uiPosOfDir + nSectorOfs, lpBuf, bufDec, byTransferLen)) {
			return FALSE;
		}
		dwRootDataLen =
			PadSizeForVolDesc(GetSizeOrDwordForVolDesc(lpBuf + 10, (DWORD)(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE)));
	}
	pDirRec[0].uiDirSize = dwRootDataLen;

	for (INT nDirRecIdx = 0; nDirRecIdx < nDirPosNum; nDirRecIdx++) {
		INT nLBA = (INT)pDirRec[nDirRecIdx].uiPosOfDir;
		if (pDirRec[nDirRecIdx].uiDirSize > pDevice->dwMaxTransferLength) {
			// [FMT] Psychic Detective Series Vol. 4 - Orgel (Japan) (v1.0)
			// [FMT] Psychic Detective Series Vol. 5 - Nightmare (Japan)
			// [IBM - PC compatible] Maria 2 - Jutai Kokuchi no Nazo (Japan) (Disc 1)
			// [IBM - PC compatible] PC Game Best Series Vol. 42 - J.B. Harold Series - Kiss of Murder - Satsui no Kuchizuke (Japan)
			// [SS] Madou Monogatari (Japan)
			// and more
			DWORD dwAdditionalTransferLen = pDirRec[nDirRecIdx].uiDirSize / pDevice->dwMaxTransferLength;
			SetCommandForTransferLength(pExecType, pDevice, pCdb, pDevice->dwMaxTransferLength, &byTransferLen);
			OutputMainInfoLogA("nLBA %d, uiDirSize: %lu, byTransferLen: %d [L:%d]\n"
				, nLBA, pDevice->dwMaxTransferLength, byTransferLen, (INT)__LINE__);

			for (DWORD n = 0; n < dwAdditionalTransferLen; n++) {
				if (!ReadDirectoryRecordDetail(pExecType, pExtArg, pDevice, pDisc, pCdb, nLBA
					, lpBuf, bufDec, byTransferLen, nDirPosNum, dwLogicalBlkCoef, nSectorOfs, pDirRec)) {
					continue;
				}
				nLBA += byTransferLen;
			}
			DWORD dwLastTblSize = pDirRec[nDirRecIdx].uiDirSize % pDevice->dwMaxTransferLength;
			SetCommandForTransferLength(pExecType, pDevice, pCdb, dwLastTblSize, &byTransferLen);
			OutputMainInfoLogA("nLBA %d, uiDirSize: %lu, byTransferLen: %d [L:%d]\n"
				, nLBA, dwLastTblSize, byTransferLen, (INT)__LINE__);

			if (!ReadDirectoryRecordDetail(pExecType, pExtArg, pDevice, pDisc, pCdb, nLBA
				, lpBuf, bufDec, byTransferLen, nDirPosNum, dwLogicalBlkCoef, nSectorOfs, pDirRec)) {
				continue;
			}
		}
		else {
			if (pDirRec[nDirRecIdx].uiDirSize == 0 || &byTransferLen == 0) {
				OutputMainErrorLogA("Directory Record is invalid\n");
				return FALSE;
			}
			SetCommandForTransferLength(pExecType, pDevice, pCdb, pDirRec[nDirRecIdx].uiDirSize, &byTransferLen);
			OutputMainInfoLogA("nLBA %d, uiDirSize: %u, byTransferLen: %d [L:%d]\n"
				, nLBA, pDirRec[nDirRecIdx].uiDirSize, byTransferLen, (INT)__LINE__);

			if (!ReadDirectoryRecordDetail(pExecType, pExtArg, pDevice, pDisc, pCdb, nLBA
				, lpBuf, bufDec, byTransferLen, nDirPosNum, dwLogicalBlkCoef, nSectorOfs, pDirRec)) {
				continue;
			}
		}
		OutputString(_T("\rReading DirectoryRecord %4d/%4d"), nDirRecIdx + 1, nDirPosNum);
	}
	OutputString(_T("\n"));
	return TRUE;
}

BOOL ReadPathTableRecord(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE pCdb,
	DWORD dwLogicalBlkCoef,
	DWORD dwPathTblSize,
	DWORD dwPathTblPos,
	INT nSectorOfs,
	PDIRECTORY_RECORD pDirRec,
	LPINT nDirPosNum
) {
	BYTE byTransferLen = 1;
	SetCommandForTransferLength(pExecType, pDevice, pCdb, dwPathTblSize, &byTransferLen);

	DWORD dwBufSize = 0;
	if (*pExecType == gd) {
		dwBufSize = (CD_RAW_SECTOR_SIZE - (dwPathTblSize % CD_RAW_SECTOR_SIZE) + dwPathTblSize) * byTransferLen * 2;
	}
	else {
		dwBufSize = DISC_RAW_READ_SIZE - (dwPathTblSize % DISC_RAW_READ_SIZE) + dwPathTblSize;
	}
	LPBYTE lpBuf = (LPBYTE)calloc(dwBufSize, sizeof(BYTE));
	if (!lpBuf) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	
	BOOL bRet = TRUE;
	try {
		LPBYTE bufDec = NULL;
		if (*pExecType == gd) {
			if (NULL == (bufDec = (LPBYTE)calloc(size_t(CD_RAW_SECTOR_SIZE * byTransferLen), sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
		}
		if (dwPathTblSize > pDevice->dwMaxTransferLength) {
			DWORD dwAdditionalTransferLen = dwPathTblSize / pDevice->dwMaxTransferLength;
			SetCommandForTransferLength(pExecType, pDevice, pCdb, pDevice->dwMaxTransferLength, &byTransferLen);
			OutputMainInfoLogA("dwPathTblSize: %lu, byTransferLen: %d [L:%d]\n"
				, pDevice->dwMaxTransferLength, byTransferLen, (INT)__LINE__);

			for (DWORD n = 0; n < dwAdditionalTransferLen; n++) {
				if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc, pCdb
					, (INT)dwPathTblPos + nSectorOfs, lpBuf + pDevice->dwMaxTransferLength * n, bufDec, byTransferLen)) {
					throw FALSE;
				}
				for (BYTE i = 0; i < byTransferLen; i++) {
					OutputCDMain(fileMainInfo, lpBuf + DISC_RAW_READ_SIZE * i, (INT)dwPathTblPos + i, DISC_RAW_READ_SIZE);
				}
				dwPathTblPos += byTransferLen;
			}
			DWORD dwLastPathTblSize = dwPathTblSize % pDevice->dwMaxTransferLength;
			SetCommandForTransferLength(pExecType, pDevice, pCdb, dwLastPathTblSize, &byTransferLen);
			OutputMainInfoLogA(
				"dwPathTblSize: %lu, byTransferLen: %d [L:%d]\n", dwLastPathTblSize, byTransferLen, (INT)__LINE__);

			DWORD dwBufOfs = pDevice->dwMaxTransferLength * dwAdditionalTransferLen;
			if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc, pCdb
				, (INT)dwPathTblPos + nSectorOfs, lpBuf + dwBufOfs, bufDec, byTransferLen)) {
				throw FALSE;
			}
			for (BYTE i = 0; i < byTransferLen; i++) {
				OutputCDMain(fileMainInfo, lpBuf + dwBufOfs + DISC_RAW_READ_SIZE * i, (INT)dwPathTblPos + i, DISC_RAW_READ_SIZE);
			}
			if (!OutputFsPathTableRecord(pDisc, lpBuf, dwLogicalBlkCoef, dwPathTblPos, dwPathTblSize, pDirRec, nDirPosNum)) {
				throw FALSE;
			}
		}
		else {
			if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc, pCdb
				, (INT)dwPathTblPos + nSectorOfs, lpBuf, bufDec, byTransferLen)) {
				throw FALSE;
			}
			OutputMainInfoLogA(
				"dwPathTblSize: %lu, byTransferLen: %d [L:%d]\n", dwPathTblSize, byTransferLen, (INT)__LINE__);
			for (BYTE i = 0; i < byTransferLen; i++) {
				OutputCDMain(fileMainInfo, lpBuf + DISC_RAW_READ_SIZE * i, (INT)dwPathTblPos + i, DISC_RAW_READ_SIZE);
			}
			if (!OutputFsPathTableRecord(pDisc, lpBuf, dwLogicalBlkCoef, dwPathTblPos, dwPathTblSize, pDirRec, nDirPosNum)) {
				throw FALSE;
			}
		}
		OutputVolDescLogA("Directory Num: %u\n", *nDirPosNum);
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FreeAndNull(lpBuf);
	return bRet;
}

BOOL ReadVolumeDescriptor(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	BYTE byIdx,
	LPBYTE pCdb,
	LPBYTE lpBuf,
	INT nPVD,
	INT nSectorOfs,
	LPBOOL lpReadVD,
	PVOLUME_DESCRIPTOR pVolDesc
) {
	if (pDisc->SCSI.lpFirstLBAListOnToc) {
		nPVD += pDisc->SCSI.lpFirstLBAListOnToc[byIdx];
	}
	INT nTmpLBA = nPVD;
	BYTE bufDec[CD_RAW_SECTOR_SIZE] = { 0 };
	for (;;) {
		if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc, pCdb, nTmpLBA + nSectorOfs, lpBuf, bufDec, 1)) {
			break;
		}
		if (!strncmp((LPCH)&lpBuf[1], "CD001", 5) ||
			(pDisc->SCSI.byCdi && !strncmp((LPCH)&lpBuf[1], "CD-I ", 5))) {
			if (nTmpLBA == nPVD) {
				DWORD dwLogicalBlkSize = GetSizeOrWordForVolDesc(lpBuf + 128);
				pVolDesc->ISO_9660.dwLogicalBlkCoef = (BYTE)(DISC_RAW_READ_SIZE / dwLogicalBlkSize);
				pVolDesc->ISO_9660.dwPathTblSize =
					GetSizeOrDwordForVolDesc(lpBuf + 132, (DWORD)(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE));
				pVolDesc->ISO_9660.dwPathTblPos = MAKEDWORD(MAKEWORD(lpBuf[140], lpBuf[141]),
					MAKEWORD(lpBuf[142], lpBuf[143])) / pVolDesc->ISO_9660.dwLogicalBlkCoef;
				if (pVolDesc->ISO_9660.dwPathTblPos == 0) {
					pVolDesc->ISO_9660.dwPathTblPos = MAKEDWORD(MAKEWORD(lpBuf[151], lpBuf[150]),
						MAKEWORD(lpBuf[149], lpBuf[148]));
				}
				pVolDesc->ISO_9660.dwRootDataLen =
					GetSizeOrDwordForVolDesc(lpBuf + 166, (DWORD)(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE));
				if (pVolDesc->ISO_9660.dwRootDataLen > 0) {
					pVolDesc->ISO_9660.dwRootDataLen = PadSizeForVolDesc(pVolDesc->ISO_9660.dwRootDataLen);
				}
				*lpReadVD = TRUE;
			}
			else if (lpBuf[0] == 2) {
				DWORD dwLogicalBlkSize = GetSizeOrWordForVolDesc(lpBuf + 128);
				pVolDesc->JOLIET.dwLogicalBlkCoef = (BYTE)(DISC_RAW_READ_SIZE / dwLogicalBlkSize);
				pVolDesc->JOLIET.dwPathTblSize =
					GetSizeOrDwordForVolDesc(lpBuf + 132, (DWORD)(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE));
				pVolDesc->JOLIET.dwPathTblPos = MAKEDWORD(MAKEWORD(lpBuf[140], lpBuf[141]),
					MAKEWORD(lpBuf[142], lpBuf[143])) / pVolDesc->JOLIET.dwLogicalBlkCoef;
				if (pVolDesc->JOLIET.dwPathTblPos == 0) {
					pVolDesc->JOLIET.dwPathTblPos = MAKEDWORD(MAKEWORD(lpBuf[151], lpBuf[150]),
						MAKEWORD(lpBuf[149], lpBuf[148]));
				}
				pVolDesc->JOLIET.dwRootDataLen =
					GetSizeOrDwordForVolDesc(lpBuf + 166, (DWORD)(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE));
				if (pVolDesc->JOLIET.dwRootDataLen > 0) {
					pVolDesc->JOLIET.dwRootDataLen = PadSizeForVolDesc(pVolDesc->JOLIET.dwRootDataLen);
				}
				*lpReadVD = TRUE;
			}
			OutputCDMain(fileMainInfo, lpBuf, nTmpLBA, DISC_RAW_READ_SIZE);
			OutputFsVolumeDescriptor(pExtArg, pDisc, lpBuf, nTmpLBA++);
		}
		else {
			break;
		}
	}
	return TRUE;
}

// These global variable is set at prngcd.cpp
extern unsigned char scrambled_table[2352];

BOOL ExecReadGD(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE pCdb,
	INT nLBA,
	BYTE byTransferLen,
	LPBYTE lpInBuf,
	LPBYTE lpOutBuf
) {
	for (INT n = 1; n <= 10; n++) {
		BOOL bRet = TRUE;
		if (!ExecReadCD(pExtArg, pDevice, pCdb, nLBA, lpInBuf,
			CD_RAW_SECTOR_SIZE * (DWORD)byTransferLen, _T(__FUNCTION__), __LINE__)) {
			if (n == 10) {
				return FALSE;
			}
			StartStopUnit(pExtArg, pDevice, STOP_UNIT_CODE, STOP_UNIT_CODE);
			DWORD milliseconds = 10000;
			OutputErrorString(_T("Retry %d/10 after %ld milliseconds\n"), n, milliseconds);
			Sleep(milliseconds);
			bRet = FALSE;
		}
		if (!bRet) {
			continue;
		}
		else {
			break;
		}
	}
	INT nOfs = pDisc->MAIN.nCombinedOffset;
	if (pDisc->MAIN.nCombinedOffset < 0) {
		nOfs = CD_RAW_SECTOR_SIZE + pDisc->MAIN.nCombinedOffset;
	}
#if 0
	OutputCDMain(fileMainInfo, lpInBuf + idx, nLBA, CD_RAW_SECTOR_SIZE);
#endif
	for (BYTE i = 0; i < byTransferLen; i++) {
		for (INT j = 0; j < CD_RAW_SECTOR_SIZE; j++) {
			lpOutBuf[j + CD_RAW_SECTOR_SIZE * i] =
				(BYTE)(lpInBuf[(nOfs + j) + CD_RAW_SECTOR_SIZE * i] ^ scrambled_table[j]);
		}
	}
#if 0
	OutputCDMain(fileMainInfo, lpOutBuf, nLBA, CD_RAW_SECTOR_SIZE);
#endif
	return TRUE;
}
