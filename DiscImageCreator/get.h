/**
 * Copyright 2011-2025 sarami
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

BOOL GetAlignedCallocatedBuffer(
	PDEVICE pDevice,
	LPBYTE* ppSrcBuf,
	UINT uiSize,
	LPBYTE* ppOutBuf,
	LPCTSTR pszFuncName,
	LONG lLineNum
);

BOOL GetHandle(
	PDEVICE pDevice
);

BOOL GetDriveOffsetManually(
	LPINT lpDriveOffset
);

BOOL GetDriveOffsetAuto(
	PDEVICE pDevice,
	LPINT lpDriveOffset
);

BOOL GetReadBufParamAndSize(
	PDEVICE pDevice,
	LPBYTE lpCommand,
	LPDWORD lpdwBufSize,
	LPBOOL lpScrambled
);

BOOL GetReadErrorFileName(
	PEXT_ARG pExtArg,
	CHAR protectFname[MAX_FNAME_FOR_VOLUME]
);

INT GetReadErrorFileIdx(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nLBA
);

BOOL GetFilenameToSkipError(
	CHAR szFilename[][MAX_FNAME_FOR_VOLUME]
);

BOOL GetC2ErrorFileName(
	PEXT_ARG pExtArg,
	CHAR protectFname[MAX_FNAME_FOR_VOLUME]
);

INT GetC2ErrorFileIdx(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nLBA
);

BOOL GetFilenameToFixError(
	CHAR szFilename[][MAX_FNAME_FOR_VOLUME]
);

BOOL GetTimeStamp(
	_TCHAR* pszTime,
	size_t stBufsize,
	DWORD dwTime
);

DWORD GetFileSize(
	LONG lOffset,
	FILE *fp
);

UINT64 GetFileSize64(
	INT64 n64Offset,
	FILE *fp
);

BOOL GetDiscSize(
	LPTSTR path,
	PUINT64 lpSize
);

WORD GetSizeOrWordForVolDesc(
	LPBYTE lpBuf
);

UINT GetSizeOrUintForVolDesc(
	LPBYTE lpBuf,
	UINT uiMax
);

BYTE GetMode(
	PDISC_PER_SECTOR pDiscPerSector,
	INT nType
);

BOOL GetWriteOffset(
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nSectorNum,
	INT nLBA
);

BOOL GetCmd(
	LPTSTR szPath,
	LPCTSTR szFname,
	LPCTSTR szExt
);

BOOL GetEccEdcCmd(
	LPTSTR pszStr,
	size_t cmdSize,
	LPCTSTR pszCmd,
	LPCTSTR pszType,
	LPCTSTR pszImgPath,
	INT nStartLBA,
	INT nEndLBA
);

BOOL GetUnscCmd(
	LPTSTR pszStr,
	LPCTSTR pszPath,
	INT nType
);

BOOL GetDVDProtectionCmd(
	PDEVICE pDevice,
	LPTSTR pszStr,
	_PROTECT_TYPE_DVD protect,
	LPCTSTR pszPath
);

UINT GetLBAfromClusterNumber(
	PEXFAT pExFat,
	UINT uiClusterNum
);
