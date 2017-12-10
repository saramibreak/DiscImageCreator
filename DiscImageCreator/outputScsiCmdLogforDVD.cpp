/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "output.h"
#include "outputScsiCmdLogforCD.h"

VOID OutputFsRecordingDateAndTime(
	LPBYTE lpBuf
) {
	WORD sTime = MAKEWORD(lpBuf[0], lpBuf[1]);
	CHAR cTimeZone = sTime >> 12 & 0x0f;
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
		"\tBoot Extent Location: %lu\n"
		"\t  Boot Extent Length: %lu\n"
		"\t        Load Address: %lu%lu\n"
		"\t       Start Address: %lu%lu\n",
		MAKELONG(MAKEWORD(lpBuf[72], lpBuf[73]), MAKEWORD(lpBuf[74], lpBuf[75])),
		MAKELONG(MAKEWORD(lpBuf[76], lpBuf[77]), MAKEWORD(lpBuf[78], lpBuf[79])),
		MAKELONG(MAKEWORD(lpBuf[80], lpBuf[81]), MAKEWORD(lpBuf[82], lpBuf[83])),
		MAKELONG(MAKEWORD(lpBuf[84], lpBuf[85]), MAKEWORD(lpBuf[86], lpBuf[87])),
		MAKELONG(MAKEWORD(lpBuf[88], lpBuf[89]), MAKEWORD(lpBuf[90], lpBuf[91])),
		MAKELONG(MAKEWORD(lpBuf[92], lpBuf[93]), MAKEWORD(lpBuf[94], lpBuf[95])));

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
		"\t\t  Extent Length: %lu\n"
		"\t\tExtent Location: %lu\n",
		MAKELONG(MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3])),
		MAKELONG(MAKEWORD(lpBuf[4], lpBuf[5]), MAKEWORD(lpBuf[6], lpBuf[7])));
}

VOID OutputFsLongAllocationDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\t\t     Extent Length: %lu\n"
		"\t\t   Extent Location: %02x%02x%02x%02x%02x%02x\n"
		"\t\tImplementation Use: %02x%02x%02x%02x%02x%02x\n"
		, MAKELONG(MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3]))
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
		"\t                     Character Set List: %lu\n"
		"\t             Maximum Character Set List: %lu\n"
		"\t                        File Set Number: %lu\n"
		"\t             File Set Descriptor Number: %lu\n"
		"\tLogical Volume Identifier Character Set:\n"
		, MAKEWORD(lpBuf[28], lpBuf[29])
		, MAKEWORD(lpBuf[30], lpBuf[31])
		, MAKELONG(MAKEWORD(lpBuf[32], lpBuf[33]), MAKEWORD(lpBuf[34], lpBuf[35]))
		, MAKELONG(MAKEWORD(lpBuf[36], lpBuf[37]), MAKEWORD(lpBuf[38], lpBuf[39]))
		, MAKELONG(MAKEWORD(lpBuf[40], lpBuf[41]), MAKEWORD(lpBuf[42], lpBuf[43]))
		, MAKELONG(MAKEWORD(lpBuf[44], lpBuf[45]), MAKEWORD(lpBuf[46], lpBuf[47]))
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
		"\t                   Integrity Type: %lu\n"
		"\tNext Integrity Extent\n"
		, MAKELONG(MAKEWORD(lpBuf[28], lpBuf[29]), MAKEWORD(lpBuf[30], lpBuf[31])));
	OutputFsExtentDescriptor(lpBuf + 32);

	OutputVolDescLogA("\t      Logical Volume Contents Use: ");
	for (INT i = 40; i < 72; i++) {
		OutputVolDescLogA("%02x", lpBuf[i]);
	}

	LONG N_P =
		MAKELONG(MAKEWORD(lpBuf[72], lpBuf[73]), MAKEWORD(lpBuf[74], lpBuf[75]));
	LONG L_IU =
		MAKELONG(MAKEWORD(lpBuf[76], lpBuf[77]), MAKEWORD(lpBuf[78], lpBuf[79]));
	OutputVolDescLogA(
		"\n"
		"\t             Number of Partitions: %lu\n"
		"\t     Length of Implementation Use: %lu\n"
		, N_P, L_IU);
	INT nOfs = N_P * 4;
	if (0 < N_P) {
		OutputVolDescLogA("\t                 Free Space Table: ");
		for (INT i = 0; i < N_P; i += 4) {
			OutputVolDescLogA("%lu \n"
				, MAKELONG(MAKEWORD(lpBuf[80 + i], lpBuf[81 + i]), MAKEWORD(lpBuf[82 + i], lpBuf[83 + i])));
		}
		OutputVolDescLogA("\t                       Size Table: ");
		for (INT i = 80 + nOfs, j = 0; j < N_P; j += 4) {
			OutputVolDescLogA("%lu "
				, MAKELONG(MAKEWORD(lpBuf[i + j], lpBuf[i + 1 + j]), MAKEWORD(lpBuf[i + 2 + j], lpBuf[i + 3 + j])));
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
	LONG N_AD =
		MAKELONG(MAKEWORD(lpBuf[20], lpBuf[21]), MAKEWORD(lpBuf[22], lpBuf[23]));
	OutputVolDescLogA(
		"\tVolume Descriptor Sequence Number: %lu\n"
		"\t Number of Allocation Descriptors: %lu\n"
		, MAKELONG(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])),
		N_AD);
	if (0 < N_AD) {
		OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR(Allocation Descriptors));
		for (INT i = 0; i < N_AD * 8; i += 8) {
			OutputFsExtentDescriptor(lpBuf + 24 + i);
		}
	}
}

VOID OutputFsLogicalVolumeDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\tVolume Descriptor Sequence Number: %lu\n"
		"\tDescriptor Character Set\n",
		MAKELONG(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])));
	OutputFsCharspec(lpBuf + 20);

	OutputVolDescLogA(
		"\tLogical Volume Identifier: %.128s\n"
		"\t      Logical Block Size : %lu\n"
		"\tDomain Identifier\n",
		(LPCH)&lpBuf[84],
		MAKELONG(MAKEWORD(lpBuf[212], lpBuf[213]), MAKEWORD(lpBuf[214], lpBuf[215])));
	OutputFsCharspec(lpBuf + 216);

	OutputVolDescLogA("\tLogical Volume Contents Use: ");
	for (INT i = 248; i < 264; i++) {
		OutputVolDescLogA("%02x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");

	LONG MT_L =
		MAKELONG(MAKEWORD(lpBuf[264], lpBuf[265]), MAKEWORD(lpBuf[266], lpBuf[267]));
	OutputVolDescLogA(
		"\t        Map Table Length: %lu\n"
		"\tNumber of Partition Maps: %lu\n"
		"\tImplementation Identifier\n",
		MT_L,
		MAKELONG(MAKEWORD(lpBuf[268], lpBuf[269]), MAKEWORD(lpBuf[270], lpBuf[271])));
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
		for (INT i = 0; i < MT_L; i++) {
			OutputVolDescLogA("%02x", lpBuf[440 + i]);
		}
		OutputVolDescLogA("\n");
	}
}

