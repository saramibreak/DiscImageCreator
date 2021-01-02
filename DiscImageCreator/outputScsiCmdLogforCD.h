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

VOID OutputFsImageDosHeader(
	PIMAGE_DOS_HEADER pIdh
);

VOID OutputFsImageOS2Header(
	PIMAGE_OS2_HEADER pIoh
);

VOID OutputFsImageNtHeader(
	PIMAGE_NT_HEADERS32 pInh
);

VOID OutputFsImageSectionHeader(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	PIMAGE_SECTION_HEADER pIsh,
	LPBOOL bSecurom
);

VOID OutputImportDirectory(
	LPBYTE lpBuf,
	DWORD dwBufSize,
	DWORD dwImportVirtualAddress,
	DWORD dwOfs
);

VOID OutputSecuRomDll4_87Header(
	LPBYTE lpBuf,
	UINT i
);

VOID OutputSecuRomDllHeader(
	LPBYTE lpBuf,
	LPUINT uiOfsOf16,
	LPUINT uiOfsOf32,
	LPUINT uiOfsOfNT,
	LPINT idx
);

VOID OutputSint16(
	LPBYTE lpBuf,
	UINT uiOfsOf16,
	UINT uiOfsOfSecuRomDll,
	INT idx
);

VOID OutputSint32(
	LPBYTE lpBuf,
	INT nOfsOf32dll,
	BOOL bDummy
);

VOID OutputSintNT(
	LPBYTE lpBuf,
	INT nOfsOfNTdll,
	BOOL bDummy
);

VOID OutputTocWithPregap(
	PDISC pDisc
);

VOID OutputCDOffset(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	BOOL bGetDriveOffset,
	INT nDriveSampleOffset,
	INT nDriveOffset,
	INT nSubChannelOffset
);

VOID OutputCDC2Error296(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA
);

VOID OutputCDMain(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwSize
);

VOID OutputCDSub96Align(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA
);

VOID OutputCDSub96Raw(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA
);

VOID OutputCDSubToLog(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	LPBYTE lpSubcodeRaw,
	INT nLBA
);
