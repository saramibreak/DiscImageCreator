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
#include "check.h"
#include "convert.h"
#include "output.h"
#include "outputScsiCmdLog.h"
#include "set.h"

VOID OutputInquiry(
	PINQUIRYDATA pInquiry
) {
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(InquiryData)
		"\t          DeviceType: ");
	switch (pInquiry->DeviceType) {
	case DIRECT_ACCESS_DEVICE:
		OutputDriveLogA("DirectAccessDevice (Floppy etc)\n");
		break;
	case READ_ONLY_DIRECT_ACCESS_DEVICE:
		OutputDriveLogA("ReadOnlyDirectAccessDevice (CD/DVD etc)\n");
		break;
	case OPTICAL_DEVICE:
		OutputDriveLogA("OpticalDisk\n");
		break;
	default:
		OutputDriveLogA("OtherDevice\n");
		break;
	}
	OutputDriveLogA(
		"\t DeviceTypeQualifier: ");
	switch (pInquiry->DeviceTypeQualifier) {
	case DEVICE_QUALIFIER_ACTIVE:
		OutputDriveLogA("Active\n");
		break;
	case DEVICE_QUALIFIER_NOT_ACTIVE:
		OutputDriveLogA("NotActive\n");
		break;
	case DEVICE_QUALIFIER_NOT_SUPPORTED:
		OutputDriveLogA("NotSupported\n");
		break;
	default:
		OutputDriveLogA("\n");
		break;
	}

	OutputDriveLogA(
		"\t  DeviceTypeModifier: %u\n"
		"\t      RemovableMedia: %s\n"
		"\t            Versions: %u\n"
		"\t  ResponseDataFormat: %u\n"
		"\t           HiSupport: %s\n"
		"\t             NormACA: %s\n"
		"\t       TerminateTask: %s\n"
		"\t                AERC: %s\n"
		"\t    AdditionalLength: %u\n"
		"\t       MediumChanger: %s\n"
		"\t           MultiPort: %s\n"
		"\t   EnclosureServices: %s\n"
		"\t           SoftReset: %s\n"
		"\t        CommandQueue: %s\n"
		"\t      LinkedCommands: %s\n"
		"\t  RelativeAddressing: %s\n",
		pInquiry->DeviceTypeModifier,
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->RemovableMedia),
		pInquiry->Versions,
		pInquiry->ResponseDataFormat,
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->HiSupport),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->NormACA),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->TerminateTask),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->AERC),
		pInquiry->AdditionalLength,
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->MediumChanger),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->MultiPort),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->EnclosureServices),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->SoftReset),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->CommandQueue),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->LinkedCommands),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->RelativeAddressing));

	OutputDriveLogA(
		"\t            VendorId: %.8s\n"
		"\t           ProductId: %.16s\n"
		"\tProductRevisionLevel: %.4s\n"
		"\t      VendorSpecific: %.20s\n",
		pInquiry->VendorId,
		pInquiry->ProductId,
		pInquiry->ProductRevisionLevel,
		pInquiry->VendorSpecific);
}

VOID OutputGetConfigurationHeader(
	PGET_CONFIGURATION_HEADER pConfigHeader
) {
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(GetConfiguration)
		"\t    DataLength: %ld\n"
		"\tCurrentProfile: "
		, MAKELONG(MAKEWORD(pConfigHeader->DataLength[3], pConfigHeader->DataLength[2]),
		MAKEWORD(pConfigHeader->DataLength[1], pConfigHeader->DataLength[0])));
	OutputGetConfigurationFeatureProfileType(
		MAKEWORD(pConfigHeader->CurrentProfile[1], pConfigHeader->CurrentProfile[0]));
	OutputDriveLogA("\n");
}

VOID OutputGetConfigurationFeatureProfileType(
	WORD wFeatureProfileType
) {
	switch (wFeatureProfileType) {
	case ProfileInvalid:
		OutputDriveLogA("Invalid");
		break;
	case ProfileNonRemovableDisk:
		OutputDriveLogA("NonRemovableDisk");
		break;
	case ProfileRemovableDisk:
		OutputDriveLogA("RemovableDisk");
		break;
	case ProfileMOErasable:
		OutputDriveLogA("MOErasable");
		break;
	case ProfileMOWriteOnce:
		OutputDriveLogA("MOWriteOnce");
		break;
	case ProfileAS_MO:
		OutputDriveLogA("AS_MO");
		break;
	case ProfileCdrom:
		OutputDriveLogA("CD-ROM");
		break;
	case ProfileCdRecordable:
		OutputDriveLogA("CD-R");
		break;
	case ProfileCdRewritable:
		OutputDriveLogA("CD-RW");
		break;
	case ProfileDvdRom:
		OutputDriveLogA("DVD-ROM");
		break;
	case ProfileDvdRecordable:
		OutputDriveLogA("DVD-R");
		break;
	case ProfileDvdRam:
		OutputDriveLogA("DVD-RAM");
		break;
	case ProfileDvdRewritable:
		OutputDriveLogA("DVD-RW");
		break;
	case ProfileDvdRWSequential:
		OutputDriveLogA("DVD-RW Sequential");
		break;
	case ProfileDvdDashRDualLayer:
		OutputDriveLogA("DVD-R DL");
		break;
	case ProfileDvdDashRLayerJump:
		OutputDriveLogA("DVD-R Layer Jump");
		break;
	case ProfileDvdPlusRW:
		OutputDriveLogA("DVD+RW");
		break;
	case ProfileDvdPlusR:
		OutputDriveLogA("DVD+R");
		break;
	case ProfileDDCdrom:
		OutputDriveLogA("DDCD-ROM");
		break;
	case ProfileDDCdRecordable:
		OutputDriveLogA("DDCD-R");
		break;
	case ProfileDDCdRewritable:
		OutputDriveLogA("DDCD-RW");
		break;
	case ProfileDvdPlusRWDualLayer:
		OutputDriveLogA("DVD+RW DL");
		break;
	case ProfileDvdPlusRDualLayer:
		OutputDriveLogA("DVD+R DL");
		break;
	case ProfileBDRom:
		OutputDriveLogA("BD-ROM");
		break;
	case ProfileBDRSequentialWritable:
		OutputDriveLogA("BD-R Sequential Writable");
		break;
	case ProfileBDRRandomWritable:
		OutputDriveLogA("BD-R Random Writable");
		break;
	case ProfileBDRewritable:
		OutputDriveLogA("BD-R");
		break;
	case ProfileHDDVDRom:
		OutputDriveLogA("HD DVD-ROM");
		break;
	case ProfileHDDVDRecordable:
		OutputDriveLogA("HD DVD-R");
		break;
	case ProfileHDDVDRam:
		OutputDriveLogA("HD DVD-RAM");
		break;
	case ProfileHDDVDRewritable:
		OutputDriveLogA("HD-DVD-RW");
		break;
	case ProfileHDDVDRDualLayer:
		OutputDriveLogA("HD-DVD-R DL");
		break;
	case ProfileHDDVDRWDualLayer:
		OutputDriveLogA("HD-DVD-RW DL");
		break;
	case ProfilePlaystationCdrom:
		OutputDriveLogA("PlayStation CD-ROM");
		break;
	case ProfilePlaystation2Cdrom:
		OutputDriveLogA("PlayStation2 CD-ROM");
		break;
	case ProfilePlaystation2DvdRom:
		OutputDriveLogA("PlayStation2 DVD-ROM");
		break;
	case ProfilePlaystation3DvdRom:
		OutputDriveLogA("PlayStation3 DVD-ROM");
		break;
	case ProfilePlaystation3BDRom:
		OutputDriveLogA("PlayStation3 BD-ROM");
		break;
	case ProfilePlaystation4BDRom:
		OutputDriveLogA("PlayStation4 BD-ROM");
		break;
	case ProfileNonStandard:
		OutputDriveLogA("NonStandard");
		break;
	default:
		OutputDriveLogA("Reserved [%#x]", wFeatureProfileType);
		break;
	}
}

VOID OutputGetConfigurationFeatureProfileList(
	PFEATURE_DATA_PROFILE_LIST pList
) {
	OutputDriveLogA("\tFeatureProfileList\n");
	for (UINT i = 0; i < pList->Header.AdditionalLength / sizeof(FEATURE_DATA_PROFILE_LIST_EX); i++) {
		OutputDriveLogA("\t\t");
		OutputGetConfigurationFeatureProfileType(
			MAKEWORD(pList->Profiles[i].ProfileNumber[1], pList->Profiles[i].ProfileNumber[0]));
		OutputDriveLogA("\n");
	}
}

VOID OutputGetConfigurationFeatureCore(
	PFEATURE_DATA_CORE pCore
) {
	OutputDriveLogA(
		"\tFeatureCore\n"
		"\t\tPhysicalInterface: ");
	LONG lVal = MAKELONG(
		MAKEWORD(pCore->PhysicalInterface[3], pCore->PhysicalInterface[2]),
		MAKEWORD(pCore->PhysicalInterface[1], pCore->PhysicalInterface[0]));
	switch (lVal) {
	case 0:
		OutputDriveLogA("Unspecified\n");
		break;
	case 1:
		OutputDriveLogA("SCSI Family\n");
		break;
	case 2:
		OutputDriveLogA("ATAPI\n");
		break;
	case 3:
		OutputDriveLogA("IEEE 1394 - 1995\n");
		break;
	case 4:
		OutputDriveLogA("IEEE 1394A\n");
		break;
	case 5:
		OutputDriveLogA("Fibre Channel\n");
		break;
	case 6:
		OutputDriveLogA("IEEE 1394B\n");
		break;
	case 7:
		OutputDriveLogA("Serial ATAPI\n");
		break;
	case 8:
		OutputDriveLogA("USB (both 1.1 and 2.0)\n");
		break;
	case 0xffff:
		OutputDriveLogA("Vendor Unique\n");
		break;
	default:
		OutputDriveLogA("Reserved: %08ld\n", lVal);
		break;
	}
	OutputDriveLogA(
		"\t\t  DeviceBusyEvent: %s\n"
		"\t\t         INQUIRY2: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pCore->DeviceBusyEvent),
		BOOLEAN_TO_STRING_YES_NO_A(pCore->INQUIRY2));
}

VOID OutputGetConfigurationFeatureMorphing(
	PFEATURE_DATA_MORPHING pMorphing
) {
	OutputDriveLogA(
		"\tFeatureMorphing\n"
		"\t\tAsynchronous: %s\n"
		"\t\t     OCEvent: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pMorphing->Asynchronous),
		BOOLEAN_TO_STRING_YES_NO_A(pMorphing->OCEvent));
}

VOID OutputGetConfigurationFeatureRemovableMedium(
	PFEATURE_DATA_REMOVABLE_MEDIUM pRemovableMedium
) {
	OutputDriveLogA(
		"\tFeatureRemovableMedium\n"
		"\t\t        Lockable: %s\n"
		"\t\tDefaultToPrevent: %s\n"
		"\t\t           Eject: %s\n"
		"\t\tLoadingMechanism: ",
		BOOLEAN_TO_STRING_YES_NO_A(pRemovableMedium->Lockable),
		BOOLEAN_TO_STRING_YES_NO_A(pRemovableMedium->DefaultToPrevent),
		BOOLEAN_TO_STRING_YES_NO_A(pRemovableMedium->Eject));
	switch (pRemovableMedium->LoadingMechanism) {
	case 0:
		OutputDriveLogA("Caddy/Slot type loading mechanism\n");
		break;
	case 1:
		OutputDriveLogA("Tray type loading mechanism\n");
		break;
	case 2:
		OutputDriveLogA("Pop-up type loading mechanism\n");
		break;
	case 4:
		OutputDriveLogA(
			"Embedded changer with individually changeable discs\n");
		break;
	case 5:
		OutputDriveLogA(
			"Embedded changer using a magazine mechanism\n");
		break;
	default:
		OutputDriveLogA(
			"Reserved: %08d\n", pRemovableMedium->LoadingMechanism);
		break;
	}
}

VOID OutputGetConfigurationFeatureWriteProtect(
	PFEATURE_DATA_WRITE_PROTECT pWriteProtect
) {
	OutputDriveLogA(
		"\tFeatureWriteProtect\n"
		"\t\t               SupportsSWPPBit: %s\n"
		"\t\tSupportsPersistentWriteProtect: %s\n"
		"\t\t               WriteInhibitDCB: %s\n"
		"\t\t           DiscWriteProtectPAC: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pWriteProtect->SupportsSWPPBit),
		BOOLEAN_TO_STRING_YES_NO_A(pWriteProtect->SupportsPersistentWriteProtect),
		BOOLEAN_TO_STRING_YES_NO_A(pWriteProtect->WriteInhibitDCB),
		BOOLEAN_TO_STRING_YES_NO_A(pWriteProtect->DiscWriteProtectPAC));
}

VOID OutputGetConfigurationFeatureRandomReadable(
	PFEATURE_DATA_RANDOM_READABLE pRandomReadable
) {
	OutputDriveLogA(
		"\tFeatureRandomReadable\n"
		"\t\t        LogicalBlockSize: %u\n"
		"\t\t                Blocking: %u\n"
		"\t\tErrorRecoveryPagePresent: %s\n",
		MAKEUINT(MAKEWORD(pRandomReadable->LogicalBlockSize[3], pRandomReadable->LogicalBlockSize[2]),
			MAKEWORD(pRandomReadable->LogicalBlockSize[1], pRandomReadable->LogicalBlockSize[0])),
		MAKEWORD(pRandomReadable->Blocking[1], pRandomReadable->Blocking[0]),
		BOOLEAN_TO_STRING_YES_NO_A(pRandomReadable->ErrorRecoveryPagePresent));
}

VOID OutputGetConfigurationFeatureMultiRead(
	PFEATURE_DATA_MULTI_READ pMultiRead
) {
	OutputDriveLogA(
		"\tFeatureMultiRead\n"
		"\t\t   Current: %u\n"
		"\t\tPersistent: %u\n"
		"\t\t   Version: %u\n",
		pMultiRead->Header.Current,
		pMultiRead->Header.Persistent,
		pMultiRead->Header.Version);
}

