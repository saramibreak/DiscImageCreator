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
#include "struct.h"
#include "output.h"
#include "outputScsiCmdLogforCD.h"
#include "_external/abgx360.h"

VOID OutputFsRecordingDateAndTime(
	LPBYTE lpBuf
) {
	WORD sTime = MAKEWORD(lpBuf[0], lpBuf[1]);
	CHAR cTimeZone = (CHAR)(sTime >> 12 & 0x0f);
	OutputVolDescLogA("\tRecording Date and Time: ");
	if (cTimeZone == 0) {
		OutputVolDescLogA("UTC ");
	}
	else if (cTimeZone == 1) {
		OutputVolDescLogA("LocalTime ");
	}
	else if (cTimeZone == 2) {
		OutputVolDescLogA("OriginalTime ");
	}
	else {
		OutputVolDescLogA("Reserved ");
	}
	SHORT nTime = sTime & 0xfff;
	OutputVolDescLogA(
		"%+03d%02d %u-%02u-%02u %02u:%02u:%02u.%02u.%02u.%02u\n",
		nTime / 60, nTime % 60, MAKEWORD(lpBuf[2], lpBuf[3]), lpBuf[4], lpBuf[5],
		lpBuf[6], lpBuf[7], lpBuf[8], lpBuf[9], lpBuf[10], lpBuf[11]);
}

VOID OutputFsRegid(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\t\t            Flags: %u\n"
		"\t\t       Identifier: %.23s\n"
		"\t\tIdentifier Suffix: ",
		lpBuf[0],
		(LPCH)&lpBuf[1]);
	for (INT i = 24; i < 32; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputFsNSRDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\t                               Structure Data: ");
	for (INT i = 8; i < 2048; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputFsBootDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR(Architecture Type));
	OutputFsRegid(lpBuf + 8);
	OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR(Boot Identifier));
	OutputFsRegid(lpBuf + 8);

	OutputVolDescLogA(
		"\tBoot Extent Location: %u\n"
		"\t  Boot Extent Length: %u\n"
		"\t        Load Address: %u%u\n"
		"\t       Start Address: %u%u\n",
		MAKEUINT(MAKEWORD(lpBuf[72], lpBuf[73]), MAKEWORD(lpBuf[74], lpBuf[75])),
		MAKEUINT(MAKEWORD(lpBuf[76], lpBuf[77]), MAKEWORD(lpBuf[78], lpBuf[79])),
		MAKEUINT(MAKEWORD(lpBuf[80], lpBuf[81]), MAKEWORD(lpBuf[82], lpBuf[83])),
		MAKEUINT(MAKEWORD(lpBuf[84], lpBuf[85]), MAKEWORD(lpBuf[86], lpBuf[87])),
		MAKEUINT(MAKEWORD(lpBuf[88], lpBuf[89]), MAKEWORD(lpBuf[90], lpBuf[91])),
		MAKEUINT(MAKEWORD(lpBuf[92], lpBuf[93]), MAKEWORD(lpBuf[94], lpBuf[95])));

	OutputFsRecordingDateAndTime(lpBuf + 96);
	OutputVolDescLogA(
		"\t               Flags: %u\n"
		"\t            Boot Use: ",
		MAKEWORD(lpBuf[108], lpBuf[109]));
	for (INT i = 142; i < 2048; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputFsTerminatingExtendedAreaDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\t                               Structure Data: ");
	for (INT i = 7; i < 2048; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputFsBeginningExtendedAreaDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\t                               Structure Data: ");
	for (INT i = 7; i < 2048; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputFsVolumeStructureDescriptorFormat(
	LPBYTE lpBuf,
	INT nLBA
) {
	OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Volume Recognition Sequence)
		"\t                               Structure Type: %u\n"
		"\t                          Standard Identifier: %.5s\n"
		"\t                            Structure Version: %u\n"
		, nLBA, nLBA, lpBuf[0], (LPCH)&lpBuf[1], lpBuf[6]);
}

VOID OutputFsVolumeRecognitionSequence(
	LPBYTE lpBuf,
	INT nLBA
) {
	if (lpBuf[0] == 0 && !strncmp((LPCH)&lpBuf[1], "BOOT2", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nLBA);
		OutputFsBootDescriptor(lpBuf);
	}
	else if (lpBuf[0] == 0 && !strncmp((LPCH)&lpBuf[1], "BEA01", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nLBA);
		OutputFsBeginningExtendedAreaDescriptor(lpBuf);
	}
	else if (lpBuf[0] == 0 && !strncmp((LPCH)&lpBuf[1], "NSR02", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nLBA);
		OutputFsNSRDescriptor(lpBuf);
	}
	else if (lpBuf[0] == 0 && !strncmp((LPCH)&lpBuf[1], "NSR03", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nLBA);
		OutputFsNSRDescriptor(lpBuf);
	}
	else if (lpBuf[0] == 0 && !strncmp((LPCH)&lpBuf[1], "TEA01", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nLBA);
		OutputFsTerminatingExtendedAreaDescriptor(lpBuf);
	}
}

VOID OutputFsCharspec(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\t\t       Character Set Type: %u\n"
		"\t\tCharacter Set Information: %.63s\n",
		lpBuf[0], (LPCH)&lpBuf[1]);
}

VOID OutputFsExtentDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\t\t  Extent Length: %u\n"
		"\t\tExtent Location: %u\n",
		MAKEUINT(MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3])),
		MAKEUINT(MAKEWORD(lpBuf[4], lpBuf[5]), MAKEWORD(lpBuf[6], lpBuf[7])));
}

VOID OutputFsLongAllocationDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\t\t     Extent Length: %u\n"
		"\t\t   Extent Location: %02x%02x%02x%02x%02x%02x\n"
		"\t\tImplementation Use: %02x%02x%02x%02x%02x%02x\n"
		, MAKEUINT(MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3]))
		, lpBuf[4], lpBuf[5], lpBuf[6], lpBuf[7], lpBuf[8], lpBuf[9]
		, lpBuf[10], lpBuf[11], lpBuf[12], lpBuf[13], lpBuf[14], lpBuf[15]
		);
}

VOID OutputFsFileSetDescriptor(
	LPBYTE lpBuf
) {
	OutputFsRecordingDateAndTime(lpBuf + 16);
	OutputVolDescLogA(
		"\t                      Interchange Level: %u\n"
		"\t              Maximum Interchange Level: %u\n"
		"\t                     Character Set List: %u\n"
		"\t             Maximum Character Set List: %u\n"
		"\t                        File Set Number: %u\n"
		"\t             File Set Descriptor Number: %u\n"
		"\tLogical Volume Identifier Character Set:\n"
		, MAKEWORD(lpBuf[28], lpBuf[29])
		, MAKEWORD(lpBuf[30], lpBuf[31])
		, MAKEUINT(MAKEWORD(lpBuf[32], lpBuf[33]), MAKEWORD(lpBuf[34], lpBuf[35]))
		, MAKEUINT(MAKEWORD(lpBuf[36], lpBuf[37]), MAKEWORD(lpBuf[38], lpBuf[39]))
		, MAKEUINT(MAKEWORD(lpBuf[40], lpBuf[41]), MAKEWORD(lpBuf[42], lpBuf[43]))
		, MAKEUINT(MAKEWORD(lpBuf[44], lpBuf[45]), MAKEWORD(lpBuf[46], lpBuf[47]))
		);
	OutputFsCharspec(lpBuf + 48);
	OutputVolDescLogA(
		"\t              Logical Volume Identifier: %.128s\n"
		"\t                 File Set Character Set:\n"
		, (LPCH)&lpBuf[112]);
	OutputFsCharspec(lpBuf + 240);
	OutputVolDescLogA(
		"\t                    File Set Identifier: %.32s\n"
		"\t              Copyright File Identifier: %.32s\n"
		"\t               Abstract File Identifier: %.32s\n"
		, (LPCH)&lpBuf[304]
		, (LPCH)&lpBuf[336]
		, (LPCH)&lpBuf[368]);
	OutputVolDescLogA(
		"\t                     Root Directory ICB:\n");
	OutputFsLongAllocationDescriptor(lpBuf + 400);
	OutputVolDescLogA(
		"\t                      Domain Identifier:\n");
	OutputFsRegid(lpBuf + 416);
	OutputVolDescLogA(
		"\t                            Next Extent:\n");
	OutputFsLongAllocationDescriptor(lpBuf + 448);
	OutputVolDescLogA(
		"\t            System Stream Directory ICB:\n");
	OutputFsLongAllocationDescriptor(lpBuf + 464);
}

VOID OutputFsLogicalVolumeIntegrityDescriptor(
	LPBYTE lpBuf
) {
	OutputFsRecordingDateAndTime(lpBuf + 16);
	OutputVolDescLogA(
		"\t                   Integrity Type: %u\n"
		"\tNext Integrity Extent\n"
		, MAKEUINT(MAKEWORD(lpBuf[28], lpBuf[29]), MAKEWORD(lpBuf[30], lpBuf[31])));
	OutputFsExtentDescriptor(lpBuf + 32);

	OutputVolDescLogA("\t      Logical Volume Contents Use: ");
	for (INT i = 40; i < 72; i++) {
		OutputVolDescLogA("%02x", lpBuf[i]);
	}

	UINT N_P =
		MAKEUINT(MAKEWORD(lpBuf[72], lpBuf[73]), MAKEWORD(lpBuf[74], lpBuf[75]));
	UINT L_IU =
		MAKEUINT(MAKEWORD(lpBuf[76], lpBuf[77]), MAKEWORD(lpBuf[78], lpBuf[79]));
	OutputVolDescLogA(
		"\n"
		"\t             Number of Partitions: %u\n"
		"\t     Length of Implementation Use: %u\n"
		, N_P, L_IU);
	UINT nOfs = N_P * 4;
	if (0 < N_P) {
		OutputVolDescLogA("\t                 Free Space Table: ");
		for (UINT i = 0; i < N_P; i += 4) {
			OutputVolDescLogA("%u \n"
				, MAKEUINT(MAKEWORD(lpBuf[80 + i], lpBuf[81 + i]), MAKEWORD(lpBuf[82 + i], lpBuf[83 + i])));
		}
		OutputVolDescLogA("\t                       Size Table: ");
		for (UINT i = 80 + nOfs, j = 0; j < N_P; j += 4) {
			OutputVolDescLogA("%u "
				, MAKEUINT(MAKEWORD(lpBuf[i + j], lpBuf[i + 1 + j]), MAKEWORD(lpBuf[i + 2 + j], lpBuf[i + 3 + j])));
		}
		OutputVolDescLogA("\n");
	}
	if (0 < L_IU) {
		nOfs = 80 + N_P * 8;
		OutputVolDescLogA(
			"\tImplementation Use\n"
			"\t\tImplementation ID\n"
			"\t\t\t flags: %d\n"
			"\t\t\t    id: %.23s\n"
			"\t\t\tsuffix: %.8s\n"
			"\t\t           Number of Files: %d\n"
			"\t\t     Number of Directories: %d\n"
			"\t\t Minimum UDF Read Revision: %d\n"
			"\t\tMinimum UDF Write Revision: %d\n"
			"\t\tMaximum UDF Write Revision: %d\n"
			, lpBuf[nOfs], (LPCH)&lpBuf[nOfs + 1], (LPCH)&lpBuf[nOfs + 25], lpBuf[nOfs + 32]
			, lpBuf[nOfs + 36], lpBuf[nOfs + 40], lpBuf[nOfs + 42], lpBuf[nOfs + 34]
		);
		OutputVolDescLogA("\n");
	}
}

VOID OutputFsTerminatingDescriptor(
	LPBYTE lpBuf
) {
	// all reserved byte
	UNREFERENCED_PARAMETER(lpBuf);
}

VOID OutputFsUnallocatedSpaceDescriptor(
	LPBYTE lpBuf
) {
	UINT N_AD =
		MAKEUINT(MAKEWORD(lpBuf[20], lpBuf[21]), MAKEWORD(lpBuf[22], lpBuf[23]));
	OutputVolDescLogA(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\t Number of Allocation Descriptors: %u\n"
		, MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])),
		N_AD);
	if (0 < N_AD) {
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR(Allocation Descriptors));
		for (UINT i = 0; i < N_AD * 8; i += 8) {
			OutputFsExtentDescriptor(lpBuf + 24 + i);
		}
	}
}

VOID OutputFsLogicalVolumeDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\tDescriptor Character Set\n",
		MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])));
	OutputFsCharspec(lpBuf + 20);

	OutputVolDescLogA(
		"\tLogical Volume Identifier: %.128s\n"
		"\t      Logical Block Size : %u\n"
		"\tDomain Identifier\n",
		(LPCH)&lpBuf[84],
		MAKEUINT(MAKEWORD(lpBuf[212], lpBuf[213]), MAKEWORD(lpBuf[214], lpBuf[215])));
	OutputFsCharspec(lpBuf + 216);

	OutputVolDescLogA("\tLogical Volume Contents Use: ");
	for (INT i = 248; i < 264; i++) {
		OutputVolDescLogA("%02x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");

	UINT MT_L =
		MAKEUINT(MAKEWORD(lpBuf[264], lpBuf[265]), MAKEWORD(lpBuf[266], lpBuf[267]));
	OutputVolDescLogA(
		"\t        Map Table Length: %u\n"
		"\tNumber of Partition Maps: %u\n"
		"\tImplementation Identifier\n",
		MT_L,
		MAKEUINT(MAKEWORD(lpBuf[268], lpBuf[269]), MAKEWORD(lpBuf[270], lpBuf[271])));
	OutputFsRegid(lpBuf + 272);

	OutputVolDescLogA("\tImplementation Use: ");
	for (INT i = 304; i < 432; i++) {
		OutputVolDescLogA("%02x", lpBuf[i]);
	}
	OutputVolDescLogA(
		"\n"
		"\tIntegrity Sequence Extent\n");
	OutputFsExtentDescriptor(lpBuf + 432);

	if (0 < MT_L) {
		OutputVolDescLogA("\tPartition Maps: ");
		for (UINT i = 0; i < MT_L; i++) {
			OutputVolDescLogA("%02x", lpBuf[440 + i]);
		}
		OutputVolDescLogA("\n");
	}
}

