/**
 * Copyright 2011-2020 sarami
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

VOID OutputFsImageDosHeader(
	PIMAGE_DOS_HEADER pIdh
) {
	OutputVolDescLogA(
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
		"\t   File address of new exe header: %08lx\n"
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
	OutputVolDescLogA(
		"\t========== Image OS/2 .EXE header (%zu byte) ==========\n"
		"\t                      Magic number: %04x\n"
		"\t                    Version number: %02x\n"
		"\t                   Revision number: %02x\n"
		"\t             Offset of Entry Table: %04x\n"
		"\t    Number of bytes in Entry Table: %04x\n"
		"\t            Checksum of whole file: %08lx\n"
		"\t                         Flag word: %04x\n"
		"\t     Automatic data segment number: %04x\n"
		"\t           Initial heap allocation: %04x\n"
		"\t          Initial stack allocation: %04x\n"
		"\t             Initial CS:IP setting: %08lx\n"
		"\t             Initial SS:SP setting: %08lx\n"
		"\t            Count of file segments: %04x\n"
		"\t Entries in Module Reference Table: %04x\n"
		"\t   Size of non-resident name table: %04x\n"
		"\t           Offset of Segment Table: %04x\n"
		"\t          Offset of Resource Table: %04x\n"
		"\t     Offset of resident name table: %04x\n"
		"\t  Offset of Module Reference Table: %04x\n"
		"\t    Offset of Imported Names Table: %04x\n"
		"\tOffset of Non-resident Names Table: %08lx\n"
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
	OutputVolDescLogA(
		"\t========== Image NT Header (%zu byte) ==========\n"
		"\tSignature: %08lx\n"
		"\t========== FileHeader ==========\n"
		"\t\t             Machine: %04x\n"
		"\t\t    NumberOfSections: %04x\n"
		"\t\t       TimeDateStamp: %08lx\n"
		"\t\tPointerToSymbolTable: %08lx\n"
		"\t\t     NumberOfSymbols: %08lx\n"
		"\t\tSizeOfOptionalHeader: %04x\n"
		"\t\t     Characteristics: %04x\n"
		"\t========== OptionalHeader ==========\n"
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
		"\t\t                  Subsystem: %04x\n"
		"\t\t         DllCharacteristics: %04x\n"
		"\t\t         SizeOfStackReserve: %08lx\n"
		"\t\t          SizeOfStackCommit: %08lx\n"
		"\t\t          SizeOfHeapReserve: %08lx\n"
		"\t\t           SizeOfHeapCommit: %08lx\n"
		"\t\t                LoaderFlags: %08lx\n"
		"\t\t        NumberOfRvaAndSizes: %08lx\n"
		"\t\t              DataDirectory:\n"
		, sizeof(IMAGE_NT_HEADERS32)
		, pInh->Signature, pInh->FileHeader.Machine, pInh->FileHeader.NumberOfSections
		, pInh->FileHeader.TimeDateStamp, pInh->FileHeader.PointerToSymbolTable
		, pInh->FileHeader.NumberOfSymbols, pInh->FileHeader.SizeOfOptionalHeader
		, pInh->FileHeader.Characteristics
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
		, pInh->OptionalHeader.Subsystem, pInh->OptionalHeader.DllCharacteristics
		, pInh->OptionalHeader.SizeOfStackReserve, pInh->OptionalHeader.SizeOfStackCommit
		, pInh->OptionalHeader.SizeOfHeapReserve, pInh->OptionalHeader.SizeOfHeapCommit
		, pInh->OptionalHeader.LoaderFlags, pInh->OptionalHeader.NumberOfRvaAndSizes
		);
	for (INT i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
		OutputVolDescLogA(
			"\t\t\tVirtualAddress[%d]: %08lx\n"
			"\t\t\t          Size[%d]: %08lx\n"
			, i, pInh->OptionalHeader.DataDirectory[i].VirtualAddress
			, i, pInh->OptionalHeader.DataDirectory[i].Size
			);
	}
}

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms680341(v=vs.85).aspx
VOID OutputFsImageSectionHeader(
	PDISC pDisc,
	PIMAGE_SECTION_HEADER pIsh
) {
	OutputVolDescLogA(
		"\t========== Image Section Header (%zu byte) ==========\n"
		"\t                Name: %.8s\n"
		"\t     PhysicalAddress: %08lx\n"
		"\t         VirtualSize: %08lx\n"
		"\t      VirtualAddress: %08lx\n"
		"\t       SizeOfRawData: %08lx\n"
		"\t    PointerToRawData: %08lx\n"
		"\tPointerToRelocations: %08lx\n"
		"\tPointerToLinenumbers: %08lx\n"
		"\t NumberOfRelocations: %04x\n"
		"\t NumberOfLinenumbers: %04x\n"
		"\t     Characteristics: %08lx\n"
		, sizeof(IMAGE_SECTION_HEADER), pIsh->Name, pIsh->Misc.PhysicalAddress
		, pIsh->Misc.VirtualSize, pIsh->VirtualAddress, pIsh->SizeOfRawData, pIsh->PointerToRawData
		, pIsh->PointerToRelocations, pIsh->PointerToLinenumbers, pIsh->NumberOfRelocations
		, pIsh->NumberOfLinenumbers, pIsh->Characteristics
	);
	if (!strncmp((LPCH)pIsh->Name, "icd1", 4)) {
		pDisc->PROTECT.byExist = codelock;
		strncpy(pDisc->PROTECT.name[0], (LPCH)pIsh->Name, sizeof(pIsh->Name));
		pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] = pDisc->PROTECT.nNextLBAOfLastVolDesc;
		pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] =
			pDisc->PROTECT.nPrevLBAOfPathTablePos - pDisc->PROTECT.nNextLBAOfLastVolDesc;
	}
	else if (!strncmp((LPCH)pIsh->Name, ".vob.pcd", 8)) {
		pDisc->PROTECT.byExist = protectCDVOB;
		strncpy(pDisc->PROTECT.name[0], (LPCH)pIsh->Name, sizeof(pIsh->Name));
	}
	else if (!strncmp((LPCH)pIsh->Name, ".cms_t", 6) || !strncmp((LPCH)pIsh->Name, ".cms_d", 6)) {
		// This string exists SecuROM OLD "Re-Volt (Europe)" and SecuROM NEW "Supreme Snowboarding (Europe) and "Beam Breakers (Europe) etc"
		pDisc->PROTECT.byExist = securomTmp;
		strncpy(pDisc->PROTECT.name[0], (LPCH)pIsh->Name, sizeof(pIsh->Name));
	}
}

VOID OutputTocWithPregap(
	PDISC pDisc
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(TOC with pregap));
	for (INT i = pDisc->SCSI.toc.FirstTrack - 1; i < pDisc->SCSI.toc.LastTrack; i++) {
		OutputDiscLogA("\tTrack %2u, Ctl %u, Mode %u", i + 1,
			pDisc->SUB.lpCtlList[i], pDisc->MAIN.lpModeList[i]);

		for (UINT j = 0; j < MAXIMUM_NUMBER_INDEXES; j++) {
			if (pDisc->SUB.lp1stLBAListOnSub[i][j] != -1) {
				OutputDiscLogA(", Index%u %6d", j, pDisc->SUB.lp1stLBAListOnSub[i][j]);
			}
			else if (j == 0) {
				OutputDiscLogA(",              ");
			}
		}
		OutputDiscLogA("\n");
	}
	if (pDisc->SUB.byDesync) {
		OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(TOC with pregap on desync));
		for (INT i = pDisc->SCSI.toc.FirstTrack - 1; i < pDisc->SCSI.toc.LastTrack; i++) {
			OutputDiscLogA("\tTrack %2u, Ctl %u, Mode %u", i + 1,
				pDisc->SUB.lpCtlList[i], pDisc->MAIN.lpModeList[i]);

			for (UINT j = 0; j < MAXIMUM_NUMBER_INDEXES; j++) {
				if (pDisc->SUB.lp1stLBAListOnSubSync[i][j] != -1) {
					OutputDiscLogA(", Index%u %6d", j, pDisc->SUB.lp1stLBAListOnSubSync[i][j]);
				}
				else if (j == 0) {
					OutputDiscLogA(",              ");
				}
			}
			OutputDiscLogA("\n");
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
	OutputDiscLogA(STR_DOUBLE_HYPHEN_B "Offset ");
	if (bGetDriveOffset) {
		OutputDiscLogA("(Drive offset referes to http://www.accuraterip.com)");
	}
	OutputDiscLogA(STR_DOUBLE_HYPHEN_E);

	if (pExtArg->byAdd && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
		pDisc->MAIN.nCombinedOffset += (INT)(pExtArg->nAudioCDOffsetNum * 4);
		pExtArg->nAudioCDOffsetNum = 0; // If it is possible, I want to repair it by a better method...
		OutputDiscLogA(
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
		OutputDiscLogA(
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
	OutputDiscLogA("\tOverread sector: %d\n", pDisc->MAIN.nAdjustSectorNum);
	if (nSubChannelOffset != 0xff) {
		OutputDiscLogA("\tSubChannel Offset: %d\n", nSubChannelOffset);
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
	OutputLogA(type, OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(C2 error)
		"       +0 +1 +2 +3 +4 +5 +6 +7\n", nLBA, nLBA);

	for (INT i = 0; i < CD_RAW_READ_C2_SIZE; i += 8) {
		OutputLogA(type, "%04X : %02X %02X %02X %02X %02X %02X %02X %02X\n"
			, i, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3]
			, lpBuf[i + 4], lpBuf[i + 5], lpBuf[i + 6], lpBuf[i + 7]);
	}
}

VOID OutputCDMain(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA,
	INT nSize
) {
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(type);
#endif
	OutputLogA(type, OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Main Channel)
		"       +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F\n", nLBA, nLBA);

	for (INT i = 0; i < nSize; i += 16) {
		if (16 > nSize - i) {
			OutputLogA(type, "%04X : ", i);
			for (INT j = 0; j < nSize - i; j++) {
				if (j == 8) {
					OutputLogA(type, " ");
				}
				OutputLogA(type, "%02X ", lpBuf[i + j]);
			}
		}
		else {
			OutputLogA(type,
				"%04X : %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X   "
				, i, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3], lpBuf[i + 4], lpBuf[i + 5]
				, lpBuf[i + 6], lpBuf[i + 7], lpBuf[i + 8], lpBuf[i + 9], lpBuf[i + 10], lpBuf[i + 11]
				, lpBuf[i + 12], lpBuf[i + 13], lpBuf[i + 14], lpBuf[i + 15]);
			for (INT j = 0; j < 16; j++) {
				INT ch = isprint(lpBuf[i + j]) ? lpBuf[i + j] : '.';
				OutputLogA(type, "%c", ch);
			}
		}
		OutputLogA(type, "\n");
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
	OutputLogA(type, OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Sub Channel)
		"\t  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B\n", nLBA, nLBA);

	for (INT i = 0, ch = 0x50; i < CD_RAW_READ_SUBCODE_SIZE; i += 12, ch++) {
		OutputLogA(type,
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
	OutputLogA(type,
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Sub Channel(Raw))
		"       +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F\n", nLBA, nLBA);

	for (INT i = 0; i < CD_RAW_READ_SUBCODE_SIZE; i += 16) {
		OutputLogA(type,
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
	CONST INT BufSize = 256;
	_TCHAR szSub0[BufSize] = {};
	_sntprintf(szSub0, BufSize,
		_T(STR_LBA "P[%02x], Q[%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]{")
		, nLBA, nLBA, pDiscPerSector->subcode.current[0], pDiscPerSector->subcode.current[12]
		, pDiscPerSector->subcode.current[13], pDiscPerSector->subcode.current[14]
		, pDiscPerSector->subcode.current[15], pDiscPerSector->subcode.current[16]
		, pDiscPerSector->subcode.current[17], pDiscPerSector->subcode.current[18]
		, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
		, pDiscPerSector->subcode.current[21], pDiscPerSector->subcode.current[22]
		, pDiscPerSector->subcode.current[23]);

	_TCHAR szSub[BufSize] = {};
	// Ctl
	switch ((pDiscPerSector->subcode.current[12] >> 4) & 0x0f) {
	case 0:
		_sntprintf(szSub, BufSize,
			_T("Audio, 2ch, Copy NG, Pre-emphasis No, "));
		break;
	case AUDIO_WITH_PREEMPHASIS:
		_sntprintf(szSub, BufSize,
			_T("Audio, 2ch, Copy NG, Pre-emphasis Yes, "));
		break;
	case DIGITAL_COPY_PERMITTED:
		_sntprintf(szSub, BufSize,
			_T("Audio, 2ch, Copy OK, Pre-emphasis No, "));
		break;
	case DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		_sntprintf(szSub, BufSize,
			_T("Audio, 2ch, Copy OK, Pre-emphasis Yes, "));
		break;
	case AUDIO_DATA_TRACK:
		_sntprintf(szSub, BufSize,
			_T(" Data,      Copy NG,                  "));
		break;
	case AUDIO_DATA_TRACK | DIGITAL_COPY_PERMITTED:
		_sntprintf(szSub, BufSize,
			_T(" Data,      Copy OK,                  "));
		break;
	case TWO_FOUR_CHANNEL_AUDIO:
		_sntprintf(szSub, BufSize,
			_T("Audio, 4ch, Copy NG, Pre-emphasis No, "));
		break;
	case TWO_FOUR_CHANNEL_AUDIO | AUDIO_WITH_PREEMPHASIS:
		_sntprintf(szSub, BufSize,
			_T("Audio, 4ch, Copy NG, Pre-emphasis Yes, "));
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED:
		_sntprintf(szSub, BufSize,
			_T("Audio, 4ch, Copy OK, Pre-emphasis No, "));
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		_sntprintf(szSub, BufSize,
			_T("Audio, 4ch, Copy OK, Pre-emphasis Yes, "));
		break;
	default:
		_sntprintf(szSub, BufSize,
			_T("Unknown,                              "));
		break;
	}

	// ADR
	_TCHAR szSub2[BufSize] = {};
	switch (pDiscPerSector->subcode.current[12] & 0x0f) {
	case ADR_ENCODES_CURRENT_POSITION:
		if (pDiscPerSector->subcode.current[13] == 0) {
			// lead-in area
			if (pDiscPerSector->subcode.current[14] == 0xa0) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], AMSF[%02x:%02x:%02x], TrackNumOf1stTrack[%02x], ProgramAreaFormat[%02x]"),
					pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
					, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]);
			}
			else if (pDiscPerSector->subcode.current[14] == 0xa1) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], AMSF[%02x:%02x:%02x], TrackNumOfLastTrack[%02x]"),
					pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
					, pDiscPerSector->subcode.current[19]);
			}
			else if (pDiscPerSector->subcode.current[14] == 0xa2) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], AMSF[%02x:%02x:%02x], StartTimeOfLead-out[%02x:%02x:%02x]"),
					pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17],
					pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
					, pDiscPerSector->subcode.current[21]);
			}
			else {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], AMSF[%02x:%02x:%02x], StartTimeOfTrack[%02x:%02x:%02x]"),
					pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17],
					pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
					, pDiscPerSector->subcode.current[21]);
			}
			if (pDiscPerSector->subcode.prev[13] == 0xaa) {
				pDisc->SCSI.nLeadoutLenOf1stSession = nLBA - pDisc->SCSI.n1stLBAofLeadout;
				OutputLogA(standardOut | fileDisc, " Lead-out length of 1st session: %d\n"
					, pDisc->SCSI.nLeadoutLenOf1stSession);
				pDisc->SCSI.n1stLBAofLeadin = nLBA;
			}
		}
		else if (pDiscPerSector->subcode.current[13] == 0xaa) {
			// lead-out area
			_sntprintf(szSub2, BufSize,
				_T("LeadOut  , Idx[%02x], RMSF[%02x:%02x:%02x], AMSF[%02x:%02x:%02x]"),
				pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
				, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17],
				pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
				, pDiscPerSector->subcode.current[21]);
		}
		else {
			_sntprintf(szSub2, BufSize,
				_T("Track[%02x], Idx[%02x], RMSF[%02x:%02x:%02x], AMSF[%02x:%02x:%02x]"),
				pDiscPerSector->subcode.current[13], pDiscPerSector->subcode.current[14]
				, pDiscPerSector->subcode.current[15], pDiscPerSector->subcode.current[16],
				pDiscPerSector->subcode.current[17], pDiscPerSector->subcode.current[19]
				, pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);
			if (pDisc->SCSI.bMultiSession && pDiscPerSector->subcode.current[13] > 1) {
				if (pDiscPerSector->subcode.prev[13] == 0 &&
					(pDiscPerSector->subcode.prev[14] == 0xa0 ||
					pDiscPerSector->subcode.prev[14] == 0xa1 ||
					pDiscPerSector->subcode.prev[14] == 0xa2)
					) {
					pDisc->SCSI.nLeadinLenOf2ndSession = nLBA - pDisc->SCSI.n1stLBAofLeadin;
					OutputLogA(standardOut | fileDisc, " Lead-in length of 2nd session: %d\n"
						, pDisc->SCSI.nLeadinLenOf2ndSession);
					pDisc->SCSI.nEndLBAOfLeadin = nLBA;
				}
				else if (pDisc->SCSI.nPregapLenOf1stTrkOf2ndSession == 0 &&
					BcdToDec(pDiscPerSector->subcode.current[13]) == pDisc->SCSI.by1stMultiSessionTrkNum &&
					pDiscPerSector->subcode.prev[14] == 0 && pDiscPerSector->subcode.current[14] == 1) {
					pDisc->SCSI.nPregapLenOf1stTrkOf2ndSession = nLBA - pDisc->SCSI.nEndLBAOfLeadin;
					OutputLogA(standardOut | fileDisc, " Pregap length of 1st track of 2nd session: %d\n"
						, pDisc->SCSI.nPregapLenOf1stTrkOf2ndSession);
				}
			}
		}
		break;
	case ADR_ENCODES_MEDIA_CATALOG: {
		_TCHAR szCatalog[META_CATALOG_SIZE] = {};
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0,
			pDisc->SUB.szCatalog, sizeof(pDisc->SUB.szCatalog), szCatalog, sizeof(szCatalog));
#else
		strncpy(szCatalog, pDisc->SUB.szCatalog, sizeof(szCatalog) / sizeof(szCatalog[0]));
#endif
		_sntprintf(szSub2, BufSize,
			_T("MediaCatalogNumber [%13s], AMSF[     :%02x]"), szCatalog, pDiscPerSector->subcode.current[21]);
		break;
	}
	case ADR_ENCODES_ISRC: {
		if (pDiscPerSector->byTrackNum == 0 || pDisc->SCSI.toc.LastTrack < pDiscPerSector->byTrackNum) {
			OutputSubErrorWithLBALogA(" Invalid Adr\n", nLBA, pDiscPerSector->byTrackNum);
		}
		else {
			_TCHAR szISRC[META_ISRC_SIZE] = {};
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, 0,
				pDisc->SUB.pszISRC[pDiscPerSector->byTrackNum - 1], META_ISRC_SIZE, szISRC, sizeof(szISRC));
#else
			strncpy(szISRC, pDisc->SUB.pszISRC[pDiscPerSector->byTrackNum - 1], sizeof(szISRC) / sizeof(szISRC[0]));
#endif
			_sntprintf(szSub2, BufSize,
				_T("ItnStdRecordingCode [%12s], AMSF[     :%02x]"), szISRC, pDiscPerSector->subcode.current[21]);
		}
		break;
	}
	case 5:
		if (pDiscPerSector->subcode.current[13] == 0) {
			if (pDiscPerSector->subcode.current[14] == 0xb0) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], StartTimeForTheNextSession[%02x:%02x:%02x], NumberOfDifferentMode-5[%02x], OutermostLead-out[%02x:%02x:%02x]"),
					pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17],
					pDiscPerSector->subcode.current[18], pDiscPerSector->subcode.current[19]
					, pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);
			}
			else if (pDiscPerSector->subcode.current[14] == 0xb1) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], NumberOfSkipIntervalPointers[%02x], NumberOfSkipTrackAssignmentsInPoint[%02x]"),
					pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]);
			}
			else if (pDiscPerSector->subcode.current[14] == 0xb2 ||
				pDiscPerSector->subcode.current[14] == 0xb3 || pDiscPerSector->subcode.current[14] == 0xb4) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], TrackNumberToSkipUponPlayback[%02x %02x %02x %02x %02x %02x %02x]"),
					pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17],
					pDiscPerSector->subcode.current[18], pDiscPerSector->subcode.current[19]
					, pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);
			}
			else if (pDiscPerSector->subcode.current[14] == 0xc0) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], OptimumRecordingPower[%02x], StartTimeOfTheFirstLead-in[%02x:%02x:%02x]"),
					pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
					, pDiscPerSector->subcode.current[21]);
			}
			else if (pDiscPerSector->subcode.current[14] == 0xc1) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], CopyOfInfoFromA1Point[%02x %02x %02x %02x %02x %02x %02x]"),
					pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17],
					pDiscPerSector->subcode.current[18], pDiscPerSector->subcode.current[19]
					, pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);
			}
			else {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], SkipIntervalStopTime[%02x:%02x:%02x], SkipIntervalStartTime[%02x:%02x:%02x]"),
					pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17],
					pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
					, pDiscPerSector->subcode.current[21]);
			}
		}
		else if (pDiscPerSector->subcode.current[13] == 0xaa) {
			// lead-out area
			_sntprintf(szSub2, BufSize,
				_T("LeadOutAdr5, Track[%02u], Idx[%02x], StartTime[%02x:%02x:%02x]"),
				BcdToDec((BYTE)(pDiscPerSector->subcode.current[14] >> 4 & 0x0f)), pDiscPerSector->subcode.current[14] & 0x0f,
				pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);
		}
		break;
	case ADR_ENCODES_CDTV_SPECIFIC:
		_sntprintf(szSub2, BufSize,
			_T("CDTV Specific   [%02x%02x%02x%02x%02x%02x%02x%02x], AMSF[     :%02x]"),
			pDiscPerSector->subcode.current[13], pDiscPerSector->subcode.current[14]
			, pDiscPerSector->subcode.current[15], pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17],
			pDiscPerSector->subcode.current[18], pDiscPerSector->subcode.current[19]
			, pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);
		break;
	case 0x0c: // I forgot what this is...
		if (pDiscPerSector->subcode.current[13] == 0) {
			if (pDiscPerSector->subcode.current[14] == 0xb1) {
				_sntprintf(szSub2, BufSize,
					_T("Point[%02x], 15[%02x], 16[%02x], 17[%02x], 18[%02x], 19[%02x], 20[%02x], 21[%02x]"),
					pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
					, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17],
					pDiscPerSector->subcode.current[18], pDiscPerSector->subcode.current[19]
					, pDiscPerSector->subcode.current[20], pDiscPerSector->subcode.current[21]);
			}
		}
		break;
	default:
		_sntprintf(szSub2, BufSize,
			_T("Adr[%02x], Track[%02x], Idx[%02x], RMSF[%02x:%02x:%02x], AMSF[%02x:%02x:%02x]"),
			pDiscPerSector->subcode.current[12], pDiscPerSector->subcode.current[13]
			, pDiscPerSector->subcode.current[14], pDiscPerSector->subcode.current[15]
			, pDiscPerSector->subcode.current[16], pDiscPerSector->subcode.current[17]
			, pDiscPerSector->subcode.current[19], pDiscPerSector->subcode.current[20]
			, pDiscPerSector->subcode.current[21]);
		break;
	}

	SUB_R_TO_W scRW[4] = {};
	BYTE tmpCode[24] = {};
	_TCHAR szSub3[128] = {};
	_tcsncat(szSub3, _T("}, RtoW["), 8);
	for (INT i = 0; i < 4; i++) {
		for (INT j = 0; j < 24; j++) {
			tmpCode[j] = (BYTE)(*(lpSubcodeRaw + (i * 24 + j)) & 0x3f);
		}
		memcpy(&scRW[i], tmpCode, sizeof(scRW[i]));
		switch (scRW[i].command) {
		case 0: // MODE 0, ITEM 0
			_tcsncat(szSub3, _T("0"), 1);
			break;
		case 8: // MODE 1, ITEM 0
			_tcsncat(szSub3, _T("Line-Graphics"), 13);
			break;
		case 9: // MODE 1, ITEM 1
			_tcsncat(szSub3, _T("TV-Graphics"), 11);
			break;
		case 10: // MODE 1, ITEM 2
			_tcsncat(szSub3, _T("Extended-TV-Graphics"), 20);
			break;
		case 20: // MODE 2, ITEM 4
			_tcsncat(szSub3, _T("CDText"), 6);
			break;
		case 24: // MODE 3, ITEM 0
			_tcsncat(szSub3, _T("Midi"), 4);
			break;
		case 56: // MODE 7, ITEM 0
			_tcsncat(szSub3, _T("User"), 4);
			break;
		default:
			_tcsncat(szSub3, _T("Reserved"), 8);
			break;
		}
		if (i < 3) {
			_tcsncat(szSub3, _T(", "), 2);
		}
		else {
			_tcsncat(szSub3, _T("]\n"), 2);
		}
	}
#ifndef _DEBUG
	_ftprintf(g_LogFile.fpSubReadable, _T("%s%s%s%s"), szSub0, szSub, szSub2, szSub3);
#else
	OutputDebugStringExA("%s%s%s%s", szSub0, szSub, szSub2, szSub3);
#endif
}
