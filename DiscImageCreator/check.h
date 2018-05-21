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
	PDISC pDisc,
	PSUB_Q pSubQ,
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
	INT nLBA
);

BOOL IsValidIntentionalC2error(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector
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
	PDEVICE pDevice,
	LPBYTE lpBuf,
	LPDWORD lpdwC2errorNum
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
