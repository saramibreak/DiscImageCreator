/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "enum.h"

VOID SetReadCDCommand(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	CDB::_READ_CD* cdb,
	CDFLAG::_READ_CD::_EXPECTED_SECTOR_TYPE type,
	DWORD dwTransferLen,
	CDFLAG::_READ_CD::_ERROR_FLAGS c2,
	CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION Sub,
	BOOL bCheckReading
);

VOID SetReadD8Command(
	PDEVICE pDevice,
	CDB::_PLXTR_READ_CDDA* cdb,
	DWORD dwTransferLen,
	CDFLAG::_PLXTR_READ_CDDA::_SUB_CHANNEL_SELECTION Sub
);

VOID SetCommandForTransferLength(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	LPBYTE pCdb,
	DWORD dwSize,
	LPBYTE lpTransferLen
);

VOID SetBufferSizeForReadCD(
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
	PEXEC_TYPE pExecType,
	PDISC pDisc
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
	WORD wTocTextEntries,
	WORD wAllTextSize
);

VOID SetAndOutputTocCDWText(
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	LPCH pTmpText,
	WORD wFirstEntries,
	WORD wTocTextEntries,
	WORD wAllTextSize
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
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nLBA,
	LPBYTE lpCurrentTrackNum,
	PMAIN_HEADER pMain,
	PSUB_Q pSubQ
);

VOID SetISRCToString(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPSTR pszOutString,
	BYTE byIdxOfTrack,
	BOOL bCopy
);

VOID SetMCNToString(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPSTR pszOutString,
	BOOL bCopy
);

VOID SetLBAForFirstAdr(
	INT nFirstLBA[][2],
	INT nRangeLBA[][2],
	LPSTR strAdr,
	LPINT nAdrLBAList,
	BYTE byIdxOfSession,
	BYTE byPlxtrDrive
);

VOID SetBufferFromTmpSubQData(
	SUB_Q_PER_SECTOR pSubQ,
	LPBYTE lpSubcode,
	BYTE byPresent
);

VOID SetBufferFromMCN(
	PDISC pDisc,
	LPBYTE lpSubcode
);

VOID SetTmpSubQDataFromBuffer(
	PSUB_Q_PER_SECTOR pSubQ,
	LPBYTE lpSubcode
);

VOID UpdateTmpSubQDataForMCN(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	PSUB_Q pSubQ,
	INT nLBA,
	BYTE byCurrentTrackNum
);

VOID UpdateTmpSubQDataForISRC(
	PSUB_Q pSubQ
);

VOID UpdateTmpSubQDataForCDTV(
	PEXEC_TYPE pExecType,
	PDISC pDisc,
	PSUB_Q pSubQ,
	INT nLBA,
	BYTE byCurrentTrackNum
);

VOID UpdateTmpSubQData(
	PSUB_Q pSubQ,
	BOOL bLibCrypt,
	BOOL bSecuRom
);

VOID UpdateTmpMainHeader(
	PMAIN_HEADER pMain,
	LPBYTE lpBuf,
	BYTE byCtl,
	INT nType
);
