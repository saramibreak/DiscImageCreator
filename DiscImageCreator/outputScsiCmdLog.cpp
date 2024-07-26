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
#include "check.h"
#include "convert.h"
#include "output.h"
#include "outputScsiCmdLog.h"
#include "set.h"

VOID OutputInquiry(
	PINQUIRYDATA pInquiry
) {
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("InquiryData")
		"\t          DeviceType: ");
	switch (pInquiry->DeviceType) {
	case DIRECT_ACCESS_DEVICE:
		OutputDriveLog("DirectAccessDevice (Floppy etc)\n");
		break;
	case READ_ONLY_DIRECT_ACCESS_DEVICE:
		OutputDriveLog("ReadOnlyDirectAccessDevice (CD/DVD etc)\n");
		break;
	case OPTICAL_DEVICE:
		OutputDriveLog("OpticalDisk\n");
		break;
	default:
		OutputDriveLog("OtherDevice\n");
		break;
	}
	OutputDriveLog(
		"\t DeviceTypeQualifier: ");
	switch (pInquiry->DeviceTypeQualifier) {
	case DEVICE_QUALIFIER_ACTIVE:
		OutputDriveLog("Active\n");
		break;
	case DEVICE_QUALIFIER_NOT_ACTIVE:
		OutputDriveLog("NotActive\n");
		break;
	case DEVICE_QUALIFIER_NOT_SUPPORTED:
		OutputDriveLog("NotSupported\n");
		break;
	default:
		OutputDriveLog("\n");
		break;
	}

	OutputDriveLog(
		"\t  DeviceTypeModifier: %hhu\n"
		"\t      RemovableMedia: %s\n"
		"\t            Versions: %u\n"
		"\t  ResponseDataFormat: %hhu\n"
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
		BOOLEAN_TO_STRING_YES_NO(pInquiry->RemovableMedia),
		pInquiry->Versions,
		pInquiry->ResponseDataFormat,
		BOOLEAN_TO_STRING_YES_NO(pInquiry->HiSupport),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->NormACA),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->TerminateTask),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->AERC),
		pInquiry->AdditionalLength,
		BOOLEAN_TO_STRING_YES_NO(pInquiry->MediumChanger),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->MultiPort),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->EnclosureServices),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->SoftReset),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->CommandQueue),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->LinkedCommands),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->RelativeAddressing));

	OutputDriveLog(
		"\t            VendorId: %.8" CHARWIDTH "s\n"
		"\t           ProductId: %.16" CHARWIDTH "s\n"
		"\tProductRevisionLevel: %.4" CHARWIDTH "s\n"
		"\t      VendorSpecific: %.20" CHARWIDTH "s\n"
		, pInquiry->VendorId
		, pInquiry->ProductId
		, pInquiry->ProductRevisionLevel
		, pInquiry->VendorSpecific);
}

VOID OutputGetConfigurationHeader(
	PGET_CONFIGURATION_HEADER pConfigHeader
) {
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("GetConfiguration")
		"\t    DataLength: %ld\n"
		"\tCurrentProfile: "
		, MAKELONG(MAKEWORD(pConfigHeader->DataLength[3], pConfigHeader->DataLength[2]),
		MAKEWORD(pConfigHeader->DataLength[1], pConfigHeader->DataLength[0])));
	OutputGetConfigurationFeatureProfileType(
		MAKEWORD(pConfigHeader->CurrentProfile[1], pConfigHeader->CurrentProfile[0]));
	OutputDriveLog("\n");
}

VOID OutputGetConfigurationFeatureProfileType(
	WORD wFeatureProfileType
) {
	switch (wFeatureProfileType) {
	case ProfileInvalid:
		OutputDriveLog("Invalid");
		break;
	case ProfileNonRemovableDisk:
		OutputDriveLog("NonRemovableDisk");
		break;
	case ProfileRemovableDisk:
		OutputDriveLog("RemovableDisk");
		break;
	case ProfileMOErasable:
		OutputDriveLog("MOErasable");
		break;
	case ProfileMOWriteOnce:
		OutputDriveLog("MOWriteOnce");
		break;
	case ProfileAS_MO:
		OutputDriveLog("AS_MO");
		break;
	case ProfileCdrom:
		OutputDriveLog("CD-ROM");
		break;
	case ProfileCdRecordable:
		OutputDriveLog("CD-R");
		break;
	case ProfileCdRewritable:
		OutputDriveLog("CD-RW");
		break;
	case ProfileDvdRom:
		OutputDriveLog("DVD-ROM");
		break;
	case ProfileDvdRecordable:
		OutputDriveLog("DVD-R");
		break;
	case ProfileDvdRam:
		OutputDriveLog("DVD-RAM");
		break;
	case ProfileDvdRewritable:
		OutputDriveLog("DVD-RW");
		break;
	case ProfileDvdRWSequential:
		OutputDriveLog("DVD-RW Sequential");
		break;
	case ProfileDvdDashRDualLayer:
		OutputDriveLog("DVD-R DL");
		break;
	case ProfileDvdDashRLayerJump:
		OutputDriveLog("DVD-R Layer Jump");
		break;
	case ProfileDvdPlusRW:
		OutputDriveLog("DVD+RW");
		break;
	case ProfileDvdPlusR:
		OutputDriveLog("DVD+R");
		break;
	case ProfileDDCdrom:
		OutputDriveLog("DDCD-ROM");
		break;
	case ProfileDDCdRecordable:
		OutputDriveLog("DDCD-R");
		break;
	case ProfileDDCdRewritable:
		OutputDriveLog("DDCD-RW");
		break;
	case ProfileDvdPlusRWDualLayer:
		OutputDriveLog("DVD+RW DL");
		break;
	case ProfileDvdPlusRDualLayer:
		OutputDriveLog("DVD+R DL");
		break;
	case ProfileBDRom:
		OutputDriveLog("BD-ROM");
		break;
	case ProfileBDRSequentialWritable:
		OutputDriveLog("BD-R Sequential Writable");
		break;
	case ProfileBDRRandomWritable:
		OutputDriveLog("BD-R Random Writable");
		break;
	case ProfileBDRewritable:
		OutputDriveLog("BD-RW");
		break;
	case ProfileHDDVDRom:
		OutputDriveLog("HD DVD-ROM");
		break;
	case ProfileHDDVDRecordable:
		OutputDriveLog("HD DVD-R");
		break;
	case ProfileHDDVDRam:
		OutputDriveLog("HD DVD-RAM");
		break;
	case ProfileHDDVDRewritable:
		OutputDriveLog("HD-DVD-RW");
		break;
	case ProfileHDDVDRDualLayer:
		OutputDriveLog("HD-DVD-R DL");
		break;
	case ProfileHDDVDRWDualLayer:
		OutputDriveLog("HD-DVD-RW DL");
		break;
	case ProfilePlaystationCdrom:
		OutputDriveLog("PlayStation CD-ROM");
		break;
	case ProfilePlaystation2Cdrom:
		OutputDriveLog("PlayStation2 CD-ROM");
		break;
	case ProfilePlaystation2DvdRom:
		OutputDriveLog("PlayStation2 DVD-ROM");
		break;
	case ProfilePlaystation3DvdRom:
		OutputDriveLog("PlayStation3 DVD-ROM");
		break;
	case ProfilePlaystation3BDRom:
		OutputDriveLog("PlayStation3 BD-ROM");
		break;
	case ProfilePlaystation4BDRom:
		OutputDriveLog("PlayStation4 BD-ROM");
		break;
	case ProfileNonStandard:
		OutputDriveLog("NonStandard");
		break;
	default:
		OutputDriveLog("Reserved [%#x]", wFeatureProfileType);
		break;
	}
}

VOID OutputGetConfigurationFeatureProfileList(
	PFEATURE_DATA_PROFILE_LIST pList
) {
	OutputDriveLog("\tFeatureProfileList\n");
	for (UINT i = 0; i < pList->Header.AdditionalLength / sizeof(FEATURE_DATA_PROFILE_LIST_EX); i++) {
		OutputDriveLog("\t\t");
		OutputGetConfigurationFeatureProfileType(
			MAKEWORD(pList->Profiles[i].ProfileNumber[1], pList->Profiles[i].ProfileNumber[0]));
		OutputDriveLog("\n");
	}
}

VOID OutputGetConfigurationFeatureCore(
	PFEATURE_DATA_CORE pCore
) {
	OutputDriveLog(
		"\tFeatureCore\n"
		"\t\tPhysicalInterface: ");
	LONG lVal = MAKELONG(
		MAKEWORD(pCore->PhysicalInterface[3], pCore->PhysicalInterface[2]),
		MAKEWORD(pCore->PhysicalInterface[1], pCore->PhysicalInterface[0]));
	switch (lVal) {
	case 0:
		OutputDriveLog("Unspecified\n");
		break;
	case 1:
		OutputDriveLog("SCSI Family\n");
		break;
	case 2:
		OutputDriveLog("ATAPI\n");
		break;
	case 3:
		OutputDriveLog("IEEE 1394 - 1995\n");
		break;
	case 4:
		OutputDriveLog("IEEE 1394A\n");
		break;
	case 5:
		OutputDriveLog("Fibre Channel\n");
		break;
	case 6:
		OutputDriveLog("IEEE 1394B\n");
		break;
	case 7:
		OutputDriveLog("Serial ATAPI\n");
		break;
	case 8:
		OutputDriveLog("USB (both 1.1 and 2.0)\n");
		break;
	case 0xffff:
		OutputDriveLog("Vendor Unique\n");
		break;
	default:
		OutputDriveLog("Reserved: %08ld\n", lVal);
		break;
	}
	OutputDriveLog(
		"\t\t  DeviceBusyEvent: %s\n"
		"\t\t         INQUIRY2: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pCore->DeviceBusyEvent),
		BOOLEAN_TO_STRING_YES_NO(pCore->INQUIRY2));
}

VOID OutputGetConfigurationFeatureMorphing(
	PFEATURE_DATA_MORPHING pMorphing
) {
	OutputDriveLog(
		"\tFeatureMorphing\n"
		"\t\tAsynchronous: %s\n"
		"\t\t     OCEvent: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pMorphing->Asynchronous),
		BOOLEAN_TO_STRING_YES_NO(pMorphing->OCEvent));
}

VOID OutputGetConfigurationFeatureRemovableMedium(
	PDEVICE pDevice,
	PFEATURE_DATA_REMOVABLE_MEDIUM pRemovableMedium
) {
	OutputDriveLog(
		"\tFeatureRemovableMedium\n"
		"\t\t        Lockable: %s\n"
		"\t\tDefaultToPrevent: %s\n"
		"\t\t           Eject: %s\n"
		"\t\tLoadingMechanism: ",
		BOOLEAN_TO_STRING_YES_NO(pRemovableMedium->Lockable),
		BOOLEAN_TO_STRING_YES_NO(pRemovableMedium->DefaultToPrevent),
		BOOLEAN_TO_STRING_YES_NO(pRemovableMedium->Eject));
	pDevice->byLoadingMechanism = pRemovableMedium->LoadingMechanism;
	switch (pRemovableMedium->LoadingMechanism) {
	case 0:
		OutputDriveLog("Caddy/Slot type loading mechanism\n");
		break;
	case 1:
		OutputDriveLog("Tray type loading mechanism\n");
		break;
	case 2:
		OutputDriveLog("Pop-up type loading mechanism\n");
		break;
	case 4:
		OutputDriveLog(
			"Embedded changer with individually changeable discs\n");
		break;
	case 5:
		OutputDriveLog(
			"Embedded changer using a magazine mechanism\n");
		break;
	default:
		OutputDriveLog(
			"Reserved: %08d\n", pRemovableMedium->LoadingMechanism);
		break;
	}
}

VOID OutputGetConfigurationFeatureWriteProtect(
	PFEATURE_DATA_WRITE_PROTECT pWriteProtect
) {
	OutputDriveLog(
		"\tFeatureWriteProtect\n"
		"\t\t               SupportsSWPPBit: %s\n"
		"\t\tSupportsPersistentWriteProtect: %s\n"
		"\t\t               WriteInhibitDCB: %s\n"
		"\t\t           DiscWriteProtectPAC: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pWriteProtect->SupportsSWPPBit),
		BOOLEAN_TO_STRING_YES_NO(pWriteProtect->SupportsPersistentWriteProtect),
		BOOLEAN_TO_STRING_YES_NO(pWriteProtect->WriteInhibitDCB),
		BOOLEAN_TO_STRING_YES_NO(pWriteProtect->DiscWriteProtectPAC));
}

