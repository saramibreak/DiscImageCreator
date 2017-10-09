/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

BOOL IsValidMainDataHeader(
	LPBYTE lpBuf
	);

BOOL IsValid3doDataHeader(
	LPBYTE lpBuf
	);

BOOL IsValidMacDataHeader(
	LPBYTE lpBuf
	);

BOOL IsValidPceSector(
	LPBYTE lpBuf
	);

BOOL IsValidPcfxSector(
	LPBYTE lpBuf
	);

BOOL IsValidPlextorDrive(
	PDEVICE pDevice
	);

BOOL IsValidPregapSector(
	PEXEC_TYPE pExecType,
	PDISC pDisc,
	PSUB_Q pSubQ,
	INT nLBA
	);

BOOL IsValidLibCryptSector(
	BOOL bLibCrypt,
	INT nLBA
	);

BOOL IsValidIntentionalSubSector(
	BOOL bIntentionalSub,
	PDISC pDisc,
	INT nLBA
	);

BOOL IsValidSubQMCN(
	LPBYTE lpSubcode
	);

BOOL IsValidSubQISRC(
	LPBYTE lpSubcode
	);

VOID CheckAndFixSubChannel(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	BYTE byCurrentTrackNum,
	INT nLBA,
	BOOL bLibCrypt,
	BOOL bSecuRom
	);

BOOL ContainsC2Error(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpBuf,
	UINT uiC2ErrorLBACnt
	);

BOOL ContainsDiffByte(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	LPBYTE lpBuf,
	UINT i
	);

VOID SupportIndex0InTrack1(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
	);

BOOL IsCheckingSubChannel(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nLBA
	);

VOID CheckAndFixMainHeader(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA,
	BYTE byCurrentTrackNum,
	INT nMainDataType
	);
