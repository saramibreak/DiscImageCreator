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

VOID OutputInquiry(
	PINQUIRYDATA pInquiry
);

VOID OutputGetConfigurationHeader(
	PGET_CONFIGURATION_HEADER pConfigHeader
);

VOID OutputGetConfigurationFeatureProfileType(
	WORD wFeatureProfileType
);

VOID OutputGetConfigurationFeatureNumber(
	PDEVICE pDevice,
	LPBYTE lpConf,
	DWORD dwAllLen
);

VOID OutputCDAtip(
	PCDROM_TOC_ATIP_DATA_BLOCK pAtip
);

VOID OutputCDTextOther(
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	WORD wTocTextEntries,
	BYTE bySizeInfoIdx,
	UINT uiSizeInfoCnt
);

VOID OutputDiscInformation(
	PDISC_INFORMATION pDiscInformation
);

VOID OutputModeParmeterHeader(
	PMODE_PARAMETER_HEADER pHeader
);

VOID OutputModeParmeterHeader10(
	PMODE_PARAMETER_HEADER10 pHeader
);

VOID OutputModeSense(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE modesense
);

VOID OutputReadBufferCapacity(
	PREAD_BUFFER_CAPACITY_DATA pReadBufCapaData
);

VOID OutputSetSpeed(
	PCDROM_SET_SPEED pSetspeed
);

VOID OutputEeprom(
	LPBYTE pBuf,
	INT nRoop,
	BOOL byPlxtrDrive
);