VOID OutputGetConfigurationFeatureRandomReadable(
	PFEATURE_DATA_RANDOM_READABLE pRandomReadable
) {
	OutputDriveLog(
		"\tFeatureRandomReadable\n"
		"\t\t        LogicalBlockSize: %u\n"
		"\t\t                Blocking: %u\n"
		"\t\tErrorRecoveryPagePresent: %s\n",
		MAKEUINT(MAKEWORD(pRandomReadable->LogicalBlockSize[3], pRandomReadable->LogicalBlockSize[2]),
			MAKEWORD(pRandomReadable->LogicalBlockSize[1], pRandomReadable->LogicalBlockSize[0])),
		MAKEWORD(pRandomReadable->Blocking[1], pRandomReadable->Blocking[0]),
		BOOLEAN_TO_STRING_YES_NO(pRandomReadable->ErrorRecoveryPagePresent));
}

VOID OutputGetConfigurationFeatureMultiRead(
	PFEATURE_DATA_MULTI_READ pMultiRead
) {
	OutputDriveLog(
		"\tFeatureMultiRead\n"
		"\t\t   Current: %hhu\n"
		"\t\tPersistent: %hhu\n"
		"\t\t   Version: %hhu\n",
		pMultiRead->Header.Current,
		pMultiRead->Header.Persistent,
		pMultiRead->Header.Version);
}

VOID OutputGetConfigurationFeatureCdRead(
	PFEATURE_DATA_CD_READ pCDRead
) {
	OutputDriveLog(
		"\tFeatureCdRead\n"
		"\t\t          CDText: %s\n"
		"\t\t     C2ErrorData: %s\n"
		"\t\tDigitalAudioPlay: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pCDRead->CDText),
		BOOLEAN_TO_STRING_YES_NO(pCDRead->C2ErrorData),
		BOOLEAN_TO_STRING_YES_NO(pCDRead->DigitalAudioPlay));
}

VOID OutputGetConfigurationFeatureDvdRead(
	PFEATURE_DATA_DVD_READ pDVDRead
) {
	OutputDriveLog(
		"\tFeatureDvdRead\n"
		"\t\t  Multi110: %s\n"
		"\t\t DualDashR: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDVDRead->Multi110),
		BOOLEAN_TO_STRING_YES_NO(pDVDRead->DualDashR));
}

VOID OutputGetConfigurationFeatureRandomWritable(
	PFEATURE_DATA_RANDOM_WRITABLE pRandomWritable
) {
	OutputDriveLog(
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
		BOOLEAN_TO_STRING_YES_NO(pRandomWritable->ErrorRecoveryPagePresent));
}

VOID OutputGetConfigurationFeatureIncrementalStreamingWritable(
	PFEATURE_DATA_INCREMENTAL_STREAMING_WRITABLE pIncremental
) {
	OutputDriveLog(
		"\tFeatureIncrementalStreamingWritable\n"
		"\t\t        DataTypeSupported: %u\n"
		"\t\t       BufferUnderrunFree: %s\n"
		"\t\t   AddressModeReservation: %s\n"
		"\t\tTrackRessourceInformation: %s\n"
		"\t\t        NumberOfLinkSizes: %u\n",
		MAKEWORD(pIncremental->DataTypeSupported[1], pIncremental->DataTypeSupported[0]),
		BOOLEAN_TO_STRING_YES_NO(pIncremental->BufferUnderrunFree),
		BOOLEAN_TO_STRING_YES_NO(pIncremental->AddressModeReservation),
		BOOLEAN_TO_STRING_YES_NO(pIncremental->TrackRessourceInformation),
		pIncremental->NumberOfLinkSizes);
	for (UINT i = 0; i < pIncremental->NumberOfLinkSizes; i++) {
		OutputDriveLog(
			"\t\tLinkSize%u: %u\n", i, pIncremental->LinkSize[i]);
	}
}

VOID OutputGetConfigurationFeatureSectorErasable(
	PFEATURE_DATA_SECTOR_ERASABLE pSectorErasable
) {
	OutputDriveLog(
		"\tFeatureSectorErasable\n"
		"\t\t   Current: %hhu\n"
		"\t\tPersistent: %hhu\n"
		"\t\t   Version: %hhu\n",
		pSectorErasable->Header.Current,
		pSectorErasable->Header.Persistent,
		pSectorErasable->Header.Version);
}

VOID OutputGetConfigurationFeatureFormattable(
	PFEATURE_DATA_FORMATTABLE pFormattable
) {
	OutputDriveLog(
		"\tFeatureFormattable\n"
		"\t\t FullCertification: %s\n"
		"\t\tQuickCertification: %s\n"
		"\t\tSpareAreaExpansion: %s\n"
		"\t\tRENoSpareAllocated: %s\n"
		"\t\t   RRandomWritable: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pFormattable->FullCertification),
		BOOLEAN_TO_STRING_YES_NO(pFormattable->QuickCertification),
		BOOLEAN_TO_STRING_YES_NO(pFormattable->SpareAreaExpansion),
		BOOLEAN_TO_STRING_YES_NO(pFormattable->RENoSpareAllocated),
		BOOLEAN_TO_STRING_YES_NO(pFormattable->RRandomWritable));
}

VOID OutputGetConfigurationFeatureDefectManagement(
	PFEATURE_DATA_DEFECT_MANAGEMENT pDefect
) {
	OutputDriveLog(
		"\tFeatureDefectManagement\n"
		"\t\tSupplimentalSpareArea: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDefect->SupplimentalSpareArea));
}

VOID OutputGetConfigurationFeatureWriteOnce(
	PFEATURE_DATA_WRITE_ONCE pWriteOnce
) {
	OutputDriveLog(
		"\tFeatureWriteOnce\n"
		"\t\t        LogicalBlockSize: %u\n"
		"\t\t                Blocking: %u\n"
		"\t\tErrorRecoveryPagePresent: %s\n",
		MAKEUINT(MAKEWORD(pWriteOnce->LogicalBlockSize[3], pWriteOnce->LogicalBlockSize[2]),
			MAKEWORD(pWriteOnce->LogicalBlockSize[1], pWriteOnce->LogicalBlockSize[0])),
		MAKEWORD(pWriteOnce->Blocking[1], pWriteOnce->Blocking[0]),
		BOOLEAN_TO_STRING_YES_NO(pWriteOnce->ErrorRecoveryPagePresent));
}

VOID OutputGetConfigurationFeatureRestrictedOverwrite(
	PFEATURE_DATA_RESTRICTED_OVERWRITE pRestricted
) {
	OutputDriveLog(
		"\tFeatureRestrictedOverwrite\n"
		"\t\t   Current: %hhu\n"
		"\t\tPersistent: %hhu\n"
		"\t\t   Version: %hhu\n",
		pRestricted->Header.Current,
		pRestricted->Header.Persistent,
		pRestricted->Header.Version);
}

VOID OutputGetConfigurationFeatureCdrwCAVWrite(
	PFEATURE_DATA_CDRW_CAV_WRITE pCDRW
) {
	OutputDriveLog(
		"\tFeatureCdrwCAVWrite\n"
		"\t\t   Current: %hhu\n"
		"\t\tPersistent: %hhu\n"
		"\t\t   Version: %hhu\n",
		pCDRW->Header.Current,
		pCDRW->Header.Persistent,
		pCDRW->Header.Version);
}

VOID OutputGetConfigurationFeatureMrw(
	PFEATURE_DATA_MRW pMrw
) {
	OutputDriveLog(
		"\tFeatureMrw\n"
		"\t\t       Write: %s\n"
		"\t\t DvdPlusRead: %s\n"
		"\t\tDvdPlusWrite: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pMrw->Write),
		BOOLEAN_TO_STRING_YES_NO(pMrw->DvdPlusRead),
		BOOLEAN_TO_STRING_YES_NO(pMrw->DvdPlusWrite));
}

VOID OutputGetConfigurationFeatureEnhancedDefectReporting(
	PFEATURE_ENHANCED_DEFECT_REPORTING pEnhanced
) {
	OutputDriveLog(
		"\tFeatureEnhancedDefectReporting\n"
		"\t\t       DRTDMSupported: %s\n"
		"\t\tNumberOfDBICacheZones: %u\n"
		"\t\t      NumberOfEntries: %u\n",
		BOOLEAN_TO_STRING_YES_NO(pEnhanced->DRTDMSupported),
		pEnhanced->NumberOfDBICacheZones,
		MAKEWORD(pEnhanced->NumberOfEntries[1], pEnhanced->NumberOfEntries[0]));
}

VOID OutputGetConfigurationFeatureDvdPlusRW(
	PFEATURE_DATA_DVD_PLUS_RW pDVDPLUSRW
) {
	OutputDriveLog(
		"\tFeatureDvdPlusRW\n"
		"\t\t     Write: %s\n"
		"\t\t CloseOnly: %s\n"
		"\t\tQuickStart: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDVDPLUSRW->Write),
		BOOLEAN_TO_STRING_YES_NO(pDVDPLUSRW->CloseOnly),
		BOOLEAN_TO_STRING_YES_NO(pDVDPLUSRW->QuickStart));
}

VOID OutputGetConfigurationFeatureDvdPlusR(
	PFEATURE_DATA_DVD_PLUS_R pDVDPLUSR
) {
	OutputDriveLog(
		"\tFeatureDvdPlusR\n"
		"\t\tWrite: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDVDPLUSR->Write));
}

VOID OutputGetConfigurationFeatureRigidRestrictedOverwrite(
	PFEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE pDVDRWRestricted
) {
	OutputDriveLog(
		"\tFeatureRigidRestrictedOverwrite\n"
		"\t\t                   Blank: %s\n"
		"\t\t            Intermediate: %s\n"
		"\t\t    DefectStatusDataRead: %s\n"
		"\t\tDefectStatusDataGenerate: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDVDRWRestricted->Blank),
		BOOLEAN_TO_STRING_YES_NO(pDVDRWRestricted->Intermediate),
		BOOLEAN_TO_STRING_YES_NO(pDVDRWRestricted->DefectStatusDataRead),
		BOOLEAN_TO_STRING_YES_NO(pDVDRWRestricted->DefectStatusDataGenerate));
}

VOID OutputGetConfigurationFeatureCdTrackAtOnce(
	PFEATURE_DATA_CD_TRACK_AT_ONCE pCDTrackAtOnce
) {
	OutputDriveLog(
		"\tFeatureCdTrackAtOnce\n"
		"\t\tRWSubchannelsRecordable: %s\n"
		"\t\t           CdRewritable: %s\n"
		"\t\t            TestWriteOk: %s\n"
		"\t\t   RWSubchannelPackedOk: %s\n"
		"\t\t      RWSubchannelRawOk: %s\n"
		"\t\t     BufferUnderrunFree: %s\n"
		"\t\t      DataTypeSupported: %u\n",
		BOOLEAN_TO_STRING_YES_NO(pCDTrackAtOnce->RWSubchannelsRecordable),
		BOOLEAN_TO_STRING_YES_NO(pCDTrackAtOnce->CdRewritable),
		BOOLEAN_TO_STRING_YES_NO(pCDTrackAtOnce->TestWriteOk),
		BOOLEAN_TO_STRING_YES_NO(pCDTrackAtOnce->RWSubchannelPackedOk),
		BOOLEAN_TO_STRING_YES_NO(pCDTrackAtOnce->RWSubchannelRawOk),
		BOOLEAN_TO_STRING_YES_NO(pCDTrackAtOnce->BufferUnderrunFree),
		MAKEWORD(pCDTrackAtOnce->DataTypeSupported[1], pCDTrackAtOnce->DataTypeSupported[0]));
}

VOID OutputGetConfigurationFeatureCdMastering(
	PFEATURE_DATA_CD_MASTERING pCDMastering
) {
	OutputDriveLog(
		"\tFeatureCdMastering\n"
		"\t\tRWSubchannelsRecordable: %s\n"
		"\t\t           CdRewritable: %s\n"
		"\t\t            TestWriteOk: %s\n"
		"\t\t        RRawRecordingOk: %s\n"
		"\t\t      RawMultiSessionOk: %s\n"
		"\t\t        SessionAtOnceOk: %s\n"
		"\t\t     BufferUnderrunFree: %s\n"
		"\t\t  MaximumCueSheetLength: %u\n",
		BOOLEAN_TO_STRING_YES_NO(pCDMastering->RWSubchannelsRecordable),
		BOOLEAN_TO_STRING_YES_NO(pCDMastering->CdRewritable),
		BOOLEAN_TO_STRING_YES_NO(pCDMastering->TestWriteOk),
		BOOLEAN_TO_STRING_YES_NO(pCDMastering->RawRecordingOk),
		BOOLEAN_TO_STRING_YES_NO(pCDMastering->RawMultiSessionOk),
		BOOLEAN_TO_STRING_YES_NO(pCDMastering->SessionAtOnceOk),
		BOOLEAN_TO_STRING_YES_NO(pCDMastering->BufferUnderrunFree),
		MAKEUINT(MAKEWORD(0, pCDMastering->MaximumCueSheetLength[2]),
			MAKEWORD(pCDMastering->MaximumCueSheetLength[1], pCDMastering->MaximumCueSheetLength[0])));
}

VOID OutputGetConfigurationFeatureDvdRecordableWrite(
	PFEATURE_DATA_DVD_RECORDABLE_WRITE pDVDRecordable
) {
	OutputDriveLog(
		"\tFeatureDvdRecordableWrite\n"
		"\t\t            DVD_RW: %s\n"
		"\t\t         TestWrite: %s\n"
		"\t\t        RDualLayer: %s\n"
		"\t\tBufferUnderrunFree: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDVDRecordable->DVD_RW),
		BOOLEAN_TO_STRING_YES_NO(pDVDRecordable->TestWrite),
		BOOLEAN_TO_STRING_YES_NO(pDVDRecordable->RDualLayer),
		BOOLEAN_TO_STRING_YES_NO(pDVDRecordable->BufferUnderrunFree));
}

VOID OutputGetConfigurationFeatureLayerJumpRecording(
	PFEATURE_DATA_LAYER_JUMP_RECORDING pLayerJumpRec
) {
	OutputDriveLog(
		"\tFeatureLayerJumpRecording\n"
		"\t\tNumberOfLinkSizes: %u\n",
		pLayerJumpRec->NumberOfLinkSizes);
	for (UINT i = 0; i < pLayerJumpRec->NumberOfLinkSizes; i++) {
		OutputDriveLog(
			"\t\tLinkSize %u: %u\n", i, pLayerJumpRec->LinkSizes[i]);
	}
}

VOID OutputGetConfigurationFeatureCDRWMediaWriteSupport(
	PFEATURE_CD_RW_MEDIA_WRITE_SUPPORT pCDRWMediaWrite
) {
	OutputDriveLog(
		"\tFeatureCDRWMediaWriteSupport\n"
		"\t\tSubtype 0: %s\n"
		"\t\tSubtype 1: %s\n"
		"\t\tSubtype 2: %s\n"
		"\t\tSubtype 3: %s\n"
		"\t\tSubtype 4: %s\n"
		"\t\tSubtype 5: %s\n"
		"\t\tSubtype 6: %s\n"
		"\t\tSubtype 7: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype0),
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype1),
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype2),
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype3),
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype4),
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype5),
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype6),
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype7));
}

