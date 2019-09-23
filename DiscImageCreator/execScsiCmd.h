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

#define CD_DRIVE_MAX_SPEED	(72)

BOOL TestUnitReady(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
);
BOOL Inquiry(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice
);

BOOL StartStopUnit(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BYTE Start,
	BYTE LoadEject
);

BOOL SynchronizeCache(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
);

BOOL ReadTOC(
	PEXT_ARG pExtArg,
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC pDisc
);

BOOL ReadTOCFull(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PCDROM_TOC_FULL_TOC_DATA pFullTocData,
	PCDROM_TOC_FULL_TOC_DATA_BLOCK* pTocData,
	LPWORD wTocEntries,
	LPBYTE* pPFullToc
);

BOOL ReadTOCAtip(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
);

BOOL ReadTOCText(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	FILE* fpCcd
);

BOOL ReadDiscInformation(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
);

BOOL ModeSense10(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
);

BOOL SendKey(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BYTE agid,
	BYTE keyFormat,
	LPBYTE key,
	WORD keyLength
);

BOOL ReportKey(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BYTE agid,
	BYTE keyFormat,
	LPBYTE key,
	WORD keyLength
);

BOOL SetDiscSpeed(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	UINT uiDiscSpeedNum
);

BOOL SetSpeedRead(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BOOL bState
);

BOOL Reset(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
);

BOOL ReadCacheForLgAsus(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	INT nLBA
);

BOOL ReadDriveInformation(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	UINT uiCDSpeed
);

BOOL ReadGDForTOC(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
);