VOID OutputFsPartitionDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\t                  Partition Flags: %u\n"
		"\t                 Partition Number: %u\n"
		"\tPartition Contents\n",
		MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])),
		MAKEWORD(lpBuf[20], lpBuf[21]),
		MAKEWORD(lpBuf[22], lpBuf[23]));

	OutputFsRegid(lpBuf + 24);
	OutputVolDescLogA("\tPartition Contents Use: ");
	for (INT i = 56; i < 184; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}

	OutputVolDescLogA(
		"\n"
		"\t                Access Type: %u\n"
		"\tPartition Starting Location: %u\n"
		"\t           Partition Length: %u\n"
		"\tImplementation Identifier\n",
		MAKEUINT(MAKEWORD(lpBuf[184], lpBuf[185]), MAKEWORD(lpBuf[186], lpBuf[187])),
		MAKEUINT(MAKEWORD(lpBuf[188], lpBuf[189]), MAKEWORD(lpBuf[190], lpBuf[191])),
		MAKEUINT(MAKEWORD(lpBuf[192], lpBuf[193]), MAKEWORD(lpBuf[194], lpBuf[195])));

	OutputFsRegid(lpBuf + 196);
	OutputVolDescLogA("\tImplementation Use: ");
	for (INT i = 228; i < 356; i++) {
		OutputVolDescLogA("%02x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputFsImplementationUseVolumeDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\tImplementation Identifier\n",
		MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])));
	OutputFsRegid(lpBuf + 20);

	INT nOfs = 52;
	OutputVolDescLogA("\tLVI Charset\n");
	OutputFsCharspec(lpBuf + nOfs);
	OutputVolDescLogA(
		"\tLogical Volume Identifier: %.128s\n"
		"\t                LV Info 1: %.36s\n"
		"\t                LV Info 2: %.36s\n"
		"\t                LV Info 3: %.36s\n"
		"\tImplemention ID\n"
		, (LPCH)&lpBuf[nOfs + 64]
		, (LPCH)&lpBuf[nOfs + 192]
		, (LPCH)&lpBuf[nOfs + 228]
		, (LPCH)&lpBuf[nOfs + 264]);

	OutputFsRegid(lpBuf + 352);
	OutputVolDescLogA("\tImplementation Use: ");
	for (INT i = nOfs + 332; i < nOfs + 460; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputFsVolumeDescriptorPointer(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\tVolume Descriptor Sequence Number: %u\n"
		OUTPUT_DHYPHEN_PLUS_STR(Next Volume Descriptor Sequence Extent),
		MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])));
	OutputFsExtentDescriptor(lpBuf + 20);
}

VOID OutputFsAnchorVolumeDescriptorPointer(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR(Main Volume Descriptor Sequence Extent));
	OutputFsExtentDescriptor(lpBuf + 16);
	OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR(Reserve Volume Descriptor Sequence Extent));
	OutputFsExtentDescriptor(lpBuf + 24);
}

VOID OutputFsPrimaryVolumeDescriptorForUDF(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\t Primary Volume Descriptor Number: %u\n"
		"\t                Volume Identifier: %.32s\n"
		"\t           Volume Sequence Number: %u\n"
		"\t   Maximum Volume Sequence Number: %u\n"
		"\t                Interchange Level: %u\n"
		"\t        Maximum Interchange Level: %u\n"
		"\t               Character Set List: %u\n"
		"\t       Maximum Character Set List: %u\n"
		"\t            Volume Set Identifier: %.128s\n"
		"\tDescriptor Character Set\n",
		MAKEUINT(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])),
		MAKEUINT(MAKEWORD(lpBuf[20], lpBuf[21]), MAKEWORD(lpBuf[22], lpBuf[23])),
		(LPCH)&lpBuf[24],
		MAKEWORD(lpBuf[56], lpBuf[57]),
		MAKEWORD(lpBuf[58], lpBuf[59]),
		MAKEWORD(lpBuf[60], lpBuf[61]),
		MAKEWORD(lpBuf[62], lpBuf[63]),
		MAKEUINT(MAKEWORD(lpBuf[64], lpBuf[65]), MAKEWORD(lpBuf[66], lpBuf[67])),
		MAKEUINT(MAKEWORD(lpBuf[68], lpBuf[69]), MAKEWORD(lpBuf[70], lpBuf[71])),
		(LPCH)&lpBuf[72]);

	OutputFsCharspec(lpBuf + 200);

	OutputVolDescLogA("\tExplanatory Character Set\n");
	OutputFsCharspec(lpBuf + 264);

	OutputVolDescLogA("\tVolume Abstract\n");
	OutputFsExtentDescriptor(lpBuf + 328);

	OutputVolDescLogA("\tVolume Copyright Notice\n");
	OutputFsExtentDescriptor(lpBuf + 336);

	OutputVolDescLogA("\tApplication Identifier\n");
	OutputFsRegid(lpBuf + 344);

	OutputFsRecordingDateAndTime(lpBuf + 376);

	OutputVolDescLogA("\tImplementation Identifier\n");
	OutputFsRegid(lpBuf + 388);

	OutputVolDescLogA("\tImplementation Use: ");
	for (INT i = 420; i < 484; i++) {
		OutputVolDescLogA("%02x", lpBuf[i]);
	}
	OutputVolDescLogA(
		"\n"
		"\tPredecessor Volume Descriptor Sequence Location: %u\n",
		MAKEUINT(MAKEWORD(lpBuf[484], lpBuf[485]), MAKEWORD(lpBuf[486], lpBuf[487])));
	OutputVolDescLogA(
		"\t                                          Flags: %u\n",
		MAKEWORD(lpBuf[488], lpBuf[489]));
}

VOID OutputFsDescriptorTag(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\t               Descriptor Version: %u\n"
		"\t                     Tag Checksum: %u\n"
		"\t                Tag Serial Number: %u\n"
		"\t                   Descriptor CRC: %x\n"
		"\t            Descriptor CRC Length: %u\n"
		"\t                     Tag Location: %u\n",
		MAKEWORD(lpBuf[2], lpBuf[3]),
		lpBuf[4],
		MAKEWORD(lpBuf[6], lpBuf[7]),
		MAKEWORD(lpBuf[8], lpBuf[9]),
		MAKEWORD(lpBuf[10], lpBuf[11]),
		MAKEUINT(MAKEWORD(lpBuf[12], lpBuf[13]), MAKEWORD(lpBuf[14], lpBuf[15])));
}

VOID OutputFsVolumeDescriptorSequence(
	LPBYTE lpBuf,
	INT nLBA
) {
	WORD wTagId = MAKEWORD(lpBuf[0], lpBuf[1]);
	if (wTagId == 0 || (10 <= wTagId && wTagId <= 255) || 267 <= wTagId) {
		return;
	}
	switch (wTagId) {
	case 1:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Primary Volume Descriptor), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsPrimaryVolumeDescriptorForUDF(lpBuf);
		break;
	case 2:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Anchor Volume Descriptor Pointer), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsAnchorVolumeDescriptorPointer(lpBuf);
		break;
	case 3:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Volume Descriptor Pointer), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsVolumeDescriptorPointer(lpBuf);
		break;
	case 4:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Implementation Use Volume Descriptor), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsImplementationUseVolumeDescriptor(lpBuf);
		break;
	case 5:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Partition Descriptor), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsPartitionDescriptor(lpBuf);
		break;
	case 6:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Logical Volume Descriptor), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsLogicalVolumeDescriptor(lpBuf);
		break;
	case 7:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Unallocated Space Descriptor), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsUnallocatedSpaceDescriptor(lpBuf);
		break;
	case 8:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Terminating Descriptor), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsTerminatingDescriptor(lpBuf);
		break;
	case 9:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Logical Volume Integrity Descriptor), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsLogicalVolumeIntegrityDescriptor(lpBuf);
		break;
	case 256:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(File Set Descriptor), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		OutputFsFileSetDescriptor(lpBuf);
		break;
	case 257:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(File Identifier Descriptor), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		break;
	case 258:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Allocation Extent Descriptor), nLBA, nLBA);
		OutputFsDescriptorTag(lpBuf);
		break;
	case 259:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Indirect Entry), nLBA, nLBA);
		break;
	case 260:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Terminal Entry), nLBA, nLBA);
		break;
	case 261:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(File Entry), nLBA, nLBA);
		break;
	case 262:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Extended Attribute Header Descriptor), nLBA, nLBA);
		break;
	case 263:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Unallocated Space Entry), nLBA, nLBA);
		break;
	case 264:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Space Bitmap Descriptor), nLBA, nLBA);
		break;
	case 265:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Partition Integrity Entry), nLBA, nLBA);
		break;
	case 266:
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Extended File Entry), nLBA, nLBA);
		break;
	default:
		break;
	}
	return;
}

