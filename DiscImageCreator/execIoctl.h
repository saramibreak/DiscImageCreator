/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

BOOL DiskGetMediaTypes(
	PDEVICE pDevice,
	LPCTSTR pszPath
);

BOOL ScsiGetAddress(
	PDEVICE pDevice
);

BOOL ScsiPassThroughDirect(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPVOID lpCdbCmd,
	BYTE byCdbCmdLength,
	LPVOID pvBuffer,
	DWORD dwBufferLength,
	LPBYTE byScsiStatus,
	LPCTSTR pszFuncName,
	LONG lLineNum
);

BOOL StorageQueryProperty(
	PDEVICE pDevice,
	LPBOOL lpBusTypeUSB
);

BOOL SetStreaming(
	PDEVICE pDevice,
	DWORD dwDiscSpeedNum
);

BOOL DvdStartSession(
	PDEVICE pDevice,
	PDVD_COPY_PROTECT_KEY dvdKey
);

BOOL ReadKey(
	PDEVICE pDevice,
	PDVD_COPY_PROTECT_KEY dvdKey
);

BOOL SendKey(
	PDEVICE pDevice,
	PDVD_COPY_PROTECT_KEY dvdKey
);
