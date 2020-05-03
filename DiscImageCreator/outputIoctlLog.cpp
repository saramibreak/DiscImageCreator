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
#include "get.h"
#include "output.h"
#include "outputIoctlLog.h"

VOID OutputAdditionalSenseCodeQualifierOther(
	BYTE byCode
) {
	if (byCode >= SCSI_ADSENSE_VENDOR_UNIQUE) {
		OutputLog(standardError | fileMainError, "VENDOR UNIQUE ERROR");
	}
	else {
		OutputLog(standardError | fileMainError, "OTHER");
	}
}

VOID OutputAdditionalSenseCodeQualifier0x00(
	BYTE byAscq
) {
	switch (byAscq) {
	case 0x00:
		OutputLog(standardError | fileMainError, "NO ADDITIONAL SENSE INFORMATION");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "I/O PROCESS TERMINATED");
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, "AUDIO PLAY OPERATION IN PROGRES");
		break;
	case 0x12:
		OutputLog(standardError | fileMainError, "AUDIO PLAY OPERATION PAUSED");
		break;
	case 0x13:
		OutputLog(standardError | fileMainError, "AUDIO PLAY OPERATION SUCCESSFULLY COMPLETED");
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, "AUDIO PLAY OPERATION STOPPED DUE TO ERROR");
		break;
	case 0x15:
		OutputLog(standardError | fileMainError, "NO CURRENT AUDIO STATUS TO RETURN");
		break;
	case 0x16:
		OutputLog(standardError | fileMainError, "OPERATION IN PROGRESS");
		break;
	case 0x17:
		OutputLog(standardError | fileMainError, "CLEANING REQUESTED");
		break;
	case 0x1d:
		OutputLog(standardError | fileMainError, "ATA PASS THROUGH INFORMATION AVAILABLE");
		break;
	case 0x1e:
		OutputLog(standardError | fileMainError, "CONFLICTING SA CREATION REQUE");
		break;
	case 0x1f:
		OutputLog(standardError | fileMainError, "LOGICAL UNIT TRANSITIONING TO ANOTHER POWER CONDITION");
		break;
	case 0x20:
		OutputLog(standardError | fileMainError, "EXTENDED COPY INFORMATION AVAILABLE");
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
		OutputLog(standardError | fileMainError, "NO INDEX/SECTOR SIGNAL");
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
		OutputLog(standardError | fileMainError, "NO SEEK COMPLETE");
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
		OutputLog(standardError | fileMainError, "PERIPHERAL DEVICE WRITE FAULT");
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
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - CAUSE_NOT_REPORTABLE");
		break;
	case SCSI_SENSEQ_BECOMING_READY:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - BECOMING_READY");
		break;
	case SCSI_SENSEQ_INIT_COMMAND_REQUIRED:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - INIT_COMMAND_REQUIRED");
		break;
	case SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - MANUAL_INTERVENTION_REQUIRED");
		break;
	case SCSI_SENSEQ_FORMAT_IN_PROGRESS:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - FORMAT_IN_PROGRESS");
		break;
	case SCSI_SENSEQ_REBUILD_IN_PROGRESS:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - REBUILD_IN_PROGRESS");
		break;
	case SCSI_SENSEQ_RECALCULATION_IN_PROGRESS:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - RECALCULATION_IN_PROGRESS");
		break;
	case SCSI_SENSEQ_OPERATION_IN_PROGRESS:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - OPERATION_IN_PROGRESS");
		break;
	case SCSI_SENSEQ_LONG_WRITE_IN_PROGRESS:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - LONG_WRITE_IN_PROGRESS");
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - SELF-TEST IN PROGRESS");
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - ASYMMETRIC ACCESS STATE TRANSITION");
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - TARGET PORT IN STANDBY STATE");
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - TARGET PORT IN UNAVAILABLE STATE");
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - AUXILIARY MEMORY NOT ACCESSIBLE");
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - NOTIFY (ENABLE SPINUP) REQUIRED");
		break;
	case 0x13:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - SA CREATION IN PROGRESS");
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - SPACE ALLOCATION IN PROGRESS");
		break;
	case 0x1a:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - START STOP UNIT COMMAND IN PROGRESS");
		break;
	case 0x1b:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - SANITIZE IN PROGRESS");
		break;
	case 0x1c:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - ADDITIONAL POWER USE NOT YET GRANTED");
		break;
	case 0x1d:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - CONFIGURATION IN PROGRESS");
		break;
	case 0x1e:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - MICROCODE ACTIVATION REQUIRED");
		break;
	case 0x1f:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - MICROCODE DOWNLOAD REQUIRED");
		break;
	case 0x20:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - LOGICAL UNIT RESET REQUIRED");
		break;
	case 0x21:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - HARD RESET REQUIRED");
		break;
	case 0x22:
		OutputLog(standardError | fileMainError, "LUN_NOT_READY - POWER CYCLE REQUIRED");
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
		OutputLog(standardError | fileMainError, "LOGICAL UNIT DOES NOT RESPOND TO SELECTION");
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
		OutputLog(standardError | fileMainError, "NO REFERENCE POSITION FOUND");
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
		OutputLog(standardError | fileMainError, "MULTIPLE PERIPHERAL DEVICES SELECTED");
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
		OutputLog(standardError | fileMainError, "LUN_COMMUNICATION - COMM_FAILURE");
		break;
	case SCSI_SENSEQ_COMM_TIMEOUT:
		OutputLog(standardError | fileMainError, "LUN_COMMUNICATION - COMM_TIMEOUT");
		break;
	case SCSI_SENSEQ_COMM_PARITY_ERROR:
		OutputLog(standardError | fileMainError, "LUN_COMMUNICATION - COMM_PARITY_ERROR");
		break;
	case SCSI_SESNEQ_COMM_CRC_ERROR:
		OutputLog(standardError | fileMainError, "LUN_COMMUNICATION - COMM_CRC_ERROR");
		break;
	case SCSI_SENSEQ_UNREACHABLE_TARGET:
		OutputLog(standardError | fileMainError, "LUN_COMMUNICATION - UNREACHABLE_TARGET");
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
		OutputLog(standardError | fileMainError, "TRACK FOLLOWING ERROR");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "TRACKING SERVO FAILURE");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "FOCUS SERVO FAILURE");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "SPINDLE SERVO FAILURE");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "HEAD SELECT FAULT");
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
		OutputLog(standardError | fileMainError, "ERROR LOG OVERFLOW");
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
		OutputLog(standardError | fileMainError, "WARNING");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "WARNING - SPECIFIED TEMPERATURE EXCEEDED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "WARNING - ENCLOSURE DEGRADED");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "WARNING - BACKGROUND SELF-TEST FAILED");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "WARNING - BACKGROUND PRE-SCAN DETECTED MEDIUM ERROR");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "WARNING - BACKGROUND MEDIUM SCAN DETECTED MEDIUM ERROR");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "WARNING - NON-VOLATILE CACHE NOW VOLATILE");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "WARNING - DEGRADED POWER TO NON-VOLATILE CACHE");
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, "WARNING - POWER LOSS EXPECTED");
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, "WARNING - DEVICE STATISTICS NOTIFICATION ACTIVE");
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
		OutputLog(standardError | fileMainError, "WRITE ERROR");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "WRITE ERROR - AUTO REALLOCATION FAILED");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "WRITE ERROR - RECOMMEND REASSIGNMENT");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "WRITE ERROR - COMPRESSION CHECK MISCOMPARE ERROR");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "WRITE ERROR - DATA EXPANSION OCCURRED DURING COMPRESSION");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "WRITE ERROR - BLOCK NOT COMPRESSIBLE");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "WRITE ERROR - RECOVERY NEEDED");
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, "WRITE ERROR - RECOVERY FAILED");
		break;
	case SCSI_SENSEQ_LOSS_OF_STREAMING:
		OutputLog(standardError | fileMainError, "WRITE ERROR - LOSS OF STREAMING");
		break;
	case SCSI_SENSEQ_PADDING_BLOCKS_ADDED:
		OutputLog(standardError | fileMainError, "WRITE ERROR - PADDING BLOCKS ADDED");
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, "WRITE ERROR - AUXILIARY MEMORY WRITE ERROR");
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, "WRITE ERROR - UNEXPECTED UNSOLICITED DATA");
		break;
	case 0x0d:
		OutputLog(standardError | fileMainError, "WRITE ERROR - NOT ENOUGH UNSOLICITED DATA");
		break;
	case 0x0f:
		OutputLog(standardError | fileMainError, "WRITE ERROR - DEFECTS IN ERROR WINDOW");
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
		OutputLog(standardError | fileMainError, "ERROR DETECTED BY THIRD PARTY TEMPORARY INITIATOR");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "THIRD PARTY DEVICE FAILURE");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "COPY TARGET DEVICE NOT REACHABLE");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "INCORRECT COPY TARGET DEVICE TYPE");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "COPY TARGET DEVICE DATA UNDERRUN");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "COPY TARGET DEVICE DATA OVERRUN");
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
		OutputLog(standardError | fileMainError, "INVALID INFORMATION UNIT");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "INFORMATION UNIT TOO SHORT");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "INFORMATION UNIT TOO LONG");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "INVALID FIELD IN COMMAND INFORMATION UNIT");
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
		OutputLog(standardError | fileMainError, "ID CRC OR ECC ERROR");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "LOGICAL BLOCK GUARD CHECK FAIL");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "LOGICAL BLOCK APPLICATION TAG CHECK FAILED");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "LOGICAL BLOCK REFERENCE TAG CHECK FAILED");
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
		OutputLog(standardError | fileMainError, "UNRECOVERED READ ERROR");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "READ RETRIES EXHAUSTED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "ERROR TOO LONG TO CORRECT");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "MULTIPLE READ ERRORS");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "UNRECOVERED READ ERROR - AUTO REALLOCATE FAILED");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "L-EC UNCORRECTABLE ERROR");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "CIRC UNRECOVERED ERROR");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "DATA RE-SYNCHRONIZATION ERROR");
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, "MISCORRECTED ERROR");
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, "UNRECOVERED READ ERROR - RECOMMEND REASSIGNMENT");
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, "UNRECOVERED READ ERROR - RECOMMEND REWRITE THE DATA");
		break;
	case 0x0d:
		OutputLog(standardError | fileMainError, "DE-COMPRESSION CRC ERROR");
		break;
	case 0x0e:
		OutputLog(standardError | fileMainError, "CANNOT DECOMPRESS USING DECLARED ALGORITHM");
		break;
	case 0x0f:
		OutputLog(standardError | fileMainError, "ERROR READING UPC/EAN NUMBER");
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, "ERROR READING ISRC NUMBER");
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, "READ ERROR - LOSS OF STREAMING");
		break;
	case 0x12:
		OutputLog(standardError | fileMainError, "AUXILIARY MEMORY READ ERROR");
		break;
	case 0x13:
		OutputLog(standardError | fileMainError, "READ ERROR - FAILED RETRANSMISSION REQUEST");
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, "READ ERROR - LBA MARKED BAD BY APPLICATION CLIENT");
		break;
	case 0x15:
		OutputLog(standardError | fileMainError, "WRITE AFTER SANITIZE REQUIRED");
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
		OutputLog(standardError | fileMainError, "ADDRESS MARK NOT FOUND FOR ID FIELD");
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
		OutputLog(standardError | fileMainError, "ADDRESS MARK NOT FOUND FOR DATA FIELD");
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
		OutputLog(standardError | fileMainError, "TRACK_ERROR - RECORDED ENTITY NOT FOUND");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "TRACK_ERROR - RECORD NOT FOUND");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "TRACK_ERROR - RECORD NOT FOUND - RECOMMEND REASSIGNMENT");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "TRACK_ERROR - RECORD NOT FOUND - DATA AUTO-REALLOCATED");
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
		OutputLog(standardError | fileMainError, "SEEK_ERROR - RANDOM POSITIONING ERROR");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "SEEK_ERROR - MECHANICAL POSITIONING ERROR");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "SEEK_ERROR - POSITIONING ERROR DETECTED BY READ OF MEDIUM");
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
		OutputLog(standardError | fileMainError, "DATA SYNCHRONIZATION MARK ERROR");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "DATA SYNC ERROR - DATA REWRITTEN");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "DATA SYNC ERROR - RECOMMEND REWRITE");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "DATA SYNC ERROR - DATA AUTO-REALLOCATED");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "DATA SYNC ERROR - RECOMMEND REASSIGNMENT");
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
		OutputLog(standardError | fileMainError, "REC_DATA_NOECC - RECOVERED DATA WITH NO ERROR CORRECTION APPLIED");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "REC_DATA_NOECC - RECOVERED DATA WITH RETRIES");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "REC_DATA_NOECC - RECOVERED DATA WITH POSITIVE HEAD OFFSET");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "REC_DATA_NOECC - RECOVERED DATA WITH NEGATIVE HEAD OFFSET");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "REC_DATA_NOECC - RECOVERED DATA WITH RETRIES AND/OR CIRC APPLIED");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "REC_DATA_NOECC - RECOVERED DATA USING PREVIOUS SECTOR ID");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - DATA AUTO-REALLOCATED");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - RECOMMEND REASSIGNMENT");
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, "REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - RECOMMEND REWRITE");
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, "REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - DATA REWRITTEN");
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
		OutputLog(standardError | fileMainError, "REC_DATA_ECC - RECOVERED DATA WITH ERROR CORRECTION APPLIED");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "REC_DATA_ECC - RECOVERED DATA WITH ERROR CORR. & RETRIES APPLIED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "REC_DATA_ECC - RECOVERED DATA - DATA AUTO-REALLOCATED");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "REC_DATA_ECC - RECOVERED DATA WITH CIRC");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "REC_DATA_ECC - RECOVERED DATA WITH L-EC");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "REC_DATA_ECC - RECOVERED DATA - RECOMMEND REASSIGNMENT");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "REC_DATA_ECC - RECOVERED DATA - RECOMMEND REWRITE");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "REC_DATA_ECC - RECOVERED DATA WITH ECC - DATA REWRITTEN");
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, "REC_DATA_ECC - RECOVERED DATA WITH LINKING");
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
		OutputLog(standardError | fileMainError, "DEFECT LIST ERROR");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "DEFECT LIST NOT AVAILABLE");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "DEFECT LIST ERROR IN PRIMARY LIST");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "DEFECT LIST ERROR IN GROWN LIST");
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
		OutputLog(standardError | fileMainError, "PARAMETER LIST LENGTH ERROR");
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
		OutputLog(standardError | fileMainError, "SYNCHRONOUS DATA TRANSFER ERROR");
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
		OutputLog(standardError | fileMainError, "DEFECT LIST NOT FOUND");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "PRIMARY DEFECT LIST NOT FOUND");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "GROWN DEFECT LIST NOT FOUND");
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
		OutputLog(standardError | fileMainError, "MISCOMPARE DURING VERIFY OPERATION");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "MISCOMPARE VERIFY OF UNMAPPED LBA");
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
		OutputLog(standardError | fileMainError, "RECOVERED ID WITH ECC CORRECTION");
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
		OutputLog(standardError | fileMainError, "PARTIAL DEFECT LIST TRANSFER");
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
		OutputLog(standardError | fileMainError, "INVALID COMMAND OPERATION CODE");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "ILLEGAL_COMMAND - ACCESS DENIED - INITIATOR PENDING-ENROLLED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "ILLEGAL_COMMAND - ACCESS DENIED - NO ACCESS RIGHTS");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "ILLEGAL_COMMAND - ACCESS DENIED - INVALID MGMT ID KEY");
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, "ILLEGAL_COMMAND - ACCESS DENIED - ENROLLMENT CONFLICT");
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, "ILLEGAL_COMMAND - ACCESS DENIED - INVALID LU IDENTIFIER");
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, "ILLEGAL_COMMAND - ACCESS DENIED - INVALID PROXY TOKEN");
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, "ILLEGAL_COMMAND - ACCESS DENIED - ACL LUN CONFLICT");
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
		OutputLog(standardError | fileMainError, "LOGICAL BLOCK ADDRESS OUT OF RANGE");
		break;
	case SCSI_SENSEQ_ILLEGAL_ELEMENT_ADDR:
		OutputLog(standardError | fileMainError, "ILLEGAL_BLOCK - MISCOMPARE DURING VERIFY OPERATION");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "ILLEGAL_BLOCK - INVALID ADDRESS FOR WRITE");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "ILLEGAL_BLOCK - INVALID WRITE CROSSING LAYER JUMP");
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
		OutputLog(standardError | fileMainError, "ILLEGAL FUNCTION");
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
		OutputLog(standardError | fileMainError, "INVALID TOKEN OPERATION, CAUSE NOT REPORTABLE");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "INVALID TOKEN OPERATION, UNSUPPORTED TOKEN TYPE");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "INVALID TOKEN OPERATION, REMOTE TOKEN USAGE NOT SUPPORTED");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "INVALID TOKEN OPERATION, REMOTE ROD TOKEN CREATION NOT SUPPORTED");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "INVALID TOKEN OPERATION, TOKEN UNKNOWN");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "INVALID TOKEN OPERATION, TOKEN CORRUPT");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "INVALID TOKEN OPERATION, TOKEN REVOKED");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "INVALID TOKEN OPERATION, TOKEN EXPIRED");
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, "INVALID TOKEN OPERATION, TOKEN CANCELLED");
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, "INVALID TOKEN OPERATION, TOKEN DELETED");
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, "INVALID TOKEN OPERATION, INVALID TOKEN LENGTH");
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
		OutputLog(standardError | fileMainError, "INVALID FIELD IN CDB");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "CDB DECRYPTION ERROR");
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, "INVALID XCDB");
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
		OutputLog(standardError | fileMainError, "LOGICAL UNIT NOT SUPPORTED");
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
		OutputLog(standardError | fileMainError, "INVALID FIELD IN PARAMETER LIST");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "PARAMETER NOT SUPPORTED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "PARAMETER VALUE INVALID");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "THRESHOLD PARAMETERS NOT SUPPORTED");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "INVALID RELEASE OF PERSISTENT RESERVATION");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "DATA DECRYPTION ERROR");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "TOO MANY TARGET DESCRIPTORS");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "UNSUPPORTED SEGMENT DESCRIPTOR TYPE CODE");
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, "TOO MANY SEGMENT DESCRIPTORS");
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, "UNSUPPORTED SEGMENT DESCRIPTOR TYPE CODE");
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, "UNEXPECTED INEXACT SEGMENT");
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, "INLINE DATA LENGTH EXCEEDED");
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, "INVALID OPERATION FOR COPY SOURCE OR DESTINATION");
		break;
	case 0x0d:
		OutputLog(standardError | fileMainError, "COPY SEGMENT GRANULARITY VIOLATION");
		break;
	case 0x0e:
		OutputLog(standardError | fileMainError, "INVALID PARAMETER WHILE PORT IS ENABLED");
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
		OutputLog(standardError | fileMainError, "WRITE PROTECTED");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "HARDWARE WRITE PROTECTED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "LOGICAL UNIT SOFTWARE WRITE PROTECTED");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "ASSOCIATED WRITE PROTECT");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "PERSISTENT WRITE PROTECT");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "PERMANENT WRITE PROTECT");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "CONDITIONAL WRITE PROTECT");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "SPACE ALLOCATION FAILED WRITE PROTECT");
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
		OutputLog(standardError | fileMainError, "NOT READY TO READY CHANGE, MEDIUM MAY HAVE CHANGED");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "IMPORT OR EXPORT ELEMENT ACCESSED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "FORMAT-LAYER MAY HAVE CHANGED");
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
		OutputLog(standardError | fileMainError, "POWER ON, RESET, OR BUS DEVICE RESET OCCURRED");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "POWER ON OCCURRED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "SCSI BUS RESET OCCURRED");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "BUS DEVICE RESET FUNCTION OCCURRED");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "DEVICE INTERNAL RESET");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "TRANSCEIVER MODE CHANGED TO SINGLE-ENDED");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "TRANSCEIVER MODE CHANGED TO LVD");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "I_T NEXUS LOSS OCCURRED");
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
		OutputLog(standardError | fileMainError, "PARAMETERS CHANGED");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "MODE PARAMETERS CHANGED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "LOG PARAMETERS CHANGED");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "RESERVATIONS PREEMPTED");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "RESERVATIONS RELEASED");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "REGISTRATIONS PREEMPTED");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "ASYMMETRIC ACCESS STATE CHANGED");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "IMPLICIT ASYMMETRIC ACCESS STATE TRANSITION FAILED");
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, "PRIORITY CHANGED");
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, "CAPACITY DATA HAS CHANGED");
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, "ERROR HISTORY I_T NEXUS CLEARED");
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, "ERROR HISTORY SNAPSHOT RELEASED");
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, "TIMESTAMP CHANGED");
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, "SA CREATION CAPABILITIES DATA HAS CHANGED");
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
		OutputLog(standardError | fileMainError, "COPY CANNOT EXECUTE SINCE HOST CANNOT DISCONNECT");
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
		OutputLog(standardError | fileMainError, "COMMAND SEQUENCE ERROR");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "CURRENT PROGRAM AREA IS NOT EMPTY");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "CURRENT PROGRAM AREA IS EMPTY");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "PERSISTENT PREVENT CONFLICT");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "PREVIOUS BUSY STATUS");
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, "PREVIOUS TASK SET FULL STATUS");
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, "PREVIOUS RESERVATION CONFLICT STATUS");
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, "ORWRITE GENERATION DOES NOT MATCH");
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
		OutputLog(standardError | fileMainError, "INSUFFICIENT TIME FOR OPERATION");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "COMMAND TIMEOUT BEFORE PROCESSING");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "COMMAND TIMEOUT DURING PROCESSING");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "COMMAND TIMEOUT DURING PROCESSING DUE TO ERROR RECOVERY");
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
		OutputLog(standardError | fileMainError, "COMMANDS CLEARED BY ANOTHER INITIATOR");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "COMMANDS CLEARED BY POWER LOSS NOTIFICATION");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "COMMANDS CLEARED BY DEVICE SERVER");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "SOME COMMANDS CLEARED BY QUEUING LAYER EVENT");
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
		OutputLog(standardError | fileMainError, "INCOMPATIBLE MEDIUM INSTALLED");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "CANNOT READ MEDIUM - UNKNOWN FORMAT");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "CANNOT READ MEDIUM - INCOMPATIBLE FORMAT");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "CLEANING CARTRIDGE INSTALLED");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "CANNOT WRITE MEDIUM - UNKNOWN FORMAT");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "CANNOT WRITE MEDIUM - INCOMPATIBLE FORMAT");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "CANNOT FORMAT MEDIUM - INCOMPATIBLE MEDIUM");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "CLEANING FAILURE");
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, "CANNOT WRITE - APPLICATION CODE MISMATCH");
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, "CURRENT SESSION NOT FIXATED FOR APPEND");
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, "CLEANING REQUEST REJECTED");
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, "MEDIUM NOT FORMATTED");
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
		OutputLog(standardError | fileMainError, "MEDIUM FORMAT CORRUPTED");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "FORMAT COMMAND FAILED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "ZONED FORMATTING FAILED DUE TO SPARE LINKING");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "SANITIZE COMMAND FAILED");
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
		OutputLog(standardError | fileMainError, "NO DEFECT SPARE LOCATION AVAILABLE");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "DEFECT LIST UPDATE FAILURE");
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
		OutputLog(standardError | fileMainError, "ENCLOSURE FAILURE");
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
		OutputLog(standardError | fileMainError, "ENCLOSURE SERVICES FAILURE");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "UNSUPPORTED ENCLOSURE FUNCTION");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "ENCLOSURE SERVICES UNAVAILABLE");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "ENCLOSURE SERVICES TRANSFER FAILURE");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "ENCLOSURE SERVICES TRANSFER REFUSED");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "ENCLOSURE SERVICES CHECKSUM ERROR");
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
		OutputLog(standardError | fileMainError, "ROUNDED PARAMETER");
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
		OutputLog(standardError | fileMainError, "THIN PROVISIONING SOFT THRESHOLD REACHED");
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
		OutputLog(standardError | fileMainError, "SAVING PARAMETERS NOT SUPPORTED");
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
		OutputLog(standardError | fileMainError, "MEDIUM NOT PRESENT");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "MEDIUM NOT PRESENT - TRAY CLOSED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "MEDIUM NOT PRESENT - TRAY OPEN");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "MEDIUM NOT PRESENT - LOADABLE");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "MEDIUM NOT PRESENT - MEDIUM AUXILIARY MEMORY ACCESSIBLE");
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
		OutputLog(standardError | fileMainError, "MEDIUM DESTINATION ELEMENT FULL");
		break;
	case SCSI_SENSEQ_SOURCE_EMPTY:
		OutputLog(standardError | fileMainError, "MEDIUM SOURCE ELEMENT EMPTY");
		break;
	case 0x0f:
		OutputLog(standardError | fileMainError, "END OF MEDIUM REACHED");
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, "MEDIUM MAGAZINE NOT ACCESSIBLE");
		break;
	case 0x12:
		OutputLog(standardError | fileMainError, "MEDIUM MAGAZINE REMOVED");
		break;
	case 0x13:
		OutputLog(standardError | fileMainError, "MEDIUM MAGAZINE INSERTED");
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, "MEDIUM MAGAZINE LOCKED");
		break;
	case 0x15:
		OutputLog(standardError | fileMainError, "MEDIUM MAGAZINE UNLOCKED");
		break;
	case 0x16:
		OutputLog(standardError | fileMainError, "MECHANICAL POSITIONING OR CHANGER ERROR");
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
		OutputLog(standardError | fileMainError, "INVALID BITS IN IDENTIFY MESSAGE");
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
		OutputLog(standardError | fileMainError, "LOGICAL UNIT HAS NOT SELF-CONFIGURED YET");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "LOGICAL UNIT FAILURE");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "TIMEOUT ON LOGICAL UNIT");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "LOGICAL UNIT FAILED SELF-TEST");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "LOGICAL UNIT UNABLE TO UPDATE SELF-TEST LOG");
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
		OutputLog(standardError | fileMainError, "TARGET_OPERATING_CONDITIONS_CHANGED");
		break;
	case SCSI_SENSEQ_MICROCODE_CHANGED:
		OutputLog(standardError | fileMainError, "MICROCODE_CHANGED");
		break;
	case SCSI_SENSEQ_OPERATING_DEFINITION_CHANGED:
		OutputLog(standardError | fileMainError, "OPERATING_DEFINITION_CHANGED");
		break;
	case SCSI_SENSEQ_INQUIRY_DATA_CHANGED:
		OutputLog(standardError | fileMainError, "INQUIRY_DATA_CHANGED");
		break;
	case SCSI_SENSEQ_COMPONENT_DEVICE_ATTACHED:
		OutputLog(standardError | fileMainError, "COMPONENT_DEVICE_ATTACHED");
		break;
	case SCSI_SENSEQ_DEVICE_IDENTIFIER_CHANGED:
		OutputLog(standardError | fileMainError, "DEVICE_IDENTIFIER_CHANGED");
		break;
	case SCSI_SENSEQ_REDUNDANCY_GROUP_MODIFIED:
		OutputLog(standardError | fileMainError, "REDUNDANCY_GROUP_MODIFIED");
		break;
	case SCSI_SENSEQ_REDUNDANCY_GROUP_DELETED:
		OutputLog(standardError | fileMainError, "REDUNDANCY_GROUP_DELETED");
		break;
	case SCSI_SENSEQ_SPARE_MODIFIED:
		OutputLog(standardError | fileMainError, "SPARE_MODIFIED");
		break;
	case SCSI_SENSEQ_SPARE_DELETED:
		OutputLog(standardError | fileMainError, "SPARE_DELETED");
		break;
	case SCSI_SENSEQ_VOLUME_SET_MODIFIED:
		OutputLog(standardError | fileMainError, "VOLUME_SET_MODIFIED");
		break;
	case SCSI_SENSEQ_VOLUME_SET_DELETED:
		OutputLog(standardError | fileMainError, "VOLUME SET DELETED");
		break;
	case SCSI_SENSEQ_VOLUME_SET_DEASSIGNED:
		OutputLog(standardError | fileMainError, "VOLUME SET DEASSIGNED");
		break;
	case SCSI_SENSEQ_VOLUME_SET_REASSIGNED:
		OutputLog(standardError | fileMainError, "VOLUME SET REASSIGNED");
		break;
	case SCSI_SENSEQ_REPORTED_LUNS_DATA_CHANGED:
		OutputLog(standardError | fileMainError, "REPORTED_LUNS_DATA_CHANGED");
		break;
	case SCSI_SENSEQ_ECHO_BUFFER_OVERWRITTEN:
		OutputLog(standardError | fileMainError, "ECHO_BUFFER_OVERWRITTEN");
		break;
	case SCSI_SENSEQ_MEDIUM_LOADABLE:
		OutputLog(standardError | fileMainError, "MEDIUM LOADABLE");
		break;
	case SCSI_SENSEQ_MEDIUM_AUXILIARY_MEMORY_ACCESSIBLE:
		OutputLog(standardError | fileMainError, "MEDIUM_AUXILIARY_MEMORY_ACCESSIBLE");
		break;
	case 0x12:
		OutputLog(standardError | fileMainError, "iSCSI IP ADDRESS ADDED");
		break;
	case 0x13:
		OutputLog(standardError | fileMainError, "iSCSI IP ADDRESS REMOVED");
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, "iSCSI IP ADDRESS CHANGED");
		break;
	case 0x15:
		OutputLog(standardError | fileMainError, "INSPECT REFERRALS SENSE DESCRIPTORS");
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
		OutputLog(standardError | fileMainError, "RAM FAILURE");
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
		OutputLog(standardError | fileMainError, "DATA PATH FAILURE");
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
		OutputLog(standardError | fileMainError, "POWER-ON OR SELF-TEST FAILURE");
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
		OutputLog(standardError | fileMainError, "MESSAGE ERROR");
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
		OutputLog(standardError | fileMainError, "INTERNAL TARGET FAILURE");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "PERSISTENT RESERVATION INFORMATION LOST");
		break;
	case 0x71:
		OutputLog(standardError | fileMainError, "ATA DEVICE FAILED SET FEATURES");
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
		OutputLog(standardError | fileMainError, "SELECT OR RESELECT FAILURE");
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
		OutputLog(standardError | fileMainError, "UNSUCCESSFUL SOFT RESET");
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
		OutputLog(standardError | fileMainError, "SCSI PARITY ERROR");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "DATA PHASE CRC ERROR DETECTED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "SCSI PARITY ERROR DETECTED DURING ST DATA PHASE");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "INFORMATION UNIT iuCRC ERROR DETECTED");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "ASYNCHRONOUS INFORMATION PROTECTION ERROR DETECTED");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "PROTOCOL SERVICE CRC ERROR");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "PHY TEST FUNCTION IN PROGRESS");
		break;
	case 0x7f:
		OutputLog(standardError | fileMainError, "SOME COMMANDS CLEARED BY ISCSI PROTOCOL EVENT");
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
		OutputLog(standardError | fileMainError, "INITIATOR DETECTED ERROR MESSAGE RECEIVED");
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
		OutputLog(standardError | fileMainError, "INVALID MESSAGE ERROR");
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
		OutputLog(standardError | fileMainError, "COMMAND PHASE ERROR");
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
		OutputLog(standardError | fileMainError, "DATA PHASE ERROR");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "INVALID TARGET PORT TRANSFER TAG RECEIVED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "TOO MUCH WRITE DATA");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "ACK/NAK TIMEOUT");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "NAK RECEIVED");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "DATA OFFSET ERROR");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "INITIATOR RESPONSE TIMEOUT");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "CONNECTION LOST");
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, "DATA-IN BUFFER OVERFLOW - DATA BUFFER SIZE");
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, "DATA-IN BUFFER OVERFLOW - DATA BUFFER DESCRIPTOR AREA");
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, "DATA-IN BUFFER ERROR");
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, "DATA-OUT BUFFER OVERFLOW - DATA BUFFER SIZE");
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, "DATA-OUT BUFFER OVERFLOW - DATA BUFFER DESCRIPTOR AREA");
		break;
	case 0x0d:
		OutputLog(standardError | fileMainError, "DATA-OUT BUFFER ERROR");
		break;
	case 0x0e:
		OutputLog(standardError | fileMainError, "PCIE FABRIC ERROR");
		break;
	case 0x0f:
		OutputLog(standardError | fileMainError, "PCIE COMPLETION TIMEOUT");
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, "PCIE COMPLETER ABORT");
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, "PCIE POISONED TLP RECEIVED");
		break;
	case 0x12:
		OutputLog(standardError | fileMainError, "PCIE ECRC CHECK FAILED");
		break;
	case 0x13:
		OutputLog(standardError | fileMainError, "PCIE UNSUPPORTED REQUEST");
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, "PCIE ACS VIOLATION");
		break;
	case 0x15:
		OutputLog(standardError | fileMainError, "PCIE TLP PREFIX BLOCKED");
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
		OutputLog(standardError | fileMainError, "LOGICAL UNIT FAILED SELF-CONFIGURATION");
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
		OutputLog(standardError | fileMainError, "OVERLAPPED COMMANDS ATTEMPTED");
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
		OutputLog(standardError | fileMainError, "ERASE FAILURE");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "ERASE FAILURE - INCOMPLETE ERASE OPERATION DETECTED");
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
		OutputLog(standardError | fileMainError, "MEDIA LOAD OR EJECT FAILED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "MEDIUM REMOVAL PREVENTED");
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
		OutputLog(standardError | fileMainError, "SYSTEM BUFFER FULL");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "INSUFFICIENT RESERVATION RESOURCES");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "INSUFFICIENT RESOURCES");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "INSUFFICIENT REGISTRATION RESOURCES");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "INSUFFICIENT ACCESS CONTROL RESOURCES");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "AUXILIARY MEMORY OUT OF SPACE");
		break;
	case 0x0b:
		OutputLog(standardError | fileMainError, "INSUFFICIENT POWER FOR OPERATION");
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, "INSUFFICIENT RESOURCES TO CREATE ROD");
		break;
	case 0x0d:
		OutputLog(standardError | fileMainError, "INSUFFICIENT RESOURCES TO CREATE ROD TOKEN");
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
		OutputLog(standardError | fileMainError, "UNABLE TO RECOVER TABLE-OF-CONTENTS");
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
		OutputLog(standardError | fileMainError, "GENERATION DOES NOT EXIST");
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
		OutputLog(standardError | fileMainError, "UPDATED BLOCK READ");
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
		OutputLog(standardError | fileMainError, "OPERATOR REQUEST OR STATE CHANGE INPUT");
		break;
	case SCSI_SENSEQ_MEDIUM_REMOVAL:
		OutputLog(standardError | fileMainError, "OPERATOR MEDIUM REMOVAL REQUEST");
		break;
	case SCSI_SENSEQ_WRITE_PROTECT_ENABLE:
		OutputLog(standardError | fileMainError, "OPERATOR SELECTED WRITE PROTECT");
		break;
	case SCSI_SENSEQ_WRITE_PROTECT_DISABLE:
		OutputLog(standardError | fileMainError, "OPERATOR SELECTED WRITE PERMIT");
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
		OutputLog(standardError | fileMainError, "LOG EXCEPTION");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "THRESHOLD CONDITION MET");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "LOG COUNTER AT MAXIMUM");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "LOG LIST CODES EXHAUSTED");
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
		OutputLog(standardError | fileMainError, "RPL STATUS CHANGE");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "SPINDLES SYNCHRONIZED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "SPINDLES NOT SYNCHRONIZED");
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
		OutputLog(standardError | fileMainError, "FAILURE PREDICTION THRESHOLD EXCEEDED");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "MEDIA FAILURE PREDICTION THRESHOLD EXCEEDED");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "LOGICAL UNIT FAILURE PREDICTION THRESHOLD EXCEEDED");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "SPARE AREA EXHAUSTION PREDICTION THRESHOLD EXCEEDED");
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, "HARDWARE IMPENDING FAILURE GENERAL HARD DRIVE FAILURE");
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, "HARDWARE IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH");
		break;
	case 0x12:
		OutputLog(standardError | fileMainError, "HARDWARE IMPENDING FAILURE DATA ERROR RATE TOO HIGH");
		break;
	case 0x13:
		OutputLog(standardError | fileMainError, "HARDWARE IMPENDING FAILURE SEEK ERROR RATE TOO HIGH");
		break;
	case 0x14:
		OutputLog(standardError | fileMainError, "HARDWARE IMPENDING FAILURE TOO MANY BLOCK REASSIGNS");
		break;
	case 0x15:
		OutputLog(standardError | fileMainError, "HARDWARE IMPENDING FAILURE ACCESS TIMES TOO HIGH");
		break;
	case 0x16:
		OutputLog(standardError | fileMainError, "HARDWARE IMPENDING FAILURE START UNIT TIMES TOO HIGH");
		break;
	case 0x17:
		OutputLog(standardError | fileMainError, "HARDWARE IMPENDING FAILURE CHANNEL PARAMETRICS");
		break;
	case 0x18:
		OutputLog(standardError | fileMainError, "HARDWARE IMPENDING FAILURE CONTROLLER DETECTED");
		break;
	case 0x19:
		OutputLog(standardError | fileMainError, "HARDWARE IMPENDING FAILURE THROUGHPUT PERFORMANCE");
		break;
	case 0x1a:
		OutputLog(standardError | fileMainError, "HARDWARE IMPENDING FAILURE SEEK TIME PERFORMANCE");
		break;
	case 0x1b:
		OutputLog(standardError | fileMainError, "HARDWARE IMPENDING FAILURE SPIN-UP RETRY COUNT");
		break;
	case 0x1c:
		OutputLog(standardError | fileMainError, "HARDWARE IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT");
		break;
	case 0x20:
		OutputLog(standardError | fileMainError, "CONTROLLER IMPENDING FAILURE GENERAL HARD DRIVE FAILURE");
		break;
	case 0x21:
		OutputLog(standardError | fileMainError, "CONTROLLER IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH");
		break;
	case 0x22:
		OutputLog(standardError | fileMainError, "CONTROLLER IMPENDING FAILURE DATA ERROR RATE TOO HIGH");
		break;
	case 0x23:
		OutputLog(standardError | fileMainError, "CONTROLLER IMPENDING FAILURE SEEK ERROR RATE TOO HIGH");
		break;
	case 0x24:
		OutputLog(standardError | fileMainError, "CONTROLLER IMPENDING FAILURE TOO MANY BLOCK REASSIGNS");
		break;
	case 0x25:
		OutputLog(standardError | fileMainError, "CONTROLLER IMPENDING FAILURE ACCESS TIMES TOO HIGH");
		break;
	case 0x26:
		OutputLog(standardError | fileMainError, "CONTROLLER IMPENDING FAILURE START UNIT TIMES TOO HIGH");
		break;
	case 0x27:
		OutputLog(standardError | fileMainError, "CONTROLLER IMPENDING FAILURE CHANNEL PARAMETRICS");
		break;
	case 0x28:
		OutputLog(standardError | fileMainError, "CONTROLLER IMPENDING FAILURE CONTROLLER DETECTED");
		break;
	case 0x29:
		OutputLog(standardError | fileMainError, "CONTROLLER IMPENDING FAILURE THROUGHPUT PERFORMANCE");
		break;
	case 0x2a:
		OutputLog(standardError | fileMainError, "CONTROLLER IMPENDING FAILURE SEEK TIME PERFORMANCE");
		break;
	case 0x2b:
		OutputLog(standardError | fileMainError, "CONTROLLER IMPENDING FAILURE SPIN-UP RETRY COUNT");
		break;
	case 0x2c:
		OutputLog(standardError | fileMainError, "CONTROLLER IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT");
		break;
	case 0x30:
		OutputLog(standardError | fileMainError, "DATA CHANNEL IMPENDING FAILURE GENERAL HARD DRIVE FAILURE");
		break;
	case 0x31:
		OutputLog(standardError | fileMainError, "DATA CHANNEL IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH");
		break;
	case 0x32:
		OutputLog(standardError | fileMainError, "DATA CHANNEL IMPENDING FAILURE DATA ERROR RATE TOO HIGH");
		break;
	case 0x33:
		OutputLog(standardError | fileMainError, "DATA CHANNEL IMPENDING FAILURE SEEK ERROR RATE TOO HIGH");
		break;
	case 0x34:
		OutputLog(standardError | fileMainError, "DATA CHANNEL IMPENDING FAILURE TOO MANY BLOCK REASSIGNS");
		break;
	case 0x35:
		OutputLog(standardError | fileMainError, "DATA CHANNEL IMPENDING FAILURE ACCESS TIMES TOO HIGH");
		break;
	case 0x36:
		OutputLog(standardError | fileMainError, "DATA CHANNEL IMPENDING FAILURE START UNIT TIMES TOO HIGH");
		break;
	case 0x37:
		OutputLog(standardError | fileMainError, "DATA CHANNEL IMPENDING FAILURE CHANNEL PARAMETRICS");
		break;
	case 0x38:
		OutputLog(standardError | fileMainError, "DATA CHANNEL IMPENDING FAILURE CONTROLLER DETECTED");
		break;
	case 0x39:
		OutputLog(standardError | fileMainError, "DATA CHANNEL IMPENDING FAILURE THROUGHPUT PERFORMANCE");
		break;
	case 0x3a:
		OutputLog(standardError | fileMainError, "DATA CHANNEL IMPENDING FAILURE SEEK TIME PERFORMANCE");
		break;
	case 0x3b:
		OutputLog(standardError | fileMainError, "DATA CHANNEL IMPENDING FAILURE SPIN-UP RETRY COUNT");
		break;
	case 0x3c:
		OutputLog(standardError | fileMainError, "DATA CHANNEL IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT");
		break;
	case 0x40:
		OutputLog(standardError | fileMainError, "SERVO IMPENDING FAILURE GENERAL HARD DRIVE FAILURE");
		break;
	case 0x41:
		OutputLog(standardError | fileMainError, "SERVO IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH");
		break;
	case 0x42:
		OutputLog(standardError | fileMainError, "SERVO IMPENDING FAILURE DATA ERROR RATE TOO HIGH");
		break;
	case 0x43:
		OutputLog(standardError | fileMainError, "SERVO IMPENDING FAILURE SEEK ERROR RATE TOO HIGH");
		break;
	case 0x44:
		OutputLog(standardError | fileMainError, "SERVO IMPENDING FAILURE TOO MANY BLOCK REASSIGNS");
		break;
	case 0x45:
		OutputLog(standardError | fileMainError, "SERVO IMPENDING FAILURE ACCESS TIMES TOO HIGH");
		break;
	case 0x46:
		OutputLog(standardError | fileMainError, "SERVO IMPENDING FAILURE START UNIT TIMES TOO HIGH");
		break;
	case 0x47:
		OutputLog(standardError | fileMainError, "SERVO IMPENDING FAILURE CHANNEL PARAMETRICS");
		break;
	case 0x48:
		OutputLog(standardError | fileMainError, "SERVO IMPENDING FAILURE CONTROLLER DETECTED");
		break;
	case 0x49:
		OutputLog(standardError | fileMainError, "SERVO IMPENDING FAILURE THROUGHPUT PERFORMANCE");
		break;
	case 0x4a:
		OutputLog(standardError | fileMainError, "SERVO IMPENDING FAILURE SEEK TIME PERFORMANCE");
		break;
	case 0x4b:
		OutputLog(standardError | fileMainError, "SERVO IMPENDING FAILURE SPIN-UP RETRY COUNT");
		break;
	case 0x4c:
		OutputLog(standardError | fileMainError, "SERVO IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT");
		break;
	case 0x50:
		OutputLog(standardError | fileMainError, "SPINDLE IMPENDING FAILURE GENERAL HARD DRIVE FAILURE");
		break;
	case 0x51:
		OutputLog(standardError | fileMainError, "SPINDLE IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH");
		break;
	case 0x52:
		OutputLog(standardError | fileMainError, "SPINDLE IMPENDING FAILURE DATA ERROR RATE TOO HIGH");
		break;
	case 0x53:
		OutputLog(standardError | fileMainError, "SPINDLE IMPENDING FAILURE SEEK ERROR RATE TOO HIGH");
		break;
	case 0x54:
		OutputLog(standardError | fileMainError, "SPINDLE IMPENDING FAILURE TOO MANY BLOCK REASSIGNS");
		break;
	case 0x55:
		OutputLog(standardError | fileMainError, "SPINDLE IMPENDING FAILURE ACCESS TIMES TOO HIGH");
		break;
	case 0x56:
		OutputLog(standardError | fileMainError, "SPINDLE IMPENDING FAILURE START UNIT TIMES TOO HIGH");
		break;
	case 0x57:
		OutputLog(standardError | fileMainError, "SPINDLE IMPENDING FAILURE CHANNEL PARAMETRICS");
		break;
	case 0x58:
		OutputLog(standardError | fileMainError, "SPINDLE IMPENDING FAILURE CONTROLLER DETECTED");
		break;
	case 0x59:
		OutputLog(standardError | fileMainError, "SPINDLE IMPENDING FAILURE THROUGHPUT PERFORMANCE");
		break;
	case 0x5a:
		OutputLog(standardError | fileMainError, "SPINDLE IMPENDING FAILURE SEEK TIME PERFORMANCE");
		break;
	case 0x5b:
		OutputLog(standardError | fileMainError, "SPINDLE IMPENDING FAILURE SPIN-UP RETRY COUNT");
		break;
	case 0x5c:
		OutputLog(standardError | fileMainError, "SPINDLE IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT");
		break;
	case 0x60:
		OutputLog(standardError | fileMainError, "FIRMWARE IMPENDING FAILURE GENERAL HARD DRIVE FAILURE");
		break;
	case 0x61:
		OutputLog(standardError | fileMainError, "FIRMWARE IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH");
		break;
	case 0x62:
		OutputLog(standardError | fileMainError, "FIRMWARE IMPENDING FAILURE DATA ERROR RATE TOO HIGH");
		break;
	case 0x63:
		OutputLog(standardError | fileMainError, "FIRMWARE IMPENDING FAILURE SEEK ERROR RATE TOO HIGH");
		break;
	case 0x64:
		OutputLog(standardError | fileMainError, "FIRMWARE IMPENDING FAILURE TOO MANY BLOCK REASSIGNS");
		break;
	case 0x65:
		OutputLog(standardError | fileMainError, "FIRMWARE IMPENDING FAILURE ACCESS TIMES TOO HIGH");
		break;
	case 0x66:
		OutputLog(standardError | fileMainError, "FIRMWARE IMPENDING FAILURE START UNIT TIMES TOO HIGH");
		break;
	case 0x67:
		OutputLog(standardError | fileMainError, "FIRMWARE IMPENDING FAILURE CHANNEL PARAMETRICS");
		break;
	case 0x68:
		OutputLog(standardError | fileMainError, "FIRMWARE IMPENDING FAILURE CONTROLLER DETECTED");
		break;
	case 0x69:
		OutputLog(standardError | fileMainError, "FIRMWARE IMPENDING FAILURE THROUGHPUT PERFORMANCE");
		break;
	case 0x6a:
		OutputLog(standardError | fileMainError, "FIRMWARE IMPENDING FAILURE SEEK TIME PERFORMANCE");
		break;
	case 0x6b:
		OutputLog(standardError | fileMainError, "FIRMWARE IMPENDING FAILURE SPIN-UP RETRY COUNT");
		break;
	case 0x6c:
		OutputLog(standardError | fileMainError, "FIRMWARE IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT");
		break;
	case 0xff:
		OutputLog(standardError | fileMainError, "FAILURE PREDICTION THRESHOLD EXCEEDED (FALSE)");
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
		OutputLog(standardError | fileMainError, "LOW POWER CONDITION ON");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "IDLE CONDITION ACTIVATED BY TIMER");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "STANDBY CONDITION ACTIVATED BY TIMER");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "IDLE CONDITION ACTIVATED BY COMMAND");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "STANDBY CONDITION ACTIVATED BY COMMAND");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "IDLE_B CONDITION ACTIVATED BY TIMER");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "IDLE_B CONDITION ACTIVATED BY COMMAND");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "IDLE_C CONDITION ACTIVATED BY TIMER");
		break;
	case 0x08:
		OutputLog(standardError | fileMainError, "IDLE_C CONDITION ACTIVATED BY COMMAND");
		break;
	case 0x09:
		OutputLog(standardError | fileMainError, "STANDBY_Y CONDITION ACTIVATED BY TIMER");
		break;
	case 0x0a:
		OutputLog(standardError | fileMainError, "STANDBY_Y CONDITION ACTIVATED BY COMMAND");
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
		OutputLog(standardError | fileMainError, "END OF USER AREA ENCOUNTERED ON THIS TRACK");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "PACKET DOES NOT FIT IN AVAILABLE SPACE");
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
		OutputLog(standardError | fileMainError, "ILLEGAL MODE FOR THIS TRACK");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "INVALID PACKET SIZE");
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
		OutputLog(standardError | fileMainError, "VOLTAGE FAULT");
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
		OutputLog(standardError | fileMainError, "SET TARGET PORT GROUPS COMMAND FAILED");
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
		OutputLog(standardError | fileMainError, "SUBSIDIARY LOGICAL UNIT NOT CONFIGURED");
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
		OutputLog(standardError | fileMainError, "COPY PROTECTION KEY EXCHANGE FAILURE - AUTHENTICATION FAILURE");
		break;
	case SCSI_SENSEQ_KEY_NOT_PRESENT:
		OutputLog(standardError | fileMainError, "COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT PRESENT");
		break;
	case SCSI_SENSEQ_KEY_NOT_ESTABLISHED:
		OutputLog(standardError | fileMainError, "COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT ESTABLISHED");
		break;
	case SCSI_SENSEQ_READ_OF_SCRAMBLED_SECTOR_WITHOUT_AUTHENTICATION:
		OutputLog(standardError | fileMainError, "READ OF SCRAMBLED SECTOR WITHOUT AUTHENTICATION");
		break;
	case SCSI_SENSEQ_MEDIA_CODE_MISMATCHED_TO_LOGICAL_UNIT:
		OutputLog(standardError | fileMainError, "MEDIA REGION CODE IS MISMATCHED TO LOGICAL UNIT REGION");
		break;
	case SCSI_SENSEQ_LOGICAL_UNIT_RESET_COUNT_ERROR:
		OutputLog(standardError | fileMainError, "DRIVE REGION MUST BE PERMANENT/REGION RESET COUNT ERROR");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "INSUFFICIENT BLOCK COUNT FOR BINDING NONCE RECORDING");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "CONFLICT IN BINDING NONCE RECORDING");
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
		OutputLog(standardError | fileMainError, "SESSION FIXATION ERROR");
		break;
	case 0x01:
		OutputLog(standardError | fileMainError, "SESSION FIXATION ERROR WRITING LEAD-IN");
		break;
	case 0x02:
		OutputLog(standardError | fileMainError, "SESSION FIXATION ERROR WRITING LEAD-OUT");
		break;
	case 0x03:
		OutputLog(standardError | fileMainError, "SESSION FIXATION ERROR - INCOMPLETE TRACK IN SESSION");
		break;
	case 0x04:
		OutputLog(standardError | fileMainError, "EMPTY OR PARTIALLY WRITTEN RESERVED TRACK");
		break;
	case 0x05:
		OutputLog(standardError | fileMainError, "NO MORE TRACK RESERVATIONS ALLOWED");
		break;
	case 0x06:
		OutputLog(standardError | fileMainError, "RMZ EXTENSION IS NOT ALLOWED");
		break;
	case 0x07:
		OutputLog(standardError | fileMainError, "NO MORE TEST ZONE EXTENSIONS ARE ALLOWED");
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
		OutputLog(standardError | fileMainError, "CD CONTROL ERROR");
		break;
	case SCSI_SENSEQ_POWER_CALIBRATION_AREA_ALMOST_FULL:
		OutputLog(standardError | fileMainError, "POWER CALIBRATION AREA ALMOST FULL");
		break;
	case SCSI_SENSEQ_POWER_CALIBRATION_AREA_FULL:
		OutputLog(standardError | fileMainError, "POWER CALIBRATION AREA IS FULL");
		break;
	case SCSI_SENSEQ_POWER_CALIBRATION_AREA_ERROR:
		OutputLog(standardError | fileMainError, "POWER CALIBRATION AREA ERROR");
		break;
	case SCSI_SENSEQ_PMA_RMA_UPDATE_FAILURE:
		OutputLog(standardError | fileMainError, "PROGRAM MEMORY AREA UPDATE FAILURE");
		break;
	case SCSI_SENSEQ_PMA_RMA_IS_FULL:
		OutputLog(standardError | fileMainError, "RMA/PMA IS FULL");
		break;
	case SCSI_SENSEQ_PMA_RMA_ALMOST_FULL:
		OutputLog(standardError | fileMainError, "RMA/PMA IS ALMOST FULL");
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, "CURRENT POWER CALIBRATION AREA ALMOST FULL");
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, "CURRENT POWER CALIBRATION AREA IS FULL");
		break;
	case 0x17:
		OutputLog(standardError | fileMainError, "RDZ IS FULL");
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
		OutputLog(standardError | fileMainError, "DIGITAL SIGNATURE VALIDATION FAILURE");
		break;
	case 0x0c:
		OutputLog(standardError | fileMainError, "UNABLE TO DECRYPT PARAMETER LIST");
		break;
	case 0x10:
		OutputLog(standardError | fileMainError, "SA CREATION PARAMETER VALUE INVALID");
		break;
	case 0x11:
		OutputLog(standardError | fileMainError, "SA CREATION PARAMETER VALUE REJECTED");
		break;
	case 0x12:
		OutputLog(standardError | fileMainError, "INVALID SA USAGE");
		break;
	case 0x30:
		OutputLog(standardError | fileMainError, "SA CREATION PARAMETER NOT SUPPORTED");
		break;
	case 0x40:
		OutputLog(standardError | fileMainError, "AUTHENTICATION FAILED");
		break;
	case 0x71:
		OutputLog(standardError | fileMainError, "LOGICAL UNIT ACCESS NOT AUTHORIZED");
		break;
	case 0x79:
		OutputLog(standardError | fileMainError, "SECURITY CONFLICT IN TRANSLATED DEVICE");
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
	OutputLog(standardError | fileMainError, "\n");
}

