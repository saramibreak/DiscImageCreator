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

VOID OutputDVDHeader(
	LPBYTE lpBuf,
	DWORD dwSectorSize,
	INT nLBA,
	BOOL bNintendoDisc
);

VOID OutputDVDStructureFormat(
	PDISC pDisc,
	BYTE byFormatCode,
	WORD wFormatLength,
	LPBYTE lpFormat,
	LPDWORD lpdwSectorLength,
	UCHAR layerNumber
);

VOID OutputDVDLayerDescriptor(
	PDISC pDisc,
	PDVD_FULL_LAYER_DESCRIPTOR dvdLayer,
	LPDWORD lpdwSectorLength,
	UCHAR layerNumber,
	LOG_TYPE type
);

VOID OutputDVDCopyrightDescriptor(
	PDVD_COPYRIGHT_DESCRIPTOR dvdCopyright,
	PPROTECT_TYPE_DVD pProtect,
	LOG_TYPE type
);

VOID OutputDiscBCADescriptor(
	PDISC pDisc,
	PDVD_BCA_DESCRIPTOR dvdBca,
	WORD wFormatLength,
	LOG_TYPE type
);

VOID OutputDVDManufacturerDescriptor(
	PDVD_MANUFACTURER_DESCRIPTOR dvdManufacturer,
	PDISC pDisc,
	LOG_TYPE type
);

VOID OutputDVDCopyrightManagementInformation(
	PDVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR dvdCopyright,
	INT nLBA
);

VOID OutputBDStructureFormat(
	PDISC pDisc,
	BYTE byFormatCode,
	WORD wFormatLength,
	LPBYTE lpFormat,
	INT nPacCnt
);

VOID OutputXboxManufacturingInfo(
	LPBYTE buf
);

VOID OutputXboxSecuritySector(
	PDISC pDisc,
	LPBYTE buf
);

VOID OutputXbox360SecuritySector(
	PDISC pDisc,
	LPBYTE buf
);