VOID OutputGetConfigurationFeatureBDRPseudoOverwrite(
	PFEATURE_BD_R_PSEUDO_OVERWRITE pBDRPseudoOverwrite
) {
	UNREFERENCED_PARAMETER(pBDRPseudoOverwrite);
	OutputDriveLog(
		"\tFeatureBDRPseudoOverwrite\n");
}

VOID OutputGetConfigurationFeatureDvdPlusRWDualLayer(
	PFEATURE_DATA_DVD_PLUS_RW_DUAL_LAYER pDVDPlusRWDL
) {
	OutputDriveLog(
		"\tFeatureDvdPlusRWDualLayer\n"
		"\t\t     Write: %s\n"
		"\t\t CloseOnly: %s\n"
		"\t\tQuickStart: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDVDPlusRWDL->Write),
		BOOLEAN_TO_STRING_YES_NO(pDVDPlusRWDL->CloseOnly),
		BOOLEAN_TO_STRING_YES_NO(pDVDPlusRWDL->QuickStart));
}

VOID OutputGetConfigurationFeatureDvdPlusRDualLayer(
	PFEATURE_DATA_DVD_PLUS_R_DUAL_LAYER pDVDPlusRDL
) {
	OutputDriveLog(
		"\tFeatureDvdPlusRDualLayer\n"
		"\t\tWrite: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDVDPlusRDL->Write));
}

VOID OutputGetConfigurationFeatureBDRead(
	PFEATURE_BD_READ pBDRead
) {
	OutputDriveLog(
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
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDREReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDREReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDREReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDREReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDRReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDRReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDRReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDRReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class0BitmapBDROMReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class1BitmapBDROMReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class2BitmapBDROMReadSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDRead->Class3BitmapBDROMReadSupport.Version7));
}

VOID OutputGetConfigurationFeatureBDWrite(
	PFEATURE_BD_WRITE pBDWrite
) {
	OutputDriveLog(
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
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->SupportsVerifyNotRequired)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDREWriteSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDREWriteSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDREWriteSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDREWriteSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class0BitmapBDRWriteSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class1BitmapBDRWriteSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class2BitmapBDRWriteSupport.Version7)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version8)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version9)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version10)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version11)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version12)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version13)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version14)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version15)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version0)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version1)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version2)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version3)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version4)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version5)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version6)
		, BOOLEAN_TO_STRING_YES_NO(pBDWrite->Class3BitmapBDRWriteSupport.Version7));
}

VOID OutputGetConfigurationFeatureTSR(
	PFEATURE_TSR pTsr
) {
	UNREFERENCED_PARAMETER(pTsr);
	OutputDriveLog(
		"\tFeatureTSR\n");
}

VOID OutputGetConfigurationFeatureHDDVDRead(
	PFEATURE_DATA_HDDVD_READ pHdRead
) {
	OutputDriveLog(
		"\tFeatureHDDVDRead\n"
		"\t\tRecordable: %s\n"
		"\t\tRewritable: %s\n"
		, BOOLEAN_TO_STRING_YES_NO(pHdRead->Recordable)
		, BOOLEAN_TO_STRING_YES_NO(pHdRead->Rewritable));
}

VOID OutputGetConfigurationFeatureHDDVDWrite(
	PFEATURE_DATA_HDDVD_WRITE pHdWrite
) {
	OutputDriveLog(
		"\tFeatureHDDVDWrite\n"
		"\t\tRecordable: %s\n"
		"\t\tRewritable: %s\n"
		, BOOLEAN_TO_STRING_YES_NO(pHdWrite->Recordable)
		, BOOLEAN_TO_STRING_YES_NO(pHdWrite->Rewritable));
}