VOID OutputSenseKey(
	BYTE byKey
) {
	switch (byKey) {
	case SCSI_SENSE_NO_SENSE:
		OutputLog(standardError | fileMainError, "NO_SENSE");
		break;
	case SCSI_SENSE_RECOVERED_ERROR:
		OutputLog(standardError | fileMainError, "RECOVERED_ERROR");
		break;
	case SCSI_SENSE_NOT_READY:
		OutputLog(standardError | fileMainError, "NOT_READY");
		break;
	case SCSI_SENSE_MEDIUM_ERROR:
		OutputLog(standardError | fileMainError, "MEDIUM_ERROR");
		break;
	case SCSI_SENSE_HARDWARE_ERROR:
		OutputLog(standardError | fileMainError, "HARDWARE_ERROR");
		break;
	case SCSI_SENSE_ILLEGAL_REQUEST:
		OutputLog(standardError | fileMainError, "ILLEGAL_REQUEST");
		break;
	case SCSI_SENSE_UNIT_ATTENTION:
		OutputLog(standardError | fileMainError, "UNIT_ATTENTION");
		break;
	case SCSI_SENSE_DATA_PROTECT:
		OutputLog(standardError | fileMainError, "DATA_PROTECT");
		break;
	case SCSI_SENSE_BLANK_CHECK:
		OutputLog(standardError | fileMainError, "BLANK_CHECK");
		break;
	case SCSI_SENSE_UNIQUE:
		OutputLog(standardError | fileMainError, "UNIQUE");
		break;
	case SCSI_SENSE_COPY_ABORTED:
		OutputLog(standardError | fileMainError, "COPY_ABORTED");
		break;
	case SCSI_SENSE_ABORTED_COMMAND:
		OutputLog(standardError | fileMainError, "ABORTED_COMMAND");
		break;
	case SCSI_SENSE_EQUAL:
		OutputLog(standardError | fileMainError, "EQUAL");
		break;
	case SCSI_SENSE_VOL_OVERFLOW:
		OutputLog(standardError | fileMainError, "VOL_OVERFLOW");
		break;
	case SCSI_SENSE_MISCOMPARE:
		OutputLog(standardError | fileMainError, "MISCOMPARE");
		break;
	case SCSI_SENSE_RESERVED:
		OutputLog(standardError | fileMainError, "RESERVED");
		break;
	default:
		OutputLog(standardError | fileMainError, "UNKNOWN");
		break;
	}
	OutputLog(standardError | fileMainError, " - ");
}

