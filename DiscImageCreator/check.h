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

BOOL IsXbox(
	PEXEC_TYPE pExecType
);

BOOL IsCDBasedDisc(
	PEXEC_TYPE pExecType,
	PDISC pDisc
);

BOOL IsDVDBasedDisc(
	PDISC pDisc
);

BOOL IsBDBasedDisc(
	PDISC pDisc
);

BOOL IsCDR(
	PDISC pDisc
);

BOOL IsValidPS3Drive(
	PDEVICE pDevice
);

BOOL IsValid0xF1SupportedDrive(
	PDEVICE pDevice
);

BOOL IsValidPlextorDrive(
	PDEVICE pDevice
);

BOOL IsPregapOfTrack1ReadableDrive(
	PDEVICE pDevice
);

BOOL IsPrextorDVDDrive(
	PDEVICE pDevice
);

VOID SupportIndex0InTrack1(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
);

BOOL IsEnoughDiskSpaceForDump(
	PEXEC_TYPE pExecType,
	_TCHAR* pszPath
);

BOOL IsValidMainDataHeader(
	LPBYTE lpBuf
);

BOOL IsValid3doDataHeader(
	LPBYTE lpBuf
);

BOOL IsFat(
	LPBYTE lpBuf
);

BOOL IsDriverDescriptorRecord(
	LPBYTE lpBuf
);

BOOL IsApplePartionMap(
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

BOOL IsValidPregapSector(
	PDISC pDisc,
	PSUBCH pSubch,
	INT nLBA
);

BOOL IsValidLibCryptSector(
	BOOL bLibCrypt,
	INT nLBA
);

BOOL IsValidSecuRomSector(
	BOOL bSecuRom,
	PDISC pDisc,
	INT nLBA
);

BOOL IsValidProtectedSector(
	PDISC pDisc,
	INT nLBA,
	INT idx
);

BOOL IsValidSafeDiscSector(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector
);

BOOL IsValidIntentionalC2error(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA,
	INT idx
);

BOOL IsCheckingSubChannel(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nLBA
);

BOOL IsValidSubQAdrMCN(
	LPBYTE lpSubcode
);

BOOL IsValidSubQAdrISRC(
	LPBYTE lpSubcode
);

BOOL IsValidSubQAdrSector(
	UINT uiSubAdditionalNum,
	PSUBCH pSubch,
	INT nRangeLBA,
	INT n1stLBA,
	INT nPrevAdrSector,
	INT nLBA
);

BOOL IsValidSubQCtl(
	PSUBCH pSubch,
	BYTE byEndCtl
);

BOOL IsValidSubQIdx(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA,
	LPBOOL bPrevIndex,
	LPBOOL bPrevPrevIndex
);

BOOL IsValidSubQTrack(
	PEXEC_TYPE pExecType,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA,
	LPBOOL bPrevTrackNum
);

BOOL IsValidSubQMSF(
	PEXEC_TYPE pExecType,
	LPBYTE lpSubcode,
	BYTE m,
	BYTE s,
	BYTE f
);

BOOL IsValidSubQRMSF(
	PEXEC_TYPE pExecType,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA
);

BOOL IsValidSubQAFrame(
	LPBYTE lpSubcode,
	INT nLBA
);

BOOL IsValidSubQAMSF(
	PEXEC_TYPE pExecType,
	BOOL bRipPregap,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA
);

BOOL ContainsC2Error(
	PDEVICE pDevice,
	LPBYTE lpBuf,
	LPUINT lpuiC2errorNum,
	INT nLBA,
	BOOL bOutputLog
);

BOOL AnalyzeIfoFile(
	PDEVICE pDevice,
	PDISC pDisc
	);
