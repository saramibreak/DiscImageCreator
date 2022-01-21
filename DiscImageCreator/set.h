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
#pragma once
#include "enum.h"

VOID SetReadCDCommand(
	PDEVICE pDevice,
	CDB::_READ_CD* cdb,
	CDFLAG::_READ_CD::_EXPECTED_SECTOR_TYPE type,
	DWORD dwTransferLen,
	CDFLAG::_READ_CD::_ERROR_FLAGS c2,
	CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION Sub
);

VOID SetReadD8Command(
	PDEVICE pDevice,
	CDB::_PLXTR_READ_CDDA* cdb,
	DWORD dwTransferLen,
	CDFLAG::_PLXTR_READ_CDDA::_SUB_CHANNEL_SELECTION Sub
);

VOID SetReadDiscCommand(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BYTE byTransferLen,
	CDFLAG::_READ_CD::_ERROR_FLAGS c2,
	CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION tmpsub,
	LPBYTE lpCmd,
	BOOL bOutputLog
);

VOID SetCommandForTransferLength(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	LPBYTE pCdb,
	DWORD dwSize,
	LPBYTE lpTransferLen,
	LPBYTE lpRoopLen
);

VOID SetBufferSizeForReadCD(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	DRIVE_DATA_ORDER order
);

VOID SetFeatureCdRead(
	PFEATURE_DATA_CD_READ pCDRead,
	PDEVICE pDevice
);

VOID SetFeatureRealTimeStreaming(
	PFEATURE_DATA_REAL_TIME_STREAMING pRTS,
	PDEVICE pDevice
);

VOID SetAndOutputToc(
	PDISC pDisc
);

VOID SetAndOutputTocForGD(
	PDISC pDisc,
	LPBYTE bufDec
);

VOID SetAndOutputTocFull(
	PDISC pDisc,
	PCDROM_TOC_FULL_TOC_DATA fullToc,
	PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData,
	WORD wTocEntries,
	FILE* fpCcd
);

VOID SetAndOutputTocCDText(
	PDISC pDisc,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	LPCH pTmpText,
	WORD wTocTextEntries
);

VOID SetAndOutputCDOffset(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	BOOL bGetDriveOffset,
	INT nDriveSampleOffset,
	INT nDriveOffset,
	INT nSubChannelOffset
);

VOID ResetAndOutputCDOffset(
	PDEVICE pDevice,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nSample
);

VOID SetCDOffset(
	PEXEC_TYPE pExecType,
	BYTE byBe,
	BYTE byPlxtrDrive,
	PDISC pDisc,
	INT nStartLBA,
	INT nEndLBA
);

VOID SetTrackAttribution(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA
);

VOID SetAdr6ToString(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPSTR pszOutString,
	BOOL bCopy
);

VOID SetISRCToString(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	LPSTR pszOutString,
	BOOL bCopy
);

VOID SetMCNToString(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPSTR pszOutString,
	BOOL bCopy
);

VOID SetLBAForFirstAdr(
	INT n1stLBA[][2],
	INT nRangeLBA[][2],
	LPCTSTR strAdr,
	LPINT nAdrLBAList,
	BYTE byIdxOfSession,
	BYTE byPlxtrDrive
);

VOID SetBufferFromTmpSubch(
	LPBYTE lpSubcode,
	SUBCH_PER_SECTOR subch,
	BOOL bCurrent,
	BOOL bUpdateCrc
);

VOID SetBufferFromMCN(
	PDISC pDisc,
	LPBYTE lpSubcode
);

VOID SetTmpSubchFromBuffer(
	PSUBCH_PER_SECTOR pSubch,
	LPBYTE lpSubcode
);

VOID UpdateTmpSubchForMCN(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA
);

VOID UpdateTmpSubchForISRC(
	PSUBCH pSubch
);

VOID UpdateTmpSubchForAdr6(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA
);

VOID UpdateTmpSubch(
	PDISC_PER_SECTOR pDiscPerSector
);

VOID UpdateTmpMainHeader(
	PDISC_PER_SECTOR pDiscPerSector,
	INT nType
);
