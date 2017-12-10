/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

BOOL ReadWriteDat(
	PEXEC_TYPE pExecType,
	PDISC pDisc,
	_TCHAR* pszFullPath,
	_TCHAR* szDrive,
	_TCHAR* szDir,
	_TCHAR* szFname,
	BOOL bDesync
);

BOOL OutputHash(
	CComPtr<IXmlWriter> pWriter,
	_TCHAR* pszFullPath,
	LPCTSTR szExt,
	UCHAR uiTrack,
	UCHAR uiLastTrack,
	BOOL bDesync
);