VOID OutputGetConfigurationFeatureHybridDisc(
	PFEATURE_HYBRID_DISC pHybridDisc
) {
	OutputDriveLog(
		"\tFeatureHybridDisc\n"
		"\t\tResetImmunity: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pHybridDisc->ResetImmunity));
}

VOID OutputGetConfigurationFeaturePowerManagement(
	PFEATURE_DATA_POWER_MANAGEMENT pPower
) {
	OutputDriveLog(
		"\tFeaturePowerManagement\n"
		"\t\t   Current: %hhu\n"
		"\t\tPersistent: %hhu\n"
		"\t\t   Version: %hhu\n",
		pPower->Header.Current,
		pPower->Header.Persistent,
		pPower->Header.Version);
}

VOID OutputGetConfigurationFeatureSMART(
	PFEATURE_DATA_SMART pSmart
) {
	OutputDriveLog(
		"\tFeatureSMART\n"
		"\t\tFaultFailureReportingPagePresent: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pSmart->FaultFailureReportingPagePresent));
}

VOID OutputGetConfigurationFeatureEmbeddedChanger(
	PFEATURE_DATA_EMBEDDED_CHANGER pEmbedded
) {
	OutputDriveLog(
		"\tFeatureEmbeddedChanger\n"
		"\t\tSupportsDiscPresent: %s\n"
		"\t\t  SideChangeCapable: %s\n"
		"\t\t  HighestSlotNumber: %hhu\n",
		BOOLEAN_TO_STRING_YES_NO(pEmbedded->SupportsDiscPresent),
		BOOLEAN_TO_STRING_YES_NO(pEmbedded->SideChangeCapable),
		pEmbedded->HighestSlotNumber);
}

VOID OutputGetConfigurationFeatureCDAudioAnalogPlay(
	PFEATURE_DATA_CD_AUDIO_ANALOG_PLAY pCDAudio
) {
	OutputDriveLog(
		"\tFeatureCDAudioAnalogPlay\n"
		"\t\t     SeperateVolume: %s\n"
		"\t\tSeperateChannelMute: %s\n"
		"\t\t      ScanSupported: %s\n"
		"\t\tNumerOfVolumeLevels: %u\n",
		BOOLEAN_TO_STRING_YES_NO(pCDAudio->SeperateVolume),
		BOOLEAN_TO_STRING_YES_NO(pCDAudio->SeperateChannelMute),
		BOOLEAN_TO_STRING_YES_NO(pCDAudio->ScanSupported),
		MAKEWORD(pCDAudio->NumerOfVolumeLevels[1], pCDAudio->NumerOfVolumeLevels[0]));
}

VOID OutputGetConfigurationFeatureMicrocodeUpgrade(
	PFEATURE_DATA_MICROCODE_UPDATE pMicrocode
) {
	OutputDriveLog(
		"\tFeatureMicrocodeUpgrade\n"
		"\t\tM5: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pMicrocode->M5));
}

VOID OutputGetConfigurationFeatureTimeout(
	PFEATURE_DATA_TIMEOUT pTimeOut
) {
	OutputDriveLog(
		"\tFeatureTimeout\n"
		"\t\t    Group3: %s\n"
		"\t\tUnitLength: %u\n",
		BOOLEAN_TO_STRING_YES_NO(pTimeOut->Group3),
		MAKEWORD(pTimeOut->UnitLength[1], pTimeOut->UnitLength[0]));
}

VOID OutputGetConfigurationFeatureDvdCSS(
	PFEATURE_DATA_DVD_CSS pDVDCss
) {
	OutputDriveLog(
		"\tFeatureDvdCSS\n"
		"\t\tCssVersion: %hhu\n",
		pDVDCss->CssVersion);
}

VOID OutputGetConfigurationFeatureRealTimeStreaming(
	PFEATURE_DATA_REAL_TIME_STREAMING pRealTimeStreaming
) {
	OutputDriveLog(
		"\tFeatureRealTimeStreaming\n"
		"\t\t        StreamRecording: %s\n"
		"\t\t    WriteSpeedInGetPerf: %s\n"
		"\t\t       WriteSpeedInMP2A: %s\n"
		"\t\t             SetCDSpeed: %s\n"
		"\t\tReadBufferCapacityBlock: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pRealTimeStreaming->StreamRecording),
		BOOLEAN_TO_STRING_YES_NO(pRealTimeStreaming->WriteSpeedInGetPerf),
		BOOLEAN_TO_STRING_YES_NO(pRealTimeStreaming->WriteSpeedInMP2A),
		BOOLEAN_TO_STRING_YES_NO(pRealTimeStreaming->SetCDSpeed),
		BOOLEAN_TO_STRING_YES_NO(pRealTimeStreaming->ReadBufferCapacityBlock));
}

VOID OutputGetConfigurationFeatureLogicalUnitSerialNumber(
	PFEATURE_DATA_LOGICAL_UNIT_SERIAL_NUMBER pLogical
) {
	OutputDriveLog(
		"\tFeatureLogicalUnitSerialNumber\n"
		"\t\tSerialNumber: ");
	for (INT i = 0; i < pLogical->Header.AdditionalLength; i++) {
		OutputDriveLog("%c", pLogical->SerialNumber[i]);
	}
	OutputDriveLog("\n");
}

VOID OutputGetConfigurationFeatureMediaSerialNumber(
	PFEATURE_MEDIA_SERIAL_NUMBER pMediaSerialNumber
) {
	OutputDriveLog(
		"\tFeatureMediaSerialNumber\n"
		"\t\t   Current: %hhu\n"
		"\t\tPersistent: %hhu\n"
		"\t\t   Version: %hhu\n",
		pMediaSerialNumber->Header.Current,
		pMediaSerialNumber->Header.Persistent,
		pMediaSerialNumber->Header.Version);
}

VOID OutputGetConfigurationFeatureDiscControlBlocks(
	PFEATURE_DATA_DISC_CONTROL_BLOCKS pDiscCtrlBlk
) {
	OutputDriveLog("\tFeatureDiscControlBlocks\n");
	for (UINT i = 0; i < pDiscCtrlBlk->Header.AdditionalLength; i++) {
		OutputDriveLog(
			"\t\tContentDescriptor %02u: %08ld\n", i,
			MAKELONG(
				MAKEWORD(pDiscCtrlBlk->Data[i].ContentDescriptor[3], pDiscCtrlBlk->Data[i].ContentDescriptor[2]),
				MAKEWORD(pDiscCtrlBlk->Data[i].ContentDescriptor[1], pDiscCtrlBlk->Data[i].ContentDescriptor[0])));
	}
}

VOID OutputGetConfigurationFeatureDvdCPRM(
	PFEATURE_DATA_DVD_CPRM pDVDCprm
) {
	OutputDriveLog(
		"\tFeatureDvdCPRM\n"
		"\t\tCPRMVersion: %hhu\n",
		pDVDCprm->CPRMVersion);
}

VOID OutputGetConfigurationFeatureFirmwareDate(
	PFEATURE_DATA_FIRMWARE_DATE pFirmwareDate
) {
	OutputDriveLog(
		"\tFeatureFirmwareDate: %.4" CHARWIDTH "s-%.2" CHARWIDTH "s-%.2" CHARWIDTH "sT%.2" CHARWIDTH "s:%.2" CHARWIDTH "s:%.2" CHARWIDTH "s\n"
		, pFirmwareDate->Year, pFirmwareDate->Month, pFirmwareDate->Day
		, pFirmwareDate->Hour, pFirmwareDate->Minute, pFirmwareDate->Seconds);
}

VOID OutputGetConfigurationFeatureAACS(
	PFEATURE_DATA_AACS pAACS
) {
	OutputDriveLog(
		"\tFeatureAACS\n"
		"\t\tBindingNonceGeneration: %s\n"
		"\t\tBindingNonceBlockCount: %u\n"
		"\t\t         NumberOfAGIDs: %hhu\n"
		"\t\t           AACSVersion: %hhu\n",
		BOOLEAN_TO_STRING_YES_NO(pAACS->BindingNonceGeneration),
		pAACS->BindingNonceBlockCount,
		pAACS->NumberOfAGIDs,
		pAACS->AACSVersion);
}

VOID OutputGetConfigurationFeatureVCPS(
	PFEATURE_VCPS pVcps
) {
	OutputDriveLog(
		"\tFeatureVCPS\n"
		"\t\t   Current: %hhu\n"
		"\t\tPersistent: %hhu\n"
		"\t\t   Version: %hhu\n",
		pVcps->Header.Current,
		pVcps->Header.Persistent,
		pVcps->Header.Version);
}

VOID OutputGetConfigurationFeatureReserved(
	PFEATURE_DATA_RESERVED pReserved
) {
	OutputDriveLog(
		"\tReserved (FeatureCode[%#04x])\n"
		"\t\tData: ", MAKEWORD(pReserved->Header.FeatureCode[1], pReserved->Header.FeatureCode[0]));
	for (INT i = 0; i < pReserved->Header.AdditionalLength; i++) {
		OutputDriveLog("%02x", pReserved->Data[i]);
	}
	OutputDriveLog("\n");
}

VOID OutputGetConfigurationFeatureVendorSpecific(
	PFEATURE_DATA_VENDOR_SPECIFIC pVendorSpecific
) {
	OutputDriveLog(
		"\tVendorSpecific (FeatureCode[%#04x])\n"
		"\t\tVendorSpecificData: ",
		MAKEWORD(pVendorSpecific->Header.FeatureCode[1], pVendorSpecific->Header.FeatureCode[0]));
	for (INT i = 0; i < pVendorSpecific->Header.AdditionalLength; i++) {
		OutputDriveLog("%02x", pVendorSpecific->VendorSpecificData[i]);
	}
	OutputDriveLog("\n");
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
			OutputGetConfigurationFeatureRemovableMedium(pDevice, (PFEATURE_DATA_REMOVABLE_MEDIUM)&lpConf[n]);
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

VOID OutputCDAtipSI1(
	PCDROM_TOC_ATIP_DATA_BLOCK pAtip
) {
//	BYTE tmp[3] = {};
//	memcpy(tmp, pAtip, 3);
	OutputDiscLog(
		"\tSpecial Information 1\n"
//		"\t\t            msf: %02u:%02u:%02u\n"
		"\t\t     WritePower: %hhu\n"
		"\t\t ReferenceSpeed: %hhu\n"
		"\t\tUnrestrictedUse: %s\n"
//		, tmp[0], tmp[1], tmp[2]
		, pAtip->WritePower
		, pAtip->CdrwReferenceSpeed
		, BOOLEAN_TO_STRING_YES_NO(pAtip->UnrestrictedUse)
	);
	switch (pAtip->IsCdrw)
	{
	case 0:
		OutputDiscLog("\t\t       DiscType: %hhu (CD-R), DiscSubType: %hhu \n"
			, pAtip->IsCdrw, pAtip->DiscSubType);
		break;
	case 1:
		OutputDiscLog("\t\t       DiscType: %hhu (CD-RW), DiscSubType: %hhu "
			, pAtip->IsCdrw, pAtip->DiscSubType);
		switch (pAtip->DiscSubType)
		{
		case 0:
			OutputDiscLog("(Standard Speed (1x-4x))\n");
			break;
		case 1:
			OutputDiscLog("(High Speed (4x-10x))\n");
			break;
		case 2:
			OutputDiscLog("(Ultra Speed (8x-24x))\n");
			break;
		case 3:
			OutputDiscLog("(Ultra Speed (8x-32x))\n");
			break;
		default:
			OutputDiscLog("(Unknown)\n");
			break;
		}
		break;
	default:
		OutputDiscLog("\t\t       DiscType: Unknown\n");
		break;
	}
}

VOID OutputCDAtipSI2(
	PCDROM_TOC_ATIP_DATA_BLOCK pAtip
) {
	OutputDiscLog(
		"\tSpecial Information 2\n"
		"\t\t      LeadInMsf: %02u:%02u:%02u\n"
		"\t\t       => Manufacturer: "
		, pAtip->LeadInMsf[0], pAtip->LeadInMsf[1], pAtip->LeadInMsf[2]
	);
	// http://web.archive.org/web/20061027043819/www.cdr-forum.de/download/cdr.pdf
	BYTE s = pAtip->LeadInMsf[1];
	BYTE f = pAtip->LeadInMsf[2];
	if (s == 21 && 40 <= f && f <= 49) {
		OutputDiscLog("Optical Disc Manufacturing Equipment\n");
	}
	else if (s == 22 && f <= 9) {
		OutputDiscLog("Woongjin Media corp.\n");
	}
	else if ((s == 22 || s == 45) && 40 <= f && f <= 49) {
		OutputDiscLog("CIS Technology Inc.\n");
	}
	else if ((s == 23 && f <= 9) || (s == 49 && 60 <= f && f <= 69)) {
		OutputDiscLog("Matsushita Electric Industrial Co., Ltd.\n");
	}
	else if (s == 23 && 10 <= f && f <= 19) {
		OutputDiscLog("Doremi Media Co., Ltd.\n");
	}
	else if ((s == 24 || s == 46) && f <= 9) {
		OutputDiscLog("Taiyo Yuden Company Limited\n");
	}
	else if ((s == 24 || s == 46) && 10 <= f && f <= 19) {
		OutputDiscLog("SONY Corporation\n");
	}
	else if ((s == 24 && 20 <= f && f <= 29) || (s == 46 && 30 <= f && f <= 39)) {
		OutputDiscLog("Computer Support Italy s.r.l.\n");
	}
	else if ((s == 24 && 30 <= f && f <= 39) || (s == 45 && 10 <= f && f <= 19)) {
		OutputDiscLog("UNITECH JAPAN INC.\n");
	}
	else if ((s == 25 && 20 <= f && f <= 29) || (s == 47 && 10 <= f && f <= 19)) {
		OutputDiscLog("Hitachi Maxell, Ltd.\n");
	}
	else if ((s == 25 && 30 <= f && f <= 39) || (s == 51 && 20 <= f && f <= 29)) {
		OutputDiscLog("INFODISC Technology Co., Ltd.\n");
	}
	else if (s == 25 && 50 <= f && f <= 59) {
		OutputDiscLog("AMS Technology Inc.\n");
	}
	else if ((s == 25 || s == 45) && 60 <= f && f <= 69) {
		OutputDiscLog("Xcitek Inc.\n");
	}
	else if ((s == 26 || s == 45) && f <= 9) {
		OutputDiscLog("FORNET INTERNATIONAL PTE LTD.\n");
	}
	else if ((s == 26 && 10 <= f && f <= 19) || (s == 47 && 40 <= f && f <= 49)) {
		OutputDiscLog("POSTECH Corporation\n");
	}
	else if (s == 26 && 20 <= f && f <= 29) {
		OutputDiscLog("SKC Co., Ltd.n");
	}
	else if (s == 26 && 30 <= f && f <= 39) {
		OutputDiscLog("OPTICAL DISC CORPRATION\n");
	}
	else if ((s == 26 || s == 46) && 40 <= f && f <= 49) {
		OutputDiscLog("FUJI Photo Film Co., Ltd\n");
	}
	else if ((s == 26 && 50 <= f && f <= 59) || (s == 48 && 60 <= f && f <= 69)) {
		OutputDiscLog("POSTECH Corporation\n");
	}
	else if ((s == 26 || s == 46) && 60 <= f && f <= 69) {
		OutputDiscLog("CMC Magnetics Corporation\n");
	}
	else if ((s == 27 && 1 <= f && f <= 9) || (s == 48 && 40 <= f && f <= 49)) {
		OutputDiscLog("DIGITAL STORAGE TECHNOLOGY CO., LTD\n");
	}
	else if ((s == 27 && 10 <= f && f <= 19) || (s == 48 && 20 <= f && f <= 29)) {
		OutputDiscLog("Kodak Japan Limited\n");
	}
	else if ((s == 27 || s == 47) && 20 <= f && f <= 29) {
		OutputDiscLog("Princo Corporation\n");
	}
	else if ((s == 27 || s == 48) && 30 <= f && f <= 39) {
		OutputDiscLog("Pioneer Video Corporation\n");
	}
	else if ((s == 27 && 40 <= f && f <= 49) || (s == 48 && 10 <= f && f <= 19)) {
		OutputDiscLog("Kodak Japan Limited\n");
	}
	else if ((s == 27 || s == 48) && 50 <= f && f <= 59) {
		OutputDiscLog("Mitsui Chemicals, Inc.\n");
	}
	else if ((s == 27 && 60 <= f && f <= 69) || (s == 48 && f <= 9)) {
		OutputDiscLog("Ricoh Company Limited\n");
	}
	else if ((s == 28 || s == 49) && 10 <= f && f <= 19) {
		OutputDiscLog("GIGASTORAGE CORPORATION\n");
	}
	else if ((s == 28 || s == 46) && 20 <= f && f <= 29) {
		OutputDiscLog("Multi Media Masters & Machinary SA\n");
	}
	else if ((s == 28 && 30 <= f && f <= 39) || (s == 46 && 50 <= f && f <= 59)) {
		OutputDiscLog("Auvistar Industry Co., Ltd.\n");
	}
	else if ((s == 28 && 40 <= f && f <= 49) || (s == 48 && 20 <= f && f <= 29)) {
		OutputDiscLog("King Pro Mediatek Inc.\n");
	}
	else if (s == 29 && f <= 9) {
		OutputDiscLog("Taeil Media Co., Ltd.\n");
	}
	else if ((s == 29 || s == 50) && 10 <= f && f <= 19) {
		OutputDiscLog("Vanguard Disc Inc.\n");
	}
	else if ((s == 30 && 10 <= f && f <= 19) || (s == 50 && 30 <= f && f <= 39)) {
		OutputDiscLog("CDA Datentrager Albrechts GmbH\n");
	}
	else if ((s == 31 && f <= 9) || (s == 47 && 50 <= f && f <= 59)) {
		OutputDiscLog("Ritek Co.\n");
	}
	else if ((s == 31 && 30 <= f && f <= 39) || (s == 51 && 10 <= f && f <= 19)) {
		OutputDiscLog("Grand Advance Technology Ltd.\n");
	}
	else if ((s == 32 || s == 49) && f <= 9) {
		OutputDiscLog("TDK Corporation\n");
	}
	else if ((s == 32 && 10 <= f && f <= 19) || (s == 47 && 60 <= f && f <= 69)) {
		OutputDiscLog("Grand Advance Technology Ltd.\n");
	}
	else if ((s == 34 || s == 50) && 20 <= f && f <= 29) {
		OutputDiscLog("Mitsubishi Chemical Corporation\n");
	}
	else {
		OutputDiscLog("Unknown\n");
	}

	INT rc = pAtip->LeadInMsf[2] % 10;
	OutputDiscLog("               => Recording characteristics: ");
	if (0 <= rc && rc <= 4) {
		OutputDiscLog("Long Strategy Type (ex. Cyanine type media)\n");
	}
	else if (5 <= rc && rc <= 9) {
		OutputDiscLog("Short Strategy Type (ex. Phthalocyanine type media)\n");
	}
}

VOID OutputCDAtipAI1(
	PCDROM_TOC_ATIP_DATA_BLOCK pAtip
) {
	if (pAtip->A1Valid) {
		OutputDiscLog(
			"\tAdditional Information 1\n"
			"\t\t       A1Values: %02u:%02u:%02u\n"
			, pAtip->A1Values[0], pAtip->A1Values[1], pAtip->A1Values[2]
		);

		OutputDiscLog(
			"\t\t        => Lowest Test Speed: %d "
			, pAtip->A1Values[0] >> 4 & 0x07
		);
		switch (pAtip->A1Values[0] >> 4 & 0x07) {
		case 0:
			OutputDiscLog("(1x nominal CD Speed)\n");
			break;
		case 2:
			OutputDiscLog("(4x nominal CD Speed)\n");
			break;
		case 6:
			OutputDiscLog("(16x nominal CD Speed)\n");
			break;
		default:
			OutputDiscLog("(Unknown CD Speed)\n");
			break;
		}

		OutputDiscLog(
			"\t\t        => Highest Test Speed: %d "
			, pAtip->A1Values[0] & 0x0f
		);
		switch (pAtip->A1Values[0] & 0x0f) {
		case 2:
			OutputDiscLog("(4x nominal CD Speed)\n");
			break;
		case 4:
			OutputDiscLog("(10x nominal CD Speed)\n");
			break;
		case 6:
			OutputDiscLog("(16x nominal CD Speed)\n");
			break;
		case 7:
			OutputDiscLog("(20x nominal CD Speed)\n");
			break;
		case 8:
			OutputDiscLog("(24x nominal CD Speed)\n");
			break;
		case 9:
			OutputDiscLog("(32x nominal CD Speed)\n");
			break;
		case 10:
			OutputDiscLog("(40x nominal CD Speed)\n");
			break;
		case 11:
			OutputDiscLog("(48x nominal CD Speed)\n");
			break;
		default:
			OutputDiscLog("(Unknown CD Speed)\n");
			break;
		}

		if (pAtip->IsCdrw) {
			OutputDiscLog(
				"\t\t        => Power Multiplication Factor p: %d\n"
				"\t\t        => Target g value of the modulation/power function: %d\n"
				"\t\t        => Recommended erase/write power ratio e: %d\n"
				"\t\t        => Write-strategy optimization for dTtop: %d\n"
				"\t\t        => Write-strategy optimization for dTera: %d\n"
				, pAtip->A1Values[1] >> 4 & 0x07
				, pAtip->A1Values[1] >> 1 & 0x03
				, pAtip->A1Values[2] >> 4 & 0x07
				, pAtip->A1Values[2] >> 2 & 0x03
				, pAtip->A1Values[2] & 0x03
			);
		}
		else {
			OutputDiscLog(
				"\t\t        => High-Speed subtype: %d\n"
				"\t\t        => Optimum B-range: %d "
				, pAtip->A1Values[1] >> 4 & 0x07
				, pAtip->A1Values[1] & 0x03
			);
			switch (pAtip->A1Values[1] & 0x03) {
			case 0:
				OutputDiscLog("(-8 ~ 0 %%)\n");
				break;
			case 1:
				OutputDiscLog("(-4 ~ 4 %%)\n");
				break;
			case 2:
				OutputDiscLog("(-0 ~ +8 %%)\n");
				break;
			case 3:
				OutputDiscLog("(+4 ~ +12 %%)\n");
				break;
			default:
				OutputDiscLog("other\n");
				break;
			};

			OutputDiscLog(
				"\t\t        => Optimum pulse length: %d "
				, pAtip->A1Values[2] >> 4 & 0x07
			);
			switch (pAtip->A1Values[2] >> 4 & 0x07) {
			case 0:
				OutputDiscLog("(n-0)T\n");
				break;
			case 1:
				OutputDiscLog("(n-0.25)T\n");
				break;
			case 2:
				OutputDiscLog("(n-0.50)T\n");
				break;
			case 3:
				OutputDiscLog("(n-0.75)T\n");
				break;
			case 4:
				OutputDiscLog("(n+0.25)T\n");
				break;
			case 5:
				OutputDiscLog("(n+0.50)T\n");
				break;
			case 6:
				OutputDiscLog("(n+0.75)T\n");
				break;
			case 7:
				OutputDiscLog("(n+1.00)T\n");
				break;
			default:
				OutputDiscLog("other\n");
				break;
			};

			OutputDiscLog(
				"\t\t        => Length of Additional Capacity & Lead-out Area: %d "
				, pAtip->A1Values[2] & 0x0f
			);
			if ((pAtip->A1Values[2] & 0x0f) == 0) {
				OutputDiscLog("(2 minutes)\n");
			}
			else {
				OutputDiscLog("other\n");
			}
		}
	}
}

VOID OutputCDAtip(
	PCDROM_TOC_ATIP_DATA_BLOCK pAtip
) {
	OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("TOC ATIP"));
	OutputCDAtipSI1(pAtip);
	OutputCDAtipSI2(pAtip);

	OutputDiscLog(
		"\tSpecial Information 3\n"
		"\t\t     LeadOutMsf: %02u:%02u:%02u\n"
		, pAtip->LeadOutMsf[0], pAtip->LeadOutMsf[1], pAtip->LeadOutMsf[2]
	);
	OutputCDAtipAI1(pAtip);
	if (pAtip->A2Valid) {
		OutputDiscLog(
			"\tAdditional Information 2\n"
			"\t\t      A2Values: %02u:%02u:%02u\n"
			, pAtip->A2Values[0], pAtip->A2Values[1], pAtip->A2Values[2]
		);
		if (pAtip->IsCdrw) {
			OutputDiscLog(
				"\t\t        => Lowest e2Tf Write-strategy Test Speed: %d "
				, pAtip->A2Values[0] >> 4 & 0x07
			);
			switch (pAtip->A2Values[0] >> 4 & 0x07) {
			case 2:
				OutputDiscLog("(4x nominal CD Speed)\n");
				break;
			case 3:
				OutputDiscLog("(8x nominal CD Speed)\n");
				break;
			default:
				OutputDiscLog("(Unknown CD Speed)\n");
				break;
			}

			OutputDiscLog(
				"\t\t        => Highest e2Tf Write-strategy Test Speed: %d "
				, pAtip->A2Values[0] & 0x0f
			);
			switch (pAtip->A2Values[0] & 0x0f) {
			case 6:
				OutputDiscLog("(10x nominal CD Speed)\n");
				break;
			case 8:
				OutputDiscLog("(24x nominal CD Speed)\n");
				break;
			case 9:
				OutputDiscLog("(32x nominal CD Speed)\n");
				break;
			default:
				OutputDiscLog("(Unknown CD Speed)\n");
				break;
			}

			OutputDiscLog(
				"\t\t        => Optimum write power indication at 16x and e2Tf Write-strategy: %d "
				, pAtip->A2Values[1] >> 4 & 0x07
			);
			switch (pAtip->A2Values[1] >> 4 & 0x07) {
			case 0:
				OutputDiscLog("(not specified)\n");
				break;
			case 1:
				OutputDiscLog("(30 mW)\n");
				break;
			case 2:
				OutputDiscLog("(32.5 mW)\n");
				break;
			case 3:
				OutputDiscLog("(35 mW)\n");
				break;
			case 4:
				OutputDiscLog("(38 mW)\n");
				break;
			case 5:
				OutputDiscLog("(41 mW)\n");
				break;
			case 6:
				OutputDiscLog("(45 mW)\n");
				break;
			case 7:
				OutputDiscLog("(50 mW)\n");
				break;
			default:
				OutputDiscLog("(Unknown)\n");
				break;
			}

			OutputDiscLog(
				"\t\t        => Optimum write power indication at HTS and e2Tf Write-strategy: %d "
				, pAtip->A2Values[1] >> 1 & 0x07
			);
			switch (pAtip->A2Values[1] >> 1 & 0x07) {
			case 0:
				OutputDiscLog("(not specified)\n");
				break;
			case 1:
				OutputDiscLog("(30 mW)\n");
				break;
			case 2:
				OutputDiscLog("(32.5 mW)\n");
				break;
			case 3:
				OutputDiscLog("(35 mW)\n");
				break;
			case 4:
				OutputDiscLog("(38 mW)\n");
				break;
			case 5:
				OutputDiscLog("(41 mW)\n");
				break;
			case 6:
				OutputDiscLog("(45 mW)\n");
				break;
			case 7:
				OutputDiscLog("(50 mW)\n");
				break;
			default:
				OutputDiscLog("(Unknown)\n");
				break;
			}

			OutputDiscLog(
				"\t\t        => Optimum erase power indication at 16x and e2Tf Write-strategy: %d "
				, pAtip->A2Values[2] >> 4 & 0x07
			);
			switch (pAtip->A2Values[2] >> 4 & 0x07) {
			case 0:
				OutputDiscLog("(not specified)\n");
				break;
			case 1:
				OutputDiscLog("(6 mW)\n");
				break;
			case 2:
				OutputDiscLog("(7 mW)\n");
				break;
			case 3:
				OutputDiscLog("(8 mW)\n");
				break;
			case 4:
				OutputDiscLog("(9.5 mW)\n");
				break;
			case 5:
				OutputDiscLog("(11 mW)\n");
				break;
			case 6:
				OutputDiscLog("(13 mW)\n");
				break;
			case 7:
				OutputDiscLog("(15 mW)\n");
				break;
			default:
				OutputDiscLog("(Unknown)\n");
				break;
			}

			OutputDiscLog(
				"\t\t        => Optimum erase power indication at HTS and e2Tf Write-strategy: %d "
				, pAtip->A2Values[2] >> 1 & 0x07
			);
			switch (pAtip->A2Values[2] >> 1 & 0x07) {
			case 0:
				OutputDiscLog("(not specified)\n");
				break;
			case 1:
				OutputDiscLog("(6 mW)\n");
				break;
			case 2:
				OutputDiscLog("(7 mW)\n");
				break;
			case 3:
				OutputDiscLog("(8 mW)\n");
				break;
			case 4:
				OutputDiscLog("(9.5 mW)\n");
				break;
			case 5:
				OutputDiscLog("(11 mW)\n");
				break;
			case 6:
				OutputDiscLog("(13 mW)\n");
				break;
			case 7:
				OutputDiscLog("(15 mW)\n");
				break;
			default:
				OutputDiscLog("(Unknown)\n");
				break;
			}
		}
		else {
			OutputDiscLog(
				"\t\t        => Indicative Optimum Writing Power at Lowest Test Speed: %d "
				, pAtip->A2Values[0] >> 4 & 0x07
			);
			switch (pAtip->A2Values[0] >> 4 & 0x07) {
			case 0:
				OutputDiscLog("(7 mW)\n");
				break;
			case 1:
				OutputDiscLog("(8 mW)\n");
				break;
			case 2:
				OutputDiscLog("(9 mW)\n");
				break;
			case 3:
				OutputDiscLog("(10 mW)\n");
				break;
			case 4:
				OutputDiscLog("(11 mW)\n");
				break;
			case 5:
				OutputDiscLog("(12 mW)\n");
				break;
			case 6:
				OutputDiscLog("(13 mW)\n");
				break;
			case 7:
				OutputDiscLog("(14 mW)\n");
				break;
			default:
				OutputDiscLog("(Unknown)\n");
				break;
			}

			OutputDiscLog(
				"\t\t        => Indicative Optimum Writing Power at Highest Test Speed: %d "
				, pAtip->A2Values[0] & 0x0f
			);
			switch (pAtip->A2Values[0] & 0x0f) {
			case 0:
				OutputDiscLog("(16 mW)\n");
				break;
			case 1:
				OutputDiscLog("(18 mW)\n");
				break;
			case 2:
				OutputDiscLog("(20 mW)\n");
				break;
			case 3:
				OutputDiscLog("(22 mW)\n");
				break;
			case 4:
				OutputDiscLog("(24 mW)\n");
				break;
			case 5:
				OutputDiscLog("(26 mW)\n");
				break;
			case 6:
				OutputDiscLog("(28 mW)\n");
				break;
			case 7:
				OutputDiscLog("(30 mW)\n");
				break;
			case 8:
				OutputDiscLog("(32 mW)\n");
				break;
			case 9:
				OutputDiscLog("(34 mW)\n");
				break;
			case 10:
				OutputDiscLog("(36 mW)\n");
				break;
			case 11:
				OutputDiscLog("(38 mW)\n");
				break;
			case 12:
				OutputDiscLog("(41 mW)\n");
				break;
			case 13:
				OutputDiscLog("(44 mW)\n");
				break;
			case 14:
				OutputDiscLog("(47 mW)\n");
				break;
			case 15:
				OutputDiscLog("(50 mW)\n");
				break;
			default:
				OutputDiscLog("(Unknown)\n");
				break;
			}

			OutputDiscLog(
				"\t\t        => Power Boost for I3 pits: %d "
				, pAtip->A2Values[1] >> 4 & 0x07
			);
			switch (pAtip->A2Values[1] >> 4 & 0x07) {
			case 0:
				OutputDiscLog("(0%%)\n");
				break;
			case 1:
				OutputDiscLog("(2%%)\n");
				break;
			case 2:
				OutputDiscLog("(4%%)\n");
				break;
			case 3:
				OutputDiscLog("(6%%)\n");
				break;
			case 4:
				OutputDiscLog("(8%%)\n");
				break;
			case 5:
				OutputDiscLog("(10%%)\n");
				break;
			case 6:
				OutputDiscLog("(12%%)\n");
				break;
			case 7:
				OutputDiscLog("(14%%)\n");
				break;
			default:
				OutputDiscLog("(Unknown)\n");
				break;
			}

			OutputDiscLog(
				"\t\t        => Pulse length correction after I3 lands: %d "
				, pAtip->A2Values[1] >> 2 & 0x03
			);
			switch (pAtip->A2Values[1] >> 2 & 0x03) {
			case 0:
				OutputDiscLog("(0T)\n");
				break;
			case 1:
				OutputDiscLog("(1/16T)\n");
				break;
			case 2:
				OutputDiscLog("(2/16T)\n");
				break;
			case 3:
				OutputDiscLog("(3/16T)\n");
				break;
			default:
				OutputDiscLog("(Unknown)\n");
				break;
			}
		}
	}
	if (pAtip->A3Valid) {
		OutputDiscLog(
			"\tAdditional Information 3\n"
			"\t\t       A3Values: %02u:%02u:%02u\n"
			, pAtip->A3Values[0], pAtip->A3Values[1], pAtip->A3Values[2]
		);
		OutputDiscLog(
			"\t\t        => Media technology type: %d "
			, pAtip->A3Values[0] >> 5 & 0x03
		);
		switch (pAtip->A3Values[0] >> 5 & 0x03) {
		case 0:
			if (pAtip->IsCdrw) {
				OutputDiscLog("(Phase-change type recording layer)\n");
			}
			else {
				OutputDiscLog("(Cyanine or comparable type recording layer)\n");
			}
			break;
		case 1:
			if (pAtip->IsCdrw) {
				OutputDiscLog("(Reserved)\n");
			}
			else {
				OutputDiscLog("(Phtalo-cyanine or comparable type recording layer)\n");
			}
			break;
		case 2:
			OutputDiscLog("(Reserved)\n");
			break;
		case 3:
			OutputDiscLog("(Other type of recording layer)\n");
			break;
		default:
			OutputDiscLog("(Unknown)\n");
			break;
		}

		WORD Mid1st = MAKEWORD(pAtip->A3Values[0] & 0x1f, pAtip->A3Values[1] & 0x7f);
		OutputDiscLog(
			"\t\t        => Media IDentification (MID) code first part: %d\n"
			"\t\t        => Media IDentification (MID) code second part: %d\n"
			"\t\t        => Product revision number: %d\n"
			, Mid1st, pAtip->A3Values[1] >> 3 & 0x0f, pAtip->A3Values[2] & 0x03
		);
	}
}

VOID OutputDiscInformation(
	PDISC_INFORMATION pDiscInformation
) {
	LPCTSTR lpDiscStatus[] = {
		_T("Empty"), _T("Incomplete"), _T("Complete"), _T("Others")
	};
	LPCTSTR lpStateOfLastSession[] = {
		_T("Empty"), _T("Incomplete"), _T("Reserved / Damaged"), _T("Complete")
	};
	LPCTSTR lpBGFormatStatus[] = {
		_T("None"), _T("Incomplete"), _T("Running"), _T("Complete")
	};
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DiscInformation")
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
		, BOOLEAN_TO_STRING_YES_NO(pDiscInformation->Erasable)
		, pDiscInformation->FirstTrackNumber
		, pDiscInformation->NumberOfSessionsLsb
		, pDiscInformation->LastSessionFirstTrackLsb
		, pDiscInformation->LastSessionLastTrackLsb
		, lpBGFormatStatus[pDiscInformation->MrwStatus]
		, BOOLEAN_TO_STRING_YES_NO(pDiscInformation->MrwDirtyBit)
		, BOOLEAN_TO_STRING_YES_NO(pDiscInformation->URU)
		, BOOLEAN_TO_STRING_YES_NO(pDiscInformation->DBC_V)
		, BOOLEAN_TO_STRING_YES_NO(pDiscInformation->DID_V));
	switch (pDiscInformation->DiscType) {
	case DISK_TYPE_CDDA:
		OutputDiscLog("CD-DA or CD-ROM Disc\n");
		break;
	case DISK_TYPE_CDI:
		OutputDiscLog("CD-I Disc\n");
		break;
	case DISK_TYPE_XA:
		OutputDiscLog("CD-ROM XA Disc\n");
		break;
	case DISK_TYPE_UNDEFINED:
		OutputDiscLog("Undefined\n");
		break;
	default:
		OutputDiscLog("Reserved\n");
		break;
	}
	if (pDiscInformation->DID_V) {
		OutputDiscLog(
			"\t          DiscIdentification: %u%u%u%u\n",
			pDiscInformation->DiskIdentification[0],
			pDiscInformation->DiskIdentification[1],
			pDiscInformation->DiskIdentification[2],
			pDiscInformation->DiskIdentification[3]);
	}
	OutputDiscLog(
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
		OutputDiscLog(
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
	OutputDiscLog(
		"\t            NumberOPCEntries: %u\n",
		pDiscInformation->NumberOPCEntries);
	if (pDiscInformation->NumberOPCEntries) {
		OutputDiscLog(
			"\t                    OPCTable\n");
	}
	for (INT i = 0; i < pDiscInformation->NumberOPCEntries; i++) {
		OutputDiscLog(
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
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("ModeParmeterHeader")
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
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("ModeParmeterHeader10")
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
	UINT perKb
) {
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("CDVD Capabilities & Mechanism Status Page")
		"\t                 PageCode: %#04hhx\n"
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
		, BOOLEAN_TO_STRING_YES_NO(cdvd->PSBit)
		, cdvd->PageLength
		, BOOLEAN_TO_STRING_YES_NO(cdvd->CDRRead)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->CDERead)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->Method2)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->DVDROMRead)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->DVDRRead)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->DVDRAMRead)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->CDRWrite)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->CDEWrite)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->TestWrite)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->DVDRWrite)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->DVDRAMWrite)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->AudioPlay)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->Composite)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->DigitalPortOne)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->DigitalPortTwo)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->Mode2Form1)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->Mode2Form2)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->MultiSession)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->BufferUnderrunFree)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->CDDA)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->CDDAAccurate)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->RWSupported)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->RWDeinterleaved)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->C2Pointers)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->ISRC)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->UPC)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->ReadBarCodeCapable)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->Lock)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->LockState)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->PreventJumper)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->Eject));

	switch (cdvd->LoadingMechanismType) {
	case LOADING_MECHANISM_CADDY:
		OutputDriveLog("caddy\n");
		break;
	case LOADING_MECHANISM_TRAY:
		OutputDriveLog("tray\n");
		break;
	case LOADING_MECHANISM_POPUP:
		OutputDriveLog("popup\n");
		break;
	case LOADING_MECHANISM_INDIVIDUAL_CHANGER:
		OutputDriveLog("individual changer\n");
		break;
	case LOADING_MECHANISM_CARTRIDGE_CHANGER:
		OutputDriveLog("cartridge changer\n");
		break;
	default:
		OutputDriveLog("unknown\n");
		break;
	}
	WORD rsm = MAKEWORD(cdvd->ReadSpeedMaximum[1], cdvd->ReadSpeedMaximum[0]);
	WORD rsc = MAKEWORD(cdvd->ReadSpeedCurrent[1], cdvd->ReadSpeedCurrent[0]);
	WORD wsm = MAKEWORD(cdvd->WriteSpeedMaximum[1], cdvd->WriteSpeedMaximum[0]);
	WORD wsc = MAKEWORD(cdvd->WriteSpeedCurrent[1], cdvd->WriteSpeedCurrent[0]);
	WORD bs = MAKEWORD(cdvd->BufferSize[1], cdvd->BufferSize[0]);
	OutputDriveLog(
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
		"\t                   Length: %hhu\n"
		"\t        WriteSpeedMaximum: %uKB/sec (%ux)\n"
		"\t        WriteSpeedCurrent: %uKB/sec (%ux)\n"
		"\t   CopyManagementRevision: %u\n"
		, BOOLEAN_TO_STRING_YES_NO(cdvd->SeparateVolume)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->SeperateChannelMute)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->SupportsDiskPresent)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->SWSlotSelection)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->SideChangeCapable)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->RWInLeadInReadable)
		, rsm, rsm / perKb
		, MAKEWORD(cdvd->NumberVolumeLevels[1],	cdvd->NumberVolumeLevels[0])
		, bs
		, rsc, rsc / perKb
		, BOOLEAN_TO_STRING_YES_NO(cdvd->BCK)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->RCK)
		, BOOLEAN_TO_STRING_YES_NO(cdvd->LSBF)
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
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("CDVD_INACTIVITY_TIMEOUT_PAGE")
		"\t                 PageCode: %#04hhx\n"
		"\t                    PSBit: %s\n"
		"\t               PageLength: %u\n"
		"\t                     SWPP: %s\n"
		"\t                     DISP: %s\n"
		"\t   GroupOneMinimumTimeout: %u\n"
		"\t   GroupTwoMinimumTimeout: %u\n"
		, inactivity->PageCode
		, BOOLEAN_TO_STRING_YES_NO(inactivity->PSBit)
		, inactivity->PageLength
		, BOOLEAN_TO_STRING_YES_NO(inactivity->SWPP)
		, BOOLEAN_TO_STRING_YES_NO(inactivity->DISP)
		, MAKEWORD(inactivity->GroupOneMinimumTimeout[1], inactivity->GroupOneMinimumTimeout[0])
		, MAKEWORD(inactivity->GroupTwoMinimumTimeout[1], inactivity->GroupTwoMinimumTimeout[0])
	);
}

