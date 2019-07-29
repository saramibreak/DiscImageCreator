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

VOID OutputSenseData(
	PSENSE_DATA pSenseData
);

VOID OutputScsiStatus(
	BYTE byScsiStatus
);

VOID OutputScsiAddress(
	PDEVICE pDevice
);

VOID OutputStorageAdaptorDescriptor(
	PSTORAGE_ADAPTER_DESCRIPTOR pAdapterDescriptor,
	LPBOOL lpBusTypeUSB
);

VOID OutputDiskGeometry(
	PDISK_GEOMETRY pGeom,
	DWORD dwGeomNum
);

VOID OutputDiskGeometryEx(
	PDISK_GEOMETRY_EX pGeom
);

VOID OutputRemovableDiskInfo(
	PGET_MEDIA_TYPES pMedia
);

VOID OutputFileAllocationTable(
	LPBYTE lpBuf,
	PFAT fat
);

VOID OutputDVDGetRegion(
	PDVD_REGION dvdRegion
);