VOID OutputSenseData(
	PSENSE_DATA pSenseData
) {
	OutputLog(standardError | fileMainError,
		"\tSenseData Key-Asc-Ascq: %02x-%02x-%02x = "
		, pSenseData->SenseKey, pSenseData->AdditionalSenseCode
		, pSenseData->AdditionalSenseCodeQualifier);
	OutputSenseKey(pSenseData->SenseKey);
	OutputAdditionalSenseCode(
		pSenseData->AdditionalSenseCode, pSenseData->AdditionalSenseCodeQualifier);
#ifdef _DEBUG
	OutputLog(standardError | fileMainError, 
		"OtherSenseData\n"
		"                  ErrorCode: %#x\n"
		"                      Valid: %s\n"
		"              SegmentNumber: %u\n"
		"            IncorrectLength: %u\n"
		"                 EndOfMedia: %s\n"
		"                   FileMark: %s\n",
		pSenseData->ErrorCode,
		BOOLEAN_TO_STRING_YES_NO(pSenseData->Valid),
		pSenseData->SegmentNumber,
		pSenseData->IncorrectLength,
		BOOLEAN_TO_STRING_YES_NO(pSenseData->EndOfMedia),
		BOOLEAN_TO_STRING_YES_NO(pSenseData->FileMark));
	if (pSenseData->Valid) {
		OutputLog(standardError | fileMainError, 
			"                Information: %u%u%u%u\n",
			pSenseData->Information[0],
			pSenseData->Information[1],
			pSenseData->Information[2],
			pSenseData->Information[3]);
	}
	OutputLog(standardError | fileMainError, 
		"      AdditionalSenseLength: %u\n"
		" CommandSpecificInformation: %u%u%u%u\n"
		"   FieldReplaceableUnitCode: %u\n"
		"           SenseKeySpecific: %u%u%u\n",
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
		"\tScsiStatus: %#04x = ", byScsiStatus);
	switch (byScsiStatus) {
	case SCSISTAT_GOOD:
		OutputLog(standardError | fileMainError, "GOOD\n");
		break;
	case SCSISTAT_CHECK_CONDITION:
		OutputLog(standardError | fileMainError, "CHECK_CONDITION\n");
		break;
	case SCSISTAT_CONDITION_MET:
		OutputLog(standardError | fileMainError, "CONDITION_MET\n");
		break;
	case SCSISTAT_BUSY:
		OutputLog(standardError | fileMainError, "BUSY\n");
		break;
	case SCSISTAT_INTERMEDIATE:
		OutputLog(standardError | fileMainError, "INTERMEDIATE\n");
		break;
	case SCSISTAT_INTERMEDIATE_COND_MET:
		OutputLog(standardError | fileMainError, "INTERMEDIATE_COND_MET\n");
		break;
	case SCSISTAT_RESERVATION_CONFLICT:
		OutputLog(standardError | fileMainError, "RESERVATION_CONFLICT\n");
		break;
	case SCSISTAT_COMMAND_TERMINATED:
		OutputLog(standardError | fileMainError, "COMMAND_TERMINATED\n");
		break;
	case SCSISTAT_QUEUE_FULL:
		OutputLog(standardError | fileMainError, "QUEUE_FULL\n");
		break;
	default:
		OutputLog(standardError | fileMainError, "UNKNOWN\n");
		break;
	}
}

