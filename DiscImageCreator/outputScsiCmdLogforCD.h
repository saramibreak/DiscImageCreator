/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "enum.h"

VOID OutputFsVolumeDescriptor(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputFsVolumeDescriptorForISO9660(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf
	);

VOID OutputFsVolumeDescriptorForJoliet(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf
	);

VOID OutputFsDirectoryRecord(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	DWORD dwExtentPos,
	DWORD dwDataLen,
	LPSTR fname
	);

BOOL OutputFsPathTableRecord(
	PDISC pDisc,
	LPBYTE lpBuf,
	BYTE byLogicalBlkCoef,
	DWORD dwPathTblPos,
	DWORD dwPathTblSize,
	PDIRECTORY_RECORD pDirRec,
	LPINT nDirPosNum
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
	LPCH pPath,
	LONG lDirSize
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
	PDISC pDisc,
	PIMAGE_SECTION_HEADER pIsh
	);

VOID OutputTocForGD(
	PDISC pDisc
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
	INT nSize
	);

VOID OutputCDSub96Align(
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
	LPBYTE lpSubcode,
	LPBYTE lpSubcodeRaw,
	INT nLBA,
	INT byTrackNum,
	FILE* fpParse
	);