VOID OutputGetConfigurationFeatureCdRead(
	PFEATURE_DATA_CD_READ pCDRead
) {
	OutputDriveLogA(
		"\tFeatureCdRead\n"
		"\t\t          CDText: %s\n"
		"\t\t     C2ErrorData: %s\n"
		"\t\tDigitalAudioPlay: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pCDRead->CDText),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRead->C2ErrorData),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRead->DigitalAudioPlay));
}

VOID OutputGetConfigurationFeatureDvdRead(
	PFEATURE_DATA_DVD_READ pDVDRead
) {
	OutputDriveLogA(
		"\tFeatureDvdRead\n"
		"\t\t  Multi110: %s\n"
		"\t\t DualDashR: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRead->Multi110),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRead->DualDashR));
}

VOID OutputGetConfigurationFeatureRandomWritable(
	PFEATURE_DATA_RANDOM_WRITABLE pRandomWritable
) {
	OutputDriveLogA(
		"\tFeatureRandomWritable\n"
		"\t\t                 LastLBA: %u\n"
		"\t\t        LogicalBlockSize: %u\n"
		"\t\t                Blocking: %u\n"
		"\t\tErrorRecoveryPagePresent: %s\n",
		MAKEUINT(MAKEWORD(pRandomWritable->LastLBA[3], pRandomWritable->LastLBA[2]),
			MAKEWORD(pRandomWritable->LastLBA[1], pRandomWritable->LastLBA[0])),
		MAKEUINT(MAKEWORD(pRandomWritable->LogicalBlockSize[3], pRandomWritable->LogicalBlockSize[2]),
			MAKEWORD(pRandomWritable->LogicalBlockSize[1], pRandomWritable->LogicalBlockSize[0])),
		MAKEWORD(pRandomWritable->Blocking[1], pRandomWritable->Blocking[0]),
		BOOLEAN_TO_STRING_YES_NO_A(pRandomWritable->ErrorRecoveryPagePresent));
}

VOID OutputGetConfigurationFeatureIncrementalStreamingWritable(
	PFEATURE_DATA_INCREMENTAL_STREAMING_WRITABLE pIncremental
) {
	OutputDriveLogA(
		"\tFeatureIncrementalStreamingWritable\n"
		"\t\t        DataTypeSupported: %u\n"
		"\t\t       BufferUnderrunFree: %s\n"
		"\t\t   AddressModeReservation: %s\n"
		"\t\tTrackRessourceInformation: %s\n"
		"\t\t        NumberOfLinkSizes: %u\n",
		MAKEWORD(pIncremental->DataTypeSupported[1], pIncremental->DataTypeSupported[0]),
		BOOLEAN_TO_STRING_YES_NO_A(pIncremental->BufferUnderrunFree),
		BOOLEAN_TO_STRING_YES_NO_A(pIncremental->AddressModeReservation),
		BOOLEAN_TO_STRING_YES_NO_A(pIncremental->TrackRessourceInformation),
		pIncremental->NumberOfLinkSizes);
	for (INT i = 0; i < pIncremental->NumberOfLinkSizes; i++) {
		OutputDriveLogA(
			"\t\tLinkSize%u: %u\n", i, pIncremental->LinkSize[i]);
	}
}

VOID OutputGetConfigurationFeatureSectorErasable(
	PFEATURE_DATA_SECTOR_ERASABLE pSectorErasable
) {
	OutputDriveLogA(
		"\tFeatureSectorErasable\n"
		"\t\t   Current: %u\n"
		"\t\tPersistent: %u\n"
		"\t\t   Version: %u\n",
		pSectorErasable->Header.Current,
		pSectorErasable->Header.Persistent,
		pSectorErasable->Header.Version);
}

VOID OutputGetConfigurationFeatureFormattable(
	PFEATURE_DATA_FORMATTABLE pFormattable
) {
	OutputDriveLogA(
		"\tFeatureFormattable\n"
		"\t\t FullCertification: %s\n"
		"\t\tQuickCertification: %s\n"
		"\t\tSpareAreaExpansion: %s\n"
		"\t\tRENoSpareAllocated: %s\n"
		"\t\t   RRandomWritable: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pFormattable->FullCertification),
		BOOLEAN_TO_STRING_YES_NO_A(pFormattable->QuickCertification),
		BOOLEAN_TO_STRING_YES_NO_A(pFormattable->SpareAreaExpansion),
		BOOLEAN_TO_STRING_YES_NO_A(pFormattable->RENoSpareAllocated),
		BOOLEAN_TO_STRING_YES_NO_A(pFormattable->RRandomWritable));
}

VOID OutputGetConfigurationFeatureDefectManagement(
	PFEATURE_DATA_DEFECT_MANAGEMENT pDefect
) {
	OutputDriveLogA(
		"\tFeatureDefectManagement\n"
		"\t\tSupplimentalSpareArea: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDefect->SupplimentalSpareArea));
}

VOID OutputGetConfigurationFeatureWriteOnce(
	PFEATURE_DATA_WRITE_ONCE pWriteOnce
) {
	OutputDriveLogA(
		"\tFeatureWriteOnce\n"
		"\t\t        LogicalBlockSize: %u\n"
		"\t\t                Blocking: %u\n"
		"\t\tErrorRecoveryPagePresent: %s\n",
		MAKEUINT(MAKEWORD(pWriteOnce->LogicalBlockSize[3], pWriteOnce->LogicalBlockSize[2]),
			MAKEWORD(pWriteOnce->LogicalBlockSize[1], pWriteOnce->LogicalBlockSize[0])),
		MAKEWORD(pWriteOnce->Blocking[1], pWriteOnce->Blocking[0]),
		BOOLEAN_TO_STRING_YES_NO_A(pWriteOnce->ErrorRecoveryPagePresent));
}

VOID OutputGetConfigurationFeatureRestrictedOverwrite(
	PFEATURE_DATA_RESTRICTED_OVERWRITE pRestricted
) {
	OutputDriveLogA(
		"\tFeatureRestrictedOverwrite\n"
		"\t\t   Current: %u\n"
		"\t\tPersistent: %u\n"
		"\t\t   Version: %u\n",
		pRestricted->Header.Current,
		pRestricted->Header.Persistent,
		pRestricted->Header.Version);
}

VOID OutputGetConfigurationFeatureCdrwCAVWrite(
	PFEATURE_DATA_CDRW_CAV_WRITE pCDRW
) {
	OutputDriveLogA(
		"\tFeatureCdrwCAVWrite\n"
		"\t\t   Current: %u\n"
		"\t\tPersistent: %u\n"
		"\t\t   Version: %u\n",
		pCDRW->Header.Current,
		pCDRW->Header.Persistent,
		pCDRW->Header.Version);
}

VOID OutputGetConfigurationFeatureMrw(
	PFEATURE_DATA_MRW pMrw
) {
	OutputDriveLogA(
		"\tFeatureMrw\n"
		"\t\t       Write: %s\n"
		"\t\t DvdPlusRead: %s\n"
		"\t\tDvdPlusWrite: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pMrw->Write),
		BOOLEAN_TO_STRING_YES_NO_A(pMrw->DvdPlusRead),
		BOOLEAN_TO_STRING_YES_NO_A(pMrw->DvdPlusWrite));
}

VOID OutputGetConfigurationFeatureEnhancedDefectReporting(
	PFEATURE_ENHANCED_DEFECT_REPORTING pEnhanced
) {
	OutputDriveLogA(
		"\tFeatureEnhancedDefectReporting\n"
		"\t\t       DRTDMSupported: %s\n"
		"\t\tNumberOfDBICacheZones: %u\n"
		"\t\t      NumberOfEntries: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(pEnhanced->DRTDMSupported),
		pEnhanced->NumberOfDBICacheZones,
		MAKEWORD(pEnhanced->NumberOfEntries[1], pEnhanced->NumberOfEntries[0]));
}

VOID OutputGetConfigurationFeatureDvdPlusRW(
	PFEATURE_DATA_DVD_PLUS_RW pDVDPLUSRW
) {
	OutputDriveLogA(
		"\tFeatureDvdPlusRW\n"
		"\t\t     Write: %s\n"
		"\t\t CloseOnly: %s\n"
		"\t\tQuickStart: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPLUSRW->Write),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPLUSRW->CloseOnly),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPLUSRW->QuickStart));
}

VOID OutputGetConfigurationFeatureDvdPlusR(
	PFEATURE_DATA_DVD_PLUS_R pDVDPLUSR
) {
	OutputDriveLogA(
		"\tFeatureDvdPlusR\n"
		"\t\tWrite: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPLUSR->Write));
}

VOID OutputGetConfigurationFeatureRigidRestrictedOverwrite(
	PFEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE pDVDRWRestricted
) {
	OutputDriveLogA(
		"\tFeatureRigidRestrictedOverwrite\n"
		"\t\t                   Blank: %s\n"
		"\t\t            Intermediate: %s\n"
		"\t\t    DefectStatusDataRead: %s\n"
		"\t\tDefectStatusDataGenerate: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRWRestricted->Blank),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRWRestricted->Intermediate),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRWRestricted->DefectStatusDataRead),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRWRestricted->DefectStatusDataGenerate));
}

VOID OutputGetConfigurationFeatureCdTrackAtOnce(
	PFEATURE_DATA_CD_TRACK_AT_ONCE pCDTrackAtOnce
) {
	OutputDriveLogA(
		"\tFeatureCdTrackAtOnce\n"
		"\t\tRWSubchannelsRecordable: %s\n"
		"\t\t           CdRewritable: %s\n"
		"\t\t            TestWriteOk: %s\n"
		"\t\t   RWSubchannelPackedOk: %s\n"
		"\t\t      RWSubchannelRawOk: %s\n"
		"\t\t     BufferUnderrunFree: %s\n"
		"\t\t      DataTypeSupported: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(pCDTrackAtOnce->RWSubchannelsRecordable),
		BOOLEAN_TO_STRING_YES_NO_A(pCDTrackAtOnce->CdRewritable),
		BOOLEAN_TO_STRING_YES_NO_A(pCDTrackAtOnce->TestWriteOk),
		BOOLEAN_TO_STRING_YES_NO_A(pCDTrackAtOnce->RWSubchannelPackedOk),
		BOOLEAN_TO_STRING_YES_NO_A(pCDTrackAtOnce->RWSubchannelRawOk),
		BOOLEAN_TO_STRING_YES_NO_A(pCDTrackAtOnce->BufferUnderrunFree),
		MAKEWORD(pCDTrackAtOnce->DataTypeSupported[1], pCDTrackAtOnce->DataTypeSupported[0]));
}

VOID OutputGetConfigurationFeatureCdMastering(
	PFEATURE_DATA_CD_MASTERING pCDMastering
) {
	OutputDriveLogA(
		"\tFeatureCdMastering\n"
		"\t\tRWSubchannelsRecordable: %s\n"
		"\t\t           CdRewritable: %s\n"
		"\t\t            TestWriteOk: %s\n"
		"\t\t        RRawRecordingOk: %s\n"
		"\t\t      RawMultiSessionOk: %s\n"
		"\t\t        SessionAtOnceOk: %s\n"
		"\t\t     BufferUnderrunFree: %s\n"
		"\t\t  MaximumCueSheetLength: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(pCDMastering->RWSubchannelsRecordable),
		BOOLEAN_TO_STRING_YES_NO_A(pCDMastering->CdRewritable),
		BOOLEAN_TO_STRING_YES_NO_A(pCDMastering->TestWriteOk),
		BOOLEAN_TO_STRING_YES_NO_A(pCDMastering->RawRecordingOk),
		BOOLEAN_TO_STRING_YES_NO_A(pCDMastering->RawMultiSessionOk),
		BOOLEAN_TO_STRING_YES_NO_A(pCDMastering->SessionAtOnceOk),
		BOOLEAN_TO_STRING_YES_NO_A(pCDMastering->BufferUnderrunFree),
		MAKEUINT(MAKEWORD(0, pCDMastering->MaximumCueSheetLength[2]),
			MAKEWORD(pCDMastering->MaximumCueSheetLength[1], pCDMastering->MaximumCueSheetLength[0])));
}

VOID OutputGetConfigurationFeatureDvdRecordableWrite(
	PFEATURE_DATA_DVD_RECORDABLE_WRITE pDVDRecordable
) {
	OutputDriveLogA(
		"\tFeatureDvdRecordableWrite\n"
		"\t\t            DVD_RW: %s\n"
		"\t\t         TestWrite: %s\n"
		"\t\t        RDualLayer: %s\n"
		"\t\tBufferUnderrunFree: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRecordable->DVD_RW),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRecordable->TestWrite),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRecordable->RDualLayer),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRecordable->BufferUnderrunFree));
}

VOID OutputGetConfigurationFeatureLayerJumpRecording(
	PFEATURE_DATA_LAYER_JUMP_RECORDING pLayerJumpRec
) {
	OutputDriveLogA(
		"\tFeatureLayerJumpRecording\n"
		"\t\tNumberOfLinkSizes: %u\n",
		pLayerJumpRec->NumberOfLinkSizes);
	for (INT i = 0; i < pLayerJumpRec->NumberOfLinkSizes; i++) {
		OutputDriveLogA(
			"\t\tLinkSize %u: %u\n", i, pLayerJumpRec->LinkSizes[i]);
	}
}

VOID OutputGetConfigurationFeatureCDRWMediaWriteSupport(
	PFEATURE_CD_RW_MEDIA_WRITE_SUPPORT pCDRWMediaWrite
) {
	OutputDriveLogA(
		"\tFeatureCDRWMediaWriteSupport\n"
		"\t\tSubtype 0: %s\n"
		"\t\tSubtype 1: %s\n"
		"\t\tSubtype 2: %s\n"
		"\t\tSubtype 3: %s\n"
		"\t\tSubtype 4: %s\n"
		"\t\tSubtype 5: %s\n"
		"\t\tSubtype 6: %s\n"
		"\t\tSubtype 7: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype0),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype1),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype2),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype3),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype4),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype5),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype6),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype7));
}

VOID OutputGetConfigurationFeatureBDRPseudoOverwrite(
	PFEATURE_BD_R_PSEUDO_OVERWRITE pBDRPseudoOverwrite
) {
	UNREFERENCED_PARAMETER(pBDRPseudoOverwrite);
	OutputDriveLogA(
		"\tFeatureBDRPseudoOverwrite\n");
}

VOID OutputGetConfigurationFeatureDvdPlusRWDualLayer(
	PFEATURE_DATA_DVD_PLUS_RW_DUAL_LAYER pDVDPlusRWDL
) {
	OutputDriveLogA(
		"\tFeatureDvdPlusRWDualLayer\n"
		"\t\t     Write: %s\n"
		"\t\t CloseOnly: %s\n"
		"\t\tQuickStart: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPlusRWDL->Write),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPlusRWDL->CloseOnly),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPlusRWDL->QuickStart));
}

VOID OutputGetConfigurationFeatureDvdPlusRDualLayer(
	PFEATURE_DATA_DVD_PLUS_R_DUAL_LAYER pDVDPlusRDL
) {
	OutputDriveLogA(
		"\tFeatureDvdPlusRDualLayer\n"
		"\t\tWrite: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPlusRDL->Write));
}

VOID OutputGetConfigurationFeatureBDRead(
	PFEATURE_BD_READ pBDRead
) {
	OutputDriveLogA(
		"\tFeatureBDRead\n"
		"\t\tClass0BitmapBDREReadSupport Version8: %s\n"
		"\t\tClass0BitmapBDREReadSupport Version9: %s\n"
		"\t\tClass0BitmapBDREReadSupport Version10: %s\n"
		"\t\tClass0BitmapBDREReadSupport Version11: %s\n"
		"\t\tClass0BitmapBDREReadSupport Version12: %s\n"
		"\t\tClass0BitmapBDREReadSupport Version13: %s\n"
		"\t\tClass0BitmapBDREReadSupport Version14: %s\n"
		"\t\tClass0BitmapBDREReadSupport Version15: %s\n"
		"\t\tClass0BitmapBDREReadSupport Version0: %s\n"
		"\t\tClass0BitmapBDREReadSupport Version1: %s\n"
		"\t\tClass0BitmapBDREReadSupport Version2: %s\n"
		"\t\tClass0BitmapBDREReadSupport Version3: %s\n"
		"\t\tClass0BitmapBDREReadSupport Version4: %s\n"
		"\t\tClass0BitmapBDREReadSupport Version5: %s\n"
		"\t\tClass0BitmapBDREReadSupport Version6: %s\n"
		"\t\tClass0BitmapBDREReadSupport Version7: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version8: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version9: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version10: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version11: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version12: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version13: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version14: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version15: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version0: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version1: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version2: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version3: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version4: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version5: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version6: %s\n"
		"\t\tClass1BitmapBDREReadSupport Version7: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version8: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version9: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version10: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version11: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version12: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version13: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version14: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version15: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version0: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version1: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version2: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version3: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version4: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version5: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version6: %s\n"
		"\t\tClass2BitmapBDREReadSupport Version7: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version8: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version9: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version10: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version11: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version12: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version13: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version14: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version15: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version0: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version1: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version2: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version3: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version4: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version5: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version6: %s\n"
		"\t\tClass3BitmapBDREReadSupport Version7: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version8: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version9: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version10: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version11: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version12: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version13: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version14: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version15: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version0: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version1: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version2: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version3: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version4: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version5: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version6: %s\n"
		"\t\tClass0BitmapBDRReadSupport Version7: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version8: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version9: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version10: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version11: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version12: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version13: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version14: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version15: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version0: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version1: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version2: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version3: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version4: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version5: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version6: %s\n"
		"\t\tClass1BitmapBDRReadSupport Version7: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version8: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version9: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version10: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version11: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version12: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version13: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version14: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version15: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version0: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version1: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version2: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version3: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version4: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version5: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version6: %s\n"
		"\t\tClass2BitmapBDRReadSupport Version7: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version8: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version9: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version10: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version11: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version12: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version13: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version14: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version15: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version0: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version1: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version2: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version3: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version4: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version5: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version6: %s\n"
		"\t\tClass3BitmapBDRReadSupport Version7: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version8: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version9: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version10: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version11: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version12: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version13: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version14: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version15: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version0: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version1: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version2: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version3: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version4: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version5: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version6: %s\n"
		"\t\tClass0BitmapBDROMReadSupport Version7: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version8: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version9: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version10: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version11: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version12: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version13: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version14: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version15: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version0: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version1: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version2: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version3: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version4: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version5: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version6: %s\n"
		"\t\tClass1BitmapBDROMReadSupport Version7: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version8: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version9: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version10: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version11: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version12: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version13: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version14: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version15: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version0: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version1: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version2: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version3: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version4: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version5: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version6: %s\n"
		"\t\tClass2BitmapBDROMReadSupport Version7: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version8: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version9: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version10: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version11: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version12: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version13: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version14: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version15: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version0: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version1: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version2: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version3: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version4: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version5: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version6: %s\n"
		"\t\tClass3BitmapBDROMReadSupport Version7: %s\n"
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDREReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDREReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDREReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDREReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDRReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDRReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDRReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDRReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class0BitmapBDROMReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class1BitmapBDROMReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class2BitmapBDROMReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDRead->Class3BitmapBDROMReadSupport.Version7));
}

VOID OutputGetConfigurationFeatureBDWrite(
	PFEATURE_BD_WRITE pBDWrite
) {
	OutputDriveLogA(
		"\tFeatureBDWrite\n"
		"\t\tSupportsVerifyNotRequired: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version8: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version9: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version10: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version11: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version12: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version13: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version14: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version15: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version0: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version1: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version2: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version3: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version4: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version5: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version6: %s\n"
		"\t\tClass0BitmapBDREWriteSupport Version7: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version8: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version9: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version10: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version11: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version12: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version13: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version14: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version15: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version0: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version1: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version2: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version3: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version4: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version5: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version6: %s\n"
		"\t\tClass1BitmapBDREWriteSupport Version7: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version8: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version9: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version10: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version11: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version12: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version13: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version14: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version15: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version0: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version1: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version2: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version3: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version4: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version5: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version6: %s\n"
		"\t\tClass2BitmapBDREWriteSupport Version7: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version8: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version9: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version10: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version11: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version12: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version13: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version14: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version15: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version0: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version1: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version2: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version3: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version4: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version5: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version6: %s\n"
		"\t\tClass3BitmapBDREWriteSupport Version7: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version8: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version9: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version10: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version11: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version12: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version13: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version14: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version15: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version0: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version1: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version2: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version3: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version4: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version5: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version6: %s\n"
		"\t\tClass0BitmapBDRWriteSupport Version7: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version8: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version9: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version10: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version11: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version12: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version13: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version14: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version15: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version0: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version1: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version2: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version3: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version4: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version5: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version6: %s\n"
		"\t\tClass1BitmapBDRWriteSupport Version7: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version8: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version9: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version10: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version11: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version12: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version13: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version14: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version15: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version0: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version1: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version2: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version3: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version4: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version5: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version6: %s\n"
		"\t\tClass2BitmapBDRWriteSupport Version7: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version8: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version9: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version10: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version11: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version12: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version13: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version14: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version15: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version0: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version1: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version2: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version3: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version4: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version5: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version6: %s\n"
		"\t\tClass3BitmapBDRWriteSupport Version7: %s\n"
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->SupportsVerifyNotRequired)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDREWriteSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDREWriteSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDREWriteSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDREWriteSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class0BitmapBDRWriteSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class1BitmapBDRWriteSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class2BitmapBDRWriteSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO_A(pBDWrite->Class3BitmapBDRWriteSupport.Version7));
}

VOID OutputGetConfigurationFeatureTSR(
	PFEATURE_TSR pTsr
) {
	UNREFERENCED_PARAMETER(pTsr);
	OutputDriveLogA(
		"\tFeatureTSR\n");
}

VOID OutputGetConfigurationFeatureHDDVDRead(
	PFEATURE_DATA_HDDVD_READ pHdRead
) {
	OutputDriveLogA(
		"\tFeatureHDDVDRead\n"
		"\t\tRecordable: %s\n"
		"\t\tRewritable: %s\n"
		, BOOLEAN_TO_STRING_YES_NO_A(pHdRead->Recordable)
		, BOOLEAN_TO_STRING_YES_NO_A(pHdRead->Rewritable));
}

VOID OutputGetConfigurationFeatureHDDVDWrite(
	PFEATURE_DATA_HDDVD_WRITE pHdWrite
) {
	OutputDriveLogA(
		"\tFeatureHDDVDWrite\n"
		"\t\tRecordable: %s\n"
		"\t\tRewritable: %s\n"
		, BOOLEAN_TO_STRING_YES_NO_A(pHdWrite->Recordable)
		, BOOLEAN_TO_STRING_YES_NO_A(pHdWrite->Rewritable));
}

VOID OutputGetConfigurationFeatureHybridDisc(
	PFEATURE_HYBRID_DISC pHybridDisc
) {
	OutputDriveLogA(
		"\tFeatureHybridDisc\n"
		"\t\tResetImmunity: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pHybridDisc->ResetImmunity));
}

VOID OutputGetConfigurationFeaturePowerManagement(
	PFEATURE_DATA_POWER_MANAGEMENT pPower
) {
	OutputDriveLogA(
		"\tFeaturePowerManagement\n"
		"\t\t   Current: %u\n"
		"\t\tPersistent: %u\n"
		"\t\t   Version: %u\n",
		pPower->Header.Current,
		pPower->Header.Persistent,
		pPower->Header.Version);
}

VOID OutputGetConfigurationFeatureSMART(
	PFEATURE_DATA_SMART pSmart
) {
	OutputDriveLogA(
		"\tFeatureSMART\n"
		"\t\tFaultFailureReportingPagePresent: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pSmart->FaultFailureReportingPagePresent));
}

VOID OutputGetConfigurationFeatureEmbeddedChanger(
	PFEATURE_DATA_EMBEDDED_CHANGER pEmbedded
) {
	OutputDriveLogA(
		"\tFeatureEmbeddedChanger\n"
		"\t\tSupportsDiscPresent: %s\n"
		"\t\t  SideChangeCapable: %s\n"
		"\t\t  HighestSlotNumber: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(pEmbedded->SupportsDiscPresent),
		BOOLEAN_TO_STRING_YES_NO_A(pEmbedded->SideChangeCapable),
		pEmbedded->HighestSlotNumber);
}

VOID OutputGetConfigurationFeatureCDAudioAnalogPlay(
	PFEATURE_DATA_CD_AUDIO_ANALOG_PLAY pCDAudio
) {
	OutputDriveLogA(
		"\tFeatureCDAudioAnalogPlay\n"
		"\t\t     SeperateVolume: %s\n"
		"\t\tSeperateChannelMute: %s\n"
		"\t\t      ScanSupported: %s\n"
		"\t\tNumerOfVolumeLevels: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(pCDAudio->SeperateVolume),
		BOOLEAN_TO_STRING_YES_NO_A(pCDAudio->SeperateChannelMute),
		BOOLEAN_TO_STRING_YES_NO_A(pCDAudio->ScanSupported),
		MAKEWORD(pCDAudio->NumerOfVolumeLevels[1], pCDAudio->NumerOfVolumeLevels[0]));
}

VOID OutputGetConfigurationFeatureMicrocodeUpgrade(
	PFEATURE_DATA_MICROCODE_UPDATE pMicrocode
) {
	OutputDriveLogA(
		"\tFeatureMicrocodeUpgrade\n"
		"\t\tM5: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pMicrocode->M5));
}

VOID OutputGetConfigurationFeatureTimeout(
	PFEATURE_DATA_TIMEOUT pTimeOut
) {
	OutputDriveLogA(
		"\tFeatureTimeout\n"
		"\t\t    Group3: %s\n"
		"\t\tUnitLength: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(pTimeOut->Group3),
		MAKEWORD(pTimeOut->UnitLength[1], pTimeOut->UnitLength[0]));
}

VOID OutputGetConfigurationFeatureDvdCSS(
	PFEATURE_DATA_DVD_CSS pDVDCss
) {
	OutputDriveLogA(
		"\tFeatureDvdCSS\n"
		"\t\tCssVersion: %u\n",
		pDVDCss->CssVersion);
}

VOID OutputGetConfigurationFeatureRealTimeStreaming(
	PFEATURE_DATA_REAL_TIME_STREAMING pRealTimeStreaming
) {
	OutputDriveLogA(
		"\tFeatureRealTimeStreaming\n"
		"\t\t        StreamRecording: %s\n"
		"\t\t    WriteSpeedInGetPerf: %s\n"
		"\t\t       WriteSpeedInMP2A: %s\n"
		"\t\t             SetCDSpeed: %s\n"
		"\t\tReadBufferCapacityBlock: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pRealTimeStreaming->StreamRecording),
		BOOLEAN_TO_STRING_YES_NO_A(pRealTimeStreaming->WriteSpeedInGetPerf),
		BOOLEAN_TO_STRING_YES_NO_A(pRealTimeStreaming->WriteSpeedInMP2A),
		BOOLEAN_TO_STRING_YES_NO_A(pRealTimeStreaming->SetCDSpeed),
		BOOLEAN_TO_STRING_YES_NO_A(pRealTimeStreaming->ReadBufferCapacityBlock));
}

VOID OutputGetConfigurationFeatureLogicalUnitSerialNumber(
	PFEATURE_DATA_LOGICAL_UNIT_SERIAL_NUMBER pLogical
) {
	OutputDriveLogA(
		"\tFeatureLogicalUnitSerialNumber\n"
		"\t\tSerialNumber: ");
	for (INT i = 0; i < pLogical->Header.AdditionalLength; i++) {
		OutputDriveLogA("%c", pLogical->SerialNumber[i]);
	}
	OutputDriveLogA("\n");
}

VOID OutputGetConfigurationFeatureMediaSerialNumber(
	PFEATURE_MEDIA_SERIAL_NUMBER pMediaSerialNumber
) {
	OutputDriveLogA(
		"\tFeatureMediaSerialNumber\n"
		"\t\t   Current: %u\n"
		"\t\tPersistent: %u\n"
		"\t\t   Version: %u\n",
		pMediaSerialNumber->Header.Current,
		pMediaSerialNumber->Header.Persistent,
		pMediaSerialNumber->Header.Version);
}

VOID OutputGetConfigurationFeatureDiscControlBlocks(
	PFEATURE_DATA_DISC_CONTROL_BLOCKS pDiscCtrlBlk
) {
	OutputDriveLogA("\tFeatureDiscControlBlocks\n");
	for (INT i = 0; i < pDiscCtrlBlk->Header.AdditionalLength; i++) {
		OutputDriveLogA(
			"\t\tContentDescriptor %02u: %08ld\n", i,
			MAKELONG(
				MAKEWORD(pDiscCtrlBlk->Data[i].ContentDescriptor[3], pDiscCtrlBlk->Data[i].ContentDescriptor[2]),
				MAKEWORD(pDiscCtrlBlk->Data[i].ContentDescriptor[1], pDiscCtrlBlk->Data[i].ContentDescriptor[0])));
	}
}

VOID OutputGetConfigurationFeatureDvdCPRM(
	PFEATURE_DATA_DVD_CPRM pDVDCprm
) {
	OutputDriveLogA(
		"\tFeatureDvdCPRM\n"
		"\t\tCPRMVersion: %u\n",
		pDVDCprm->CPRMVersion);
}

VOID OutputGetConfigurationFeatureFirmwareDate(
	PFEATURE_DATA_FIRMWARE_DATE pFirmwareDate
) {
	OutputDriveLogA(
		"\tFeatureFirmwareDate: %.4s-%.2s-%.2s %.2s:%.2s:%.2s\n"
		, pFirmwareDate->Year, pFirmwareDate->Month, pFirmwareDate->Day
		, pFirmwareDate->Hour, pFirmwareDate->Minute, pFirmwareDate->Seconds);
}

VOID OutputGetConfigurationFeatureAACS(
	PFEATURE_DATA_AACS pAACS
) {
	OutputDriveLogA(
		"\tFeatureAACS\n"
		"\t\tBindingNonceGeneration: %s\n"
		"\t\tBindingNonceBlockCount: %u\n"
		"\t\t         NumberOfAGIDs: %u\n"
		"\t\t           AACSVersion: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(pAACS->BindingNonceGeneration),
		pAACS->BindingNonceBlockCount,
		pAACS->NumberOfAGIDs,
		pAACS->AACSVersion);
}

VOID OutputGetConfigurationFeatureVCPS(
	PFEATURE_VCPS pVcps
) {
	OutputDriveLogA(
		"\tFeatureVCPS\n"
		"\t\t   Current: %u\n"
		"\t\tPersistent: %u\n"
		"\t\t   Version: %u\n",
		pVcps->Header.Current,
		pVcps->Header.Persistent,
		pVcps->Header.Version);
}

VOID OutputGetConfigurationFeatureReserved(
	PFEATURE_DATA_RESERVED pReserved
) {
	OutputDriveLogA(
		"\tReserved (FeatureCode[%#04x])\n"
		"\t\tData: ", MAKEWORD(pReserved->Header.FeatureCode[1], pReserved->Header.FeatureCode[0]));
	for (INT i = 0; i < pReserved->Header.AdditionalLength; i++) {
		OutputDriveLogA("%02x", pReserved->Data[i]);
	}
	OutputDriveLogA("\n");
}

VOID OutputGetConfigurationFeatureVendorSpecific(
	PFEATURE_DATA_VENDOR_SPECIFIC pVendorSpecific
) {
	OutputDriveLogA(
		"\tVendorSpecific (FeatureCode[%#04x])\n"
		"\t\tVendorSpecificData: ",
		MAKEWORD(pVendorSpecific->Header.FeatureCode[1], pVendorSpecific->Header.FeatureCode[0]));
	for (INT i = 0; i < pVendorSpecific->Header.AdditionalLength; i++) {
		OutputDriveLogA("%02x", pVendorSpecific->VendorSpecificData[i]);
	}
	OutputDriveLogA("\n");
}

VOID OutputGetConfigurationFeatureNumber(
	PDEVICE pDevice,
	LPBYTE lpConf,
	DWORD dwAllLen
) {
	DWORD n = 0;
	while (n < dwAllLen) {
		WORD wCode = MAKEWORD(lpConf[n + 1], lpConf[n]);
		switch (wCode) {
		case FeatureProfileList:
			OutputGetConfigurationFeatureProfileList((PFEATURE_DATA_PROFILE_LIST)&lpConf[n]);
			break;
		case FeatureCore:
			OutputGetConfigurationFeatureCore((PFEATURE_DATA_CORE)&lpConf[n]);
			break;
		case FeatureMorphing:
			OutputGetConfigurationFeatureMorphing((PFEATURE_DATA_MORPHING)&lpConf[n]);
			break;
		case FeatureRemovableMedium:
			OutputGetConfigurationFeatureRemovableMedium((PFEATURE_DATA_REMOVABLE_MEDIUM)&lpConf[n]);
			break;
		case FeatureWriteProtect:
			OutputGetConfigurationFeatureWriteProtect((PFEATURE_DATA_WRITE_PROTECT)&lpConf[n]);
			break;
		case FeatureRandomReadable:
			OutputGetConfigurationFeatureRandomReadable((PFEATURE_DATA_RANDOM_READABLE)&lpConf[n]);
			break;
		case FeatureMultiRead:
			OutputGetConfigurationFeatureMultiRead((PFEATURE_DATA_MULTI_READ)&lpConf[n]);
			break;
		case FeatureCdRead:
			OutputGetConfigurationFeatureCdRead((PFEATURE_DATA_CD_READ)&lpConf[n]);
			SetFeatureCdRead((PFEATURE_DATA_CD_READ)&lpConf[n], pDevice);
			break;
		case FeatureDvdRead:
			OutputGetConfigurationFeatureDvdRead((PFEATURE_DATA_DVD_READ)&lpConf[n]);
			break;
		case FeatureRandomWritable:
			OutputGetConfigurationFeatureRandomWritable((PFEATURE_DATA_RANDOM_WRITABLE)&lpConf[n]);
			break;
		case FeatureIncrementalStreamingWritable:
			OutputGetConfigurationFeatureIncrementalStreamingWritable((PFEATURE_DATA_INCREMENTAL_STREAMING_WRITABLE)&lpConf[n]);
			break;
		case FeatureSectorErasable:
			OutputGetConfigurationFeatureSectorErasable((PFEATURE_DATA_SECTOR_ERASABLE)&lpConf[n]);
			break;
		case FeatureFormattable:
			OutputGetConfigurationFeatureFormattable((PFEATURE_DATA_FORMATTABLE)&lpConf[n]);
			break;
		case FeatureDefectManagement:
			OutputGetConfigurationFeatureDefectManagement((PFEATURE_DATA_DEFECT_MANAGEMENT)&lpConf[n]);
			break;
		case FeatureWriteOnce:
			OutputGetConfigurationFeatureWriteOnce((PFEATURE_DATA_WRITE_ONCE)&lpConf[n]);
			break;
		case FeatureRestrictedOverwrite:
			OutputGetConfigurationFeatureRestrictedOverwrite((PFEATURE_DATA_RESTRICTED_OVERWRITE)&lpConf[n]);
			break;
		case FeatureCdrwCAVWrite:
			OutputGetConfigurationFeatureCdrwCAVWrite((PFEATURE_DATA_CDRW_CAV_WRITE)&lpConf[n]);
			break;
		case FeatureMrw:
			OutputGetConfigurationFeatureMrw((PFEATURE_DATA_MRW)&lpConf[n]);
			break;
		case FeatureEnhancedDefectReporting:
			OutputGetConfigurationFeatureEnhancedDefectReporting((PFEATURE_ENHANCED_DEFECT_REPORTING)&lpConf[n]);
			break;
		case FeatureDvdPlusRW:
			OutputGetConfigurationFeatureDvdPlusRW((PFEATURE_DATA_DVD_PLUS_RW)&lpConf[n]);
			break;
		case FeatureDvdPlusR:
			OutputGetConfigurationFeatureDvdPlusR((PFEATURE_DATA_DVD_PLUS_R)&lpConf[n]);
			break;
		case FeatureRigidRestrictedOverwrite:
			OutputGetConfigurationFeatureRigidRestrictedOverwrite((PFEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE)&lpConf[n]);
			break;
		case FeatureCdTrackAtOnce:
			OutputGetConfigurationFeatureCdTrackAtOnce((PFEATURE_DATA_CD_TRACK_AT_ONCE)&lpConf[n]);
			break;
		case FeatureCdMastering:
			OutputGetConfigurationFeatureCdMastering((PFEATURE_DATA_CD_MASTERING)&lpConf[n]);
			break;
		case FeatureDvdRecordableWrite:
			OutputGetConfigurationFeatureDvdRecordableWrite((PFEATURE_DATA_DVD_RECORDABLE_WRITE)&lpConf[n]);
			break;
		case FeatureDDCDRead:
			break;
		case FeatureDDCDRWrite:
			break;
		case FeatureDDCDRWWrite:
			break;
		case FeatureLayerJumpRecording:
			OutputGetConfigurationFeatureLayerJumpRecording((PFEATURE_DATA_LAYER_JUMP_RECORDING)&lpConf[n]);
			break;
		case FeatureCDRWMediaWriteSupport:
			OutputGetConfigurationFeatureCDRWMediaWriteSupport((PFEATURE_CD_RW_MEDIA_WRITE_SUPPORT)&lpConf[n]);
			break;
		case FeatureBDRPseudoOverwrite:
			OutputGetConfigurationFeatureBDRPseudoOverwrite((PFEATURE_BD_R_PSEUDO_OVERWRITE)&lpConf[n]);
			break;
		case FeatureDvdPlusRWDualLayer:
			OutputGetConfigurationFeatureDvdPlusRWDualLayer((PFEATURE_DATA_DVD_PLUS_RW_DUAL_LAYER)&lpConf[n]);
			break;
		case FeatureDvdPlusRDualLayer:
			OutputGetConfigurationFeatureDvdPlusRDualLayer((PFEATURE_DATA_DVD_PLUS_R_DUAL_LAYER)&lpConf[n]);
			break;
		case FeatureBDRead:
			OutputGetConfigurationFeatureBDRead((PFEATURE_BD_READ)&lpConf[n]);
			break;
		case FeatureBDWrite:
			OutputGetConfigurationFeatureBDWrite((PFEATURE_BD_WRITE)&lpConf[n]);
			break;
		case FeatureTSR:
			OutputGetConfigurationFeatureTSR((PFEATURE_TSR)&lpConf[n]);
			break;
		case FeatureHDDVDRead:
			OutputGetConfigurationFeatureHDDVDRead((PFEATURE_DATA_HDDVD_READ)&lpConf[n]);
			break;
		case FeatureHDDVDWrite:
			OutputGetConfigurationFeatureHDDVDWrite((PFEATURE_DATA_HDDVD_WRITE)&lpConf[n]);
			break;
		case FeatureHybridDisc:
			OutputGetConfigurationFeatureHybridDisc((PFEATURE_HYBRID_DISC)&lpConf[n]);
			break;
		case FeaturePowerManagement:
			OutputGetConfigurationFeaturePowerManagement((PFEATURE_DATA_POWER_MANAGEMENT)&lpConf[n]);
			break;
		case FeatureSMART:
			OutputGetConfigurationFeatureSMART((PFEATURE_DATA_SMART)&lpConf[n]);
			break;
		case FeatureEmbeddedChanger:
			OutputGetConfigurationFeatureEmbeddedChanger((PFEATURE_DATA_EMBEDDED_CHANGER)&lpConf[n]);
			break;
		case FeatureCDAudioAnalogPlay:
			OutputGetConfigurationFeatureCDAudioAnalogPlay((PFEATURE_DATA_CD_AUDIO_ANALOG_PLAY)&lpConf[n]);
			break;
		case FeatureMicrocodeUpgrade:
			OutputGetConfigurationFeatureMicrocodeUpgrade((PFEATURE_DATA_MICROCODE_UPDATE)&lpConf[n]);
			break;
		case FeatureTimeout:
			OutputGetConfigurationFeatureTimeout((PFEATURE_DATA_TIMEOUT)&lpConf[n]);
			break;
		case FeatureDvdCSS:
			OutputGetConfigurationFeatureDvdCSS((PFEATURE_DATA_DVD_CSS)&lpConf[n]);
			break;
		case FeatureRealTimeStreaming:
			OutputGetConfigurationFeatureRealTimeStreaming((PFEATURE_DATA_REAL_TIME_STREAMING)&lpConf[n]);
			SetFeatureRealTimeStreaming((PFEATURE_DATA_REAL_TIME_STREAMING)&lpConf[n], pDevice);
			break;
		case FeatureLogicalUnitSerialNumber:
			OutputGetConfigurationFeatureLogicalUnitSerialNumber((PFEATURE_DATA_LOGICAL_UNIT_SERIAL_NUMBER)&lpConf[n]);
			break;
		case FeatureMediaSerialNumber:
			OutputGetConfigurationFeatureMediaSerialNumber((PFEATURE_MEDIA_SERIAL_NUMBER)&lpConf[n]);
			break;
		case FeatureDiscControlBlocks:
			OutputGetConfigurationFeatureDiscControlBlocks((PFEATURE_DATA_DISC_CONTROL_BLOCKS)&lpConf[n]);
			break;
		case FeatureDvdCPRM:
			OutputGetConfigurationFeatureDvdCPRM((PFEATURE_DATA_DVD_CPRM)&lpConf[n]);
			break;
		case FeatureFirmwareDate:
			OutputGetConfigurationFeatureFirmwareDate((PFEATURE_DATA_FIRMWARE_DATE)&lpConf[n]);
			break;
		case FeatureAACS:
			OutputGetConfigurationFeatureAACS((PFEATURE_DATA_AACS)&lpConf[n]);
			break;
		case FeatureVCPS:
			OutputGetConfigurationFeatureVCPS((PFEATURE_VCPS)&lpConf[n]);
			break;
		default:
			if (0x0111 <= wCode && wCode <= 0xfeff) {
				OutputGetConfigurationFeatureReserved((PFEATURE_DATA_RESERVED)&lpConf[n]);
			}
			else if (0xff00 <= wCode/* && wCode <= 0xffff*/) {
				OutputGetConfigurationFeatureVendorSpecific((PFEATURE_DATA_VENDOR_SPECIFIC)&lpConf[n]);
			}
			break;
		}
		n += sizeof(FEATURE_HEADER) + lpConf[n + 3];
	}
}

VOID OutputCDAtip(
	PCDROM_TOC_ATIP_DATA_BLOCK pAtip
) {
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(TOC ATIP)
		"\tCdrwReferenceSpeed: %u\n"
		"\t        WritePower: %u\n"
		"\t   UnrestrictedUse: %s\n"
		, pAtip->CdrwReferenceSpeed
		, pAtip->WritePower
		, BOOLEAN_TO_STRING_YES_NO_A(pAtip->UnrestrictedUse)
	);
	switch (pAtip->IsCdrw)
	{
	case 0:	OutputDiscLogA("\t          DiscType: CD-R, DiscSubType: %u\n", pAtip->DiscSubType);
		break;
	case 1:	OutputDiscLogA("\t          DiscType: CD-RW, ");
		switch (pAtip->DiscSubType)
		{
		case 0: OutputDiscLogA("DiscSubType: Standard Speed\n");
			break;
		case 1: OutputDiscLogA("DiscSubType: High Speed\n");
			break;
		default: OutputDiscLogA("DiscSubType: Unknown\n");
			break;
		}
		break;
	default: OutputDiscLogA("          DiscType: Unknown\n");
		break;
	}
	OutputDiscLogA(
		"\t         LeadInMsf: %02u:%02u:%02u\n"
		"\t        LeadOutMsf: %02u:%02u:%02u\n"
		, pAtip->LeadInMsf[0], pAtip->LeadInMsf[1], pAtip->LeadInMsf[2]
		, pAtip->LeadOutMsf[0], pAtip->LeadOutMsf[1], pAtip->LeadOutMsf[2]
	);
	if (pAtip->A1Valid) {
		OutputDiscLogA(
			"\t A1Values: %02u:%02u:%02u\n"
			, pAtip->A1Values[0], pAtip->A1Values[1], pAtip->A1Values[2]
		);
	}
	if (pAtip->A2Valid) {
		OutputDiscLogA(
			"\t A2Values: %02u:%02u:%02u\n"
			, pAtip->A2Values[0], pAtip->A2Values[1], pAtip->A2Values[2]
		);
	}
	if (pAtip->A3Valid) {
		OutputDiscLogA(
			"\t A3Values: %02u:%02u:%02u\n"
			, pAtip->A3Values[0], pAtip->A3Values[1], pAtip->A3Values[2]
		);
	}
}

VOID OutputCDTextOther(
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	WORD wTocTextEntries,
	BYTE bySizeInfoIdx,
	UINT uiSizeInfoCnt
) {
	INT nTocInfoCnt = 0;
	INT nSizeInfoCnt = 0;
	for (size_t z = 0; z <= bySizeInfoIdx; z++) {
		if (pDesc[z].PackType == CDROM_CD_TEXT_PACK_TOC_INFO) {
			// detail in Page 54-55 of EN 60908:1999
			OutputDiscLogA("\tTocInfo\n");
			if (nTocInfoCnt == 0) {
				OutputDiscLogA(
					"\t\t  First track number: %u\n"
					"\t\t   Last track number: %u\n"
					"\t\t       Lead-out(msf): %u:%u:%u\n"
					, pDesc[z].Text[0],	pDesc[z].Text[1], pDesc[z].Text[3]
					, pDesc[z].Text[4], pDesc[z].Text[5]);
			}
			if (nTocInfoCnt == 1) {
				for (INT i = 0, j = 0; i < 4; i++, j += 3) {
					OutputDiscLogA(
						"\t\t    Track %d(msf): %u:%u:%u\n"
						, i + 1, pDesc[z].Text[j], pDesc[z].Text[j + 1], pDesc[z].Text[j + 2]);
				}
			}
			nTocInfoCnt++;
		}
		else if (pDesc[z].PackType == CDROM_CD_TEXT_PACK_TOC_INFO2) {
			OutputDiscLogA(
				"\tTocInfo2\n"
				"\t\t         Priority number: %u\n"
				"\t\t     Number of intervals: %u\n"
				"\t\t    Start point(minutes): %u\n"
				"\t\t    Start point(seconds): %u\n"
				"\t\t     Start point(frames): %u\n"
				"\t\t      End point(minutes): %u\n"
				"\t\t      End point(seconds): %u\n"
				"\t\t       End point(frames): %u\n"
				, pDesc[z].Text[0], pDesc[z].Text[1], pDesc[z].Text[6], pDesc[z].Text[7]
				, pDesc[z].Text[8], pDesc[z].Text[9], pDesc[z].Text[10], pDesc[z].Text[11]);
		}
		else if (pDesc[z].PackType == CDROM_CD_TEXT_PACK_SIZE_INFO) {
			// detail in Page 56 of EN 60908:1999
			OutputDiscLogA("\tSizeInfo\n");
			if (nSizeInfoCnt == 0) {
				BYTE ch = pDesc[wTocTextEntries - uiSizeInfoCnt].Text[0];
				OutputDiscLogA(
					"\t\t         Charactor Code for this BLOCK: 0x%02x ", ch);
				switch (ch) {
				case 0x00:
					OutputDiscLogA("(ISO/IEC 8859-1 [Latin-1])\n");
					break;
				case 0x01:
					OutputDiscLogA("(ISO/IEC 646 [ASCII])\n");
					break;
				case 0x80:
					OutputDiscLogA("(Shift-JIS)\n");
					break;
				case 0x81:
					OutputDiscLogA("(Korean character code)\n");
					break;
				case 0x82:
					OutputDiscLogA("(Mandarin Chinese character code)\n");
					break;
				default:
					OutputDiscLogA("(Reserved)\n");
					break;
				}
				OutputDiscLogA(
					"\t\t                    First track Number: %u\n"
					"\t\t                     Last track Number: %u\n"
					"\t\t                         Mode2 PACKETs: %s\n"
					"\t\t          Program area copy protection: %s\n"
					"\t\t            Copyright asserted for $85: %s\n"
					"\t\t        Copyright asserted for $81-$84: %s\n"
					"\t\t            Copyright asserted for $80: %s\n"
					"\t\tNumber of PACKS with $80 (ALBUM_NAME) : %u\n"
					"\t\tNumber of PACKS with $81 (PERFORMER)  : %u\n"
					"\t\tNumber of PACKS with $82 (SONGWRITER) : %u\n"
					"\t\tNumber of PACKS with $83 (COMPOSER)   : %u\n"
					"\t\tNumber of PACKS with $84 (ARRANGER)   : %u\n"
					"\t\tNumber of PACKS with $85 (MESSAGES)   : %u\n"
					"\t\tNumber of PACKS with $86 (DISC_ID)    : %u\n"
					"\t\tNumber of PACKS with $87 (GENRE)      : %u\n"
					, pDesc[wTocTextEntries - uiSizeInfoCnt].Text[1]
					, pDesc[wTocTextEntries - uiSizeInfoCnt].Text[2]
					, BOOLEAN_TO_STRING_YES_NO_A(pDesc[wTocTextEntries - uiSizeInfoCnt].Text[3] & 0x80)
					, BOOLEAN_TO_STRING_YES_NO_A(pDesc[wTocTextEntries - uiSizeInfoCnt].Text[3] & 0x40)
					, BOOLEAN_TO_STRING_YES_NO_A(pDesc[wTocTextEntries - uiSizeInfoCnt].Text[3] & 0x04)
					, BOOLEAN_TO_STRING_YES_NO_A(pDesc[wTocTextEntries - uiSizeInfoCnt].Text[3] & 0x02)
					, BOOLEAN_TO_STRING_YES_NO_A(pDesc[wTocTextEntries - uiSizeInfoCnt].Text[3] & 0x01)
					, pDesc[wTocTextEntries - uiSizeInfoCnt].Text[4]
					, pDesc[wTocTextEntries - uiSizeInfoCnt].Text[5]
					, pDesc[wTocTextEntries - uiSizeInfoCnt].Text[6]
					, pDesc[wTocTextEntries - uiSizeInfoCnt].Text[7]
					, pDesc[wTocTextEntries - uiSizeInfoCnt].Text[8]
					, pDesc[wTocTextEntries - uiSizeInfoCnt].Text[9]
					, pDesc[wTocTextEntries - uiSizeInfoCnt].Text[10]
					, pDesc[wTocTextEntries - uiSizeInfoCnt].Text[11]);
			}
			else if (nSizeInfoCnt == 1) {
				OutputDiscLogA(
					"\t\tNumber of PACKS with $88 (TOC_INFO)   : %u\n"
					"\t\tNumber of PACKS with $89 (TOC_INFO2)  : %u\n"
					"\t\tNumber of PACKS with $8b              : %u\n"
					"\t\tNumber of PACKS with $8a              : %u\n"
					"\t\tNumber of PACKS with $8c              : %u\n"
					"\t\tNumber of PACKS with $8d (CLOSED_INFO): %u\n"
					"\t\tNumber of PACKS with $8e (UPC_EAN)    : %u\n"
					"\t\tNumber of PACKS with $8f (SIZE_INFO)  : %u\n"
					"\t\t       Last Sequence number of BLOCK 0: %u\n"
					"\t\t       Last Sequence number of BLOCK 1: %u\n"
					"\t\t       Last Sequence number of BLOCK 2: %u\n"
					"\t\t       Last Sequence number of BLOCK 3: %u\n"
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 1].Text[0]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 1].Text[1]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 1].Text[2]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 1].Text[3]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 1].Text[4]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 1].Text[5]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 1].Text[6]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 1].Text[7]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 1].Text[8]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 1].Text[9]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 1].Text[10]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 1].Text[11]);
			}
			else if (nSizeInfoCnt == 2) {
				OutputDiscLogA(
					"\t\t       Last Sequence number of BLOCK 4: %u\n"
					"\t\t       Last Sequence number of BLOCK 5: %u\n"
					"\t\t       Last Sequence number of BLOCK 6: %u\n"
					"\t\t       Last Sequence number of BLOCK 7: %u\n"
					"\t\t                 Language code BLOCK 0: 0x%02x\n"
					"\t\t                 Language code BLOCK 1: 0x%02x\n"
					"\t\t                 Language code BLOCK 2: 0x%02x\n"
					"\t\t                 Language code BLOCK 3: 0x%02x\n"
					"\t\t                 Language code BLOCK 4: 0x%02x\n"
					"\t\t                 Language code BLOCK 5: 0x%02x\n"
					"\t\t                 Language code BLOCK 6: 0x%02x\n"
					"\t\t                 Language code BLOCK 7: 0x%02x\n"
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 2].Text[0]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 2].Text[1]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 2].Text[2]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 2].Text[3]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 2].Text[4]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 2].Text[5]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 2].Text[6]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 2].Text[7]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 2].Text[8]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 2].Text[9]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 2].Text[10]
					, pDesc[wTocTextEntries - uiSizeInfoCnt + 2].Text[11]);
			}
			nSizeInfoCnt++;
		}
	}
}