VOID OutputFsPartitionDescriptor(
	LPBYTE lpBuf
) {
	OutputVolDescLogA(
		"\tVolume Descriptor Sequence Number: %lu\n"
		"\t                  Partition Flags: %u\n"
		"\t                 Partition Number: %u\n"
		"\tPartition Contents\n",
		MAKELONG(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])),
		MAKEWORD(lpBuf[20], lpBuf[21]),
		MAKEWORD(lpBuf[22], lpBuf[23]));

	OutputFsRegid(lpBuf + 24);
	OutputVolDescLogA("\tPartition Contents Use: ");
	for (INT i = 56; i < 184; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}

	OutputVolDescLogA(
		"\n"
		"\t                Access Type: %lu\n"
		"\tPartition Starting Location: %lu\n"
		"\t           Partition Length: %lu\n"
		"\tImplementation Identifier\n",
		MAKELONG(MAKEWORD(lpBuf[184], lpBuf[185]), MAKEWORD(lpBuf[186], lpBuf[187])),
		MAKELONG(MAKEWORD(lpBuf[188], lpBuf[189]), MAKEWORD(lpBuf[190], lpBuf[191])),
		MAKELONG(MAKEWORD(lpBuf[192], lpBuf[193]), MAKEWORD(lpBuf[194], lpBuf[195])));

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
		"\tVolume Descriptor Sequence Number: %lu\n"
		"\tImplementation Identifier\n",
		MAKELONG(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])));
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
		"\tVolume Descriptor Sequence Number: %lu\n"
		OUTPUT_DHYPHEN_PLUS_STR(Next Volume Descriptor Sequence Extent),
		MAKELONG(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])));
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
		"\tVolume Descriptor Sequence Number: %lu\n"
		"\t Primary Volume Descriptor Number: %lu\n"
		"\t                Volume Identifier: %.32s\n"
		"\t           Volume Sequence Number: %u\n"
		"\t   Maximum Volume Sequence Number: %u\n"
		"\t                Interchange Level: %u\n"
		"\t        Maximum Interchange Level: %u\n"
		"\t               Character Set List: %lu\n"
		"\t       Maximum Character Set List: %lu\n"
		"\t            Volume Set Identifier: %.128s\n"
		"\tDescriptor Character Set\n",
		MAKELONG(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])),
		MAKELONG(MAKEWORD(lpBuf[20], lpBuf[21]), MAKEWORD(lpBuf[22], lpBuf[23])),
		(LPCH)&lpBuf[24],
		MAKEWORD(lpBuf[56], lpBuf[57]),
		MAKEWORD(lpBuf[58], lpBuf[59]),
		MAKEWORD(lpBuf[60], lpBuf[61]),
		MAKEWORD(lpBuf[62], lpBuf[63]),
		MAKELONG(MAKEWORD(lpBuf[64], lpBuf[65]), MAKEWORD(lpBuf[66], lpBuf[67])),
		MAKELONG(MAKEWORD(lpBuf[68], lpBuf[69]), MAKEWORD(lpBuf[70], lpBuf[71])),
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
		"\tPredecessor Volume Descriptor Sequence Location: %lu\n",
		MAKELONG(MAKEWORD(lpBuf[484], lpBuf[485]), MAKEWORD(lpBuf[486], lpBuf[487])));
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
		"\t                     Tag Location: %lu\n",
		MAKEWORD(lpBuf[2], lpBuf[3]),
		lpBuf[4],
		MAKEWORD(lpBuf[6], lpBuf[7]),
		MAKEWORD(lpBuf[8], lpBuf[9]),
		MAKEWORD(lpBuf[10], lpBuf[11]),
		MAKELONG(MAKEWORD(lpBuf[12], lpBuf[13]), MAKEWORD(lpBuf[14], lpBuf[15])));
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
	}
	return;
}

VOID OutputDVDLayerDescriptor(
	PDVD_FULL_LAYER_DESCRIPTOR dvdLayer,
	LPBYTE lpBcaFlag,
	LPDWORD lpdwSectorLength
) {
	LPCSTR lpBookType[] = {
		"DVD-ROM", "DVD-RAM", "DVD-R", "DVD-RW",
		"HD DVD-ROM", "HD DVD-RAM", "HD DVD-R", "Reserved",
		"Reserved", "DVD+RW", "DVD+R", "Reserved",
		"Reserved", "DVD+RW DL", "DVD+R DL", "Reserved"
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
		"0.74ƒÊm/track", "0.80ƒÊm/track", "0.615ƒÊm/track", "0.40ƒÊm/track",
		"0.34ƒÊm/track", "Reserved", "Reserved", "Reserved",
		"Reserved", "Reserved", "Reserved", "Reserved",
		"Reserved", "Reserved", "Reserved", "Reserved"
	};

	LPCSTR lpLinearDensity[] = {
		"0.267ƒÊm/bit", "0.293ƒÊm/bit", "0.409 to 0.435ƒÊm/bit", "Reserved",
		"0.280 to 0.291ƒÊm/bit", "0.153ƒÊm/bit", "0.130 to 0.140ƒÊm/bit", "Reserved",
		"0.353ƒÊm/bit", "Reserved", "Reserved", "Reserved",
		"Reserved", "Reserved", "Reserved", "Reserved"
	};

	DWORD dwStartingDataSector = dvdLayer->commonHeader.StartingDataSector;
	DWORD dwEndDataSector = dvdLayer->commonHeader.EndDataSector;
	DWORD dwEndLayerZeroSector = dvdLayer->commonHeader.EndLayerZeroSector;
	REVERSE_LONG(&dwStartingDataSector);
	REVERSE_LONG(&dwEndDataSector);
	REVERSE_LONG(&dwEndLayerZeroSector);

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
	*lpBcaFlag = dvdLayer->commonHeader.BCAFlag;

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
	}
	else {
		*lpdwSectorLength = dwEndDataSector - dwStartingDataSector + 1;
		OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(SectorLength)
			"\tLayerZeroSector: %7lu (%#lx)\n", *lpdwSectorLength, *lpdwSectorLength);
	}
}

