/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
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

VOID OutputFloppyInfo(
	PDISK_GEOMETRY pGeom,
	DWORD dwGeomNum
	);