VOID OutputDVDRamLayerDescriptor(
	PDISC pDisc,
	LPBYTE lpBuf
) {
	if ((lpBuf[0] & 0x0f) == 1) { // ECMA-272
		OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(PhysicalFormatInformation)
			"\t                             Disk Category: %s\n"
			"\t                            Version Number: %d\n"
			"\t                                 Disk size: %s\n"
			"\t                     Maximum transfer rate: %s\n"
			"\t                Number of recording layers: %s\n"
			"\t                  Type of recording layers: %s\n"
			"\t                Average Channel bit length: %s\n"
			"\t                       Average track pitch: %s\n"
			"\tFirst Recorded Data Field of the Data Zone: %7u (%#x)\n"
			"\t Last Recorded Data Field of the Data Zone: %7u (%#x)\n"
			"\t                  Disk type identification: %s\n"
			"\t                                  Velocity: %s\n"
			"\t                    Read power at Velocity: %s\n"
			"\t                on land tracks at Velocity\n"
			"\t                                Peak power: %s\n"
			"\t                              Bias power 1: %s\n"
			"\t                 First pulse starting time: direction is the %s to the laser spot scanning, %s\n"
			"\t                   First pulse ending time: %s\n"
			"\t                   Multiple-pulse duration: %s\n"
			"\t                  Last pulse starting time: direction is the %s to the laser spot scanning, %s\n"
			"\t                    Last pulse ending time: %s\n"
			"\t                              Bias power 2: %s\n"
			"\t              on groove tracks at Velocity\n"
			"\t                                Peak power: %s\n"
			"\t                              Bias power 1: %s\n"
			"\t                 First pulse starting time: direction is the %s to the laser spot scanning, %s\n"
			"\t                   First pulse ending time: %s\n"
			"\t                   Multiple-pulse duration: %s\n"
			"\t                  Last pulse starting time: direction is the %s to the laser spot scanning, %s\n"
			"\t                    Last pulse ending time: %s\n"
			"\t                              Bias power 2: %s\n"
			, (lpBuf[0] & 0x10) == 0x10 ? "rewritable disk" : "other"
			, lpBuf[0] & 0x0f
			, (lpBuf[1] & 0xf0) == 0 ? "120mm" : "80mm"
			, (lpBuf[1] & 0x0f) == 0x02 ? "10,08 Mbits/s." : "other"
			, (lpBuf[2] & 0x60) == 0 ? "single layer" : "other"
			, (lpBuf[2] & 0x0f) == 0x04 ? "rewritable recording layer" : "other"
			, (lpBuf[3] & 0xf0) == 0x20 ? "0,205 um to 0,218 um" : "other"
			, (lpBuf[3] & 0x0f) == 0 ? "0,74 um" : "other"
			, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], 0))
			, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], 0))
			, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], 0))
			, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], 0))
			, lpBuf[32] == 0 ? "disk shall not be recorded without a case" : "disk may be recorded with or without a case"
			, lpBuf[48] == 0x3c ? "6,0 m/s" : "other"
			, lpBuf[49] == 0x0a ? "1,0 mW" : "other"
			, lpBuf[50] == 0x6e ? "11,0 mW" : "other"
			, lpBuf[51] == 0x32 ? "5,0 mW" : "other"
			, (lpBuf[52] & 0x80) == 0x80 ? "opposite" : "same"
			, ((lpBuf[52] & 0x3f) == 0x11) ? "TSFP of 17 ns" : "other"
			, lpBuf[53] == 0x33 ? "TEFP of 51 ns" : "other"
			, lpBuf[54] == 0x11 ? "TMP of 17 ns" : "other"
			, (lpBuf[55] & 0x80) == 0x80 ? "opposite" : "same"
			, ((lpBuf[55] & 0x3f) == 0) ? "TSLP of 0 ns" : "other"
			, lpBuf[56] == 0x22 ? "TELP of 34 ns" : "other"
			, lpBuf[57] == 0x44 ? "TLE of 68 ns" : "other"
			, lpBuf[58] == 0x6e ? "11,0 mW" : "other"
			, lpBuf[59] == 0x32 ? "5,0 mW" : "other"
			, (lpBuf[60] & 0x80) == 0x80 ? "opposite" : "same"
			, ((lpBuf[60] & 0x3f) == 0x11) ? "TSFP of 17 ns" : "other"
			, lpBuf[61] == 0x33 ? "TEFP of 51 ns" : "other"
			, lpBuf[62] == 0x11 ? "TMP of 17 ns" : "other"
			, (lpBuf[63] & 0x80) == 0x80 ? "opposite" : "same"
			, ((lpBuf[63] & 0x3f) == 0) ? "TSLP of 0 ns" : "other"
			, lpBuf[64] == 0x22 ? "TELP of 34 ns" : "other"
			, lpBuf[65] == 0x44 ? "TLE of 68 ns" : "other"
		);
	}
	else if ((lpBuf[0] & 0x0f) == 6) { // ECMA-330
		OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(PhysicalFormatInformation)
			"\t                                      Disk Category: %s\n"
			"\t                                     Version Number: %d\n"
			"\t                                          Disk size: %s\n"
			"\t                              Maximum transfer rate: %s\n"
			"\t                         Number of recording layers: %s\n"
			"\t                           Type of recording layers: %s\n"
			"\t                         Average Channel bit length: %s\n"
			"\t                                Average track pitch: %s\n"
			"\t         First Recorded Data Field of the Data Zone: %7u (%#x)\n"
			"\t          Last Recorded Data Field of the Data Zone: %7u (%#x)\n"
			"\t                                            BCAFlag: %s\n"
			"\t                           Disk type identification: %s\n"
			"\t                                           Velocity: %s\n"
			"\t                             Read power at Velocity: %s\n"
			"\t                  Adaptive write pulse control mode: case %s\n"
			"\t                         Peak power for land tracks: %02x\n"
			"\t                       Bias power 1 for land tracks: %02x\n"
			"\t                       Bias power 2 for land tracks: %02x\n"
			"\t                       Bias power 3 for land tracks: %02x\n"
			"\t                       Peak power for groove tracks: %02x\n"
			"\t                     Bias power 1 for groove tracks: %02x\n"
			"\t                     Bias power 2 for groove tracks: %02x\n"
			"\t                     Bias power 3 for groove tracks: %02x\n"
			"\t                               First pulse end time: %02x\n"
			"\t                               First pulse duration: %02x\n"
			"\t                            Multiple pulse duration: %02x\n"
			"\t                              Last pulse start time: %02x\n"
			"\t                                Last pulse duration: %02x\n"
			"\t Bias power 2 duration on land tracks at Velocity 1: %02x\n"
			, (lpBuf[0] & 0x10) == 0x10 ? "rewritable disk" : "other"
			, lpBuf[0] & 0x0f
			, (lpBuf[1] & 0xf0) == 0 ? "120mm" : "80mm"
			, (lpBuf[1] & 0x0f) == 0x0f ? "not specified" : "other"
			, (lpBuf[2] & 0x60) == 0 ? "single layer" : "other"
			, (lpBuf[2] & 0x0f) == 0x04 ? "rewritable recording layer" : "other"
			, (lpBuf[3] & 0xf0) == 0x40 ? "0,140 um to 0,148 um" : "other"
			, (lpBuf[3] & 0x0f) == 0x02 ? "0,615 um" : "other"
			, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], 0))
			, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], 0))
			, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], 0))
			, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], 0))
			, (lpBuf[16] & 0x80) == 0x80 ? "Exist" : "No"
			, lpBuf[32] == 0 ? "disk shall not be recorded without a case" : "disk may be recorded with or without a case"
			, lpBuf[500] == 0x52 ? "8,2 m/s" : "other"
			, lpBuf[501] == 0x0a ? "1,0 mW" : "other"
			, (lpBuf[502] & 0x80) == 0x80 ? "2" : "1"
			, lpBuf[503], lpBuf[504], lpBuf[505], lpBuf[506]
			, lpBuf[507], lpBuf[508], lpBuf[509], lpBuf[510]
			, lpBuf[511], lpBuf[512], lpBuf[513], lpBuf[514]
			, lpBuf[515], lpBuf[516]
		);
		OutputDiscLogA(
			"\t      First pulse start time\n"
			"\t   Mark 3T, Leading Space 3T: direction is the %s to the laser spot scanning, %02x\n"
			"\t   Mark 4T, Leading Space 3T: direction is the %s to the laser spot scanning, %02x\n"
			"\t   Mark 5T, Leading Space 3T: direction is the %s to the laser spot scanning, %02x\n"
			"\t  Mark >5T, Leading Space 3T: direction is the %s to the laser spot scanning, %02x\n"
			"\t   Mark 3T, Leading Space 4T: direction is the %s to the laser spot scanning, %02x\n"
			"\t   Mark 4T, Leading Space 4T: direction is the %s to the laser spot scanning, %02x\n"
			"\t   Mark 5T, Leading Space 4T: direction is the %s to the laser spot scanning, %02x\n"
			"\t  Mark >5T, Leading Space 4T: direction is the %s to the laser spot scanning, %02x\n"
			"\t   Mark 3T, Leading Space 5T: direction is the %s to the laser spot scanning, %02x\n"
			"\t   Mark 4T, Leading Space 5T: direction is the %s to the laser spot scanning, %02x\n"
			"\t   Mark 5T, Leading Space 5T: direction is the %s to the laser spot scanning, %02x\n"
			"\t  Mark >5T, Leading Space 5T: direction is the %s to the laser spot scanning, %02x\n"
			"\t  Mark 3T, Leading Space >5T: direction is the %s to the laser spot scanning, %02x\n"
			"\t  Mark 4T, Leading Space >5T: direction is the %s to the laser spot scanning, %02x\n"
			"\t  Mark 5T, Leading Space >5T: direction is the %s to the laser spot scanning, %02x\n"
			"\t Mark >5T, Leading Space >5T: direction is the %s to the laser spot scanning, %02x\n"
			"\t       Last pulse start time\n"
			"\t  Mark 3T, Trailing Space 3T: %02x\n"
			"\t  Mark 4T, Trailing Space 3T: %02x\n"
			"\t  Mark 5T, Trailing Space 3T: %02x\n"
			"\t Mark >5T, Trailing Space 3T: %02x\n"
			"\t  Mark 3T, Trailing Space 4T: %02x\n"
			"\t  Mark 4T, Trailing Space 4T: %02x\n"
			"\t  Mark 5T, Trailing Space 4T: %02x\n"
			"\t Mark >5T, Trailing Space 4T: %02x\n"
			"\t  Mark 3T, Trailing Space 5T: %02x\n"
			"\t  Mark 4T, Trailing Space 5T: %02x\n"
			"\t  Mark 5T, Trailing Space 5T: %02x\n"
			"\t Mark >5T, Trailing Space 5T: %02x\n"
			"\t Mark 3T, Trailing Space >5T: %02x\n"
			"\t Mark 4T, Trailing Space >5T: %02x\n"
			"\t Mark 5T, Trailing Space >5T: %02x\n"
			"\tMark >5T, Trailing Space >5T: %02x\n"
			, (lpBuf[517] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[517] & 0x3f
			, (lpBuf[518] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[518] & 0x3f
			, (lpBuf[519] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[519] & 0x3f
			, (lpBuf[520] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[520] & 0x3f
			, (lpBuf[521] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[521] & 0x3f
			, (lpBuf[522] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[522] & 0x3f
			, (lpBuf[523] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[523] & 0x3f
			, (lpBuf[524] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[524] & 0x3f
			, (lpBuf[525] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[525] & 0x3f
			, (lpBuf[526] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[526] & 0x3f
			, (lpBuf[527] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[527] & 0x3f
			, (lpBuf[528] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[528] & 0x3f
			, (lpBuf[529] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[529] & 0x3f
			, (lpBuf[530] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[530] & 0x3f
			, (lpBuf[531] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[531] & 0x3f
			, (lpBuf[532] & 0x80) == 0x80 ? "opposite" : "same"
			, lpBuf[532] & 0x3f
			, lpBuf[533], lpBuf[534], lpBuf[535], lpBuf[536], lpBuf[537], lpBuf[538]
			, lpBuf[539], lpBuf[540], lpBuf[541], lpBuf[542], lpBuf[543], lpBuf[544]
			, lpBuf[545], lpBuf[546], lpBuf[547], lpBuf[548]
		);
		OutputDiscLogA(
			"\t                                                          Disk manufacturer's name: %.48s\n"
			"\t                                     Disk manufacturer's supplementary information: %.16s\n"
			"\t                                                    Write power control parameters\n"
			"\t                                                                        Identifier: %02x%02x\n"
			"\t       Ratio of Peak power for land tracks to threshold peak power for land tracks: %02x\n"
			"\t                                                            T     Target asymmetry: %s, %02x\n"
			"\t                                                              Temporary Peak power: %02x\n"
			"\t                                                            Temporary Bias power 1: %02x\n"
			"\t                                                            Temporary Bias power 2: %02x\n"
			"\t                                                            Temporary Bias power 3: %02x\n"
			"\t   Ratio of Peak power for groove tracks to threshold peak power for groove tracks: %02x\n"
			"\t    Ratio of Peak power for land tracks to threshold 6T peak power for land tracks: %02x\n"
			"\tRatio of Peak power for groove tracks to threshold 6T peak power for groove tracks: %02x\n"
			, (LPCH)&lpBuf[549], (LPCH)&lpBuf[597]
			, lpBuf[613], lpBuf[614]
			, lpBuf[615]
			, (lpBuf[616] & 0x80) == 0x80 ? "in case of minus sign" : "in case of 0 or plus sign"
			, lpBuf[616] & 0x3f
			, lpBuf[617]
			, lpBuf[618], lpBuf[619], lpBuf[620], lpBuf[621], lpBuf[622], lpBuf[623]
		);
		pDisc->DVD.version = (UCHAR)(lpBuf[0] & 0x0f);
		pDisc->DVD.ucBca = (UCHAR)((lpBuf[16] >> 7) & 0x01);
	}
}

VOID OutputDVDLayerDescriptor(
	PDISC pDisc,
	PDVD_FULL_LAYER_DESCRIPTOR dvdLayer,
	LPDWORD lpdwSectorLength,
	UCHAR layerNumber
) {
	// Nintendo optical discs output "Reserved5"
	LPCSTR lpBookType[] = {
		"DVD-ROM", "DVD-RAM", "DVD-R", "DVD-RW",
		"HD DVD-ROM", "HD DVD-RAM", "HD DVD-R", "Reserved1",
		"Reserved2", "DVD+RW", "DVD+R", "Reserved3",
		"Reserved4", "DVD+RW DL", "DVD+R DL", "Reserved5"
	};

	LPCSTR lpMaximumRate[] = {
		"2.52 Mbps", "5.04 Mbps", "10.08 Mbps", "20.16 Mbps",
		"30.24 Mbps", "Reserved", "Reserved", "Reserved",
		"Reserved", "Reserved", "Reserved", "Reserved",
		"Reserved", "Reserved", "Reserved", "Not Specified"
	};

	LPCSTR lpLayerType[] = {
		"Unknown", "Layer contains embossed data", "Layer contains recordable data", "Unknown",
		"Layer contains rewritable data", "Unknown", "Unknown", "Unknown",
		"Reserved", "Unknown", "Unknown", "Unknown",
		"Unknown", "Unknown", "Unknown", "Unknown"
	};

	LPCSTR lpTrackDensity[] = {
		"0.74um/track", "0.80um/track", "0.615um/track", "0.40um/track",
		"0.34um/track", "Reserved", "Reserved", "Reserved",
		"Reserved", "Reserved", "Reserved", "Reserved",
		"Reserved", "Reserved", "Reserved", "Reserved"
	};

	LPCSTR lpLinearDensity[] = {
		"0.267um/bit", "0.293um/bit", "0.409 to 0.435um/bit", "Reserved",
		"0.280 to 0.291um/bit", "0.153um/bit", "0.130 to 0.140um/bit", "Reserved",
		"0.353um/bit", "Reserved", "Reserved", "Reserved",
		"Reserved", "Reserved", "Reserved", "Reserved"
	};
#ifdef _WIN32
	DWORD dwStartingDataSector = dvdLayer->commonHeader.StartingDataSector;
	DWORD dwEndDataSector = dvdLayer->commonHeader.EndDataSector;
	DWORD dwEndLayerZeroSector = dvdLayer->commonHeader.EndLayerZeroSector;
	REVERSE_LONG(&dwStartingDataSector);
	REVERSE_LONG(&dwEndDataSector);
	REVERSE_LONG(&dwEndLayerZeroSector);
#else
	LPBYTE buf = (LPBYTE)dvdLayer;
	DWORD dwStartingDataSector = MAKEUINT(MAKEWORD(buf[7], buf[6]), MAKEWORD(buf[5], 0));
	DWORD dwEndDataSector = MAKEUINT(MAKEWORD(buf[11], buf[10]), MAKEWORD(buf[9], 0));
	DWORD dwEndLayerZeroSector = MAKEUINT(MAKEWORD(buf[15], buf[14]), MAKEWORD(buf[13], 0));
#endif
	if (pDisc->DVD.dwDVDStartPsn == 0) {
		pDisc->DVD.dwDVDStartPsn = dwStartingDataSector;
	}
	else {
		pDisc->DVD.dwXboxStartPsn = dwStartingDataSector;
	}

	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(PhysicalFormatInformation)
		"\t       BookVersion: %u\n"
		"\t          BookType: %s\n"
		"\t       MinimumRate: %s\n"
		"\t          DiskSize: %s\n"
		"\t         LayerType: %s\n"
		"\t         TrackPath: %s\n"
		"\t    NumberOfLayers: %s\n"
		"\t      TrackDensity: %s\n"
		"\t     LinearDensity: %s\n"
		"\tStartingDataSector: %7lu (%#lx)\n"
		"\t     EndDataSector: %7lu (%#lx)\n"
		"\tEndLayerZeroSector: %7lu (%#lx)\n"
		"\t           BCAFlag: %s\n"
		"\t     MediaSpecific: \n",
		dvdLayer->commonHeader.BookVersion,
		lpBookType[dvdLayer->commonHeader.BookType],
		lpMaximumRate[dvdLayer->commonHeader.MinimumRate],
		dvdLayer->commonHeader.DiskSize == 0 ? "120mm" : "80mm",
		lpLayerType[dvdLayer->commonHeader.LayerType],
		dvdLayer->commonHeader.TrackPath == 0 ? "Parallel Track Path" : "Opposite Track Path",
		dvdLayer->commonHeader.NumberOfLayers == 0 ? "Single Layer" : "Double Layer",
		lpTrackDensity[dvdLayer->commonHeader.TrackDensity],
		lpLinearDensity[dvdLayer->commonHeader.LinearDensity],
		dwStartingDataSector, dwStartingDataSector,
		dwEndDataSector, dwEndDataSector,
		dwEndLayerZeroSector, dwEndLayerZeroSector,
		dvdLayer->commonHeader.BCAFlag == 0 ? "No" : "Exist");
	pDisc->DVD.ucBca = dvdLayer->commonHeader.BCAFlag;

	OutputCDMain(fileDisc, dvdLayer->MediaSpecific, 0, sizeof(dvdLayer->MediaSpecific));

	if (dvdLayer->commonHeader.TrackPath) {
		DWORD dwEndLayerZeroSectorLen = dwEndLayerZeroSector - dwStartingDataSector + 1;
		DWORD dwEndLayerOneSectorLen = dwEndLayerZeroSector - (~dwEndDataSector & 0xffffff) + 1;
		*lpdwSectorLength = dwEndLayerZeroSectorLen + dwEndLayerOneSectorLen;
		OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(SectorLength)
			"\t LayerZeroSector: %7lu (%#lx)\n"
			"\t+ LayerOneSector: %7lu (%#lx)\n"
			"\t------------------------------------\n"
			"\t  LayerAllSector: %7lu (%#lx)\n"
			, dwEndLayerZeroSectorLen, dwEndLayerZeroSectorLen
			, dwEndLayerOneSectorLen, dwEndLayerOneSectorLen
			, *lpdwSectorLength, *lpdwSectorLength);

		if (pDisc->DVD.dwLayer0SectorLength == 0) {
			pDisc->DVD.dwLayer0SectorLength = dwEndLayerZeroSectorLen;
		}
		if (pDisc->DVD.dwLayer1SectorLength == 0) {
			pDisc->DVD.dwLayer1SectorLength = dwEndLayerOneSectorLen;
		}
	}
	else {
		if (layerNumber == 0) {
			*lpdwSectorLength = dwEndDataSector - dwStartingDataSector + 1;
			OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(SectorLength)
				"\tLayerZeroSector: %7lu (%#lx)\n", *lpdwSectorLength, *lpdwSectorLength);
		}
		else if (layerNumber == 1) {
			*lpdwSectorLength = dwEndDataSector - dwStartingDataSector + 1;
			OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(SectorLength)
				"\tLayerOneSector: %7lu (%#lx)\n", *lpdwSectorLength, *lpdwSectorLength);
		}
	}
}

VOID OutputDVDRegion(
	UCHAR ucRMI,
	UCHAR ucFlag,
	CONST CHAR* region
) {
	if ((ucRMI & ucFlag) == 0) {
		OutputDiscLogA("%s", region);
	}
}

VOID OutputDVDCopyrightDescriptor(
	PDVD_COPYRIGHT_DESCRIPTOR dvdCopyright,
	PPROTECT_TYPE_DVD pProtect
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(CopyrightInformation)
		"\t    CopyrightProtectionType: ");
	switch (dvdCopyright->CopyrightProtectionType) {
	case 0:
		OutputDiscLogA("No\n");
		*pProtect = noProtect;
		break;
	case 1:
		OutputDiscLogA("CSS/CPPM\n");
		*pProtect = css;
		break;
	case 2:
		OutputDiscLogA("CPRM\n");
		*pProtect = cprm;
		break;
	case 3:
		OutputDiscLogA("AACS with HD DVD content\n");
		*pProtect = aacs;
		break;
	case 10:
		OutputDiscLogA("AACS with BD content\n");
		*pProtect = aacs;
		break;
	default:
		// Nintendo optical discs output 0xfd
		OutputDiscLogA("Unknown (%#02x)\n", dvdCopyright->CopyrightProtectionType);
		*pProtect = noProtect;
		break;
	}
	OutputDiscLogA("\tRegionManagementInformation:");
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x01, " 1");
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x02, " 2");
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x04, " 3");
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x08, " 4");
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x10, " 5");
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x20, " 6");
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x40, " 7");
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x80, " 8");
	OutputDiscLogA("\n");
}