VOID OutputDiscInformation(
	PDISC_INFORMATION pDiscInformation
) {
	LPCSTR lpDiscStatus[] = {
		"Empty", "Incomplete", "Complete", "Others"
	};
	LPCSTR lpStateOfLastSession[] = {
		"Empty", "Incomplete", "Reserved / Damaged", "Complete"
	};
	LPCSTR lpBGFormatStatus[] = {
		"None", "Incomplete", "Running", "Complete"
	};
	OutputDiscLogA(
		OUTPUT_DHYPHEN_PLUS_STR(DiscInformation)
		"\t                  DiscStatus: %s\n"
		"\t           LastSessionStatus: %s\n"
		"\t                    Erasable: %s\n"
		"\t            FirstTrackNumber: %u\n"
		"\t         NumberOfSessionsLsb: %u\n"
		"\t    LastSessionFirstTrackLsb: %u\n"
		"\t     LastSessionLastTrackLsb: %u\n"
		"\t                   MrwStatus: %s\n"
		"\t                 MrwDirtyBit: %s\n"
		"\t             UnrestrictedUse: %s\n"
		"\t            DiscBarCodeValid: %s\n"
		"\t                 DiscIDValid: %s\n"
		"\t                    DiscType: "
		, lpDiscStatus[pDiscInformation->DiscStatus]
		, lpStateOfLastSession[pDiscInformation->LastSessionStatus]
		, BOOLEAN_TO_STRING_YES_NO_A(pDiscInformation->Erasable)
		, pDiscInformation->FirstTrackNumber
		, pDiscInformation->NumberOfSessionsLsb
		, pDiscInformation->LastSessionFirstTrackLsb
		, pDiscInformation->LastSessionLastTrackLsb
		, lpBGFormatStatus[pDiscInformation->MrwStatus]
		, BOOLEAN_TO_STRING_YES_NO_A(pDiscInformation->MrwDirtyBit)
		, BOOLEAN_TO_STRING_YES_NO_A(pDiscInformation->URU)
		, BOOLEAN_TO_STRING_YES_NO_A(pDiscInformation->DBC_V)
		, BOOLEAN_TO_STRING_YES_NO_A(pDiscInformation->DID_V));
	switch (pDiscInformation->DiscType) {
	case DISK_TYPE_CDDA:
		OutputDiscLogA("CD-DA or CD-ROM Disc\n");
		break;
	case DISK_TYPE_CDI:
		OutputDiscLogA("CD-I Disc\n");
		break;
	case DISK_TYPE_XA:
		OutputDiscLogA("CD-ROM XA Disc\n");
		break;
	case DISK_TYPE_UNDEFINED:
		OutputDiscLogA("Undefined\n");
		break;
	default:
		OutputDiscLogA("Reserved\n");
		break;
	}
	if (pDiscInformation->DID_V) {
		OutputDiscLogA(
			"\t          DiscIdentification: %u%u%u%u\n",
			pDiscInformation->DiskIdentification[0],
			pDiscInformation->DiskIdentification[1],
			pDiscInformation->DiskIdentification[2],
			pDiscInformation->DiskIdentification[3]);
	}
	OutputDiscLogA(
		"\t           LastSessionLeadIn: %02x:%02x:%02x:%02x\n"
		"\tLastPossibleLeadOutStartTime: %02x:%02x:%02x:%02x\n",
		pDiscInformation->LastSessionLeadIn[0],
		pDiscInformation->LastSessionLeadIn[1],
		pDiscInformation->LastSessionLeadIn[2],
		pDiscInformation->LastSessionLeadIn[3],
		pDiscInformation->LastPossibleLeadOutStartTime[0],
		pDiscInformation->LastPossibleLeadOutStartTime[1],
		pDiscInformation->LastPossibleLeadOutStartTime[2],
		pDiscInformation->LastPossibleLeadOutStartTime[3]);
	if (pDiscInformation->DBC_V) {
		OutputDiscLogA(
			"\t                 DiscBarCode: %u%u%u%u%u%u%u%u\n",
			pDiscInformation->DiskBarCode[0],
			pDiscInformation->DiskBarCode[1],
			pDiscInformation->DiskBarCode[2],
			pDiscInformation->DiskBarCode[3],
			pDiscInformation->DiskBarCode[4],
			pDiscInformation->DiskBarCode[5],
			pDiscInformation->DiskBarCode[6],
			pDiscInformation->DiskBarCode[7]);
	}
	OutputDiscLogA(
		"\t            NumberOPCEntries: %u\n",
		pDiscInformation->NumberOPCEntries);
	if (pDiscInformation->NumberOPCEntries) {
		OutputDiscLogA(
			"\t                    OPCTable\n");
	}
	for (INT i = 0; i < pDiscInformation->NumberOPCEntries; i++) {
		OutputDiscLogA(
			"\t\t                     Speed: %u%u\n"
			"\t\t                 OPCValues: %u%u%u%u%u%u\n",
			pDiscInformation->OPCTable[0].Speed[0],
			pDiscInformation->OPCTable[0].Speed[1],
			pDiscInformation->OPCTable[0].OPCValue[0],
			pDiscInformation->OPCTable[0].OPCValue[1],
			pDiscInformation->OPCTable[0].OPCValue[2],
			pDiscInformation->OPCTable[0].OPCValue[3],
			pDiscInformation->OPCTable[0].OPCValue[4],
			pDiscInformation->OPCTable[0].OPCValue[5]);
	}
}