VOID OutputScsiAddress(
	PDEVICE pDevice
) {
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("IoctlScsiGetAddress")
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
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("StorageAdapterDescriptor")
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
		OutputDriveLog("(Buffers must be aligned on BYTE boundaries)\n");
		break;
	case 1:
		OutputDriveLog("(Buffers must be aligned on WORD boundaries)\n");
		break;
	case 3:
		OutputDriveLog("(Buffers must be aligned on DWORD32 boundaries)\n");
		break;
	case 7:
		OutputDriveLog("(Buffers must be aligned on DWORD64 boundaries)\n");
		break;
	default:
		OutputDriveLog("\n");
		break;
	}
	OutputDriveLog(
		"\t       AdapterUsesPio: %s\n"
		"\t     AdapterScansDown: %s\n"
		"\t      CommandQueueing: %s\n"
		"\t  AcceleratedTransfer: %s\n"
		"\t              BusType: "
		, BOOLEAN_TO_STRING_TRUE_FALSE(pAdapterDescriptor->AdapterUsesPio)
		, BOOLEAN_TO_STRING_TRUE_FALSE(pAdapterDescriptor->AdapterScansDown)
		, BOOLEAN_TO_STRING_TRUE_FALSE(pAdapterDescriptor->CommandQueueing)
		, BOOLEAN_TO_STRING_TRUE_FALSE(pAdapterDescriptor->AcceleratedTransfer));
	switch (pAdapterDescriptor->BusType) {
	case STORAGE_BUS_TYPE::BusTypeUnknown:
		OutputDriveLog("BusTypeUnknown\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeScsi:
		OutputDriveLog("BusTypeScsi\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeAtapi:
		OutputDriveLog("BusTypeAtapi\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeAta:
		OutputDriveLog("BusTypeAta\n");
		break;
	case STORAGE_BUS_TYPE::BusType1394:
		OutputDriveLog("BusType1394\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeSsa:
		OutputDriveLog("BusTypeSsa\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeFibre:
		OutputDriveLog("BusTypeFibre\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeUsb:
		OutputDriveLog("BusTypeUsb\n");
		*lpBusTypeUSB = TRUE;
		break;
	case STORAGE_BUS_TYPE::BusTypeRAID:
		OutputDriveLog("BusTypeRAID\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeiScsi:
		OutputDriveLog("BusTypeiScsi\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeSas:
		OutputDriveLog("BusTypeSas\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeSata:
		OutputDriveLog("BusTypeSata\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeSd:
		OutputDriveLog("BusTypeSd\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeMmc:
		OutputDriveLog("BusTypeMmc\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeVirtual:
		OutputDriveLog("BusTypeVirtual\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeFileBackedVirtual:
		OutputDriveLog("BusTypeFileBackedVirtual\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeMax:
		OutputDriveLog("BusTypeMax\n");
		break;
	case STORAGE_BUS_TYPE::BusTypeMaxReserved:
		OutputDriveLog("BusTypeMaxReserved\n");
		break;
	default:
		OutputDriveLog("BusTypeUnknown\n");
		break;
	}
	OutputDriveLog(
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
		OutputDiscLog("Format is unknown\n");
		break;
	case F5_1Pt2_512:
		OutputDiscLog("5.25\", 1.2MB, 512 bytes/sector\n");
		break;
	case F3_1Pt44_512:
		OutputDiscLog("3.5\", 1.44MB, 512 bytes/sector\n");
		break;
	case F3_2Pt88_512:
		OutputDiscLog("3.5\", 2.88MB, 512 bytes/sector\n");
		break;
	case F3_20Pt8_512:
		OutputDiscLog("3.5\", 20.8MB, 512 bytes/sector\n");
		break;
	case F3_720_512:
		OutputDiscLog("3.5\", 720KB, 512 bytes/sector\n");
		break;
	case F5_360_512:
		OutputDiscLog("5.25\", 360KB, 512 bytes/sector\n");
		break;
	case F5_320_512:
		OutputDiscLog("5.25\", 320KB, 512 bytes/sector\n");
		break;
	case F5_320_1024:
		OutputDiscLog("5.25\", 320KB, 1024 bytes/sector\n");
		break;
	case F5_180_512:
		OutputDiscLog("5.25\", 180KB, 512 bytes/sector\n");
		break;
	case F5_160_512:
		OutputDiscLog("5.25\", 160KB, 512 bytes/sector\n");
		break;
	case RemovableMedia:
		OutputDiscLog("Removable media other than floppy\n");
		break;
	case FixedMedia:
		OutputDiscLog("Fixed hard disk media\n");
		break;
	case F3_120M_512:
		OutputDiscLog("3.5\", 120M Floppy\n");
		break;
	case F3_640_512:
		OutputDiscLog("3.5\", 640KB, 512 bytes/sector\n");
		break;
	case F5_640_512:
		OutputDiscLog("5.25\", 640KB, 512 bytes/sector\n");
		break;
	case F5_720_512:
		OutputDiscLog("5.25\", 720KB, 512 bytes/sector\n");
		break;
	case F3_1Pt2_512:
		OutputDiscLog("3.5\", 1.2Mb, 512 bytes/sector\n");
		break;
	case F3_1Pt23_1024:
		OutputDiscLog("3.5\", 1.23Mb, 1024 bytes/sector\n");
		break;
	case F5_1Pt23_1024:
		OutputDiscLog("5.25\", 1.23MB, 1024 bytes/sector\n");
		break;
	case F3_128Mb_512:
		OutputDiscLog("3.5\" MO, 128Mb, 512 bytes/sector\n");
		break;
	case F3_230Mb_512:
		OutputDiscLog("3.5\" MO, 230Mb, 512 bytes/sector\n");
		break;
	case F8_256_128:
		OutputDiscLog("8\", 256KB, 128 bytes/sector\n");
		break;
	case F3_200Mb_512:
		OutputDiscLog("3.5\", 200M Floppy (HiFD)\n");
		break;
	case F3_240M_512:
		OutputDiscLog("3.5\", 240Mb Floppy (HiFD)\n");
		break;
	case F3_32M_512:
		OutputDiscLog("3.5\", 32Mb Floppy\n");
		break;
	default:
		OutputDiscLog("Unknown media type: %d\n", mType);
		break;
	}
}

VOID OutputStorageMediaType(
	STORAGE_MEDIA_TYPE mType
) {
	switch (mType) {
	case DDS_4mm:
		OutputDiscLog("Tape - DAT DDS1,2,... (all vendors)\n");
		break;
	case MiniQic:
		OutputDiscLog("Tape - miniQIC Tape\n");
		break;
	case Travan:
		OutputDiscLog("Tape - Travan TR-1,2,3,...\n");
		break;
	case QIC:
		OutputDiscLog("Tape - QIC\n");
		break;
	case MP_8mm:
		OutputDiscLog("Tape - 8mm Exabyte Metal Particle\n");
		break;
	case AME_8mm:
		OutputDiscLog("Tape - 8mm Exabyte Advanced Metal Evap\n");
		break;
	case AIT1_8mm:
		OutputDiscLog("Tape - 8mm Sony AIT\n");
		break;
	case DLT:
		OutputDiscLog("Tape - DLT Compact IIIxt, IV\n");
		break;
	case NCTP:
		OutputDiscLog("Tape - Philips NCTP\n");
		break;
	case IBM_3480:
		OutputDiscLog("Tape - IBM 3480\n");
		break;
	case IBM_3490E:
		OutputDiscLog("Tape - IBM 3490E\n");
		break;
	case IBM_Magstar_3590:
		OutputDiscLog("Tape - IBM Magstar 3590\n");
		break;
	case IBM_Magstar_MP:
		OutputDiscLog("Tape - IBM Magstar MP\n");
		break;
	case STK_DATA_D3:
		OutputDiscLog("Tape - STK Data D3\n");
		break;
	case SONY_DTF:
		OutputDiscLog("Tape - Sony DTF\n");
		break;
	case DV_6mm:
		OutputDiscLog("Tape - 6mm Digital Video\n");
		break;
	case DMI:
		OutputDiscLog("Tape - Exabyte DMI and compatibles\n");
		break;
	case SONY_D2:
		OutputDiscLog("Tape - Sony D2S and D2L\n");
		break;
	case CLEANER_CARTRIDGE:
		OutputDiscLog("Cleaner - All Drive types that support Drive Cleaners\n");
		break;
	case CD_ROM:
		OutputDiscLog("Opt_Disk - CD\n");
		break;
	case CD_R:
		OutputDiscLog("Opt_Disk - CD-Recordable (Write Once)\n");
		break;
	case CD_RW:
		OutputDiscLog("Opt_Disk - CD-Rewriteable\n");
		break;
	case DVD_ROM:
		OutputDiscLog("Opt_Disk - DVD-ROM\n");
		break;
	case DVD_R:
		OutputDiscLog("Opt_Disk - DVD-Recordable (Write Once)\n");
		break;
	case DVD_RW:
		OutputDiscLog("Opt_Disk - DVD-Rewriteable\n");
		break;
	case MO_3_RW:
		OutputDiscLog("Opt_Disk - 3.5\" Rewriteable MO Disk\n");
		break;
	case MO_5_WO:
		OutputDiscLog("Opt_Disk - MO 5.25\" Write Once\n");
		break;
	case MO_5_RW:
		OutputDiscLog("Opt_Disk - MO 5.25\" Rewriteable (not LIMDOW)\n");
		break;
	case MO_5_LIMDOW:
		OutputDiscLog("Opt_Disk - MO 5.25\" Rewriteable (LIMDOW)\n");
		break;
	case PC_5_WO:
		OutputDiscLog("Opt_Disk - Phase Change 5.25\" Write Once Optical\n");
		break;
	case PC_5_RW:
		OutputDiscLog("Opt_Disk - Phase Change 5.25\" Rewriteable\n");
		break;
	case PD_5_RW:
		OutputDiscLog("Opt_Disk - PhaseChange Dual Rewriteable\n");
		break;
	case ABL_5_WO:
		OutputDiscLog("Opt_Disk - Ablative 5.25\" Write Once Optical\n");
		break;
	case PINNACLE_APEX_5_RW:
		OutputDiscLog("Opt_Disk - Pinnacle Apex 4.6GB Rewriteable Optical\n");
		break;
	case SONY_12_WO:
		OutputDiscLog("Opt_Disk - Sony 12\" Write Once\n");
		break;
	case PHILIPS_12_WO:
		OutputDiscLog("Opt_Disk - Philips/LMS 12\" Write Once\n");
		break;
	case HITACHI_12_WO:
		OutputDiscLog("Opt_Disk - Hitachi 12\" Write Once\n");
		break;
	case CYGNET_12_WO:
		OutputDiscLog("Opt_Disk - Cygnet/ATG 12\" Write Once\n");
		break;
	case KODAK_14_WO:
		OutputDiscLog("Opt_Disk - Kodak 14\" Write Once\n");
		break;
	case MO_NFR_525:
		OutputDiscLog("Opt_Disk - Near Field Recording (Terastor)\n");
		break;
	case NIKON_12_RW:
		OutputDiscLog("Opt_Disk - Nikon 12\" Rewriteable\n");
		break;
	case IOMEGA_ZIP:
		OutputDiscLog("Mag_Disk - Iomega Zip\n");
		break;
	case IOMEGA_JAZ:
		OutputDiscLog("Mag_Disk - Iomega Jaz\n");
		break;
	case SYQUEST_EZ135:
		OutputDiscLog("Mag_Disk - Syquest EZ135\n");
		break;
	case SYQUEST_EZFLYER:
		OutputDiscLog("Mag_Disk - Syquest EzFlyer\n");
		break;
	case SYQUEST_SYJET:
		OutputDiscLog("Mag_Disk - Syquest SyJet\n");
		break;
	case AVATAR_F2:
		OutputDiscLog("Mag_Disk - 2.5\" Floppy\n");
		break;
	case MP2_8mm:
		OutputDiscLog("Tape - 8mm Hitachi\n");
		break;
	case DST_S:
		OutputDiscLog("Ampex DST Small Tapes\n");
		break;
	case DST_M:
		OutputDiscLog("Ampex DST Medium Tapes\n");
		break;
	case DST_L:
		OutputDiscLog("Ampex DST Large Tapes\n");
		break;
	case VXATape_1:
		OutputDiscLog("Ecrix 8mm Tape 1\n");
		break;
	case VXATape_2:
		OutputDiscLog("Ecrix 8mm Tape 2\n");
		break;
	case STK_9840:
		OutputDiscLog("STK 9840\n");
		break;
	case LTO_Ultrium:
		OutputDiscLog("IBM, HP, Seagate LTO Ultrium\n");
		break;
	case LTO_Accelis:
		OutputDiscLog("IBM, HP, Seagate LTO Accelis\n");
		break;
	case DVD_RAM:
		OutputDiscLog("Opt_Disk - DVD-RAM\n");
		break;
	case AIT_8mm:
		OutputDiscLog("AIT2 or higher\n");
		break;
	case ADR_1:
		OutputDiscLog("OnStream ADR Mediatypes 1\n");
		break;
	case ADR_2:
		OutputDiscLog("OnStream ADR Mediatypes 2\n");
		break;
	case STK_9940:
		OutputDiscLog("STK 9940\n");
		break;
	case SAIT:
		OutputDiscLog("SAIT Tapes\n");
		break;
	case VXATape:
		OutputDiscLog("VXA (Ecrix 8mm) Tape\n");
		break;
	default:
		OutputDiscLog("Unknown storage media type: %d\n", mType);
		break;
	}
}

VOID OutputDeviceType(
	DWORD deviceType
) {
	switch (deviceType) {
	case FILE_DEVICE_BEEP:
		OutputDiscLog("BEEP\n");
		break;
	case FILE_DEVICE_CD_ROM:
		OutputDiscLog("CD_ROM\n");
		break;
	case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
		OutputDiscLog("CD_ROM_FILE_SYSTEM\n");
		break;
	case FILE_DEVICE_CONTROLLER:
		OutputDiscLog("CONTROLLER\n");
		break;
	case FILE_DEVICE_DATALINK:
		OutputDiscLog("DATALINK\n");
		break;
	case FILE_DEVICE_DFS:
		OutputDiscLog("DFS\n");
		break;
	case FILE_DEVICE_DISK:
		OutputDiscLog("DISK\n");
		break;
	case FILE_DEVICE_DISK_FILE_SYSTEM:
		OutputDiscLog("DISK_FILE_SYSTEM\n");
		break;
	case FILE_DEVICE_FILE_SYSTEM:
		OutputDiscLog("FILE_SYSTEM\n");
		break;
	case FILE_DEVICE_INPORT_PORT:
		OutputDiscLog("INPORT_PORT\n");
		break;
	case FILE_DEVICE_KEYBOARD:
		OutputDiscLog("KEYBOARD\n");
		break;
	case FILE_DEVICE_MAILSLOT:
		OutputDiscLog("MAILSLOT\n");
		break;
	case FILE_DEVICE_MIDI_IN:
		OutputDiscLog("MIDI_IN\n");
		break;
	case FILE_DEVICE_MIDI_OUT:
		OutputDiscLog("MIDI_OUT\n");
		break;
	case FILE_DEVICE_MOUSE:
		OutputDiscLog("MOUSE\n");
		break;
	case FILE_DEVICE_MULTI_UNC_PROVIDER:
		OutputDiscLog("MULTI_UNC_PROVIDER\n");
		break;
	case FILE_DEVICE_NAMED_PIPE:
		OutputDiscLog("NAMED_PIPE\n");
		break;
	case FILE_DEVICE_NETWORK:
		OutputDiscLog("NETWORK\n");
		break;
	case FILE_DEVICE_NETWORK_BROWSER:
		OutputDiscLog("NETWORK_BROWSER\n");
		break;
	case FILE_DEVICE_NETWORK_FILE_SYSTEM:
		OutputDiscLog("NETWORK_FILE_SYSTEM\n");
		break;
	case FILE_DEVICE_NULL:
		OutputDiscLog("NULL\n");
		break;
	case FILE_DEVICE_PARALLEL_PORT:
		OutputDiscLog("PARALLEL_PORT\n");
		break;
	case FILE_DEVICE_PHYSICAL_NETCARD:
		OutputDiscLog("PHYSICAL_NETCARD\n");
		break;
	case FILE_DEVICE_PRINTER:
		OutputDiscLog("PRINTER\n");
		break;
	case FILE_DEVICE_SCANNER:
		OutputDiscLog("SCANNER\n");
		break;
	case FILE_DEVICE_SERIAL_MOUSE_PORT:
		OutputDiscLog("SERIAL_MOUSE_PORT\n");
		break;
	case FILE_DEVICE_SERIAL_PORT:
		OutputDiscLog("SERIAL_PORT\n");
		break;
	case FILE_DEVICE_SCREEN:
		OutputDiscLog("SCREEN\n");
		break;
	case FILE_DEVICE_SOUND:
		OutputDiscLog("SOUND\n");
		break;
	case FILE_DEVICE_STREAMS:
		OutputDiscLog("STREAMS\n");
		break;
	case FILE_DEVICE_TAPE:
		OutputDiscLog("TAPE\n");
		break;
	case FILE_DEVICE_TAPE_FILE_SYSTEM:
		OutputDiscLog("TAPE_FILE_SYSTEM\n");
		break;
	case FILE_DEVICE_TRANSPORT:
		OutputDiscLog("TRANSPORT\n");
		break;
	case FILE_DEVICE_UNKNOWN:
		OutputDiscLog("UNKNOWN\n");
		break;
	case FILE_DEVICE_VIDEO:
		OutputDiscLog("VIDEO\n");
		break;
	case FILE_DEVICE_VIRTUAL_DISK:
		OutputDiscLog("VIRTUAL_DISK\n");
		break;
	case FILE_DEVICE_WAVE_IN:
		OutputDiscLog("WAVE_IN\n");
		break;
	case FILE_DEVICE_WAVE_OUT:
		OutputDiscLog("WAVE_OUT\n");
		break;
	case FILE_DEVICE_8042_PORT:
		OutputDiscLog("8042_PORT\n");
		break;
	case FILE_DEVICE_NETWORK_REDIRECTOR:
		OutputDiscLog("NETWORK_REDIRECTOR\n");
		break;
	case FILE_DEVICE_BATTERY:
		OutputDiscLog("BATTERY\n");
		break;
	case FILE_DEVICE_BUS_EXTENDER:
		OutputDiscLog("BUS_EXTENDER\n");
		break;
	case FILE_DEVICE_MODEM:
		OutputDiscLog("MODEM\n");
		break;
	case FILE_DEVICE_VDM:
		OutputDiscLog("VDM\n");
		break;
	case FILE_DEVICE_MASS_STORAGE:
		OutputDiscLog("MASS_STORAGE\n");
		break;
	case FILE_DEVICE_SMB:
		OutputDiscLog("SMB\n");
		break;
	case FILE_DEVICE_KS:
		OutputDiscLog("KS\n");
		break;
	case FILE_DEVICE_CHANGER:
		OutputDiscLog("CHANGER\n");
		break;
	case FILE_DEVICE_SMARTCARD:
		OutputDiscLog("SMARTCARD\n");
		break;
	case FILE_DEVICE_ACPI:
		OutputDiscLog("ACPI\n");
		break;
	case FILE_DEVICE_DVD:
		OutputDiscLog("DVD\n");
		break;
	case FILE_DEVICE_FULLSCREEN_VIDEO:
		OutputDiscLog("FULLSCREEN_VIDEO\n");
		break;
	case FILE_DEVICE_DFS_FILE_SYSTEM:
		OutputDiscLog("DFS_FILE_SYSTEM\n");
		break;
	case FILE_DEVICE_DFS_VOLUME:
		OutputDiscLog("DFS_VOLUME\n");
		break;
	case FILE_DEVICE_SERENUM:
		OutputDiscLog("SERENUM\n");
		break;
	case FILE_DEVICE_TERMSRV:
		OutputDiscLog("TERMSRV\n");
		break;
	case FILE_DEVICE_KSEC:
		OutputDiscLog("KSEC\n");
		break;
	case FILE_DEVICE_FIPS:
		OutputDiscLog("FIPS\n");
		break;
	case FILE_DEVICE_INFINIBAND:
		OutputDiscLog("INFINIBAND\n");
		break;
	case FILE_DEVICE_VMBUS:
		OutputDiscLog("VMBUS\n");
		break;
	case FILE_DEVICE_CRYPT_PROVIDER:
		OutputDiscLog("CRYPT_PROVIDER\n");
		break;
	case FILE_DEVICE_WPD:
		OutputDiscLog("WPD\n");
		break;
	case FILE_DEVICE_BLUETOOTH:
		OutputDiscLog("BLUETOOTH\n");
		break;
	case FILE_DEVICE_MT_COMPOSITE:
		OutputDiscLog("MT_COMPOSITE\n");
		break;
	case FILE_DEVICE_MT_TRANSPORT:
		OutputDiscLog("MT_TRANSPORT\n");
		break;
	case FILE_DEVICE_BIOMETRIC:
		OutputDiscLog("BIOMETRIC\n");
		break;
	case FILE_DEVICE_PMI:
		OutputDiscLog("PMI\n");
		break;
	default:
		OutputDiscLog("Unknown Device: %ld\n", deviceType);
		break;
	}
}

VOID OutputDiskGeometry(
	PDISK_GEOMETRY pGeom,
	DWORD dwGeomNum
) {
	if (dwGeomNum > 1) {
		OutputDiscLog(
			OUTPUT_DHYPHEN_PLUS_STR("DISK_GEOMETRY")
			"SupportedMediaType\n");
	}
	else if (dwGeomNum == 1) {
		OutputDiscLog("CurrentMediaType\n");
	}
	for (DWORD i = 0; i < dwGeomNum; i++) {
		OutputDiscLog("\t        MediaType: ");
		OutputMediaType(pGeom[i].MediaType);
		DWORD dwDiskSize = pGeom[i].Cylinders.u.LowPart * pGeom[i].TracksPerCylinder *
			pGeom[i].SectorsPerTrack * pGeom[i].BytesPerSector;
		OutputDiscLog(
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
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DISK_GEOMETRY_EX")
		"\t            DiskSize: %llu (Bytes)\n"
		"\t SizeOfPartitionInfo: %lu\n"
		"\t      PartitionStyle: %u "
		, pGeom->DiskSize.QuadPart
		, partition->SizeOfPartitionInfo
		, partition->PartitionStyle
	);
	if (partition->SizeOfPartitionInfo) {
		switch (partition->PartitionStyle) {
		case PARTITION_STYLE_MBR:
			OutputDiscLog(
				"(MBR)\n"
				"\t           Signiture: %lu\n"
				"\t            CheckSum: %lu\n"
				, partition->Mbr.Signature
				, partition->Mbr.CheckSum
			);
			break;
		case PARTITION_STYLE_GPT:
			OutputDiscLog(
				"(GPT)\n"
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
			break;
		case PARTITION_STYLE_RAW:
			OutputDiscLog("(GPT)\n");
			break;
		default:
			OutputDiscLog("(Unknown)\n");
			break;
		}
	}

	OutputDiscLog(
		"\t    SizeOfDetectInfo: %lu\n"
		"\t       DetectionType: %u "
		, detection->SizeOfDetectInfo
		, detection->DetectionType
	);
	if (detection->SizeOfDetectInfo) {
		switch (detection->DetectionType) {
		case DetectNone:
			OutputDiscLog("(None)\n");
			break;
		case DetectInt13:
			OutputDiscLog(
				"(Int13)\n"
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
			break;
		case DetectExInt13:
			OutputDiscLog(
				"(ExInt13)\n"
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
			break;
		default:
			OutputDiscLog("(Unknown)\n");
			break;
		}
	}

}

VOID OutputRemovableDiskInfo(
	PGET_MEDIA_TYPES pMedia
) {
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR("DEVICE_MEDIA_INFO")
		"\t          DeviceType: "
	);
	OutputDeviceType(pMedia->DeviceType);
	OutputDiscLog(
		"\t      MediaInfoCount: %ld\n"
		, pMedia->MediaInfoCount
	);
	for (DWORD i = 0; i < pMedia->MediaInfoCount; i++) {
		// TODO: pMedia->MediaInfo[i].DeviceSpecific.TapeInfo
		OutputDiscLog(
			"\t           Cylinders: %ld\n"
			"\t           MediaType: "
			, pMedia->MediaInfo[i].DeviceSpecific.RemovableDiskInfo.Cylinders.u.LowPart
		);
		STORAGE_MEDIA_TYPE mType = pMedia->MediaInfo[i].DeviceSpecific.RemovableDiskInfo.MediaType;
		if (mType < DDS_4mm) {
			OutputMediaType((MEDIA_TYPE)mType);
		}
		else {
			OutputStorageMediaType(mType);
		}

		DWORD mc = pMedia->MediaInfo[i].DeviceSpecific.RemovableDiskInfo.MediaCharacteristics;
		OutputDiscLog(
			"\t   TracksPerCylinder: %ld\n"
			"\t     SectorsPerTrack: %ld\n"
			"\t      BytesPerSector: %ld\n"
			"\t    NumberMediaSides: %ld\n"
			"\tMediaCharacteristics:"
			, pMedia->MediaInfo[i].DeviceSpecific.RemovableDiskInfo.TracksPerCylinder
			, pMedia->MediaInfo[i].DeviceSpecific.RemovableDiskInfo.SectorsPerTrack
			, pMedia->MediaInfo[i].DeviceSpecific.RemovableDiskInfo.BytesPerSector
			, pMedia->MediaInfo[i].DeviceSpecific.RemovableDiskInfo.NumberMediaSides
		);
		if ((mc & MEDIA_ERASEABLE) == MEDIA_ERASEABLE) {
			OutputDiscLog(" Eraseable");
		}
		if ((mc & MEDIA_WRITE_ONCE) == MEDIA_WRITE_ONCE) {
			OutputDiscLog(" WriteOnce");
		}
		if ((mc & MEDIA_READ_ONLY) == MEDIA_READ_ONLY) {
			OutputDiscLog(" ReadOnly");
		}
		if ((mc & MEDIA_READ_WRITE) == MEDIA_READ_WRITE) {
			OutputDiscLog(" ReadWrite");
		}
		if ((mc & MEDIA_WRITE_PROTECTED) == MEDIA_WRITE_PROTECTED) {
			OutputDiscLog(" WriteProtected");
		}
		if ((mc & MEDIA_CURRENTLY_MOUNTED) == MEDIA_CURRENTLY_MOUNTED) {
			OutputDiscLog(" CurrentlyMounted");
		}
		OutputDiscLog("\n");
	}
}

VOID OutputDVDGetRegion(
	PDVD_REGION dvdRegion
) {
	OutputDriveLog(
		OUTPUT_DHYPHEN_PLUS_STR("DVD Region")
		"\tSystemRegion: %d\n"
		"\t  ResetCount: %d\n"
		, dvdRegion->SystemRegion
		, dvdRegion->ResetCount
	);
}
