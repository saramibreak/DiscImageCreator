/**
 * Copyright 2011-2024 sarami
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

VOID OutputFsVolumeDescriptor(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	PVOLUME_DESCRIPTOR pVolDesc,
	INT nLBA
);

VOID OutputFsVolumeDescriptorForISO9660(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	PVOLUME_DESCRIPTOR pVolDesc
);

VOID OutputFsVolumeDescriptorForJoliet(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	PVOLUME_DESCRIPTOR pVolDesc
);

VOID OutputFsDirectoryRecord(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	UINT uiExtentPos,
	UINT uiDataLen,
	LPSTR fname,
	PPATH_TABLE_RECORD pPathTblRec,
	UINT uiPathTblIdx
);

BOOL OutputFsPathTableRecord(
	LPBYTE lpBuf,
	UINT uiLogicalBlkCoef,
	UINT uiPathTblPos,
	UINT uiPathTblSize,
	BOOL bPathType,
	PPATH_TABLE_RECORD pPathTblRec,
	LPUINT uiDirPosNum
);

VOID OutputFsFileAllocationTable(
	LPBYTE lpBuf,
	PFAT fat
);

VOID OutputFsFATLDirEntry(
	LPBYTE lpBuf,
	LPUINT i,
	_TCHAR* pTab
);

VOID OutputFsFATDirEntry(
	LPBYTE lpBuf,
	UINT i,
	_TCHAR* pTab
);

VOID OutputFsExFAT(
	LPBYTE lpBuf,
	PEXFAT pExFat
);

VOID OutputFsExFATDirectoryEntry0x81(
	LPBYTE lpBuf,
	INT i,
	_TCHAR* pTab
);

VOID OutputFsExFATDirectoryEntry0x82(
	LPBYTE lpBuf,
	INT i,
	_TCHAR* pTab
);

VOID OutputFsExFATDirectoryEntry0x83(
	LPBYTE lpBuf,
	INT i,
	_TCHAR* pTab
);

VOID OutputFsExFATDirectoryEntry0x85(
	LPBYTE lpBuf,
	WORD attr,
	INT i,
	_TCHAR* pTab
);

VOID OutputFsExFATDirectoryEntry0xa0(
	LPBYTE lpBuf,
	INT i,
	_TCHAR* pTab
);

VOID OutputFsExFATDirectoryEntry0xc0(
	LPBYTE lpBuf,
	INT NameLength,
	UINT FirstCluster,
	INT i,
	_TCHAR* pTab
);

VOID OutputFsExFATDirectoryEntry0xc1(
	LPBYTE lpBuf,
	LPBOOL bName1st,
	INT i,
	_TCHAR* pTab
);

VOID OutputFsDriveDescriptorRecord(
	LPBYTE lpBuf
);

VOID OutputFsPartitionMap(
	LPBYTE lpBuf,
	LPBOOL bHfs
);

VOID OutputFsMasterDirectoryBlocks(
	LPBYTE lpBuf,
	INT nLBA
);

VOID OutputFs3doHeader(
	LPBYTE lpBuf,
	INT nLBA
);

VOID OutputFs3doDirectoryRecord(
	LPBYTE lpBuf,
	INT nLBA,
	LPCCH pPath,
	UINT uiDirSize
);

VOID OutputFsPceStuff(
	LPBYTE lpBuf,
	INT nLBA
);

VOID OutputFsPceBootSector(
	LPBYTE lpBuf,
	INT nLBA
);

VOID OutputFsPcfxHeader(
	LPBYTE lpBuf,
	INT nLBA
);

VOID OutputFsPcfxSector(
	LPBYTE lpBuf,
	INT nLBA
);

VOID OutputFsVolumeDescriptorSequence(
	LPBYTE lpBuf,
	INT nLBA,
	PUDF pUdf
);

VOID OutputFsVolumeRecognitionSequence(
	LPBYTE lpBuf,
	INT nLBA,
	LPBOOL pUDF
);
