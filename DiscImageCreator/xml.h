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
#ifndef _WIN32
#include "_external/tinyxml2.h"
using namespace tinyxml2;
#endif

BOOL ReadWriteDat(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	_TCHAR* pszFullPath,
	_TCHAR* szDrive,
	_TCHAR* szDir,
	_TCHAR* szFname,
	BOOL bDesync
);

BOOL OutputHash(
#ifdef _WIN32
	CComPtr<IXmlWriter> pWriter,
#else
	XMLElement* pWriter,
#endif
	_TCHAR* pszFullPath,
	LPCTSTR szExt,
	UCHAR uiTrack,
	UCHAR uiLastTrack,
	BOOL bDesync
);