VOID OutputPagePowerCondition(
	LPBYTE modesense,
	size_t pcOfs
) {
	PPOWER_CONDITION_PAGE power = (PPOWER_CONDITION_PAGE)(modesense + pcOfs);
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("POWER_CONDITION_PAGE")
		"\t                 PageCode: %#04hhx\n"
		"\t                    PSBit: %s\n"
		"\t               PageLength: %u\n"
		"\t                  Standby: %s\n"
		"\t                     Idle: %s\n"
		"\t                IdleTimer: %u\n"
		"\t             StandbyTimer: %u\n"
		, power->PageCode
		, BOOLEAN_TO_STRING_YES_NO(power->PSBit)
		, power->PageLength
		, BOOLEAN_TO_STRING_YES_NO(power->Standby)
		, BOOLEAN_TO_STRING_YES_NO(power->Idle)
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
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("CDVD_FEATURE_SET_PAGE")
		"\t                 PageCode: %#04hhx\n"
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
		, BOOLEAN_TO_STRING_YES_NO(feature->PSBit)
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
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("CDAUDIO_CONTROL_PAGE")
		"\t                 PageCode: %#04hhx\n"
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
		, BOOLEAN_TO_STRING_YES_NO(ctrl->PSBit)
		, ctrl->PageLength
		, BOOLEAN_TO_STRING_YES_NO(ctrl->StopOnTrackCrossing)
		, BOOLEAN_TO_STRING_YES_NO(ctrl->Immediate)
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
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("CD_DEVICE_PARAMETERS_PAGE")
		"\t                 PageCode: %#04hhx\n"
		"\t                    PSBit: %s\n"
		"\t               PageLength: %u\n"
		"\tInactivityTimerMultiplier: %u\n"
		"\t TheNumberOfSUnitPerMUnit: %d\n"
		"\t TheNumberOfFUnitPerSUnit: %d\n"
		, buf[0] & 0x3f
		, BOOLEAN_TO_STRING_YES_NO((buf[0] >> 7) & 0x01)
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
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("MODE_CACHING_PAGE")
		"\t                 PageCode: %#04hhx\n"
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
		, BOOLEAN_TO_STRING_YES_NO(caching->PageSavable)
		, caching->PageLength
		, BOOLEAN_TO_STRING_YES_NO(caching->ReadDisableCache)
		, BOOLEAN_TO_STRING_YES_NO(caching->MultiplicationFactor)
		, BOOLEAN_TO_STRING_YES_NO(caching->WriteCacheEnable)
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
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("MODE_CDROM_WRITE_PARAMETERS_PAGE")
		"\t                 PageCode: %#04hhx\n"
		"\t              PageSavable: %s\n"
		"\t               PageLength: %u\n"
		"\t                WriteType: %d\n"
		"\t                TestWrite: %s\n"
		"\t            LinkSizeValid: %s\n"
		"\tBufferUnderrunFreeEnabled: %s\n"
		"\t                TrackMode: %hhu\n"
		"\t                     Copy: %s\n"
		"\t              FixedPacket: %s\n"
		"\t             MultiSession: %hhu\n"
		"\t            DataBlockType: %hhu\n"
		"\t                 LinkSize: %u\n"
		"\t      HostApplicationCode: %hhu\n"
		"\t            SessionFormat: %u\n"
		"\t               PacketSize: %u\n"
		"\t         AudioPauseLength: %u\n"
		"\t       MediaCatalogNumber: %16" CHARWIDTH "s\n"
		"\t                     ISRC: %16" CHARWIDTH "s\n"
		"\t            SubHeaderData: %u\n"
		, wparam->PageCode
		, BOOLEAN_TO_STRING_YES_NO(wparam->PageSavable)
		, wparam->PageLength
		, wparam->WriteType
		, BOOLEAN_TO_STRING_YES_NO(wparam->TestWrite)
		, BOOLEAN_TO_STRING_YES_NO(wparam->LinkSizeValid)
		, BOOLEAN_TO_STRING_YES_NO(wparam->BufferUnderrunFreeEnabled)
		, wparam->TrackMode
		, BOOLEAN_TO_STRING_YES_NO(wparam->Copy)
		, BOOLEAN_TO_STRING_YES_NO(wparam->FixedPacket)
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
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("MODE_RIGID_GEOMETRY_PAGE")
		"\t                PageCode: %#04hhx\n"
		"\t             PageSavable: %s\n"
		"\t              PageLength: %u\n"
		"\t       NumberOfCylinders: %u\n"
		"\t           NumberOfHeads: %u\n"
		"\t        StartWritePrecom: %u\n"
		"\t     StartReducedCurrent: %u\n"
		"\t           DriveStepRate: %u\n"
		"\t       LandZoneCyclinder: %u\n"
		"\t  RotationalPositionLock: %s\n"
		"\t          RotationOffset: %u\n"
		"\t            RoataionRate: %u\n"
		, geom->PageCode
		, BOOLEAN_TO_STRING_YES_NO(geom->PageSavable)
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
		, BOOLEAN_TO_STRING_YES_NO(geom->RotationalPositionLock)
		, geom->RotationOffset
		, MAKEWORD(geom->RoataionRate[1], geom->RoataionRate[0])
	);
}