VOID OutputDVDCopyrightDescriptor(
	PDVD_COPYRIGHT_DESCRIPTOR dvdCopyright,
	LPBOOL lpbCPRM
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(CopyrightInformation)
		"\t    CopyrightProtectionType: ");
	switch (dvdCopyright->CopyrightProtectionType) {
	case 0:
		OutputDiscLogA("No\n");
		break;
	case 1:
		OutputDiscLogA("CSS/CPPM\n");
		break;
	case 2:
		OutputDiscLogA("CPRM\n");
		*lpbCPRM = TRUE;
		break;
	case 3:
		OutputDiscLogA("AACS with HD DVD content\n");
		break;
	case 10:
		OutputDiscLogA("AACS with BD content\n");
		break;
	default:
		OutputDiscLogA("Unknown: %02x\n", dvdCopyright->CopyrightProtectionType);
		break;
	}
	OutputDiscLogA(
		"\tRegionManagementInformation: %02x\n", dvdCopyright->RegionManagementInformation);
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

VOID OutputDVDBCADescriptor(
	PDVD_BCA_DESCRIPTOR dvdBca,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(BCAInformation));
	OutputCDMain(fileDisc, dvdBca->BCAInformation, 0, wFormatLength);
}

VOID OutputDVDManufacturerDescriptor(
	PDVD_MANUFACTURER_DESCRIPTOR dvdManufacturer
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(ManufacturingInformation));
	OutputCDMain(fileDisc, dvdManufacturer->ManufacturingInformation, 0,
		sizeof(dvdManufacturer->ManufacturingInformation));
}

VOID OutputDVDMediaId(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(Media ID));
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDVDMediaKeyBlock(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(MediaKeyBlock)
		"\tMedia Key Block Total Packs: %u"
		"\tmedia key block: ",
		lpFormat[3]);
	for (WORD k = 0; k < wFormatLength; k++) {
		OutputDiscLogA("%02x", lpFormat[k]);
	}
	OutputDiscLogA("\n");
}

VOID OutputDVDRamDds(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVD-RAM DDS)"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDVDRamMediumStatus(
	PDVD_RAM_MEDIUM_STATUS dvdRamMeium
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDRamMediumStatus)
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

VOID OutputDVDRamSpareArea(
	PDVD_RAM_SPARE_AREA_INFORMATION dvdRamSpare
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDRamSpareAreaInformation)
		"\t          FreePrimarySpareSectors: %lu\n"
		"\t     FreeSupplementalSpareSectors: %lu\n"
		"\tAllocatedSupplementalSpareSectors: %lu\n",
		MAKELONG(
		MAKEWORD(dvdRamSpare->FreePrimarySpareSectors[3], dvdRamSpare->FreePrimarySpareSectors[2]),
		MAKEWORD(dvdRamSpare->FreePrimarySpareSectors[1], dvdRamSpare->FreePrimarySpareSectors[0])),
		MAKELONG(
		MAKEWORD(dvdRamSpare->FreeSupplementalSpareSectors[3], dvdRamSpare->FreeSupplementalSpareSectors[2]),
		MAKEWORD(dvdRamSpare->FreeSupplementalSpareSectors[1], dvdRamSpare->FreeSupplementalSpareSectors[0])),
		MAKELONG(
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
	for (BYTE i = 0; i < nRoop; i++) {
		OutputCDMain(fileDisc, lpFormat + DISC_RAW_READ_SIZE * i, 0, DISC_RAW_READ_SIZE);
	}
}