VOID OutputDVDCommonInfo(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	for (WORD k = 0; k < wFormatLength; k++) {
		OutputDiscLogA("%02x", lpFormat[k]);
	}
	OutputDiscLogA("\n");

}

VOID OutputDVDDiskKeyDescriptor(
	PDVD_DISK_KEY_DESCRIPTOR dvdDiskKey
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DiskKeyData));
	OutputCDMain(fileDisc, dvdDiskKey->DiskKeyData, 0, sizeof(dvdDiskKey->DiskKeyData));
}

VOID OutputDiscBCADescriptor(
	PDISC pDisc,
	PDVD_BCA_DESCRIPTOR dvdBca,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(BCAInformation));
	if (pDisc->DVD.protect == cprm) {
		OutputDiscLogA(
			"\t   BCA Record ID: %d\n"
			"\t  Version Number: %d\n"
			"\t     Data Length: %d\n"
			"\tMedia Identifier:"
			, MAKEWORD(dvdBca->BCAInformation[1], dvdBca->BCAInformation[0])
			, dvdBca->BCAInformation[2], dvdBca->BCAInformation[3]
		);
		for (BYTE i = 0; i < dvdBca->BCAInformation[3]; i++) {
			OutputDiscLogA(" %02x", dvdBca->BCAInformation[4 + i]);
		}
		OutputDiscLogA("\n");
		if (pDisc->SCSI.wCurrentMedia == ProfileDvdRecordable ||
			pDisc->SCSI.wCurrentMedia == ProfileDvdRewritable) {
			OutputDiscLogA(
				"\t      BCA Record ID: %d\n"
				"\t     Version Number: %d\n"
				"\t        Data Length: %d\n"
				"\tMKB Validation Data:"
				, MAKEWORD(dvdBca->BCAInformation[13], dvdBca->BCAInformation[12])
				, dvdBca->BCAInformation[14], dvdBca->BCAInformation[15]
			);
			for (BYTE i = 0; i < dvdBca->BCAInformation[15]; i++) {
				OutputDiscLogA(" %02x", dvdBca->BCAInformation[16 + i]);
			}
			OutputDiscLogA("\n");
		}
	}
	else {
		OutputCDMain(fileDisc, dvdBca->BCAInformation, 0, wFormatLength);
	}
}

VOID OutputDVDManufacturerDescriptor(
	PDVD_MANUFACTURER_DESCRIPTOR dvdManufacturer,
	PDISC_TYPE pDiscType
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(ManufacturingInformation));
	OutputCDMain(fileDisc, dvdManufacturer->ManufacturingInformation, 0,
		sizeof(dvdManufacturer->ManufacturingInformation));
	if (!strncmp((LPCH)&dvdManufacturer->ManufacturingInformation[16], "Nintendo Game Disk", 18)) {
		*pDiscType = DISC_TYPE::gamecube;
	}
	else if (!strncmp((LPCH)&dvdManufacturer->ManufacturingInformation[16], "Nintendo NNGC Disk", 18)) {
		*pDiscType = DISC_TYPE::wii;
	}
	else {
		*pDiscType = DISC_TYPE::formal;
	}

}

VOID OutputDVDMediaId(
	LPBYTE lpFormat
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(MediaID)
		"\t           Media Identifier:");
	for (INT i = 0; i < 8; i++) {
		OutputDiscLogA(" %02x", lpFormat[i]);
	}
	OutputDiscLogA(
		"\n\tMessage Authentication Code:");
	for (INT i = 8; i < 18; i++) {
		OutputDiscLogA(" %02x", lpFormat[i]);
	}
	OutputDiscLogA(" -> it seems it's always random\n");
}

VOID OutputDVDMediaKeyBlock(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(MediaKeyBlock)
		"\tMedia Key Block Total Packs: %u\n"
		, *(lpFormat - 1));
	OutputDiscLogA(
		"\tMessage Authentication Code:");
	for (INT i = 0; i < 10; i++) {
		OutputDiscLogA(" %02x", lpFormat[i]);
	}
	OutputDiscLogA(
		" -> it seems it's always random\n"
		"\tMedia Key Block\n"
	);
	OutputCDMain(fileDisc, lpFormat + 16, 0, (INT)(wFormatLength  - 16));
}

VOID OutputDiscDefinitionStructure(
	UCHAR version,
	LPBYTE lpFormat
) {
	if (version == 1) {

	}
	else if (version == 6) {
		WORD zonesNum = MAKEWORD(lpFormat[11], lpFormat[10]);
		OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DiscDefinitionStructure)
			"\t                        DDS Identifier: %02x%02x\n"
			"\t                    Disk Certification\n"
			"\t                            Formatting: %s\n"
			"\t                           The disk has %s certified by a user\n"
			"\t                           The disk has %s certified by a manufacturer\n"
			"\t                  DDS/PDL Update Count: %d\n"
			"\t                      Number of Groups: %d\n"
			"\t                       Number of Zones: %d\n"
			"\tFirst sector in the Primary spare area: %7u (%#x)\n"
			"\t Last sector in the Primary spare area: %7u (%#x)\n"
			"\t                  First logical sector: %7u (%#x)\n"
			, lpFormat[0], lpFormat[1]
			, (lpFormat[3] & 0x80) == 0x80 ? "in process" : "has been completed"
			, (lpFormat[3] & 0x10) == 0x10 ? "been" : "not been"
			, (lpFormat[3] & 0x01) == 0x01 ? "been" : "not been"
			, MAKEUINT(MAKEWORD(lpFormat[7], lpFormat[6]), MAKEWORD(lpFormat[5], lpFormat[4]))
			, MAKEWORD(lpFormat[9], lpFormat[8])
			, zonesNum
			, MAKEUINT(MAKEWORD(lpFormat[83], lpFormat[82]), MAKEWORD(lpFormat[81], lpFormat[80]))
			, MAKEUINT(MAKEWORD(lpFormat[83], lpFormat[82]), MAKEWORD(lpFormat[81], lpFormat[80]))
			, MAKEUINT(MAKEWORD(lpFormat[87], lpFormat[86]), MAKEWORD(lpFormat[85], lpFormat[84]))
			, MAKEUINT(MAKEWORD(lpFormat[87], lpFormat[86]), MAKEWORD(lpFormat[85], lpFormat[84]))
			, MAKEUINT(MAKEWORD(lpFormat[91], lpFormat[90]), MAKEWORD(lpFormat[89], lpFormat[88]))
			, MAKEUINT(MAKEWORD(lpFormat[91], lpFormat[90]), MAKEWORD(lpFormat[89], lpFormat[88]))
		);
		for (INT i = 256, j = 0; j < (INT)zonesNum; i += 4, j++) {
			OutputDiscLogA("\t                Start LSN for the Zone: %7u (%#x)\n"
				, MAKEUINT(MAKEWORD(lpFormat[i + 3], lpFormat[i + 2]), MAKEWORD(lpFormat[i + 1], lpFormat[i]))
				, MAKEUINT(MAKEWORD(lpFormat[i + 3], lpFormat[i + 2]), MAKEWORD(lpFormat[i + 1], lpFormat[i]))
			);
		}
	}
	else {
		OutputCDMain(fileDisc, lpFormat, 0, 2048);
	}
}

VOID OutputDvdRamMediumStatus(
	PDVD_RAM_MEDIUM_STATUS dvdRamMeium
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DvdRamMediumStatus)
		"\t              PersistentWriteProtect: %s\n"
		"\t               CartridgeWriteProtect: %s\n"
		"\t           MediaSpecificWriteInhibit: %s\n"
		"\t                  CartridgeNotSealed: %s\n"
		"\t                    MediaInCartridge: %s\n"
		"\t              DiscTypeIdentification: %x\n"
		"\tMediaSpecificWriteInhibitInformation: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(dvdRamMeium->PersistentWriteProtect),
		BOOLEAN_TO_STRING_YES_NO_A(dvdRamMeium->CartridgeWriteProtect),
		BOOLEAN_TO_STRING_YES_NO_A(dvdRamMeium->MediaSpecificWriteInhibit),
		BOOLEAN_TO_STRING_YES_NO_A(dvdRamMeium->CartridgeNotSealed),
		BOOLEAN_TO_STRING_YES_NO_A(dvdRamMeium->MediaInCartridge),
		dvdRamMeium->DiscTypeIdentification,
		BOOLEAN_TO_STRING_YES_NO_A(dvdRamMeium->MediaSpecificWriteInhibitInformation));
}

VOID OutputDiscSpareAreaInformation(
	PDVD_RAM_SPARE_AREA_INFORMATION dvdRamSpare
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDRamSpareAreaInformation)
		"\t          FreePrimarySpareSectors: %u\n"
		"\t     FreeSupplementalSpareSectors: %u\n"
		"\tAllocatedSupplementalSpareSectors: %u\n",
		MAKEUINT(
		MAKEWORD(dvdRamSpare->FreePrimarySpareSectors[3], dvdRamSpare->FreePrimarySpareSectors[2]),
		MAKEWORD(dvdRamSpare->FreePrimarySpareSectors[1], dvdRamSpare->FreePrimarySpareSectors[0])),
		MAKEUINT(
		MAKEWORD(dvdRamSpare->FreeSupplementalSpareSectors[3], dvdRamSpare->FreeSupplementalSpareSectors[2]),
		MAKEWORD(dvdRamSpare->FreeSupplementalSpareSectors[1], dvdRamSpare->FreeSupplementalSpareSectors[0])),
		MAKEUINT(
		MAKEWORD(dvdRamSpare->AllocatedSupplementalSpareSectors[3], dvdRamSpare->AllocatedSupplementalSpareSectors[2]),
		MAKEWORD(dvdRamSpare->AllocatedSupplementalSpareSectors[1], dvdRamSpare->AllocatedSupplementalSpareSectors[0])));
}

VOID OutputDVDRamRecordingType(
	PDVD_RAM_RECORDING_TYPE dvdRamRecording
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDRamRecordingType)
		"\tRealTimeData: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(dvdRamRecording->RealTimeData));
}

VOID OutputDVDRmdLastBorderOut(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(RMD in last border-out));
	INT nRoop = wFormatLength / DISC_RAW_READ_SIZE;
	for (INT i = 0; i < nRoop; i++) {
		OutputCDMain(fileDisc, lpFormat + DISC_RAW_READ_SIZE * i, 0, DISC_RAW_READ_SIZE);
	}
}

