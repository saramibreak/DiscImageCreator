/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

#define CD_DRIVE_MAX_SPEED	(72)
#define DVD_DRIVE_MAX_SPEED	(16)

BOOL TestUnitReady(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
);

BOOL StartStopUnit(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BYTE Start,
	BYTE LoadEject
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
	PDISC_PER_SECTOR pDiscPerSector,
	FILE* fpCcd
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

BOOL SetDiscSpeed(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	DWORD dwDiscSpeedNum
);

BOOL Reset(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
);

BOOL ReadDriveInformation(
	PEXEC_TYPE pExexType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	DWORD dwCDSpeed
);

BOOL ExecReadCD(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPBYTE lpCmd,
	INT nLBA,
	LPBYTE lpBuf,
	DWORD dwBufSize,
	LPCTSTR pszFuncName,
	LONG lLineNum
);

BOOL ReadVolumeDescriptor(
	PEXEC_TYPE pExexType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	BYTE byIdx,
	LPBYTE pCdb,
	LPBYTE lpBuf,
	INT nPVD,
	LPBOOL lpReadVD,
	PVOLUME_DESCRIPTOR pVolDesc
);

BOOL ReadPathTableRecord(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE pCdb,
	DWORD dwLogicalBlkCoef,
	DWORD dwPathTblSize,
	DWORD dwPathTblPos,
	PDIRECTORY_RECORD pDirRec,
	LPINT nDirPosNum
);

BOOL ReadDirectoryRecord(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE pCdb,
	LPBYTE lpBuf,
	DWORD dwLogicalBlkCoef,
	DWORD dwRootDataLen,
	PDIRECTORY_RECORD pDirRec,
	INT nDirPosNum
);

BOOL ExecReadGD(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE pCdb,
	INT nLBA,
	BYTE byTransferLen,
	LPBYTE lpInBuf,
	LPBYTE lpOutBuf
);
