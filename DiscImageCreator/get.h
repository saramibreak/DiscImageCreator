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

BOOL GetAlignedCallocatedBuffer(
	PDEVICE pDevice,
	LPBYTE* ppSrcBuf,
	DWORD dwSize,
	LPBYTE* ppOutBuf,
	LPCTSTR pszFuncName,
	LONG lLineNum
);

BOOL GetHandle(
	PDEVICE pDevice,
	_TCHAR* szBuf,
	size_t bufSize
);

VOID GetDriveOffsetManually(
	LPINT lpDriveOffset
);

BOOL GetDriveOffsetAuto(
	LPCSTR szProductId,
	LPINT lpDriveOffset
);

DWORD GetFileSize(
	LONG lOffset,
	FILE *fp
);

UINT64 GetFileSize64(
	INT64 n64Offset,
	FILE *fp
);

BYTE GetMode(
	PDISC_PER_SECTOR pDiscPerSector,
	INT nType
);

BOOL GetWriteOffset(
	PDISC pDisc,
	LPBYTE lpBuf
);

BOOL GetEccEdcCmd(
	LPTSTR pszStr,
	size_t cmdSize,
	LPCTSTR pszCmd,
	LPCTSTR pszImgPath,
	INT nStartLBA,
	INT nEndLBA
);

BOOL GetUnscCmd(
	LPTSTR pszStr,
	LPCTSTR pszPath
);

WORD GetSizeOrWordForVolDesc(
	LPBYTE lpBuf
);

DWORD GetSizeOrDwordForVolDesc(
	LPBYTE lpBuf,
	DWORD dwMax
);