VOID OutputDVDRecordingManagementAreaData(
	PDVD_RECORDING_MANAGEMENT_AREA_DATA dvdRecordingMan,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDRecordingManagementAreaData)
		"\tLastRecordedRMASectorNumber: %lu\n"
		"\t                   RMDBytes: \n",
		MAKELONG(MAKEWORD(dvdRecordingMan->LastRecordedRMASectorNumber[3],
		dvdRecordingMan->LastRecordedRMASectorNumber[2]),
		MAKEWORD(dvdRecordingMan->LastRecordedRMASectorNumber[1],
		dvdRecordingMan->LastRecordedRMASectorNumber[0])));
	UINT nRoop = (wFormatLength - sizeof(DVD_RECORDING_MANAGEMENT_AREA_DATA)) / DISC_RAW_READ_SIZE;
	for (BYTE i = 0; i < nRoop; i++) {
		OutputCDMain(fileDisc, dvdRecordingMan->RMDBytes + DISC_RAW_READ_SIZE * i, 0, DISC_RAW_READ_SIZE);
	}
}

VOID OutputDVDPreRecordedInformation(
	PDVD_PRERECORDED_INFORMATION dvdPreRecorded
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDPreRecordedInformation)
		"\t                      FieldID_1: %02x\n"
		"\t            DiscApplicatiowCode: %02x\n"
		"\t               DiscPhysicalCode: %02x\n"
		"\tLastAddressOfDataRecordableArea: %lu\n"
		"\t                  ExtensiowCode: %02x\n"
		"\t                    PartVers1on: %02x\n"
		"\t                      FieldID_2: %02x\n"
		"\t               OpcSuggestedCode: %02x\n"
		"\t                 WavelengthCode: %02x\n"
		"\t              WriteStrategyCode: %02x%02x%02x%02x\n"
		"\t                      FieldID_3: %02x\n"
		"\t               ManufacturerId_3: %02x%02x%02x%02x%02x%02x\n"
		"\t                      FieldID_4: %02x\n"
		"\t               ManufacturerId_4: %02x%02x%02x%02x%02x%02x\n"
		"\t                      FieldID_5: %02x\n"
		"\t               ManufacturerId_5: %02x%02x%02x%02x%02x%02x\n",
		dvdPreRecorded->FieldID_1,
		dvdPreRecorded->DiscApplicationCode,
		dvdPreRecorded->DiscPhysicalCode,
		MAKELONG(MAKEWORD(dvdPreRecorded->LastAddressOfDataRecordableArea[0]
			, dvdPreRecorded->LastAddressOfDataRecordableArea[1]),
		MAKEWORD(dvdPreRecorded->LastAddressOfDataRecordableArea[2], 0)),
		dvdPreRecorded->ExtensionCode,
		dvdPreRecorded->PartVers1on,
		dvdPreRecorded->FieldID_2,
		dvdPreRecorded->OpcSuggestedCode,
		dvdPreRecorded->WavelengthCode,
		dvdPreRecorded->WriteStrategyCode[0], dvdPreRecorded->WriteStrategyCode[1],
		dvdPreRecorded->WriteStrategyCode[2], dvdPreRecorded->WriteStrategyCode[3],
		dvdPreRecorded->FieldID_3,
		dvdPreRecorded->ManufacturerId_3[0], dvdPreRecorded->ManufacturerId_3[1],
		dvdPreRecorded->ManufacturerId_3[2], dvdPreRecorded->ManufacturerId_3[3],
		dvdPreRecorded->ManufacturerId_3[4], dvdPreRecorded->ManufacturerId_3[5],
		dvdPreRecorded->FieldID_4,
		dvdPreRecorded->ManufacturerId_4[0], dvdPreRecorded->ManufacturerId_4[1],
		dvdPreRecorded->ManufacturerId_4[2], dvdPreRecorded->ManufacturerId_4[3],
		dvdPreRecorded->ManufacturerId_4[4], dvdPreRecorded->ManufacturerId_4[5],
		dvdPreRecorded->FieldID_5,
		dvdPreRecorded->ManufacturerId_5[0], dvdPreRecorded->ManufacturerId_5[1],
		dvdPreRecorded->ManufacturerId_5[2], dvdPreRecorded->ManufacturerId_5[3],
		dvdPreRecorded->ManufacturerId_5[4], dvdPreRecorded->ManufacturerId_5[5]);
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
		"\t         Layer0Sectors: %lu\n",
		BOOLEAN_TO_STRING_YES_NO_A(dvdDualLayer->Layer0SectorsImmutable),
		MAKELONG(MAKEWORD(dvdDualLayer->Layer0Sectors[3], dvdDualLayer->Layer0Sectors[2]),
		MAKEWORD(dvdDualLayer->Layer0Sectors[1], dvdDualLayer->Layer0Sectors[0])));
}
VOID OutputDVDDualLayerMiddleZone(
	PDVD_DUAL_LAYER_MIDDLE_ZONE_START_ADDRESS dvdDualLayerMiddle
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDDualLayerMiddleZoneStartAddress)
		"\t                   InitStatus: %s\n"
		"\tShiftedMiddleAreaStartAddress: %lu\n",
		BOOLEAN_TO_STRING_YES_NO_A(dvdDualLayerMiddle->InitStatus),
		MAKELONG(MAKEWORD(dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[3],
		dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[2]),
		MAKEWORD(dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[1],
		dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[0])));
}