VOID OutputModeParmeterHeader(
	PMODE_PARAMETER_HEADER pHeader
) {
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(ModeParmeterHeader)
		"\t           ModeDataLength: %u\n"
		"\t               MediumType: %u\n"
		"\t  DeviceSpecificParameter: %u\n"
		"\t    BlockDescriptorLength: %u\n"
		, pHeader->ModeDataLength
		, pHeader->MediumType
		, pHeader->DeviceSpecificParameter
		, pHeader->BlockDescriptorLength);
}

VOID OutputModeParmeterHeader10(
	PMODE_PARAMETER_HEADER10 pHeader
) {
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(ModeParmeterHeader10)
		"\t           ModeDataLength: %u\n"
		"\t               MediumType: %u\n"
		"\t  DeviceSpecificParameter: %u\n"
		"\t    BlockDescriptorLength: %u\n"
		, MAKEWORD(pHeader->ModeDataLength[1],
		pHeader->ModeDataLength[0])
		, pHeader->MediumType
		, pHeader->DeviceSpecificParameter
		, MAKEWORD(pHeader->BlockDescriptorLength[1],
		pHeader->BlockDescriptorLength[0]));
}

VOID OutputCDVDCapabilitiesPage(
	PCDVD_CAPABILITIES_PAGE cdvd,
	INT perKb
) {
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(CDVD Capabilities & Mechanism Status Page)
		"\t                 PageCode: %#04x\n"
		"\t                    PSBit: %s\n"
		"\t               PageLength: %u\n"
		"\t                  CDRRead: %s\n"
		"\t                  CDERead: %s\n"
		"\t                  Method2: %s\n"
		"\t               DVDROMRead: %s\n"
		"\t                 DVDRRead: %s\n"
		"\t               DVDRAMRead: %s\n"
		"\t                 CDRWrite: %s\n"
		"\t                 CDEWrite: %s\n"
		"\t                TestWrite: %s\n"
		"\t                DVDRWrite: %s\n"
		"\t              DVDRAMWrite: %s\n"
		"\t                AudioPlay: %s\n"
		"\t                Composite: %s\n"
		"\t           DigitalPortOne: %s\n"
		"\t           DigitalPortTwo: %s\n"
		"\t               Mode2Form1: %s\n"
		"\t               Mode2Form2: %s\n"
		"\t             MultiSession: %s\n"
		"\t       BufferUnderrunFree: %s\n"
		"\t                     CDDA: %s\n"
		"\t             CDDAAccurate: %s\n"
		"\t              RWSupported: %s\n"
		"\t          RWDeinterleaved: %s\n"
		"\t               C2Pointers: %s\n"
		"\t                     ISRC: %s\n"
		"\t                      UPC: %s\n"
		"\t       ReadBarCodeCapable: %s\n"
		"\t                     Lock: %s\n"
		"\t                LockState: %s\n"
		"\t            PreventJumper: %s\n"
		"\t                    Eject: %s\n"
		"\t     LoadingMechanismType: "
		, cdvd->PageCode
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->PSBit)
		, cdvd->PageLength
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->CDRRead)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->CDERead)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->Method2)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->DVDROMRead)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->DVDRRead)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->DVDRAMRead)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->CDRWrite)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->CDEWrite)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->TestWrite)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->DVDRWrite)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->DVDRAMWrite)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->AudioPlay)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->Composite)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->DigitalPortOne)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->DigitalPortTwo)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->Mode2Form1)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->Mode2Form2)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->MultiSession)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->BufferUnderrunFree)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->CDDA)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->CDDAAccurate)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->RWSupported)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->RWDeinterleaved)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->C2Pointers)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->ISRC)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->UPC)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->ReadBarCodeCapable)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->Lock)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->LockState)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->PreventJumper)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->Eject));

	switch (cdvd->LoadingMechanismType) {
	case LOADING_MECHANISM_CADDY:
		OutputDriveLogA("caddy\n");
		break;
	case LOADING_MECHANISM_TRAY:
		OutputDriveLogA("tray\n");
		break;
	case LOADING_MECHANISM_POPUP:
		OutputDriveLogA("popup\n");
		break;
	case LOADING_MECHANISM_INDIVIDUAL_CHANGER:
		OutputDriveLogA("individual changer\n");
		break;
	case LOADING_MECHANISM_CARTRIDGE_CHANGER:
		OutputDriveLogA("cartridge changer\n");
		break;
	default:
		OutputDriveLogA("unknown\n");
		break;
	}
	WORD rsm = MAKEWORD(cdvd->ReadSpeedMaximum[1], cdvd->ReadSpeedMaximum[0]);
	WORD rsc = MAKEWORD(cdvd->ReadSpeedCurrent[1], cdvd->ReadSpeedCurrent[0]);
	WORD wsm = MAKEWORD(cdvd->WriteSpeedMaximum[1], cdvd->WriteSpeedMaximum[0]);
	WORD wsc = MAKEWORD(cdvd->WriteSpeedCurrent[1], cdvd->WriteSpeedCurrent[0]);
	WORD bs = MAKEWORD(cdvd->BufferSize[1], cdvd->BufferSize[0]);
	OutputDriveLogA(
		"\t           SeparateVolume: %s\n"
		"\t      SeperateChannelMute: %s\n"
		"\t      SupportsDiskPresent: %s\n"
		"\t          SWSlotSelection: %s\n"
		"\t        SideChangeCapable: %s\n"
		"\t       RWInLeadInReadable: %s\n"
		"\t         ReadSpeedMaximum: %uKB/sec (%ux)\n"
		"\t       NumberVolumeLevels: %u\n"
		"\t               BufferSize: %u\n"
		"\t         ReadSpeedCurrent: %uKB/sec (%ux)\n"
		"\t                      BCK: %s\n"
		"\t                      RCK: %s\n"
		"\t                     LSBF: %s\n"
		"\t                   Length: %u\n"
		"\t        WriteSpeedMaximum: %uKB/sec (%ux)\n"
		"\t        WriteSpeedCurrent: %uKB/sec (%ux)\n"
		"\t   CopyManagementRevision: %u\n"
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->SeparateVolume)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->SeperateChannelMute)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->SupportsDiskPresent)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->SWSlotSelection)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->SideChangeCapable)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->RWInLeadInReadable)
		, rsm, rsm / perKb
		, MAKEWORD(cdvd->NumberVolumeLevels[1],
			cdvd->NumberVolumeLevels[0])
		, bs
		, rsc, rsc / perKb
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->BCK)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->RCK)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->LSBF)
		, cdvd->Length
		, wsm, wsm / perKb
		, wsc, wsc / perKb
		, MAKEWORD(cdvd->CopyManagementRevision[1], cdvd->CopyManagementRevision[0]));
}

