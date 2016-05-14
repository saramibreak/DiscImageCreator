/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

#define DVD_RAW_READ	(2064)

BOOL ReadDVD(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszPath
	);

BOOL ReadDVDForCMI(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL ReadDVDRaw(
	PDEVICE pDevice,
	LPCSTR szVendorId,
	LPCTSTR pszPath
	);

BOOL ReadDVDStructure(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	);