VOID OutputPageMrw(
	LPBYTE modesense,
	size_t pcOfs
) {
	PMODE_MRW_PAGE mrw = (PMODE_MRW_PAGE)(modesense + pcOfs);
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("MODE_MRW_PAGE")
		"\t                PageCode: %#04hhx\n"
		"\t             PageSavable: %s\n"
		"\t              PageLength: %u\n"
		"\t                LbaSpace: %s\n"
		, mrw->PageCode
		, BOOLEAN_TO_STRING_YES_NO(mrw->PageSavable)
		, mrw->PageLength
		, BOOLEAN_TO_STRING_YES_NO(mrw->LbaSpace)
	);
}

VOID OutputPageDisconnect(
	LPBYTE modesense,
	size_t pcOfs
) {
	PMODE_DISCONNECT_PAGE dis = (PMODE_DISCONNECT_PAGE)(modesense + pcOfs);
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("MODE_DISCONNECT_PAGE")
		"\t                 PageCode: %#04hhx\n"
		"\t              PageSavable: %s\n"
		"\t               PageLength: %u\n"
		"\t          BufferFullRatio: %u\n"
		"\t         BufferEmptyRatio: %u\n"
		"\t       BusInactivityLimit: %u\n"
		"\t        BusDisconnectTime: %u\n"
		"\t           BusConnectTime: %u\n"
		"\t         MaximumBurstSize: %u\n"
		"\t   DataTransferDisconnect: %hhu\n"
		, dis->PageCode
		, BOOLEAN_TO_STRING_YES_NO(dis->PageSavable)
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
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("MODE_READ_WRITE_RECOVERY_PAGE")
		"\t                 PageCode: %#04hhx\n"
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
		, BOOLEAN_TO_STRING_YES_NO(rec->PSBit)
		, rec->PageLength
		, BOOLEAN_TO_STRING_YES_NO(rec->DCRBit)
		, BOOLEAN_TO_STRING_YES_NO(rec->PERBit)
		, BOOLEAN_TO_STRING_YES_NO(rec->EERBit)
		, BOOLEAN_TO_STRING_YES_NO(rec->RCBit)
		, BOOLEAN_TO_STRING_YES_NO(rec->TBBit)
		, BOOLEAN_TO_STRING_YES_NO(rec->ARRE)
		, BOOLEAN_TO_STRING_YES_NO(rec->AWRE)
		, rec->ReadRetryCount
		, rec->WriteRetryCount
	);
}

