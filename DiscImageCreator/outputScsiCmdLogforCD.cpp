/**
 * Copyright 2011-2021 sarami
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
#include "struct.h"
#include "convert.h"
#include "get.h"
#include "output.h"
#include "_external/NonStandardFunction.h"

VOID OutputFsImageDosHeader(
	PIMAGE_DOS_HEADER pIdh
) {
	OutputVolDescLog(
		"\t========== Image Dos Header (%zu byte) ==========\n"
		"\t                     Magic number: %04x\n"
		"\t       Bytes on last page of file: %04x\n"
		"\t                    Pages in file: %04x\n"
		"\t                      Relocations: %04x\n"
		"\t     Size of header in paragraphs: %04x\n"
		"\t  Minimum extra paragraphs needed: %04x\n"
		"\t  Maximum extra paragraphs needed: %04x\n"
		"\t      Initial (relative) SS value: %04x\n"
		"\t                 Initial SP value: %04x\n"
		"\t                         Checksum: %04x\n"
		"\t                 Initial IP value: %04x\n"
		"\t      Initial (relative) CS value: %04x\n"
		"\t File address of relocation table: %04x\n"
		"\t                   Overlay number: %04x\n"
		"\t   OEM identifier (for e_oeminfo): %04x\n"
		"\tOEM information; e_oemid specific: %04x\n"
		"\t   File address of new exe header: %08ld\n"
		, sizeof(IMAGE_DOS_HEADER)
		, pIdh->e_magic, pIdh->e_cblp, pIdh->e_cp, pIdh->e_crlc, pIdh->e_cparhdr
		, pIdh->e_minalloc, pIdh->e_maxalloc, pIdh->e_ss, pIdh->e_sp, pIdh->e_csum
		, pIdh->e_ip, pIdh->e_cs, pIdh->e_lfarlc, pIdh->e_ovno, pIdh->e_oemid
		, pIdh->e_oeminfo, pIdh->e_lfanew
		);
}

VOID OutputFsImageOS2Header(
	PIMAGE_OS2_HEADER pIoh
) {
	OutputVolDescLog(
		"\t========== Image OS/2 .EXE header (%zu byte) ==========\n"
		"\t                      Magic number: %04x\n"
		"\t                    Version number: %02d\n"
		"\t                   Revision number: %02d\n"
		"\t             Offset of Entry Table: %04x\n"
		"\t    Number of bytes in Entry Table: %04x\n"
		"\t            Checksum of whole file: %08ld\n"
		"\t                         Flag word: %04x\n"
		"\t     Automatic data segment number: %04x\n"
		"\t           Initial heap allocation: %04x\n"
		"\t          Initial stack allocation: %04x\n"
		"\t             Initial CS:IP setting: %08ld\n"
		"\t             Initial SS:SP setting: %08ld\n"
		"\t            Count of file segments: %04x\n"
		"\t Entries in Module Reference Table: %04x\n"
		"\t   Size of non-resident name table: %04x\n"
		"\t           Offset of Segment Table: %04x\n"
		"\t          Offset of Resource Table: %04x\n"
		"\t     Offset of resident name table: %04x\n"
		"\t  Offset of Module Reference Table: %04x\n"
		"\t    Offset of Imported Names Table: %04x\n"
		"\tOffset of Non-resident Names Table: %08ld\n"
		"\t          Count of movable entries: %04x\n"
		"\t     Segment alignment shift count: %04x\n"
		"\t        Count of resource segments: %04x\n"
		"\t           Target Operating system: %02x\n"
		"\t                  Other .EXE flags: %02x\n"
		"\t           offset to return thunks: %04x\n"
		"\t      offset to segment ref. bytes: %04x\n"
		"\t       Minimum code swap area size: %04x\n"
		"\t   Expected Windows version number: %04x\n"
		, sizeof(IMAGE_OS2_HEADER)
		, pIoh->ne_magic, pIoh->ne_ver, pIoh->ne_rev, pIoh->ne_enttab, pIoh->ne_cbenttab
		, pIoh->ne_crc, pIoh->ne_flags, pIoh->ne_autodata, pIoh->ne_heap, pIoh->ne_stack
		, pIoh->ne_csip, pIoh->ne_sssp, pIoh->ne_cseg, pIoh->ne_cmod, pIoh->ne_cbnrestab
		, pIoh->ne_segtab, pIoh->ne_rsrctab, pIoh->ne_restab, pIoh->ne_modtab
		, pIoh->ne_imptab, pIoh->ne_nrestab, pIoh->ne_cmovent, pIoh->ne_align
		, pIoh->ne_cres, pIoh->ne_exetyp, pIoh->ne_flagsothers, pIoh->ne_pretthunks
		, pIoh->ne_psegrefbytes, pIoh->ne_swaparea, pIoh->ne_expver
		);
}

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms680336(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms680313(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms680339(v=vs.85).aspx
VOID OutputFsImageNtHeader(
	PIMAGE_NT_HEADERS32 pInh
) {
	time_t timeDateStamp = pInh->FileHeader.TimeDateStamp;
	tm* ctime = gmtime(&timeDateStamp);
	_TCHAR szTime[128] = {};
	_tcsftime(szTime, sizeof(szTime) / sizeof(szTime[0]), _T("%FT%T"), ctime);

	OutputVolDescLog(
		"\t========== Image NT Header (%zu byte) ==========\n"
		"\tSignature: %08lx\n"
		"\t========== File Header ==========\n"
		"\t\t             Machine: %04x "
		, sizeof(IMAGE_NT_HEADERS32)
		, pInh->Signature, pInh->FileHeader.Machine
	);
	switch (pInh->FileHeader.Machine) {
	case 0x014c:
		OutputVolDescLog("(x86)\n");
		break;
	case 0x0162:
		OutputVolDescLog("(R3000)\n");
		break;
	case 0x0166:
		OutputVolDescLog("(R4000)\n");
		break;
	case 0x0168:
		OutputVolDescLog("(R10000)\n");
		break;
	case 0x0169:
		OutputVolDescLog("(WCE v2)\n");
		break;
	case 0x0184:
		OutputVolDescLog("(Alpha AXP)\n");
		break;
	case 0x01a2:
		OutputVolDescLog("(SH3)\n");
		break;
	case 0x01a3:
		OutputVolDescLog("(SH3 DSP)\n");
		break;
	case 0x01a4:
		OutputVolDescLog("(SH3E)\n");
		break;
	case 0x01a6:
		OutputVolDescLog("(SH4)\n");
		break;
	case 0x01a8:
		OutputVolDescLog("(SH5)\n");
		break;
	case 0x01c0:
		OutputVolDescLog("(ARM)\n");
		break;
	case 0x01c2:
		OutputVolDescLog("(THUMB)\n");
		break;
	case 0x01d3:
		OutputVolDescLog("(AM33)\n");
		break;
	case 0x01F0:
		OutputVolDescLog("(PowerPC)\n");
		break;
	case 0x01f1:
		OutputVolDescLog("(PowerPC FP)\n");
		break;
	case 0x0200:
		OutputVolDescLog("(IA64)\n");
		break;
	case 0x0266:
		OutputVolDescLog("(MIPS16)\n");
		break;
	case 0x0284:
		OutputVolDescLog("(ALPHA64)\n");
		break;
	case 0x0366:
		OutputVolDescLog("(MIPS FPU)\n");
		break;
	case 0x0466:
		OutputVolDescLog("(MIPS FPU 16)\n");
		break;
	case 0x0520:
		OutputVolDescLog("(TriCore)\n");
		break;
	case 0x0CEF:
		OutputVolDescLog("(CEF)\n");
		break;
	case 0x0EBC:
		OutputVolDescLog("(EBC)\n");
		break;
	case 0x8664:
		OutputVolDescLog("(AMD64)\n");
		break;
	case 0x9041:
		OutputVolDescLog("(M32R)\n");
		break;
	case 0xC0EE:
		OutputVolDescLog("(CEE)\n");
		break;
	default:
		OutputVolDescLog("(UNKNOWN)\n");
		break;
	}
	OutputVolDescLog(
		"\t\t    NumberOfSections: %04x\n"
		"\t\t       TimeDateStamp: %08lx (%s)\n"
		"\t\tPointerToSymbolTable: %08lx\n"
		"\t\t     NumberOfSymbols: %08lx\n"
		"\t\tSizeOfOptionalHeader: %04x\n"
		"\t\t     Characteristics: %04x\n"
		, pInh->FileHeader.NumberOfSections
		, pInh->FileHeader.TimeDateStamp, szTime
		, pInh->FileHeader.PointerToSymbolTable
		, pInh->FileHeader.NumberOfSymbols, pInh->FileHeader.SizeOfOptionalHeader
		, pInh->FileHeader.Characteristics
	);
	if ((pInh->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED) == IMAGE_FILE_RELOCS_STRIPPED) {
		OutputVolDescLog("\t\t                    => Relocation info stripped from file\n");
	}
	if ((pInh->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) == IMAGE_FILE_EXECUTABLE_IMAGE) {
		OutputVolDescLog("\t\t                    => File is executable\n");
	}
	if ((pInh->FileHeader.Characteristics & IMAGE_FILE_LINE_NUMS_STRIPPED) == IMAGE_FILE_LINE_NUMS_STRIPPED) {
		OutputVolDescLog("\t\t                    => Line nunbers stripped from file\n");
	}
	if ((pInh->FileHeader.Characteristics & IMAGE_FILE_LOCAL_SYMS_STRIPPED) == IMAGE_FILE_LOCAL_SYMS_STRIPPED) {
		OutputVolDescLog("\t\t                    => Local symbols stripped from file\n");
	}
	if ((pInh->FileHeader.Characteristics & IMAGE_FILE_AGGRESIVE_WS_TRIM) == IMAGE_FILE_AGGRESIVE_WS_TRIM) {
		OutputVolDescLog("\t\t                    => Agressively trim working set\n");
	}
	if ((pInh->FileHeader.Characteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE) == IMAGE_FILE_LARGE_ADDRESS_AWARE) {
		OutputVolDescLog("\t\t                    => App can handle >2gb addresses\n");
	}
	if ((pInh->FileHeader.Characteristics & IMAGE_FILE_BYTES_REVERSED_LO) == IMAGE_FILE_BYTES_REVERSED_LO) {
		OutputVolDescLog("\t\t                    => Bytes of machine word are reversed\n");
	}
	if ((pInh->FileHeader.Characteristics & IMAGE_FILE_32BIT_MACHINE) == IMAGE_FILE_32BIT_MACHINE) {
		OutputVolDescLog("\t\t                    => 32 bit word machine\n");
	}
	if ((pInh->FileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED) == IMAGE_FILE_DEBUG_STRIPPED) {
		OutputVolDescLog("\t\t                    => Debugging info stripped from file in .DBG file\n");
	}
	if ((pInh->FileHeader.Characteristics & IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP) == IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP) {
		OutputVolDescLog("\t\t                    => If Image is on removable media, copy and run from the swap file\n");
	}
	if ((pInh->FileHeader.Characteristics & IMAGE_FILE_NET_RUN_FROM_SWAP) == IMAGE_FILE_NET_RUN_FROM_SWAP) {
		OutputVolDescLog("\t\t                    => If Image is on Net, copy and run from the swap file\n");
	}
	if ((pInh->FileHeader.Characteristics & IMAGE_FILE_SYSTEM) == IMAGE_FILE_SYSTEM) {
		OutputVolDescLog("\t\t                    => System File\n");
	}
	if ((pInh->FileHeader.Characteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL) {
		OutputVolDescLog("\t\t                    => File is a DLL\n");
	}
	if ((pInh->FileHeader.Characteristics & IMAGE_FILE_UP_SYSTEM_ONLY) == IMAGE_FILE_UP_SYSTEM_ONLY) {
		OutputVolDescLog("\t\t                    => File should only be run on a UP machine\n");
	}
	if ((pInh->FileHeader.Characteristics & IMAGE_FILE_BYTES_REVERSED_HI) == IMAGE_FILE_BYTES_REVERSED_HI) {
		OutputVolDescLog("\t\t                    => Bytes of machine word are reversed\n");
	}

	OutputVolDescLog(
		"\t========== Optional Header ==========\n"
		"\t\t                      Magic: %04x\n"
		"\t\t         MajorLinkerVersion: %02x\n"
		"\t\t         MinorLinkerVersion: %02x\n"
		"\t\t                 SizeOfCode: %08lx\n"
		"\t\t      SizeOfInitializedData: %08lx\n"
		"\t\t    SizeOfUninitializedData: %08lx\n"
		"\t\t        AddressOfEntryPoint: %08lx\n"
		"\t\t                 BaseOfCode: %08lx\n"
		"\t\t                 BaseOfData: %08lx\n"
		"\t\t                  ImageBase: %08lx\n"
		"\t\t           SectionAlignment: %08lx\n"
		"\t\t              FileAlignment: %08lx\n"
		"\t\tMajorOperatingSystemVersion: %04x\n"
		"\t\tMinorOperatingSystemVersion: %04x\n"
		"\t\t          MajorImageVersion: %04x\n"
		"\t\t          MinorImageVersion: %04x\n"
		"\t\t      MajorSubsystemVersion: %04x\n"
		"\t\t      MinorSubsystemVersion: %04x\n"
		"\t\t          Win32VersionValue: %08lx\n"
		"\t\t                SizeOfImage: %08lx\n"
		"\t\t              SizeOfHeaders: %08lx\n"
		"\t\t                   CheckSum: %08lx\n"
		"\t\t                  Subsystem: %04x "
		, pInh->OptionalHeader.Magic, pInh->OptionalHeader.MajorLinkerVersion
		, pInh->OptionalHeader.MinorLinkerVersion, pInh->OptionalHeader.SizeOfCode
		, pInh->OptionalHeader.SizeOfInitializedData, pInh->OptionalHeader.SizeOfUninitializedData
		, pInh->OptionalHeader.AddressOfEntryPoint, pInh->OptionalHeader.BaseOfCode
		, pInh->OptionalHeader.BaseOfData, pInh->OptionalHeader.ImageBase
		, pInh->OptionalHeader.SectionAlignment, pInh->OptionalHeader.FileAlignment
		, pInh->OptionalHeader.MajorOperatingSystemVersion, pInh->OptionalHeader.MinorOperatingSystemVersion
		, pInh->OptionalHeader.MajorImageVersion, pInh->OptionalHeader.MinorImageVersion
		, pInh->OptionalHeader.MajorSubsystemVersion, pInh->OptionalHeader.MinorSubsystemVersion
		, pInh->OptionalHeader.Win32VersionValue, pInh->OptionalHeader.SizeOfImage
		, pInh->OptionalHeader.SizeOfHeaders, pInh->OptionalHeader.CheckSum
		, pInh->OptionalHeader.Subsystem
	);

	switch (pInh->OptionalHeader.Subsystem) {
	case IMAGE_SUBSYSTEM_NATIVE:
		OutputVolDescLog("(NATIVE)\n");
		break;
	case IMAGE_SUBSYSTEM_WINDOWS_GUI:
		OutputVolDescLog("(WINDOWS_GUI)\n");
		break;
	case IMAGE_SUBSYSTEM_WINDOWS_CUI:
		OutputVolDescLog("(WINDOWS_CUI)\n");
		break;
	case IMAGE_SUBSYSTEM_OS2_CUI:
		OutputVolDescLog("(OS2_CUI)\n");
		break;
	case IMAGE_SUBSYSTEM_POSIX_CUI:
		OutputVolDescLog("(POSIX_CUI)\n");
		break;
	case IMAGE_SUBSYSTEM_NATIVE_WINDOWS:
		OutputVolDescLog("(NATIVE_WINDOWS)\n");
		break;
	case IMAGE_SUBSYSTEM_WINDOWS_CE_GUI:
		OutputVolDescLog("(WINDOWS_CE_GUI)\n");
		break;
	case IMAGE_SUBSYSTEM_EFI_APPLICATION:
		OutputVolDescLog("(EFI_APPLICATION)\n");
		break;
	case IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
		OutputVolDescLog("(EFI_BOOT_SERVICE_DRIVER)\n");
		break;
	case IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
		OutputVolDescLog("(EFI_RUNTIME_DRIVER)\n");
		break;
	case IMAGE_SUBSYSTEM_EFI_ROM:
		OutputVolDescLog("(EFI_ROM)\n");
		break;
	case IMAGE_SUBSYSTEM_XBOX:
		OutputVolDescLog("(XBOX)\n");
		break;
	case IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION:
		OutputVolDescLog("(WINDOWS_BOOT_APPLICATION)\n");
		break;
	default:
		OutputVolDescLog("(UNKNOWN)\n");
		break;
	}

	OutputVolDescLog(
		"\t\t         DllCharacteristics: %04x\n"
		, pInh->OptionalHeader.DllCharacteristics
	);
	if (pInh->OptionalHeader.DllCharacteristics) {
		BOOL bSet = FALSE;
		if ((pInh->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE) == IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE) {
			OutputVolDescLog("\t\t                          => DLL can move\n");
			bSet = TRUE;
		}
		if ((pInh->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY) == IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY) {
			OutputVolDescLog("\t\t                          => Code Integrity Image\n");
			bSet = TRUE;
		}
		if ((pInh->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NX_COMPAT) == IMAGE_DLLCHARACTERISTICS_NX_COMPAT) {
			OutputVolDescLog("\t\t                          => Image is NX compatible\n");
			bSet = TRUE;
		}
		if ((pInh->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NO_ISOLATION) == IMAGE_DLLCHARACTERISTICS_NO_ISOLATION) {
			OutputVolDescLog("\t\t                          => Image understands isolation and doesn't want it\n");
			bSet = TRUE;
		}
		if ((pInh->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NO_SEH) == IMAGE_DLLCHARACTERISTICS_NO_SEH) {
			OutputVolDescLog("\t\t                          => Image does not use SEH.  No SE handler may reside in this image\n");
			bSet = TRUE;
		}
		if ((pInh->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NO_BIND) == IMAGE_DLLCHARACTERISTICS_NO_BIND) {
			OutputVolDescLog("\t\t                          => Do not bind this image\n");
			bSet = TRUE;
		}
		if ((pInh->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_WDM_DRIVER) == IMAGE_DLLCHARACTERISTICS_WDM_DRIVER) {
			OutputVolDescLog("\t\t                          => Driver uses WDM model\n");
			bSet = TRUE;
		}
		if ((pInh->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE) == IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE) {
			OutputVolDescLog("\t\t                          => Terminal Server Aware\n");
			bSet = TRUE;
		}
		if ((pInh->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NO_BIND) == IMAGE_DLLCHARACTERISTICS_NO_BIND) {
			OutputVolDescLog("\t\t                          => Do not bind this image\n");
			bSet = TRUE;
		}
		if (!bSet) {
			OutputVolDescLog("\t\t                          => Reserved\n");
		}
	}

	OutputVolDescLog(
		"\t\t         SizeOfStackReserve: %08lx\n"
		"\t\t          SizeOfStackCommit: %08lx\n"
		"\t\t          SizeOfHeapReserve: %08lx\n"
		"\t\t           SizeOfHeapCommit: %08lx\n"
		"\t\t                LoaderFlags: %08lx\n"
		"\t\t        NumberOfRvaAndSizes: %08lx\n"
		, pInh->OptionalHeader.SizeOfStackReserve, pInh->OptionalHeader.SizeOfStackCommit
		, pInh->OptionalHeader.SizeOfHeapReserve, pInh->OptionalHeader.SizeOfHeapCommit
		, pInh->OptionalHeader.LoaderFlags, pInh->OptionalHeader.NumberOfRvaAndSizes
		);
	for (INT i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
		switch (i) {
		case IMAGE_DIRECTORY_ENTRY_EXPORT:
			OutputVolDescLog("\t\tExport Directory\n");
			break;
		case IMAGE_DIRECTORY_ENTRY_IMPORT:
			OutputVolDescLog("\t\tImport Directory\n");
			break;
		case IMAGE_DIRECTORY_ENTRY_RESOURCE:
			OutputVolDescLog("\t\tResource Directory\n");
			break;
		case IMAGE_DIRECTORY_ENTRY_EXCEPTION:
			OutputVolDescLog("\t\tException Directory\n");
			break;
		case IMAGE_DIRECTORY_ENTRY_SECURITY:
			OutputVolDescLog("\t\tSecurity Directory\n");
			break;
		case IMAGE_DIRECTORY_ENTRY_BASERELOC:
			OutputVolDescLog("\t\tBase Relocation Table\n");
			break;
		case IMAGE_DIRECTORY_ENTRY_DEBUG:
			OutputVolDescLog("\t\tDebug Directory\n");
			break;
		case IMAGE_DIRECTORY_ENTRY_ARCHITECTURE:
			OutputVolDescLog("\t\tArchitecture Specific Data\n");
			break;
		case IMAGE_DIRECTORY_ENTRY_GLOBALPTR:
			OutputVolDescLog("\t\tRVA of GP\n");
			break;
		case IMAGE_DIRECTORY_ENTRY_TLS:
			OutputVolDescLog("\t\tTLS Directory\n");
			break;
		case IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG:
			OutputVolDescLog("\t\tLoad Configuration Directory\n");
			break;
		case IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT:
			OutputVolDescLog("\t\tBound Import Directory in headers\n");
			break;
		case IMAGE_DIRECTORY_ENTRY_IAT:
			OutputVolDescLog("\t\tImport Address Table\n");
			break;
		case IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT:
			OutputVolDescLog("\t\tDelay Load Import Descriptors\n");
			break;
		case IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR:
			OutputVolDescLog("\t\tCOM Runtime descriptor\n");
			break;
		default:
			OutputVolDescLog("\t\tReserved\n");
			break;
		}
		OutputVolDescLog(
			"\t\t\tVirtualAddress: %08lx\n"
			"\t\t\t          Size: %08lx\n"
			, pInh->OptionalHeader.DataDirectory[i].VirtualAddress
			, pInh->OptionalHeader.DataDirectory[i].Size
		);
	}
}

VOID OutputImportDirectory(
	LPBYTE lpBuf,
	DWORD dwBufSize,
	DWORD dwImportVirtualAddress,
	DWORD dwOfs
) {
	INT nDllNum = 0;
	size_t stDescOfs = 0;
	for (;;) {
		if (dwBufSize <= dwOfs + stDescOfs) {
			break;
		}
		PIMAGE_IMPORT_DESCRIPTOR imp = (PIMAGE_IMPORT_DESCRIPTOR)&lpBuf[dwOfs + stDescOfs];
		if (imp->OriginalFirstThunk == 0 && imp->TimeDateStamp == 0 &&
			imp->ForwarderChain == 0 && imp->Name == 0 && imp->FirstThunk == 0) {
			break;
		}
		else {
			if (dwBufSize > dwOfs + imp->Name - dwImportVirtualAddress) {
				OutputVolDescLog(
					"\t========== IMAGE_IMPORT_DESCRIPTOR %d ==========\n"
					"\t\tOriginalFirstThunk: %08lx\n"
					"\t\t     TimeDateStamp: %08lx\n"
					"\t\t    ForwarderChain: %08lx\n"
					"\t\t              Name: %08lx (%" CHARWIDTH "s)\n"
					"\t\t        FirstThunk: %08lx\n"
					, nDllNum + 1, imp->OriginalFirstThunk, imp->TimeDateStamp
					, imp->ForwarderChain, imp->Name
					, &lpBuf[dwOfs + imp->Name - dwImportVirtualAddress]
					, imp->FirstThunk
				);
				size_t stThunkOfs = 0;
				for (;;) {
					PIMAGE_THUNK_DATA32 thunk = 0;
					if (imp->OriginalFirstThunk && dwBufSize > dwOfs + imp->OriginalFirstThunk - dwImportVirtualAddress + stThunkOfs) {
						thunk = (PIMAGE_THUNK_DATA32)&lpBuf[dwOfs + imp->OriginalFirstThunk - dwImportVirtualAddress + stThunkOfs];
					}
					else if (imp->FirstThunk && dwBufSize > dwOfs + imp->FirstThunk - dwImportVirtualAddress + stThunkOfs) {
						thunk = (PIMAGE_THUNK_DATA32)&lpBuf[dwOfs + imp->FirstThunk - dwImportVirtualAddress + stThunkOfs];
					}
					else {
						break;
					}

					if (thunk->u1.AddressOfData == 0) {
						break;
					}
					else {
						OutputVolDescLog(
							"\t\t========== IMAGE_THUNK_DATA ==========\n"
							"\t\t\tAddressOfData: %08lx\n", thunk->u1.AddressOfData
						);
						if (dwBufSize > dwOfs + thunk->u1.AddressOfData - dwImportVirtualAddress) {
							PIMAGE_IMPORT_BY_NAME byname =
								(PIMAGE_IMPORT_BY_NAME)&lpBuf[dwOfs + thunk->u1.AddressOfData - dwImportVirtualAddress];
							OutputVolDescLog(
								"\t\t\t========== IMAGE_IMPORT_BY_NAME ==========\n"
								"\t\t\t\tHint: %04x\n"
								"\t\t\t\tName: %" CHARWIDTH "s\n"
								, byname->Hint, byname->Name
							);
						}
					}

					stThunkOfs += sizeof(IMAGE_THUNK_DATA32);
				}
			}
			stDescOfs += sizeof(IMAGE_IMPORT_DESCRIPTOR);
			nDllNum++;
		}
	}
}

BOOL IsKnownSectionName(
	LPCCH lpName
) {
	if (strcasestr(lpName, "bss") ||
		strcasestr(lpName, "tls") ||
		strcasestr(lpName, "text") ||
		strcasestr(lpName, "data") ||
		strcasestr(lpName, "rsrc") ||
		strcasestr(lpName, "orpc") ||
		strcasestr(lpName, "BINK") ||
		strcasestr(lpName, "CODE") ||
		strcasestr(lpName, "reloc") ||
		strcasestr(lpName, "debug") ||
		strcasestr(lpName, "drectve") ||
		strcasestr(lpName, "msvcjmc") ||
		strcasestr(lpName, "biosdll") ||
		strcasestr(lpName, "olebrk") ||
		strcasestr(lpName, "Process") ||
		strcasestr(lpName, "EDTQ") ||
		strcasestr(lpName, "H26xColo") ||
		strcasestr(lpName, "PAGE") ||
		strcasestr(lpName, "POOL") ||
		strcasestr(lpName, "INIT") ||
		strcasestr(lpName, "SHARED") ||
		strcasestr(lpName, "STL") ||
		strcasestr(lpName, "CRT") ||
		strcasestr(lpName, "GLOBAL_I") ||
		strcasestr(lpName, "LOCKMEM") ||
		strcasestr(lpName, "PNP") ||
		strcasestr(lpName, "MONITOR") ||
		strcasestr(lpName, "MISYSPTE") ||
		strcasestr(lpName, "MMX") ||
		strcasestr(lpName, "SECUR") ||
		strcasestr(lpName, "SEG") ||
		strcasestr(lpName, "UPX") ||
		strcasestr(lpName, "ENGINE") ||
		strcasestr(lpName, "_PARA_DA") ||
		strcasestr(lpName, "MSSMIXER")
		) {
		return TRUE;
	}
	return FALSE;
}

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms680341(v=vs.85).aspx
VOID OutputFsImageSectionHeader(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	PIMAGE_SECTION_HEADER pIsh,
	LPBOOL bSecurom
) {
	OutputVolDescLog(
		"\t========== Image Section Header (%zu byte) ==========\n"
		"\t                Name: %.8" CHARWIDTH "s\n"
		"\t         VirtualSize: %08lx\n"
		"\t      VirtualAddress: %08lx\n"
		"\t       SizeOfRawData: %08lx\n"
		"\t    PointerToRawData: %08lx\n"
		"\tPointerToRelocations: %08lx\n"
		"\tPointerToLinenumbers: %08lx\n"
		"\t NumberOfRelocations: %04x\n"
		"\t NumberOfLinenumbers: %04x\n"
		"\t     Characteristics: %08lx\n"
		, sizeof(IMAGE_SECTION_HEADER), pIsh->Name
		, pIsh->Misc.VirtualSize, pIsh->VirtualAddress, pIsh->SizeOfRawData, pIsh->PointerToRawData
		, pIsh->PointerToRelocations, pIsh->PointerToLinenumbers, pIsh->NumberOfRelocations
		, pIsh->NumberOfLinenumbers, pIsh->Characteristics
	);
	if ((pIsh->Characteristics & IMAGE_SCN_TYPE_NO_PAD) == IMAGE_SCN_TYPE_NO_PAD) {
		OutputVolDescLog(
			"\t                    => should not be padded to the next boundary\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_CNT_CODE) == IMAGE_SCN_CNT_CODE) {
		OutputVolDescLog(
			"\t                    => contains executable code\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) == IMAGE_SCN_CNT_INITIALIZED_DATA) {
		OutputVolDescLog(
			"\t                    => contains initialized data\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) == IMAGE_SCN_CNT_UNINITIALIZED_DATA) {
		OutputVolDescLog(
			"\t                    => contains uninitialized data\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_LNK_INFO) == IMAGE_SCN_LNK_INFO) {
		OutputVolDescLog(
			"\t                    => contains comments or other information\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_LNK_REMOVE) == IMAGE_SCN_LNK_REMOVE) {
		OutputVolDescLog(
			"\t                    => will not become part of the image\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_LNK_COMDAT) == IMAGE_SCN_LNK_COMDAT) {
		OutputVolDescLog(
			"\t                    => contains COMDAT data\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_NO_DEFER_SPEC_EXC) == IMAGE_SCN_NO_DEFER_SPEC_EXC) {
		OutputVolDescLog(
			"\t                    => Reset speculative exceptions handling bits in the TLB entries\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_GPREL) == IMAGE_SCN_GPREL) {
		OutputVolDescLog(
			"\t                    => contains data referenced through the global pointer\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_ALIGN_1BYTES) == IMAGE_SCN_ALIGN_1BYTES) {
		OutputVolDescLog(
			"\t                    => Align data on a 1-byte boundary\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_ALIGN_2BYTES) == IMAGE_SCN_ALIGN_2BYTES) {
		OutputVolDescLog(
			"\t                    => Align data on a 2-byte boundary\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_ALIGN_4BYTES) == IMAGE_SCN_ALIGN_4BYTES) {
		OutputVolDescLog(
			"\t                    => Align data on a 4-byte boundary\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_ALIGN_8BYTES) == IMAGE_SCN_ALIGN_8BYTES) {
		OutputVolDescLog(
			"\t                    => Align data on a 8-byte boundary\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_ALIGN_16BYTES) == IMAGE_SCN_ALIGN_16BYTES) {
		OutputVolDescLog(
			"\t                    => Align data on a 16-byte boundary\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_ALIGN_32BYTES) == IMAGE_SCN_ALIGN_32BYTES) {
		OutputVolDescLog(
			"\t                    => Align data on a 32-byte boundary\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_ALIGN_64BYTES) == IMAGE_SCN_ALIGN_64BYTES) {
		OutputVolDescLog(
			"\t                    => Align data on a 64-byte boundary\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_ALIGN_128BYTES) == IMAGE_SCN_ALIGN_128BYTES) {
		OutputVolDescLog(
			"\t                    => Align data on a 128-byte boundary\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_ALIGN_256BYTES) == IMAGE_SCN_ALIGN_256BYTES) {
		OutputVolDescLog(
			"\t                    => Align data on a 256-byte boundary\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_ALIGN_512BYTES) == IMAGE_SCN_ALIGN_512BYTES) {
		OutputVolDescLog(
			"\t                    => Align data on a 512-byte boundary\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_ALIGN_1024BYTES) == IMAGE_SCN_ALIGN_1024BYTES) {
		OutputVolDescLog(
			"\t                    => Align data on a 1024-byte boundary\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_ALIGN_2048BYTES) == IMAGE_SCN_ALIGN_2048BYTES) {
		OutputVolDescLog(
			"\t                    => Align data on a 2048-byte boundary\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_ALIGN_4096BYTES) == IMAGE_SCN_ALIGN_4096BYTES) {
		OutputVolDescLog(
			"\t                    => Align data on a 4096-byte boundary\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_ALIGN_8192BYTES) == IMAGE_SCN_ALIGN_8192BYTES) {
		OutputVolDescLog(
			"\t                    => Align data on a 8192-byte boundary\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_LNK_NRELOC_OVFL) == IMAGE_SCN_LNK_NRELOC_OVFL) {
		OutputVolDescLog(
			"\t                    => contains extended relocations\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_MEM_DISCARDABLE) == IMAGE_SCN_MEM_DISCARDABLE) {
		OutputVolDescLog(
			"\t                    => can be discarded as needed\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_MEM_NOT_CACHED) == IMAGE_SCN_MEM_NOT_CACHED) {
		OutputVolDescLog(
			"\t                    => cannot be cached\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_MEM_NOT_PAGED) == IMAGE_SCN_MEM_NOT_PAGED) {
		OutputVolDescLog(
			"\t                    => cannot be paged\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_MEM_SHARED) == IMAGE_SCN_MEM_SHARED) {
		OutputVolDescLog(
			"\t                    => can be shared in memory\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_MEM_EXECUTE) == IMAGE_SCN_MEM_EXECUTE) {
		OutputVolDescLog(
			"\t                    => can be executed as code\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_MEM_READ) == IMAGE_SCN_MEM_READ) {
		OutputVolDescLog(
			"\t                    => can be read\n"
		);
	}
	if ((pIsh->Characteristics & IMAGE_SCN_MEM_WRITE) == IMAGE_SCN_MEM_WRITE) {
		OutputVolDescLog(
			"\t                    => can be written to\n"
		);
	}

	if (pDisc != NULL) {
		if (!strncmp((LPCCH)pIsh->Name, "icd1", 4)) {
			pDisc->PROTECT.byExist = codelock;
			strncpy(pDisc->PROTECT.name[0], (LPCCH)pIsh->Name, sizeof(pDisc->PROTECT.name[0]));
			pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = pDisc->PROTECT.nNextLBAOfLastVolDesc;
			pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] =
				pDisc->PROTECT.nPrevLBAOfPathTablePos - pDisc->PROTECT.nNextLBAOfLastVolDesc;
		}
		else if (!strncmp((LPCCH)pIsh->Name, ".vob.pcd", 8)) {
			pDisc->PROTECT.byExist = protectCDVOB;
			strncpy(pDisc->PROTECT.name[0], (LPCCH)pIsh->Name, sizeof(pDisc->PROTECT.name[0]));
		}
		else if (!strncmp((LPCCH)pIsh->Name, ".cms_t", 6) || !strncmp((LPCCH)pIsh->Name, ".cms_d", 6)
			) {
			// This string exists SecuROM OLD "Re-Volt (Europe)" and SecuROM NEW "Supreme Snowboarding (Europe) and "Beam Breakers (Europe) etc"
			pDisc->PROTECT.byExist = securomTmp;
			strncpy(pDisc->PROTECT.name[0], (LPCCH)pIsh->Name, sizeof(pDisc->PROTECT.name[0]));
			*bSecurom = TRUE;
		}
		else if (pExtArg != NULL && pExtArg->byIntentionalSub && !IsKnownSectionName((LPCCH)pIsh->Name)) {
			// some SecuROM discs have random section names
			if (pDisc->PROTECT.byExist == no) {
				pDisc->PROTECT.byExist = securomTmp;
				strncpy(pDisc->PROTECT.name[0], (LPCCH)pIsh->Name, sizeof(pDisc->PROTECT.name[0]));
			}
			*bSecurom = TRUE;
		}
	}
}

VOID OutputSecuRomDll4_87Header(
	LPBYTE lpBuf,
	UINT i
) {
	BYTE scrTbl[] = {
		      0x00, 0x02,
		0x08, 0x03, 0x08,
		0x14, 0x06, 0x0e,
		// I don't know whether the upper data is correct
		0x20, 0x09, 0x14,
		0x2c, 0x0c, 0x1a,
		0x38, 0x0f, 0x20,
		0x44, 0x12, 0x26,
		0x50, 0x15, 0x2c,
		0x5c, 0x18, 0x32,
		0x68, 0x1b, 0x38,
		0x74, 0x1e, 0x3e,
		0x80, 0x21, 0x44,
		0x8c, 0x24, 0x4a,
		// I don't know whether the lower data is correct
		0x98, 0x27, 0x50,
		0xa4, 0x2a, 0x56,
		0xb0, 0x2d, 0x5c,
		0xbc, 0x30, 0x62,
		0xc8, 0x33, 0x68,
		0xd4, 0x36, 0x6e,
		0xe0, 0x39, 0x74,
		0xec, 0x3c, 0x7a,
		0xf8, 0x3f, 0x80,
		0x04, 0x42, 0x86,
		0x10, 0x45, 0x8c,
		0x1c, 0x48, 0x92,
		0x28, 0x4b, 0x98,
		0x34, 0x4e, 0x9e,
		0x40, 0x51
	};
	_TCHAR ver[8] = {};
	_sntprintf(ver, sizeof(ver), _T("%c%c%c%c%c%c%c")
		, lpBuf[8 + i] ^ scrTbl[8], lpBuf[9 + i] ^ scrTbl[9], lpBuf[10 + i] ^ scrTbl[10], lpBuf[11 + i] ^ scrTbl[11]
		, lpBuf[12 + i] ^ scrTbl[12], lpBuf[13 + i] ^ scrTbl[13], lpBuf[14 + i] ^ scrTbl[14]);
	OutputString("\nDetected SecuROM %s\n", ver);
	OutputVolDescLog(
		"\t" OUTPUT_DHYPHEN_PLUS_STR("SecuROM DLL Header")
		"\t\t         Signature: %02x %02x %02x %02x\n"
		"\t\t     Unknown Value: %02x %02x %02x %02x\n"
		"\t\t           Version: %s\n"
		"\t\t    Unknown String: %c%c%c%c\n"
		, lpBuf[0 + i] ^ scrTbl[0], lpBuf[1 + i] ^ scrTbl[1], lpBuf[2 + i] ^ scrTbl[2], lpBuf[3 + i] ^ scrTbl[3]
		, lpBuf[4 + i] ^ scrTbl[4], lpBuf[5 + i] ^ scrTbl[5], lpBuf[6 + i] ^ scrTbl[6], lpBuf[7 + i] ^ scrTbl[7]
		, ver
		, lpBuf[16 + i] ^ scrTbl[16], lpBuf[17 + i] ^ scrTbl[17], lpBuf[18 + i] ^ scrTbl[18], lpBuf[19 + i] ^ scrTbl[19]
	);
	for (INT k = 3, j = 0; k < 12; k++, j += 2) {
		OutputVolDescLog(
			"\t\tSecuROM Sector[%02d]: %5u (%04x)\n"
			, k, MAKEWORD(lpBuf[20 + i + j] ^ scrTbl[20 + j], lpBuf[21 + i + j] ^ scrTbl[21 + j])
			, MAKEWORD(lpBuf[20 + i + j] ^ scrTbl[20 + j], lpBuf[21 + i + j] ^ scrTbl[21 + j])
		);
	};
	for (INT m = 0, j = 0; m < 13; m++, j += 2) {
		OutputVolDescLog(
			"\t\tUnknown Value: %5u (%04x)\n"
			, MAKEWORD(lpBuf[38 + i + j] ^ scrTbl[38 + j], lpBuf[39 + i + j] ^ scrTbl[39 + j])
			, MAKEWORD(lpBuf[38 + i + j] ^ scrTbl[38 + j], lpBuf[39 + i + j] ^ scrTbl[39 + j])
		);
	}
	CHAR str[10] = {};
	for (INT m = 0, j = 0; m < 10; m++, j++) {
		str[m] = lpBuf[64 + i + j] ^ scrTbl[64 + j];
	}
	OutputVolDescLog(
		"\t\tUnknown String: %.10" CHARWIDTH "s\n", &str[0]
	);
	OutputVolDescLog(
		"\t\tOffset of SecuROM DLL Header: %5u (%04x)\n"
		, MAKEUINT(MAKEWORD(lpBuf[82 + i], lpBuf[83 + i]), MAKEWORD(lpBuf[84 + i], lpBuf[85 + i]))
		, MAKEUINT(MAKEWORD(lpBuf[82 + i], lpBuf[83 + i]), MAKEWORD(lpBuf[84 + i], lpBuf[85 + i]))
	);
	for (INT m = 0, j = 0; m < 20; m++, j += 2) {
		OutputVolDescLog(
			"\t\tUnknown Value: %5u (%04x)\n"
			, MAKEWORD(lpBuf[86 + i + j], lpBuf[87 + i + j])
			, MAKEWORD(lpBuf[86 + i + j], lpBuf[87 + i + j])
		);
	}
}

VOID OutputSecuRomDllHeader(
	LPBYTE lpBuf,
	LPUINT uiOfsOf16,
	LPUINT uiOfsOf32,
	LPUINT uiOfsOfNT,
	LPINT idx
) {
	OutputLog(standardOut | fileDisc, "\nDetected SecuROM %.8" CHARWIDTH "s\n", &lpBuf[8]);
	OutputVolDescLog(
		"\t" OUTPUT_DHYPHEN_PLUS_STR("SecuROM DLL Header")
		"\t\t         Signature: %.4" CHARWIDTH "s\n"
		"\t\t     Unknown Value: %08x\n"
		"\t\t           Version: %.8" CHARWIDTH "s\n"
		"\t\t    Unknown String: %.4" CHARWIDTH "s\n"
		, &lpBuf[0]
		, MAKEUINT(MAKEWORD(lpBuf[4], lpBuf[5]), MAKEWORD(lpBuf[6], lpBuf[7]))
		, &lpBuf[8]
		, &lpBuf[16]
	);
	for (INT i = 3, j = 0; i < 12; i++, j += 2) {
		OutputVolDescLog(
			"\t\tSecuROM Sector[%02d]: %5d (%04x)\n"
			, i, MAKEWORD(lpBuf[20 + j], lpBuf[21 + j]), MAKEWORD(lpBuf[20 + j], lpBuf[21 + j])
		);
	};
	for (INT i = 0, j = 0; i < 13; i++, j += 2) {
		OutputVolDescLog(
			"\t\tUnknown Value: %5d (%04x)\n"
			, MAKEWORD(lpBuf[38 + j], lpBuf[39 + j]), MAKEWORD(lpBuf[38 + j], lpBuf[39 + j])
		);
	}

	if (!strncmp((LPCCH)&lpBuf[8], "4.6", 3)) {
		*uiOfsOf16 = MAKEUINT(MAKEWORD(lpBuf[96], lpBuf[97]), MAKEWORD(lpBuf[98], lpBuf[99]));
		*uiOfsOf32 = MAKEUINT(MAKEWORD(lpBuf[144], lpBuf[145]), MAKEWORD(lpBuf[146], lpBuf[147]));
		*uiOfsOfNT = MAKEUINT(MAKEWORD(lpBuf[192], lpBuf[193]), MAKEWORD(lpBuf[194], lpBuf[195]));
		*idx = 32;
		OutputVolDescLog(
			"\t\tUnknown String: %.10" CHARWIDTH "s\n", &lpBuf[64]
		);
		for (INT i = 0, j = 0; i < 11; i++, j += 2) {
			OutputVolDescLog(
				"\t\tUnknown Value: %5d (%04x)\n"
				, MAKEWORD(lpBuf[74 + j], lpBuf[75 + j]), MAKEWORD(lpBuf[74 + j], lpBuf[75 + j])
			);
		}
	}
	else if (!strncmp((LPCCH)&lpBuf[8], "4.8", 3)) {
		*uiOfsOf16 = MAKEUINT(MAKEWORD(lpBuf[132], lpBuf[133]), MAKEWORD(lpBuf[134], lpBuf[135]));
		*uiOfsOf32 = MAKEUINT(MAKEWORD(lpBuf[180], lpBuf[181]), MAKEWORD(lpBuf[182], lpBuf[183]));
		*uiOfsOfNT = MAKEUINT(MAKEWORD(lpBuf[228], lpBuf[229]), MAKEWORD(lpBuf[230], lpBuf[231]));
		*idx = 68;
		OutputVolDescLog(
			"\t\tUnknown String: %.10" CHARWIDTH "s\n", &lpBuf[64]
		);
		for (INT i = 0, j = 0; i < 29; i++, j += 2) {
			OutputVolDescLog(
				"\t\tUnknown Value: %5d (%04x)\n"
				, MAKEWORD(lpBuf[74 + j], lpBuf[75 + j]), MAKEWORD(lpBuf[74 + j], lpBuf[75 + j])
			);
		}
	}
	else {
		*uiOfsOf16 = MAKEUINT(MAKEWORD(lpBuf[64], lpBuf[65]), MAKEWORD(lpBuf[66], lpBuf[67]));
		*uiOfsOf32 = MAKEUINT(MAKEWORD(lpBuf[112], lpBuf[113]), MAKEWORD(lpBuf[114], lpBuf[115]));
		*uiOfsOfNT = MAKEUINT(MAKEWORD(lpBuf[160], lpBuf[161]), MAKEWORD(lpBuf[162], lpBuf[163]));
	}

	OutputVolDescLog(
		"\t\t-----------------------\n"
		"\t\tOffset of dll: %08x\n"
		"\t\t  Size of dll: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\t         Name: %.12" CHARWIDTH "s\n"
		"\t\tUnknown Value: %08x\n"
		"\t\t-----------------------\n"
		"\t\tOffset of dll: %08x\n"
		"\t\t  Size of dll: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\t         Name: %.12" CHARWIDTH "s\n"
		"\t\tUnknown Value: %08x\n"
		"\t\t-----------------------\n"
		"\t\tOffset of dll: %08x\n"
		"\t\t  Size of dll: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\tUnknown Value: %08x\n"
		"\t\t         Name: %.12" CHARWIDTH "s\n"
		"\t\tUnknown Value: %08x\n"
		, *uiOfsOf16
		, MAKEUINT(MAKEWORD(lpBuf[68 + *idx], lpBuf[69 + *idx]), MAKEWORD(lpBuf[70 + *idx], lpBuf[71 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[72 + *idx], lpBuf[73 + *idx]), MAKEWORD(lpBuf[74 + *idx], lpBuf[75 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[76 + *idx], lpBuf[77 + *idx]), MAKEWORD(lpBuf[78 + *idx], lpBuf[79 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[80 + *idx], lpBuf[81 + *idx]), MAKEWORD(lpBuf[82 + *idx], lpBuf[83 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[84 + *idx], lpBuf[85 + *idx]), MAKEWORD(lpBuf[86 + *idx], lpBuf[87 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[88 + *idx], lpBuf[89 + *idx]), MAKEWORD(lpBuf[90 + *idx], lpBuf[91 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[92 + *idx], lpBuf[93 + *idx]), MAKEWORD(lpBuf[94 + *idx], lpBuf[95 + *idx]))
		, &lpBuf[96 + *idx]
		, MAKEUINT(MAKEWORD(lpBuf[108 + *idx], lpBuf[109 + *idx]), MAKEWORD(lpBuf[110 + *idx], lpBuf[111 + *idx]))
		, *uiOfsOf32
		, MAKEUINT(MAKEWORD(lpBuf[116 + *idx], lpBuf[117 + *idx]), MAKEWORD(lpBuf[118 + *idx], lpBuf[119 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[120 + *idx], lpBuf[121 + *idx]), MAKEWORD(lpBuf[122 + *idx], lpBuf[123 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[124 + *idx], lpBuf[125 + *idx]), MAKEWORD(lpBuf[126 + *idx], lpBuf[127 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[128 + *idx], lpBuf[129 + *idx]), MAKEWORD(lpBuf[130 + *idx], lpBuf[131 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[132 + *idx], lpBuf[133 + *idx]), MAKEWORD(lpBuf[134 + *idx], lpBuf[135 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[136 + *idx], lpBuf[137 + *idx]), MAKEWORD(lpBuf[138 + *idx], lpBuf[139 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[140 + *idx], lpBuf[141 + *idx]), MAKEWORD(lpBuf[142 + *idx], lpBuf[143 + *idx]))
		, &lpBuf[144 + *idx]
		, MAKEUINT(MAKEWORD(lpBuf[156 + *idx], lpBuf[157 + *idx]), MAKEWORD(lpBuf[158 + *idx], lpBuf[159 + *idx]))
		, *uiOfsOfNT
		, MAKEUINT(MAKEWORD(lpBuf[164 + *idx], lpBuf[165 + *idx]), MAKEWORD(lpBuf[166 + *idx], lpBuf[167 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[168 + *idx], lpBuf[169 + *idx]), MAKEWORD(lpBuf[160 + *idx], lpBuf[161 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[172 + *idx], lpBuf[173 + *idx]), MAKEWORD(lpBuf[174 + *idx], lpBuf[175 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[176 + *idx], lpBuf[177 + *idx]), MAKEWORD(lpBuf[178 + *idx], lpBuf[179 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[180 + *idx], lpBuf[181 + *idx]), MAKEWORD(lpBuf[182 + *idx], lpBuf[183 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[184 + *idx], lpBuf[185 + *idx]), MAKEWORD(lpBuf[186 + *idx], lpBuf[187 + *idx]))
		, MAKEUINT(MAKEWORD(lpBuf[188 + *idx], lpBuf[189 + *idx]), MAKEWORD(lpBuf[190 + *idx], lpBuf[191 + *idx]))
		, &lpBuf[192 + *idx]
		, MAKEUINT(MAKEWORD(lpBuf[204 + *idx], lpBuf[205 + *idx]), MAKEWORD(lpBuf[206 + *idx], lpBuf[207 + *idx]))
	);
}

VOID OutputSint16(
	LPBYTE lpBuf,
	UINT uiOfsOf16,
	UINT uiOfsOfSecuRomDll,
	INT idx
) {
	OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR("SIntf16.dll"));
	PIMAGE_DOS_HEADER pIDh2 = (PIMAGE_DOS_HEADER)&lpBuf[208 + idx];
	OutputFsImageDosHeader(pIDh2);
	PIMAGE_OS2_HEADER pIOh = (PIMAGE_OS2_HEADER)&lpBuf[uiOfsOf16 - uiOfsOfSecuRomDll + pIDh2->e_lfanew];
	OutputFsImageOS2Header(pIOh);
}

VOID OutputSint32(
	LPBYTE lpBuf,
	INT nOfsOf32dll,
	BOOL bDummy
) {
	OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR("SIntf32.dll"));
	PIMAGE_DOS_HEADER pIDh3 = (PIMAGE_DOS_HEADER)&lpBuf[nOfsOf32dll];
	OutputFsImageDosHeader(pIDh3);
	PIMAGE_NT_HEADERS32 pINH3 = (PIMAGE_NT_HEADERS32)&lpBuf[nOfsOf32dll + pIDh3->e_lfanew];
	OutputFsImageNtHeader(pINH3);
	ULONG nOfs3 = nOfsOf32dll + pIDh3->e_lfanew + sizeof(IMAGE_NT_HEADERS32);
	for (INT i = 0; i < pINH3->FileHeader.NumberOfSections; i++) {
		OutputFsImageSectionHeader(NULL, NULL, (PIMAGE_SECTION_HEADER)&lpBuf[nOfs3], &bDummy);
		nOfs3 += sizeof(IMAGE_SECTION_HEADER);
	}
}

VOID OutputSintNT(
	LPBYTE lpBuf,
	INT nOfsOfNTdll,
	BOOL bDummy
) {
	OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR("SIntfNT.dll"));
	PIMAGE_DOS_HEADER pIDh4 = (PIMAGE_DOS_HEADER)&lpBuf[nOfsOfNTdll];
	OutputFsImageDosHeader(pIDh4);
	PIMAGE_NT_HEADERS32 pINH4 = (PIMAGE_NT_HEADERS32)&lpBuf[nOfsOfNTdll + pIDh4->e_lfanew];
	OutputFsImageNtHeader(pINH4);
	ULONG nOfs4 = nOfsOfNTdll + pIDh4->e_lfanew + sizeof(IMAGE_NT_HEADERS32);
	for (INT i = 0; i < pINH4->FileHeader.NumberOfSections; i++) {
		OutputFsImageSectionHeader(NULL, NULL, (PIMAGE_SECTION_HEADER)&lpBuf[nOfs4], &bDummy);
		nOfs4 += sizeof(IMAGE_SECTION_HEADER);
	}
}

VOID OutputTocWithPregap(
	PDISC pDisc
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("TOC with pregap"));
	for (INT i = pDisc->SCSI.toc.FirstTrack - 1; i < pDisc->SCSI.toc.LastTrack; i++) {
		OutputDiscLog("\tTrack %2d, Ctl %u, Mode %u", i + 1,
			pDisc->SUB.lpCtlList[i], pDisc->MAIN.lpModeList[i]);

		for (UINT j = 0; j < MAXIMUM_NUMBER_INDEXES; j++) {
			if (pDisc->SUB.lp1stLBAListOnSub[i][j] != -1) {
				OutputDiscLog(", Index%u %6d", j, pDisc->SUB.lp1stLBAListOnSub[i][j]);
			}
			else if (j == 0) {
				OutputDiscLog(",              ");
			}
		}
		OutputDiscLog("\n");
	}
	if (pDisc->SUB.byDesync) {
		OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("TOC with pregap on desync"));
		for (INT i = pDisc->SCSI.toc.FirstTrack - 1; i < pDisc->SCSI.toc.LastTrack; i++) {
			OutputDiscLog("\tTrack %2d, Ctl %u, Mode %u", i + 1,
				pDisc->SUB.lpCtlList[i], pDisc->MAIN.lpModeList[i]);

			for (UINT j = 0; j < MAXIMUM_NUMBER_INDEXES; j++) {
				if (pDisc->SUB.lp1stLBAListOnSubSync[i][j] != -1) {
					OutputDiscLog(", Index%u %6d", j, pDisc->SUB.lp1stLBAListOnSubSync[i][j]);
				}
				else if (j == 0) {
					OutputDiscLog(",              ");
				}
			}
			OutputDiscLog("\n");
		}
	}
}

VOID OutputCDOffset(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	BOOL bGetDriveOffset,
	INT nDriveSampleOffset,
	INT nDriveOffset,
	INT nSubChannelOffset
) {
	OutputDiscLog(STR_DOUBLE_HYPHEN_B "Offset ");
	if (bGetDriveOffset) {
		OutputDiscLog("(Drive offset referes to http://www.accuraterip.com)");
	}
	OutputDiscLog(STR_DOUBLE_HYPHEN_E);

	if (pExtArg->byAdd && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
		pDisc->MAIN.nCombinedOffset += (INT)(pExtArg->nAudioCDOffsetNum * 4);
		pExtArg->nAudioCDOffsetNum = 0; // If it is possible, I want to repair it by a better method...
		OutputDiscLog(
			"\t       Combined Offset(Byte) %6d, (Samples) %5d\n"
			"\t-         Drive Offset(Byte) %6d, (Samples) %5d\n"
			"\t----------------------------------------------------\n"
			"\t User Specified Offset(Byte) %6d, (Samples) %5d\n",
			pDisc->MAIN.nCombinedOffset, pDisc->MAIN.nCombinedOffset / 4,
			nDriveOffset, nDriveSampleOffset,
			pDisc->MAIN.nCombinedOffset - nDriveOffset,
			(pDisc->MAIN.nCombinedOffset - nDriveOffset) / 4);
	}
	else {
		OutputDiscLog(
			"\t Combined Offset(Byte) %6d, (Samples) %5d\n"
			"\t-   Drive Offset(Byte) %6d, (Samples) %5d\n"
			"\t----------------------------------------------\n"
			"\t       CD Offset(Byte) %6d, (Samples) %5d\n",
			pDisc->MAIN.nCombinedOffset, pDisc->MAIN.nCombinedOffset / 4,
			nDriveOffset, nDriveSampleOffset,
			pDisc->MAIN.nCombinedOffset - nDriveOffset,
			(pDisc->MAIN.nCombinedOffset - nDriveOffset) / 4);
	}

	if (pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE == 0) {
		pDisc->MAIN.nAdjustSectorNum =
			pDisc->MAIN.nCombinedOffset / CD_RAW_SECTOR_SIZE;
	}
	else if (0 < pDisc->MAIN.nCombinedOffset) {
		pDisc->MAIN.nAdjustSectorNum =
			pDisc->MAIN.nCombinedOffset / CD_RAW_SECTOR_SIZE + 1;
	}
	else if (pDisc->MAIN.nCombinedOffset < 0) {
		pDisc->MAIN.nAdjustSectorNum =
			pDisc->MAIN.nCombinedOffset / CD_RAW_SECTOR_SIZE - 1;
	}
	OutputDiscLog("\tOverread sector: %d\n", pDisc->MAIN.nAdjustSectorNum);
	if (nSubChannelOffset != 0xff) {
		OutputDiscLog("\tSubChannel Offset: %d\n", nSubChannelOffset);
	}
}

VOID OutputCDC2Error296(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA
) {
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(type);
#endif
	OutputLog(type, OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("C2 error")
		"       +0 +1 +2 +3 +4 +5 +6 +7\n", nLBA, (UINT)nLBA);

	for (UINT i = 0; i < CD_RAW_READ_C2_SIZE; i += 8) {
		OutputLog(type, "%04X : %02X %02X %02X %02X %02X %02X %02X %02X\n"
			, i, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3]
			, lpBuf[i + 4], lpBuf[i + 5], lpBuf[i + 6], lpBuf[i + 7]);
	}
}

VOID OutputCDMain(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwSize
) {
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(type);
#endif
	OutputLog(type, OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Main Channel")
		"       +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F\n", nLBA, (UINT)nLBA);

	for (DWORD i = 0; i < dwSize; i += 16) {
		if (16 > dwSize - i) {
			OutputLog(type, "%04lX : ", i);
			for (DWORD j = 0; j < dwSize - i; j++) {
				if (j == 8) {
					OutputLog(type, " ");
				}
				OutputLog(type, "%02X ", lpBuf[i + j]);
			}
		}
		else {
			OutputLog(type,
				"%04lX : %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X   "
				, i, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3], lpBuf[i + 4], lpBuf[i + 5]
				, lpBuf[i + 6], lpBuf[i + 7], lpBuf[i + 8], lpBuf[i + 9], lpBuf[i + 10], lpBuf[i + 11]
				, lpBuf[i + 12], lpBuf[i + 13], lpBuf[i + 14], lpBuf[i + 15]);
			for (INT j = 0; j < 16; j++) {
				INT ch = isprint(lpBuf[i + j]) ? lpBuf[i + j] : '.';
				OutputLog(type, "%c", ch);
			}
		}
		OutputLog(type, "\n");
	}
}

VOID OutputCDSub96Align(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA
) {
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(type);
#endif
	OutputLog(type, OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Sub Channel")
		"\t  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B\n", nLBA, (UINT)nLBA);

	for (INT i = 0, ch = 0x50; i < CD_RAW_READ_SUBCODE_SIZE; i += 12, ch++) {
		OutputLog(type,
			"\t%c %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n"
			, ch, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3], lpBuf[i + 4], lpBuf[i + 5]
			, lpBuf[i + 6], lpBuf[i + 7], lpBuf[i + 8], lpBuf[i + 9], lpBuf[i + 10], lpBuf[i + 11]);
	}
}

VOID OutputCDSub96Raw(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA
) {
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(type);
#endif
	OutputLog(type,
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("Sub Channel(Raw)")
		"       +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F\n", nLBA, (UINT)nLBA);

	for (UINT i = 0; i < CD_RAW_READ_SUBCODE_SIZE; i += 16) {
		OutputLog(type,
			"%04X : %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X\n"
			, i, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3], lpBuf[i + 4], lpBuf[i + 5]
			, lpBuf[i + 6], lpBuf[i + 7], lpBuf[i + 8], lpBuf[i + 9], lpBuf[i + 10], lpBuf[i + 11]
			, lpBuf[i + 12], lpBuf[i + 13], lpBuf[i + 14], lpBuf[i + 15]);
	}
}

VOID OutputCDSubToLog(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	LPBYTE lpSubcodeRaw,
	INT nLBA
) {
	OutputSubReadableLog(
		STR_LBA "P[%02x], Q[%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]{"
		, nLBA, (UINT)nLBA, pDiscPerSector->subcode.current[0], pDiscPerSector->subcode.current[12]
		, pDiscPerSector->subcode.current[13], pDiscPerSector->subcode.current[14]
		, pDiscPerSector->subcode.current[15], pDiscPerSector->subcode.current[16]
		, pDiscPerSector->subcode.current[17], pDiscPerSector->subcode.current[18]
		, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
		, pDiscPerSector->subcode.current[21], pDiscPerSector->subcode.current[22]
		, pDiscPerSector->subcode.current[23]);

	// Ctl
	switch ((pDiscPerSector->subcode.current[12] >> 4) & 0x0f) {
	case 0:
		OutputSubReadableLog(
			"Audio, 2ch, Copy NG, Pre-emphasis No, ");
		break;
	case AUDIO_WITH_PREEMPHASIS:
		OutputSubReadableLog(
			"Audio, 2ch, Copy NG, Pre-emphasis Yes, ");
		break;
	case DIGITAL_COPY_PERMITTED:
		OutputSubReadableLog(
			"Audio, 2ch, Copy OK, Pre-emphasis No, ");
		break;
	case DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		OutputSubReadableLog(
			"Audio, 2ch, Copy OK, Pre-emphasis Yes, ");
		break;
	case AUDIO_DATA_TRACK:
		OutputSubReadableLog(
			" Data,      Copy NG,                  ");
		break;
	case AUDIO_DATA_TRACK | DIGITAL_COPY_PERMITTED:
		OutputSubReadableLog(
			" Data,      Copy OK,                  ");
		break;
	case TWO_FOUR_CHANNEL_AUDIO:
		OutputSubReadableLog(
			"Audio, 4ch, Copy NG, Pre-emphasis No, ");
		break;
	case TWO_FOUR_CHANNEL_AUDIO | AUDIO_WITH_PREEMPHASIS:
		OutputSubReadableLog(
			"Audio, 4ch, Copy NG, Pre-emphasis Yes, ");
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED:
		OutputSubReadableLog(
			"Audio, 4ch, Copy OK, Pre-emphasis No, ");
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		OutputSubReadableLog(
			"Audio, 4ch, Copy OK, Pre-emphasis Yes, ");
		break;
	default:
		OutputSubReadableLog(
			"Unknown,                              ");
		break;
	}

	// ADR
	switch (pDiscPerSector->subcode.current[12] & 0x0f) {
	case ADR_ENCODES_CURRENT_POSITION:
		if (pDiscPerSector->subcode.current[13] == 0) {
			// lead-in area
			if (pDiscPerSector->subcode.current[14] == 0xa0) {
				OutputSubReadableLog(
					"Point[%02x], AMSF[%02x:%02x:%02x], TrackNumOf1stTrack[%02x], ProgramAreaFormat[%02x]"
					, pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
					, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]);
			}
			else if (pDiscPerSector->subcode.current[14] == 0xa1) {
				OutputSubReadableLog(
					"Point[%02x], AMSF[%02x:%02x:%02x], TrackNumOfLastTrack[%02x]"
					, pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
					, pDiscPerSector->subcode.current[19]);
			}
			else if (pDiscPerSector->subcode.current[14] == 0xa2) {
				OutputSubReadableLog(
					"Point[%02x], AMSF[%02x:%02x:%02x], StartTimeOfLead-out[%02x:%02x:%02x]"
					, pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
					, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
					, pDiscPerSector->subcode.current[21]);
			}
			else {
				OutputSubReadableLog(
					"Point[%02x], AMSF[%02x:%02x:%02x], StartTimeOfTrack[%02x:%02x:%02x]"
					, pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
					, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
					, pDiscPerSector->subcode.current[21]);
			}
			if (pDiscPerSector->subcode.prev[13] == 0xaa) {
				pDisc->SCSI.nLeadoutLenOf1stSession = nLBA - pDisc->SCSI.n1stLBAofLeadout;
				OutputLog(standardOut | fileDisc
					, " Lead-out length of 1st session: %d\n", pDisc->SCSI.nLeadoutLenOf1stSession);
				pDisc->SCSI.n1stLBAofLeadin = nLBA;
			}
		}
		else if (pDiscPerSector->subcode.current[13] == 0xaa) {
			// lead-out area
			OutputSubReadableLog(
				"LeadOut  , Idx[%02x], RMSF[%02x:%02x:%02x], AMSF[%02x:%02x:%02x]"
				, pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
				, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
				, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
				, pDiscPerSector->subcode.current[21]);
		}
		else {
			OutputSubReadableLog(
				"Track[%02x], Idx[%02x], RMSF[%02x:%02x:%02x], AMSF[%02x:%02x:%02x]"
				, pDiscPerSector->subcode.current[13], pDiscPerSector->subcode.current[14]
				, pDiscPerSector->subcode.current[15], pDiscPerSector->subcode.current[16]
				, pDiscPerSector->subcode.current[17], pDiscPerSector->subcode.current[19]
				, pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);

			if (pDisc->SCSI.bMultiSession && pDiscPerSector->subcode.current[13] > 1) {
				if ((pDiscPerSector->subcode.prev[13] == 0 &&
					(pDiscPerSector->subcode.prev[14] == 0xa0 ||
					pDiscPerSector->subcode.prev[14] == 0xa1 ||
					pDiscPerSector->subcode.prev[14] == 0xa2)) ||
					(((pDiscPerSector->subch.current.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) &&
					pDiscPerSector->subcode.current[14] == 0 && pDiscPerSector->subcode.current[15] == 0 &&
					pDiscPerSector->subcode.current[16] == 1 && pDiscPerSector->subcode.current[17] == 0x74)) {
					if (pDisc->MAIN.nAdjustSectorNum < 0) {
						nLBA += pDisc->MAIN.nAdjustSectorNum;
					}
					pDisc->SCSI.nLeadinLenOf2ndSession = nLBA - pDisc->SCSI.n1stLBAofLeadin;
					OutputLog(standardOut | fileDisc
						, " Lead-in length of 2nd session: %d\n", pDisc->SCSI.nLeadinLenOf2ndSession);
					pDisc->SCSI.nEndLBAOfLeadin = nLBA;
				}
				else if (pDisc->SCSI.nPregapLenOf1stTrkOf2ndSession == 0 &&
					BcdToDec(pDiscPerSector->subcode.current[13]) == pDisc->SCSI.by1stMultiSessionTrkNum &&
					pDiscPerSector->subcode.prev[14] == 0 && pDiscPerSector->subcode.current[14] == 1) {
					pDisc->SCSI.nPregapLenOf1stTrkOf2ndSession = nLBA - pDisc->SCSI.nEndLBAOfLeadin;
					OutputLog(standardOut | fileDisc
						, " Pregap length of 1st track of 2nd session: %d\n", pDisc->SCSI.nPregapLenOf1stTrkOf2ndSession);
				}
			}
		}
		break;
	case ADR_ENCODES_MEDIA_CATALOG: {
		_TCHAR szCatalog[META_CATALOG_SIZE] = {};
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0
			, pDisc->SUB.szCatalog, sizeof(pDisc->SUB.szCatalog), szCatalog, sizeof(szCatalog));
#else
		strncpy(szCatalog, pDisc->SUB.szCatalog, sizeof(szCatalog) / sizeof(szCatalog[0]));
#endif
		OutputSubReadableLog(
			"MediaCatalogNumber [%13s], AMSF[     :%02x]", szCatalog, pDiscPerSector->subcode.current[21]);
		break;
	}
	case ADR_ENCODES_ISRC: {
		if (pDiscPerSector->byTrackNum == 0 || pDisc->SCSI.toc.LastTrack < pDiscPerSector->byTrackNum) {
			OutputSubErrorWithLBALog(" Invalid Adr\n", nLBA, pDiscPerSector->byTrackNum);
		}
		else {
			_TCHAR szISRC[META_ISRC_SIZE] = {};
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, 0
				, pDisc->SUB.pszISRC[pDiscPerSector->byTrackNum - 1], META_ISRC_SIZE, szISRC, sizeof(szISRC));
#else
			strncpy(szISRC, pDisc->SUB.pszISRC[pDiscPerSector->byTrackNum - 1], sizeof(szISRC) / sizeof(szISRC[0]) - 1);
#endif
			OutputSubReadableLog(
				"ItnStdRecordingCode [%12s], AMSF[     :%02x]", szISRC, pDiscPerSector->subcode.current[21]);
		}
		break;
	}
	case 5:
		if (pDiscPerSector->subcode.current[13] == 0) {
			if (pDiscPerSector->subcode.current[14] == 0xb0) {
				OutputSubReadableLog(
					"Point[%02x], StartTimeForTheNextSession[%02x:%02x:%02x], NumberOfDifferentMode-5[%02x], OutermostLead-out[%02x:%02x:%02x]"
					, pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
					, pDiscPerSector->subcode.current[18], pDiscPerSector->subcode.current[19]
					, pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);
			}
			else if (pDiscPerSector->subcode.current[14] == 0xb1) {
				OutputSubReadableLog(
					"Point[%02x], NumberOfSkipIntervalPointers[%02x], NumberOfSkipTrackAssignmentsInPoint[%02x]",
					pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]);
			}
			else if (pDiscPerSector->subcode.current[14] == 0xb2 ||
				pDiscPerSector->subcode.current[14] == 0xb3 || pDiscPerSector->subcode.current[14] == 0xb4) {
				OutputSubReadableLog(
					"Point[%02x], TrackNumberToSkipUponPlayback[%02x %02x %02x %02x %02x %02x %02x]"
					, pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
					, pDiscPerSector->subcode.current[18], pDiscPerSector->subcode.current[19]
					, pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);
			}
			else if (pDiscPerSector->subcode.current[14] == 0xc0) {
				OutputSubReadableLog(
					"Point[%02x], OptimumRecordingPower[%02x], StartTimeOfTheFirstLead-in[%02x:%02x:%02x]"
					, pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
					, pDiscPerSector->subcode.current[21]);
			}
			else if (pDiscPerSector->subcode.current[14] == 0xc1) {
				OutputSubReadableLog(
					"Point[%02x], CopyOfInfoFromA1Point[%02x %02x %02x %02x %02x %02x %02x]"
					, pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
					, pDiscPerSector->subcode.current[18], pDiscPerSector->subcode.current[19]
					, pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);
			}
			else {
				OutputSubReadableLog(
					"Point[%02x], SkipIntervalStopTime[%02x:%02x:%02x], SkipIntervalStartTime[%02x:%02x:%02x]"
					, pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
					, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
					, pDiscPerSector->subcode.current[21]);
			}
		}
		else if (pDiscPerSector->subcode.current[13] == 0xaa) {
			// lead-out area
			OutputSubReadableLog(
				"LeadOutAdr5, Track[%02u], Idx[%02x], StartTime[%02x:%02x:%02x]"
				, BcdToDec((BYTE)(pDiscPerSector->subcode.current[14] >> 4 & 0x0f))
				, pDiscPerSector->subcode.current[14] & 0x0f, pDiscPerSector->subcode.current[19]
				, pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);
		}
		break;
	case ADR_ENCODES_6:
		OutputSubReadableLog(
			"Unknown Data    [%02x%02x%02x%02x%02x%02x%02x%02x], AMSF[     :%02x]"
			, pDiscPerSector->subcode.current[13], pDiscPerSector->subcode.current[14]
			, pDiscPerSector->subcode.current[15], pDiscPerSector->subcode.current[16]
			, pDiscPerSector->subcode.current[17], pDiscPerSector->subcode.current[18]
			, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
			, pDiscPerSector->subcode.current[21]);
		break;
	case 0x0c: // I forgot what this is...
		if (pDiscPerSector->subcode.current[13] == 0) {
			if (pDiscPerSector->subcode.current[14] == 0xb1) {
				OutputSubReadableLog(
					"Point[%02x], 15[%02x], 16[%02x], 17[%02x], 18[%02x], 19[%02x], 20[%02x], 21[%02x]"
					, pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
					, pDiscPerSector->subcode.current[18], pDiscPerSector->subcode.current[19]
					, pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);
			}
		}
		break;
	default:
		OutputSubReadableLog(
			"Adr[%02x], Track[%02x], Idx[%02x], RMSF[%02x:%02x:%02x], AMSF[%02x:%02x:%02x]"
			, pDiscPerSector->subcode.current[12], pDiscPerSector->subcode.current[13]
			, pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
			, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
			, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
			, pDiscPerSector->subcode.current[21]);
		break;
	}

	SUB_R_TO_W scRW[4] = {};
	BYTE tmpCode[24] = {};
	OutputSubReadableLog( "}, RtoW[");
	for (INT i = 0; i < 4; i++) {
		for (INT j = 0; j < 24; j++) {
			tmpCode[j] = (BYTE)(*(lpSubcodeRaw + (i * 24 + j)) & 0x3f);
		}
		memcpy(&scRW[i], tmpCode, sizeof(scRW[i]));
		switch (scRW[i].command) {
		case 0: // MODE 0, ITEM 0
			OutputSubReadableLog("0");
			break;
		case 8: // MODE 1, ITEM 0
			OutputSubReadableLog("LINE-GRAPHICS");
			switch (scRW[i].instruction) {
			case 4:
				OutputSubReadableLog("->Write FONT");
				break;
			case 12:
				OutputSubReadableLog("->Soft scroll SCREEN");
				break;
			default:
				OutputSubReadableLog("->Unknown[%02d]", scRW[i].instruction);
				break;
			}
			break;
		case 9: // MODE 1, ITEM 1
			OutputSubReadableLog("TV-GRAPHICS");
			switch (scRW[i].instruction) {
			case 1:
				OutputSubReadableLog("->Preset MEMORY");
				break;
			case 2:
				OutputSubReadableLog("->Preset BORDER");
				break;
			case 6:
				OutputSubReadableLog("->Write FONT FORE/BACKGROUND");
				break;
			case 20:
				OutputSubReadableLog("->Soft scroll SCREEN with preset");
				break;
			case 24:
				OutputSubReadableLog("->Soft scroll SCREEN with copy");
				break;
			case 28:
				OutputSubReadableLog("->Define color transparency");
				break;
			case 30:
				OutputSubReadableLog("->Load CLUT 0 .. 7");
				break;
			case 31:
				OutputSubReadableLog("->Load CLUT 8 .. 15");
				break;
			case 38:
				OutputSubReadableLog("->XOR FONT with 2 colors");
				break;
			default:
				OutputSubReadableLog("->Unknown[%02d]", scRW[i].instruction);
				break;
			}
			break;
		case 10: // MODE 1, ITEM 2
			OutputSubReadableLog("Extended-TV-Graphics");
			switch (scRW[i].instruction) {
			case 3:
				OutputSubReadableLog("->MEMORY control");
				break;
			case 6:
				OutputSubReadableLog("->Write Additional FONT FORE/BACKGROUND");
				break;
			case 14:
				OutputSubReadableLog("->XOR additional FONT with 2 colors");
				break;
			case 16:
				OutputSubReadableLog("->Load CLUT 0 .. 7");
				break;
			case 17:
				OutputSubReadableLog("->Load CLUT 8 .. 15");
				break;
			case 18:
				OutputSubReadableLog("->Load CLUT 16 .. 23");
				break;
			case 19:
				OutputSubReadableLog("->Load CLUT 24 .. 31");
				break;
			case 20:
				OutputSubReadableLog("->Load CLUT 32 .. 39");
				break;
			case 21:
				OutputSubReadableLog("->Load CLUT 40 .. 47");
				break;
			case 22:
				OutputSubReadableLog("->Load CLUT 48 .. 55");
				break;
			case 23:
				OutputSubReadableLog("->Load CLUT 56 .. 63");
				break;
			case 24:
				OutputSubReadableLog("->Load CLUT 64 .. 71");
				break;
			case 25:
				OutputSubReadableLog("->Load CLUT 72 .. 79");
				break;
			case 26:
				OutputSubReadableLog("->Load CLUT 80 .. 87");
				break;
			case 27:
				OutputSubReadableLog("->Load CLUT 88 .. 95");
				break;
			case 28:
				OutputSubReadableLog("->Load CLUT 96 .. 103");
				break;
			case 29:
				OutputSubReadableLog("->Load CLUT 104 .. 111");
				break;
			case 30:
				OutputSubReadableLog("->Load CLUT 112 .. 119");
				break;
			case 31:
				OutputSubReadableLog("->Load CLUT 120 .. 127");
				break;
			case 32:
				OutputSubReadableLog("->Load CLUT 128 .. 135");
				break;
			case 33:
				OutputSubReadableLog("->Load CLUT 136 .. 143");
				break;
			case 34:
				OutputSubReadableLog("->Load CLUT 144 .. 151");
				break;
			case 35:
				OutputSubReadableLog("->Load CLUT 152 .. 159");
				break;
			case 36:
				OutputSubReadableLog("->Load CLUT 160 .. 167");
				break;
			case 37:
				OutputSubReadableLog("->Load CLUT 168 .. 175");
				break;
			case 38:
				OutputSubReadableLog("->Load CLUT 176 .. 183");
				break;
			case 39:
				OutputSubReadableLog("->Load CLUT 184 .. 191");
				break;
			case 40:
				OutputSubReadableLog("->Load CLUT 192 .. 199");
				break;
			case 41:
				OutputSubReadableLog("->Load CLUT 200 .. 207");
				break;
			case 42:
				OutputSubReadableLog("->Load CLUT 208 .. 215");
				break;
			case 43:
				OutputSubReadableLog("->Load CLUT 216 .. 223");
				break;
			case 44:
				OutputSubReadableLog("->Load CLUT 224 .. 231");
				break;
			case 45:
				OutputSubReadableLog("->Load CLUT 232 .. 239");
				break;
			case 46:
				OutputSubReadableLog("->Load CLUT 240 .. 247");
				break;
			case 47:
				OutputSubReadableLog("->Load CLUT 248 .. 255");
				break;
			case 48:
				OutputSubReadableLog("->Load CLUT additional 0 .. 15");
				break;
			case 49:
				OutputSubReadableLog("->Load CLUT additional 16 .. 31");
				break;
			case 50:
				OutputSubReadableLog("->Load CLUT additional 32 .. 47");
				break;
			case 51:
				OutputSubReadableLog("->Load CLUT additional 48 .. 63");
				break;
			case 52:
				OutputSubReadableLog("->Load CLUT additional 64 .. 79");
				break;
			case 53:
				OutputSubReadableLog("->Load CLUT additional 80 .. 95");
				break;
			case 54:
				OutputSubReadableLog("->Load CLUT additional 96 .. 111");
				break;
			case 55:
				OutputSubReadableLog("->Load CLUT additional 112 .. 127");
				break;
			case 56:
				OutputSubReadableLog("->Load CLUT additional 128 .. 143");
				break;
			case 57:
				OutputSubReadableLog("->Load CLUT additional 144 .. 159");
				break;
			case 58:
				OutputSubReadableLog("->Load CLUT additional 160 .. 175");
				break;
			case 59:
				OutputSubReadableLog("->Load CLUT additional 176 .. 181");
				break;
			case 60:
				OutputSubReadableLog("->Load CLUT additional 192 .. 207");
				break;
			case 61:
				OutputSubReadableLog("->Load CLUT additional 208 .. 223");
				break;
			case 62:
				OutputSubReadableLog("->Load CLUT additional 224 .. 239");
				break;
			case 63:
				OutputSubReadableLog("->Load CLUT additional 240 .. 255");
				break;
			default:
				OutputSubReadableLog("->Unknown[%02d]", scRW[i].instruction);
				break;
			}
			break;
		case 17: // MODE 2, ITEM 1,2,3,5,6,7 or MODE 4
		case 18:
		case 19:
		case 21:
		case 22:
		case 23:
		case 32:
			OutputSubReadableLog("CD TEXT");
			break;
		case 24: // MODE 3, ITEM 0
			OutputSubReadableLog("MIDI");
			break;
		case 56: // MODE 7, ITEM 0
			OutputSubReadableLog("USER");
			break;
		default:
			OutputSubReadableLog("Unknown[%02d]", scRW[i].command);
			break;
		}

		if (i < 3) {
			OutputSubReadableLog(", ");
		}
		else {
			OutputSubReadableLog("]\n");
		}
	}
}