VOID OutputDVDRecordingManagementAreaData(
	PDVD_RECORDING_MANAGEMENT_AREA_DATA dvdRecordingMan,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDRecordingManagementAreaData)
		"\tLastRecordedRMASectorNumber: %u\n"
		"\t                   RMDBytes: \n"
		, MAKEUINT(MAKEWORD(dvdRecordingMan->LastRecordedRMASectorNumber[3]
		, dvdRecordingMan->LastRecordedRMASectorNumber[2])
		, MAKEWORD(dvdRecordingMan->LastRecordedRMASectorNumber[1]
		, dvdRecordingMan->LastRecordedRMASectorNumber[0]))
	);
	OutputCDMain(fileDisc, dvdRecordingMan->RMDBytes, 0
		, (INT)(wFormatLength - sizeof(DVD_RECORDING_MANAGEMENT_AREA_DATA)));
}

VOID OutputDVDPreRecordedInformation(
	PDVD_PRERECORDED_INFORMATION dvdPreRecorded
) {
	UINT addr = MAKEUINT(MAKEWORD(dvdPreRecorded->LastAddressOfDataRecordableArea[0]
		, dvdPreRecorded->LastAddressOfDataRecordableArea[1])
		, MAKEWORD(dvdPreRecorded->LastAddressOfDataRecordableArea[2], 0));

	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDPreRecordedInformation)
		"\t                      FieldID_1: %02x\n"
		"\t            DiscApplicatiowCode: %02x\n"
		"\t               DiscPhysicalCode: %02x\n"
		"\tLastAddressOfDataRecordableArea: %u (%#x)\n"
		"\t                  ExtensiowCode: %02x\n"
		"\t                    PartVers1on: %02x\n"
		"\t                      FieldID_2: %02x\n"
		"\t               OpcSuggestedCode: %02x\n"
		"\t                 WavelengthCode: %02x\n"
		"\t              WriteStrategyCode: %02x%02x%02x%02x\n"
		"\t                      FieldID_3: %02x\n"
		"\t               ManufacturerId_3: %.6s\n"
		"\t                      FieldID_4: %02x\n"
		"\t               ManufacturerId_4: %.6s\n"
		"\t                      FieldID_5: %02x\n"
		"\t               ManufacturerId_5: %.6s\n"
		, dvdPreRecorded->FieldID_1
		, dvdPreRecorded->DiscApplicationCode
		, dvdPreRecorded->DiscPhysicalCode
		, addr, addr
		, dvdPreRecorded->ExtensionCode
		, dvdPreRecorded->PartVers1on
		, dvdPreRecorded->FieldID_2
		, dvdPreRecorded->OpcSuggestedCode
		, dvdPreRecorded->WavelengthCode
		, dvdPreRecorded->WriteStrategyCode[0], dvdPreRecorded->WriteStrategyCode[1]
		, dvdPreRecorded->WriteStrategyCode[2], dvdPreRecorded->WriteStrategyCode[3]
		, dvdPreRecorded->FieldID_3
		, (LPCH)&dvdPreRecorded->ManufacturerId_3[0]
		, dvdPreRecorded->FieldID_4
		, (LPCH)&dvdPreRecorded->ManufacturerId_4[0]
		, dvdPreRecorded->FieldID_5
		, (LPCH)&dvdPreRecorded->ManufacturerId_5[0]
	);
}

VOID OutputDVDUniqueDiscIdentifer(
	PDVD_UNIQUE_DISC_IDENTIFIER dvdUnique
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDUniqueDiscIdentifer)
		"\t RandomNumber: %u\n"
		"\tDate and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s\n"
		, MAKEWORD(dvdUnique->RandomNumber[1], dvdUnique->RandomNumber[0])
		, dvdUnique->Year, dvdUnique->Month, dvdUnique->Day
		, dvdUnique->Hour, dvdUnique->Minute, dvdUnique->Second);
}

VOID OutputDVDAdipInformation(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(ADIP information)"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDVDDualLayerRecordingInformation(
	PDVD_DUAL_LAYER_RECORDING_INFORMATION dvdDualLayer
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDDualLayerRecordingInformation)
		"\tLayer0SectorsImmutable: %s\n"
		"\t         Layer0Sectors: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(dvdDualLayer->Layer0SectorsImmutable),
		MAKEUINT(MAKEWORD(dvdDualLayer->Layer0Sectors[3], dvdDualLayer->Layer0Sectors[2]),
		MAKEWORD(dvdDualLayer->Layer0Sectors[1], dvdDualLayer->Layer0Sectors[0])));
}

VOID OutputDVDDualLayerMiddleZone(
	PDVD_DUAL_LAYER_MIDDLE_ZONE_START_ADDRESS dvdDualLayerMiddle
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDDualLayerMiddleZoneStartAddress)
		"\t                   InitStatus: %s\n"
		"\tShiftedMiddleAreaStartAddress: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(dvdDualLayerMiddle->InitStatus),
		MAKEUINT(MAKEWORD(dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[3],
		dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[2]),
		MAKEWORD(dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[1],
		dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[0])));
}

VOID OutputDVDDualLayerJumpInterval(
	PDVD_DUAL_LAYER_JUMP_INTERVAL_SIZE dvdDualLayerJump
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDDualLayerJumpIntervalSize)
		"\tJumpIntervalSize: %u\n",
		MAKEUINT(MAKEWORD(dvdDualLayerJump->JumpIntervalSize[3],
		dvdDualLayerJump->JumpIntervalSize[2]),
		MAKEWORD(dvdDualLayerJump->JumpIntervalSize[1],
		dvdDualLayerJump->JumpIntervalSize[0])));
}

VOID OutputDVDDualLayerManualLayerJump(
	PDVD_DUAL_LAYER_MANUAL_LAYER_JUMP dvdDualLayerMan
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDDualLayerManualLayerJump)
		"\tManualJumpLayerAddress: %u\n",
		MAKEUINT(MAKEWORD(dvdDualLayerMan->ManualJumpLayerAddress[3],
		dvdDualLayerMan->ManualJumpLayerAddress[2]),
		MAKEWORD(dvdDualLayerMan->ManualJumpLayerAddress[1],
		dvdDualLayerMan->ManualJumpLayerAddress[0])));
}

VOID OutputDVDDualLayerRemapping(
	PDVD_DUAL_LAYER_REMAPPING_INFORMATION dvdDualLayerRemapping
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDDualLayerRemappingInformation)
		"\tManualJumpLayerAddress: %u\n",
		MAKEUINT(MAKEWORD(dvdDualLayerRemapping->RemappingAddress[3],
		dvdDualLayerRemapping->RemappingAddress[2]),
		MAKEWORD(dvdDualLayerRemapping->RemappingAddress[1],
		dvdDualLayerRemapping->RemappingAddress[0])));
}

VOID OutputDVDDiscControlBlockHeader(
	PDVD_DISC_CONTROL_BLOCK_HEADER dvdDiscCtrlBlk
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDDiscControlBlockHeader)
		"\tContentDescriptor: %u\n"
		"\t           AsByte: %u\n"
		"\t         VendorId: ",
		MAKEUINT(MAKEWORD(dvdDiscCtrlBlk->ContentDescriptor[3],
		dvdDiscCtrlBlk->ContentDescriptor[2]),
		MAKEWORD(dvdDiscCtrlBlk->ContentDescriptor[1],
		dvdDiscCtrlBlk->ContentDescriptor[0])),
		MAKEUINT(MAKEWORD(dvdDiscCtrlBlk->ProhibitedActions.AsByte[3],
		dvdDiscCtrlBlk->ProhibitedActions.AsByte[2]),
		MAKEWORD(dvdDiscCtrlBlk->ProhibitedActions.AsByte[1],
		dvdDiscCtrlBlk->ProhibitedActions.AsByte[0])));
	for (WORD k = 0; k < sizeof(dvdDiscCtrlBlk->VendorId); k++) {
		OutputDiscLogA("%c", dvdDiscCtrlBlk->VendorId[k]);
	}
	OutputDiscLogA("\n");
}

VOID OutputDVDDiscControlBlockWriteInhibit(
	PDVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT dvdDiscCtrlBlkWrite
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDDiscControlBlockWriteInhibit)
		"\t      UpdateCount: %u\n"
		"\t           AsByte: %u\n"
		"\t   UpdatePassword: ",
		MAKEUINT(MAKEWORD(dvdDiscCtrlBlkWrite->UpdateCount[3], dvdDiscCtrlBlkWrite->UpdateCount[2]),
		MAKEWORD(dvdDiscCtrlBlkWrite->UpdateCount[1], dvdDiscCtrlBlkWrite->UpdateCount[0])),
		MAKEUINT(MAKEWORD(dvdDiscCtrlBlkWrite->WriteProtectActions.AsByte[3],
		dvdDiscCtrlBlkWrite->WriteProtectActions.AsByte[2]),
		MAKEWORD(dvdDiscCtrlBlkWrite->WriteProtectActions.AsByte[1],
		dvdDiscCtrlBlkWrite->WriteProtectActions.AsByte[0])));
	for (WORD k = 0; k < sizeof(dvdDiscCtrlBlkWrite->UpdatePassword); k++) {
		OutputDiscLogA("%c", dvdDiscCtrlBlkWrite->UpdatePassword[k]);
	}
	OutputDiscLogA("\n");
}

VOID OutputDVDDiscControlBlockSession(
	PDVD_DISC_CONTROL_BLOCK_SESSION dvdDiscCtrlBlkSession
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDDiscControlBlockSession)
		"\tSessionNumber: %u\n"
		"\t       DiscID: \n",
		MAKEWORD(dvdDiscCtrlBlkSession->SessionNumber[1], dvdDiscCtrlBlkSession->SessionNumber[0]));
	for (WORD k = 0; k < sizeof(dvdDiscCtrlBlkSession->DiscID); k++) {
		OutputDiscLogA("%c", dvdDiscCtrlBlkSession->DiscID[k]);
	}
	OutputDiscLogA("\n");

	for (UINT j = 0; j < sizeof(dvdDiscCtrlBlkSession->SessionItem) / sizeof(DVD_DISC_CONTROL_BLOCK_SESSION_ITEM); j++) {
		OutputDiscLogA(
			"\t  SessionItem: %u\n"
			"\t\t     AsByte: ", j);
		for (WORD k = 0; k < sizeof(dvdDiscCtrlBlkSession->SessionItem[j].AsByte); k++) {
			OutputDiscLogA("%c", dvdDiscCtrlBlkSession->SessionItem[j].AsByte[k]);
		}
		OutputDiscLogA("\n");
	}
}

VOID OutputDVDDiscControlBlockList(
	PDVD_DISC_CONTROL_BLOCK_LIST dvdDiscCtrlBlkList,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDDiscControlBlockListT)
		"\tReadabldDCBs: %s\n"
		"\tWritableDCBs: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(dvdDiscCtrlBlkList->ReadabldDCBs),
		BOOLEAN_TO_STRING_YES_NO_A(dvdDiscCtrlBlkList->WritableDCBs));
	OutputDiscLogA(
		"\tDVD_DISC_CONTROL_BLOCK_LIST_DCB: ");
	for (WORD k = 0; k < wFormatLength - sizeof(DVD_DISC_CONTROL_BLOCK_LIST); k++) {
		OutputDiscLogA("%u",
			MAKEUINT(MAKEWORD(dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[3], dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[2]),
			MAKEWORD(dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[1], dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[0])));
	}
	OutputDiscLogA("\n");

}

VOID OutputDVDMtaEccBlock(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(MTA ECC Block)"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDiscWriteProtectionStatus(
	PDVD_WRITE_PROTECTION_STATUS dvdWrite
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DiscWriteProtectionStatus)
		"\tSoftwareWriteProtectUntilPowerdown: %s\n"
		"\t       MediaPersistentWriteProtect: %s\n"
		"\t             CartridgeWriteProtect: %s\n"
		"\t         MediaSpecificWriteProtect: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(dvdWrite->SoftwareWriteProtectUntilPowerdown),
		BOOLEAN_TO_STRING_YES_NO_A(dvdWrite->MediaPersistentWriteProtect),
		BOOLEAN_TO_STRING_YES_NO_A(dvdWrite->CartridgeWriteProtect),
		BOOLEAN_TO_STRING_YES_NO_A(dvdWrite->MediaSpecificWriteProtect));
}