VOID OutputDVDDualLayerJumpInterval(
	PDVD_DUAL_LAYER_JUMP_INTERVAL_SIZE dvdDualLayerJump
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDDualLayerJumpIntervalSize)
		"\tJumpIntervalSize: %lu\n",
		MAKELONG(MAKEWORD(dvdDualLayerJump->JumpIntervalSize[3],
		dvdDualLayerJump->JumpIntervalSize[2]),
		MAKEWORD(dvdDualLayerJump->JumpIntervalSize[1],
		dvdDualLayerJump->JumpIntervalSize[0])));
}

VOID OutputDVDDualLayerManualLayerJump(
	PDVD_DUAL_LAYER_MANUAL_LAYER_JUMP dvdDualLayerMan
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDDualLayerManualLayerJump)
		"\tManualJumpLayerAddress: %lu\n",
		MAKELONG(MAKEWORD(dvdDualLayerMan->ManualJumpLayerAddress[3],
		dvdDualLayerMan->ManualJumpLayerAddress[2]),
		MAKEWORD(dvdDualLayerMan->ManualJumpLayerAddress[1],
		dvdDualLayerMan->ManualJumpLayerAddress[0])));
}

VOID OutputDVDDualLayerRemapping(
	PDVD_DUAL_LAYER_REMAPPING_INFORMATION dvdDualLayerRemapping
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDDualLayerRemappingInformation)
		"\tManualJumpLayerAddress: %lu\n",
		MAKELONG(MAKEWORD(dvdDualLayerRemapping->RemappingAddress[3],
		dvdDualLayerRemapping->RemappingAddress[2]),
		MAKEWORD(dvdDualLayerRemapping->RemappingAddress[1],
		dvdDualLayerRemapping->RemappingAddress[0])));
}

