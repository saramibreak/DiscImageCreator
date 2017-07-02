/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

BOOL ReadWriteDat(
	PEXEC_TYPE pExecType,
	_TCHAR* pszFullPath,
	UCHAR uiLastTrack,
	_TCHAR* szDrive,
	_TCHAR* szDir,
	_TCHAR* szFname
);

BOOL OutputHash(
	CComPtr<IXmlWriter> pWriter,
	_TCHAR* pszFullPath,
	LPCTSTR szExt,
	UCHAR uiTrack,
	UCHAR uiLastTrack
);