VOID OutputDiscAACSVolumeIdentifier(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(AACS Volume Identifiers)"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDiscPreRecordedAACSMediaSerialNumber(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(PreRecorded AACS Media Serial Number)"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDiscAACSMediaIdentifier(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(AACS Media Identifier)"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDiscAACSMediaKeyBlock(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(AACS Media Key Block)"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDiscListOfRecognizedFormatLayers(
	PDVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS_TYPE_CODE dvdListOf
) {
	OutputDiscLogA(
		"\t\tNumberOfRecognizedFormatLayers: %u\n"
		"\t\t             OnlineFormatlayer: %u\n"
		"\t\t            DefaultFormatLayer: %u\n",
		dvdListOf->NumberOfRecognizedFormatLayers,
		dvdListOf->OnlineFormatlayer,
		dvdListOf->DefaultFormatLayer);
}

VOID OutputDVDStructureFormat(
	PDISC pDisc,
	BYTE byFormatCode,
	WORD wFormatLength,
	LPBYTE lpFormat,
	LPDWORD lpdwSectorLength,
	UCHAR layerNumber
) {
	switch (byFormatCode) {
	case DvdPhysicalDescriptor:
	case 0x10:
		if (pDisc->SCSI.wCurrentMedia == ProfileDvdRam || pDisc->SCSI.wCurrentMedia == ProfileHDDVDRam) {
			OutputDVDRamLayerDescriptor(pDisc, lpFormat);
		}
		else {
			OutputDVDLayerDescriptor(pDisc, (PDVD_FULL_LAYER_DESCRIPTOR)lpFormat, lpdwSectorLength, layerNumber);
		}
		break;
	case DvdCopyrightDescriptor:
		OutputDVDCopyrightDescriptor((PDVD_COPYRIGHT_DESCRIPTOR)lpFormat, &(pDisc->DVD.protect));
		break;
	case DvdDiskKeyDescriptor:
		OutputDVDDiskKeyDescriptor((PDVD_DISK_KEY_DESCRIPTOR)lpFormat);
		break;
	case DvdBCADescriptor:
		OutputDiscBCADescriptor(pDisc, (PDVD_BCA_DESCRIPTOR)lpFormat, wFormatLength);
		break;
	case DvdManufacturerDescriptor:
		OutputDVDManufacturerDescriptor((PDVD_MANUFACTURER_DESCRIPTOR)lpFormat, &(pDisc->DVD.disc));
		break;
	case 0x06:
		OutputDVDMediaId(lpFormat);
		break;
	case 0x07:
		OutputDVDMediaKeyBlock(lpFormat, wFormatLength);
		break;
	case 0x08:
		OutputDiscDefinitionStructure(pDisc->DVD.version, lpFormat);
		break;
	case 0x09:
		OutputDvdRamMediumStatus((PDVD_RAM_MEDIUM_STATUS)lpFormat);
		break;
	case 0x0a:
		OutputDiscSpareAreaInformation((PDVD_RAM_SPARE_AREA_INFORMATION)lpFormat);
		break;
	case 0x0b:
		OutputDVDRamRecordingType((PDVD_RAM_RECORDING_TYPE)lpFormat);
		break;
	case 0x0c:
		OutputDVDRmdLastBorderOut(lpFormat, wFormatLength);
		break;
	case 0x0d:
		OutputDVDRecordingManagementAreaData((PDVD_RECORDING_MANAGEMENT_AREA_DATA)lpFormat, wFormatLength);
		break;
	case 0x0e:
		OutputDVDPreRecordedInformation((PDVD_PRERECORDED_INFORMATION)lpFormat);
		break;
	case 0x0f:
		OutputDVDUniqueDiscIdentifer((PDVD_UNIQUE_DISC_IDENTIFIER)lpFormat);
		break;
	case 0x11:
		OutputDVDAdipInformation(lpFormat, wFormatLength);
		break;
		// formats 0x12, 0x15 are is unstructured in public spec
		// formats 0x13, 0x14, 0x16 through 0x18 are not yet defined
		// formats 0x19, 0x1A are HD DVD-R
		// formats 0x1B through 0x1F are not yet defined
	case 0x20:
		OutputDVDDualLayerRecordingInformation((PDVD_DUAL_LAYER_RECORDING_INFORMATION)lpFormat);
		break;
	case 0x21:
		OutputDVDDualLayerMiddleZone((PDVD_DUAL_LAYER_MIDDLE_ZONE_START_ADDRESS)lpFormat);
		break;
	case 0x22:
		OutputDVDDualLayerJumpInterval((PDVD_DUAL_LAYER_JUMP_INTERVAL_SIZE)lpFormat);
		break;
	case 0x23:
		OutputDVDDualLayerManualLayerJump((PDVD_DUAL_LAYER_MANUAL_LAYER_JUMP)lpFormat);
		break;
	case 0x24:
		OutputDVDDualLayerRemapping((PDVD_DUAL_LAYER_REMAPPING_INFORMATION)lpFormat);
		break;
		// formats 0x25 through 0x2F are not yet defined
	case 0x30:
	{
		OutputDVDDiscControlBlockHeader((PDVD_DISC_CONTROL_BLOCK_HEADER)lpFormat);
		WORD len = wFormatLength;
		if (len == sizeof(DVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT)) {
			OutputDVDDiscControlBlockWriteInhibit((PDVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT)lpFormat);
		}
		else if (len == sizeof(DVD_DISC_CONTROL_BLOCK_SESSION)) {
			OutputDVDDiscControlBlockSession((PDVD_DISC_CONTROL_BLOCK_SESSION)lpFormat);
		}
		else if (len == sizeof(DVD_DISC_CONTROL_BLOCK_LIST)) {
			OutputDVDDiscControlBlockList((PDVD_DISC_CONTROL_BLOCK_LIST)lpFormat, wFormatLength);
		}
		break;
	}
	case 0x31:
		OutputDVDMtaEccBlock(lpFormat, wFormatLength);
		break;
		// formats 0x32 through 0xBF are not yet defined
	case 0xc0:
		OutputDiscWriteProtectionStatus((PDVD_WRITE_PROTECTION_STATUS)lpFormat);
		break;
		// formats 0xC1 through 0x7F are not yet defined
	case 0x80:
		OutputDiscAACSVolumeIdentifier(lpFormat, wFormatLength);
		break;
	case 0x81:
		OutputDiscPreRecordedAACSMediaSerialNumber(lpFormat, wFormatLength);
		break;
	case 0x82:
		OutputDiscAACSMediaIdentifier(lpFormat, wFormatLength);
		break;
	case 0x83:
		OutputDiscAACSMediaKeyBlock(lpFormat, wFormatLength);
		break;
		// formats 0x84 through 0x8F are not yet defined
	case 0x90:
		OutputDiscListOfRecognizedFormatLayers((PDVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS_TYPE_CODE)lpFormat);
		break;
		// formats 0x91 through 0xFE are not yet defined
	default:
		OutputDiscLogA("\tUnknown: %02x\n", byFormatCode);
		break;
	}
}

VOID OutputDVDCopyrightManagementInformation(
	PDVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR dvdCopyright,
	INT nLBA
) {
	if ((dvdCopyright->CPR_MAI & 0x80) == 0x80) {
		if ((dvdCopyright->CPR_MAI & 0x40) == 0x40) {
			switch (dvdCopyright->CPR_MAI & 0x0f) {
			case 0:
				OutputDiscWithLBALogA("This sector is scrambled by CSS", nLBA);
				break;
			case 0x01:
				OutputDiscWithLBALogA("This sector is encrypted by CPPM", nLBA);
				break;
			default:
				OutputDiscWithLBALogA("reserved", nLBA);
			}
		}
		else {
			OutputDiscWithLBALogA("CSS or CPPM doesn't exist in this sector", nLBA);
		}

		switch (dvdCopyright->CPR_MAI & 0x30) {
		case 0:
			OutputDiscLogA(", copying is permitted without restriction\n");
			break;
		case 0x10:
			OutputDiscLogA(", reserved\n");
			break;
		case 0x20:
			OutputDiscLogA(", one generation of copies may be made\n");
			break;
		case 0x30:
			OutputDiscLogA(", no copying is permitted\n");
			break;
		default:
			OutputDiscLogA("\n");
		}
	}
	else {
		OutputDiscWithLBALogA("No protected sector\n", nLBA);
	}
}

VOID OutputBDDiscInformation(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DiscInformationFromPIC)
		"\t            DiscInformationIdentifier: %.2s\n"
		"\t                DiscInformationFormat: %02x\n"
		"\t         NumberOfDIUnitsInEachDIBlock: %02x\n"
		"\t                     DiscTypeSpecific: %02x\n"
		"\tDIUnitSequenceNumber/ContinuationFlag: %02x\n"
		"\t       NumberOfBytesInUseInThisDIUnit: %02x\n"
		"\t                   DiscTypeIdentifier: %.3s\n"
		"\t               DiscSize/Class/Version: %02x\n"
		"\t        DIUnitFormatDependentContents: "
		, &lpFormat[0], lpFormat[2], lpFormat[3], lpFormat[4], lpFormat[5]
		, lpFormat[6], &lpFormat[8], lpFormat[11]);
	for (WORD k = 0; k < 52; k++) {
		OutputDiscLogA("%02x", lpFormat[12 + k]);
	}
	OutputDiscLogA("\n\t                               Others: ");
	for (WORD k = 0; k < wFormatLength - 64; k++) {
		OutputDiscLogA("%02x", lpFormat[64 + k]);
	}
	OutputDiscLogA("\n");
}

VOID OutputCartridgeStatus(
	LPBYTE lpFormat
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(CartridgeStatus)
		"\t               CartridgeWriteProtect: %s\n"
		"\t                  CartridgeNotSealed: %s\n"
		"\t                    MediaInCartridge: %s\n"
		, BOOLEAN_TO_STRING_YES_NO_A((lpFormat[0] & 0x04) == 0x04)
		, BOOLEAN_TO_STRING_YES_NO_A((lpFormat[0] & 0x40) == 0x40)
		, BOOLEAN_TO_STRING_YES_NO_A((lpFormat[0] & 0x80) == 0x80)
	);
}

VOID OutputBDRawDefectList(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	UNREFERENCED_PARAMETER(wFormatLength);
	UINT Entries = MAKEUINT(MAKEWORD(lpFormat[15], lpFormat[14]), MAKEWORD(lpFormat[13], lpFormat[12]));
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(RawDefectList)
		"\t       DefectListIdentifier: %.2s\n"
		"\t           DefectListFormat: %02x\n"
		"\t      DefectListUpdateCount: %02x\n"
		"\t  NumberOfDefectListEntries: %02x\n"
		"\tDiscTypeSpecificInformation: "
		, &lpFormat[0], lpFormat[2], MAKEUINT(MAKEWORD(lpFormat[7], lpFormat[6])
			, MAKEWORD(lpFormat[5], lpFormat[4])), Entries);
	for (WORD k = 0; k < 48; k++) {
		OutputDiscLogA("%02x ", lpFormat[16 + k]);
	}
	OutputDiscLogA("\nDefectEntries: ");
	for (UINT k = 0; k < Entries; k += 8) {
		OutputDiscLogA("%02x%02x%02x%02x%02x%02x%02x%02x "
			, lpFormat[64 + k], lpFormat[65 + k], lpFormat[66 + k], lpFormat[67 + k]
			, lpFormat[68 + k], lpFormat[69 + k], lpFormat[70 + k], lpFormat[71 + k]);
	}
	OutputDiscLogA("\n");
}

VOID OutputBDPhysicalAddressControl(
	LPBYTE lpFormat,
	WORD wFormatLength,
	INT nPacCnt
) {
	if (nPacCnt == 0) {
		OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(PacHeaderList));
		for (WORD i = 0; i < (wFormatLength - 2) / 384; i++) {
			OutputDiscLogA(
				"\t                       PAC ID: %.3s\n"
				"\t            PAC format number: %02x\n"
				"\t             PAC Update Count: %d\n"
				"\t            Unknown PAC Rules: %02x %02x %02x %02x\n"
				"\tUnknown PAC Entire Disc Flags: %d\n"
				"\t           Number of Segments: %d\n"
				, (LPCH)&lpFormat[0]
				, lpFormat[3]
				, MAKEUINT(MAKEWORD(lpFormat[7], lpFormat[6]), MAKEWORD(lpFormat[5], lpFormat[4]))
				, lpFormat[8], lpFormat[9], lpFormat[10], lpFormat[11]
				, lpFormat[12]
				, lpFormat[15]
			);
			for (BYTE j = 0; j < lpFormat[15]; j++) {
				OutputDiscLogA("\t Segment %d: %02x %02x %02x %02x %02x %02x %02x %02x\n"
					, j
					, lpFormat[16 + j * 8], lpFormat[17 + j * 8], lpFormat[18 + j * 8], lpFormat[19 + j * 8]
					, lpFormat[20 + j * 8], lpFormat[21 + j * 8], lpFormat[22 + j * 8], lpFormat[23 + j * 8]
				);

			}
		}
	}
	else if (nPacCnt == 1) {
		OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(PacHeaderList));
		for (WORD i = 0; i < (wFormatLength - 2) / 4; i++) {
			OutputDiscLogA(
				"\t                       PAC ID: %.3s\n"
				"\t            PAC format number: %02x\n"
				, (LPCH)&lpFormat[0]
				, lpFormat[3]
			);
		}
	}
	else {
		OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(PacData));
		OutputCDMain(fileDisc, lpFormat, 0, (INT)wFormatLength);
	}
}

VOID OutputBDStructureFormat(
	PDISC pDisc,
	BYTE byFormatCode,
	WORD wFormatLength,
	LPBYTE lpFormat,
	INT nPacCnt
) {
	OutputDiscLogA("Disc Structure Data Length: %d\n", wFormatLength);
	switch (byFormatCode) {
	case 0:
		OutputBDDiscInformation(lpFormat, wFormatLength);
		break;
		// format 0x01, 0x02 is reserved
	case DvdBCADescriptor:
		OutputDiscBCADescriptor(pDisc, (PDVD_BCA_DESCRIPTOR)lpFormat, wFormatLength);
		break;
		// format 0x04 - 0x07 is reserved
	case 0x08:
		OutputDiscDefinitionStructure(0, lpFormat);
		break;
	case 0x09:
		OutputCartridgeStatus(lpFormat);
		break;
	case 0x0a:
		OutputDiscSpareAreaInformation((PDVD_RAM_SPARE_AREA_INFORMATION)lpFormat);
		break;
		// format 0x0b - 0x11 is reserved
	case 0x12:
		OutputBDRawDefectList(lpFormat, wFormatLength);
		break;
		// format 0x13 - 0x2f is reserved
	case 0x30:
		OutputBDPhysicalAddressControl(lpFormat, wFormatLength, nPacCnt);
		break;
		// formats 0x31 through 0x7F are not yet defined
	case 0x80:
		OutputDiscAACSVolumeIdentifier(lpFormat, wFormatLength);
		break;
	case 0x81:
		OutputDiscPreRecordedAACSMediaSerialNumber(lpFormat, wFormatLength);
		break;
	case 0x82:
		OutputDiscAACSMediaIdentifier(lpFormat, wFormatLength);
		break;
	case 0x83:
		OutputDiscAACSMediaKeyBlock(lpFormat, wFormatLength);
		break;
	case 0x84:
		break;
	case 0x85:
		break;
	case 0x86:
		break;
		// formats 0x87 through 0x8F are not yet defined
	case 0x90:
		OutputDiscListOfRecognizedFormatLayers((PDVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS_TYPE_CODE)lpFormat);
		break;
		// formats 0x91 through 0xbf are not yet defined
	case 0xc0:
		OutputDiscWriteProtectionStatus((PDVD_WRITE_PROTECTION_STATUS)lpFormat);
		break;
		// formats 0xc1 through 0xfe are not yet defined
	default:
		OutputDiscLogA("\tReserved\n");
		break;
	}
}