VOID OutputPageCDvdInactivity(
	LPBYTE modesense,
	size_t pcOfs
) {
	PCDVD_INACTIVITY_TIMEOUT_PAGE inactivity = (PCDVD_INACTIVITY_TIMEOUT_PAGE)(modesense + pcOfs);
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(CDVD_INACTIVITY_TIMEOUT_PAGE)
		"\t                 PageCode: %#04x\n"
		"\t                    PSBit: %s\n"
		"\t               PageLength: %u\n"
		"\t                     SWPP: %s\n"
		"\t                     DISP: %s\n"
		"\t   GroupOneMinimumTimeout: %d\n"
		"\t   GroupTwoMinimumTimeout: %d\n"
		, inactivity->PageCode
		, BOOLEAN_TO_STRING_YES_NO_A(inactivity->PSBit)
		, inactivity->PageLength
		, BOOLEAN_TO_STRING_YES_NO_A(inactivity->SWPP)
		, BOOLEAN_TO_STRING_YES_NO_A(inactivity->DISP)
		, MAKEWORD(inactivity->GroupOneMinimumTimeout[1], inactivity->GroupOneMinimumTimeout[0])
		, MAKEWORD(inactivity->GroupTwoMinimumTimeout[1], inactivity->GroupTwoMinimumTimeout[0])
	);
}