VOID OutputDVDDiscControlBlockHeader(
	PDVD_DISC_CONTROL_BLOCK_HEADER dvdDiscCtrlBlk
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDDiscControlBlockHeader)
		"\tContentDescriptor: %lu\n"
		"\t           AsByte: %lu\n"
		"\t         VendorId: ",
		MAKELONG(MAKEWORD(dvdDiscCtrlBlk->ContentDescriptor[3],
		dvdDiscCtrlBlk->ContentDescriptor[2]),
		MAKEWORD(dvdDiscCtrlBlk->ContentDescriptor[1],
		dvdDiscCtrlBlk->ContentDescriptor[0])),
		MAKELONG(MAKEWORD(dvdDiscCtrlBlk->ProhibitedActions.AsByte[3],
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
		"\t      UpdateCount: %lu\n"
		"\t           AsByte: %lu\n"
		"\t   UpdatePassword: ",
		MAKELONG(MAKEWORD(dvdDiscCtrlBlkWrite->UpdateCount[3], dvdDiscCtrlBlkWrite->UpdateCount[2]),
		MAKEWORD(dvdDiscCtrlBlkWrite->UpdateCount[1], dvdDiscCtrlBlkWrite->UpdateCount[0])),
		MAKELONG(MAKEWORD(dvdDiscCtrlBlkWrite->WriteProtectActions.AsByte[3],
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

	for (DWORD j = 0; j < sizeof(dvdDiscCtrlBlkSession->SessionItem); j++) {
		OutputDiscLogA(
			"\t  SessionItem: %lu\n"
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
		OutputDiscLogA("%lu",
			MAKELONG(MAKEWORD(dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[3], dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[2]),
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

VOID OutputDVDWriteProtectionStatus(
	PDVD_WRITE_PROTECTION_STATUS dvdWrite
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DVDWriteProtectionStatus)
		"\tSoftwareWriteProtectUntilPowerdown: %s\n"
		"\t       MediaPersistentWriteProtect: %s\n"
		"\t             CartridgeWriteProtect: %s\n"
		"\t         MediaSpecificWriteProtect: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(dvdWrite->SoftwareWriteProtectUntilPowerdown),
		BOOLEAN_TO_STRING_YES_NO_A(dvdWrite->MediaPersistentWriteProtect),
		BOOLEAN_TO_STRING_YES_NO_A(dvdWrite->CartridgeWriteProtect),
		BOOLEAN_TO_STRING_YES_NO_A(dvdWrite->MediaSpecificWriteProtect));
}

VOID OutputDVDAACSVolumeIdentifier(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(AACS Volume Identifiers)"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDVDPreRecordedAACSMediaSerialNumber(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(PreRecorded AACS Media Serial Number)"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDVDAACSMediaIdentifier(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(AACS Media Identifier)"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDVDAACSMediaKeyBlock(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(AACS Media Key Block)"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDVDListOfRecognizedFormatLayers(
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
	BYTE byFormatCode,
	LPBYTE lpBcaFlag,
	LPBOOL lpbCPRM,
	WORD wFormatLength,
	LPBYTE lpFormat,
	LPDWORD lpdwSectorLength
) {
	switch (byFormatCode) {
	case DvdPhysicalDescriptor:
	case 0x10:
		OutputDVDLayerDescriptor((PDVD_FULL_LAYER_DESCRIPTOR)lpFormat, lpBcaFlag, lpdwSectorLength);
		break;
	case DvdCopyrightDescriptor:
		OutputDVDCopyrightDescriptor((PDVD_COPYRIGHT_DESCRIPTOR)lpFormat, lpbCPRM);
		break;
	case DvdDiskKeyDescriptor:
		OutputDVDDiskKeyDescriptor((PDVD_DISK_KEY_DESCRIPTOR)lpFormat);
		break;
	case DvdBCADescriptor:
		OutputDVDBCADescriptor((PDVD_BCA_DESCRIPTOR)lpFormat, wFormatLength);
		break;
	case DvdManufacturerDescriptor:
		OutputDVDManufacturerDescriptor((PDVD_MANUFACTURER_DESCRIPTOR)lpFormat);
		break;
	case 0x06:
		OutputDVDMediaId(lpFormat, wFormatLength);
		break;
	case 0x07:
		OutputDVDMediaKeyBlock(lpFormat, wFormatLength);
		break;
	case 0x08:
		OutputDVDRamDds(lpFormat, wFormatLength);
		break;
	case 0x09:
		OutputDVDRamMediumStatus((PDVD_RAM_MEDIUM_STATUS)lpFormat);
		break;
	case 0x0a:
		OutputDVDRamSpareArea((PDVD_RAM_SPARE_AREA_INFORMATION)lpFormat);
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
		OutputDVDWriteProtectionStatus((PDVD_WRITE_PROTECTION_STATUS)lpFormat);
		break;
		// formats 0xC1 through 0x7F are not yet defined
	case 0x80:
		OutputDVDAACSVolumeIdentifier(lpFormat, wFormatLength);
		break;
	case 0x81:
		OutputDVDPreRecordedAACSMediaSerialNumber(lpFormat, wFormatLength);
		break;
	case 0x82:
		OutputDVDAACSMediaIdentifier(lpFormat, wFormatLength);
		break;
	case 0x83:
		OutputDVDAACSMediaKeyBlock(lpFormat, wFormatLength);
		break;
		// formats 0x84 through 0x8F are not yet defined
	case 0x90:
		OutputDVDListOfRecognizedFormatLayers((PDVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS_TYPE_CODE)lpFormat);
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
	OutputDiscLogA(STR_LBA, nLBA, nLBA);
	if ((dvdCopyright->CPR_MAI & 0x80) == 0x80) {
		if ((dvdCopyright->CPR_MAI & 0x40) == 0x40) {
			switch (dvdCopyright->CPR_MAI & 0x0f) {
			case 0:
				OutputDiscLogA("This sector is scrambled by CSS");
				break;
			case 0x01:
				OutputDiscLogA("This sector is encrypted by CPPM");
				break;
			default:
				OutputDiscLogA("reserved");
			}
		}
		else {
			OutputDiscLogA("CSS or CPPM doesn't exist in this sector");
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
		OutputDiscLogA("No protected sector\n");
	}
}