// http://xboxdevwiki.net/Xbe#Title_ID
VOID OutputPublisher(
	LPBYTE buf
) {
	if (!strncmp((LPCH)buf, "AC", 2)) {
		OutputDiscLogA("Acclaim Entertainment\n");
	}
	else if (!strncmp((LPCH)buf, "AH", 2)) {
		OutputDiscLogA("ARUSH Entertainment\n");
	}
	else if (!strncmp((LPCH)buf, "AQ", 2)) {
		OutputDiscLogA("Aqua System\n");
	}
	else if (!strncmp((LPCH)buf, "AS", 2)) {
		OutputDiscLogA("ASK\n");
	}
	else if (!strncmp((LPCH)buf, "AT", 2)) {
		OutputDiscLogA("Atlus\n");
	}
	else if (!strncmp((LPCH)buf, "AV", 2)) {
		OutputDiscLogA("Activision\n");
	}
	else if (!strncmp((LPCH)buf, "AY", 2)) {
		OutputDiscLogA("Aspyr Media\n");
	}
	else if (!strncmp((LPCH)buf, "BA", 2)) {
		OutputDiscLogA("Bandai\n");
	}
	else if (!strncmp((LPCH)buf, "BL", 2)) {
		OutputDiscLogA("Black Box\n");
	}
	else if (!strncmp((LPCH)buf, "BM", 2)) {
		OutputDiscLogA("BAM! Entertainment\n");
	}
	else if (!strncmp((LPCH)buf, "BR", 2)) {
		OutputDiscLogA("Broccoli Co.\n");
	}
	else if (!strncmp((LPCH)buf, "BS", 2)) {
		OutputDiscLogA("Bethesda Softworks\n");
	}
	else if (!strncmp((LPCH)buf, "BU", 2)) {
		OutputDiscLogA("Bunkasha Co.\n");
	}
	else if (!strncmp((LPCH)buf, "BV", 2)) {
		OutputDiscLogA("Buena Vista Games\n");
	}
	else if (!strncmp((LPCH)buf, "BW", 2)) {
		OutputDiscLogA("BBC Multimedia\n");
	}
	else if (!strncmp((LPCH)buf, "BZ", 2)) {
		OutputDiscLogA("Blizzard\n");
	}
	else if (!strncmp((LPCH)buf, "CC", 2)) {
		OutputDiscLogA("Capcom\n");
	}
	else if (!strncmp((LPCH)buf, "CK", 2)) {
		OutputDiscLogA("Kemco Corporation\n");
	}
	else if (!strncmp((LPCH)buf, "CM", 2)) {
		OutputDiscLogA("Codemasters\n");
	}
	else if (!strncmp((LPCH)buf, "CV", 2)) {
		OutputDiscLogA("Crave Entertainment\n");
	}
	else if (!strncmp((LPCH)buf, "DC", 2)) {
		OutputDiscLogA("DreamCatcher Interactive\n");
	}
	else if (!strncmp((LPCH)buf, "DX", 2)) {
		OutputDiscLogA("Davilex\n");
	}
	else if (!strncmp((LPCH)buf, "EA", 2)) {
		OutputDiscLogA("Electronic Arts\n");
	}
	else if (!strncmp((LPCH)buf, "EC", 2)) {
		OutputDiscLogA("Encore inc\n");
	}
	else if (!strncmp((LPCH)buf, "EL", 2)) {
		OutputDiscLogA("Enlight Software\n");
	}
	else if (!strncmp((LPCH)buf, "EM", 2)) {
		OutputDiscLogA("Empire Interactive\n");
	}
	else if (!strncmp((LPCH)buf, "ES", 2)) {
		OutputDiscLogA("Eidos Interactive\n");
	}
	else if (!strncmp((LPCH)buf, "FI", 2)) {
		OutputDiscLogA("Fox Interactive\n");
	}
	else if (!strncmp((LPCH)buf, "FS", 2)) {
		OutputDiscLogA("From Software\n");
	}
	else if (!strncmp((LPCH)buf, "GE", 2)) {
		OutputDiscLogA("Genki Co.\n");
	}
	else if (!strncmp((LPCH)buf, "GV", 2)) {
		OutputDiscLogA("Groove Games\n");
	}
	else if (!strncmp((LPCH)buf, "HE", 2)) {
		OutputDiscLogA("Tru Blu (Entertainment division of Home Entertainment Suppliers)\n");
	}
	else if (!strncmp((LPCH)buf, "HP", 2)) {
		OutputDiscLogA("Hip games\n");
	}
	else if (!strncmp((LPCH)buf, "HU", 2)) {
		OutputDiscLogA("Hudson Soft\n");
	}
	else if (!strncmp((LPCH)buf, "HW", 2)) {
		OutputDiscLogA("Highwaystar\n");
	}
	else if (!strncmp((LPCH)buf, "IA", 2)) {
		OutputDiscLogA("Mad Catz Interactive\n");
	}
	else if (!strncmp((LPCH)buf, "IF", 2)) {
		OutputDiscLogA("Idea Factory\n");
	}
	else if (!strncmp((LPCH)buf, "IG", 2)) {
		OutputDiscLogA("Infogrames\n");
	}
	else if (!strncmp((LPCH)buf, "IL", 2)) {
		OutputDiscLogA("Interlex Corporation\n");
	}
	else if (!strncmp((LPCH)buf, "IM", 2)) {
		OutputDiscLogA("Imagine Media\n");
	}
	else if (!strncmp((LPCH)buf, "IO", 2)) {
		OutputDiscLogA("Ignition Entertainment\n");
	}
	else if (!strncmp((LPCH)buf, "IP", 2)) {
		OutputDiscLogA("Interplay Entertainment\n");
	}
	else if (!strncmp((LPCH)buf, "IX", 2)) {
		OutputDiscLogA("InXile Entertainment\n");
	}
	else if (!strncmp((LPCH)buf, "JA", 2)) {
		OutputDiscLogA("Jaleco\n");
	}
	else if (!strncmp((LPCH)buf, "JW", 2)) {
		OutputDiscLogA("JoWooD\n");
	}
	else if (!strncmp((LPCH)buf, "KB", 2)) {
		OutputDiscLogA("Kemco\n");
	}
	else if (!strncmp((LPCH)buf, "KI", 2)) {
		OutputDiscLogA("Kids Station Inc.\n");
	}
	else if (!strncmp((LPCH)buf, "KN", 2)) {
		OutputDiscLogA("Konami\n");
	}
	else if (!strncmp((LPCH)buf, "KO", 2)) {
		OutputDiscLogA("KOEI\n");
	}
	else if (!strncmp((LPCH)buf, "KU", 2)) {
		OutputDiscLogA("Kobi and/or GAE (formerly Global A Entertainment)\n");
	}
	else if (!strncmp((LPCH)buf, "LA", 2)) {
		OutputDiscLogA("LucasArts\n");
	}
	else if (!strncmp((LPCH)buf, "LS", 2)) {
		OutputDiscLogA("Black Bean Games (publishing arm of Leader S.p.A.\n");
	}
	else if (!strncmp((LPCH)buf, "MD", 2)) {
		OutputDiscLogA("Metro3D\n");
	}
	else if (!strncmp((LPCH)buf, "ME", 2)) {
		OutputDiscLogA("Medix\n");
	}
	else if (!strncmp((LPCH)buf, "MI", 2)) {
		OutputDiscLogA("Microids\n");
	}
	else if (!strncmp((LPCH)buf, "MJ", 2)) {
		OutputDiscLogA("Majesco Entertainment\n");
	}
	else if (!strncmp((LPCH)buf, "MM", 2)) {
		OutputDiscLogA("Myelin Media\n");
	}
	else if (!strncmp((LPCH)buf, "MP", 2)) {
		OutputDiscLogA("MediaQuest\n");
	}
	else if (!strncmp((LPCH)buf, "MS", 2)) {
		OutputDiscLogA("Microsoft Game Studios\n");
	}
	else if (!strncmp((LPCH)buf, "MW", 2)) {
		OutputDiscLogA("Midway Games\n");
	}
	else if (!strncmp((LPCH)buf, "MX", 2)) {
		OutputDiscLogA("Empire Interactive\n");
	}
	else if (!strncmp((LPCH)buf, "NK", 2)) {
		OutputDiscLogA("NewKidCo\n");
	}
	else if (!strncmp((LPCH)buf, "NL", 2)) {
		OutputDiscLogA("NovaLogic\n");
	}
	else if (!strncmp((LPCH)buf, "NM", 2)) {
		OutputDiscLogA("Namco\n");
	}
	else if (!strncmp((LPCH)buf, "OX", 2)) {
		OutputDiscLogA("Oxygen Interactive\n");
	}
	else if (!strncmp((LPCH)buf, "PC", 2)) {
		OutputDiscLogA("Playlogic Entertainment\n");
	}
	else if (!strncmp((LPCH)buf, "PL", 2)) {
		OutputDiscLogA("Phantagram Co., Ltd.\n");
	}
	else if (!strncmp((LPCH)buf, "RA", 2)) {
		OutputDiscLogA("Rage\n");
	}
	else if (!strncmp((LPCH)buf, "SA", 2)) {
		OutputDiscLogA("Sammy\n");
	}
	else if (!strncmp((LPCH)buf, "SC", 2)) {
		OutputDiscLogA("SCi Games\n");
	}
	else if (!strncmp((LPCH)buf, "SE", 2)) {
		OutputDiscLogA("SEGA\n");
	}
	else if (!strncmp((LPCH)buf, "SN", 2)) {
		OutputDiscLogA("SNK\n");
	}
	else if (!strncmp((LPCH)buf, "SS", 2)) {
		OutputDiscLogA("Simon & Schuster\n");
	}
	else if (!strncmp((LPCH)buf, "SU", 2)) {
		OutputDiscLogA("Success Corporation\n");
	}
	else if (!strncmp((LPCH)buf, "SW", 2)) {
		OutputDiscLogA("Swing! Deutschland\n");
	}
	else if (!strncmp((LPCH)buf, "TA", 2)) {
		OutputDiscLogA("Takara\n");
	}
	else if (!strncmp((LPCH)buf, "TC", 2)) {
		OutputDiscLogA("Tecmo\n");
	}
	else if (!strncmp((LPCH)buf, "TD", 2)) {
		OutputDiscLogA("The 3DO Company (or just 3DO)\n");
	}
	else if (!strncmp((LPCH)buf, "TK", 2)) {
		OutputDiscLogA("Takuyo\n");
	}
	else if (!strncmp((LPCH)buf, "TM", 2)) {
		OutputDiscLogA("TDK Mediactive\n");
	}
	else if (!strncmp((LPCH)buf, "TQ", 2)) {
		OutputDiscLogA("THQ\n");
	}
	else if (!strncmp((LPCH)buf, "TS", 2)) {
		OutputDiscLogA("Titus Interactive\n");
	}
	else if (!strncmp((LPCH)buf, "TT", 2)) {
		OutputDiscLogA("Take-Two Interactive Software\n");
	}
	else if (!strncmp((LPCH)buf, "US", 2)) {
		OutputDiscLogA("Ubisoft\n");
	}
	else if (!strncmp((LPCH)buf, "VC", 2)) {
		OutputDiscLogA("Victor Interactive Software\n");
	}
	else if (!strncmp((LPCH)buf, "VN", 2)) {
		OutputDiscLogA("Vivendi Universal (just took Interplays publishing rights)\n");
	}
	else if (!strncmp((LPCH)buf, "VU", 2)) {
		OutputDiscLogA("Vivendi Universal Games\n");
	}
	else if (!strncmp((LPCH)buf, "VV", 2)) {
		OutputDiscLogA("Vivendi Universal Games\n");
	}
	else if (!strncmp((LPCH)buf, "WE", 2)) {
		OutputDiscLogA("Wanadoo Edition\n");
	}
	else if (!strncmp((LPCH)buf, "WR", 2)) {
		OutputDiscLogA("Warner Bros. Interactive Entertainment\n");
	}
	else if (!strncmp((LPCH)buf, "XI", 2)) {
		OutputDiscLogA("XPEC Entertainment and Idea Factory\n");
	}
	else if (!strncmp((LPCH)buf, "XK", 2)) {
		OutputDiscLogA("Xbox kiosk disc\n");
	}
	else if (!strncmp((LPCH)buf, "XL", 2)) {
		OutputDiscLogA("Xbox special bundled or live demo disc\n");
	}
	else if (!strncmp((LPCH)buf, "XM", 2)) {
		OutputDiscLogA("Evolved Games\n");
	}
	else if (!strncmp((LPCH)buf, "XP", 2)) {
		OutputDiscLogA("XPEC Entertainment\n");
	}
	else if (!strncmp((LPCH)buf, "XR", 2)) {
		OutputDiscLogA("Panorama\n");
	}
	else if (!strncmp((LPCH)buf, "YB", 2)) {
		OutputDiscLogA("YBM Sisa (South-Korea)\n");
	}
	else if (!strncmp((LPCH)buf, "ZD", 2)) {
		OutputDiscLogA("Zushi Games (formerly Zoo Digital Publishing)\n");
	}
	else {
		OutputDiscLogA("Unknown\n");
	}
}

VOID OutputRegion(
	BYTE buf
) {
	if (buf == 'W') {
		OutputDiscLogA("World\n");
	}
	else if (buf == 'A') {
		OutputDiscLogA("USA\n");
	}
	else if (buf == 'J') {
		OutputDiscLogA("Japan\n");
	}
	else if (buf == 'E') {
		OutputDiscLogA("Europe\n");
	}
	else if (buf == 'K') {
		OutputDiscLogA("USA, Japan\n");
	}
	else if (buf == 'L') {
		OutputDiscLogA("USA, Europe\n");
	}
	else if (buf == 'H') {
		OutputDiscLogA("Japan, Europe\n");
	}
	else {
		OutputDiscLogA("Other\n")
	}
}