VOID OutputPagePowerCondition(
	LPBYTE modesense,
	size_t pcOfs
) {
	PPOWER_CONDITION_PAGE power = (PPOWER_CONDITION_PAGE)(modesense + pcOfs);
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(POWER_CONDITION_PAGE)
		"\t                 PageCode: %#04x\n"
		"\t                    PSBit: %s\n"
		"\t               PageLength: %u\n"
		"\t                  Standby: %s\n"
		"\t                     Idle: %s\n"
		"\t                IdleTimer: %d\n"
		"\t             StandbyTimer: %d\n"
		, power->PageCode
		, BOOLEAN_TO_STRING_YES_NO_A(power->PSBit)
		, power->PageLength
		, BOOLEAN_TO_STRING_YES_NO_A(power->Standby)
		, BOOLEAN_TO_STRING_YES_NO_A(power->Idle)
		, MAKEUINT(MAKEWORD(power->IdleTimer[3], power->IdleTimer[2])
			, MAKEWORD(power->IdleTimer[1], power->IdleTimer[0]))
		, MAKEUINT(MAKEWORD(power->StandbyTimer[3], power->StandbyTimer[2])
			, MAKEWORD(power->StandbyTimer[1], power->StandbyTimer[0]))
	);
}

VOID OutputPageCDvdFeatureSet(
	LPBYTE modesense,
	size_t pcOfs
) {
	PCDVD_FEATURE_SET_PAGE feature = (PCDVD_FEATURE_SET_PAGE)(modesense + pcOfs);
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(CDVD_FEATURE_SET_PAGE)
		"\t                 PageCode: %#04x\n"
		"\t                    PSBit: %s\n"
		"\t               PageLength: %u\n"
		"\t                  CDAudio: %d\n"
		"\t          EmbeddedChanger: %d\n"
		"\t              PacketSMART: %d\n"
		"\t        PersistantPrevent: %d\n"
		"\t  EventStatusNotification: %d\n"
		"\t            DigitalOutput: %d\n"
		"\t   CDSequentialRecordable: %d\n"
		"\t  DVDSequentialRecordable: %d\n"
		"\t         RandomRecordable: %d\n"
		"\t              KeyExchange: %d\n"
		, feature->PageCode
		, BOOLEAN_TO_STRING_YES_NO_A(feature->PSBit)
		, feature->PageLength
		, MAKEWORD(feature->CDAudio[1], feature->CDAudio[0])
		, MAKEWORD(feature->EmbeddedChanger[1], feature->EmbeddedChanger[0])
		, MAKEWORD(feature->PacketSMART[1], feature->PacketSMART[0])
		, MAKEWORD(feature->PersistantPrevent[1], feature->PersistantPrevent[0])
		, MAKEWORD(feature->EventStatusNotification[1], feature->EventStatusNotification[0])
		, MAKEWORD(feature->DigitalOutput[1], feature->DigitalOutput[0])
		, MAKEWORD(feature->CDSequentialRecordable[1], feature->CDSequentialRecordable[0])
		, MAKEWORD(feature->DVDSequentialRecordable[1], feature->DVDSequentialRecordable[0])
		, MAKEWORD(feature->RandomRecordable[1], feature->RandomRecordable[0])
		, MAKEWORD(feature->KeyExchange[1], feature->KeyExchange[0])
	);
}

VOID OutputPageCdAudioControl(
	LPBYTE modesense,
	size_t pcOfs
) {
	PCDAUDIO_CONTROL_PAGE ctrl = (PCDAUDIO_CONTROL_PAGE)(modesense + pcOfs);
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(CDAUDIO_CONTROL_PAGE)
		"\t                 PageCode: %#04x\n"
		"\t                    PSBit: %s\n"
		"\t               PageLength: %u\n"
		"\t      StopOnTrackCrossing: %s\n"
		"\t                Immediate: %s\n"
		"\t      ChannelSelection[0]: %d\n"
		"\t                Volume[0]: %d\n"
		"\t      ChannelSelection[1]: %d\n"
		"\t                Volume[1]: %d\n"
		"\t      ChannelSelection[2]: %d\n"
		"\t                Volume[2]: %d\n"
		"\t      ChannelSelection[3]: %d\n"
		"\t                Volume[3]: %d\n"
		, ctrl->PageCode
		, BOOLEAN_TO_STRING_YES_NO_A(ctrl->PSBit)
		, ctrl->PageLength
		, BOOLEAN_TO_STRING_YES_NO_A(ctrl->StopOnTrackCrossing)
		, BOOLEAN_TO_STRING_YES_NO_A(ctrl->Immediate)
		, ctrl->CDDAOutputPorts[0].ChannelSelection
		, ctrl->CDDAOutputPorts[0].Volume
		, ctrl->CDDAOutputPorts[1].ChannelSelection
		, ctrl->CDDAOutputPorts[1].Volume
		, ctrl->CDDAOutputPorts[2].ChannelSelection
		, ctrl->CDDAOutputPorts[2].Volume
		, ctrl->CDDAOutputPorts[3].ChannelSelection
		, ctrl->CDDAOutputPorts[3].Volume
	);
}

VOID OutputPageCdDeviceParameters(
	LPBYTE modesense,
	size_t pcOfs
) {
	LPBYTE buf = modesense + pcOfs;
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(CD_DEVICE_PARAMETERS_PAGE)
		"\t                 PageCode: %#04x\n"
		"\t                    PSBit: %s\n"
		"\t               PageLength: %u\n"
		"\tInactivityTimerMultiplier: %u\n"
		"\t TheNumberOfSUnitPerMUnit: %d\n"
		"\t TheNumberOfFUnitPerSUnit: %d\n"
		, buf[0] & 0x3f
		, BOOLEAN_TO_STRING_YES_NO_A((buf[0] >> 7) & 0x01)
		, buf[1]
		, buf[3] & 0x0f
		, MAKEWORD(buf[5], buf[4])
		, MAKEWORD(buf[7], buf[6])
	);
}

VOID OutputPageWriteCaching(
	LPBYTE modesense,
	size_t pcOfs
) {
	PMODE_CACHING_PAGE caching = (PMODE_CACHING_PAGE)(modesense + pcOfs);
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(MODE_CACHING_PAGE)
		"\t                 PageCode: %#04x\n"
		"\t              PageSavable: %s\n"
		"\t               PageLength: %u\n"
		"\t         ReadDisableCache: %s\n"
		"\t     MultiplicationFactor: %s\n"
		"\t         WriteCacheEnable: %s\n"
		"\t   WriteRetensionPriority: %d\n"
		"\t    ReadRetensionPriority: %d\n"
		"\t  DisablePrefetchTransfer: %d\n"
		"\t          MinimumPrefetch: %d\n"
		"\t          MaximumPrefetch: %d\n"
		"\t   MaximumPrefetchCeiling: %d\n"
		, caching->PageCode
		, BOOLEAN_TO_STRING_YES_NO_A(caching->PageSavable)
		, caching->PageLength
		, BOOLEAN_TO_STRING_YES_NO_A(caching->ReadDisableCache)
		, BOOLEAN_TO_STRING_YES_NO_A(caching->MultiplicationFactor)
		, BOOLEAN_TO_STRING_YES_NO_A(caching->WriteCacheEnable)
		, caching->WriteRetensionPriority
		, caching->ReadRetensionPriority
		, MAKEWORD(caching->DisablePrefetchTransfer[1], caching->DisablePrefetchTransfer[0])
		, MAKEWORD(caching->MinimumPrefetch[1], caching->MinimumPrefetch[0])
		, MAKEWORD(caching->MaximumPrefetch[1], caching->MaximumPrefetch[0])
		, MAKEWORD(caching->MaximumPrefetchCeiling[1], caching->MaximumPrefetchCeiling[0])
	);
}

VOID OutputPageWriteParameters(
	LPBYTE modesense,
	size_t pcOfs
) {
	PMODE_CDROM_WRITE_PARAMETERS_PAGE2 wparam = (PMODE_CDROM_WRITE_PARAMETERS_PAGE2)(modesense + pcOfs);
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(MODE_CDROM_WRITE_PARAMETERS_PAGE)
		"\t                 PageCode: %#04x\n"
		"\t              PageSavable: %s\n"
		"\t               PageLength: %u\n"
		"\t                WriteType: %d\n"
		"\t                TestWrite: %s\n"
		"\t            LinkSizeValid: %s\n"
		"\tBufferUnderrunFreeEnabled: %s\n"
		"\t                TrackMode: %d\n"
		"\t                     Copy: %s\n"
		"\t              FixedPacket: %s\n"
		"\t             MultiSession: %d\n"
		"\t            DataBlockType: %d\n"
		"\t                 LinkSize: %d\n"
		"\t      HostApplicationCode: %d\n"
		"\t            SessionFormat: %d\n"
		"\t               PacketSize: %d\n"
		"\t         AudioPauseLength: %d\n"
		"\t       MediaCatalogNumber: %16s\n"
		"\t                     ISRC: %16s\n"
		"\t            SubHeaderData: %d\n"
		, wparam->PageCode
		, BOOLEAN_TO_STRING_YES_NO_A(wparam->PageSavable)
		, wparam->PageLength
		, wparam->WriteType
		, BOOLEAN_TO_STRING_YES_NO_A(wparam->TestWrite)
		, BOOLEAN_TO_STRING_YES_NO_A(wparam->LinkSizeValid)
		, BOOLEAN_TO_STRING_YES_NO_A(wparam->BufferUnderrunFreeEnabled)
		, wparam->TrackMode
		, BOOLEAN_TO_STRING_YES_NO_A(wparam->Copy)
		, BOOLEAN_TO_STRING_YES_NO_A(wparam->FixedPacket)
		, wparam->MultiSession
		, wparam->DataBlockType
		, wparam->LinkSize
		, wparam->HostApplicationCode
		, wparam->SessionFormat
		, MAKEUINT(MAKEWORD(wparam->PacketSize[3], wparam->PacketSize[2])
			, MAKEWORD(wparam->PacketSize[1], wparam->PacketSize[0]))
		, MAKEWORD(wparam->AudioPauseLength[1], wparam->AudioPauseLength[0])
		, &wparam->MediaCatalogNumber[0]
		, &wparam->ISRC[0]
		, MAKEUINT(MAKEWORD(wparam->SubHeaderData[3], wparam->SubHeaderData[2])
			, MAKEWORD(wparam->SubHeaderData[1], wparam->SubHeaderData[0]))
	);
}

