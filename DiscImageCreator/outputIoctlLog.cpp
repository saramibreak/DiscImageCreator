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
#include "get.h"
#include "output.h"
#include "outputIoctlLog.h"

VOID OutputAdditionalSenseCodeQualifierOther(
	BYTE byCode
) {
	if (byCode >= SCSI_ADSENSE_VENDOR_UNIQUE) {
		OutputLog(standardError | fileMainError, _T("VENDOR UNIQUE ERROR"));
	}
	else {
		OutputLog(standardError | fileMainError, _T("OTHER"));
	}
}

VOID OutputAdditionalSenseCodeQualifier0x00(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("NO ADDITIONAL SENSE INFORMATION"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("I/O PROCESS TERMINATED"));
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, _T("AUDIO PLAY OPERATION IN PROGRES"));
		break;
	case 0x12:
		OutputLog(standardError | fileMainError, _T("AUDIO PLAY OPERATION PAUSED"));
		break;
	case 0x13:
		OutputLog(standardError | fileMainError, _T("AUDIO PLAY OPERATION SUCCESSFULLY COMPLETED"));
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, _T("AUDIO PLAY OPERATION STOPPED DUE TO ERROR"));
		break;
	case 0x15:
		OutputLog(standardError | fileMainError, _T("NO CURRENT AUDIO STATUS TO RETURN"));
		break;
	case 0x16:
		OutputLog(standardError | fileMainError, _T("OPERATION IN PROGRESS"));
		break;
	case 0x17:
		OutputLog(standardError | fileMainError, _T("CLEANING REQUESTED"));
		break;
	case 0x1d:
		OutputLog(standardError | fileMainError, _T("ATA PASS THROUGH INFORMATION AVAILABLE"));
		break;
	case 0x1e:
		OutputLog(standardError | fileMainError, _T("CONFLICTING SA CREATION REQUE"));
		break;
	case 0x1f:
		OutputLog(standardError | fileMainError, _T("LOGICAL UNIT TRANSITIONING TO ANOTHER POWER CONDITION"));
		break;
	case 0x20:
		OutputLog(standardError | fileMainError, _T("EXTENDED COPY INFORMATION AVAILABLE"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x01(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("NO INDEX/SECTOR SIGNAL"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x02(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("NO SEEK COMPLETE"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x03(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("PERIPHERAL DEVICE WRITE FAULT"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x04(
	BYTE byAscq
) {
	switch (byAscq) {
	case SCSI_SENSEQ_CAUSE_NOT_REPORTABLE:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - CAUSE_NOT_REPORTABLE"));
		break;
	case SCSI_SENSEQ_BECOMING_READY:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - BECOMING_READY"));
		break;
	case SCSI_SENSEQ_INIT_COMMAND_REQUIRED:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - INIT_COMMAND_REQUIRED"));
		break;
	case SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - MANUAL_INTERVENTION_REQUIRED"));
		break;
	case SCSI_SENSEQ_FORMAT_IN_PROGRESS:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - FORMAT_IN_PROGRESS"));
		break;
	case SCSI_SENSEQ_REBUILD_IN_PROGRESS:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - REBUILD_IN_PROGRESS"));
		break;
	case SCSI_SENSEQ_RECALCULATION_IN_PROGRESS:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - RECALCULATION_IN_PROGRESS"));
		break;
	case SCSI_SENSEQ_OPERATION_IN_PROGRESS:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - OPERATION_IN_PROGRESS"));
		break;
	case SCSI_SENSEQ_LONG_WRITE_IN_PROGRESS:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - LONG_WRITE_IN_PROGRESS"));
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - SELF-TEST IN PROGRESS"));
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - ASYMMETRIC ACCESS STATE TRANSITION"));
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - TARGET PORT IN STANDBY STATE"));
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - TARGET PORT IN UNAVAILABLE STATE"));
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - AUXILIARY MEMORY NOT ACCESSIBLE"));
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - NOTIFY (ENABLE SPINUP) REQUIRED"));
		break;
	case 0x13:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - SA CREATION IN PROGRESS"));
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - SPACE ALLOCATION IN PROGRESS"));
		break;
	case 0x1a:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - START STOP UNIT COMMAND IN PROGRESS"));
		break;
	case 0x1b:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - SANITIZE IN PROGRESS"));
		break;
	case 0x1c:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - ADDITIONAL POWER USE NOT YET GRANTED"));
		break;
	case 0x1d:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - CONFIGURATION IN PROGRESS"));
		break;
	case 0x1e:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - MICROCODE ACTIVATION REQUIRED"));
		break;
	case 0x1f:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - MICROCODE DOWNLOAD REQUIRED"));
		break;
	case 0x20:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - LOGICAL UNIT RESET REQUIRED"));
		break;
	case 0x21:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - HARD RESET REQUIRED"));
		break;
	case 0x22:
		OutputLog(standardError | fileMainError, _T("LUN_NOT_READY - POWER CYCLE REQUIRED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x05(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("LOGICAL UNIT DOES NOT RESPOND TO SELECTION"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x06(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("NO REFERENCE POSITION FOUND"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x07(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("MULTIPLE PERIPHERAL DEVICES SELECTED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x08(
	BYTE byAscq
) {
	switch (byAscq) {
	case SCSI_SENSEQ_COMM_FAILURE:
		OutputLog(standardError | fileMainError, _T("LUN_COMMUNICATION - COMM_FAILURE"));
		break;
	case SCSI_SENSEQ_COMM_TIMEOUT:
		OutputLog(standardError | fileMainError, _T("LUN_COMMUNICATION - COMM_TIMEOUT"));
		break;
	case SCSI_SENSEQ_COMM_PARITY_ERROR:
		OutputLog(standardError | fileMainError, _T("LUN_COMMUNICATION - COMM_PARITY_ERROR"));
		break;
	case SCSI_SESNEQ_COMM_CRC_ERROR:
		OutputLog(standardError | fileMainError, _T("LUN_COMMUNICATION - COMM_CRC_ERROR"));
		break;
	case SCSI_SENSEQ_UNREACHABLE_TARGET:
		OutputLog(standardError | fileMainError, _T("LUN_COMMUNICATION - UNREACHABLE_TARGET"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x09(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("TRACK FOLLOWING ERROR"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("TRACKING SERVO FAILURE"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("FOCUS SERVO FAILURE"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("SPINDLE SERVO FAILURE"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("HEAD SELECT FAULT"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x0a(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("ERROR LOG OVERFLOW"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x0b(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("WARNING"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("WARNING - SPECIFIED TEMPERATURE EXCEEDED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("WARNING - ENCLOSURE DEGRADED"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("WARNING - BACKGROUND SELF-TEST FAILED"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("WARNING - BACKGROUND PRE-SCAN DETECTED MEDIUM ERROR"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("WARNING - BACKGROUND MEDIUM SCAN DETECTED MEDIUM ERROR"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("WARNING - NON-VOLATILE CACHE NOW VOLATILE"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("WARNING - DEGRADED POWER TO NON-VOLATILE CACHE"));
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, _T("WARNING - POWER LOSS EXPECTED"));
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, _T("WARNING - DEVICE STATISTICS NOTIFICATION ACTIVE"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x0c(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("WRITE ERROR"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("WRITE ERROR - AUTO REALLOCATION FAILED"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("WRITE ERROR - RECOMMEND REASSIGNMENT"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("WRITE ERROR - COMPRESSION CHECK MISCOMPARE ERROR"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("WRITE ERROR - DATA EXPANSION OCCURRED DURING COMPRESSION"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("WRITE ERROR - BLOCK NOT COMPRESSIBLE"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("WRITE ERROR - RECOVERY NEEDED"));
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, _T("WRITE ERROR - RECOVERY FAILED"));
		break;
	case SCSI_SENSEQ_LOSS_OF_STREAMING:
		OutputLog(standardError | fileMainError, _T("WRITE ERROR - LOSS OF STREAMING"));
		break;
	case SCSI_SENSEQ_PADDING_BLOCKS_ADDED:
		OutputLog(standardError | fileMainError, _T("WRITE ERROR - PADDING BLOCKS ADDED"));
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, _T("WRITE ERROR - AUXILIARY MEMORY WRITE ERROR"));
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, _T("WRITE ERROR - UNEXPECTED UNSOLICITED DATA"));
		break;
	case 0x0d:
		OutputLog(standardError | fileMainError, _T("WRITE ERROR - NOT ENOUGH UNSOLICITED DATA"));
		break;
	case 0x0f:
		OutputLog(standardError | fileMainError, _T("WRITE ERROR - DEFECTS IN ERROR WINDOW"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x0d(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("ERROR DETECTED BY THIRD PARTY TEMPORARY INITIATOR"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("THIRD PARTY DEVICE FAILURE"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("COPY TARGET DEVICE NOT REACHABLE"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("INCORRECT COPY TARGET DEVICE TYPE"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("COPY TARGET DEVICE DATA UNDERRUN"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("COPY TARGET DEVICE DATA OVERRUN"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x0e(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("INVALID INFORMATION UNIT"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("INFORMATION UNIT TOO SHORT"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("INFORMATION UNIT TOO LONG"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("INVALID FIELD IN COMMAND INFORMATION UNIT"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x10(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("ID CRC OR ECC ERROR"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("LOGICAL BLOCK GUARD CHECK FAIL"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("LOGICAL BLOCK APPLICATION TAG CHECK FAILED"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("LOGICAL BLOCK REFERENCE TAG CHECK FAILED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x11(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("UNRECOVERED READ ERROR"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("READ RETRIES EXHAUSTED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("ERROR TOO LONG TO CORRECT"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("MULTIPLE READ ERRORS"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("UNRECOVERED READ ERROR - AUTO REALLOCATE FAILED"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("L-EC UNCORRECTABLE ERROR"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("CIRC UNRECOVERED ERROR"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("DATA RE-SYNCHRONIZATION ERROR"));
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, _T("MISCORRECTED ERROR"));
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, _T("UNRECOVERED READ ERROR - RECOMMEND REASSIGNMENT"));
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, _T("UNRECOVERED READ ERROR - RECOMMEND REWRITE THE DATA"));
		break;
	case 0x0d:
		OutputLog(standardError | fileMainError, _T("DE-COMPRESSION CRC ERROR"));
		break;
	case 0x0e:
		OutputLog(standardError | fileMainError, _T("CANNOT DECOMPRESS USING DECLARED ALGORITHM"));
		break;
	case 0x0f:
		OutputLog(standardError | fileMainError, _T("ERROR READING UPC/EAN NUMBER"));
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, _T("ERROR READING ISRC NUMBER"));
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, _T("READ ERROR - LOSS OF STREAMING"));
		break;
	case 0x12:
		OutputLog(standardError | fileMainError, _T("AUXILIARY MEMORY READ ERROR"));
		break;
	case 0x13:
		OutputLog(standardError | fileMainError, _T("READ ERROR - FAILED RETRANSMISSION REQUEST"));
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, _T("READ ERROR - LBA MARKED BAD BY APPLICATION CLIENT"));
		break;
	case 0x15:
		OutputLog(standardError | fileMainError, _T("WRITE AFTER SANITIZE REQUIRED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x12(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("ADDRESS MARK NOT FOUND FOR ID FIELD"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x13(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("ADDRESS MARK NOT FOUND FOR DATA FIELD"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x14(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("TRACK_ERROR - RECORDED ENTITY NOT FOUND"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("TRACK_ERROR - RECORD NOT FOUND"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("TRACK_ERROR - RECORD NOT FOUND - RECOMMEND REASSIGNMENT"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("TRACK_ERROR - RECORD NOT FOUND - DATA AUTO-REALLOCATED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x15(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("SEEK_ERROR - RANDOM POSITIONING ERROR"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("SEEK_ERROR - MECHANICAL POSITIONING ERROR"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("SEEK_ERROR - POSITIONING ERROR DETECTED BY READ OF MEDIUM"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x16(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("DATA SYNCHRONIZATION MARK ERROR"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("DATA SYNC ERROR - DATA REWRITTEN"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("DATA SYNC ERROR - RECOMMEND REWRITE"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("DATA SYNC ERROR - DATA AUTO-REALLOCATED"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("DATA SYNC ERROR - RECOMMEND REASSIGNMENT"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x17(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("REC_DATA_NOECC - RECOVERED DATA WITH NO ERROR CORRECTION APPLIED"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("REC_DATA_NOECC - RECOVERED DATA WITH RETRIES"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("REC_DATA_NOECC - RECOVERED DATA WITH POSITIVE HEAD OFFSET"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("REC_DATA_NOECC - RECOVERED DATA WITH NEGATIVE HEAD OFFSET"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("REC_DATA_NOECC - RECOVERED DATA WITH RETRIES AND/OR CIRC APPLIED"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("REC_DATA_NOECC - RECOVERED DATA USING PREVIOUS SECTOR ID"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - DATA AUTO-REALLOCATED"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - RECOMMEND REASSIGNMENT"));
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, _T("REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - RECOMMEND REWRITE"));
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, _T("REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - DATA REWRITTEN"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x18(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("REC_DATA_ECC - RECOVERED DATA WITH ERROR CORRECTION APPLIED"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("REC_DATA_ECC - RECOVERED DATA WITH ERROR CORR. & RETRIES APPLIED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("REC_DATA_ECC - RECOVERED DATA - DATA AUTO-REALLOCATED"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("REC_DATA_ECC - RECOVERED DATA WITH CIRC"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("REC_DATA_ECC - RECOVERED DATA WITH L-EC"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("REC_DATA_ECC - RECOVERED DATA - RECOMMEND REASSIGNMENT"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("REC_DATA_ECC - RECOVERED DATA - RECOMMEND REWRITE"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("REC_DATA_ECC - RECOVERED DATA WITH ECC - DATA REWRITTEN"));
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, _T("REC_DATA_ECC - RECOVERED DATA WITH LINKING"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x19(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("DEFECT LIST ERROR"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("DEFECT LIST NOT AVAILABLE"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("DEFECT LIST ERROR IN PRIMARY LIST"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("DEFECT LIST ERROR IN GROWN LIST"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x1a(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("PARAMETER LIST LENGTH ERROR"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x1b(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("SYNCHRONOUS DATA TRANSFER ERROR"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x1c(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("DEFECT LIST NOT FOUND"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("PRIMARY DEFECT LIST NOT FOUND"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("GROWN DEFECT LIST NOT FOUND"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x1d(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("MISCOMPARE DURING VERIFY OPERATION"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("MISCOMPARE VERIFY OF UNMAPPED LBA"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x1e(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("RECOVERED ID WITH ECC CORRECTION"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x1f(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("PARTIAL DEFECT LIST TRANSFER"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x20(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("INVALID COMMAND OPERATION CODE"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("ILLEGAL_COMMAND - ACCESS DENIED - INITIATOR PENDING-ENROLLED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("ILLEGAL_COMMAND - ACCESS DENIED - NO ACCESS RIGHTS"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("ILLEGAL_COMMAND - ACCESS DENIED - INVALID MGMT ID KEY"));
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, _T("ILLEGAL_COMMAND - ACCESS DENIED - ENROLLMENT CONFLICT"));
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, _T("ILLEGAL_COMMAND - ACCESS DENIED - INVALID LU IDENTIFIER"));
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, _T("ILLEGAL_COMMAND - ACCESS DENIED - INVALID PROXY TOKEN"));
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, _T("ILLEGAL_COMMAND - ACCESS DENIED - ACL LUN CONFLICT"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x21(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("LOGICAL BLOCK ADDRESS OUT OF RANGE"));
		break;
	case SCSI_SENSEQ_ILLEGAL_ELEMENT_ADDR:
		OutputLog(standardError | fileMainError, _T("ILLEGAL_BLOCK - MISCOMPARE DURING VERIFY OPERATION"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("ILLEGAL_BLOCK - INVALID ADDRESS FOR WRITE"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("ILLEGAL_BLOCK - INVALID WRITE CROSSING LAYER JUMP"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x22(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("ILLEGAL FUNCTION"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x23(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("INVALID TOKEN OPERATION, CAUSE NOT REPORTABLE"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("INVALID TOKEN OPERATION, UNSUPPORTED TOKEN TYPE"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("INVALID TOKEN OPERATION, REMOTE TOKEN USAGE NOT SUPPORTED"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("INVALID TOKEN OPERATION, REMOTE ROD TOKEN CREATION NOT SUPPORTED"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("INVALID TOKEN OPERATION, TOKEN UNKNOWN"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("INVALID TOKEN OPERATION, TOKEN CORRUPT"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("INVALID TOKEN OPERATION, TOKEN REVOKED"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("INVALID TOKEN OPERATION, TOKEN EXPIRED"));
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, _T("INVALID TOKEN OPERATION, TOKEN CANCELLED"));
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, _T("INVALID TOKEN OPERATION, TOKEN DELETED"));
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, _T("INVALID TOKEN OPERATION, INVALID TOKEN LENGTH"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x24(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("INVALID FIELD IN CDB"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("CDB DECRYPTION ERROR"));
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, _T("INVALID XCDB"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x25(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("LOGICAL UNIT NOT SUPPORTED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x26(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("INVALID FIELD IN PARAMETER LIST"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("PARAMETER NOT SUPPORTED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("PARAMETER VALUE INVALID"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("THRESHOLD PARAMETERS NOT SUPPORTED"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("INVALID RELEASE OF PERSISTENT RESERVATION"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("DATA DECRYPTION ERROR"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("TOO MANY TARGET DESCRIPTORS"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("UNSUPPORTED SEGMENT DESCRIPTOR TYPE CODE"));
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, _T("TOO MANY SEGMENT DESCRIPTORS"));
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, _T("UNSUPPORTED SEGMENT DESCRIPTOR TYPE CODE"));
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, _T("UNEXPECTED INEXACT SEGMENT"));
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, _T("INLINE DATA LENGTH EXCEEDED"));
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, _T("INVALID OPERATION FOR COPY SOURCE OR DESTINATION"));
		break;
	case 0x0d:
		OutputLog(standardError | fileMainError, _T("COPY SEGMENT GRANULARITY VIOLATION"));
		break;
	case 0x0e:
		OutputLog(standardError | fileMainError, _T("INVALID PARAMETER WHILE PORT IS ENABLED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x27(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("WRITE PROTECTED"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("HARDWARE WRITE PROTECTED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("LOGICAL UNIT SOFTWARE WRITE PROTECTED"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("ASSOCIATED WRITE PROTECT"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("PERSISTENT WRITE PROTECT"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("PERMANENT WRITE PROTECT"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("CONDITIONAL WRITE PROTECT"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("SPACE ALLOCATION FAILED WRITE PROTECT"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x28(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("NOT READY TO READY CHANGE, MEDIUM MAY HAVE CHANGED"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("IMPORT OR EXPORT ELEMENT ACCESSED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("FORMAT-LAYER MAY HAVE CHANGED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x29(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("POWER ON, RESET, OR BUS DEVICE RESET OCCURRED"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("POWER ON OCCURRED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("SCSI BUS RESET OCCURRED"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("BUS DEVICE RESET FUNCTION OCCURRED"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("DEVICE INTERNAL RESET"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("TRANSCEIVER MODE CHANGED TO SINGLE-ENDED"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("TRANSCEIVER MODE CHANGED TO LVD"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("I_T NEXUS LOSS OCCURRED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x2a(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("PARAMETERS CHANGED"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("MODE PARAMETERS CHANGED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("LOG PARAMETERS CHANGED"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("RESERVATIONS PREEMPTED"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("RESERVATIONS RELEASED"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("REGISTRATIONS PREEMPTED"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("ASYMMETRIC ACCESS STATE CHANGED"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("IMPLICIT ASYMMETRIC ACCESS STATE TRANSITION FAILED"));
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, _T("PRIORITY CHANGED"));
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, _T("CAPACITY DATA HAS CHANGED"));
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, _T("ERROR HISTORY I_T NEXUS CLEARED"));
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, _T("ERROR HISTORY SNAPSHOT RELEASED"));
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, _T("TIMESTAMP CHANGED"));
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, _T("SA CREATION CAPABILITIES DATA HAS CHANGED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x2b(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("COPY CANNOT EXECUTE SINCE HOST CANNOT DISCONNECT"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x2c(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("COMMAND SEQUENCE ERROR"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("CURRENT PROGRAM AREA IS NOT EMPTY"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("CURRENT PROGRAM AREA IS EMPTY"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("PERSISTENT PREVENT CONFLICT"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("PREVIOUS BUSY STATUS"));
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, _T("PREVIOUS TASK SET FULL STATUS"));
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, _T("PREVIOUS RESERVATION CONFLICT STATUS"));
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, _T("ORWRITE GENERATION DOES NOT MATCH"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x2e(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("INSUFFICIENT TIME FOR OPERATION"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("COMMAND TIMEOUT BEFORE PROCESSING"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("COMMAND TIMEOUT DURING PROCESSING"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("COMMAND TIMEOUT DURING PROCESSING DUE TO ERROR RECOVERY"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x2f(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("COMMANDS CLEARED BY ANOTHER INITIATOR"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("COMMANDS CLEARED BY POWER LOSS NOTIFICATION"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("COMMANDS CLEARED BY DEVICE SERVER"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("SOME COMMANDS CLEARED BY QUEUING LAYER EVENT"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x30(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("INCOMPATIBLE MEDIUM INSTALLED"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("CANNOT READ MEDIUM - UNKNOWN FORMAT"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("CANNOT READ MEDIUM - INCOMPATIBLE FORMAT"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("CLEANING CARTRIDGE INSTALLED"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("CANNOT WRITE MEDIUM - UNKNOWN FORMAT"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("CANNOT WRITE MEDIUM - INCOMPATIBLE FORMAT"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("CANNOT FORMAT MEDIUM - INCOMPATIBLE MEDIUM"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("CLEANING FAILURE"));
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, _T("CANNOT WRITE - APPLICATION CODE MISMATCH"));
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, _T("CURRENT SESSION NOT FIXATED FOR APPEND"));
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, _T("CLEANING REQUEST REJECTED"));
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, _T("MEDIUM NOT FORMATTED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x31(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("MEDIUM FORMAT CORRUPTED"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("FORMAT COMMAND FAILED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("ZONED FORMATTING FAILED DUE TO SPARE LINKING"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("SANITIZE COMMAND FAILED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x32(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("NO DEFECT SPARE LOCATION AVAILABLE"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("DEFECT LIST UPDATE FAILURE"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x34(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("ENCLOSURE FAILURE"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x35(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("ENCLOSURE SERVICES FAILURE"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("UNSUPPORTED ENCLOSURE FUNCTION"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("ENCLOSURE SERVICES UNAVAILABLE"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("ENCLOSURE SERVICES TRANSFER FAILURE"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("ENCLOSURE SERVICES TRANSFER REFUSED"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("ENCLOSURE SERVICES CHECKSUM ERROR"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x37(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("ROUNDED PARAMETER"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x38(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x07:
		OutputLog(standardError | fileMainError, _T("THIN PROVISIONING SOFT THRESHOLD REACHED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x39(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("SAVING PARAMETERS NOT SUPPORTED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x3a(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("MEDIUM NOT PRESENT"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("MEDIUM NOT PRESENT - TRAY CLOSED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("MEDIUM NOT PRESENT - TRAY OPEN"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("MEDIUM NOT PRESENT - LOADABLE"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("MEDIUM NOT PRESENT - MEDIUM AUXILIARY MEMORY ACCESSIBLE"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x3b(
	BYTE byAscq
) {
	switch (byAscq) {
	case SCSI_SENSEQ_DESTINATION_FULL:
		OutputLog(standardError | fileMainError, _T("MEDIUM DESTINATION ELEMENT FULL"));
		break;
	case SCSI_SENSEQ_SOURCE_EMPTY:
		OutputLog(standardError | fileMainError, _T("MEDIUM SOURCE ELEMENT EMPTY"));
		break;
	case 0x0f:
		OutputLog(standardError | fileMainError, _T("END OF MEDIUM REACHED"));
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, _T("MEDIUM MAGAZINE NOT ACCESSIBLE"));
		break;
	case 0x12:
		OutputLog(standardError | fileMainError, _T("MEDIUM MAGAZINE REMOVED"));
		break;
	case 0x13:
		OutputLog(standardError | fileMainError, _T("MEDIUM MAGAZINE INSERTED"));
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, _T("MEDIUM MAGAZINE LOCKED"));
		break;
	case 0x15:
		OutputLog(standardError | fileMainError, _T("MEDIUM MAGAZINE UNLOCKED"));
		break;
	case 0x16:
		OutputLog(standardError | fileMainError, _T("MECHANICAL POSITIONING OR CHANGER ERROR"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x3d(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("INVALID BITS IN IDENTIFY MESSAGE"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x3e(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("LOGICAL UNIT HAS NOT SELF-CONFIGURED YET"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("LOGICAL UNIT FAILURE"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("TIMEOUT ON LOGICAL UNIT"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("LOGICAL UNIT FAILED SELF-TEST"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("LOGICAL UNIT UNABLE TO UPDATE SELF-TEST LOG"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x3f(
	BYTE byAscq
) {
	switch (byAscq) {
	case SCSI_SENSEQ_TARGET_OPERATING_CONDITIONS_CHANGED:
		OutputLog(standardError | fileMainError, _T("TARGET_OPERATING_CONDITIONS_CHANGED"));
		break;
	case SCSI_SENSEQ_MICROCODE_CHANGED:
		OutputLog(standardError | fileMainError, _T("MICROCODE_CHANGED"));
		break;
	case SCSI_SENSEQ_OPERATING_DEFINITION_CHANGED:
		OutputLog(standardError | fileMainError, _T("OPERATING_DEFINITION_CHANGED"));
		break;
	case SCSI_SENSEQ_INQUIRY_DATA_CHANGED:
		OutputLog(standardError | fileMainError, _T("INQUIRY_DATA_CHANGED"));
		break;
	case SCSI_SENSEQ_COMPONENT_DEVICE_ATTACHED:
		OutputLog(standardError | fileMainError, _T("COMPONENT_DEVICE_ATTACHED"));
		break;
	case SCSI_SENSEQ_DEVICE_IDENTIFIER_CHANGED:
		OutputLog(standardError | fileMainError, _T("DEVICE_IDENTIFIER_CHANGED"));
		break;
	case SCSI_SENSEQ_REDUNDANCY_GROUP_MODIFIED:
		OutputLog(standardError | fileMainError, _T("REDUNDANCY_GROUP_MODIFIED"));
		break;
	case SCSI_SENSEQ_REDUNDANCY_GROUP_DELETED:
		OutputLog(standardError | fileMainError, _T("REDUNDANCY_GROUP_DELETED"));
		break;
	case SCSI_SENSEQ_SPARE_MODIFIED:
		OutputLog(standardError | fileMainError, _T("SPARE_MODIFIED"));
		break;
	case SCSI_SENSEQ_SPARE_DELETED:
		OutputLog(standardError | fileMainError, _T("SPARE_DELETED"));
		break;
	case SCSI_SENSEQ_VOLUME_SET_MODIFIED:
		OutputLog(standardError | fileMainError, _T("VOLUME_SET_MODIFIED"));
		break;
	case SCSI_SENSEQ_VOLUME_SET_DELETED:
		OutputLog(standardError | fileMainError, _T("VOLUME SET DELETED"));
		break;
	case SCSI_SENSEQ_VOLUME_SET_DEASSIGNED:
		OutputLog(standardError | fileMainError, _T("VOLUME SET DEASSIGNED"));
		break;
	case SCSI_SENSEQ_VOLUME_SET_REASSIGNED:
		OutputLog(standardError | fileMainError, _T("VOLUME SET REASSIGNED"));
		break;
	case SCSI_SENSEQ_REPORTED_LUNS_DATA_CHANGED:
		OutputLog(standardError | fileMainError, _T("REPORTED_LUNS_DATA_CHANGED"));
		break;
	case SCSI_SENSEQ_ECHO_BUFFER_OVERWRITTEN:
		OutputLog(standardError | fileMainError, _T("ECHO_BUFFER_OVERWRITTEN"));
		break;
	case SCSI_SENSEQ_MEDIUM_LOADABLE:
		OutputLog(standardError | fileMainError, _T("MEDIUM LOADABLE"));
		break;
	case SCSI_SENSEQ_MEDIUM_AUXILIARY_MEMORY_ACCESSIBLE:
		OutputLog(standardError | fileMainError, _T("MEDIUM_AUXILIARY_MEMORY_ACCESSIBLE"));
		break;
	case 0x12:
		OutputLog(standardError | fileMainError, _T("iSCSI IP ADDRESS ADDED"));
		break;
	case 0x13:
		OutputLog(standardError | fileMainError, _T("iSCSI IP ADDRESS REMOVED"));
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, _T("iSCSI IP ADDRESS CHANGED"));
		break;
	case 0x15:
		OutputLog(standardError | fileMainError, _T("INSPECT REFERRALS SENSE DESCRIPTORS"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x40(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("RAM FAILURE"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x41(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("DATA PATH FAILURE"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x42(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("POWER-ON OR SELF-TEST FAILURE"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x43(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("MESSAGE ERROR"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x44(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("INTERNAL TARGET FAILURE"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("PERSISTENT RESERVATION INFORMATION LOST"));
		break;
	case 0x71:
		OutputLog(standardError | fileMainError, _T("ATA DEVICE FAILED SET FEATURES"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x45(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("SELECT OR RESELECT FAILURE"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x46(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("UNSUCCESSFUL SOFT RESET"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x47(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("SCSI PARITY ERROR"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("DATA PHASE CRC ERROR DETECTED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("SCSI PARITY ERROR DETECTED DURING ST DATA PHASE"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("INFORMATION UNIT iuCRC ERROR DETECTED"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("ASYNCHRONOUS INFORMATION PROTECTION ERROR DETECTED"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("PROTOCOL SERVICE CRC ERROR"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("PHY TEST FUNCTION IN PROGRESS"));
		break;
	case 0x7f:
		OutputLog(standardError | fileMainError, _T("SOME COMMANDS CLEARED BY ISCSI PROTOCOL EVENT"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x48(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("INITIATOR DETECTED ERROR MESSAGE RECEIVED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x49(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("INVALID MESSAGE ERROR"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x4a(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("COMMAND PHASE ERROR"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x4b(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("DATA PHASE ERROR"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("INVALID TARGET PORT TRANSFER TAG RECEIVED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("TOO MUCH WRITE DATA"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("ACK/NAK TIMEOUT"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("NAK RECEIVED"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("DATA OFFSET ERROR"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("INITIATOR RESPONSE TIMEOUT"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("CONNECTION LOST"));
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, _T("DATA-IN BUFFER OVERFLOW - DATA BUFFER SIZE"));
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, _T("DATA-IN BUFFER OVERFLOW - DATA BUFFER DESCRIPTOR AREA"));
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, _T("DATA-IN BUFFER ERROR"));
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, _T("DATA-OUT BUFFER OVERFLOW - DATA BUFFER SIZE"));
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, _T("DATA-OUT BUFFER OVERFLOW - DATA BUFFER DESCRIPTOR AREA"));
		break;
	case 0x0d:
		OutputLog(standardError | fileMainError, _T("DATA-OUT BUFFER ERROR"));
		break;
	case 0x0e:
		OutputLog(standardError | fileMainError, _T("PCIE FABRIC ERROR"));
		break;
	case 0x0f:
		OutputLog(standardError | fileMainError, _T("PCIE COMPLETION TIMEOUT"));
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, _T("PCIE COMPLETER ABORT"));
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, _T("PCIE POISONED TLP RECEIVED"));
		break;
	case 0x12:
		OutputLog(standardError | fileMainError, _T("PCIE ECRC CHECK FAILED"));
		break;
	case 0x13:
		OutputLog(standardError | fileMainError, _T("PCIE UNSUPPORTED REQUEST"));
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, _T("PCIE ACS VIOLATION"));
		break;
	case 0x15:
		OutputLog(standardError | fileMainError, _T("PCIE TLP PREFIX BLOCKED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x4c(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("LOGICAL UNIT FAILED SELF-CONFIGURATION"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x4e(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("OVERLAPPED COMMANDS ATTEMPTED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x51(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("ERASE FAILURE"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("ERASE FAILURE - INCOMPLETE ERASE OPERATION DETECTED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x53(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("MEDIA LOAD OR EJECT FAILED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("MEDIUM REMOVAL PREVENTED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x55(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x01:
		OutputLog(standardError | fileMainError, _T("SYSTEM BUFFER FULL"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("INSUFFICIENT RESERVATION RESOURCES"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("INSUFFICIENT RESOURCES"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("INSUFFICIENT REGISTRATION RESOURCES"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("INSUFFICIENT ACCESS CONTROL RESOURCES"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("AUXILIARY MEMORY OUT OF SPACE"));
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, _T("INSUFFICIENT POWER FOR OPERATION"));
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, _T("INSUFFICIENT RESOURCES TO CREATE ROD"));
		break;
	case 0x0d:
		OutputLog(standardError | fileMainError, _T("INSUFFICIENT RESOURCES TO CREATE ROD TOKEN"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x57(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("UNABLE TO RECOVER TABLE-OF-CONTENTS"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x58(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("GENERATION DOES NOT EXIST"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x59(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("UPDATED BLOCK READ"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x5a(
	BYTE byAscq
) {
	switch (byAscq) {
	case SCSI_SENSEQ_STATE_CHANGE_INPUT:
		OutputLog(standardError | fileMainError, _T("OPERATOR REQUEST OR STATE CHANGE INPUT"));
		break;
	case SCSI_SENSEQ_MEDIUM_REMOVAL:
		OutputLog(standardError | fileMainError, _T("OPERATOR MEDIUM REMOVAL REQUEST"));
		break;
	case SCSI_SENSEQ_WRITE_PROTECT_ENABLE:
		OutputLog(standardError | fileMainError, _T("OPERATOR SELECTED WRITE PROTECT"));
		break;
	case SCSI_SENSEQ_WRITE_PROTECT_DISABLE:
		OutputLog(standardError | fileMainError, _T("OPERATOR SELECTED WRITE PERMIT"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x5b(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("LOG EXCEPTION"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("THRESHOLD CONDITION MET"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("LOG COUNTER AT MAXIMUM"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("LOG LIST CODES EXHAUSTED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x5c(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("RPL STATUS CHANGE"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("SPINDLES SYNCHRONIZED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("SPINDLES NOT SYNCHRONIZED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x5d(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("FAILURE PREDICTION THRESHOLD EXCEEDED"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("MEDIA FAILURE PREDICTION THRESHOLD EXCEEDED"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("LOGICAL UNIT FAILURE PREDICTION THRESHOLD EXCEEDED"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("SPARE AREA EXHAUSTION PREDICTION THRESHOLD EXCEEDED"));
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, _T("HARDWARE IMPENDING FAILURE GENERAL HARD DRIVE FAILURE"));
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, _T("HARDWARE IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH"));
		break;
	case 0x12:
		OutputLog(standardError | fileMainError, _T("HARDWARE IMPENDING FAILURE DATA ERROR RATE TOO HIGH"));
		break;
	case 0x13:
		OutputLog(standardError | fileMainError, _T("HARDWARE IMPENDING FAILURE SEEK ERROR RATE TOO HIGH"));
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, _T("HARDWARE IMPENDING FAILURE TOO MANY BLOCK REASSIGNS"));
		break;
	case 0x15:
		OutputLog(standardError | fileMainError, _T("HARDWARE IMPENDING FAILURE ACCESS TIMES TOO HIGH"));
		break;
	case 0x16:
		OutputLog(standardError | fileMainError, _T("HARDWARE IMPENDING FAILURE START UNIT TIMES TOO HIGH"));
		break;
	case 0x17:
		OutputLog(standardError | fileMainError, _T("HARDWARE IMPENDING FAILURE CHANNEL PARAMETRICS"));
		break;
	case 0x18:
		OutputLog(standardError | fileMainError, _T("HARDWARE IMPENDING FAILURE CONTROLLER DETECTED"));
		break;
	case 0x19:
		OutputLog(standardError | fileMainError, _T("HARDWARE IMPENDING FAILURE THROUGHPUT PERFORMANCE"));
		break;
	case 0x1a:
		OutputLog(standardError | fileMainError, _T("HARDWARE IMPENDING FAILURE SEEK TIME PERFORMANCE"));
		break;
	case 0x1b:
		OutputLog(standardError | fileMainError, _T("HARDWARE IMPENDING FAILURE SPIN-UP RETRY COUNT"));
		break;
	case 0x1c:
		OutputLog(standardError | fileMainError, _T("HARDWARE IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT"));
		break;
	case 0x20:
		OutputLog(standardError | fileMainError, _T("CONTROLLER IMPENDING FAILURE GENERAL HARD DRIVE FAILURE"));
		break;
	case 0x21:
		OutputLog(standardError | fileMainError, _T("CONTROLLER IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH"));
		break;
	case 0x22:
		OutputLog(standardError | fileMainError, _T("CONTROLLER IMPENDING FAILURE DATA ERROR RATE TOO HIGH"));
		break;
	case 0x23:
		OutputLog(standardError | fileMainError, _T("CONTROLLER IMPENDING FAILURE SEEK ERROR RATE TOO HIGH"));
		break;
	case 0x24:
		OutputLog(standardError | fileMainError, _T("CONTROLLER IMPENDING FAILURE TOO MANY BLOCK REASSIGNS"));
		break;
	case 0x25:
		OutputLog(standardError | fileMainError, _T("CONTROLLER IMPENDING FAILURE ACCESS TIMES TOO HIGH"));
		break;
	case 0x26:
		OutputLog(standardError | fileMainError, _T("CONTROLLER IMPENDING FAILURE START UNIT TIMES TOO HIGH"));
		break;
	case 0x27:
		OutputLog(standardError | fileMainError, _T("CONTROLLER IMPENDING FAILURE CHANNEL PARAMETRICS"));
		break;
	case 0x28:
		OutputLog(standardError | fileMainError, _T("CONTROLLER IMPENDING FAILURE CONTROLLER DETECTED"));
		break;
	case 0x29:
		OutputLog(standardError | fileMainError, _T("CONTROLLER IMPENDING FAILURE THROUGHPUT PERFORMANCE"));
		break;
	case 0x2a:
		OutputLog(standardError | fileMainError, _T("CONTROLLER IMPENDING FAILURE SEEK TIME PERFORMANCE"));
		break;
	case 0x2b:
		OutputLog(standardError | fileMainError, _T("CONTROLLER IMPENDING FAILURE SPIN-UP RETRY COUNT"));
		break;
	case 0x2c:
		OutputLog(standardError | fileMainError, _T("CONTROLLER IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT"));
		break;
	case 0x30:
		OutputLog(standardError | fileMainError, _T("DATA CHANNEL IMPENDING FAILURE GENERAL HARD DRIVE FAILURE"));
		break;
	case 0x31:
		OutputLog(standardError | fileMainError, _T("DATA CHANNEL IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH"));
		break;
	case 0x32:
		OutputLog(standardError | fileMainError, _T("DATA CHANNEL IMPENDING FAILURE DATA ERROR RATE TOO HIGH"));
		break;
	case 0x33:
		OutputLog(standardError | fileMainError, _T("DATA CHANNEL IMPENDING FAILURE SEEK ERROR RATE TOO HIGH"));
		break;
	case 0x34:
		OutputLog(standardError | fileMainError, _T("DATA CHANNEL IMPENDING FAILURE TOO MANY BLOCK REASSIGNS"));
		break;
	case 0x35:
		OutputLog(standardError | fileMainError, _T("DATA CHANNEL IMPENDING FAILURE ACCESS TIMES TOO HIGH"));
		break;
	case 0x36:
		OutputLog(standardError | fileMainError, _T("DATA CHANNEL IMPENDING FAILURE START UNIT TIMES TOO HIGH"));
		break;
	case 0x37:
		OutputLog(standardError | fileMainError, _T("DATA CHANNEL IMPENDING FAILURE CHANNEL PARAMETRICS"));
		break;
	case 0x38:
		OutputLog(standardError | fileMainError, _T("DATA CHANNEL IMPENDING FAILURE CONTROLLER DETECTED"));
		break;
	case 0x39:
		OutputLog(standardError | fileMainError, _T("DATA CHANNEL IMPENDING FAILURE THROUGHPUT PERFORMANCE"));
		break;
	case 0x3a:
		OutputLog(standardError | fileMainError, _T("DATA CHANNEL IMPENDING FAILURE SEEK TIME PERFORMANCE"));
		break;
	case 0x3b:
		OutputLog(standardError | fileMainError, _T("DATA CHANNEL IMPENDING FAILURE SPIN-UP RETRY COUNT"));
		break;
	case 0x3c:
		OutputLog(standardError | fileMainError, _T("DATA CHANNEL IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT"));
		break;
	case 0x40:
		OutputLog(standardError | fileMainError, _T("SERVO IMPENDING FAILURE GENERAL HARD DRIVE FAILURE"));
		break;
	case 0x41:
		OutputLog(standardError | fileMainError, _T("SERVO IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH"));
		break;
	case 0x42:
		OutputLog(standardError | fileMainError, _T("SERVO IMPENDING FAILURE DATA ERROR RATE TOO HIGH"));
		break;
	case 0x43:
		OutputLog(standardError | fileMainError, _T("SERVO IMPENDING FAILURE SEEK ERROR RATE TOO HIGH"));
		break;
	case 0x44:
		OutputLog(standardError | fileMainError, _T("SERVO IMPENDING FAILURE TOO MANY BLOCK REASSIGNS"));
		break;
	case 0x45:
		OutputLog(standardError | fileMainError, _T("SERVO IMPENDING FAILURE ACCESS TIMES TOO HIGH"));
		break;
	case 0x46:
		OutputLog(standardError | fileMainError, _T("SERVO IMPENDING FAILURE START UNIT TIMES TOO HIGH"));
		break;
	case 0x47:
		OutputLog(standardError | fileMainError, _T("SERVO IMPENDING FAILURE CHANNEL PARAMETRICS"));
		break;
	case 0x48:
		OutputLog(standardError | fileMainError, _T("SERVO IMPENDING FAILURE CONTROLLER DETECTED"));
		break;
	case 0x49:
		OutputLog(standardError | fileMainError, _T("SERVO IMPENDING FAILURE THROUGHPUT PERFORMANCE"));
		break;
	case 0x4a:
		OutputLog(standardError | fileMainError, _T("SERVO IMPENDING FAILURE SEEK TIME PERFORMANCE"));
		break;
	case 0x4b:
		OutputLog(standardError | fileMainError, _T("SERVO IMPENDING FAILURE SPIN-UP RETRY COUNT"));
		break;
	case 0x4c:
		OutputLog(standardError | fileMainError, _T("SERVO IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT"));
		break;
	case 0x50:
		OutputLog(standardError | fileMainError, _T("SPINDLE IMPENDING FAILURE GENERAL HARD DRIVE FAILURE"));
		break;
	case 0x51:
		OutputLog(standardError | fileMainError, _T("SPINDLE IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH"));
		break;
	case 0x52:
		OutputLog(standardError | fileMainError, _T("SPINDLE IMPENDING FAILURE DATA ERROR RATE TOO HIGH"));
		break;
	case 0x53:
		OutputLog(standardError | fileMainError, _T("SPINDLE IMPENDING FAILURE SEEK ERROR RATE TOO HIGH"));
		break;
	case 0x54:
		OutputLog(standardError | fileMainError, _T("SPINDLE IMPENDING FAILURE TOO MANY BLOCK REASSIGNS"));
		break;
	case 0x55:
		OutputLog(standardError | fileMainError, _T("SPINDLE IMPENDING FAILURE ACCESS TIMES TOO HIGH"));
		break;
	case 0x56:
		OutputLog(standardError | fileMainError, _T("SPINDLE IMPENDING FAILURE START UNIT TIMES TOO HIGH"));
		break;
	case 0x57:
		OutputLog(standardError | fileMainError, _T("SPINDLE IMPENDING FAILURE CHANNEL PARAMETRICS"));
		break;
	case 0x58:
		OutputLog(standardError | fileMainError, _T("SPINDLE IMPENDING FAILURE CONTROLLER DETECTED"));
		break;
	case 0x59:
		OutputLog(standardError | fileMainError, _T("SPINDLE IMPENDING FAILURE THROUGHPUT PERFORMANCE"));
		break;
	case 0x5a:
		OutputLog(standardError | fileMainError, _T("SPINDLE IMPENDING FAILURE SEEK TIME PERFORMANCE"));
		break;
	case 0x5b:
		OutputLog(standardError | fileMainError, _T("SPINDLE IMPENDING FAILURE SPIN-UP RETRY COUNT"));
		break;
	case 0x5c:
		OutputLog(standardError | fileMainError, _T("SPINDLE IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT"));
		break;
	case 0x60:
		OutputLog(standardError | fileMainError, _T("FIRMWARE IMPENDING FAILURE GENERAL HARD DRIVE FAILURE"));
		break;
	case 0x61:
		OutputLog(standardError | fileMainError, _T("FIRMWARE IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH"));
		break;
	case 0x62:
		OutputLog(standardError | fileMainError, _T("FIRMWARE IMPENDING FAILURE DATA ERROR RATE TOO HIGH"));
		break;
	case 0x63:
		OutputLog(standardError | fileMainError, _T("FIRMWARE IMPENDING FAILURE SEEK ERROR RATE TOO HIGH"));
		break;
	case 0x64:
		OutputLog(standardError | fileMainError, _T("FIRMWARE IMPENDING FAILURE TOO MANY BLOCK REASSIGNS"));
		break;
	case 0x65:
		OutputLog(standardError | fileMainError, _T("FIRMWARE IMPENDING FAILURE ACCESS TIMES TOO HIGH"));
		break;
	case 0x66:
		OutputLog(standardError | fileMainError, _T("FIRMWARE IMPENDING FAILURE START UNIT TIMES TOO HIGH"));
		break;
	case 0x67:
		OutputLog(standardError | fileMainError, _T("FIRMWARE IMPENDING FAILURE CHANNEL PARAMETRICS"));
		break;
	case 0x68:
		OutputLog(standardError | fileMainError, _T("FIRMWARE IMPENDING FAILURE CONTROLLER DETECTED"));
		break;
	case 0x69:
		OutputLog(standardError | fileMainError, _T("FIRMWARE IMPENDING FAILURE THROUGHPUT PERFORMANCE"));
		break;
	case 0x6a:
		OutputLog(standardError | fileMainError, _T("FIRMWARE IMPENDING FAILURE SEEK TIME PERFORMANCE"));
		break;
	case 0x6b:
		OutputLog(standardError | fileMainError, _T("FIRMWARE IMPENDING FAILURE SPIN-UP RETRY COUNT"));
		break;
	case 0x6c:
		OutputLog(standardError | fileMainError, _T("FIRMWARE IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT"));
		break;
	case 0xff:
		OutputLog(standardError | fileMainError, _T("FAILURE PREDICTION THRESHOLD EXCEEDED (FALSE)"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x5e(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("LOW POWER CONDITION ON"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("IDLE CONDITION ACTIVATED BY TIMER"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("STANDBY CONDITION ACTIVATED BY TIMER"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("IDLE CONDITION ACTIVATED BY COMMAND"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("STANDBY CONDITION ACTIVATED BY COMMAND"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("IDLE_B CONDITION ACTIVATED BY TIMER"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("IDLE_B CONDITION ACTIVATED BY COMMAND"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("IDLE_C CONDITION ACTIVATED BY TIMER"));
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, _T("IDLE_C CONDITION ACTIVATED BY COMMAND"));
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, _T("STANDBY_Y CONDITION ACTIVATED BY TIMER"));
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, _T("STANDBY_Y CONDITION ACTIVATED BY COMMAND"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x63(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("END OF USER AREA ENCOUNTERED ON THIS TRACK"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("PACKET DOES NOT FIT IN AVAILABLE SPACE"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x64(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("ILLEGAL MODE FOR THIS TRACK"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("INVALID PACKET SIZE"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x65(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("VOLTAGE FAULT"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x67(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x0a:
		OutputLog(standardError | fileMainError, _T("SET TARGET PORT GROUPS COMMAND FAILED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x68(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x0a:
		OutputLog(standardError | fileMainError, _T("SUBSIDIARY LOGICAL UNIT NOT CONFIGURED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x6f(
	BYTE byAscq
) {
	switch (byAscq) {
	case SCSI_SENSEQ_AUTHENTICATION_FAILURE:
		OutputLog(standardError | fileMainError, _T("COPY PROTECTION KEY EXCHANGE FAILURE - AUTHENTICATION FAILURE"));
		break;
	case SCSI_SENSEQ_KEY_NOT_PRESENT:
		OutputLog(standardError | fileMainError, _T("COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT PRESENT"));
		break;
	case SCSI_SENSEQ_KEY_NOT_ESTABLISHED:
		OutputLog(standardError | fileMainError, _T("COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT ESTABLISHED"));
		break;
	case SCSI_SENSEQ_READ_OF_SCRAMBLED_SECTOR_WITHOUT_AUTHENTICATION:
		OutputLog(standardError | fileMainError, _T("READ OF SCRAMBLED SECTOR WITHOUT AUTHENTICATION"));
		break;
	case SCSI_SENSEQ_MEDIA_CODE_MISMATCHED_TO_LOGICAL_UNIT:
		OutputLog(standardError | fileMainError, _T("MEDIA REGION CODE IS MISMATCHED TO LOGICAL UNIT REGION"));
		break;
	case SCSI_SENSEQ_LOGICAL_UNIT_RESET_COUNT_ERROR:
		OutputLog(standardError | fileMainError, _T("DRIVE REGION MUST BE PERMANENT/REGION RESET COUNT ERROR"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("INSUFFICIENT BLOCK COUNT FOR BINDING NONCE RECORDING"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("CONFLICT IN BINDING NONCE RECORDING"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x72(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("SESSION FIXATION ERROR"));
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, _T("SESSION FIXATION ERROR WRITING LEAD-IN"));
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, _T("SESSION FIXATION ERROR WRITING LEAD-OUT"));
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, _T("SESSION FIXATION ERROR - INCOMPLETE TRACK IN SESSION"));
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, _T("EMPTY OR PARTIALLY WRITTEN RESERVED TRACK"));
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, _T("NO MORE TRACK RESERVATIONS ALLOWED"));
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, _T("RMZ EXTENSION IS NOT ALLOWED"));
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, _T("NO MORE TEST ZONE EXTENSIONS ARE ALLOWED"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x73(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, _T("CD CONTROL ERROR"));
		break;
	case SCSI_SENSEQ_POWER_CALIBRATION_AREA_ALMOST_FULL:
		OutputLog(standardError | fileMainError, _T("POWER CALIBRATION AREA ALMOST FULL"));
		break;
	case SCSI_SENSEQ_POWER_CALIBRATION_AREA_FULL:
		OutputLog(standardError | fileMainError, _T("POWER CALIBRATION AREA IS FULL"));
		break;
	case SCSI_SENSEQ_POWER_CALIBRATION_AREA_ERROR:
		OutputLog(standardError | fileMainError, _T("POWER CALIBRATION AREA ERROR"));
		break;
	case SCSI_SENSEQ_PMA_RMA_UPDATE_FAILURE:
		OutputLog(standardError | fileMainError, _T("PROGRAM MEMORY AREA UPDATE FAILURE"));
		break;
	case SCSI_SENSEQ_PMA_RMA_IS_FULL:
		OutputLog(standardError | fileMainError, _T("RMA/PMA IS FULL"));
		break;
	case SCSI_SENSEQ_PMA_RMA_ALMOST_FULL:
		OutputLog(standardError | fileMainError, _T("RMA/PMA IS ALMOST FULL"));
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, _T("CURRENT POWER CALIBRATION AREA ALMOST FULL"));
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, _T("CURRENT POWER CALIBRATION AREA IS FULL"));
		break;
	case 0x17:
		OutputLog(standardError | fileMainError, _T("RDZ IS FULL"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x74(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x08:
		OutputLog(standardError | fileMainError, _T("DIGITAL SIGNATURE VALIDATION FAILURE"));
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, _T("UNABLE TO DECRYPT PARAMETER LIST"));
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, _T("SA CREATION PARAMETER VALUE INVALID"));
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, _T("SA CREATION PARAMETER VALUE REJECTED"));
		break;
	case 0x12:
		OutputLog(standardError | fileMainError, _T("INVALID SA USAGE"));
		break;
	case 0x30:
		OutputLog(standardError | fileMainError, _T("SA CREATION PARAMETER NOT SUPPORTED"));
		break;
	case 0x40:
		OutputLog(standardError | fileMainError, _T("AUTHENTICATION FAILED"));
		break;
	case 0x71:
		OutputLog(standardError | fileMainError, _T("LOGICAL UNIT ACCESS NOT AUTHORIZED"));
		break;
	case 0x79:
		OutputLog(standardError | fileMainError, _T("SECURITY CONFLICT IN TRANSLATED DEVICE"));
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
}

// reference
// http://www.t10.org/lists/asc-num.htm
// http://www.t10.org/lists/op-num.htm
// use below
//  Direct Access Block Device (SBC-3)
//  C/DVD Device (MMC-6)
//  Optical Memory Block Device (SBC)
VOID OutputAdditionalSenseCode(
	BYTE byAsc,
	BYTE byAscq
) {
	switch (byAsc) {
	case SCSI_ADSENSE_NO_SENSE:
		OutputAdditionalSenseCodeQualifier0x00(byAscq);
		break;
	case 0x01:
		OutputAdditionalSenseCodeQualifier0x01(byAscq);
		break;
	case SCSI_ADSENSE_NO_SEEK_COMPLETE:
		OutputAdditionalSenseCodeQualifier0x02(byAscq);
		break;
	case 0x03:
		OutputAdditionalSenseCodeQualifier0x03(byAscq);
		break;
	case SCSI_ADSENSE_LUN_NOT_READY:
		OutputAdditionalSenseCodeQualifier0x04(byAscq);
		break;
	case 0x05:
		OutputAdditionalSenseCodeQualifier0x05(byAscq);
		break;
	case 0x06:
		OutputAdditionalSenseCodeQualifier0x06(byAscq);
		break;
	case 0x07:
		OutputAdditionalSenseCodeQualifier0x07(byAscq);
		break;
	case SCSI_ADSENSE_LUN_COMMUNICATION:
		OutputAdditionalSenseCodeQualifier0x08(byAscq);
		break;
	case 0x09:
		OutputAdditionalSenseCodeQualifier0x09(byAscq);
		break;
	case 0x0a:
		OutputAdditionalSenseCodeQualifier0x0a(byAscq);
		break;
	case 0x0b:
		OutputAdditionalSenseCodeQualifier0x0b(byAscq);
		break;
	case SCSI_ADSENSE_WRITE_ERROR:
		OutputAdditionalSenseCodeQualifier0x0c(byAscq);
		break;
	case 0x0d:
		OutputAdditionalSenseCodeQualifier0x0d(byAscq);
		break;
	case 0x0e:
		OutputAdditionalSenseCodeQualifier0x0e(byAscq);
		break;
	case 0x10:
		OutputAdditionalSenseCodeQualifier0x10(byAscq);
		break;
	case 0x11:
		OutputAdditionalSenseCodeQualifier0x11(byAscq);
		break;
	case 0x12:
		OutputAdditionalSenseCodeQualifier0x12(byAscq);
		break;
	case 0x13:
		OutputAdditionalSenseCodeQualifier0x13(byAscq);
		break;
	case SCSI_ADSENSE_TRACK_ERROR:
		OutputAdditionalSenseCodeQualifier0x14(byAscq);
		break;
	case SCSI_ADSENSE_SEEK_ERROR:
		OutputAdditionalSenseCodeQualifier0x15(byAscq);
		break;
	case 0x16:
		OutputAdditionalSenseCodeQualifier0x16(byAscq);
		break;
	case SCSI_ADSENSE_REC_DATA_NOECC:
		OutputAdditionalSenseCodeQualifier0x17(byAscq);
		break;
	case SCSI_ADSENSE_REC_DATA_ECC:
		OutputAdditionalSenseCodeQualifier0x18(byAscq);
		break;
	case 0x19:
		OutputAdditionalSenseCodeQualifier0x19(byAscq);
		break;
	case SCSI_ADSENSE_PARAMETER_LIST_LENGTH:
		OutputAdditionalSenseCodeQualifier0x1a(byAscq);
		break;
	case 0x1b:
		OutputAdditionalSenseCodeQualifier0x1b(byAscq);
		break;
	case 0x1c:
		OutputAdditionalSenseCodeQualifier0x1c(byAscq);
		break;
	case 0x1d:
		OutputAdditionalSenseCodeQualifier0x1d(byAscq);
		break;
	case 0x1e:
		OutputAdditionalSenseCodeQualifier0x1e(byAscq);
		break;
	case 0x1f:
		OutputAdditionalSenseCodeQualifier0x1f(byAscq);
		break;
	case SCSI_ADSENSE_ILLEGAL_COMMAND:
		OutputAdditionalSenseCodeQualifier0x20(byAscq);
		break;
	case SCSI_ADSENSE_ILLEGAL_BLOCK:
		OutputAdditionalSenseCodeQualifier0x21(byAscq);
		break;
	case 0x22:
		OutputAdditionalSenseCodeQualifier0x22(byAscq);
		break;
	case 0x23:
		OutputAdditionalSenseCodeQualifier0x23(byAscq);
		break;
	case SCSI_ADSENSE_INVALID_CDB:
		OutputAdditionalSenseCodeQualifier0x24(byAscq);
		break;
	case SCSI_ADSENSE_INVALID_LUN:
		OutputAdditionalSenseCodeQualifier0x25(byAscq);
		break;
	case SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST:
		OutputAdditionalSenseCodeQualifier0x26(byAscq);
		break;
	case SCSI_ADSENSE_WRITE_PROTECT:
		OutputAdditionalSenseCodeQualifier0x27(byAscq);
		break;
	case SCSI_ADSENSE_MEDIUM_CHANGED:
		OutputAdditionalSenseCodeQualifier0x28(byAscq);
		break;
	case SCSI_ADSENSE_BUS_RESET:
		OutputAdditionalSenseCodeQualifier0x29(byAscq);
		break;
	case SCSI_ADSENSE_PARAMETERS_CHANGED:
		OutputAdditionalSenseCodeQualifier0x2a(byAscq);
		break;
	case 0x2b:
		OutputAdditionalSenseCodeQualifier0x2b(byAscq);
		break;
	case 0x2c:
		OutputAdditionalSenseCodeQualifier0x2c(byAscq);
		break;
	case SCSI_ADSENSE_INSUFFICIENT_TIME_FOR_OPERATION:
		OutputAdditionalSenseCodeQualifier0x2e(byAscq);
		break;
	case 0x2f:
		OutputAdditionalSenseCodeQualifier0x2f(byAscq);
		break;
	case SCSI_ADSENSE_INVALID_MEDIA:
		OutputAdditionalSenseCodeQualifier0x30(byAscq);
		break;
	case 0x31:
		OutputAdditionalSenseCodeQualifier0x31(byAscq);
		break;
	case 0x32:
		OutputAdditionalSenseCodeQualifier0x32(byAscq);
		break;
	case 0x34:
		OutputAdditionalSenseCodeQualifier0x34(byAscq);
		break;
	case 0x35:
		OutputAdditionalSenseCodeQualifier0x35(byAscq);
		break;
	case 0x37:
		OutputAdditionalSenseCodeQualifier0x37(byAscq);
		break;
	case 0x38:
		OutputAdditionalSenseCodeQualifier0x38(byAscq);
		break;
	case 0x39:
		OutputAdditionalSenseCodeQualifier0x39(byAscq);
		break;
	case SCSI_ADSENSE_NO_MEDIA_IN_DEVICE:
		OutputAdditionalSenseCodeQualifier0x3a(byAscq);
		break;
	case SCSI_ADSENSE_POSITION_ERROR:
		OutputAdditionalSenseCodeQualifier0x3b(byAscq);
		break;
	case 0x3d:
		OutputAdditionalSenseCodeQualifier0x3d(byAscq);
		break;
	case 0x3e:
		OutputAdditionalSenseCodeQualifier0x3e(byAscq);
		break;
	case SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED:
		OutputAdditionalSenseCodeQualifier0x3f(byAscq);
		break;
	case 0x40:
		OutputAdditionalSenseCodeQualifier0x40(byAscq);
		break;
	case 0x41:
		OutputAdditionalSenseCodeQualifier0x41(byAscq);
		break;
	case 0x42:
		OutputAdditionalSenseCodeQualifier0x42(byAscq);
		break;
	case 0x43:
		OutputAdditionalSenseCodeQualifier0x43(byAscq);
		break;
	case 0x44:
		OutputAdditionalSenseCodeQualifier0x44(byAscq);
		break;
	case 0x45:
		OutputAdditionalSenseCodeQualifier0x45(byAscq);
		break;
	case 0x46:
		OutputAdditionalSenseCodeQualifier0x46(byAscq);
		break;
	case 0x47:
		OutputAdditionalSenseCodeQualifier0x47(byAscq);
		break;
	case 0x48:
		OutputAdditionalSenseCodeQualifier0x48(byAscq);
		break;
	case 0x49:
		OutputAdditionalSenseCodeQualifier0x49(byAscq);
		break;
	case 0x4a:
		OutputAdditionalSenseCodeQualifier0x4a(byAscq);
		break;
	case 0x4b:
		OutputAdditionalSenseCodeQualifier0x4b(byAscq);
		break;
	case 0x4c:
		OutputAdditionalSenseCodeQualifier0x4c(byAscq);
		break;
	case 0x4e:
		OutputAdditionalSenseCodeQualifier0x4e(byAscq);
		break;
	case 0x51:
		OutputAdditionalSenseCodeQualifier0x51(byAscq);
		break;
	case 0x53:
		OutputAdditionalSenseCodeQualifier0x53(byAscq);
		break;
	case 0x55:
		OutputAdditionalSenseCodeQualifier0x55(byAscq);
		break;
	case 0x57:
		OutputAdditionalSenseCodeQualifier0x57(byAscq);
		break;
	case 0x58:
		OutputAdditionalSenseCodeQualifier0x58(byAscq);
		break;
	case 0x59:
		OutputAdditionalSenseCodeQualifier0x59(byAscq);
		break;
	case SCSI_ADSENSE_OPERATOR_REQUEST:
		OutputAdditionalSenseCodeQualifier0x5a(byAscq);
		break;
	case 0x5b:
		OutputAdditionalSenseCodeQualifier0x5b(byAscq);
		break;
	case 0x5c:
		OutputAdditionalSenseCodeQualifier0x5c(byAscq);
		break;
	case SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED:
		OutputAdditionalSenseCodeQualifier0x5d(byAscq);
		break;
	case 0x5e:
		OutputAdditionalSenseCodeQualifier0x5e(byAscq);
		break;
	case 0x63:
		OutputAdditionalSenseCodeQualifier0x63(byAscq);
		break;
	case SCSI_ADSENSE_ILLEGAL_MODE_FOR_THIS_TRACK:
		OutputAdditionalSenseCodeQualifier0x64(byAscq);
		break;
	case 0x65:
		OutputAdditionalSenseCodeQualifier0x65(byAscq);
		break;
	case 0x67:
		OutputAdditionalSenseCodeQualifier0x67(byAscq);
		break;
	case 0x68:
		OutputAdditionalSenseCodeQualifier0x68(byAscq);
		break;
	case SCSI_ADSENSE_COPY_PROTECTION_FAILURE:
		OutputAdditionalSenseCodeQualifier0x6f(byAscq);
		break;
	case 0x72:
		OutputAdditionalSenseCodeQualifier0x72(byAscq);
		break;
	case SCSI_ADSENSE_POWER_CALIBRATION_ERROR:
		OutputAdditionalSenseCodeQualifier0x73(byAscq);
		break;
	case 0x74:
		OutputAdditionalSenseCodeQualifier0x74(byAscq);
		break;
	default:
		OutputAdditionalSenseCodeQualifierOther(byAscq);
		break;
	}
	OutputLog(standardError | fileMainError, _T("\n"));
}

VOID OutputSenseKey(
	BYTE byKey
) {
	switch (byKey) {
	case SCSI_SENSE_NO_SENSE:
		OutputLog(standardError | fileMainError, _T("NO_SENSE"));
		break;
	case SCSI_SENSE_RECOVERED_ERROR:
		OutputLog(standardError | fileMainError, _T("RECOVERED_ERROR"));
		break;
	case SCSI_SENSE_NOT_READY:
		OutputLog(standardError | fileMainError, _T("NOT_READY"));
		break;
	case SCSI_SENSE_MEDIUM_ERROR:
		OutputLog(standardError | fileMainError, _T("MEDIUM_ERROR"));
		break;
	case SCSI_SENSE_HARDWARE_ERROR:
		OutputLog(standardError | fileMainError, _T("HARDWARE_ERROR"));
		break;
	case SCSI_SENSE_ILLEGAL_REQUEST:
		OutputLog(standardError | fileMainError, _T("ILLEGAL_REQUEST"));
		break;
	case SCSI_SENSE_UNIT_ATTENTION:
		OutputLog(standardError | fileMainError, _T("UNIT_ATTENTION"));
		break;
	case SCSI_SENSE_DATA_PROTECT:
		OutputLog(standardError | fileMainError, _T("DATA_PROTECT"));
		break;
	case SCSI_SENSE_BLANK_CHECK:
		OutputLog(standardError | fileMainError, _T("BLANK_CHECK"));
		break;
	case SCSI_SENSE_UNIQUE:
		OutputLog(standardError | fileMainError, _T("UNIQUE"));
		break;
	case SCSI_SENSE_COPY_ABORTED:
		OutputLog(standardError | fileMainError, _T("COPY_ABORTED"));
		break;
	case SCSI_SENSE_ABORTED_COMMAND:
		OutputLog(standardError | fileMainError, _T("ABORTED_COMMAND"));
		break;
	case SCSI_SENSE_EQUAL:
		OutputLog(standardError | fileMainError, _T("EQUAL"));
		break;
	case SCSI_SENSE_VOL_OVERFLOW:
		OutputLog(standardError | fileMainError, _T("VOL_OVERFLOW"));
		break;
	case SCSI_SENSE_MISCOMPARE:
		OutputLog(standardError | fileMainError, _T("MISCOMPARE"));
		break;
	case SCSI_SENSE_RESERVED:
		OutputLog(standardError | fileMainError, _T("RESERVED"));
		break;
	default:
		OutputLog(standardError | fileMainError, _T("UNKNOWN"));
		break;
	}
	OutputLog(standardError | fileMainError, _T(" - "));
}

VOID OutputSenseData(
	PSENSE_DATA pSenseData
) {
	OutputLog(standardError | fileMainError,
		_T("\tSenseData Key-Asc-Ascq: %02x-%02x-%02x = ")
		, pSenseData->SenseKey, pSenseData->AdditionalSenseCode
		, pSenseData->AdditionalSenseCodeQualifier);
	OutputSenseKey(pSenseData->SenseKey);
	OutputAdditionalSenseCode(
		pSenseData->AdditionalSenseCode, pSenseData->AdditionalSenseCodeQualifier);
#ifdef _DEBUG
	OutputLog(standardError | fileMainError, 
		_T("OtherSenseData\n")
		_T("                  ErrorCode: %#x\n")
		_T("                      Valid: %s\n")
		_T("              SegmentNumber: %u\n")
		_T("            IncorrectLength: %u\n")
		_T("                 EndOfMedia: %s\n")
		_T("                   FileMark: %s\n"),
		pSenseData->ErrorCode,
		BOOLEAN_TO_STRING_YES_NO(pSenseData->Valid),
		pSenseData->SegmentNumber,
		pSenseData->IncorrectLength,
		BOOLEAN_TO_STRING_YES_NO(pSenseData->EndOfMedia),
		BOOLEAN_TO_STRING_YES_NO(pSenseData->FileMark));
	if (pSenseData->Valid) {
		OutputLog(standardError | fileMainError, 
			_T("                Information: %u%u%u%u\n"),
			pSenseData->Information[0],
			pSenseData->Information[1],
			pSenseData->Information[2],
			pSenseData->Information[3]);
	}
	OutputLog(standardError | fileMainError, 
		_T("      AdditionalSenseLength: %u\n")
		_T(" CommandSpecificInformation: %u%u%u%u\n")
		_T("   FieldReplaceableUnitCode: %u\n")
		_T("           SenseKeySpecific: %u%u%u\n"),
		pSenseData->AdditionalSenseLength,
		pSenseData->CommandSpecificInformation[0],
		pSenseData->CommandSpecificInformation[1],
		pSenseData->CommandSpecificInformation[2],
		pSenseData->CommandSpecificInformation[3],
		pSenseData->FieldReplaceableUnitCode,
		pSenseData->SenseKeySpecific[0],
		pSenseData->SenseKeySpecific[1],
		pSenseData->SenseKeySpecific[2]);
#endif
}

VOID OutputScsiStatus(
	BYTE byScsiStatus
) {
	OutputLog(standardError | fileMainError,
		_T("\tScsiStatus: %#04x = "), byScsiStatus);
	switch (byScsiStatus) {
	case SCSISTAT_GOOD:
		OutputLog(standardError | fileMainError, _T("GOOD\n"));
		break;
	case SCSISTAT_CHECK_CONDITION:
		OutputLog(standardError | fileMainError, _T("CHECK_CONDITION\n"));
		break;
	case SCSISTAT_CONDITION_MET:
		OutputLog(standardError | fileMainError, _T("CONDITION_MET\n"));
		break;
	case SCSISTAT_BUSY:
		OutputLog(standardError | fileMainError, _T("BUSY\n"));
		break;
	case SCSISTAT_INTERMEDIATE:
		OutputLog(standardError | fileMainError, _T("INTERMEDIATE\n"));
		break;
	case SCSISTAT_INTERMEDIATE_COND_MET:
		OutputLog(standardError | fileMainError, _T("INTERMEDIATE_COND_MET\n"));
		break;
	case SCSISTAT_RESERVATION_CONFLICT:
		OutputLog(standardError | fileMainError, _T("RESERVATION_CONFLICT\n"));
		break;
	case SCSISTAT_COMMAND_TERMINATED:
		OutputLog(standardError | fileMainError, _T("COMMAND_TERMINATED\n"));
		break;
	case SCSISTAT_QUEUE_FULL:
		OutputLog(standardError | fileMainError, _T("QUEUE_FULL\n"));
		break;
	default:
		OutputLog(standardError | fileMainError, _T("UNKNOWN\n"));
		break;
	}
}

VOID OutputScsiAddress(
	PDEVICE pDevice
) {
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(IoctlScsiGetAddress)
		"\t    Length: %lu\n"
		"\tPortNumber: %u\n"
		"\t    PathId: %u\n"
		"\t  TargetId: %u\n"
		"\t       Lun: %u\n"
		, pDevice->address.Length
		, pDevice->address.PortNumber
		, pDevice->address.PathId
		, pDevice->address.TargetId
		, pDevice->address.Lun);
}

VOID OutputStorageAdaptorDescriptor(
	PSTORAGE_ADAPTER_DESCRIPTOR pAdapterDescriptor,
	LPBOOL lpBusTypeUSB
) {
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(StorageAdapterDescriptor)
		"\t              Version: %lu\n"
		"\t                 Size: %lu\n"
		"\tMaximumTransferLength: %lu (bytes)\n"
		"\t MaximumPhysicalPages: %lu\n"
		"\t        AlignmentMask: %lu "
		, pAdapterDescriptor->Version
		, pAdapterDescriptor->Size
		, pAdapterDescriptor->MaximumTransferLength
		, pAdapterDescriptor->MaximumPhysicalPages
		, pAdapterDescriptor->AlignmentMask);
	switch (pAdapterDescriptor->AlignmentMask) {
	case 0:
		OutputDriveLogA("(Buffers must be aligned on BYTE boundaries)\n");
		break;
	case 1:
		OutputDriveLogA("(Buffers must be aligned on WORD boundaries)\n");
		break;
	case 3:
		OutputDriveLogA("(Buffers must be aligned on DWORD32 boundaries)\n");
		break;
	case 7:
		OutputDriveLogA("(Buffers must be aligned on DWORD64 boundaries)\n");
		break;
	default:
		OutputDriveLogA("\n");
		break;
	}
	OutputDriveLogA(
		"\t       AdapterUsesPio: %s\n"
		"\t     AdapterScansDown: %s\n"
		"\t      CommandQueueing: %s\n"
		"\t  AcceleratedTransfer: %s\n"
		"\t              BusType: "
		, BOOLEAN_TO_STRING_TRUE_FALSE_A(pAdapterDescriptor->AdapterUsesPio)
		, BOOLEAN_TO_STRING_TRUE_FALSE_A(pAdapterDescriptor->AdapterScansDown)
		, BOOLEAN_TO_STRING_TRUE_FALSE_A(pAdapterDescriptor->CommandQueueing)
		, BOOLEAN_TO_STRING_TRUE_FALSE_A(pAdapterDescriptor->AcceleratedTransfer));
	switch (pAdapterDescriptor->BusType) {
	case STORAGE_BUS_TYPE::BusTypeUnknown:
		OutputDriveLogA("BusTypeUnknown\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeScsi:
		OutputDriveLogA("BusTypeScsi\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeAtapi:
		OutputDriveLogA("BusTypeAtapi\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeAta:
		OutputDriveLogA("BusTypeAta\n");
		break;
	case STORAGE_BUS_TYPE::BusType1394:
		OutputDriveLogA("BusType1394\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeSsa:
		OutputDriveLogA("BusTypeSsa\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeFibre:
		OutputDriveLogA("BusTypeFibre\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeUsb:
		OutputDriveLogA("BusTypeUsb\n");
		*lpBusTypeUSB = TRUE;
		break;
	case STORAGE_BUS_TYPE::BusTypeRAID:
		OutputDriveLogA("BusTypeRAID\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeiScsi:
		OutputDriveLogA("BusTypeiScsi\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeSas:
		OutputDriveLogA("BusTypeSas\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeSata:
		OutputDriveLogA("BusTypeSata\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeSd:
		OutputDriveLogA("BusTypeSd\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeMmc:
		OutputDriveLogA("BusTypeMmc\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeVirtual:
		OutputDriveLogA("BusTypeVirtual\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeFileBackedVirtual:
		OutputDriveLogA("BusTypeFileBackedVirtual\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeMax:
		OutputDriveLogA("BusTypeMax\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeMaxReserved:
		OutputDriveLogA("BusTypeMaxReserved\n");
		break;
	default:
		OutputDriveLogA("BusTypeUnknown\n");
		break;
	}
	OutputDriveLogA(
		"\t      BusMajorVersion: %u\n"
		"\t      BusMinorVersion: %u\n",
		pAdapterDescriptor->BusMajorVersion,
		pAdapterDescriptor->BusMinorVersion);
}

VOID OutputMediaType(
	MEDIA_TYPE mType
) {
	switch (mType) {
	case Unknown:
		OutputDiscLogA("Format is unknown\n");
		break;
	case F5_1Pt2_512:
		OutputDiscLogA("5.25, 1.2MB, 512 bytes/sector\n");
		break;
	case F3_1Pt44_512:
		OutputDiscLogA("3.5, 1.44MB, 512 bytes/sector\n");
		break;
	case F3_2Pt88_512:
		OutputDiscLogA("3.5, 2.88MB, 512 bytes/sector\n");
		break;
	case F3_20Pt8_512:
		OutputDiscLogA("3.5, 20.8MB, 512 bytes/sector\n");
		break;
	case F3_720_512:
		OutputDiscLogA("3.5, 720KB, 512 bytes/sector\n");
		break;
	case F5_360_512:
		OutputDiscLogA("5.25, 360KB, 512 bytes/sector\n");
		break;
	case F5_320_512:
		OutputDiscLogA("5.25, 320KB, 512 bytes/sector\n");
		break;
	case F5_320_1024:
		OutputDiscLogA("5.25, 320KB, 1024 bytes/sector\n");
		break;
	case F5_180_512:
		OutputDiscLogA("5.25, 180KB, 512 bytes/sector\n");
		break;
	case F5_160_512:
		OutputDiscLogA("5.25, 160KB, 512 bytes/sector\n");
		break;
	case RemovableMedia:
		OutputDiscLogA("Removable media other than floppy\n");
		break;
	case FixedMedia:
		OutputDiscLogA("Fixed hard disk media\n");
		break;
	case F3_120M_512:
		OutputDiscLogA("3.5, 120M Floppy\n");
		break;
	case F3_640_512:
		OutputDiscLogA("3.5, 640KB, 512 bytes/sector\n");
		break;
	case F5_640_512:
		OutputDiscLogA("5.25, 640KB, 512 bytes/sector\n");
		break;
	case F5_720_512:
		OutputDiscLogA("5.25, 720KB, 512 bytes/sector\n");
		break;
	case F3_1Pt2_512:
		OutputDiscLogA("3.5, 1.2Mb, 512 bytes/sector\n");
		break;
	case F3_1Pt23_1024:
		OutputDiscLogA("3.5, 1.23Mb, 1024 bytes/sector\n");
		break;
	case F5_1Pt23_1024:
		OutputDiscLogA("5.25, 1.23MB, 1024 bytes/sector\n");
		break;
	case F3_128Mb_512:
		OutputDiscLogA("3.5 MO, 128Mb, 512 bytes/sector\n");
		break;
	case F3_230Mb_512:
		OutputDiscLogA("3.5 MO, 230Mb, 512 bytes/sector\n");
		break;
	case F8_256_128:
		OutputDiscLogA("8, 256KB, 128 bytes/sector\n");
		break;
	case F3_200Mb_512:
		OutputDiscLogA("3.5, 200M Floppy (HiFD)\n");
		break;
	case F3_240M_512:
		OutputDiscLogA("3.5, 240Mb Floppy (HiFD)\n");
		break;
	case F3_32M_512:
		OutputDiscLogA("3.5, 32Mb Floppy\n");
		break;
	default:
		OutputDiscLogA("Unknown media type\n");
		break;
	}
}

VOID OutputDiskGeometry(
	PDISK_GEOMETRY pGeom,
	DWORD dwGeomNum
) {
	if (dwGeomNum > 1) {
		OutputDiscLogA(
			OUTPUT_DHYPHEN_PLUS_STR(DISK_GEOMETRY)
			"SupportedMediaType\n");
	}
	else if (dwGeomNum == 1) {
		OutputDiscLogA("CurrentMediaType\n");
	}
	for (DWORD i = 0; i < dwGeomNum; i++) {
		OutputDiscLogA("\t        MediaType: ");
		OutputMediaType(pGeom[i].MediaType);
		DWORD dwDiskSize = pGeom[i].Cylinders.u.LowPart * pGeom[i].TracksPerCylinder *
			pGeom[i].SectorsPerTrack * pGeom[i].BytesPerSector;
		OutputDiscLogA(
			"\t        Cylinders: %lu\n"
			"\tTracksPerCylinder: %lu\n"
			"\t  SectorsPerTrack: %lu\n"
			"\t   BytesPerSector: %lu (Bytes)\n"
			"\t--------------------------\n"
			"\t         DiskSize: %lu (Bytes)\n\n"
			, pGeom[i].Cylinders.u.LowPart
			, pGeom[i].TracksPerCylinder
			, pGeom[i].SectorsPerTrack
			, pGeom[i].BytesPerSector
			, dwDiskSize);
	}
}

VOID OutputDiskGeometryEx(
	PDISK_GEOMETRY_EX pGeom
) {
	PDISK_PARTITION_INFO partition = DiskGeometryGetPartition(pGeom);
	PDISK_DETECTION_INFO detection = DiskGeometryGetDetect(pGeom);
	OutputDiscLogA(
		OUTPUT_DHYPHEN_PLUS_STR(DISK_GEOMETRY_EX)
		"\t            DiskSize: %llu (Bytes)\n"
		"\t SizeOfPartitionInfo: %lu\n"
		"\t      PartitionStyle: %u\n"
		, pGeom->DiskSize.QuadPart
		, partition->SizeOfPartitionInfo
		, partition->PartitionStyle
	);
	if (partition->SizeOfPartitionInfo) {
		switch (partition->PartitionStyle) {
		case PARTITION_STYLE_MBR:
#ifdef _WIN32
			OutputDiscLogA(
				"\t           Signiture: %lu\n"
				"\t            CheckSum: %lu\n"
				, partition->Mbr.Signature
				, partition->Mbr.CheckSum
			);
#else
			OutputDiscLogA(
				"\t           Signiture: %lu\n"
				"\t            CheckSum: %lu\n"
				, partition->DUMMYUNIONNAME.Mbr.Signature
				, partition->DUMMYUNIONNAME.Mbr.CheckSum
			);
#endif
			break;
		case PARTITION_STYLE_GPT:
#ifdef _WIN32
			OutputDiscLogA(
				"\t        DiskId.Data1: %lu\n"
				"\t        DiskId.Data2: %u\n"
				"\t        DiskId.Data3: %u\n"
				"\t        DiskId.Data4: %02x%02x%02x%02x%02x%02x%02x%02x\n"
				, partition->Gpt.DiskId.Data1
				, partition->Gpt.DiskId.Data2
				, partition->Gpt.DiskId.Data3
				, partition->Gpt.DiskId.Data4[0], partition->Gpt.DiskId.Data4[1]
				, partition->Gpt.DiskId.Data4[2], partition->Gpt.DiskId.Data4[3]
				, partition->Gpt.DiskId.Data4[4], partition->Gpt.DiskId.Data4[5]
				, partition->Gpt.DiskId.Data4[6], partition->Gpt.DiskId.Data4[7]
			);
#else
			OutputDiscLogA(
				"\t        DiskId.Data1: %lu\n"
				"\t        DiskId.Data2: %u\n"
				"\t        DiskId.Data3: %u\n"
				"\t        DiskId.Data4: %02x%02x%02x%02x%02x%02x%02x%02x\n"
				, partition->DUMMYUNIONNAME.Gpt.DiskId.Data1
				, partition->DUMMYUNIONNAME.Gpt.DiskId.Data2
				, partition->DUMMYUNIONNAME.Gpt.DiskId.Data3
				, partition->DUMMYUNIONNAME.Gpt.DiskId.Data4[0], partition->DUMMYUNIONNAME.Gpt.DiskId.Data4[1]
				, partition->DUMMYUNIONNAME.Gpt.DiskId.Data4[2], partition->DUMMYUNIONNAME.Gpt.DiskId.Data4[3]
				, partition->DUMMYUNIONNAME.Gpt.DiskId.Data4[4], partition->DUMMYUNIONNAME.Gpt.DiskId.Data4[5]
				, partition->DUMMYUNIONNAME.Gpt.DiskId.Data4[6], partition->DUMMYUNIONNAME.Gpt.DiskId.Data4[7]
			);
#endif
			break;
		case PARTITION_STYLE_RAW:
			break;
		default:
			break;
		}
		OutputDiscLogA(
			"\t    SizeOfDetectInfo: %lu\n"
			"\t       DetectionType: %u\n"
			, detection->SizeOfDetectInfo
			, detection->DetectionType
		);
	}
	if (detection->SizeOfDetectInfo) {
		switch (detection->DetectionType) {
		case DetectNone:
			break;
		case DetectInt13:
#ifdef _WIN32
			OutputDiscLogA(
				"\t         DriveSelect: %u\n"
				"\t        MaxCylinders: %lu\n"
				"\t     SectorsPerTrack: %u\n"
				"\t            MaxHeads: %u\n"
				"\t        NumberDrives: %u\n"
				, detection->Int13.DriveSelect
				, detection->Int13.MaxCylinders
				, detection->Int13.SectorsPerTrack
				, detection->Int13.MaxHeads
				, detection->Int13.NumberDrives
			);
#else
			OutputDiscLogA(
				"\t         DriveSelect: %u\n"
				"\t        MaxCylinders: %lu\n"
				"\t     SectorsPerTrack: %u\n"
				"\t            MaxHeads: %u\n"
				"\t        NumberDrives: %u\n"
				, detection->DUMMYUNIONNAME.DUMMYSTRUCTNAME.Int13.DriveSelect
				, detection->DUMMYUNIONNAME.DUMMYSTRUCTNAME.Int13.MaxCylinders
				, detection->DUMMYUNIONNAME.DUMMYSTRUCTNAME.Int13.SectorsPerTrack
				, detection->DUMMYUNIONNAME.DUMMYSTRUCTNAME.Int13.MaxHeads
				, detection->DUMMYUNIONNAME.DUMMYSTRUCTNAME.Int13.NumberDrives
			);
#endif
			break;
		case DetectExInt13:
#ifdef _WIN32
			OutputDiscLogA(
				"\t        ExBufferSize: %u\n"
				"\t             ExFlags: %u\n"
				"\t         ExCylinders: %lu\n"
				"\t             ExHeads: %lu\n"
				"\t   ExSectorsPerTrack: %lu\n"
				"\t   ExSectorsPerDrive: %llu\n"
				"\t        ExSectorSize: %u\n"
				"\t          ExReserved: %u\n"
				, detection->ExInt13.ExBufferSize
				, detection->ExInt13.ExFlags
				, detection->ExInt13.ExCylinders
				, detection->ExInt13.ExHeads
				, detection->ExInt13.ExSectorsPerTrack
				, detection->ExInt13.ExSectorsPerDrive
				, detection->ExInt13.ExSectorSize
				, detection->ExInt13.ExReserved
			);
#else
			OutputDiscLogA(
				"\t        ExBufferSize: %u\n"
				"\t             ExFlags: %u\n"
				"\t         ExCylinders: %lu\n"
				"\t             ExHeads: %lu\n"
				"\t   ExSectorsPerTrack: %lu\n"
				"\t   ExSectorsPerDrive: %llu\n"
				"\t        ExSectorSize: %u\n"
				"\t          ExReserved: %u\n"
				, detection->DUMMYUNIONNAME.DUMMYSTRUCTNAME.ExInt13.ExBufferSize
				, detection->DUMMYUNIONNAME.DUMMYSTRUCTNAME.ExInt13.ExFlags
				, detection->DUMMYUNIONNAME.DUMMYSTRUCTNAME.ExInt13.ExCylinders
				, detection->DUMMYUNIONNAME.DUMMYSTRUCTNAME.ExInt13.ExHeads
				, detection->DUMMYUNIONNAME.DUMMYSTRUCTNAME.ExInt13.ExSectorsPerTrack
				, detection->DUMMYUNIONNAME.DUMMYSTRUCTNAME.ExInt13.ExSectorsPerDrive
				, detection->DUMMYUNIONNAME.DUMMYSTRUCTNAME.ExInt13.ExSectorSize
				, detection->DUMMYUNIONNAME.DUMMYSTRUCTNAME.ExInt13.ExReserved
			);
#endif
			break;
		default:
			break;
		}
	}

}

VOID OutputRemovableDiskInfo(
	PGET_MEDIA_TYPES pMedia
) {
	OutputDiscLogA(
		OUTPUT_DHYPHEN_PLUS_STR(DEVICE_MEDIA_INFO)
		"\t          DeviceType: %ld\n"
		"\t      MediaInfoCount: %ld\n"
		, pMedia->DeviceType, pMedia->MediaInfoCount
	);
	for (DWORD i = 0; i < pMedia->MediaInfoCount; i++) {
		OutputDiscLogA(
			"\t           Cylinders: %ld\n"
			"\t           MediaType: "
			, pMedia->MediaInfo[i].DeviceSpecific.RemovableDiskInfo.Cylinders.u.LowPart
		);
		STORAGE_MEDIA_TYPE mType = pMedia->MediaInfo[i].DeviceSpecific.RemovableDiskInfo.MediaType;
		if (mType < DDS_4mm) {
			OutputMediaType((MEDIA_TYPE)mType);
		}
		else {
			// TODO
			OutputDiscLogA("Other media\n");
		}

		OutputDiscLogA(
			"\t   TracksPerCylinder: %ld\n"
			"\t     SectorsPerTrack: %ld\n"
			"\t      BytesPerSector: %ld\n"
			"\t    NumberMediaSides: %ld\n"
			"\tMediaCharacteristics: %lx\n"
			, pMedia->MediaInfo[i].DeviceSpecific.RemovableDiskInfo.TracksPerCylinder
			, pMedia->MediaInfo[i].DeviceSpecific.RemovableDiskInfo.SectorsPerTrack
			, pMedia->MediaInfo[i].DeviceSpecific.RemovableDiskInfo.BytesPerSector
			, pMedia->MediaInfo[i].DeviceSpecific.RemovableDiskInfo.NumberMediaSides
			, pMedia->MediaInfo[i].DeviceSpecific.RemovableDiskInfo.MediaCharacteristics
		);
	}
}

VOID OutputFileAllocationTable(
	LPBYTE lpBuf,
	PFAT fat
) {
	if ((lpBuf[0] == 0xeb && lpBuf[2] == 0x90) || lpBuf[0] == 0xe9) {
		WORD BytsPerSec = MAKEWORD(lpBuf[11], lpBuf[12]);
		fat->SecPerClus = lpBuf[13];
		WORD RsvdSecCnt = MAKEWORD(lpBuf[14], lpBuf[15]);
		BYTE NumFATs = lpBuf[16];
		fat->RootEntCnt = MAKEWORD(lpBuf[17], lpBuf[18]);
		UINT FATSz = MAKEWORD(lpBuf[22], lpBuf[23]);
		UINT TotSec16 = MAKEWORD(lpBuf[19], lpBuf[20]);
		UINT TotSec32 = MAKEUINT(MAKEWORD(lpBuf[32], lpBuf[33]), MAKEWORD(lpBuf[34], lpBuf[35]));
		OutputDiscLogA(
			OUTPUT_DHYPHEN_PLUS_STR(FileAllocationTable)
			"\t        BS_JmpBoot: %#02x %#02x %#02x\n"
			"\t        BS_OEMName: %.8s\n"
			"\t    BPB_BytsPerSec: %d\n"
			"\t    BPB_SecPerClus: %d\n"
			"\t    BPB_RsvdSecCnt: %d\n"
			"\t       BPB_NumFATs: %d\n"
			"\t    BPB_RootEntCnt: %d\n"
			"\t      BPB_TotSec16: %d\n"
			"\t         BPB_Media: %#x\n"
			"\t       BPB_FATSz16: %d\n"
			"\t     BPB_SecPerTrk: %d\n"
			"\t      BPB_NumHeads: %d\n"
			"\t       BPB_HiddSec: %d\n"
			"\t      BPB_TotSec32: %d\n"
			, lpBuf[0], lpBuf[1], lpBuf[2]
			, (LPCH)&lpBuf[3]
			, BytsPerSec
			, fat->SecPerClus
			, RsvdSecCnt
			, NumFATs
			, fat->RootEntCnt
			, TotSec16
			, lpBuf[21]
			, FATSz
			, MAKEWORD(lpBuf[24], lpBuf[25])
			, MAKEWORD(lpBuf[26], lpBuf[27])
			, MAKEUINT(MAKEWORD(lpBuf[28], lpBuf[29]), MAKEWORD(lpBuf[30], lpBuf[31]))
			, TotSec32
		);
		INT i = 36;
		if (fat->RootEntCnt == 0 && FATSz == 0) {
			FATSz = MAKEUINT(MAKEWORD(lpBuf[36], lpBuf[37]), MAKEWORD(lpBuf[38], lpBuf[39]));
			OutputDiscLogA(
				"\t       BPB_FATSz32: %d\n"
				"\t      BPB_ExtFlags: %#04x\n"
				"\t         BPB_FSVer: %#04x\n"
				"\t      BPB_RootClus: %d\n"
				"\t        BPB_FSInfo: %d\n"
				"\t     BPB_BkBootSec: %d\n"
				, FATSz
				, MAKEWORD(lpBuf[40], lpBuf[41])
				, MAKEWORD(lpBuf[42], lpBuf[43])
				, MAKEUINT(MAKEWORD(lpBuf[44], lpBuf[45]), MAKEWORD(lpBuf[46], lpBuf[47]))
				, MAKEWORD(lpBuf[48], lpBuf[49])
				, MAKEWORD(lpBuf[50], lpBuf[51])
			);
			i = 64;
		}
		OutputDiscLogA(
			"\t         BS_DrvNum: %#x\n"
			"\t      BS_Reserved1: %d\n"
			"\t        BS_BootSig: %#x\n"
			"\t          BS_VolID: %#08x\n"
			"\t         BS_VolLab: %.11s\n"
			"\t     BS_FilSysType: %.8s\n"
			"\t       BS_BootCode: "
			, lpBuf[i]
			, lpBuf[i + 1]
			, lpBuf[i + 2]
			, MAKEUINT(MAKEWORD(lpBuf[i + 3], lpBuf[i + 4]), MAKEWORD(lpBuf[i + 5], lpBuf[i + 6]))
			, (LPCH)&lpBuf[i + 7]
			, (LPCH)&lpBuf[i + 18]
		);
		for (INT j = i + 26; j < 510; j++) {
			OutputDiscLogA("%02x ", lpBuf[j]);
		}
		OutputDiscLogA(
			"\n"
			"\t       BS_BootSign: %#x\n"
			, MAKEWORD(lpBuf[510], lpBuf[511])
		);

		WORD FatStartSector = RsvdSecCnt;
		UINT FatSectorSize = FATSz * NumFATs;
		fat->RootDirStartSector = FatStartSector + FatSectorSize;
		UINT RootDirSectorSize = (UINT)(32 * fat->RootEntCnt + BytsPerSec - 1) / BytsPerSec;
		fat->DataStartSector = fat->RootDirStartSector + RootDirSectorSize;

		UINT TotSec = TotSec16 != 0 ? TotSec16 : TotSec32;
		UINT DataSectorSize = TotSec - fat->DataStartSector;
		UINT CountofClusters = DataSectorSize / fat->SecPerClus;
		OutputDiscLogA(
			"\t    FatStartSector: %d\n"
			"\t     FatSectorSize: %d\n"
			"\tRootDirStartSector: %d\n"
			"\t RootDirSectorSize: %d\n"
			"\t   DataStartSector: %d\n"
			"\t    DataSectorSize: %d\n"
			"\t   CountofClusters: %d "
			, FatStartSector, FatSectorSize
			, fat->RootDirStartSector, RootDirSectorSize
			, fat->DataStartSector, DataSectorSize
			, CountofClusters
		);
		if (CountofClusters <= 4085) {
			OutputDiscLogA(" => FAT12\n");
		}
		else if (4086 <= CountofClusters && CountofClusters <= 65525) {
			OutputDiscLogA(" => FAT16\n");
		}
		else {
			OutputDiscLogA(" => FAT32\n");
		}
	}
}

VOID OutputDVDGetRegion(
	PDVD_REGION dvdRegion
) {
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(DVD Region)
		"\tSystemRegion: %d\n"
		"\t  ResetCount: %d\n"
		, dvdRegion->SystemRegion
		, dvdRegion->ResetCount
	);
}
