/**
 * Copyright 2011-2024 sarami
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

VOID OutputDVDHeader(
	LPBYTE lpBuf,
	DWORD dwSectorSize,
	INT nLBA,
	BOOL bNintendoDisc
) {
	OutputRawReadableLog(
		STR_LBA "SectorInfo[%02x]{Layer %d, %s, "
		, nLBA, (UINT)nLBA, lpBuf[0], lpBuf[0] & 0x01
		, (lpBuf[0] & 0x02) == 0 ? _T("read-only") : _T("other")
	);
	switch ((lpBuf[0] & 0x0c) >> 2) {
	case 0:
		OutputRawReadableLog("Data Zone, ");
		break;
	case 1:
		OutputRawReadableLog("Lead-in Zone, ");
		break;
	case 2:
		OutputRawReadableLog("Lead-out Zone, ");
		break;
	case 3:
		OutputRawReadableLog("Middle Zone, ");
		break;
	default:
		break;
	}
	UINT gotSectorNum = MAKEUINT(MAKEWORD(lpBuf[3], lpBuf[2]), MAKEWORD(lpBuf[1], 0));
	if (bNintendoDisc) {
		OutputRawReadableLog(
			"%s}, SectorNumber[%06x (%8u)], IED[%04x], Unknown[%02x%02x%02x%02x%02x%02x], "
			, (lpBuf[0] & 0x20) == 0 ? _T("greater than 40%") : _T("         40% max")
			, gotSectorNum, gotSectorNum, MAKEWORD(lpBuf[5], lpBuf[4])
			, lpBuf[2054], lpBuf[2055], lpBuf[2056], lpBuf[2057], lpBuf[2058], lpBuf[2059]
		);
	}
	else {
		OutputRawReadableLog(
			"%s}, SectorNumber[%06x (%8u)], IED[%04x], CPR_MAI[%02x%02x%02x%02x%02x%02x], "
			, (lpBuf[0] & 0x20) == 0 ? _T("greater than 40%") : _T("         40% max")
			, gotSectorNum, gotSectorNum, MAKEWORD(lpBuf[5], lpBuf[4])
			, lpBuf[6], lpBuf[7], lpBuf[8], lpBuf[9], lpBuf[10], lpBuf[11]
		);
	}
	UINT edc = MAKEUINT(MAKEWORD(lpBuf[2063], lpBuf[2062]), MAKEWORD(lpBuf[2061], lpBuf[2060]));
	if (dwSectorSize == DVD_RAW_SECTOR_SIZE_2384) {
		edc = MAKEUINT(MAKEWORD(lpBuf[2173], lpBuf[2172]), MAKEWORD(lpBuf[2171], lpBuf[2170]));
	}
	OutputRawReadableLog("EDC[%08x]\n", edc);
}

VOID OutputDVDRamLayerDescriptor(
	PDISC pDisc,
	LPBYTE lpBuf
) {
	if ((lpBuf[0] & 0x0f) == 1) { // ECMA-272
		OutputDiscLog(
			OUTPUT_DHYPHEN_PLUS_STR("PhysicalFormatInformation")
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
			, (lpBuf[0] & 0x10) == 0x10 ? _T("rewritable disk") : _T("other")
			, lpBuf[0] & 0x0f
			, (lpBuf[1] & 0xf0) == 0 ? _T("120 mm") : _T("80 mm")
			, (lpBuf[1] & 0x0f) == 0x02 ? _T("10,08 Mbits/s") : _T("other")
			, (lpBuf[2] & 0x60) == 0 ? _T("single layer") : _T("other")
			, (lpBuf[2] & 0x0f) == 0x04 ? _T("rewritable recording layer") : _T("other")
			, (lpBuf[3] & 0xf0) == 0x20 ? _T("0,205 um to 0,218 um") : _T("other")
			, (lpBuf[3] & 0x0f) == 0 ? _T("0,74 um") : _T("other")
			, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], 0))
			, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], 0))
			, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], 0))
			, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], 0))
			, lpBuf[32] == 0 ? _T("disk shall not be recorded without a case") : _T("disk may be recorded with or without a case")
			, lpBuf[48] == 0x3c ? _T("6,0 m/s") : _T("other")
			, lpBuf[49] == 0x0a ? _T("1,0 mW") : _T("other")
			, lpBuf[50] == 0x6e ? _T("11,0 mW") : _T("other")
			, lpBuf[51] == 0x32 ? _T("5,0 mW") : _T("other")
			, (lpBuf[52] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, ((lpBuf[52] & 0x3f) == 0x11) ? _T("TSFP of 17 ns") : _T("other")
			, lpBuf[53] == 0x33 ? _T("TEFP of 51 ns") : _T("other")
			, lpBuf[54] == 0x11 ? _T("TMP of 17 ns") : _T("other")
			, (lpBuf[55] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, ((lpBuf[55] & 0x3f) == 0) ? _T("TSLP of 0 ns") : _T("other")
			, lpBuf[56] == 0x22 ? _T("TELP of 34 ns") : _T("other")
			, lpBuf[57] == 0x44 ? _T("TLE of 68 ns") : _T("other")
			, lpBuf[58] == 0x6e ? _T("11,0 mW") : _T("other")
			, lpBuf[59] == 0x32 ? _T("5,0 mW") : _T("other")
			, (lpBuf[60] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, ((lpBuf[60] & 0x3f) == 0x11) ? _T("TSFP of 17 ns") : _T("other")
			, lpBuf[61] == 0x33 ? _T("TEFP of 51 ns") : _T("other")
			, lpBuf[62] == 0x11 ? _T("TMP of 17 ns") : _T("other")
			, (lpBuf[63] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, ((lpBuf[63] & 0x3f) == 0) ? _T("TSLP of 0 ns") : _T("other")
			, lpBuf[64] == 0x22 ? _T("TELP of 34 ns") : _T("other")
			, lpBuf[65] == 0x44 ? _T("TLE of 68 ns") : _T("other")
		);
	}
	else if ((lpBuf[0] & 0x0f) == 6) { // ECMA-330
		OutputDiscLog(
			OUTPUT_DHYPHEN_PLUS_STR("PhysicalFormatInformation")
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
			, (lpBuf[0] & 0x10) == 0x10 ? _T("rewritable disk") : _T("other")
			, lpBuf[0] & 0x0f
			, (lpBuf[1] & 0xf0) == 0 ? _T("120 mm") : _T("80 mm")
			, (lpBuf[1] & 0x0f) == 0x0f ? _T("not specified") : _T("other")
			, (lpBuf[2] & 0x60) == 0 ? _T("single layer") : _T("other")
			, (lpBuf[2] & 0x0f) == 0x04 ? _T("rewritable recording layer") : _T("other")
			, (lpBuf[3] & 0xf0) == 0x40 ? _T("0,140 um to 0,148 um") : _T("other")
			, (lpBuf[3] & 0x0f) == 0x02 ? _T("0,615 um") : _T("other")
			, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], 0))
			, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], 0))
			, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], 0))
			, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], 0))
			, (lpBuf[16] & 0x80) == 0x80 ? _T("Exist") : _T("No")
			, lpBuf[32] == 0 ? _T("disk shall not be recorded without a case") : _T("disk may be recorded with or without a case")
			, lpBuf[500] == 0x52 ? _T("8,2 m/s") : _T("other")
			, lpBuf[501] == 0x0a ? _T("1,0 mW") : _T("other")
			, (lpBuf[502] & 0x80) == 0x80 ? _T("2") : _T("1")
			, lpBuf[503], lpBuf[504], lpBuf[505], lpBuf[506]
			, lpBuf[507], lpBuf[508], lpBuf[509], lpBuf[510]
			, lpBuf[511], lpBuf[512], lpBuf[513], lpBuf[514]
			, lpBuf[515], lpBuf[516]
		);
		OutputDiscLog(
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
			, (lpBuf[517] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[517] & 0x3f
			, (lpBuf[518] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[518] & 0x3f
			, (lpBuf[519] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[519] & 0x3f
			, (lpBuf[520] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[520] & 0x3f
			, (lpBuf[521] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[521] & 0x3f
			, (lpBuf[522] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[522] & 0x3f
			, (lpBuf[523] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[523] & 0x3f
			, (lpBuf[524] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[524] & 0x3f
			, (lpBuf[525] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[525] & 0x3f
			, (lpBuf[526] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[526] & 0x3f
			, (lpBuf[527] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[527] & 0x3f
			, (lpBuf[528] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[528] & 0x3f
			, (lpBuf[529] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[529] & 0x3f
			, (lpBuf[530] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[530] & 0x3f
			, (lpBuf[531] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[531] & 0x3f
			, (lpBuf[532] & 0x80) == 0x80 ? _T("opposite") : _T("same")
			, lpBuf[532] & 0x3f
			, lpBuf[533], lpBuf[534], lpBuf[535], lpBuf[536], lpBuf[537], lpBuf[538]
			, lpBuf[539], lpBuf[540], lpBuf[541], lpBuf[542], lpBuf[543], lpBuf[544]
			, lpBuf[545], lpBuf[546], lpBuf[547], lpBuf[548]
		);
		OutputDiscLog(
			"\t                                                          Disk manufacturer's name: %.48" CHARWIDTH "s\n"
			"\t                                     Disk manufacturer's supplementary information: %.16" CHARWIDTH "s\n"
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
			, &lpBuf[549], &lpBuf[597]
			, lpBuf[613], lpBuf[614]
			, lpBuf[615]
			, (lpBuf[616] & 0x80) == 0x80 ? _T("in case of minus sign") : _T("in case of 0 or plus sign")
			, lpBuf[616] & 0x3f
			, lpBuf[617]
			, lpBuf[618], lpBuf[619], lpBuf[620], lpBuf[621], lpBuf[622], lpBuf[623]
		);
		pDisc->DVD.version = (UCHAR)(lpBuf[0] & 0x0f);
		pDisc->DVD.ucBca = (UCHAR)((lpBuf[16] >> 7) & 0x01);
	}
}

VOID OutputDVDMinusRLayerDescriptor(
	LPBYTE lpBuf
) {
	if ((lpBuf[0] & 0x0f) == 1 || (lpBuf[0] & 0x0f) == 2) { // ECMA-279, ECMA-338
		OutputDiscLog(
			OUTPUT_DHYPHEN_PLUS_STR("PreRecordedPhysicalFormatInformation")
			"\t                                      Version Number: %d\n"
			"\t                                       Disk Category: %s\n"
			"\t                               Maximum transfer rate: "
			, lpBuf[0] & 0x0f
			, (lpBuf[0] & 0xf0) == 0x10 ? _T("Recordable disk") : _T("other")
		);
		switch (lpBuf[1] & 0x0f) {
		case 0:
			OutputDiscLog("2.52 Mbits/s\n");
			break;
		case 1:
			OutputDiscLog("5.04 Mbits/s\n");
			break;
		case 2:
			OutputDiscLog("10.08 Mbits/s\n");
			break;
		default:
			OutputDiscLog("other\n");
			break;
		}
		OutputDiscLog(
			"\t                                           Disk size: %s\n"
			"\t                                          Layer type: %s\n"
			"\t                                          Track path: %s\n"
			"\t                                    Number of layers: %d\n"
			"\t                                 Average track pitch: %s\n"
			"\t                                  Channel bit length: %s\n"
			"\t              First Physical Sector of the Data Zone: %u (0x%06x)\n"
			"\t       Sector of the last Rzone in the Bordered Area: %u (0x%06x)\n"
			"\tSector of the first sector of the current Border Out: %u (0x%08x)\n"
			"\t    Sector of the first sector of the next Border In: %u (0x%08x)\n"
			, (lpBuf[1] & 0xf0) == 0 ? _T("120 mm") : _T("80 mm")
			, (lpBuf[2] & 0x0f) == 0x02 ? _T("disk contains Recordable user data Zone(s)") : _T("other")
			, (lpBuf[2] & 0x10) == 0 ? _T("Parallel") : _T("other")
			, (lpBuf[2] & 0x60) == 0 ? 1 : 2
			, (lpBuf[3] & 0x0f) == 1 ? _T("0.80 um") : _T("other")
			, (lpBuf[3] & 0xf0) == 1 ? _T("0.147 um") : _T("other")
			, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], 0))
			, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], 0))
			, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], 0))
			, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], 0))
			, MAKEUINT(MAKEWORD(lpBuf[35], lpBuf[34]), MAKEWORD(lpBuf[33], lpBuf[32]))
			, MAKEUINT(MAKEWORD(lpBuf[35], lpBuf[34]), MAKEWORD(lpBuf[33], lpBuf[32]))
			, MAKEUINT(MAKEWORD(lpBuf[39], lpBuf[38]), MAKEWORD(lpBuf[37], lpBuf[36]))
			, MAKEUINT(MAKEWORD(lpBuf[39], lpBuf[38]), MAKEWORD(lpBuf[37], lpBuf[36]))
		);
	}
	else if ((lpBuf[0] & 0x0f) == 5) { // ECMA-359
		OutputDiscLog(
			OUTPUT_DHYPHEN_PLUS_STR("PreRecordedPhysicalFormatInformation")
			"\t                                                         Version Number: %d\n"
			"\t                                                          Disk Category: %s\n"
			"\t                                                  Maximum transfer rate: %s\n"
			"\t                                                              Disk size: %s\n"
			"\t                                                             Layer type: %s\n"
			"\t                                                             Track path: %s\n"
			"\t                                                       Number of layers: %d\n"
			"\t                                                    Average track pitch: %s\n"
			"\t                                                     Channel bit length: %s\n"
			"\t                                 First Physical Sector of the Data Zone: %u (0x%06x)\n"
			"\t                                    Outer limit of Data Recordable Zone: %u (0x%06x)\n"
			"\t                                                                   NBCA: %s\n"
			"\t                       Start sector of Current RMD in Extra Border Zone: %u (0x%08x)\n"
			"\tStart sector of Physical format information blocks in Extra Border Zone: %u (0x%08x)\n"
			, lpBuf[0] & 0x0f
			, (lpBuf[0] & 0xf0) == 0x10 ? _T("Recordable disk") : _T("other")
			, (lpBuf[1] & 0x0f) == 0x0f ? _T("Not specified") : _T("other")
			, (lpBuf[1] & 0xf0) == 0 ? _T("120 mm") : _T("80 mm")
			, (lpBuf[2] & 0x0f) == 0x02 ? _T("disk contains Recordable user data Zone(s)") : _T("other")
			, (lpBuf[2] & 0x10) == 0 ? _T("Parallel") : _T("other")
			, (lpBuf[2] & 0x60) == 0 ? 1 : 2
			, (lpBuf[3] & 0x0f) == 0 ? _T("0.74 um") : _T("other")
			, (lpBuf[3] & 0xf0) == 0 ? _T("0.133 um") : _T("other")
			, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], 0))
			, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], 0))
			, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], 0))
			, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], 0))
			, (lpBuf[16] & 0x80) == 0 ? _T("does not exist") : _T("exist")
			, MAKEUINT(MAKEWORD(lpBuf[35], lpBuf[34]), MAKEWORD(lpBuf[33], lpBuf[32]))
			, MAKEUINT(MAKEWORD(lpBuf[35], lpBuf[34]), MAKEWORD(lpBuf[33], lpBuf[32]))
			, MAKEUINT(MAKEWORD(lpBuf[39], lpBuf[38]), MAKEWORD(lpBuf[37], lpBuf[36]))
			, MAKEUINT(MAKEWORD(lpBuf[39], lpBuf[38]), MAKEWORD(lpBuf[37], lpBuf[36]))
		);
	}
}

VOID OutputDVDPlusRLayerDescriptor(
	LPBYTE lpBuf
) {
	// ECMA-349
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("PhysicalFormatInformationInADIP")
		"\t                                                                 Disk Category: %s, %s, %s\n"
		"\t                                                                Version Number: %d\n"
		"\t                                                                     Disk size: %s\n"
		"\t                                                         Maximum transfer rate: %s\n"
		"\t                                                            Recording layer(s): %s\n"
		"\t                                                            Channel bit length: %s\n"
		"\t                                                           Average track pitch: %s\n"
		"\t                                        First Physical Sector of the Data Zone: %u (0x%06x)\n"
		"\t                                Last possible Physical Sector of the Data Zone: %u (0x%06x)\n"
		"\t                                                             General Flag bits: %s\n"
		"\t                                                         Disk Application Code: %s\n"
		"\t                                                    Extended Information block: %d\n"
		"\t                                                          Disk Manufacturer ID: %8" CHARWIDTH "s\n"
		"\t                                                                 Media Type ID: %3" CHARWIDTH "s\n"
		"\t                                                       Product revision number: %u\n"
		"\t                    number of Physical format information bytes in use in ADIP: %u\n"
		"\t                       Primary recording velocity for the basic write strategy: %u (0x%x)\n"
		"\t                         Upper recording velocity for the basic write strategy: %u (0x%x)\n"
		"\t                                                                    Wavelength: %u (0x%x)\n"
		"\t                               Normalized Write power dependency on Wavelength: %u (0x%x)\n"
		"\t                                    Maximum read power, Pr at Primary velocity: %u (0x%x)\n"
		"\t                                                      PIND at Primary velocity: %u (0x%x)\n"
		"\t                                                   Btarget at Primary velocity: %u (0x%x)\n"
		"\t                                      Maximum read power, Pr at Upper velocity: %u (0x%x)\n"
		"\t                                                        PIND at Upper velocity: %u (0x%x)\n"
		"\t                                                     Btarget at Upper velocity: %u (0x%x)\n"
		"\t    Ttop (>=4T) first pulse duration for current mark >=4T at Primary velocity: %u (0x%x)\n"
		"\t      Ttop (=3T) first pulse duration for current mark =3T at Primary velocity: %u (0x%x)\n"
		"\t                                  Tmp multi pulse duration at Primary velocity: %u (0x%x)\n"
		"\t                                   Tlp last pulse duration at Primary velocity: %u (0x%x)\n"
		"\t  dTtop (>=4T) first pulse lead time for current mark >=4T at Primary velocity: %u (0x%x)\n"
		"\t    dTtop (=3T) first pulse lead time for current mark =3T at Primary velocity: %u (0x%x)\n"
		"\tdTle first pulse leading edge shift for previous space =3T at Primary velocity: %u (0x%x)\n"
		"\t      Ttop (>=4T) first pulse duration for current mark >=4T at Upper velocity: %u (0x%x)\n"
		"\t         Ttop (3T) first pulse duration for current mark =3T at Upper velocity: %u (0x%x)\n"
		"\t                                    Tmp multi pulse duration at Upper velocity: %u (0x%x)\n"
		"\t                                     Tlp last pulse duration at Upper velocity: %u (0x%x)\n"
		"\t    dTtop (>=4T) first pulse lead time for current mark >=4T at Upper velocity: %u (0x%x)\n"
		"\t      dTtop (=3T) first pulse lead time for current mark =3T at Upper velocity: %u (0x%x)\n"
		"\t  dTle first pulse leading edge shift for previous space =3T at Upper velocity: %u (0x%x)\n"
		, (lpBuf[0] & 0x80) == 0x80 ? _T("+R/+RW Format") : _T("other")
		, (lpBuf[0] & 0x40) == 0 ? _T("single layer") : _T("other")
		, (lpBuf[0] & 0x20) == 0x20 ? _T("+R disk") : _T("other")
		, lpBuf[0] & 0x0f
		, (lpBuf[1] & 0xf0) == 0 ? _T("120 mm") : _T("80 mm")
		, (lpBuf[1] & 0x0f) == 0x0f ? _T("not specified") : _T("other")
		, (lpBuf[2] & 0x02) == 0x02 ? _T("write-once recording layer") : _T("other")
		, (lpBuf[3] & 0xf0) == 0 ? _T("0.133 um") : _T("other")
		, (lpBuf[3] & 0x0f) == 0 ? _T("0.74 um") : _T("other")
		, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], 0))
		, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], 0))
		, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], 0))
		, MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], 0))
		, (lpBuf[16] & 0x40) == 0 ? _T("no Extended format information for VCPS is present") : _T("Data Zone contains Extended format information for VCPS as defined")
		, lpBuf[17] == 0 ? _T("disk for General Purpose use") : _T("other")
		, lpBuf[18]
		, &lpBuf[19]
		, &lpBuf[27]
		, lpBuf[30]
		, lpBuf[31]
		, lpBuf[32], lpBuf[32]
		, lpBuf[33], lpBuf[33]
		, lpBuf[34], lpBuf[34]
		, lpBuf[35], lpBuf[35]
		, lpBuf[36], lpBuf[36]
		, lpBuf[37], lpBuf[37]
		, lpBuf[38], lpBuf[38]
		, lpBuf[39], lpBuf[39]
		, lpBuf[40], lpBuf[40]
		, lpBuf[41], lpBuf[41]
		, lpBuf[42], lpBuf[42]
		, lpBuf[43], lpBuf[43]
		, lpBuf[44], lpBuf[44]
		, lpBuf[45], lpBuf[45]
		, lpBuf[46], lpBuf[46]
		, lpBuf[47], lpBuf[47]
		, lpBuf[48], lpBuf[48]
		, lpBuf[49], lpBuf[49]
		, lpBuf[50], lpBuf[50]
		, lpBuf[51], lpBuf[51]
		, lpBuf[52], lpBuf[52]
		, lpBuf[53], lpBuf[53]
		, lpBuf[54], lpBuf[54]
		, lpBuf[55], lpBuf[55]
	);
}

VOID OutputDVDLayerDescriptor(
	PDISC pDisc,
	PDVD_FULL_LAYER_DESCRIPTOR dvdLayer,
	LPDWORD lpdwSectorLength,
	UCHAR layerNumber,
	LOG_TYPE type
) {
	// Nintendo optical discs output "Reserved5"
	LPCTSTR lpBookType[] = {
		_T("DVD-ROM"), _T("DVD-RAM"), _T("DVD-R"), _T("DVD-RW"),
		_T("HD DVD-ROM"), _T("HD DVD-RAM"), _T("HD DVD-R"), _T("Reserved1"),
		_T("Reserved2"), _T("DVD+RW"), _T("DVD+R"), _T("Reserved3"),
		_T("Reserved4"), _T("DVD+RW DL"), _T("DVD+R DL"), _T("Reserved5")
	};

	LPCTSTR lpMaximumRate[] = {
		_T("2.52 Mbps"), _T("5.04 Mbps"), _T("10.08 Mbps"), _T("20.16 Mbps"),
		_T("30.24 Mbps"), _T("Reserved"), _T("Reserved"), _T("Reserved"),
		_T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved"),
		_T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Not Specified")
	};

	LPCTSTR lpLayerType[] = {
		_T("Unknown"), _T("Layer contains embossed data"), _T("Layer contains recordable data"), _T("Unknown"),
		_T("Layer contains rewritable data"), _T("Unknown"), _T("Unknown"), _T("Unknown"),
		_T("Reserved"), _T("Unknown"), _T("Unknown"), _T("Unknown"),
		_T("Unknown"), _T("Unknown"), _T("Unknown"), _T("Unknown")
	};

	LPCTSTR lpTrackDensity[] = {
		_T("0.74 um/track"), _T("0.80 um/track"), _T("0.615 um/track"), _T("0.40 um/track"),
		_T("0.34um/track"), _T("Reserved"), _T("Reserved"), _T("Reserved"),
		_T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved"),
		_T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved")
	};

	LPCTSTR lpLinearDensity[] = {
		_T("0.267 um/bit"), _T("0.293 um/bit"), _T("0.409 to 0.435 um/bit"), _T("Reserved"),
		_T("0.280 to 0.291 um/bit"), _T("0.153 um/bit"), _T("0.130 to 0.140 um/bit"), _T("Reserved"),
		_T("0.353 um/bit"), _T("Reserved"), _T("Reserved"), _T("Reserved"),
		_T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved")
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

	OutputLog(type,
		OUTPUT_DHYPHEN_PLUS_STR("PhysicalFormatInformation")
		"\t       BookVersion: %hhu\n"
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
		"\t     MediaSpecific: \n"
		, dvdLayer->commonHeader.BookVersion
		, lpBookType[dvdLayer->commonHeader.BookType]
		, lpMaximumRate[dvdLayer->commonHeader.MinimumRate]
		, dvdLayer->commonHeader.DiskSize == 0 ? _T("120 mm") : _T("80 mm")
		, lpLayerType[dvdLayer->commonHeader.LayerType]
		, dvdLayer->commonHeader.TrackPath == 0 ? _T("Parallel Track Path") : _T("Opposite Track Path")
		, dvdLayer->commonHeader.NumberOfLayers == 0 ? _T("Single Layer") : _T("Double Layer")
		, lpTrackDensity[dvdLayer->commonHeader.TrackDensity]
		, lpLinearDensity[dvdLayer->commonHeader.LinearDensity]
		, dwStartingDataSector, dwStartingDataSector
		, dwEndDataSector, dwEndDataSector
		, dwEndLayerZeroSector, dwEndLayerZeroSector
		, dvdLayer->commonHeader.BCAFlag == 0 ? _T("No") : _T("Exist")
	);
	pDisc->DVD.ucBca = dvdLayer->commonHeader.BCAFlag;

	OutputMainChannel(type, dvdLayer->MediaSpecific, NULL, 0, sizeof(dvdLayer->MediaSpecific));

	if (dvdLayer->commonHeader.TrackPath) {
		DWORD dwEndLayerZeroSectorLen = dwEndLayerZeroSector - dwStartingDataSector + 1;
		DWORD dwEndLayerOneSectorLen = dwEndLayerZeroSector - (~dwEndDataSector & 0xffffff) + 1;
		*lpdwSectorLength = dwEndLayerZeroSectorLen + dwEndLayerOneSectorLen;
		OutputLog(type, OUTPUT_DHYPHEN_PLUS_STR("SectorLength")
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
		else {
			pDisc->DVD.dwXboxLayer0SectorLength = dwEndLayerZeroSectorLen;
		}
		if (pDisc->DVD.dwLayer1SectorLength == 0) {
			pDisc->DVD.dwLayer1SectorLength = dwEndLayerOneSectorLen;
		}
		else {
			pDisc->DVD.dwXboxLayer1SectorLength = dwEndLayerOneSectorLen;
		}
	}
	else {
		if (layerNumber == 0) {
			*lpdwSectorLength = dwEndDataSector - dwStartingDataSector + 1;
			pDisc->DVD.dwLayer0SectorLength = *lpdwSectorLength;
			OutputLog(type,
				OUTPUT_DHYPHEN_PLUS_STR("SectorLength")
				"\tLayerZeroSector: %7lu (%#lx)\n"
				, *lpdwSectorLength, *lpdwSectorLength);
		}
		else if (layerNumber == 1) {
			*lpdwSectorLength = dwEndDataSector - dwStartingDataSector + 1;
			OutputLog(type,
				OUTPUT_DHYPHEN_PLUS_STR("SectorLength")
				"\tLayerOneSector: %7lu (%#lx)\n"
				, *lpdwSectorLength, *lpdwSectorLength);
		}
	}
}

VOID OutputDVDRegion(
	UCHAR ucRMI,
	UCHAR ucFlag,
	CONST _TCHAR* region,
	LOG_TYPE type
) {
	if ((ucRMI & ucFlag) == 0) {
		OutputLog(type, "%s", region);
	}
}

VOID OutputDVDCopyrightDescriptor(
	PDVD_COPYRIGHT_DESCRIPTOR dvdCopyright,
	PPROTECT_TYPE_DVD pProtect,
	LOG_TYPE type
) {
	OutputLog(type,
		OUTPUT_DHYPHEN_PLUS_STR("CopyrightInformation")
		"\t    CopyrightProtectionType: ");
	switch (dvdCopyright->CopyrightProtectionType) {
	case 0:
		OutputLog(type, "No\n");
		*pProtect = noProtect;
		break;
	case 1:
		OutputLog(type, "CSS/CPPM\n");
		*pProtect = css;
		// cppm is set by ReadDirectoryRecordDetail of execScsiCmdforFileSystem.cpp
		break;
	case 2:
		OutputLog(type, "CPRM\n");
		*pProtect = cprm;
		break;
	case 3:
		OutputLog(type, "AACS with HD DVD content\n");
		*pProtect = aacs;
		break;
	case 10:
		OutputLog(type, "AACS with BD content\n");
		*pProtect = aacs;
		break;
	default:
		// Nintendo optical discs output 0xfd
		OutputLog(type, "Unknown (%#02x)\n", dvdCopyright->CopyrightProtectionType);
		*pProtect = noProtect;
		break;
	}
	OutputLog(type, "\tRegionManagementInformation:");
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x01, _T(" 1"), type);
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x02, _T(" 2"), type);
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x04, _T(" 3"), type);
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x08, _T(" 4"), type);
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x10, _T(" 5"), type);
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x20, _T(" 6"), type);
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x40, _T(" 7"), type);
	OutputDVDRegion(dvdCopyright->RegionManagementInformation, 0x80, _T(" 8"), type);
	OutputLog(type, "\n");
}

VOID OutputDVDDiskKeyDescriptor(
	PDVD_DISK_KEY_DESCRIPTOR dvdDiskKey
) {
	OutputMainChannel(fileDisc, dvdDiskKey->DiskKeyData, _T("DiskKeyData"), 0, sizeof(dvdDiskKey->DiskKeyData));
}

VOID OutputDiscBCADescriptor(
	PDISC pDisc,
	PDVD_BCA_DESCRIPTOR dvdBca,
	WORD wFormatLength,
	LOG_TYPE type
) {
	OutputLog(type, OUTPUT_DHYPHEN_PLUS_STR("BCAInformation"));
	if (pDisc->DVD.protect == cprm) {
		OutputLog(type,
			"\t   BCA Record ID: %d\n"
			"\t  Version Number: %d\n"
			"\t     Data Length: %d\n"
			"\tMedia Identifier:"
			, MAKEWORD(dvdBca->BCAInformation[1], dvdBca->BCAInformation[0])
			, dvdBca->BCAInformation[2], dvdBca->BCAInformation[3]
		);
		for (BYTE i = 0; i < dvdBca->BCAInformation[3]; i++) {
			OutputLog(type, " %02x", dvdBca->BCAInformation[4 + i]);
		}
		OutputLog(type, "\n");
		if (pDisc->SCSI.wCurrentMedia == ProfileDvdRecordable ||
			pDisc->SCSI.wCurrentMedia == ProfileDvdRewritable) {
			OutputLog(type,
				"\t      BCA Record ID: %d\n"
				"\t     Version Number: %d\n"
				"\t        Data Length: %d\n"
				"\tMKB Validation Data:"
				, MAKEWORD(dvdBca->BCAInformation[13], dvdBca->BCAInformation[12])
				, dvdBca->BCAInformation[14], dvdBca->BCAInformation[15]
			);
			for (BYTE i = 0; i < dvdBca->BCAInformation[15]; i++) {
				OutputLog(type, " %02x", dvdBca->BCAInformation[16 + i]);
			}
			OutputLog(type, "\n");
		}
	}
	else {
		OutputMainChannel(type, dvdBca->BCAInformation, NULL, 0, wFormatLength);
	}
}

VOID OutputDVDManufacturerDescriptor(
	PDVD_MANUFACTURER_DESCRIPTOR dvdManufacturer,
	PDISC pDisc,
	LOG_TYPE type
) {
	OutputMainChannel(type, dvdManufacturer->ManufacturingInformation
		, _T("ManufacturingInformation"), 0, sizeof(dvdManufacturer->ManufacturingInformation));
	if (pDisc->DVD.discType != DISC_TYPE_DVD::xboxdvd) {
		if (!strncmp((LPCCH)&dvdManufacturer->ManufacturingInformation[16], "Nintendo Game Disk", 18) ||
			!strncmp((LPCCH)&dvdManufacturer->ManufacturingInformation[16], "Nintendo Emulation Disk", 23)) {
			pDisc->DVD.discType = DISC_TYPE_DVD::gamecube;
		}
		else if (!strncmp((LPCCH)&dvdManufacturer->ManufacturingInformation[16], "Nintendo NNGC Disk", 18) ||
			!strncmp((LPCCH)&dvdManufacturer->ManufacturingInformation[16], "Nintendo NNGC Emu. Disk", 23)) {
			pDisc->DVD.discType = DISC_TYPE_DVD::wii;
		}
		else {
			if (pDisc->DVD.discType == DISC_TYPE_DVD::video && pDisc->SCSI.wCurrentMedia == ProfileDvdRom) {
				for (UINT i = 0; i < sizeof(dvdManufacturer->ManufacturingInformation); i++) {
					if (dvdManufacturer->ManufacturingInformation[i] != 0) {
						// Pursuit of Happyness, the (0 43396 15085 0)
						// Stranger than Fiction (0 43396 15407 0)
						pDisc->DVD.discType = DISC_TYPE_DVD::protect;
						strncpy(pDisc->PROTECT.name[0], "ARccOS", 7);
						pDisc->PROTECT.byExist = arccos;
						break;
					}
				}
			}
		}
	}
}

VOID OutputDVDMediaId(
	LPBYTE lpFormat
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("MediaID")
		"\t           Media Identifier:");
	for (INT i = 0; i < 8; i++) {
		OutputDiscLog(" %02x", lpFormat[i]);
	}
	OutputDiscLog(
		"\n\tMessage Authentication Code:");
	for (INT i = 8; i < 18; i++) {
		OutputDiscLog(" %02x", lpFormat[i]);
	}
	OutputDiscLog(" -> it seems it's always random\n");
}

VOID OutputDVDMediaKeyBlock(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("MediaKeyBlock")
		"\tMedia Key Block Total Packs: %u\n"
		, *(lpFormat - 1));
	OutputDiscLog(
		"\tMessage Authentication Code:");
	for (INT i = 0; i < 10; i++) {
		OutputDiscLog(" %02x", lpFormat[i]);
	}
	OutputDiscLog(
		" -> it seems it's always random\n"
		"\tMedia Key Block\n"
	);
	OutputMainChannel(fileDisc, lpFormat + 16, NULL, 0, (UINT)(wFormatLength  - 16));
}

VOID OutputDiscDefinitionStructure(
	UCHAR version,
	LPBYTE lpFormat
) {
	if (version == 1) {

	}
	else if (version == 6) {
		WORD zonesNum = MAKEWORD(lpFormat[11], lpFormat[10]);
		OutputDiscLog(
			OUTPUT_DHYPHEN_PLUS_STR("DiscDefinitionStructure")
			"\t                        DDS Identifier: %02x%02x\n"
			"\t                    Disk Certification\n"
			"\t                            Formatting: %s\n"
			"\t                           The disk has %s certified by a user\n"
			"\t                           The disk has %s certified by a manufacturer\n"
			"\t                  DDS/PDL Update Count: %u\n"
			"\t                      Number of Groups: %u\n"
			"\t                       Number of Zones: %u\n"
			"\tFirst sector in the Primary spare area: %7u (%#08x)\n"
			"\t Last sector in the Primary spare area: %7u (%#08x)\n"
			"\t                  First logical sector: %7u (%#08x)\n"
			, lpFormat[0], lpFormat[1]
			, (lpFormat[3] & 0x80) == 0x80 ? _T("in process") : _T("has been completed")
			, (lpFormat[3] & 0x10) == 0x10 ? _T("been") : _T("not been")
			, (lpFormat[3] & 0x01) == 0x01 ? _T("been") : _T("not been")
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
			OutputDiscLog("\t                Start LSN for the Zone: %7u (%#x)\n"
				, MAKEUINT(MAKEWORD(lpFormat[i + 3], lpFormat[i + 2]), MAKEWORD(lpFormat[i + 1], lpFormat[i]))
				, MAKEUINT(MAKEWORD(lpFormat[i + 3], lpFormat[i + 2]), MAKEWORD(lpFormat[i + 1], lpFormat[i]))
			);
		}
	}
	else {
		OutputMainChannel(fileDisc, lpFormat, NULL, 0, 2048);
	}
}

VOID OutputDVDRamMediumStatus(
	PDVD_RAM_MEDIUM_STATUS dvdRamMeium
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVDRamMediumStatus")
		"\t              PersistentWriteProtect: %s\n"
		"\t               CartridgeWriteProtect: %s\n"
		"\t           MediaSpecificWriteInhibit: %s\n"
		"\t                  CartridgeNotSealed: %s\n"
		"\t                    MediaInCartridge: %s\n"
		"\t              DiscTypeIdentification: %x\n"
		"\tMediaSpecificWriteInhibitInformation: %s\n"
		, BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->PersistentWriteProtect)
		, BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->CartridgeWriteProtect)
		, BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->MediaSpecificWriteInhibit)
		, BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->CartridgeNotSealed)
		, BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->MediaInCartridge)
		, dvdRamMeium->DiscTypeIdentification
		, BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->MediaSpecificWriteInhibitInformation));
}

VOID OutputDiscSpareAreaInformation(
	PDVD_RAM_SPARE_AREA_INFORMATION dvdRamSpare
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVDRamSpareAreaInformation")
		"\t          FreePrimarySpareSectors: %u\n"
		"\t     FreeSupplementalSpareSectors: %u\n"
		"\tAllocatedSupplementalSpareSectors: %u\n"
		, MAKEUINT(MAKEWORD(dvdRamSpare->FreePrimarySpareSectors[3], dvdRamSpare->FreePrimarySpareSectors[2]),
			MAKEWORD(dvdRamSpare->FreePrimarySpareSectors[1], dvdRamSpare->FreePrimarySpareSectors[0]))
		, MAKEUINT(MAKEWORD(dvdRamSpare->FreeSupplementalSpareSectors[3], dvdRamSpare->FreeSupplementalSpareSectors[2]),
			MAKEWORD(dvdRamSpare->FreeSupplementalSpareSectors[1], dvdRamSpare->FreeSupplementalSpareSectors[0]))
		, MAKEUINT(MAKEWORD(dvdRamSpare->AllocatedSupplementalSpareSectors[3], dvdRamSpare->AllocatedSupplementalSpareSectors[2]),
			MAKEWORD(dvdRamSpare->AllocatedSupplementalSpareSectors[1], dvdRamSpare->AllocatedSupplementalSpareSectors[0])));
}

VOID OutputDVDRamRecordingType(
	PDVD_RAM_RECORDING_TYPE dvdRamRecording
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVDRamRecordingType")
		"\tRealTimeData: %s\n"
		, BOOLEAN_TO_STRING_YES_NO(dvdRamRecording->RealTimeData));
}

VOID OutputDVDRmdLastBorderOut(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("RMD in last border-out"));
	INT nRoop = wFormatLength / DISC_MAIN_DATA_SIZE;
	for (INT i = 0; i < nRoop; i++) {
		OutputMainChannel(fileDisc, lpFormat + DISC_MAIN_DATA_SIZE * i, NULL, 0, DISC_MAIN_DATA_SIZE);
	}
}

VOID OutputDVDRecordingManagementAreaData(
	PDVD_RECORDING_MANAGEMENT_AREA_DATA dvdRecordingMan,
	WORD wFormatLength
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVDRecordingManagementAreaData")
		"\tLastRecordedRMASectorNumber: %u\n"
		"\t                   RMDBytes: \n"
		, MAKEUINT(MAKEWORD(dvdRecordingMan->LastRecordedRMASectorNumber[3]
		, dvdRecordingMan->LastRecordedRMASectorNumber[2])
		, MAKEWORD(dvdRecordingMan->LastRecordedRMASectorNumber[1]
		, dvdRecordingMan->LastRecordedRMASectorNumber[0]))
	);
	OutputMainChannel(fileDisc, dvdRecordingMan->RMDBytes, NULL, 0
		, wFormatLength - sizeof(DVD_RECORDING_MANAGEMENT_AREA_DATA));
}

VOID OutputDVDPreRecordedInformation(
	PDVD_PRERECORDED_INFORMATION dvdPreRecorded
) {
	UINT addr = MAKEUINT(MAKEWORD(dvdPreRecorded->LastAddressOfDataRecordableArea[0]
		, dvdPreRecorded->LastAddressOfDataRecordableArea[1])
		, MAKEWORD(dvdPreRecorded->LastAddressOfDataRecordableArea[2], 0));

	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVDPreRecordedInformation")
		"\t                      FieldID_1: %02x\n"
		"\t            DiscApplicatiowCode: %02x\n"
		"\t               DiscPhysicalCode: %02x\n"
		"\tLastAddressOfDataRecordableArea: %u (%#x)\n"
		"\t                  ExtensiowCode: %02hhx\n"
		"\t                    PartVers1on: %02hhx\n"
		"\t                      FieldID_2: %02x\n"
		"\t               OpcSuggestedCode: %02x\n"
		"\t                 WavelengthCode: %02x\n"
		"\t              WriteStrategyCode: %02x%02x%02x%02x\n"
		"\t                      FieldID_3: %02x\n"
		"\t               ManufacturerId_3: %.6" CHARWIDTH "s\n"
		"\t                      FieldID_4: %02x\n"
		"\t               ManufacturerId_4: %.6" CHARWIDTH "s\n"
		"\t                      FieldID_5: %02x\n"
		"\t               ManufacturerId_5: %.6" CHARWIDTH "s\n"
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
		, &dvdPreRecorded->ManufacturerId_3[0]
		, dvdPreRecorded->FieldID_4
		, &dvdPreRecorded->ManufacturerId_4[0]
		, dvdPreRecorded->FieldID_5
		, &dvdPreRecorded->ManufacturerId_5[0]
	);
}

VOID OutputDVDUniqueDiscIdentifer(
	PDVD_UNIQUE_DISC_IDENTIFIER dvdUnique
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVDUniqueDiscIdentifer")
		"\t RandomNumber: %u\n"
		"\tDate and Time: %.4" CHARWIDTH "s-%.2" CHARWIDTH "s-%.2" CHARWIDTH "sT%.2" CHARWIDTH "s:%.2" CHARWIDTH "s:%.2" CHARWIDTH "s\n"
		, MAKEWORD(dvdUnique->RandomNumber[1], dvdUnique->RandomNumber[0])
		, dvdUnique->Year, dvdUnique->Month, dvdUnique->Day
		, dvdUnique->Hour, dvdUnique->Minute, dvdUnique->Second);
}

VOID OutputDVDCommonInfo(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	for (WORD k = 0; k < wFormatLength; k++) {
		OutputDiscLog("%02x", lpFormat[k]);
	}
	OutputDiscLog("\n");

}

VOID OutputDVDAdipInformation(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("ADIP information"));
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDVDDualLayerRecordingInformation(
	PDVD_DUAL_LAYER_RECORDING_INFORMATION dvdDualLayer
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVDDualLayerRecordingInformation")
		"\tLayer0SectorsImmutable: %s\n"
		"\t         Layer0Sectors: %u\n"
		, BOOLEAN_TO_STRING_YES_NO(dvdDualLayer->Layer0SectorsImmutable)
		, MAKEUINT(MAKEWORD(dvdDualLayer->Layer0Sectors[3], dvdDualLayer->Layer0Sectors[2])
			, MAKEWORD(dvdDualLayer->Layer0Sectors[1], dvdDualLayer->Layer0Sectors[0])));
}

VOID OutputDVDDualLayerMiddleZone(
	PDVD_DUAL_LAYER_MIDDLE_ZONE_START_ADDRESS dvdDualLayerMiddle
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVDDualLayerMiddleZoneStartAddress")
		"\t                   InitStatus: %s\n"
		"\tShiftedMiddleAreaStartAddress: %u\n"
		, BOOLEAN_TO_STRING_YES_NO(dvdDualLayerMiddle->InitStatus)
		, MAKEUINT(MAKEWORD(dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[3],
			dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[2])
		, MAKEWORD(dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[1],
			dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[0])));
}

VOID OutputDVDDualLayerJumpInterval(
	PDVD_DUAL_LAYER_JUMP_INTERVAL_SIZE dvdDualLayerJump
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVDDualLayerJumpIntervalSize")
		"\tJumpIntervalSize: %u\n"
		, MAKEUINT(MAKEWORD(dvdDualLayerJump->JumpIntervalSize[3],
			dvdDualLayerJump->JumpIntervalSize[2])
		, MAKEWORD(dvdDualLayerJump->JumpIntervalSize[1],
			dvdDualLayerJump->JumpIntervalSize[0])));
}

VOID OutputDVDDualLayerManualLayerJump(
	PDVD_DUAL_LAYER_MANUAL_LAYER_JUMP dvdDualLayerMan
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVDDualLayerManualLayerJump")
		"\tManualJumpLayerAddress: %u\n"
		, MAKEUINT(MAKEWORD(dvdDualLayerMan->ManualJumpLayerAddress[3],
			dvdDualLayerMan->ManualJumpLayerAddress[2])
		, MAKEWORD(dvdDualLayerMan->ManualJumpLayerAddress[1],
			dvdDualLayerMan->ManualJumpLayerAddress[0])));
}

VOID OutputDVDDualLayerRemapping(
	PDVD_DUAL_LAYER_REMAPPING_INFORMATION dvdDualLayerRemapping
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVDDualLayerRemappingInformation")
		"\tManualJumpLayerAddress: %u\n"
		, MAKEUINT(MAKEWORD(dvdDualLayerRemapping->RemappingAddress[3],
			dvdDualLayerRemapping->RemappingAddress[2])
		, MAKEWORD(dvdDualLayerRemapping->RemappingAddress[1],
			dvdDualLayerRemapping->RemappingAddress[0])));
}

VOID OutputDVDDiscControlBlockHeader(
	PDVD_DISC_CONTROL_BLOCK_HEADER dvdDiscCtrlBlk
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVDDiscControlBlockHeader")
		"\tContentDescriptor: %u\n"
		"\t           AsByte: %u\n"
		"\t         VendorId: "
		, MAKEUINT(MAKEWORD(dvdDiscCtrlBlk->ContentDescriptor[3],
			dvdDiscCtrlBlk->ContentDescriptor[2])
		, MAKEWORD(dvdDiscCtrlBlk->ContentDescriptor[1],
			dvdDiscCtrlBlk->ContentDescriptor[0]))
		, MAKEUINT(MAKEWORD(dvdDiscCtrlBlk->ProhibitedActions.AsByte[3],
			dvdDiscCtrlBlk->ProhibitedActions.AsByte[2])
		, MAKEWORD(dvdDiscCtrlBlk->ProhibitedActions.AsByte[1],
			dvdDiscCtrlBlk->ProhibitedActions.AsByte[0])));
	for (WORD k = 0; k < sizeof(dvdDiscCtrlBlk->VendorId); k++) {
		OutputDiscLog("%c", dvdDiscCtrlBlk->VendorId[k]);
	}
	OutputDiscLog("\n");
}

VOID OutputDVDDiscControlBlockWriteInhibit(
	PDVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT dvdDiscCtrlBlkWrite
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVDDiscControlBlockWriteInhibit")
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
		OutputDiscLog("%c", dvdDiscCtrlBlkWrite->UpdatePassword[k]);
	}
	OutputDiscLog("\n");
}

VOID OutputDVDDiscControlBlockSession(
	PDVD_DISC_CONTROL_BLOCK_SESSION dvdDiscCtrlBlkSession
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVDDiscControlBlockSession")
		"\tSessionNumber: %u\n"
		"\t       DiscID: \n",
		MAKEWORD(dvdDiscCtrlBlkSession->SessionNumber[1], dvdDiscCtrlBlkSession->SessionNumber[0]));
	for (WORD k = 0; k < sizeof(dvdDiscCtrlBlkSession->DiscID); k++) {
		OutputDiscLog("%c", dvdDiscCtrlBlkSession->DiscID[k]);
	}
	OutputDiscLog("\n");

	for (UINT j = 0; j < sizeof(dvdDiscCtrlBlkSession->SessionItem) / sizeof(DVD_DISC_CONTROL_BLOCK_SESSION_ITEM); j++) {
		OutputDiscLog(
			"\t  SessionItem: %u\n"
			"\t\t     AsByte: ", j);
		for (WORD k = 0; k < sizeof(dvdDiscCtrlBlkSession->SessionItem[j].AsByte); k++) {
			OutputDiscLog("%c", dvdDiscCtrlBlkSession->SessionItem[j].AsByte[k]);
		}
		OutputDiscLog("\n");
	}
}

VOID OutputDVDDiscControlBlockList(
	PDVD_DISC_CONTROL_BLOCK_LIST dvdDiscCtrlBlkList,
	WORD wFormatLength
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVDDiscControlBlockListT")
		"\tReadabldDCBs: %s\n"
		"\tWritableDCBs: %s\n"
		"\tDVD_DISC_CONTROL_BLOCK_LIST_DCB: "
		, BOOLEAN_TO_STRING_YES_NO(dvdDiscCtrlBlkList->ReadabldDCBs)
		, BOOLEAN_TO_STRING_YES_NO(dvdDiscCtrlBlkList->WritableDCBs));

	for (WORD k = 0; k < wFormatLength - sizeof(DVD_DISC_CONTROL_BLOCK_LIST); k++) {
		OutputDiscLog("%u",
			MAKEUINT(MAKEWORD(dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[3], dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[2]),
			MAKEWORD(dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[1], dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[0])));
	}
	OutputDiscLog("\n");

}

VOID OutputDVDMtaEccBlock(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("MTA ECC Block")"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDiscWriteProtectionStatus(
	PDVD_WRITE_PROTECTION_STATUS dvdWrite
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("DiscWriteProtectionStatus")
		"\tSoftwareWriteProtectUntilPowerdown: %s\n"
		"\t       MediaPersistentWriteProtect: %s\n"
		"\t             CartridgeWriteProtect: %s\n"
		"\t         MediaSpecificWriteProtect: %s\n"
		, BOOLEAN_TO_STRING_YES_NO(dvdWrite->SoftwareWriteProtectUntilPowerdown)
		, BOOLEAN_TO_STRING_YES_NO(dvdWrite->MediaPersistentWriteProtect)
		, BOOLEAN_TO_STRING_YES_NO(dvdWrite->CartridgeWriteProtect)
		, BOOLEAN_TO_STRING_YES_NO(dvdWrite->MediaSpecificWriteProtect));
}

VOID OutputDiscAACSVolumeIdentifier(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("AACS Volume Identifiers")"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDiscPreRecordedAACSMediaSerialNumber(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("PreRecorded AACS Media Serial Number")"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDiscAACSMediaIdentifier(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("AACS Media Identifier")"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDiscAACSMediaKeyBlock(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("AACS Media Key Block")"\t");
	OutputDVDCommonInfo(lpFormat, wFormatLength);
}

VOID OutputDiscListOfRecognizedFormatLayers(
	PDVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS_TYPE_CODE dvdListOf
) {
	OutputDiscLog(
		"\t\tNumberOfRecognizedFormatLayers: %u\n"
		"\t\t             OnlineFormatlayer: %hhu\n"
		"\t\t            DefaultFormatLayer: %hhu\n"
		, dvdListOf->NumberOfRecognizedFormatLayers
		, dvdListOf->OnlineFormatlayer
		, dvdListOf->DefaultFormatLayer);
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
		if (pDisc->SCSI.wCurrentMedia == ProfileDvdRam || pDisc->SCSI.wCurrentMedia == ProfileHDDVDRam) {
			OutputDVDRamLayerDescriptor(pDisc, lpFormat);
		}
		else if (pDisc->SCSI.wCurrentMedia == ProfileDvdPlusR) {
			OutputDVDPlusRLayerDescriptor(lpFormat);
		}
		else {
			OutputDVDLayerDescriptor(pDisc, (PDVD_FULL_LAYER_DESCRIPTOR)lpFormat, lpdwSectorLength, layerNumber, fileDisc);
		}
		break;
	case DvdCopyrightDescriptor:
		OutputDVDCopyrightDescriptor((PDVD_COPYRIGHT_DESCRIPTOR)lpFormat, &(pDisc->DVD.protect), fileDisc);
		break;
	case DvdDiskKeyDescriptor:
		OutputDVDDiskKeyDescriptor((PDVD_DISK_KEY_DESCRIPTOR)lpFormat);
		break;
	case DvdBCADescriptor:
		OutputDiscBCADescriptor(pDisc, (PDVD_BCA_DESCRIPTOR)lpFormat, wFormatLength, fileDisc);
		break;
	case DvdManufacturerDescriptor:
		OutputDVDManufacturerDescriptor((PDVD_MANUFACTURER_DESCRIPTOR)lpFormat, pDisc, fileDisc);
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
		OutputDVDRamMediumStatus((PDVD_RAM_MEDIUM_STATUS)lpFormat);
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
	case 0x10:
		OutputDVDMinusRLayerDescriptor(lpFormat);
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
		OutputDiscLog("\tUnknown: %02x\n", byFormatCode);
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
				OutputDiscWithLBALog("This sector is scrambled by CSS", nLBA);
				break;
			case 0x01:
				OutputDiscWithLBALog("This sector is encrypted by CPPM", nLBA);
				break;
			default:
				OutputDiscWithLBALog("reserved", nLBA);
			}
		}
		else {
			OutputDiscWithLBALog("CSS or CPPM doesn't exist in this sector", nLBA);
		}

		switch (dvdCopyright->CPR_MAI & 0x30) {
		case 0:
			OutputDiscLog(", copying is permitted without restriction\n");
			break;
		case 0x10:
			OutputDiscLog(", reserved\n");
			break;
		case 0x20:
			OutputDiscLog(", one generation of copies may be made\n");
			break;
		case 0x30:
			OutputDiscLog(", no copying is permitted\n");
			break;
		default:
			OutputDiscLog("\n");
		}
	}
	else {
		OutputDiscWithLBALog("No protected sector\n", nLBA);
	}
}

VOID OutputBDDiscInformation(
	PDISC pDisc,
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("DiscInformationFromPIC"));
	BOOL bBdRom = TRUE;
	INT nSize = 2048;
	INT nBlock = 64;
	if (strncmp((CONST CHAR*)&lpFormat[8], "BDO", 3)) {
		bBdRom = FALSE;
		nSize = 3584;
		nBlock = 112;
	}
	UINT uiEndLogicalSector[4] = {};
	UINT uiStartPhysicalSector[4] = {};
	UINT uiEndPhysicalSector[4] = {};
	INT n = 0;
	INT nDIFormat = 0;
	INT nLayerNum = 0;

	// JIS X 6230:2017 http://kikakurui.com/x6/X6230-2017-01.html BD-R SL, DL 
	// JIS X 6231:2017 http://kikakurui.com/x6/X6231-2017-01.html BD-R TL, QL
	// JIS X 6232:2017 http://kikakurui.com/x6/X6232-2017-01.html BD-RE SL, DL
	// JIS X 6233:2017 http://kikakurui.com/x6/X6233-2017-01.html BD-RE TL
	for (INT i = 0; i < nSize; i += nBlock) {
		if (lpFormat[0 + i] == 0) {
			break;
		}
		nDIFormat = lpFormat[2 + i] & 0x7f;
		nLayerNum = lpFormat[3 + i] & 0x07;
		OutputDiscLog(
			"\tDiscInformationUnits\n"
			"\t             DiscInformationIdentifier: %.2" CHARWIDTH "s\n"
			"\t                               BCACode: %s\n"
			"\t                 DiscInformationFormat: %02d\n"
			"\t          NumberOfDIUnitsInEachDIBlock: %02d\n"
			"\tNumberOfLayersToWhichThisDIUnitApplies: %02d\n"
			"\t                      DiscTypeSpecific: %02d\n"
			"\t                  DIUnitSequenceNumber: %02d\n"
			"\t                      ContinuationFlag: %s\n"
			"\t        NumberOfBytesInUseInThisDIUnit: %02d\n"
			"\t                    DiscTypeIdentifier: %.3" CHARWIDTH "s\n"
			"\t                              DiscSize: %s\n"
			"\t                                 Class: %02d\n"
			"\t                               Version: %02d\n"
			"\t         DIUnitFormatDependentContents\n"
			, &lpFormat[0 + i], lpFormat[2 + i] >> 7 == 0 ? _T("Yes") : _T("No")
			, nDIFormat, lpFormat[3 + i] >> 3
			, nLayerNum, lpFormat[4 + i], lpFormat[5 + i]
			, lpFormat[6 + i] >> 7 == 0 ? _T("No") : _T("Yes"), lpFormat[6 + i] & 0x7f
			, &lpFormat[8 + i], lpFormat[11 + i] >> 6 == 0 ? _T("120 mm") : _T("80 mm")
			, (lpFormat[11 + i] >> 4) & 0x02, lpFormat[11 + i] & 0x03
		);
		INT nLayerType = lpFormat[12 + i] & 0x0f;
		OutputDiscLog(
			"\t\t                    NumberOfLayers: %02d\n"
			"\t\t                         LayerType: %02d "
			, lpFormat[12 + i] >> 4, nLayerType
		);
		switch (nLayerType) {
		case 1:
			OutputDiscLog("(ReadOnly)\n");
			break;
		case 2:
			OutputDiscLog("(Writable)\n");
			break;
		case 4:
			OutputDiscLog("(Rewritable)\n");
			break;
		default:
			OutputDiscLog("(Unknown)\n");
			break;
		}
		INT nChannelBit = lpFormat[13 + i] & 0x0f;
		OutputDiscLog(
			"\t\t                        ChannelBit: %02d ", nChannelBit);
		switch (nChannelBit) {
		case 1:
			OutputDiscLog("(74.5 nm)\n");
			break;
		case 4:
			OutputDiscLog("(58.26 nm)\n");
			break;
		case 5:
			OutputDiscLog("(55.87 nm)\n");
			break;
		default:
			OutputDiscLog("(Reserved)\n");
			break;
		}
		pDisc->DVD.ucBca = (UCHAR)(lpFormat[16 + i] & 0x0f);
		OutputDiscLog(
			"\t\t             Push-pullPolarityFlag: %02d\n"
			"\t\t            RecordMarkPolarityFlag: %02d\n"
			"\t\t                     BCADescriptor: %s\n"
			"\t\t               MaximumTransferRate: %02d\n"
			, lpFormat[14 + i], lpFormat[15 + i]
			, pDisc->DVD.ucBca == 0 ? _T("No") : _T("Exist"), lpFormat[17 + i]
		);
		uiEndLogicalSector[n] = MAKEUINT(MAKEWORD(lpFormat[23 + i], lpFormat[22 + i]), MAKEWORD(lpFormat[21 + i], lpFormat[20 + i]));
		uiStartPhysicalSector[n] = MAKEUINT(MAKEWORD(lpFormat[27 + i], lpFormat[26 + i]), MAKEWORD(lpFormat[25 + i], lpFormat[24 + i]));
		uiEndPhysicalSector[n] = MAKEUINT(MAKEWORD(lpFormat[31 + i], lpFormat[30 + i]), MAKEWORD(lpFormat[29 + i], lpFormat[28 + i]));

		if (bBdRom) {
			OutputDiscLog(
				"\t\t            EndLogicalSectorNumber: %08x\n"
				"\t\t         StartPhysicalSectorNumber: %08x\n"
				"\t\t           EndPhysicalSectorNumber: %08x\n"
				, uiEndLogicalSector[n], uiStartPhysicalSector[n], uiEndPhysicalSector[n]
			);
			n++;
		}
		else {
			OutputDiscLog(
				"\t\t                          PhysicalADIPAddress: %08x\n"
				"\t\t                              LastADIPAddress: %08x\n"
				"\t\t                        NominalRecordingSpeed: %.2f m/s\n"
				"\t\t                            MaxRecordingSpeed: %s\n"
				"\t\t                            MinRecordingSpeed: %s\n"
				"\t\t             MaxDCReadingPowerOfStandardSpeed: %02x\n"
				"\t\t     MaxDCReadingPowerOfNominalRecordingSpeed: %06x\n"
				"\t\t       MaxRFPiledReadingPowerOfReferenceSpeed: %02x\n"
				"\t\tMaxRFPiledReadingPowerOfNominalRecordingSpeed: %06x\n"
				, uiStartPhysicalSector[n], uiEndPhysicalSector[n], (DOUBLE)(MAKEWORD(lpFormat[33 + i], lpFormat[32 + i])) / 100
				, lpFormat[34 + i] == 0x64 ? _T("Same As above") : _T("Other")
				, lpFormat[35 + i] == 0x64 ? _T("Same As above") : _T("Other")
				, lpFormat[36 + i], MAKEUINT(MAKEWORD(lpFormat[39 + i], lpFormat[38 + i]), MAKEWORD(lpFormat[37 + i], 0))
				, lpFormat[40 + i], MAKEUINT(MAKEWORD(lpFormat[43 + i], lpFormat[42 + i]), MAKEWORD(lpFormat[41 + i], 0))
			);
			if (!strncmp((CONST CHAR*)&lpFormat[8], "BDR", 3)) {
				OutputDiscLog(
					"\t\t                                         PIND: %02x\n"
					"\t\t                                         mIND: %02x\n"
					"\t\t                                            p: %02x\n"
					"\t\t                                          eBW: %02x\n"
					"\t\t                                           ec: %02x\n"
					"\t\t                                           es: %02x\n"
					"\t\t                                            k: %02x\n"
					"\t\t                                            b: %02x\n"
					, lpFormat[48 + i], lpFormat[49 + i], lpFormat[50 + i], lpFormat[51 + i]
					, lpFormat[52 + i], lpFormat[53 + i], lpFormat[54 + i], lpFormat[55 + i]
				);
				if (nDIFormat == 1) {
					OutputDiscLog(
						"\t\t                                          TMP: %02x\n"
						"\t\t                       dTtopOfRunLength5Tto4T: %02x\n"
						"\t\t                       dTtopOfRunLength4Tto4T: %02x\n"
						"\t\t                       dTtopOfRunLength3Tto4T: %02x\n"
						"\t\t                       dTtopOfRunLength2Tto4T: %02x\n"
						"\t\t                       dTtopOfRunLength5Tto3T: %02x\n"
						"\t\t                       dTtopOfRunLength4Tto3T: %02x\n"
						"\t\t                       dTtopOfRunLength3Tto3T: %02x\n"
						"\t\t                       dTtopOfRunLength2Tto3T: %02x\n"
						"\t\t                       dTtopOfRunLength5Tto2T: %02x\n"
						"\t\t                       dTtopOfRunLength4Tto2T: %02x\n"
						"\t\t                       dTtopOfRunLength3Tto2T: %02x\n"
						"\t\t                       dTtopOfRunLength2Tto2T: %02x\n"
						"\t\t                        TtopOfRunLength5Tto4T: %04x\n"
						"\t\t                        TtopOfRunLength4Tto4T: %04x\n"
						"\t\t                        TtopOfRunLength3Tto4T: %04x\n"
						"\t\t                        TtopOfRunLength2Tto4T: %04x\n"
						"\t\t                        TtopOfRunLength5Tto3T: %04x\n"
						"\t\t                        TtopOfRunLength4Tto3T: %04x\n"
						"\t\t                        TtopOfRunLength3Tto3T: %04x\n"
						"\t\t                        TtopOfRunLength2Tto3T: %04x\n"
						"\t\t                        TtopOfRunLength5Tto2T: %04x\n"
						"\t\t                        TtopOfRunLength4Tto2T: %04x\n"
						"\t\t                        TtopOfRunLength3Tto2T: %04x\n"
						"\t\t                        TtopOfRunLength2Tto2T: %04x\n"
						"\t\t                             TLPOfRunLength4T: %02x\n"
						"\t\t                             TLPOfRunLength3T: %02x\n"
						"\t\t                        dTSOfRunLength4TtoN-1: %02x\n"
						"\t\t                        dTSOfRunLength3TtoN-1: %02x\n"
						"\t\t                        dTSOfRunLength2TtoN-1: %02x\n"
						, lpFormat[56 + i], lpFormat[57 + i], lpFormat[58 + i], lpFormat[59 + i], lpFormat[60 + i]
						, lpFormat[61 + i], lpFormat[62 + i], lpFormat[63 + i], lpFormat[64 + i]
						, lpFormat[65 + i], lpFormat[66 + i], lpFormat[67 + i], lpFormat[68 + i]
						, MAKEWORD(lpFormat[70 + i], lpFormat[69 + i]), MAKEWORD(lpFormat[72 + i], lpFormat[71 + i])
						, MAKEWORD(lpFormat[74 + i], lpFormat[73 + i]), MAKEWORD(lpFormat[76 + i], lpFormat[75 + i])
						, MAKEWORD(lpFormat[78 + i], lpFormat[77 + i]), MAKEWORD(lpFormat[80 + i], lpFormat[79 + i])
						, MAKEWORD(lpFormat[82 + i], lpFormat[81 + i]), MAKEWORD(lpFormat[84 + i], lpFormat[83 + i])
						, MAKEWORD(lpFormat[86 + i], lpFormat[85 + i]), MAKEWORD(lpFormat[88 + i], lpFormat[87 + i])
						, MAKEWORD(lpFormat[90 + i], lpFormat[89 + i]), MAKEWORD(lpFormat[92 + i], lpFormat[91 + i])
						, lpFormat[93 + i], lpFormat[94 + i], lpFormat[95 + i], lpFormat[96 + i], lpFormat[97 + i]
					);
				}
				else if (nDIFormat == 2) {
					OutputDiscLog(
						"\t\t                                          TMP: %04x\n"
						"\t\t                       dTtopOfRunLength5,7,9T: %02x\n"
						"\t\t                       dTtopOfRunLength4,6,8T: %02x\n"
						"\t\t                           dTtopOfRunLength3T: %02x\n"
						"\t\t                           dTtopOfRunLength2T: %02x\n"
						"\t\t                        TtopOfRunLength5,7,9T: %04x\n"
						"\t\t                        TtopOfRunLength4,6,8T: %04x\n"
						"\t\t                            TtopOfRunLength3T: %04x\n"
						"\t\t                            TtopOfRunLength2T: %04x\n"
						"\t\t                         TLPOfRunLength5,7,9T: %04x\n"
						"\t\t                         TLPOfRunLength4,6,8T: %04x\n"
						"\t\t                         dTSOfRunLength5,7,9T: %02x\n"
						"\t\t                         dTSOfRunLength4,6,8T: %02x\n"
						"\t\t                             dTSOfRunLength3T: %02x\n"
						"\t\t                             dTSOfRunLength2T: %02x\n"
						, MAKEWORD(lpFormat[57 + i], lpFormat[56 + i])
						, lpFormat[58 + i], lpFormat[59 + i], lpFormat[60 + i], lpFormat[61 + i]
						, MAKEWORD(lpFormat[63 + i], lpFormat[62 + i]), MAKEWORD(lpFormat[65 + i], lpFormat[64 + i])
						, MAKEWORD(lpFormat[67 + i], lpFormat[66 + i]), MAKEWORD(lpFormat[69 + i], lpFormat[68 + i])
						, MAKEWORD(lpFormat[71 + i], lpFormat[70 + i]), MAKEWORD(lpFormat[73 + i], lpFormat[72 + i])
						, lpFormat[74 + i], lpFormat[75 + i], lpFormat[76 + i], lpFormat[77 + i]
					);
				}
				else if (nDIFormat == 3) {
					OutputDiscLog(
						"\t\t                             dTCOfRunLength5T: %02x\n"
						"\t\t                             dTCOfRunLength4T: %02x\n"
						"\t\t                             dTCOfRunLength3T: %02x\n"
						"\t\t                       dTtopOfRunLength4Tto4T: %02x\n"
						"\t\t                       dTtopOfRunLength3Tto4T: %02x\n"
						"\t\t                       dTtopOfRunLength2Tto4T: %02x\n"
						"\t\t                       dTtopOfRunLength4Tto3T: %02x\n"
						"\t\t                       dTtopOfRunLength3Tto3T: %02x\n"
						"\t\t                       dTtopOfRunLength2Tto3T: %02x\n"
						"\t\t                       dTtopOfRunLength4Tto2T: %02x\n"
						"\t\t                       dTtopOfRunLength3Tto2T: %02x\n"
						"\t\t                       dTtopOfRunLength2Tto2T: %02x\n"
						"\t\t                        TtopOfRunLength4Tto4T: %02x\n"
						"\t\t                        TtopOfRunLength3Tto4T: %02x\n"
						"\t\t                        TtopOfRunLength2Tto4T: %02x\n"
						"\t\t                        TtopOfRunLength4Tto3T: %02x\n"
						"\t\t                        TtopOfRunLength3Tto3T: %02x\n"
						"\t\t                        TtopOfRunLength2Tto3T: %02x\n"
						"\t\t                        TtopOfRunLength4Tto2T: %02x\n"
						"\t\t                        TtopOfRunLength3Tto2T: %02x\n"
						"\t\t                        TtopOfRunLength2Tto2T: %02x\n"
						"\t\t                             TLPOfRunLength4T: %02x\n"
						"\t\t                             dTSOfRunLength4T: %02x\n"
						"\t\t                             dTSOfRunLength3T: %02x\n"
						"\t\t                             dTSOfRunLength2T: %02x\n"
						, lpFormat[56 + i], lpFormat[57 + i], lpFormat[58 + i], lpFormat[59 + i], lpFormat[60 + i]
						, lpFormat[61 + i], lpFormat[62 + i], lpFormat[63 + i], lpFormat[64 + i], lpFormat[65 + i]
						, lpFormat[66 + i], lpFormat[67 + i], lpFormat[68 + i], lpFormat[69 + i], lpFormat[70 + i]
						, lpFormat[71 + i], lpFormat[72 + i], lpFormat[73 + i], lpFormat[74 + i], lpFormat[75 + i]
						, lpFormat[76 + i], lpFormat[77 + i], lpFormat[78 + i], lpFormat[79 + i], lpFormat[80 + i]
					);
				}
			}
			else if (!strncmp((CONST CHAR*)&lpFormat[8], "BDW", 3)) {
				if (nDIFormat == 1) {
					OutputDiscLog(
						"\t\t                  PINDOfNominalRecordingSpeed: %02x\n"
						"\t\t                  mINDOfNominalRecordingSpeed: %02x\n"
						"\t\t                     pOfNominalRecordingSpeed: %02x\n"
						"\t\t                   eBWOfNominalRecordingSpeed: %02x\n"
						"\t\t                    ecOfNominalRecordingSpeed: %02x\n"
						"\t\t                   eE1OfNominalRecordingSpeed: %02x\n"
						"\t\t                   eE2OfNominalRecordingSpeed: %02x\n"
						"\t\t                     kOfNominalRecordingSpeed: %02x\n"
						"\t\t                      PINDOfMaxRecordingSpeed: %02x\n"
						"\t\t                      mINDOfMaxRecordingSpeed: %02x\n"
						"\t\t                         pOfMaxRecordingSpeed: %02x\n"
						"\t\t                       eBWOfMaxRecordingSpeed: %02x\n"
						"\t\t                        ecOfMaxRecordingSpeed: %02x\n"
						"\t\t                       eE1OfMaxRecordingSpeed: %02x\n"
						"\t\t                       eE2OfMaxRecordingSpeed: %02x\n"
						"\t\t                         kOfMaxRecordingSpeed: %02x\n"
						"\t\t                      PINDOfMinRecordingSpeed: %02x\n"
						"\t\t                      mINDOfMinRecordingSpeed: %02x\n"
						"\t\t                         pOfMinRecordingSpeed: %02x\n"
						"\t\t                       eBWOfMinRecordingSpeed: %02x\n"
						"\t\t                        ecOfMinRecordingSpeed: %02x\n"
						"\t\t                       eE1OfMaxRecordingSpeed: %02x\n"
						"\t\t                       eE2OfMaxRecordingSpeed: %02x\n"
						"\t\t                         kOfMinRecordingSpeed: %02x\n"
						"\t\t                                          TMP: %02x\n"
						"\t\t                            TtopOfRunLength4T: %02x\n"
						"\t\t                            TtopOfRunLength3T: %02x\n"
						"\t\t                            TtopOfRunLength2T: %02x\n"
						"\t\t    dTtopOfRunLength4TOfNominalRecordingSpeed: %02x\n"
						"\t\t    dTtopOfRunLength3TOfNominalRecordingSpeed: %02x\n"
						"\t\t    dTtopOfRunLength2TOfNominalRecordingSpeed: %02x\n"
						"\t\t        dTtopOfRunLength4TOfMaxRecordingSpeed: %02x\n"
						"\t\t        dTtopOfRunLength3TOfMaxRecordingSpeed: %02x\n"
						"\t\t        dTtopOfRunLength2TOfMaxRecordingSpeed: %02x\n"
						"\t\t        dTtopOfRunLength4TOfMinRecordingSpeed: %02x\n"
						"\t\t        dTtopOfRunLength3TOfMinRecordingSpeed: %02x\n"
						"\t\t        dTtopOfRunLength2TOfMinRecordingSpeed: %02x\n"
						"\t\t                                           TE: %02x\n"
						"\t\t      dTEOfRunLength4TOfNominalRecordingSpeed: %02x\n"
						"\t\t      dTEOfRunLength3TOfNominalRecordingSpeed: %02x\n"
						"\t\t      dTEOfRunLength2TOfNominalRecordingSpeed: %02x\n"
						"\t\t          dTEOfRunLength4TOfMaxRecordingSpeed: %02x\n"
						"\t\t          dTEOfRunLength3TOfMaxRecordingSpeed: %02x\n"
						"\t\t          dTEOfRunLength2TOfMaxRecordingSpeed: %02x\n"
						"\t\t          dTEOfRunLength4TOfMinRecordingSpeed: %02x\n"
						"\t\t          dTEOfRunLength3TOfMinRecordingSpeed: %02x\n"
						"\t\t          dTEOfRunLength2TOfMinRecordingSpeed: %02x\n"
						"\t\t                                   DeleteFlag: %02x\n"
						, lpFormat[48 + i], lpFormat[49 + i], lpFormat[50 + i], lpFormat[51 + i]
						, lpFormat[52 + i], lpFormat[53 + i], lpFormat[54 + i], lpFormat[55 + i]
						, lpFormat[56 + i], lpFormat[57 + i], lpFormat[58 + i], lpFormat[59 + i]
						, lpFormat[60 + i], lpFormat[61 + i], lpFormat[62 + i], lpFormat[63 + i]
						, lpFormat[64 + i], lpFormat[65 + i], lpFormat[66 + i], lpFormat[67 + i]
						, lpFormat[68 + i], lpFormat[69 + i], lpFormat[70 + i], lpFormat[71 + i]
						, lpFormat[72 + i], lpFormat[73 + i], lpFormat[74 + i], lpFormat[75 + i]
						, lpFormat[76 + i], lpFormat[77 + i], lpFormat[78 + i], lpFormat[79 + i]
						, lpFormat[80 + i], lpFormat[81 + i], lpFormat[82 + i], lpFormat[83 + i]
						, lpFormat[84 + i], lpFormat[88 + i], lpFormat[89 + i], lpFormat[90 + i]
						, lpFormat[91 + i], lpFormat[92 + i], lpFormat[93 + i], lpFormat[94 + i]
						, lpFormat[95 + i], lpFormat[96 + i], lpFormat[97 + i], lpFormat[98 + i]
					);
				}
				else if (nDIFormat == 2) {
					OutputDiscLog(
						"\t\t                                         PIND: %02x\n"
						"\t\t                                         mIND: %02x\n"
						"\t\t                                            p: %02x\n"
						"\t\t                                          eBW: %02x\n"
						"\t\t                                           ec: %02x\n"
						"\t\t                                           es: %02x\n"
						"\t\t                                            k: %02x\n"
						"\t\t                                          TMP: %02x\n"
						"\t\t                       dTtopOfRunLength5Tto4T: %02x\n"
						"\t\t                       dTtopOfRunLength4Tto4T: %02x\n"
						"\t\t                       dTtopOfRunLength3Tto4T: %02x\n"
						"\t\t                       dTtopOfRunLength2Tto4T: %02x\n"
						"\t\t                       dTtopOfRunLength5Tto3T: %02x\n"
						"\t\t                       dTtopOfRunLength4Tto3T: %02x\n"
						"\t\t                       dTtopOfRunLength3Tto3T: %02x\n"
						"\t\t                       dTtopOfRunLength2Tto3T: %02x\n"
						"\t\t                       dTtopOfRunLength5Tto2T: %02x\n"
						"\t\t                       dTtopOfRunLength4Tto2T: %02x\n"
						"\t\t                       dTtopOfRunLength3Tto2T: %02x\n"
						"\t\t                       dTtopOfRunLength2Tto2T: %02x\n"
						"\t\t                        TtopOfRunLength5Tto4T: %04x\n"
						"\t\t                        TtopOfRunLength4Tto4T: %04x\n"
						"\t\t                        TtopOfRunLength3Tto4T: %04x\n"
						"\t\t                        TtopOfRunLength2Tto4T: %04x\n"
						"\t\t                        TtopOfRunLength5Tto3T: %04x\n"
						"\t\t                        TtopOfRunLength4Tto3T: %04x\n"
						"\t\t                        TtopOfRunLength3Tto3T: %04x\n"
						"\t\t                        TtopOfRunLength2Tto3T: %04x\n"
						"\t\t                        TtopOfRunLength5Tto2T: %04x\n"
						"\t\t                        TtopOfRunLength4Tto2T: %04x\n"
						"\t\t                        TtopOfRunLength3Tto2T: %04x\n"
						"\t\t                        TtopOfRunLength2Tto2T: %04x\n"
						"\t\t                             TLPOfRunLength4T: %02x\n"
						"\t\t                             TLPOfRunLength3T: %02x\n"
						"\t\t                             dTEOfRunLength4T: %02x\n"
						"\t\t                             dTEOfRunLength3T: %02x\n"
						"\t\t                             dTEOfRunLength2T: %02x\n"
						, lpFormat[48 + i], lpFormat[49 + i], lpFormat[50 + i], lpFormat[51 + i]
						, lpFormat[52 + i], lpFormat[53 + i], lpFormat[54 + i]
						, lpFormat[56 + i], lpFormat[57 + i], lpFormat[58 + i], lpFormat[59 + i]
						, lpFormat[60 + i], lpFormat[61 + i], lpFormat[62 + i], lpFormat[63 + i]
						, lpFormat[64 + i], lpFormat[65 + i], lpFormat[66 + i], lpFormat[67 + i], lpFormat[68 + i]
						, MAKEWORD(lpFormat[70 + i], lpFormat[69 + i]), MAKEWORD(lpFormat[72 + i], lpFormat[71 + i])
						, MAKEWORD(lpFormat[74 + i], lpFormat[73 + i]), MAKEWORD(lpFormat[76 + i], lpFormat[75 + i])
						, MAKEWORD(lpFormat[78 + i], lpFormat[77 + i]), MAKEWORD(lpFormat[80 + i], lpFormat[79 + i])
						, MAKEWORD(lpFormat[82 + i], lpFormat[81 + i]), MAKEWORD(lpFormat[84 + i], lpFormat[83 + i])
						, MAKEWORD(lpFormat[86 + i], lpFormat[85 + i]), MAKEWORD(lpFormat[88 + i], lpFormat[87 + i])
						, MAKEWORD(lpFormat[90 + i], lpFormat[89 + i]), MAKEWORD(lpFormat[92 + i], lpFormat[91 + i])
						, lpFormat[93 + i], lpFormat[94 + i], lpFormat[95 + i], lpFormat[96 + i], lpFormat[96 + i]
					);
				}
				else if (nDIFormat == 3) {
					OutputDiscLog(
						"\t\t                                         PIND: %02x\n"
						"\t\t                                         mIND: %02x\n"
						"\t\t                                            p: %02x\n"
						"\t\t                                          eBW: %02x\n"
						"\t\t                                           ec: %02x\n"
						"\t\t                                           es: %02x\n"
						"\t\t                                            k: %02x\n"
						"\t\t                                          TMP: %02x%02x\n"
						"\t\t                       dTtopOfRunLength5,7,9T: %02x\n"
						"\t\t                       dTtopOfRunLength4,6,8T: %02x\n"
						"\t\t                           dTtopOfRunLength3T: %02x\n"
						"\t\t                           dTtopOfRunLength2T: %02x\n"
						"\t\t                        TtopOfRunLength5,7,9T: %04x\n"
						"\t\t                        TtopOfRunLength4,6,8T: %04x\n"
						"\t\t                            TtopOfRunLength3T: %04x\n"
						"\t\t                            TtopOfRunLength2T: %04x\n"
						"\t\t                         TLPOfRunLength5,7,9T: %04x\n"
						"\t\t                         TLPOfRunLength4,6,8T: %04x\n"
						"\t\t                         dTEOfRunLength5,7,9T: %02x\n"
						"\t\t                         dTEOfRunLength4,6,8T: %02x\n"
						"\t\t                             dTEOfRunLength3T: %04x\n"
						"\t\t                             dTEOfRunLength2T: %04x\n"
						, lpFormat[48 + i], lpFormat[49 + i], lpFormat[50 + i], lpFormat[51 + i]
						, lpFormat[52 + i], lpFormat[53 + i], lpFormat[54 + i], lpFormat[56 + i], lpFormat[57 + i]
						, lpFormat[58 + i], lpFormat[59 + i], lpFormat[60 + i], lpFormat[61 + i]
						, MAKEWORD(lpFormat[63 + i], lpFormat[62 + i]), MAKEWORD(lpFormat[65 + i], lpFormat[64 + i])
						, MAKEWORD(lpFormat[67 + i], lpFormat[66 + i]), MAKEWORD(lpFormat[69 + i], lpFormat[68 + i])
						, MAKEWORD(lpFormat[71 + i], lpFormat[70 + i]), MAKEWORD(lpFormat[73 + i], lpFormat[72 + i])
						, lpFormat[74 + i], lpFormat[75 + i], lpFormat[76 + i], lpFormat[77 + i]
					);
				}
			}
			INT year = MAKEWORD(lpFormat[110 + i] & 0xf0, lpFormat[109 + i]) >> 4;
			INT month = lpFormat[110 + i] & 0x0f;
			OutputDiscLog(
				"\t                    DiscManufacturerID: %.6" CHARWIDTH "s\n"
				"\t                           MediaTypeID: %.3" CHARWIDTH "s\n"
				"\t                             TimeStamp: %02d%02d\n"
				"\t                 ProductRevisionNumber: %02x\n"
				, &lpFormat[100 + i], &lpFormat[106 + i], year, month, lpFormat[111 + i]
			);
		}
		OutputDiscLog("\n");
	}
	OutputDiscLog("\tEmergencyBrakeData\n"
		"\t             EmergencyBrakeIdentifier: %.2" CHARWIDTH "s\n"
		"\t                               EBData: "
		, &lpFormat[nSize]
	);
	for (WORD k = 0; k < wFormatLength - nSize - 4; k++) {
		OutputDiscLog("%02x", lpFormat[nSize + 2 + k]);
	}
	OutputDiscLog("\n");

	if (bBdRom) {
		UINT uiEndLayerZeroSectorLen = uiEndPhysicalSector[0] - uiStartPhysicalSector[0] + 2;
		if (nLayerNum == 0) {
			OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("SectorLength")
				"\t LayerZeroSector: %8u (%#x)\n"
				, uiEndLayerZeroSectorLen, uiEndLayerZeroSectorLen
			);
		}
		else if (nLayerNum == 1) {
			UINT uiEndLayerOneSectorLen = uiEndLogicalSector[1] - uiStartPhysicalSector[1] + 1;
			UINT uiLayerAllSectorLen = uiEndLayerZeroSectorLen + uiEndLayerOneSectorLen;

			OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("SectorLength")
				"\t LayerZeroSector: %8u (%#x)\n"
				"\t+ LayerOneSector: %8u (%#x)\n"
				"\t--------------------------------------\n"
				"\t  LayerAllSector: %8u (%#x)\n"
				, uiEndLayerZeroSectorLen, uiEndLayerZeroSectorLen
				, uiEndLayerOneSectorLen, uiEndLayerOneSectorLen
				, uiLayerAllSectorLen, uiLayerAllSectorLen
			);
		}
		// TODO: Layer 3, 4
	}
}

VOID OutputCartridgeStatus(
	LPBYTE lpFormat
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("CartridgeStatus")
		"\t               CartridgeWriteProtect: %s\n"
		"\t                  CartridgeNotSealed: %s\n"
		"\t                    MediaInCartridge: %s\n"
		, BOOLEAN_TO_STRING_YES_NO((lpFormat[0] & 0x04) == 0x04)
		, BOOLEAN_TO_STRING_YES_NO((lpFormat[0] & 0x40) == 0x40)
		, BOOLEAN_TO_STRING_YES_NO((lpFormat[0] & 0x80) == 0x80)
	);
}

VOID OutputBDRawDefectList(
	LPBYTE lpFormat,
	WORD wFormatLength
) {
	UNREFERENCED_PARAMETER(wFormatLength);
	UINT Entries = MAKEUINT(MAKEWORD(lpFormat[15], lpFormat[14]), MAKEWORD(lpFormat[13], lpFormat[12]));
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("RawDefectList")
		"\t       DefectListIdentifier: %.2" CHARWIDTH "s\n"
		"\t           DefectListFormat: %02x\n"
		"\t      DefectListUpdateCount: %02x\n"
		"\t  NumberOfDefectListEntries: %02x\n"
		"\tDiscTypeSpecificInformation: "
		, &lpFormat[0], lpFormat[2]
		, MAKEUINT(MAKEWORD(lpFormat[7], lpFormat[6]), MAKEWORD(lpFormat[5], lpFormat[4]))
		, Entries
	);
	for (WORD k = 0; k < 48; k++) {
		OutputDiscLog("%02x ", lpFormat[16 + k]);
	}
	OutputDiscLog("\n\t              DefectEntries: ");
	for (UINT k = 0; k < Entries; k += 8) {
		OutputDiscLog("%02x%02x%02x%02x%02x%02x%02x%02x "
			, lpFormat[64 + k], lpFormat[65 + k], lpFormat[66 + k], lpFormat[67 + k]
			, lpFormat[68 + k], lpFormat[69 + k], lpFormat[70 + k], lpFormat[71 + k]
		);
	}
	OutputDiscLog("\n");
}

VOID OutputBDPhysicalAddressControl(
	LPBYTE lpFormat,
	WORD wFormatLength,
	INT nPacCnt
) {
	if (nPacCnt == 0) {
		OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("PacHeaderList"));
		for (WORD i = 0; i < (wFormatLength - 2) / 384; i++) {
			OutputDiscLog(
				"\t                       PAC ID: %.3" CHARWIDTH "s\n"
				"\t            PAC format number: %02x\n"
				"\t             PAC Update Count: %u\n"
				"\t            Unknown PAC Rules: %02x %02x %02x %02x\n"
				"\tUnknown PAC Entire Disc Flags: %d\n"
				"\t           Number of Segments: %d\n"
				, &lpFormat[0]
				, lpFormat[3]
				, MAKEUINT(MAKEWORD(lpFormat[7], lpFormat[6]), MAKEWORD(lpFormat[5], lpFormat[4]))
				, lpFormat[8], lpFormat[9], lpFormat[10], lpFormat[11]
				, lpFormat[12]
				, lpFormat[15]
			);
			for (BYTE j = 0; j < lpFormat[15]; j++) {
				OutputDiscLog("\t Segment %d: %02x %02x %02x %02x %02x %02x %02x %02x\n"
					, j
					, lpFormat[16 + j * 8], lpFormat[17 + j * 8], lpFormat[18 + j * 8], lpFormat[19 + j * 8]
					, lpFormat[20 + j * 8], lpFormat[21 + j * 8], lpFormat[22 + j * 8], lpFormat[23 + j * 8]
				);

			}
		}
	}
	else if (nPacCnt == 1) {
		OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("PacHeaderList"));
		for (WORD i = 0; i < (wFormatLength - 2) / 4; i++) {
			OutputDiscLog(
				"\t                       PAC ID: %.3" CHARWIDTH "s\n"
				"\t            PAC format number: %02x\n"
				, &lpFormat[0]
				, lpFormat[3]
			);
		}
	}
	else {
		OutputMainChannel(fileDisc, lpFormat, _T("PacData"), 0, wFormatLength);
	}
}

VOID OutputBDStructureFormat(
	PDISC pDisc,
	BYTE byFormatCode,
	WORD wFormatLength,
	LPBYTE lpFormat,
	INT nPacCnt
) {
	OutputDiscLog("Disc Structure Data Length: %d\n", wFormatLength);
	switch (byFormatCode) {
	case 0:
		OutputBDDiscInformation(pDisc, lpFormat, wFormatLength);
		break;
	case DvdCopyrightDescriptor:
		OutputDVDCopyrightDescriptor((PDVD_COPYRIGHT_DESCRIPTOR)lpFormat, &(pDisc->DVD.protect), fileDisc);
		break;
		// format 0x02 is reserved
	case DvdBCADescriptor:
		OutputDiscBCADescriptor(pDisc, (PDVD_BCA_DESCRIPTOR)lpFormat, wFormatLength, fileDisc);
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
		OutputDiscLog("\tReserved\n");
		break;
	}
}

// http://xboxdevwiki.net/Xbe#Title_ID
VOID OutputXboxPublisher(
	LPBYTE buf
) {
	if (!strncmp((LPCCH)buf, "AC", 2)) {
		OutputDiscLog("Acclaim Entertainment\n");
	}
	else if (!strncmp((LPCCH)buf, "AH", 2)) {
		OutputDiscLog("ARUSH Entertainment\n");
	}
	else if (!strncmp((LPCCH)buf, "AQ", 2)) {
		OutputDiscLog("Aqua System\n");
	}
	else if (!strncmp((LPCCH)buf, "AS", 2)) {
		OutputDiscLog("ASK\n");
	}
	else if (!strncmp((LPCCH)buf, "AT", 2)) {
		OutputDiscLog("Atlus\n");
	}
	else if (!strncmp((LPCCH)buf, "AV", 2)) {
		OutputDiscLog("Activision\n");
	}
	else if (!strncmp((LPCCH)buf, "AY", 2)) {
		OutputDiscLog("Aspyr Media\n");
	}
	else if (!strncmp((LPCCH)buf, "BA", 2)) {
		OutputDiscLog("Bandai\n");
	}
	else if (!strncmp((LPCCH)buf, "BL", 2)) {
		OutputDiscLog("Black Box\n");
	}
	else if (!strncmp((LPCCH)buf, "BM", 2)) {
		OutputDiscLog("BAM! Entertainment\n");
	}
	else if (!strncmp((LPCCH)buf, "BR", 2)) {
		OutputDiscLog("Broccoli Co.\n");
	}
	else if (!strncmp((LPCCH)buf, "BS", 2)) {
		OutputDiscLog("Bethesda Softworks\n");
	}
	else if (!strncmp((LPCCH)buf, "BU", 2)) {
		OutputDiscLog("Bunkasha Co.\n");
	}
	else if (!strncmp((LPCCH)buf, "BV", 2)) {
		OutputDiscLog("Buena Vista Games\n");
	}
	else if (!strncmp((LPCCH)buf, "BW", 2)) {
		OutputDiscLog("BBC Multimedia\n");
	}
	else if (!strncmp((LPCCH)buf, "BZ", 2)) {
		OutputDiscLog("Blizzard\n");
	}
	else if (!strncmp((LPCCH)buf, "CC", 2)) {
		OutputDiscLog("Capcom\n");
	}
	else if (!strncmp((LPCCH)buf, "CK", 2)) {
		OutputDiscLog("Kemco Corporation\n");
	}
	else if (!strncmp((LPCCH)buf, "CM", 2)) {
		OutputDiscLog("Codemasters\n");
	}
	else if (!strncmp((LPCCH)buf, "CV", 2)) {
		OutputDiscLog("Crave Entertainment\n");
	}
	else if (!strncmp((LPCCH)buf, "DC", 2)) {
		OutputDiscLog("DreamCatcher Interactive\n");
	}
	else if (!strncmp((LPCCH)buf, "DX", 2)) {
		OutputDiscLog("Davilex\n");
	}
	else if (!strncmp((LPCCH)buf, "EA", 2)) {
		OutputDiscLog("Electronic Arts\n");
	}
	else if (!strncmp((LPCCH)buf, "EC", 2)) {
		OutputDiscLog("Encore inc\n");
	}
	else if (!strncmp((LPCCH)buf, "EL", 2)) {
		OutputDiscLog("Enlight Software\n");
	}
	else if (!strncmp((LPCCH)buf, "EM", 2)) {
		OutputDiscLog("Empire Interactive\n");
	}
	else if (!strncmp((LPCCH)buf, "ES", 2)) {
		OutputDiscLog("Eidos Interactive\n");
	}
	else if (!strncmp((LPCCH)buf, "FI", 2)) {
		OutputDiscLog("Fox Interactive\n");
	}
	else if (!strncmp((LPCCH)buf, "FS", 2)) {
		OutputDiscLog("From Software\n");
	}
	else if (!strncmp((LPCCH)buf, "GE", 2)) {
		OutputDiscLog("Genki Co.\n");
	}
	else if (!strncmp((LPCCH)buf, "GV", 2)) {
		OutputDiscLog("Groove Games\n");
	}
	else if (!strncmp((LPCCH)buf, "HE", 2)) {
		OutputDiscLog("Tru Blu (Entertainment division of Home Entertainment Suppliers)\n");
	}
	else if (!strncmp((LPCCH)buf, "HP", 2)) {
		OutputDiscLog("Hip games\n");
	}
	else if (!strncmp((LPCCH)buf, "HU", 2)) {
		OutputDiscLog("Hudson Soft\n");
	}
	else if (!strncmp((LPCCH)buf, "HW", 2)) {
		OutputDiscLog("Highwaystar\n");
	}
	else if (!strncmp((LPCCH)buf, "IA", 2)) {
		OutputDiscLog("Mad Catz Interactive\n");
	}
	else if (!strncmp((LPCCH)buf, "IF", 2)) {
		OutputDiscLog("Idea Factory\n");
	}
	else if (!strncmp((LPCCH)buf, "IG", 2)) {
		OutputDiscLog("Infogrames\n");
	}
	else if (!strncmp((LPCCH)buf, "IL", 2)) {
		OutputDiscLog("Interlex Corporation\n");
	}
	else if (!strncmp((LPCCH)buf, "IM", 2)) {
		OutputDiscLog("Imagine Media\n");
	}
	else if (!strncmp((LPCCH)buf, "IO", 2)) {
		OutputDiscLog("Ignition Entertainment\n");
	}
	else if (!strncmp((LPCCH)buf, "IP", 2)) {
		OutputDiscLog("Interplay Entertainment\n");
	}
	else if (!strncmp((LPCCH)buf, "IX", 2)) {
		OutputDiscLog("InXile Entertainment\n");
	}
	else if (!strncmp((LPCCH)buf, "JA", 2)) {
		OutputDiscLog("Jaleco\n");
	}
	else if (!strncmp((LPCCH)buf, "JW", 2)) {
		OutputDiscLog("JoWooD\n");
	}
	else if (!strncmp((LPCCH)buf, "KB", 2)) {
		OutputDiscLog("Kemco\n");
	}
	else if (!strncmp((LPCCH)buf, "KI", 2)) {
		OutputDiscLog("Kids Station Inc.\n");
	}
	else if (!strncmp((LPCCH)buf, "KN", 2)) {
		OutputDiscLog("Konami\n");
	}
	else if (!strncmp((LPCCH)buf, "KO", 2)) {
		OutputDiscLog("KOEI\n");
	}
	else if (!strncmp((LPCCH)buf, "KU", 2)) {
		OutputDiscLog("Kobi and/or GAE (formerly Global A Entertainment)\n");
	}
	else if (!strncmp((LPCCH)buf, "LA", 2)) {
		OutputDiscLog("LucasArts\n");
	}
	else if (!strncmp((LPCCH)buf, "LS", 2)) {
		OutputDiscLog("Black Bean Games (publishing arm of Leader S.p.A.\n");
	}
	else if (!strncmp((LPCCH)buf, "MD", 2)) {
		OutputDiscLog("Metro3D\n");
	}
	else if (!strncmp((LPCCH)buf, "ME", 2)) {
		OutputDiscLog("Medix\n");
	}
	else if (!strncmp((LPCCH)buf, "MI", 2)) {
		OutputDiscLog("Microids\n");
	}
	else if (!strncmp((LPCCH)buf, "MJ", 2)) {
		OutputDiscLog("Majesco Entertainment\n");
	}
	else if (!strncmp((LPCCH)buf, "MM", 2)) {
		OutputDiscLog("Myelin Media\n");
	}
	else if (!strncmp((LPCCH)buf, "MP", 2)) {
		OutputDiscLog("MediaQuest\n");
	}
	else if (!strncmp((LPCCH)buf, "MS", 2)) {
		OutputDiscLog("Microsoft Game Studios\n");
	}
	else if (!strncmp((LPCCH)buf, "MW", 2)) {
		OutputDiscLog("Midway Games\n");
	}
	else if (!strncmp((LPCCH)buf, "MX", 2)) {
		OutputDiscLog("Empire Interactive\n");
	}
	else if (!strncmp((LPCCH)buf, "NK", 2)) {
		OutputDiscLog("NewKidCo\n");
	}
	else if (!strncmp((LPCCH)buf, "NL", 2)) {
		OutputDiscLog("NovaLogic\n");
	}
	else if (!strncmp((LPCCH)buf, "NM", 2)) {
		OutputDiscLog("Namco\n");
	}
	else if (!strncmp((LPCCH)buf, "OX", 2)) {
		OutputDiscLog("Oxygen Interactive\n");
	}
	else if (!strncmp((LPCCH)buf, "PC", 2)) {
		OutputDiscLog("Playlogic Entertainment\n");
	}
	else if (!strncmp((LPCCH)buf, "PL", 2)) {
		OutputDiscLog("Phantagram Co., Ltd.\n");
	}
	else if (!strncmp((LPCCH)buf, "RA", 2)) {
		OutputDiscLog("Rage\n");
	}
	else if (!strncmp((LPCCH)buf, "SA", 2)) {
		OutputDiscLog("Sammy\n");
	}
	else if (!strncmp((LPCCH)buf, "SC", 2)) {
		OutputDiscLog("SCi Games\n");
	}
	else if (!strncmp((LPCCH)buf, "SE", 2)) {
		OutputDiscLog("SEGA\n");
	}
	else if (!strncmp((LPCCH)buf, "SN", 2)) {
		OutputDiscLog("SNK\n");
	}
	else if (!strncmp((LPCCH)buf, "SS", 2)) {
		OutputDiscLog("Simon & Schuster\n");
	}
	else if (!strncmp((LPCCH)buf, "SU", 2)) {
		OutputDiscLog("Success Corporation\n");
	}
	else if (!strncmp((LPCCH)buf, "SW", 2)) {
		OutputDiscLog("Swing! Deutschland\n");
	}
	else if (!strncmp((LPCCH)buf, "TA", 2)) {
		OutputDiscLog("Takara\n");
	}
	else if (!strncmp((LPCCH)buf, "TC", 2)) {
		OutputDiscLog("Tecmo\n");
	}
	else if (!strncmp((LPCCH)buf, "TD", 2)) {
		OutputDiscLog("The 3DO Company (or just 3DO)\n");
	}
	else if (!strncmp((LPCCH)buf, "TK", 2)) {
		OutputDiscLog("Takuyo\n");
	}
	else if (!strncmp((LPCCH)buf, "TM", 2)) {
		OutputDiscLog("TDK Mediactive\n");
	}
	else if (!strncmp((LPCCH)buf, "TQ", 2)) {
		OutputDiscLog("THQ\n");
	}
	else if (!strncmp((LPCCH)buf, "TS", 2)) {
		OutputDiscLog("Titus Interactive\n");
	}
	else if (!strncmp((LPCCH)buf, "TT", 2)) {
		OutputDiscLog("Take-Two Interactive Software\n");
	}
	else if (!strncmp((LPCCH)buf, "US", 2)) {
		OutputDiscLog("Ubisoft\n");
	}
	else if (!strncmp((LPCCH)buf, "VC", 2)) {
		OutputDiscLog("Victor Interactive Software\n");
	}
	else if (!strncmp((LPCCH)buf, "VN", 2)) {
		OutputDiscLog("Vivendi Universal (just took Interplays publishing rights)\n");
	}
	else if (!strncmp((LPCCH)buf, "VU", 2)) {
		OutputDiscLog("Vivendi Universal Games\n");
	}
	else if (!strncmp((LPCCH)buf, "VV", 2)) {
		OutputDiscLog("Vivendi Universal Games\n");
	}
	else if (!strncmp((LPCCH)buf, "WE", 2)) {
		OutputDiscLog("Wanadoo Edition\n");
	}
	else if (!strncmp((LPCCH)buf, "WR", 2)) {
		OutputDiscLog("Warner Bros. Interactive Entertainment\n");
	}
	else if (!strncmp((LPCCH)buf, "XI", 2)) {
		OutputDiscLog("XPEC Entertainment and Idea Factory\n");
	}
	else if (!strncmp((LPCCH)buf, "XK", 2)) {
		OutputDiscLog("Xbox kiosk disc\n");
	}
	else if (!strncmp((LPCCH)buf, "XL", 2)) {
		OutputDiscLog("Xbox special bundled or live demo disc\n");
	}
	else if (!strncmp((LPCCH)buf, "XM", 2)) {
		OutputDiscLog("Evolved Games\n");
	}
	else if (!strncmp((LPCCH)buf, "XP", 2)) {
		OutputDiscLog("XPEC Entertainment\n");
	}
	else if (!strncmp((LPCCH)buf, "XR", 2)) {
		OutputDiscLog("Panorama\n");
	}
	else if (!strncmp((LPCCH)buf, "YB", 2)) {
		OutputDiscLog("YBM Sisa (South-Korea)\n");
	}
	else if (!strncmp((LPCCH)buf, "ZD", 2)) {
		OutputDiscLog("Zushi Games (formerly Zoo Digital Publishing)\n");
	}
	else {
		OutputDiscLog("Unknown\n");
	}
}

VOID OutputXboxRegion(
	BYTE buf
) {
	if (buf == 'W') {
		OutputDiscLog("World\n");
	}
	else if (buf == 'A') {
		OutputDiscLog("USA\n");
	}
	else if (buf == 'J') {
		OutputDiscLog("Japan\n");
	}
	else if (buf == 'E') {
		OutputDiscLog("Europe\n");
	}
	else if (buf == 'K') {
		OutputDiscLog("USA, Japan\n");
	}
	else if (buf == 'L') {
		OutputDiscLog("USA, Europe\n");
	}
	else if (buf == 'H') {
		OutputDiscLog("Japan, Europe\n");
	}
	else {
		OutputDiscLog("Other\n")
	}
}

VOID OutputXboxManufacturingInfo(
	LPBYTE buf
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DiscManufacturingInformation")
		"\tSystem Version: %02u\n"
		, buf[0]
	);

	char date[20] = {};
	printwin32filetime(MAKEUINT64(MAKEUINT(MAKEWORD(buf[16], buf[17]), MAKEWORD(buf[18], buf[19]))
		, MAKEUINT(MAKEWORD(buf[20], buf[21]), MAKEWORD(buf[22], buf[23]))), date);
	if (buf[0] == 0x01) {
		OutputDiscLog(
			"\t     Publisher: "
		);
		OutputXboxPublisher(&buf[8]);
		OutputDiscLog(
			"\t        Serial: %c%c%c\n"
			"\t       Version: 1.%c%c\n"
			"\t        Region: "
			, buf[10], buf[11], buf[12], buf[13], buf[14]
		);
		OutputXboxRegion(buf[15]);
		OutputDiscLog(
			"\t     Timestamp: %" CHARWIDTH "s\n"
			"\t       Unknown: %02u\n"
			, date, buf[24]);
	}
	else if (buf[0] == 0x02) {
		OutputDiscLog(
			"\t     Timestamp: %" CHARWIDTH "s\n"
			"\t       Unknown: %02u\n"
			"\t      Media ID: "
			, date, buf[24]);
		for (WORD k = 32; k < 48; k++) {
			OutputDiscLog("%02x", buf[k]);
		}
		OutputDiscLog(
			"\n"
			"\t     Publisher: "
		);
		OutputXboxPublisher(&buf[64]);

		OutputDiscLog(
			"\t        Serial: %c%c%c%c\n"
			"\t       Version: 1.%c%c\n"
			"\t        Region: "
			, buf[66], buf[67], buf[68], buf[69], buf[70], buf[71]
		);
		OutputXboxRegion(buf[72]);
		if (buf[73] == '0' && buf[74] == 'X') {
			OutputDiscLog(
				"\t       Unknown: %c%c\n"
				"\t          Disc: %c of %c\n"
				, buf[73], buf[74], buf[75], buf[76])
		}
		else {
			OutputDiscLog(
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
	OutputDVDStructureFormat(pDisc, DvdPhysicalDescriptor, DISC_MAIN_DATA_SIZE, buf, &dwSectorLen, 0);
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("SecuritySector")
		"\t                     CPR_MAI Key: %08x\n"
		"\t      Version of challenge table: %02u\n"
		"\t     Number of challenge entries: %u\n"
		"\t     Encrypted challenge entries: "
		, MAKEUINT(MAKEWORD(buf[723], buf[722]), MAKEWORD(buf[721], buf[720]))
		, buf[768], buf[769]
	);
	for (WORD k = 770; k < 1024; k++) {
		OutputDiscLog("%02x", buf[k]);
	}

	char date[20] = {};
	printwin32filetime(MAKEUINT64(MAKEUINT(MAKEWORD(buf[1055], buf[1056]), MAKEWORD(buf[1057], buf[1058]))
		, MAKEUINT(MAKEWORD(buf[1059], buf[1060]), MAKEWORD(buf[1061], buf[1062]))), date);
	OutputDiscLog(
		"\n"
		"\t            Timestamp of unknown: %" CHARWIDTH "s\n"
		"\t                         Unknown: "
		, date
	);
	for (WORD k = 1083; k < 1099; k++) {
		OutputDiscLog("%02x", buf[k]);
	}

	printwin32filetime(MAKEUINT64(MAKEUINT(MAKEWORD(buf[1183], buf[1184]), MAKEWORD(buf[1185], buf[1186]))
		, MAKEUINT(MAKEWORD(buf[1187], buf[1188]), MAKEWORD(buf[1189], buf[1190]))), date);
	OutputDiscLog(
		"\n"
		"\t          Timestamp of authoring: %" CHARWIDTH "s\n"
		"\t                         Unknown: %02x\n"
		"\t                         Unknown: "
		, date, buf[1210]
	);
	for (WORD k = 1211; k < 1227; k++) {
		OutputDiscLog("%02x", buf[k]);
	}

	OutputDiscLog(
		"\n"
		"\t                    SHA-1 hash A: "
	);
	for (WORD k = 1227; k < 1247; k++) {
		OutputDiscLog("%02x", buf[k]);
	}

	OutputDiscLog(
		"\n"
		"\t                     Signature A: "
	);
	for (WORD k = 1247; k < 1503; k++) {
		OutputDiscLog("%02x", buf[k]);
	}

	printwin32filetime(MAKEUINT64(MAKEUINT(MAKEWORD(buf[1503], buf[1504]), MAKEWORD(buf[1505], buf[1506]))
		, MAKEUINT(MAKEWORD(buf[1507], buf[1508]), MAKEWORD(buf[1509], buf[1510]))), date);
	OutputDiscLog(
		"\n"
		"\t          Timestamp of mastering: %" CHARWIDTH "s\n"
		"\t                         Unknown: %02x\n"
		"\t                         Unknown: "
		, date, buf[1530]
	);
	for (WORD k = 1531; k < 1547; k++) {
		OutputDiscLog("%02x", buf[k]);
	}

	OutputDiscLog(
		"\n"
		"\t                    SHA-1 hash B: "
	);
	for (WORD k = 1547; k < 1567; k++) {
		OutputDiscLog("%02x", buf[k]);
	}

	OutputDiscLog(
		"\n"
		"\t                     Signature B: "
	);
	for (WORD k = 1567; k < 1632; k++) {
		OutputDiscLog("%02x", buf[k]);
	}

	OutputDiscLog(
		"\n"
		"\tNumber of security sector ranges: %u\n"
		"\t          security sector ranges: \n"
		, buf[1632]
	);
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
			OutputDiscLog("\t\t       Layer 0");
			startLBA = startPsn - 0x30000;
			endLBA = endPsn - 0x30000;
			pDisc->DVD.securitySectorRange[i][0] = startLBA;
			pDisc->DVD.securitySectorRange[i][1] = endLBA;
		}
		else if (i < 16) {
			OutputDiscLog("\t\t       Layer 1");
			startLBA = dwEndLayerZeroSector * 2 - (~startPsn & 0xffffff) - 0x30000 + 1;
			endLBA = dwEndLayerZeroSector * 2 - (~endPsn & 0xffffff) - 0x30000 + 1;
			pDisc->DVD.securitySectorRange[i][0] = startLBA;
			pDisc->DVD.securitySectorRange[i][1] = endLBA;
		}
		else {
			OutputDiscLog("\t\tUnknown ranges");
			startLBA = startPsn;
			endLBA = endPsn;
		}
		OutputDiscLog("\t\tUnknown: %02x%02x%02x, startLBA-endLBA: %8lu-%8lu\n"
			, buf[k], buf[k + 1], buf[k + 2], startLBA, endLBA);
	}
}

// https://abgx360.xecuter.com/dl/abgx360_v1.0.6_source.zip
VOID OutputXbox360SecuritySector(
	PDISC pDisc,
	LPBYTE buf
) {
	DWORD dwSectorLen = 0;
	OutputDVDStructureFormat(pDisc, DvdPhysicalDescriptor, DISC_MAIN_DATA_SIZE, buf, &dwSectorLen, 0);
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("SecuritySector")
		"\t                         Unknown: "
	);
	for (WORD k = 256; k < 284; k++) {
		OutputDiscLog("%02x", buf[k]);
	}
	OutputDiscLog("\n");

	for (INT i = 0; i < 21; i++) {
		OutputDiscLog(
			"\t             [%02d] Challenge Data: %02x%02x%02x%02x, Response: %02x%02x%02x%02x%02x\n"
			, i + 1, buf[512 + i * 9], buf[512 + i * 9 + 1], buf[512 + i * 9 + 2], buf[512 + i * 9 + 3]
			, buf[512 + i * 9 + 4], buf[512 + i * 9 + 5], buf[512 + i * 9 + 6], buf[512 + i * 9 + 7], buf[512 + i * 9 + 8]
		);
	}

	OutputDiscLog(
		"\t                     CPR_MAI Key: %08x\n"
		"\t      Version of challenge table: %02u\n"
		"\t     Number of challenge entries: %u\n"
		"\t     Encrypted challenge entries: "
		, MAKEUINT(MAKEWORD(buf[723], buf[722]), MAKEWORD(buf[721], buf[720]))
		, buf[768], buf[769]
	);
	for (WORD k = 770; k < 1024; k++) {
		OutputDiscLog("%02x", buf[k]);
	}
	OutputDiscLog("\n");

	BYTE dcrt[252] = {};
	decryptChallengeResponse(dcrt, buf);
	for (INT i = 0; i < 21; i++) {
		OutputDiscLog(
			"\t                    Decrypted[%02d] -> Challenge Type: %02x, Challenge ID: %02x"
			", Tolerance: %02x, Type: %02x, Challenge Data: %02x%02x%02x%02x, Response: %02x%02x%02x%02x\n"
			, i + 1, dcrt[i * 12], dcrt[i * 12 + 1], dcrt[i * 12 + 2], dcrt[i * 12 + 3]
			, dcrt[i * 12 + 4], dcrt[i * 12 + 5], dcrt[i * 12 + 6], dcrt[i * 12 + 7]
			, dcrt[i * 12 + 8], dcrt[i * 12 + 9], dcrt[i * 12 + 10], dcrt[i * 12 + 11]
		);
	}

	OutputDiscLog(
		"\t                        Media ID: "
	);
	for (WORD k = 1120; k < 1136; k++) {
		OutputDiscLog("%02x", buf[k]);
	}

	CHAR date[21] = {};
	printwin32filetime(MAKEUINT64(MAKEUINT(MAKEWORD(buf[1183], buf[1184]), MAKEWORD(buf[1185], buf[1186]))
		, MAKEUINT(MAKEWORD(buf[1187], buf[1188]), MAKEWORD(buf[1189], buf[1190]))), date);
	OutputDiscLog(
		"\n"
		"\t          Timestamp of authoring: %" CHARWIDTH "s\n"
		"\t                         Unknown: %02x\n"
		"\t                         Unknown: "
		, date, buf[1210]
	);
	for (WORD k = 1211; k < 1227; k++) {
		OutputDiscLog("%02x", buf[k]);
	}

	OutputDiscLog(
		"\n"
		"\t                    SHA-1 hash A: "
	);
	for (WORD k = 1227; k < 1247; k++) {
		OutputDiscLog("%02x", buf[k]);
	}

	OutputDiscLog(
		"\n"
		"\t                     Signature A: "
	);
	for (WORD k = 1247; k < 1503; k++) {
		OutputDiscLog("%02x", buf[k]);
	}

	printwin32filetime(MAKEUINT64(MAKEUINT(MAKEWORD(buf[1503], buf[1504]), MAKEWORD(buf[1505], buf[1506]))
		, MAKEUINT(MAKEWORD(buf[1507], buf[1508]), MAKEWORD(buf[1509], buf[1510]))), date);
	OutputDiscLog(
		"\n"
		"\t          Timestamp of mastering: %" CHARWIDTH "s\n"
		"\t                         Unknown: %02x\n"
		"\t                         Unknown: "
		, date, buf[1530]
	);
	for (WORD k = 1531; k < 1547; k++) {
		OutputDiscLog("%02x", buf[k]);
	}

	OutputDiscLog(
		"\n"
		"\t                    SHA-1 hash B: "
	);
	for (WORD k = 1547; k < 1567; k++) {
		OutputDiscLog("%02x", buf[k]);
	}
	OutputDiscLog(
		"\n"
		"\t                     Signature B: "
	);
	for (WORD k = 1567; k < 1632; k++) {
		OutputDiscLog("%02x", buf[k]);
	}
	OutputDiscLog(
		"\n"
		"\tNumber of security sector ranges: %u\n"
		"\t          security sector ranges: \n"
		, buf[1632]
	);
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
			OutputDiscLog("\t\t       Layer 0");
			startLBA = startPsn - 0x30000;
			endLBA = endPsn - 0x30000;
			pDisc->DVD.securitySectorRange[i][0] = startLBA;
			pDisc->DVD.securitySectorRange[i][1] = endLBA;
		}
		else if (i == 3) {
			OutputDiscLog("\t\t       Layer 1");
			startLBA = dwEndLayerZeroSector * 2 - (~startPsn & 0xffffff) - 0x30000 + 1;
			endLBA = dwEndLayerZeroSector * 2 - (~endPsn & 0xffffff) - 0x30000 + 1;
			pDisc->DVD.securitySectorRange[i][0] = startLBA;
			pDisc->DVD.securitySectorRange[i][1] = endLBA;
		}
		else {
			OutputDiscLog("\t\tUnknown ranges");
			startLBA = startPsn;
			endLBA = endPsn;
		}
		OutputDiscLog("\t\tResponse Type: %02x, Challenge ID: %02x, Mod: %02x, startLBA-endLBA: %8lu-%8lu\n"
			, buf[k], buf[k + 1], buf[k + 2], startLBA, endLBA);
	}
}