VOID OutputPageRigidGeometry(
	LPBYTE modesense,
	size_t pcOfs
) {
	PMODE_RIGID_GEOMETRY_PAGE geom = (PMODE_RIGID_GEOMETRY_PAGE)(modesense + pcOfs);
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(MODE_RIGID_GEOMETRY_PAGE)
		"\t                PageCode: %#04x\n"
		"\t             PageSavable: %s\n"
		"\t              PageLength: %u\n"
		"\t       NumberOfCylinders: %d\n"
		"\t           NumberOfHeads: %d\n"
		"\t        StartWritePrecom: %d\n"
		"\t     StartReducedCurrent: %d\n"
		"\t           DriveStepRate: %d\n"
		"\t       LandZoneCyclinder: %d\n"
		"\t  RotationalPositionLock: %s\n"
		"\t          RotationOffset: %d\n"
		"\t            RoataionRate: %d\n"
		, geom->PageCode
		, BOOLEAN_TO_STRING_YES_NO_A(geom->PageSavable)
		, geom->PageLength
		, MAKEUINT(MAKEWORD(geom->NumberOfCylinders[2], geom->NumberOfCylinders[1])
			, MAKEWORD(geom->NumberOfCylinders[0], 0))
		, geom->NumberOfHeads
		, MAKEUINT(MAKEWORD(geom->StartWritePrecom[2], geom->StartWritePrecom[1])
			, MAKEWORD(geom->StartWritePrecom[0], 0))
		, MAKEUINT(MAKEWORD(geom->StartReducedCurrent[2], geom->StartReducedCurrent[1])
			, MAKEWORD(geom->StartReducedCurrent[0], 0))
		, MAKEWORD(geom->DriveStepRate[1], geom->DriveStepRate[0])
		, MAKEUINT(MAKEWORD(geom->LandZoneCyclinder[2], geom->LandZoneCyclinder[1])
			, MAKEWORD(geom->LandZoneCyclinder[0], 0))
		, BOOLEAN_TO_STRING_YES_NO_A(geom->RotationalPositionLock)
		, geom->RotationOffset
		, MAKEWORD(geom->RoataionRate[1], geom->RoataionRate[0])
	);
}

VOID OutputPageMrw(
	LPBYTE modesense,
	size_t pcOfs
) {
	PMODE_MRW_PAGE mrw = (PMODE_MRW_PAGE)(modesense + pcOfs);
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(MODE_MRW_PAGE)
		"\t                PageCode: %#04x\n"
		"\t             PageSavable: %s\n"
		"\t              PageLength: %u\n"
		"\t                LbaSpace: %s\n"
		, mrw->PageCode
		, BOOLEAN_TO_STRING_YES_NO_A(mrw->PageSavable)
		, mrw->PageLength
		, BOOLEAN_TO_STRING_YES_NO_A(mrw->LbaSpace)
	);
}

VOID OutputPageDisconnect(
	LPBYTE modesense,
	size_t pcOfs
) {
	PMODE_DISCONNECT_PAGE dis = (PMODE_DISCONNECT_PAGE)(modesense + pcOfs);
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(MODE_DISCONNECT_PAGE)
		"\t                 PageCode: %#04x\n"
		"\t              PageSavable: %s\n"
		"\t               PageLength: %u\n"
		"\t          BufferFullRatio: %u\n"
		"\t         BufferEmptyRatio: %u\n"
		"\t       BusInactivityLimit: %u\n"
		"\t        BusDisconnectTime: %u\n"
		"\t           BusConnectTime: %u\n"
		"\t         MaximumBurstSize: %u\n"
		"\t   DataTransferDisconnect: %u\n"
		, dis->PageCode
		, BOOLEAN_TO_STRING_YES_NO_A(dis->PageSavable)
		, dis->PageLength
		, dis->BufferFullRatio
		, dis->BufferEmptyRatio
		, MAKEWORD(dis->BusInactivityLimit[1], dis->BusInactivityLimit[0])
		, MAKEWORD(dis->BusDisconnectTime[1], dis->BusDisconnectTime[0])
		, MAKEWORD(dis->BusConnectTime[1], dis->BusConnectTime[0])
		, MAKEWORD(dis->MaximumBurstSize[1], dis->MaximumBurstSize[0])
		, dis->DataTransferDisconnect
	);
}

VOID OutputPageErrorRecovery(
	LPBYTE modesense,
	size_t pcOfs
) {
	PMODE_READ_WRITE_RECOVERY_PAGE rec = (PMODE_READ_WRITE_RECOVERY_PAGE)(modesense + pcOfs);
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(MODE_READ_WRITE_RECOVERY_PAGE)
		"\t                 PageCode: %#04x\n"
		"\t              PageSavable: %s\n"
		"\t               PageLength: %u\n"
		"\t                   DCRBit: %s\n"
		"\t                   PERBit: %s\n"
		"\t                   EERBit: %s\n"
		"\t                    RCBit: %s\n"
		"\t                    TBBit: %s\n"
		"\t                     ARRE: %s\n"
		"\t                     AWRE: %s\n"
		"\t           ReadRetryCount: %d\n"
		"\t          WriteRetryCount: %d\n"
		, rec->PageCode
		, BOOLEAN_TO_STRING_YES_NO_A(rec->PSBit)
		, rec->PageLength
		, BOOLEAN_TO_STRING_YES_NO_A(rec->DCRBit)
		, BOOLEAN_TO_STRING_YES_NO_A(rec->PERBit)
		, BOOLEAN_TO_STRING_YES_NO_A(rec->EERBit)
		, BOOLEAN_TO_STRING_YES_NO_A(rec->RCBit)
		, BOOLEAN_TO_STRING_YES_NO_A(rec->TBBit)
		, BOOLEAN_TO_STRING_YES_NO_A(rec->ARRE)
		, BOOLEAN_TO_STRING_YES_NO_A(rec->AWRE)
		, rec->ReadRetryCount
		, rec->WriteRetryCount
	);
}

VOID OutputPageUnknown(
	LPBYTE modesense,
	size_t pcOfs,
	LPCTSTR str
) {
	int len = *(modesense + pcOfs + 1);
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(%s)
		"\t                 PageCode: %#04x\n"
		"\t                    PSBit: %s\n"
		"\t               PageLength: %u\n"
		"\t                  Unknown: "
		, str
		, *(modesense + pcOfs) & 0x3f
		, BOOLEAN_TO_STRING_YES_NO_A((*(modesense + pcOfs) >> 7) & 0x01)
		, len
	);
	for (int t = 0; t < len; t++) {
		OutputDriveLogA("%02x", *(modesense + pcOfs + t));
	}
	OutputDriveLogA("\n");
}