VOID OutputManufacturingInfoForXbox(
	LPBYTE buf
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DiscManufacturingInformation)
		"\tSystem Version: %02u\n"
		, buf[0]
	);

	char date[20] = {};
	printwin32filetime(MAKEUINT64(MAKEUINT(MAKEWORD(buf[16], buf[17]), MAKEWORD(buf[18], buf[19]))
		, MAKEUINT(MAKEWORD(buf[20], buf[21]), MAKEWORD(buf[22], buf[23]))), date);
	if (buf[0] == 0x01) {
		OutputDiscLogA(
			"\t     Publisher: "
		);
		OutputPublisher(&buf[8]);
		OutputDiscLogA(
			"\t        Serial: %c%c%c\n"
			"\t       Version: 1.%c%c\n"
			"\t        Region: "
			, buf[10], buf[11], buf[12], buf[13], buf[14]
		);
		OutputRegion(buf[15]);
		OutputDiscLogA(
			"\t     Timestamp: %s\n"
			"\t       Unknown: %02u\n"
			, date, buf[24]);
	}
	else if (buf[0] == 0x02) {
		OutputDiscLogA(
			"\t     Timestamp: %s\n"
			"\t       Unknown: %02u\n"
			"\t      Media ID: "
			, date, buf[24]);
		for (WORD k = 32; k < 48; k++) {
			OutputDiscLogA("%02x", buf[k]);
		}
		OutputDiscLogA(
			"\n"
			"\t     Publisher: "
		);
		OutputPublisher(&buf[64]);

		OutputDiscLogA(
			"\t        Serial: %c%c%c%c\n"
			"\t       Version: 1.%c%c\n"
			"\t        Region: "
			, buf[66], buf[67], buf[68], buf[69], buf[70], buf[71]
		);
		OutputRegion(buf[72]);
		if (buf[73] == '0' && buf[74] == 'X') {
			OutputDiscLogA(
				"\t       Unknown: %c%c\n"
				"\t          Disc: %c of %c\n"
				, buf[73], buf[74], buf[75], buf[76])
		}
		else {
			OutputDiscLogA(
				"\t       Unknown: %c%c%c\n"
				"\t          Disc: %c of %c\n"
				, buf[73], buf[74], buf[75], buf[76], buf[77])
		}
	}
}

// http://xboxdevwiki.net/Xbox_Game_Disc#Security_Sectors_.28SS.bin.29
VOID OutputXboxSecuritySector(
	PDISC pDisc,
	LPBYTE buf
) {
	DWORD dwSectorLen = 0;
	OutputDVDStructureFormat(pDisc, 0x10, DISC_RAW_READ_SIZE, buf, &dwSectorLen, 0);
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(SecuritySector)
		"\t                     CPR_MAI Key: %08x\n"
		"\t      Version of challenge table: %02u\n"
		"\t     Number of challenge entries: %u\n"
		"\t     Encrypted challenge entries: "
		, MAKEUINT(MAKEWORD(buf[723], buf[722]), MAKEWORD(buf[721], buf[720]))
		, buf[768], buf[769]);
	for (WORD k = 770; k < 1024; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}

	char date[20] = {};
	printwin32filetime(MAKEUINT64(MAKEUINT(MAKEWORD(buf[1055], buf[1056]), MAKEWORD(buf[1057], buf[1058]))
		, MAKEUINT(MAKEWORD(buf[1059], buf[1060]), MAKEWORD(buf[1061], buf[1062]))), date);
	OutputDiscLogA(
		"\n"
		"\t            Timestamp of unknown: %s\n"
		"\t                         Unknown: "
		, date
	);
	for (WORD k = 1083; k < 1099; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}

	printwin32filetime(MAKEUINT64(MAKEUINT(MAKEWORD(buf[1183], buf[1184]), MAKEWORD(buf[1185], buf[1186]))
		, MAKEUINT(MAKEWORD(buf[1187], buf[1188]), MAKEWORD(buf[1189], buf[1190]))), date);
	OutputDiscLogA(
		"\n"
		"\t          Timestamp of authoring: %s\n"
		"\t                         Unknown: %02x\n"
		"\t                         Unknown: "
		, date, buf[1210]
	);
	for (WORD k = 1211; k < 1227; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}

	OutputDiscLogA(
		"\n"
		"\t                    SHA-1 hash A: "
	);
	for (WORD k = 1227; k < 1247; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}

	OutputDiscLogA(
		"\n"
		"\t                     Signature A: "
	);
	for (WORD k = 1247; k < 1503; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}

	printwin32filetime(MAKEUINT64(MAKEUINT(MAKEWORD(buf[1503], buf[1504]), MAKEWORD(buf[1505], buf[1506]))
		, MAKEUINT(MAKEWORD(buf[1507], buf[1508]), MAKEWORD(buf[1509], buf[1510]))), date);
	OutputDiscLogA(
		"\n"
		"\t          Timestamp of mastering: %s\n"
		"\t                         Unknown: %02x\n"
		"\t                         Unknown: "
		, date, buf[1530]
	);
	for (WORD k = 1531; k < 1547; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}

	OutputDiscLogA(
		"\n"
		"\t                    SHA-1 hash B: "
	);
	for (WORD k = 1547; k < 1567; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}

	OutputDiscLogA(
		"\n"
		"\t                     Signature B: "
	);
	for (WORD k = 1567; k < 1632; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}

	OutputDiscLogA(
		"\n"
		"\tNumber of security sector ranges: %u\n"
		"\t          security sector ranges: \n"
		, buf[1632]);

	DWORD startLBA = 0, endLBA = 0;
#ifdef _WIN32
	PDVD_FULL_LAYER_DESCRIPTOR dvdLayer = (PDVD_FULL_LAYER_DESCRIPTOR)buf;
	DWORD dwEndLayerZeroSector = dvdLayer->commonHeader.EndLayerZeroSector;
	REVERSE_LONG(&dwEndLayerZeroSector);
#else
	DWORD dwEndLayerZeroSector = MAKEUINT(MAKEWORD(buf[15], buf[14]), MAKEWORD(buf[13], 0));
#endif
	BYTE ssNum = buf[1632];

	for (INT k = 1633, i = 0; i < ssNum; k += 9, i++) {
		DWORD startPsn = MAKEDWORD(MAKEWORD(buf[k + 5], buf[k + 4]), MAKEWORD(buf[k + 3], 0));
		DWORD endPsn = MAKEDWORD(MAKEWORD(buf[k + 8], buf[k + 7]), MAKEWORD(buf[k + 6], 0));
		if (i < 8) {
			OutputDiscLogA("\t\t       Layer 0");
			startLBA = startPsn - 0x30000;
			endLBA = endPsn - 0x30000;
			pDisc->DVD.securitySectorRange[i][0] = startLBA;
			pDisc->DVD.securitySectorRange[i][1] = endLBA;
		}
		else if (i < 16) {
			OutputDiscLogA("\t\t       Layer 1");
			startLBA = dwEndLayerZeroSector * 2 - (~startPsn & 0xffffff) - 0x30000 + 1;
			endLBA = dwEndLayerZeroSector * 2 - (~endPsn & 0xffffff) - 0x30000 + 1;
			pDisc->DVD.securitySectorRange[i][0] = startLBA;
			pDisc->DVD.securitySectorRange[i][1] = endLBA;
		}
		else {
			OutputDiscLogA("\t\tUnknown ranges");
			startLBA = startPsn;
			endLBA = endPsn;
		}
		OutputDiscLogA("\t\tUnknown: %02x%02x%02x, startLBA-endLBA: %8lu-%8lu\n"
			, buf[k], buf[k + 1], buf[k + 2], startLBA, endLBA);
	}
}

// https://abgx360.xecuter.com/dl/abgx360_v1.0.6_source.zip
VOID OutputXbox360SecuritySector(
	PDISC pDisc,
	LPBYTE buf
) {
	DWORD dwSectorLen = 0;
	OutputDVDStructureFormat(pDisc, 0x10, DISC_RAW_READ_SIZE, buf, &dwSectorLen, 0);
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(SecuritySector)
		"\t                         Unknown: "
	);
	for (WORD k = 256; k < 284; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}
	OutputDiscLogA("\n");

	for (INT i = 0; i < 21; i++) {
		OutputDiscLogA(
			"\t             [%02d] Challenge Data: %02x%02x%02x%02x, Response: %02x%02x%02x%02x%02x\n"
			, i + 1, buf[512 + i * 9], buf[512 + i * 9 + 1], buf[512 + i * 9 + 2], buf[512 + i * 9 + 3]
			, buf[512 + i * 9 + 4], buf[512 + i * 9 + 5], buf[512 + i * 9 + 6], buf[512 + i * 9 + 7], buf[512 + i * 9 + 8]
		);
	}

	OutputDiscLogA(
		"\t                     CPR_MAI Key: %08x\n"
		"\t      Version of challenge table: %02u\n"
		"\t     Number of challenge entries: %u\n"
		"\t     Encrypted challenge entries: "
		, MAKEUINT(MAKEWORD(buf[723], buf[722]), MAKEWORD(buf[721], buf[720]))
		, buf[768], buf[769]);
	for (WORD k = 770; k < 1024; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}
	OutputDiscLogA("\n");

	BYTE dcrt[252] = {};
	decryptChallengeResponse(dcrt, buf);
	for (INT i = 0; i < 21; i++) {
		OutputDiscLogA(
			"\t                    Decrypted[%02d] -> Challenge Type: %02x, Challenge ID: %02x"
			", Tolerance: %02x, Type: %02x, Challenge Data: %02x%02x%02x%02x, Response: %02x%02x%02x%02x\n"
			, i + 1, dcrt[i * 12], dcrt[i * 12 + 1], dcrt[i * 12 + 2], dcrt[i * 12 + 3]
			, dcrt[i * 12 + 4], dcrt[i * 12 + 5], dcrt[i * 12 + 6], dcrt[i * 12 + 7]
			, dcrt[i * 12 + 8], dcrt[i * 12 + 9], dcrt[i * 12 + 10], dcrt[i * 12 + 11]
		);
	}

	OutputDiscLogA(
		"\t                        Media ID: "
	);
	for (WORD k = 1120; k < 1136; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}

	CHAR date[21] = {};
	printwin32filetime(MAKEUINT64(MAKEUINT(MAKEWORD(buf[1183], buf[1184]), MAKEWORD(buf[1185], buf[1186]))
		, MAKEUINT(MAKEWORD(buf[1187], buf[1188]), MAKEWORD(buf[1189], buf[1190]))), date);
	OutputDiscLogA(
		"\n"
		"\t          Timestamp of authoring: %s\n"
		"\t                         Unknown: %02x\n"
		"\t                         Unknown: "
		, date, buf[1210]
	);
	for (WORD k = 1211; k < 1227; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}

	OutputDiscLogA(
		"\n"
		"\t                    SHA-1 hash A: "
	);
	for (WORD k = 1227; k < 1247; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}

	OutputDiscLogA(
		"\n"
		"\t                     Signature A: "
	);
	for (WORD k = 1247; k < 1503; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}

	printwin32filetime(MAKEUINT64(MAKEUINT(MAKEWORD(buf[1503], buf[1504]), MAKEWORD(buf[1505], buf[1506]))
		, MAKEUINT(MAKEWORD(buf[1507], buf[1508]), MAKEWORD(buf[1509], buf[1510]))), date);
	OutputDiscLogA(
		"\n"
		"\t          Timestamp of mastering: %s\n"
		"\t                         Unknown: %02x\n"
		"\t                         Unknown: "
		, date, buf[1530]
	);
	for (WORD k = 1531; k < 1547; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}

	OutputDiscLogA(
		"\n"
		"\t                    SHA-1 hash B: "
	);
	for (WORD k = 1547; k < 1567; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}
	OutputDiscLogA(
		"\n"
		"\t                     Signature B: "
	);

	for (WORD k = 1567; k < 1632; k++) {
		OutputDiscLogA("%02x", buf[k]);
	}
	OutputDiscLogA(
		"\n"
		"\tNumber of security sector ranges: %u\n"
		"\t          security sector ranges: \n"
		, buf[1632]);

	DWORD startLBA = 0, endLBA = 0;
#ifdef _WIN32
	PDVD_FULL_LAYER_DESCRIPTOR dvdLayer = (PDVD_FULL_LAYER_DESCRIPTOR)buf;
	DWORD dwEndLayerZeroSector = dvdLayer->commonHeader.EndLayerZeroSector;
	REVERSE_LONG(&dwEndLayerZeroSector);
#else
	DWORD dwEndLayerZeroSector = MAKEUINT(MAKEWORD(buf[15], buf[14]), MAKEWORD(buf[13], 0));
#endif
	BYTE ssNum = buf[1632];

	for (INT k = 1633, i = 0; i < ssNum; k += 9, i++) {
		DWORD startPsn = MAKEDWORD(MAKEWORD(buf[k + 5], buf[k + 4]), MAKEWORD(buf[k + 3], 0));
		DWORD endPsn = MAKEDWORD(MAKEWORD(buf[k + 8], buf[k + 7]), MAKEWORD(buf[k + 6], 0));
		if (i == 0) {
			OutputDiscLogA("\t\t       Layer 0");
			startLBA = startPsn - 0x30000;
			endLBA = endPsn - 0x30000;
			pDisc->DVD.securitySectorRange[i][0] = startLBA;
			pDisc->DVD.securitySectorRange[i][1] = endLBA;
		}
		else if (i == 3) {
			OutputDiscLogA("\t\t       Layer 1");
			startLBA = dwEndLayerZeroSector * 2 - (~startPsn & 0xffffff) - 0x30000 + 1;
			endLBA = dwEndLayerZeroSector * 2 - (~endPsn & 0xffffff) - 0x30000 + 1;
			pDisc->DVD.securitySectorRange[i][0] = startLBA;
			pDisc->DVD.securitySectorRange[i][1] = endLBA;
		}
		else {
			OutputDiscLogA("\t\tUnknown ranges");
			startLBA = startPsn;
			endLBA = endPsn;
		}
		OutputDiscLogA("\t\tResponse Type: %02x, Challenge ID: %02x, Mod: %02x, startLBA-endLBA: %8lu-%8lu\n"
			, buf[k], buf[k + 1], buf[k + 2], startLBA, endLBA);
	}
}
