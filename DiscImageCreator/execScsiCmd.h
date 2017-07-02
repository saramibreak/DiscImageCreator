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
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	BYTE byIdx,
	CDB::_READ12* pCdb,
	LPBYTE lpBuf,
	LPBOOL pPVD,
	LPDWORD dwPathTblSize,
	LPDWORD dwPathTblPos,
	LPDWORD dwRootDataLen
);

BOOL ReadPathTableRecord(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb,
	DWORD dwPathTblSize,
	DWORD dwPathTblPos,
	PDIRECTORY_RECORD pDirRec,
	LPINT nDirPosNum
);

BOOL ReadDirectoryRecord(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb,
	LPBYTE lpBuf,
	DWORD dwRootDataLen,
	PDIRECTORY_RECORD pDirRec,
	INT nDirPosNum
);