VOID OutputModeSense(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE modesense
) {
	WORD size = MAKEWORD(modesense[1], modesense[0]);
	BYTE pagecode = modesense[sizeof(MODE_PARAMETER_HEADER10)];
	INT pcLen = modesense[sizeof(MODE_PARAMETER_HEADER10) + 1];
	size_t pcOfs = sizeof(MODE_PARAMETER_HEADER10);
	INT ofs1 = 2;
	size_t bufOfs = sizeof(MODE_PARAMETER_HEADER10) + pcLen + ofs1;

	if (*pExecType != drivespeed) {
		OutputModeParmeterHeader10((PMODE_PARAMETER_HEADER10)modesense);
	}
	do {
		if (*pExecType == drivespeed && pagecode != MODE_PAGE_CAPABILITIES) {
			pagecode = (BYTE)(modesense[bufOfs] & 0x3f);
			pcLen += modesense[bufOfs + 1];
			pcOfs = bufOfs;
			ofs1 += 2;
			bufOfs = sizeof(MODE_PARAMETER_HEADER10) + pcLen + ofs1;
			continue;
		}
		if (pagecode == MODE_PAGE_VENDOR_SPECIFIC) {
			OutputPageUnknown(modesense, pcOfs, "VENDOR_SPECIFIC");
		}
		else if (pagecode == MODE_PAGE_ERROR_RECOVERY) {
			OutputPageErrorRecovery(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_DISCONNECT) {
			OutputPageDisconnect(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_MRW) {
			OutputPageMrw(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_RIGID_GEOMETRY) {
			OutputPageRigidGeometry(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_WRITE_PARAMETERS) {
			OutputPageWriteParameters(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_VERIFY_ERROR) {
			OutputPageUnknown(modesense, pcOfs, "VERIFY_ERROR");
		}
		else if (pagecode == MODE_PAGE_CACHING) {
			OutputPageWriteCaching(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_PERIPHERAL) {
			OutputPageUnknown(modesense, pcOfs, "PERIPHERAL");
		}
		else if (pagecode == MODE_PAGE_CONTROL) {
			OutputPageUnknown(modesense, pcOfs, "CONTROL");
		}
		else if (pagecode == MODE_PAGE_MEDIUM_TYPES) {
			OutputPageUnknown(modesense, pcOfs, "MEDIUM_TYPES");
		}
		else if (pagecode == MODE_PAGE_NOTCH_PARTITION) {
			OutputPageUnknown(modesense, pcOfs, "NOTCH_PARTITION");
		}
		else if (pagecode == 0x0d) {
			OutputPageCdDeviceParameters(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_CD_AUDIO_CONTROL) {
			OutputPageCdAudioControl(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_DATA_COMPRESS) {
			OutputPageUnknown(modesense, pcOfs, "DATA_COMPRESS");
		}
		else if (pagecode == MODE_PAGE_DEVICE_CONFIG) {
			OutputPageUnknown(modesense, pcOfs, "DEVICE_CONFIG");
		}
		else if (pagecode == MODE_PAGE_MEDIUM_PARTITION) {
			OutputPageUnknown(modesense, pcOfs, "MEDIUM_PARTITION");
		}
		else if (pagecode == MODE_PAGE_ENCLOSURE_SERVICES_MANAGEMENT) {
			OutputPageUnknown(modesense, pcOfs, "ENCLOSURE_SERVICES_MANAGEMENT");
		}
		else if (pagecode == MODE_PAGE_EXTENDED) {
			OutputPageUnknown(modesense, pcOfs, "EXTENDED");
		}
		else if (pagecode == MODE_PAGE_EXTENDED_DEVICE_SPECIFIC) {
			OutputPageUnknown(modesense, pcOfs, "EXTENDED_DEVICE_SPECIFIC");
		}
		else if (pagecode == MODE_PAGE_CDVD_FEATURE_SET) {
			OutputPageCDvdFeatureSet(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_PROTOCOL_SPECIFIC_PORT) {
			OutputPageUnknown(modesense, pcOfs, "PROTOCOL_SPECIFIC_PORT");
		}
		else if (pagecode == MODE_PAGE_POWER_CONDITION) {
			OutputPagePowerCondition(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_LUN_MAPPING) {
			OutputPageUnknown(modesense, pcOfs, "LUN_MAPPING");
		}
		else if (pagecode == MODE_PAGE_FAULT_REPORTING) {
			OutputPageUnknown(modesense, pcOfs, "FAULT_REPORTING");
		}
		else if (pagecode == MODE_PAGE_CDVD_INACTIVITY) {
			OutputPageCDvdInactivity(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_ELEMENT_ADDRESS) {
			OutputPageUnknown(modesense, pcOfs, "ELEMENT_ADDRESS");
		}
		else if (pagecode == MODE_PAGE_TRANSPORT_GEOMETRY) {
			OutputPageUnknown(modesense, pcOfs, "TRANSPORT_GEOMETRY");
		}
		else if (pagecode == MODE_PAGE_DEVICE_CAPABILITIES) {
			OutputPageUnknown(modesense, pcOfs, "DEVICE_CAPABILITIES");
		}
		else if (pagecode == MODE_PAGE_CAPABILITIES) {
			PCDVD_CAPABILITIES_PAGE capabilities = (PCDVD_CAPABILITIES_PAGE)(modesense + pcOfs);
			WORD rsm = MAKEWORD(capabilities->ReadSpeedMaximum[1],
				capabilities->ReadSpeedMaximum[0]);
			INT perKb = 176;
			if (IsDVDBasedDisc(pDisc)) {
				perKb = 1385;
			}
			else if (IsBDBasedDisc(pDisc)) {
				perKb = 4496;
			}
			if (*pExecType == drivespeed) {
				OutputString("ReadSpeedMaximum: %uKB/sec (%ux)\n", rsm, rsm / perKb);
			}
			else {
				pDevice->wMaxReadSpeed = rsm;
				OutputCDVDCapabilitiesPage(capabilities, perKb);
			}
			break;
		}
		else {
			OutputPageUnknown(modesense, pcOfs, "Unknown");
		}
		pagecode = (BYTE)(modesense[bufOfs] & 0x3f);
		pcLen += modesense[bufOfs + 1];
		pcOfs = bufOfs;
		ofs1 += 2;
		bufOfs = sizeof(MODE_PARAMETER_HEADER10) + pcLen + ofs1;
	} while (pcOfs <= size);
}

VOID OutputReadBufferCapacity(
	PREAD_BUFFER_CAPACITY_DATA pReadBufCapaData
) {
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(ReadBufferCapacity)
		"\t    TotalBufferSize: %uKByte\n"
		"\tAvailableBufferSize: %uKByte\n",
		MAKEUINT(MAKEWORD(pReadBufCapaData->TotalBufferSize[3],	pReadBufCapaData->TotalBufferSize[2]),
			MAKEWORD(pReadBufCapaData->TotalBufferSize[1], pReadBufCapaData->TotalBufferSize[0])) / 1024,
		MAKEUINT(MAKEWORD(pReadBufCapaData->AvailableBufferSize[3],	pReadBufCapaData->AvailableBufferSize[2]),
			MAKEWORD(pReadBufCapaData->AvailableBufferSize[1], pReadBufCapaData->AvailableBufferSize[0])) / 1024);
}

VOID OutputSetSpeed(
	PCDROM_SET_SPEED pSetspeed
) {
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(SetSpeed)
		"\t    RequestType: %s\n"
		"\t      ReadSpeed: %uKB/sec\n"
		"\t     WriteSpeed: %uKB/sec\n"
		"\tRotationControl: %s\n",
		pSetspeed->RequestType == 0 ?
		"CdromSetSpeed" : "CdromSetStreaming",
		pSetspeed->ReadSpeed,
		pSetspeed->WriteSpeed,
		pSetspeed->RotationControl == 0 ?
		"CdromDefaultRotation" : "CdromCAVRotation");
}

VOID OutputEepromUnknownByte(
	LPBYTE pBuf,
	UINT startIdx,
	UINT endIdx
) {
	if (startIdx <= endIdx) {
		OutputDriveLogA("\t   Unknown[%03d]: ", startIdx);
		for (UINT i = startIdx; i <= endIdx; i++) {
			OutputDriveLogA("%02x ", pBuf[i]);
		}
		OutputDriveLogA("\n");
	}
}

VOID OutputEepromOverPX712(
	LPBYTE pBuf
) {
	OutputDriveLogA("\t    Silent Mode: ");
	if (pBuf[0] == 1) {
		OutputDriveLogA(
			"Enabled\n"
			"\t\t       Access Time: ");
		if (pBuf[1] == 0) {
			OutputDriveLogA("Fast\n");
		}
		else if (pBuf[1] == 1) {
			OutputDriveLogA("Slow\n");
		}
		OutputDriveLogA(
			"\t\t     CD Read Speed: %dx\n"
			"\t\t    DVD Read Speed: %dx\n"
			"\t\t    CD Write Speed: %dx\n"
			"\t\t           Unknown: %02x\n"
			"\t\t           Unknown: %02x\n"
			"\t\t  Tray Speed Eject: %02x (Low d0 - 80 High)\n"
			"\t\tTray Speed Loading: %02x (Low 2f - 7f High)\n"
			, pBuf[2], pBuf[3], pBuf[4]
			, pBuf[5], pBuf[6], pBuf[7], pBuf[8]);
	}
	else {
		OutputDriveLogA("Disable\n");
	}
	
	UINT tmp = 9;
	OutputDriveLogA("\t        SecuRec: ");
	while (tmp < 29) {
		OutputDriveLogA("%02x ", pBuf[tmp]);
		tmp += 1;
	}

	OutputDriveLogA("\n\t      SpeedRead: ");
	if (pBuf[29] == 0xf0 || pBuf[29] == 0) {
		OutputDriveLogA("Enable");
	}
	else if (pBuf[29] == 0xff || pBuf[29] == 0x0f) {
		OutputDriveLogA("Disable");
	}
	OutputDriveLogA("\n\t        Unknown: %x\n", pBuf[30]);

	OutputDriveLogA("\t  Spindown Time: ");
	switch (pBuf[31]) {
	case 0:
		OutputDriveLogA("Infinite\n");
		break;
	case 1:
		OutputDriveLogA("125 ms\n");
		break;
	case 2:
		OutputDriveLogA("250 ms\n");
		break;
	case 3:
		OutputDriveLogA("500 ms\n");
		break;
	case 4:
		OutputDriveLogA("1 second\n");
		break;
	case 5:
		OutputDriveLogA("2 seconds\n");
		break;
	case 6:
		OutputDriveLogA("4 seconds\n");
		break;
	case 7:
		OutputDriveLogA("8 seconds\n");
		break;
	case 8:
		OutputDriveLogA("16 seconds\n");
		break;
	case 9:
		OutputDriveLogA("32 seconds\n");
		break;
	case 10:
		OutputDriveLogA("1 minite\n");
		break;
	case 11:
		OutputDriveLogA("2 minites\n");
		break;
	case 12:
		OutputDriveLogA("4 minites\n");
		break;
	case 13:
		OutputDriveLogA("8 minites\n");
		break;
	case 14:
		OutputDriveLogA("16 minites\n");
		break;
	case 15:
		OutputDriveLogA("32 minites\n");
		break;
	default:
		OutputDriveLogA("Unset\n");
		break;
	}

	LONG ucr = 
		MAKELONG(MAKEWORD(pBuf[37], pBuf[36]), MAKEWORD(pBuf[35], pBuf[34]));
	LONG ucw = 
		MAKELONG(MAKEWORD(pBuf[41], pBuf[40]), MAKEWORD(pBuf[39], pBuf[38]));
	LONG udr = 
		MAKELONG(MAKEWORD(pBuf[45], pBuf[44]), MAKEWORD(pBuf[43], pBuf[42]));
	LONG udw = 
		MAKELONG(MAKEWORD(pBuf[49], pBuf[48]), MAKEWORD(pBuf[47], pBuf[46]));
	OutputDriveLogA(
		"\tDisc load count: %u\n"
		"\t   CD read time: %02lu:%02lu:%02lu\n"
		"\t  CD write time: %02lu:%02lu:%02lu\n"
		"\t  DVD read time: %02lu:%02lu:%02lu\n"
		"\t DVD write time: %02lu:%02lu:%02lu\n"
		, MAKEWORD(pBuf[33], pBuf[32])
		, ucr / 3600, ucr / 60 % 60, ucr % 60
		, ucw / 3600, ucw / 60 % 60, ucw % 60
		, udr / 3600, udr / 60 % 60, udr % 60
		, udw / 3600, udw / 60 % 60, udw % 60);

	OutputEepromUnknownByte(pBuf, 50, 114);

	OutputDriveLogA("\tChange BookType: ");
	switch (pBuf[115]) {
	case 0xfc:
		OutputDriveLogA("for DVD+R, DVD+R DL\n");
		break;
	case 0xfd:
		OutputDriveLogA("for DVD+R\n");
		break;
	case 0xfe:
		OutputDriveLogA("for DVD+R DL\n");
		break;
	case 0xff:
		OutputDriveLogA("Disable\n");
		break;
	default:
		OutputDriveLogA("Unknown[%02x]\n", pBuf[115]);
		break;
	}
}

VOID OutputEeprom(
	LPBYTE pBuf,
	INT nRoop,
	BOOL byPlxtrDrive
) {
	if (nRoop == 0) {
		OutputDriveLogA(
			"\t      Signature: %02x %02x\n"
			"\t       VendorId: %.8s\n"
			"\t      ProductId: %.16s\n"
			"\t   SerialNumber: %06lu\n"
			, pBuf[0], pBuf[1]
			, (LPCH)&pBuf[2]
			, (LPCH)&pBuf[10]
			, strtoul((LPCH)&pBuf[26], NULL, 16));
		OutputEepromUnknownByte(pBuf, 31, 40);

		switch (byPlxtrDrive) {
		case PLXTR_DRIVE_TYPE::PX760A:
		case PLXTR_DRIVE_TYPE::PX755A:
		case PLXTR_DRIVE_TYPE::PX716AL:
		case PLXTR_DRIVE_TYPE::PX716A:
		case PLXTR_DRIVE_TYPE::PX714A:
		case PLXTR_DRIVE_TYPE::PX712A:
			OutputDriveLogA("\t            TLA: %.4s\n", (LPCH)&pBuf[41]);
			break;
		default:
			OutputEepromUnknownByte(pBuf, 41, 44);
			break;
		}
		OutputEepromUnknownByte(pBuf, 45, 107);

		switch (byPlxtrDrive) {
		case PLXTR_DRIVE_TYPE::PX760A:
		case PLXTR_DRIVE_TYPE::PX755A:
		case PLXTR_DRIVE_TYPE::PX716AL:
		case PLXTR_DRIVE_TYPE::PX716A:
		case PLXTR_DRIVE_TYPE::PX714A:
			OutputEepromUnknownByte(pBuf, 108, 255);
			break;
		case PLXTR_DRIVE_TYPE::PX712A:
			OutputEepromUnknownByte(pBuf, 108, 255);
			OutputEepromOverPX712(&pBuf[256]);
			OutputEepromUnknownByte(pBuf, 372, 510);
			OutputDriveLogA(
				"\t            Sum: %02x (SpeedRead: %02x + Spindown Time: %02x + BookType: %02x + Others)\n"
				, pBuf[511], pBuf[285], pBuf[287], pBuf[371]);
			break;
		case PLXTR_DRIVE_TYPE::PX708A2:
		case PLXTR_DRIVE_TYPE::PX708A:
		case PLXTR_DRIVE_TYPE::PX704A:
		{
			OutputEepromUnknownByte(pBuf, 108, 114);
			LONG ucr = MAKELONG(MAKEWORD(pBuf[120], pBuf[119]), MAKEWORD(pBuf[118], pBuf[117]));
			LONG ucw = MAKELONG(MAKEWORD(pBuf[125], pBuf[124]), MAKEWORD(pBuf[123], pBuf[122]));
			OutputDriveLogA(
				"\tDisc load count: %u\n"
				"\t   CD read time: %02lu:%02lu:%02lu\n"
				"\t        Unknown: %02x\n"
				"\t  CD write time: %02lu:%02lu:%02lu\n"
				, MAKEWORD(pBuf[116], pBuf[115])
				, ucr / 3600, ucr / 60 % 60, ucr % 60
				, pBuf[121]
				, ucw / 3600, ucw / 60 % 60, ucw % 60);
			OutputEepromUnknownByte(pBuf, 126, 211);
			LONG udr =
				MAKELONG(MAKEWORD(pBuf[215], pBuf[214]), MAKEWORD(pBuf[213], pBuf[212]));
			LONG udw =
				MAKELONG(MAKEWORD(pBuf[219], pBuf[218]), MAKEWORD(pBuf[217], pBuf[216]));
			OutputDriveLogA(
				"\t  DVD read time: %02lu:%02lu:%02lu\n"
				"\t DVD write time: %02lu:%02lu:%02lu\n"
				, udr / 3600, udr / 60 % 60, udr % 60
				, udw / 3600, udw / 60 % 60, udw % 60);
			OutputEepromUnknownByte(pBuf, 220, 255);
			break;
		}
		case PLXTR_DRIVE_TYPE::PX320A:
		{
			OutputEepromUnknownByte(pBuf, 108, 123);
			LONG ucr = MAKELONG(MAKEWORD(pBuf[127], pBuf[126]), MAKEWORD(pBuf[125], pBuf[124]));
			OutputDriveLogA(
				"\t   CD read time: %02lu:%02lu:%02lu\n"
				, ucr / 3600, ucr / 60 % 60, ucr % 60);
			OutputEepromUnknownByte(pBuf, 128, 187);
			LONG udr =
				MAKELONG(MAKEWORD(pBuf[191], pBuf[190]), MAKEWORD(pBuf[189], pBuf[188]));
			OutputDriveLogA(
				"\t  DVD read time: %02lu:%02lu:%02lu\n"
				, udr / 3600, udr / 60 % 60, udr % 60);
			OutputEepromUnknownByte(pBuf, 192, 226);
			OutputDriveLogA(
				"\tDisc load count: %u\n"
				, MAKEWORD(pBuf[228], pBuf[227]));
			OutputEepromUnknownByte(pBuf, 229, 255);
			break;
		}
		case PLXTR_DRIVE_TYPE::PREMIUM2:
		case PLXTR_DRIVE_TYPE::PREMIUM:
		case PLXTR_DRIVE_TYPE::PXW5224A:
		case PLXTR_DRIVE_TYPE::PXW4824A:
		case PLXTR_DRIVE_TYPE::PXW4012A:
		case PLXTR_DRIVE_TYPE::PXW4012S:
		{
			LONG ucr = MAKELONG(MAKEWORD(pBuf[111], pBuf[110]), MAKEWORD(pBuf[109], pBuf[108]));
			LONG ucw = MAKELONG(MAKEWORD(pBuf[125], pBuf[124]), MAKEWORD(pBuf[123], pBuf[122]));
			OutputDriveLogA(
				"\t   CD read time: %02lu:%02lu:%02lu\n"
				"\t        Unknown: %02x %02x %02x %02x %02x %02x %02x %02x\n"
				"\tDisc load count: %u\n"
				"\t  CD write time: %02lu:%02lu:%02lu\n"
				, ucr / 3600, ucr / 60 % 60, ucr % 60
				, pBuf[112], pBuf[113], pBuf[114], pBuf[115], pBuf[116], pBuf[117], pBuf[118], pBuf[119]
				, MAKEWORD(pBuf[121], pBuf[120])
				, ucw / 3600, ucw / 60 % 60, ucw % 60);
			OutputEepromUnknownByte(pBuf, 126, 127);
			break;
		}
		case PLXTR_DRIVE_TYPE::PXW2410A:
		case PLXTR_DRIVE_TYPE::PXS88T:
		case PLXTR_DRIVE_TYPE::PXW1610A:
		case PLXTR_DRIVE_TYPE::PXW1210A:
		case PLXTR_DRIVE_TYPE::PXW1210S:
		{
			OutputEepromUnknownByte(pBuf, 108, 119);
			LONG ucw = MAKELONG(MAKEWORD(pBuf[125], pBuf[124]), MAKEWORD(pBuf[123], pBuf[122]));
			OutputDriveLogA(
				"\tDisc load count: %u\n"
				"\t  CD write time: %02lu:%02lu:%02lu\n"
				, MAKEWORD(pBuf[121], pBuf[120])
				, ucw / 3600, ucw / 60 % 60, ucw % 60);
			OutputEepromUnknownByte(pBuf, 126, 127);
			break;
		}
		case PLXTR_DRIVE_TYPE::PXW124TS:
		case PLXTR_DRIVE_TYPE::PXW8432T:
		case PLXTR_DRIVE_TYPE::PXW8220T:
		case PLXTR_DRIVE_TYPE::PXW4220T:
		case PLXTR_DRIVE_TYPE::PXR820T:
		{
			OutputEepromUnknownByte(pBuf, 108, 121);
			LONG ucw = MAKELONG(MAKEWORD(pBuf[125], pBuf[124]), MAKEWORD(pBuf[123], pBuf[122]));
			OutputDriveLogA(
				"\t  CD write time: %02lu:%02lu:%02lu\n"
				, ucw / 3600, ucw / 60 % 60, ucw % 60);
			OutputEepromUnknownByte(pBuf, 126, 127);
			break;
		}
		case PLXTR_DRIVE_TYPE::PXR412C:
		case PLXTR_DRIVE_TYPE::PX40TS:
		case PLXTR_DRIVE_TYPE::PX40TSUW:
		case PLXTR_DRIVE_TYPE::PX40TW:
		case PLXTR_DRIVE_TYPE::PX32TS:
		case PLXTR_DRIVE_TYPE::PX32CS:
		case PLXTR_DRIVE_TYPE::PX20TS:
		case PLXTR_DRIVE_TYPE::PX12TS:
		case PLXTR_DRIVE_TYPE::PX12CS:
		case PLXTR_DRIVE_TYPE::PX8XCS:
			OutputEepromUnknownByte(pBuf, 108, 127);
			break;
		default:
			break;
		}
	}
	else if (nRoop == 1 && byPlxtrDrive <= PLXTR_DRIVE_TYPE::PX714A) {
		OutputEepromOverPX712(pBuf);
		
		OutputDriveLogA("\t  Auto Strategy: ");
		switch (pBuf[116]) {
		case 0x06:
			if (PLXTR_DRIVE_TYPE::PX716AL <= byPlxtrDrive && byPlxtrDrive <= PLXTR_DRIVE_TYPE::PX714A) {
				OutputDriveLogA("AS OFF\n");
			}
			else {
				OutputDriveLogA("Unknown[%02x]\n", pBuf[116]);
			}
			break;
		case 0x07:
			if (PLXTR_DRIVE_TYPE::PX716AL <= byPlxtrDrive && byPlxtrDrive <= PLXTR_DRIVE_TYPE::PX714A) {
				OutputDriveLogA("Auto Selection\n");
			}
			else {
				OutputDriveLogA("AS ON\n");
			}
			break;
		case 0x0b:
			OutputDriveLogA("AS ON(Forced)\n");
			break;
		case 0x0e:
			if (byPlxtrDrive <= PLXTR_DRIVE_TYPE::PX755A) {
				OutputDriveLogA("AS OFF\n");
			}
			else {
				OutputDriveLogA("Unknown[%02x]\n", pBuf[116]);
			}
			break;
		case 0x0f:
			if (byPlxtrDrive <= PLXTR_DRIVE_TYPE::PX755A) {
				OutputDriveLogA("Auto Selection\n");
			}
			else {
				OutputDriveLogA("Unknown[%02x]\n", pBuf[116]);
			}
			break;
		default:
			OutputDriveLogA("Unknown[%02x]\n", pBuf[116]);
			break;
		}
		OutputEepromUnknownByte(pBuf, 117, 254);
		OutputDriveLogA(
			"\t            Sum: %02x (SpeedRead: %02x + Spindown Time: %02x + BookType: %02x + Auto Strategy: %02x + Others)\n"
			, pBuf[255], pBuf[29], pBuf[31], pBuf[115], pBuf[116]);
	}
	else {
		OutputEepromUnknownByte(pBuf, 0, 255);
	}
}
