/**
 * Copyright 2011-2023 sarami
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

BOOL InitC2(
	PDISC* pDisc
);

BOOL InitLBAPerTrack(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
);

BOOL InitTocFullData(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
);

BOOL InitTocTextData(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC* pDisc
);

VOID InitMainDataHeader(
	PEXEC_TYPE pExecType,
	PMAIN_HEADER pMain,
	INT nLBA
);

BOOL InitProtectData(
	PDISC* pDisc
);

BOOL InitSubData(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
);

#ifndef _DEBUG
BOOL InitLogFile(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	_TCHAR* szFullPath
);
#endif
VOID TerminateC2(
	PDISC* pDisc
);

VOID TerminateLBAPerTrack(
	PDISC* pDisc
);

VOID TerminateTocFullData(
	PDISC* pDisc
);

VOID TerminateTocTextData(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC* pDisc
);

VOID TerminateProtectData(
	PDISC* pDisc
);

VOID TerminateSubData(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
);

#ifndef _DEBUG
VOID TerminateLogFile(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg
);
#endif
