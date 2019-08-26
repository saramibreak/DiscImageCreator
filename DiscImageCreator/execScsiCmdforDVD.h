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

#define DVD_RAW_READ	(2064)
#define DVD_RAW_READ2	(2384)

BOOL ReadDVDReverse(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPCTSTR pszFullPath,
	INT nStartLBA,
	INT nLastLBA
);

BOOL ReadDVD(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath
);

BOOL ReadDVDForCMI(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
);

BOOL ReadDVDRaw(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath
);

BOOL ReadDiscStructure(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath
);

BOOL SetLockState(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BYTE byState
);

BOOL ReadXboxDVD(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath
);

BOOL ReadXboxDVDBySwap(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath
);

BOOL ReadSACD(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszFullPath
);