VOID OutputPageUnknown(
	LPBYTE modesense,
	size_t pcOfs,
	LPCTSTR str
) {
	UINT len = *(modesense + pcOfs + 1);
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("%s")
		"\t                 PageCode: %#04hhx\n"
		"\t                    PSBit: %s\n"
		"\t               PageLength: %u\n"
		"\t                  Unknown: "
		, str
		, *(modesense + pcOfs) & 0x3f
		, BOOLEAN_TO_STRING_YES_NO((*(modesense + pcOfs) >> 7) & 0x01)
		, len
	);
	for (UINT t = 0; t < len; t++) {
		OutputDriveLog("%02x", *(modesense + pcOfs + t));
	}
	OutputDriveLog("\n");
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
			OutputPageUnknown(modesense, pcOfs, _T("VENDOR_SPECIFIC"));
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
			OutputPageUnknown(modesense, pcOfs, _T("VERIFY_ERROR"));
		}
		else if (pagecode == MODE_PAGE_CACHING) {
			OutputPageWriteCaching(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_PERIPHERAL) {
			OutputPageUnknown(modesense, pcOfs, _T("PERIPHERAL"));
		}
		else if (pagecode == MODE_PAGE_CONTROL) {
			OutputPageUnknown(modesense, pcOfs, _T("CONTROL"));
		}
		else if (pagecode == MODE_PAGE_MEDIUM_TYPES) {
			OutputPageUnknown(modesense, pcOfs, _T("MEDIUM_TYPES"));
		}
		else if (pagecode == MODE_PAGE_NOTCH_PARTITION) {
			OutputPageUnknown(modesense, pcOfs, _T("NOTCH_PARTITION"));
		}
		else if (pagecode == 0x0d) {
			OutputPageCdDeviceParameters(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_CD_AUDIO_CONTROL) {
			OutputPageCdAudioControl(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_DATA_COMPRESS) {
			OutputPageUnknown(modesense, pcOfs, _T("DATA_COMPRESS"));
		}
		else if (pagecode == MODE_PAGE_DEVICE_CONFIG) {
			OutputPageUnknown(modesense, pcOfs, _T("DEVICE_CONFIG"));
		}
		else if (pagecode == MODE_PAGE_MEDIUM_PARTITION) {
			OutputPageUnknown(modesense, pcOfs, _T("MEDIUM_PARTITION"));
		}
		else if (pagecode == MODE_PAGE_ENCLOSURE_SERVICES_MANAGEMENT) {
			OutputPageUnknown(modesense, pcOfs, _T("ENCLOSURE_SERVICES_MANAGEMENT"));
		}
		else if (pagecode == MODE_PAGE_EXTENDED) {
			OutputPageUnknown(modesense, pcOfs, _T("EXTENDED"));
		}
		else if (pagecode == MODE_PAGE_EXTENDED_DEVICE_SPECIFIC) {
			OutputPageUnknown(modesense, pcOfs, _T("EXTENDED_DEVICE_SPECIFIC"));
		}
		else if (pagecode == MODE_PAGE_CDVD_FEATURE_SET) {
			OutputPageCDvdFeatureSet(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_PROTOCOL_SPECIFIC_PORT) {
			OutputPageUnknown(modesense, pcOfs, _T("PROTOCOL_SPECIFIC_PORT"));
		}
		else if (pagecode == MODE_PAGE_POWER_CONDITION) {
			OutputPagePowerCondition(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_LUN_MAPPING) {
			OutputPageUnknown(modesense, pcOfs, _T("LUN_MAPPING"));
		}
		else if (pagecode == MODE_PAGE_FAULT_REPORTING) {
			OutputPageUnknown(modesense, pcOfs, _T("FAULT_REPORTING"));
		}
		else if (pagecode == MODE_PAGE_CDVD_INACTIVITY) {
			OutputPageCDvdInactivity(modesense, pcOfs);
		}
		else if (pagecode == MODE_PAGE_ELEMENT_ADDRESS) {
			OutputPageUnknown(modesense, pcOfs, _T("ELEMENT_ADDRESS"));
		}
		else if (pagecode == MODE_PAGE_TRANSPORT_GEOMETRY) {
			OutputPageUnknown(modesense, pcOfs, _T("TRANSPORT_GEOMETRY"));
		}
		else if (pagecode == MODE_PAGE_DEVICE_CAPABILITIES) {
			OutputPageUnknown(modesense, pcOfs, _T("DEVICE_CAPABILITIES"));
		}
		else if (pagecode == MODE_PAGE_CAPABILITIES) {
			PCDVD_CAPABILITIES_PAGE capabilities = (PCDVD_CAPABILITIES_PAGE)(modesense + pcOfs);
			WORD rsm = MAKEWORD(capabilities->ReadSpeedMaximum[1],
				capabilities->ReadSpeedMaximum[0]);
			UINT perKb = 176;
			if (IsDVDorRelatedDisc(pDisc)) {
				perKb = 1385;
			}
			else if (IsBDorRelatedDisc(pDisc)) {
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
			OutputPageUnknown(modesense, pcOfs, _T("Unknown"));
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
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("ReadBufferCapacity")
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
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("SetSpeed")
		"\t    RequestType: %s\n"
		"\t      ReadSpeed: %uKB/sec\n"
		"\t     WriteSpeed: %uKB/sec\n"
		"\tRotationControl: %s\n",
		pSetspeed->RequestType == 0 ? _T("CdromSetSpeed") : _T("CdromSetStreaming"),
		pSetspeed->ReadSpeed,
		pSetspeed->WriteSpeed,
		pSetspeed->RotationControl == 0 ? _T("CdromDefaultRotation") : _T("CdromCAVRotation"));
}

VOID OutputEepromUnknownByte(
	LPBYTE pBuf,
	UINT startIdx,
	UINT endIdx
) {
	if (startIdx <= endIdx) {
		OutputDriveLog("\t   Unknown[%03u]: ", startIdx);
		for (UINT i = startIdx; i <= endIdx; i++) {
			OutputDriveLog("%02x ", pBuf[i]);
		}
		OutputDriveLog("\n");
	}
}

VOID OutputEepromOverPX712(
	PDEVICE pDevice,
	LPBYTE pBuf
) {
	OutputDriveLog("\t    Silent Mode: ");
	if (pBuf[0] == 1) {
		OutputDriveLog(
			"Enabled\n"
			"\t\t       Access Time: ");
		if (pBuf[1] == 0) {
			OutputDriveLog("Fast\n");
		}
		else if (pBuf[1] == 1) {
			OutputDriveLog("Slow\n");
		}
		OutputDriveLog(
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
		OutputDriveLog("Disable\n");
	}
	
	UINT tmp = 9;
	OutputDriveLog("\t        SecuRec: ");
	while (tmp < 29) {
		OutputDriveLog("%02x ", pBuf[tmp]);
		tmp += 1;
	}

	OutputDriveLog("\n\t      SpeedRead: ");
	if (pBuf[29] == 0xf0 || pBuf[29] == 0) {
		OutputDriveLog("Enable");
	}
	else if (pBuf[29] == 0xff || pBuf[29] == 0x0f) {
		OutputDriveLog("Disable");
		pDevice->byDisabldSpeedRead = TRUE;
	}
	OutputDriveLog("\n\t        Unknown: %x\n", pBuf[30]);

	OutputDriveLog("\t  Spindown Time: ");
	switch (pBuf[31]) {
	case 0:
		OutputDriveLog("Infinite\n");
		break;
	case 1:
		OutputDriveLog("125 ms\n");
		break;
	case 2:
		OutputDriveLog("250 ms\n");
		break;
	case 3:
		OutputDriveLog("500 ms\n");
		break;
	case 4:
		OutputDriveLog("1 second\n");
		break;
	case 5:
		OutputDriveLog("2 seconds\n");
		break;
	case 6:
		OutputDriveLog("4 seconds\n");
		break;
	case 7:
		OutputDriveLog("8 seconds\n");
		break;
	case 8:
		OutputDriveLog("16 seconds\n");
		break;
	case 9:
		OutputDriveLog("32 seconds\n");
		break;
	case 10:
		OutputDriveLog("1 minute\n");
		break;
	case 11:
		OutputDriveLog("2 minutes\n");
		break;
	case 12:
		OutputDriveLog("4 minutes\n");
		break;
	case 13:
		OutputDriveLog("8 minutes\n");
		break;
	case 14:
		OutputDriveLog("16 minutes\n");
		break;
	case 15:
		OutputDriveLog("32 minutes\n");
		break;
	default:
		OutputDriveLog("Unset\n");
		break;
	}

	UINT ucr =
		MAKEUINT(MAKEWORD(pBuf[37], pBuf[36]), MAKEWORD(pBuf[35], pBuf[34]));
	UINT ucw =
		MAKEUINT(MAKEWORD(pBuf[41], pBuf[40]), MAKEWORD(pBuf[39], pBuf[38]));
	UINT udr =
		MAKEUINT(MAKEWORD(pBuf[45], pBuf[44]), MAKEWORD(pBuf[43], pBuf[42]));
	UINT udw = 
		MAKEUINT(MAKEWORD(pBuf[49], pBuf[48]), MAKEWORD(pBuf[47], pBuf[46]));
	OutputDriveLog(
		"\tDisc load count: %u\n"
		"\t   CD read time: %02u:%02u:%02u\n"
		"\t  CD write time: %02u:%02u:%02u\n"
		"\t  DVD read time: %02u:%02u:%02u\n"
		"\t DVD write time: %02u:%02u:%02u\n"
		, MAKEWORD(pBuf[33], pBuf[32])
		, ucr / 3600, ucr / 60 % 60, ucr % 60
		, ucw / 3600, ucw / 60 % 60, ucw % 60
		, udr / 3600, udr / 60 % 60, udr % 60
		, udw / 3600, udw / 60 % 60, udw % 60);

	OutputEepromUnknownByte(pBuf, 50, 114);

	OutputDriveLog("\tChange BookType: ");
	switch (pBuf[115]) {
	case 0xfc:
		OutputDriveLog("for DVD+R, DVD+R DL\n");
		break;
	case 0xfd:
		OutputDriveLog("for DVD+R\n");
		break;
	case 0xfe:
		OutputDriveLog("for DVD+R DL\n");
		break;
	case 0xff:
		OutputDriveLog("Disable\n");
		break;
	default:
		OutputDriveLog("Unknown[%02x]\n", pBuf[115]);
		break;
	}
}

VOID OutputEeprom(
	PDEVICE pDevice,
	LPBYTE pBuf,
	INT nRoop,
	BOOL byPlxtrDrive
) {
	if (nRoop == 0) {
		OutputDriveLog(
			"\t      Signature: %02x %02x\n"
			"\t       VendorId: %.8" CHARWIDTH "s\n"
			"\t      ProductId: %.16" CHARWIDTH "s\n"
			"\t   SerialNumber: %06lu\n"
			, pBuf[0], pBuf[1]
			, &pBuf[2]
			, &pBuf[10]
			, strtoul((LPCCH)&pBuf[26], NULL, 16));
		OutputEepromUnknownByte(pBuf, 31, 40);

		switch (byPlxtrDrive) {
		case PLXTR_DRIVE_TYPE::PX760A:
		case PLXTR_DRIVE_TYPE::PX755A:
		case PLXTR_DRIVE_TYPE::PX716AL:
		case PLXTR_DRIVE_TYPE::PX716A:
		case PLXTR_DRIVE_TYPE::PX714A:
		case PLXTR_DRIVE_TYPE::PX712A:
			OutputDriveLog("\t            TLA: %.4" CHARWIDTH "s\n", &pBuf[41]);
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
			OutputEepromOverPX712(pDevice, &pBuf[256]);
			OutputEepromUnknownByte(pBuf, 372, 510);
			OutputDriveLog(
				"\t            Sum: %02x (SpeedRead: %02x + Spindown Time: %02x + BookType: %02x + Others)\n"
				, pBuf[511], pBuf[285], pBuf[287], pBuf[371]);
			break;
		case PLXTR_DRIVE_TYPE::PX708A2:
		case PLXTR_DRIVE_TYPE::PX708A:
		case PLXTR_DRIVE_TYPE::PX704A:
		{
			OutputEepromUnknownByte(pBuf, 108, 114);
			UINT ucr = MAKEUINT(MAKEWORD(pBuf[120], pBuf[119]), MAKEWORD(pBuf[118], pBuf[117]));
			UINT ucw = MAKEUINT(MAKEWORD(pBuf[125], pBuf[124]), MAKEWORD(pBuf[123], pBuf[122]));
			OutputDriveLog(
				"\tDisc load count: %u\n"
				"\t   CD read time: %02u:%02u:%02u\n"
				"\t        Unknown: %02x\n"
				"\t  CD write time: %02u:%02u:%02u\n"
				, MAKEWORD(pBuf[116], pBuf[115])
				, ucr / 3600, ucr / 60 % 60, ucr % 60
				, pBuf[121]
				, ucw / 3600, ucw / 60 % 60, ucw % 60);
			OutputEepromUnknownByte(pBuf, 126, 211);
			UINT udr =
				MAKEUINT(MAKEWORD(pBuf[215], pBuf[214]), MAKEWORD(pBuf[213], pBuf[212]));
			UINT udw =
				MAKEUINT(MAKEWORD(pBuf[219], pBuf[218]), MAKEWORD(pBuf[217], pBuf[216]));
			OutputDriveLog(
				"\t  DVD read time: %02u:%02u:%02u\n"
				"\t DVD write time: %02u:%02u:%02u\n"
				, udr / 3600, udr / 60 % 60, udr % 60
				, udw / 3600, udw / 60 % 60, udw % 60);
			OutputEepromUnknownByte(pBuf, 220, 255);
			break;
		}
		case PLXTR_DRIVE_TYPE::PX320A:
		{
			OutputEepromUnknownByte(pBuf, 108, 123);
			UINT ucr = MAKEUINT(MAKEWORD(pBuf[127], pBuf[126]), MAKEWORD(pBuf[125], pBuf[124]));
			OutputDriveLog(
				"\t   CD read time: %02u:%02u:%02u\n"
				, ucr / 3600, ucr / 60 % 60, ucr % 60);
			OutputEepromUnknownByte(pBuf, 128, 187);
			UINT udr =
				MAKEUINT(MAKEWORD(pBuf[191], pBuf[190]), MAKEWORD(pBuf[189], pBuf[188]));
			OutputDriveLog(
				"\t  DVD read time: %02u:%02u:%02u\n"
				, udr / 3600, udr / 60 % 60, udr % 60);
			OutputEepromUnknownByte(pBuf, 192, 226);
			OutputDriveLog(
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
			UINT ucr = MAKEUINT(MAKEWORD(pBuf[111], pBuf[110]), MAKEWORD(pBuf[109], pBuf[108]));
			UINT ucw = MAKEUINT(MAKEWORD(pBuf[125], pBuf[124]), MAKEWORD(pBuf[123], pBuf[122]));
			OutputDriveLog(
				"\t   CD read time: %02u:%02u:%02u\n"
				"\t        Unknown: %02x %02x %02x %02x %02x %02x %02x %02x\n"
				"\tDisc load count: %u\n"
				"\t  CD write time: %02u:%02u:%02u\n"
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
			UINT ucw = MAKEUINT(MAKEWORD(pBuf[125], pBuf[124]), MAKEWORD(pBuf[123], pBuf[122]));
			OutputDriveLog(
				"\tDisc load count: %u\n"
				"\t  CD write time: %02u:%02u:%02u\n"
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
			UINT ucw = MAKEUINT(MAKEWORD(pBuf[125], pBuf[124]), MAKEWORD(pBuf[123], pBuf[122]));
			OutputDriveLog(
				"\t  CD write time: %02u:%02u:%02u\n"
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
		OutputEepromOverPX712(pDevice, pBuf);
		
		OutputDriveLog("\t  Auto Strategy: ");
		switch (pBuf[116]) {
		case 0x06:
			if (PLXTR_DRIVE_TYPE::PX716AL <= byPlxtrDrive && byPlxtrDrive <= PLXTR_DRIVE_TYPE::PX714A) {
				OutputDriveLog("AS OFF\n");
			}
			else {
				OutputDriveLog("Unknown[%02x]\n", pBuf[116]);
			}
			break;
		case 0x07:
			if (PLXTR_DRIVE_TYPE::PX716AL <= byPlxtrDrive && byPlxtrDrive <= PLXTR_DRIVE_TYPE::PX714A) {
				OutputDriveLog("Auto Selection\n");
			}
			else {
				OutputDriveLog("AS ON\n");
			}
			break;
		case 0x0b:
			OutputDriveLog("AS ON(Forced)\n");
			break;
		case 0x0e:
			if (byPlxtrDrive <= PLXTR_DRIVE_TYPE::PX755A) {
				OutputDriveLog("AS OFF\n");
			}
			else {
				OutputDriveLog("Unknown[%02x]\n", pBuf[116]);
			}
			break;
		case 0x0f:
			if (byPlxtrDrive <= PLXTR_DRIVE_TYPE::PX755A) {
				OutputDriveLog("Auto Selection\n");
			}
			else {
				OutputDriveLog("Unknown[%02x]\n", pBuf[116]);
			}
			break;
		default:
			OutputDriveLog("Unknown[%02x]\n", pBuf[116]);
			break;
		}
		OutputEepromUnknownByte(pBuf, 117, 254);
		OutputDriveLog(
			"\t            Sum: %02x (SpeedRead: %02x + Spindown Time: %02x + BookType: %02x + Auto Strategy: %02x + Others)\n"
			, pBuf[255], pBuf[29], pBuf[31], pBuf[115], pBuf[116]);
	}
	else {
		OutputEepromUnknownByte(pBuf, 0, 255);
	}
}
