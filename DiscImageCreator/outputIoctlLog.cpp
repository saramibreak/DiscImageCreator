/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "get.h"
#include "output.h"
#include "outputIoctlLog.h"

VOID OutputAdditionalSenseCodeQualifier0x00(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("NO ADDITIONAL SENSE INFORMATION"));
		break;
	case 0x06:
		OutputErrorString(_T("I/O PROCESS TERMINATED"));
		break;
	case 0x11:
		OutputErrorString(_T("AUDIO PLAY OPERATION IN PROGRES"));
		break;
	case 0x12:
		OutputErrorString(_T("AUDIO PLAY OPERATION PAUSED"));
		break;
	case 0x13:
		OutputErrorString(_T("AUDIO PLAY OPERATION SUCCESSFULLY COMPLETED"));
		break;
	case 0x14:
		OutputErrorString(_T("AUDIO PLAY OPERATION STOPPED DUE TO ERROR"));
		break;
	case 0x15:
		OutputErrorString(_T("NO CURRENT AUDIO STATUS TO RETURN"));
		break;
	case 0x16:
		OutputErrorString(_T("OPERATION IN PROGRESS"));
		break;
	case 0x17:
		OutputErrorString(_T("CLEANING REQUESTED"));
		break;
	case 0x1d:
		OutputErrorString(_T("ATA PASS THROUGH INFORMATION AVAILABLE"));
		break;
	case 0x1e:
		OutputErrorString(_T("CONFLICTING SA CREATION REQUE"));
		break;
	case 0x1f:
		OutputErrorString(_T("LOGICAL UNIT TRANSITIONING TO ANOTHER POWER CONDITION"));
		break;
	case 0x20:
		OutputErrorString(_T("EXTENDED COPY INFORMATION AVAILABLE"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x01(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("NO INDEX/SECTOR SIGNAL"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x02(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("NO SEEK COMPLETE"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x03(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("PERIPHERAL DEVICE WRITE FAULT"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x04(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case SCSI_SENSEQ_CAUSE_NOT_REPORTABLE:
		OutputErrorString(_T("LUN_NOT_READY - CAUSE_NOT_REPORTABLE"));
		break;
	case SCSI_SENSEQ_BECOMING_READY:
		OutputErrorString(_T("LUN_NOT_READY - BECOMING_READY"));
		break;
	case SCSI_SENSEQ_INIT_COMMAND_REQUIRED:
		OutputErrorString(_T("LUN_NOT_READY - INIT_COMMAND_REQUIRED"));
		break;
	case SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED:
		OutputErrorString(_T("LUN_NOT_READY - MANUAL_INTERVENTION_REQUIRED"));
		break;
	case SCSI_SENSEQ_FORMAT_IN_PROGRESS:
		OutputErrorString(_T("LUN_NOT_READY - FORMAT_IN_PROGRESS"));
		break;
	case SCSI_SENSEQ_REBUILD_IN_PROGRESS:
		OutputErrorString(_T("LUN_NOT_READY - REBUILD_IN_PROGRESS"));
		break;
	case SCSI_SENSEQ_RECALCULATION_IN_PROGRESS:
		OutputErrorString(_T("LUN_NOT_READY - RECALCULATION_IN_PROGRESS"));
		break;
	case SCSI_SENSEQ_OPERATION_IN_PROGRESS:
		OutputErrorString(_T("LUN_NOT_READY - OPERATION_IN_PROGRESS"));
		break;
	case SCSI_SENSEQ_LONG_WRITE_IN_PROGRESS:
		OutputErrorString(_T("LUN_NOT_READY - LONG_WRITE_IN_PROGRESS"));
		break;
	case 0x09:
		OutputErrorString(_T("LUN_NOT_READY - SELF-TEST IN PROGRESS"));
		break;
	case 0x0a:
		OutputErrorString(_T("LUN_NOT_READY - ASYMMETRIC ACCESS STATE TRANSITION"));
		break;
	case 0x0b:
		OutputErrorString(_T("LUN_NOT_READY - TARGET PORT IN STANDBY STATE"));
		break;
	case 0x0c:
		OutputErrorString(_T("LUN_NOT_READY - TARGET PORT IN UNAVAILABLE STATE"));
		break;
	case 0x10:
		OutputErrorString(_T("LUN_NOT_READY - AUXILIARY MEMORY NOT ACCESSIBLE"));
		break;
	case 0x11:
		OutputErrorString(_T("LUN_NOT_READY - NOTIFY (ENABLE SPINUP) REQUIRED"));
		break;
	case 0x13:
		OutputErrorString(_T("LUN_NOT_READY - SA CREATION IN PROGRESS"));
		break;
	case 0x14:
		OutputErrorString(_T("LUN_NOT_READY - SPACE ALLOCATION IN PROGRESS"));
		break;
	case 0x1a:
		OutputErrorString(_T("LUN_NOT_READY - START STOP UNIT COMMAND IN PROGRESS"));
		break;
	case 0x1b:
		OutputErrorString(_T("LUN_NOT_READY - SANITIZE IN PROGRESS"));
		break;
	case 0x1c:
		OutputErrorString(_T("LUN_NOT_READY - ADDITIONAL POWER USE NOT YET GRANTED"));
		break;
	case 0x1d:
		OutputErrorString(_T("LUN_NOT_READY - CONFIGURATION IN PROGRESS"));
		break;
	case 0x1e:
		OutputErrorString(_T("LUN_NOT_READY - MICROCODE ACTIVATION REQUIRED"));
		break;
	case 0x1f:
		OutputErrorString(_T("LUN_NOT_READY - MICROCODE DOWNLOAD REQUIRED"));
		break;
	case 0x20:
		OutputErrorString(_T("LUN_NOT_READY - LOGICAL UNIT RESET REQUIRED"));
		break;
	case 0x21:
		OutputErrorString(_T("LUN_NOT_READY - HARD RESET REQUIRED"));
		break;
	case 0x22:
		OutputErrorString(_T("LUN_NOT_READY - POWER CYCLE REQUIRED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x05(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("LOGICAL UNIT DOES NOT RESPOND TO SELECTION"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x06(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("NO REFERENCE POSITION FOUND"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x07(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("MULTIPLE PERIPHERAL DEVICES SELECTED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x08(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case SCSI_SENSEQ_COMM_FAILURE:
		OutputErrorString(_T("LUN_COMMUNICATION - COMM_FAILURE"));
		break;
	case SCSI_SENSEQ_COMM_TIMEOUT:
		OutputErrorString(_T("LUN_COMMUNICATION - COMM_TIMEOUT"));
		break;
	case SCSI_SENSEQ_COMM_PARITY_ERROR:
		OutputErrorString(_T("LUN_COMMUNICATION - COMM_PARITY_ERROR"));
		break;
	case SCSI_SESNEQ_COMM_CRC_ERROR:
		OutputErrorString(_T("LUN_COMMUNICATION - COMM_CRC_ERROR"));
		break;
	case SCSI_SENSEQ_UNREACHABLE_TARGET:
		OutputErrorString(_T("LUN_COMMUNICATION - UNREACHABLE_TARGET"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x09(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("TRACK FOLLOWING ERROR"));
		break;
	case 0x01:
		OutputErrorString(_T("TRACKING SERVO FAILURE"));
		break;
	case 0x02:
		OutputErrorString(_T("FOCUS SERVO FAILURE"));
		break;
	case 0x03:
		OutputErrorString(_T("SPINDLE SERVO FAILURE"));
		break;
	case 0x04:
		OutputErrorString(_T("HEAD SELECT FAULT"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x0a(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("ERROR LOG OVERFLOW"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x0b(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("WARNING"));
		break;
	case 0x01:
		OutputErrorString(_T("WARNING - SPECIFIED TEMPERATURE EXCEEDED"));
		break;
	case 0x02:
		OutputErrorString(_T("WARNING - ENCLOSURE DEGRADED"));
		break;
	case 0x03:
		OutputErrorString(_T("WARNING - BACKGROUND SELF-TEST FAILED"));
		break;
	case 0x04:
		OutputErrorString(_T("WARNING - BACKGROUND PRE-SCAN DETECTED MEDIUM ERROR"));
		break;
	case 0x05:
		OutputErrorString(_T("WARNING - BACKGROUND MEDIUM SCAN DETECTED MEDIUM ERROR"));
		break;
	case 0x06:
		OutputErrorString(_T("WARNING - NON-VOLATILE CACHE NOW VOLATILE"));
		break;
	case 0x07:
		OutputErrorString(_T("WARNING - DEGRADED POWER TO NON-VOLATILE CACHE"));
		break;
	case 0x08:
		OutputErrorString(_T("WARNING - POWER LOSS EXPECTED"));
		break;
	case 0x09:
		OutputErrorString(_T("WARNING - DEVICE STATISTICS NOTIFICATION ACTIVE"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x0c(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("WRITE ERROR"));
		break;
	case 0x02:
		OutputErrorString(_T("WRITE ERROR - AUTO REALLOCATION FAILED"));
		break;
	case 0x03:
		OutputErrorString(_T("WRITE ERROR - RECOMMEND REASSIGNMENT"));
		break;
	case 0x04:
		OutputErrorString(_T("WRITE ERROR - COMPRESSION CHECK MISCOMPARE ERROR"));
		break;
	case 0x05:
		OutputErrorString(_T("WRITE ERROR - DATA EXPANSION OCCURRED DURING COMPRESSION"));
		break;
	case 0x06:
		OutputErrorString(_T("WRITE ERROR - BLOCK NOT COMPRESSIBLE"));
		break;
	case 0x07:
		OutputErrorString(_T("WRITE ERROR - RECOVERY NEEDED"));
		break;
	case 0x08:
		OutputErrorString(_T("WRITE ERROR - RECOVERY FAILED"));
		break;
	case SCSI_SENSEQ_LOSS_OF_STREAMING:
		OutputErrorString(_T("WRITE ERROR - LOSS OF STREAMING"));
		break;
	case SCSI_SENSEQ_PADDING_BLOCKS_ADDED:
		OutputErrorString(_T("WRITE ERROR - PADDING BLOCKS ADDED"));
		break;
	case 0x0b:
		OutputErrorString(_T("WRITE ERROR - AUXILIARY MEMORY WRITE ERROR"));
		break;
	case 0x0c:
		OutputErrorString(_T("WRITE ERROR - UNEXPECTED UNSOLICITED DATA"));
		break;
	case 0x0d:
		OutputErrorString(_T("WRITE ERROR - NOT ENOUGH UNSOLICITED DATA"));
		break;
	case 0x0f:
		OutputErrorString(_T("WRITE ERROR - DEFECTS IN ERROR WINDOW"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x0d(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("ERROR DETECTED BY THIRD PARTY TEMPORARY INITIATOR"));
		break;
	case 0x01:
		OutputErrorString(_T("THIRD PARTY DEVICE FAILURE"));
		break;
	case 0x02:
		OutputErrorString(_T("COPY TARGET DEVICE NOT REACHABLE"));
		break;
	case 0x03:
		OutputErrorString(_T("INCORRECT COPY TARGET DEVICE TYPE"));
		break;
	case 0x04:
		OutputErrorString(_T("COPY TARGET DEVICE DATA UNDERRUN"));
		break;
	case 0x05:
		OutputErrorString(_T("COPY TARGET DEVICE DATA OVERRUN"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x0e(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("INVALID INFORMATION UNIT"));
		break;
	case 0x01:
		OutputErrorString(_T("INFORMATION UNIT TOO SHORT"));
		break;
	case 0x02:
		OutputErrorString(_T("INFORMATION UNIT TOO LONG"));
		break;
	case 0x03:
		OutputErrorString(_T("INVALID FIELD IN COMMAND INFORMATION UNIT"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x10(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("ID CRC OR ECC ERROR"));
		break;
	case 0x01:
		OutputErrorString(_T("LOGICAL BLOCK GUARD CHECK FAIL"));
		break;
	case 0x02:
		OutputErrorString(_T("LOGICAL BLOCK APPLICATION TAG CHECK FAILED"));
		break;
	case 0x03:
		OutputErrorString(_T("LOGICAL BLOCK REFERENCE TAG CHECK FAILED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x11(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("UNRECOVERED READ ERROR"));
		break;
	case 0x01:
		OutputErrorString(_T("READ RETRIES EXHAUSTED"));
		break;
	case 0x02:
		OutputErrorString(_T("ERROR TOO LONG TO CORRECT"));
		break;
	case 0x03:
		OutputErrorString(_T("MULTIPLE READ ERRORS"));
		break;
	case 0x04:
		OutputErrorString(_T("UNRECOVERED READ ERROR - AUTO REALLOCATE FAILED"));
		break;
	case 0x05:
		OutputErrorString(_T("L-EC UNCORRECTABLE ERROR"));
		break;
	case 0x06:
		OutputErrorString(_T("CIRC UNRECOVERED ERROR"));
		break;
	case 0x07:
		OutputErrorString(_T("DATA RE-SYNCHRONIZATION ERROR"));
		break;
	case 0x0a:
		OutputErrorString(_T("MISCORRECTED ERROR"));
		break;
	case 0x0b:
		OutputErrorString(_T("UNRECOVERED READ ERROR - RECOMMEND REASSIGNMENT"));
		break;
	case 0x0c:
		OutputErrorString(_T("UNRECOVERED READ ERROR - RECOMMEND REWRITE THE DATA"));
		break;
	case 0x0d:
		OutputErrorString(_T("DE-COMPRESSION CRC ERROR"));
		break;
	case 0x0e:
		OutputErrorString(_T("CANNOT DECOMPRESS USING DECLARED ALGORITHM"));
		break;
	case 0x0f:
		OutputErrorString(_T("ERROR READING UPC/EAN NUMBER"));
		break;
	case 0x10:
		OutputErrorString(_T("ERROR READING ISRC NUMBER"));
		break;
	case 0x11:
		OutputErrorString(_T("READ ERROR - LOSS OF STREAMING"));
		break;
	case 0x12:
		OutputErrorString(_T("AUXILIARY MEMORY READ ERROR"));
		break;
	case 0x13:
		OutputErrorString(_T("READ ERROR - FAILED RETRANSMISSION REQUEST"));
		break;
	case 0x14:
		OutputErrorString(_T("READ ERROR - LBA MARKED BAD BY APPLICATION CLIENT"));
		break;
	case 0x15:
		OutputErrorString(_T("WRITE AFTER SANITIZE REQUIRED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x12(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("ADDRESS MARK NOT FOUND FOR ID FIELD"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x13(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("ADDRESS MARK NOT FOUND FOR DATA FIELD"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x14(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("TRACK_ERROR - RECORDED ENTITY NOT FOUND"));
		break;
	case 0x01:
		OutputErrorString(_T("TRACK_ERROR - RECORD NOT FOUND"));
		break;
	case 0x05:
		OutputErrorString(_T("TRACK_ERROR - RECORD NOT FOUND - RECOMMEND REASSIGNMENT"));
		break;
	case 0x06:
		OutputErrorString(_T("TRACK_ERROR - RECORD NOT FOUND - DATA AUTO-REALLOCATED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x15(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("SEEK_ERROR - RANDOM POSITIONING ERROR"));
		break;
	case 0x01:
		OutputErrorString(_T("SEEK_ERROR - MECHANICAL POSITIONING ERROR"));
		break;
	case 0x02:
		OutputErrorString(_T("SEEK_ERROR - POSITIONING ERROR DETECTED BY READ OF MEDIUM"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x16(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("DATA SYNCHRONIZATION MARK ERROR"));
		break;
	case 0x01:
		OutputErrorString(_T("DATA SYNC ERROR - DATA REWRITTEN"));
		break;
	case 0x02:
		OutputErrorString(_T("DATA SYNC ERROR - RECOMMEND REWRITE"));
		break;
	case 0x03:
		OutputErrorString(_T("DATA SYNC ERROR - DATA AUTO-REALLOCATED"));
		break;
	case 0x04:
		OutputErrorString(_T("DATA SYNC ERROR - RECOMMEND REASSIGNMENT"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x17(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("REC_DATA_NOECC - RECOVERED DATA WITH NO ERROR CORRECTION APPLIED"));
		break;
	case 0x01:
		OutputErrorString(_T("REC_DATA_NOECC - RECOVERED DATA WITH RETRIES"));
		break;
	case 0x02:
		OutputErrorString(_T("REC_DATA_NOECC - RECOVERED DATA WITH POSITIVE HEAD OFFSET"));
		break;
	case 0x03:
		OutputErrorString(_T("REC_DATA_NOECC - RECOVERED DATA WITH NEGATIVE HEAD OFFSET"));
		break;
	case 0x04:
		OutputErrorString(_T("REC_DATA_NOECC - RECOVERED DATA WITH RETRIES AND/OR CIRC APPLIED"));
		break;
	case 0x05:
		OutputErrorString(_T("REC_DATA_NOECC - RECOVERED DATA USING PREVIOUS SECTOR ID"));
		break;
	case 0x06:
		OutputErrorString(_T("REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - DATA AUTO-REALLOCATED"));
		break;
	case 0x07:
		OutputErrorString(_T("REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - RECOMMEND REASSIGNMENT"));
		break;
	case 0x08:
		OutputErrorString(_T("REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - RECOMMEND REWRITE"));
		break;
	case 0x09:
		OutputErrorString(_T("REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - DATA REWRITTEN"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x18(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("REC_DATA_ECC - RECOVERED DATA WITH ERROR CORRECTION APPLIED"));
		break;
	case 0x01:
		OutputErrorString(_T("REC_DATA_ECC - RECOVERED DATA WITH ERROR CORR. & RETRIES APPLIED"));
		break;
	case 0x02:
		OutputErrorString(_T("REC_DATA_ECC - RECOVERED DATA - DATA AUTO-REALLOCATED"));
		break;
	case 0x03:
		OutputErrorString(_T("REC_DATA_ECC - RECOVERED DATA WITH CIRC"));
		break;
	case 0x04:
		OutputErrorString(_T("REC_DATA_ECC - RECOVERED DATA WITH L-EC"));
		break;
	case 0x05:
		OutputErrorString(_T("REC_DATA_ECC - RECOVERED DATA - RECOMMEND REASSIGNMENT"));
		break;
	case 0x06:
		OutputErrorString(_T("REC_DATA_ECC - RECOVERED DATA - RECOMMEND REWRITE"));
		break;
	case 0x07:
		OutputErrorString(_T("REC_DATA_ECC - RECOVERED DATA WITH ECC - DATA REWRITTEN"));
		break;
	case 0x08:
		OutputErrorString(_T("REC_DATA_ECC - RECOVERED DATA WITH LINKING"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x19(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("DEFECT LIST ERROR"));
		break;
	case 0x01:
		OutputErrorString(_T("DEFECT LIST NOT AVAILABLE"));
		break;
	case 0x02:
		OutputErrorString(_T("DEFECT LIST ERROR IN PRIMARY LIST"));
		break;
	case 0x03:
		OutputErrorString(_T("DEFECT LIST ERROR IN GROWN LIST"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x1a(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("PARAMETER LIST LENGTH ERROR"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x1b(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("SYNCHRONOUS DATA TRANSFER ERROR"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x1c(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("DEFECT LIST NOT FOUND"));
		break;
	case 0x01:
		OutputErrorString(_T("PRIMARY DEFECT LIST NOT FOUND"));
		break;
	case 0x02:
		OutputErrorString(_T("GROWN DEFECT LIST NOT FOUND"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x1d(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("MISCOMPARE DURING VERIFY OPERATION"));
		break;
	case 0x01:
		OutputErrorString(_T("MISCOMPARE VERIFY OF UNMAPPED LBA"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x1e(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("RECOVERED ID WITH ECC CORRECTION"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x1f(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("PARTIAL DEFECT LIST TRANSFER"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x20(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("INVALID COMMAND OPERATION CODE"));
		break;
	case 0x01:
		OutputErrorString(_T("ILLEGAL_COMMAND - ACCESS DENIED - INITIATOR PENDING-ENROLLED"));
		break;
	case 0x02:
		OutputErrorString(_T("ILLEGAL_COMMAND - ACCESS DENIED - NO ACCESS RIGHTS"));
		break;
	case 0x03:
		OutputErrorString(_T("ILLEGAL_COMMAND - ACCESS DENIED - INVALID MGMT ID KEY"));
		break;
	case 0x08:
		OutputErrorString(_T("ILLEGAL_COMMAND - ACCESS DENIED - ENROLLMENT CONFLICT"));
		break;
	case 0x09:
		OutputErrorString(_T("ILLEGAL_COMMAND - ACCESS DENIED - INVALID LU IDENTIFIER"));
		break;
	case 0x0a:
		OutputErrorString(_T("ILLEGAL_COMMAND - ACCESS DENIED - INVALID PROXY TOKEN"));
		break;
	case 0x0b:
		OutputErrorString(_T("ILLEGAL_COMMAND - ACCESS DENIED - ACL LUN CONFLICT"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x21(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("LOGICAL BLOCK ADDRESS OUT OF RANGE"));
		break;
	case SCSI_SENSEQ_ILLEGAL_ELEMENT_ADDR:
		OutputErrorString(_T("ILLEGAL_BLOCK - MISCOMPARE DURING VERIFY OPERATION"));
		break;
	case 0x02:
		OutputErrorString(_T("ILLEGAL_BLOCK - INVALID ADDRESS FOR WRITE"));
		break;
	case 0x03:
		OutputErrorString(_T("ILLEGAL_BLOCK - INVALID WRITE CROSSING LAYER JUMP"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x22(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("ILLEGAL FUNCTION"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x23(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("INVALID TOKEN OPERATION, CAUSE NOT REPORTABLE"));
		break;
	case 0x01:
		OutputErrorString(_T("INVALID TOKEN OPERATION, UNSUPPORTED TOKEN TYPE"));
		break;
	case 0x02:
		OutputErrorString(_T("INVALID TOKEN OPERATION, REMOTE TOKEN USAGE NOT SUPPORTED"));
		break;
	case 0x03:
		OutputErrorString(_T("INVALID TOKEN OPERATION, REMOTE ROD TOKEN CREATION NOT SUPPORTED"));
		break;
	case 0x04:
		OutputErrorString(_T("INVALID TOKEN OPERATION, TOKEN UNKNOWN"));
		break;
	case 0x05:
		OutputErrorString(_T("INVALID TOKEN OPERATION, TOKEN CORRUPT"));
		break;
	case 0x06:
		OutputErrorString(_T("INVALID TOKEN OPERATION, TOKEN REVOKED"));
		break;
	case 0x07:
		OutputErrorString(_T("INVALID TOKEN OPERATION, TOKEN EXPIRED"));
		break;
	case 0x08:
		OutputErrorString(_T("INVALID TOKEN OPERATION, TOKEN CANCELLED"));
		break;
	case 0x09:
		OutputErrorString(_T("INVALID TOKEN OPERATION, TOKEN DELETED"));
		break;
	case 0x0a:
		OutputErrorString(_T("INVALID TOKEN OPERATION, INVALID TOKEN LENGTH"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x24(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("INVALID FIELD IN CDB"));
		break;
	case 0x01:
		OutputErrorString(_T("CDB DECRYPTION ERROR"));
		break;
	case 0x08:
		OutputErrorString(_T("INVALID XCDB"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x25(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("LOGICAL UNIT NOT SUPPORTED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x26(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("INVALID FIELD IN PARAMETER LIST"));
		break;
	case 0x01:
		OutputErrorString(_T("PARAMETER NOT SUPPORTED"));
		break;
	case 0x02:
		OutputErrorString(_T("PARAMETER VALUE INVALID"));
		break;
	case 0x03:
		OutputErrorString(_T("THRESHOLD PARAMETERS NOT SUPPORTED"));
		break;
	case 0x04:
		OutputErrorString(_T("INVALID RELEASE OF PERSISTENT RESERVATION"));
		break;
	case 0x05:
		OutputErrorString(_T("DATA DECRYPTION ERROR"));
		break;
	case 0x06:
		OutputErrorString(_T("TOO MANY TARGET DESCRIPTORS"));
		break;
	case 0x07:
		OutputErrorString(_T("UNSUPPORTED SEGMENT DESCRIPTOR TYPE CODE"));
		break;
	case 0x08:
		OutputErrorString(_T("TOO MANY SEGMENT DESCRIPTORS"));
		break;
	case 0x09:
		OutputErrorString(_T("UNSUPPORTED SEGMENT DESCRIPTOR TYPE CODE"));
		break;
	case 0x0a:
		OutputErrorString(_T("UNEXPECTED INEXACT SEGMENT"));
		break;
	case 0x0b:
		OutputErrorString(_T("INLINE DATA LENGTH EXCEEDED"));
		break;
	case 0x0c:
		OutputErrorString(_T("INVALID OPERATION FOR COPY SOURCE OR DESTINATION"));
		break;
	case 0x0d:
		OutputErrorString(_T("COPY SEGMENT GRANULARITY VIOLATION"));
		break;
	case 0x0e:
		OutputErrorString(_T("INVALID PARAMETER WHILE PORT IS ENABLED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x27(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("WRITE PROTECTED"));
		break;
	case 0x01:
		OutputErrorString(_T("HARDWARE WRITE PROTECTED"));
		break;
	case 0x02:
		OutputErrorString(_T("LOGICAL UNIT SOFTWARE WRITE PROTECTED"));
		break;
	case 0x03:
		OutputErrorString(_T("ASSOCIATED WRITE PROTECT"));
		break;
	case 0x04:
		OutputErrorString(_T("PERSISTENT WRITE PROTECT"));
		break;
	case 0x05:
		OutputErrorString(_T("PERMANENT WRITE PROTECT"));
		break;
	case 0x06:
		OutputErrorString(_T("CONDITIONAL WRITE PROTECT"));
		break;
	case 0x07:
		OutputErrorString(_T("SPACE ALLOCATION FAILED WRITE PROTECT"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x28(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("NOT READY TO READY CHANGE, MEDIUM MAY HAVE CHANGED"));
		break;
	case 0x01:
		OutputErrorString(_T("IMPORT OR EXPORT ELEMENT ACCESSED"));
		break;
	case 0x02:
		OutputErrorString(_T("FORMAT-LAYER MAY HAVE CHANGED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x29(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("POWER ON, RESET, OR BUS DEVICE RESET OCCURRED"));
		break;
	case 0x01:
		OutputErrorString(_T("POWER ON OCCURRED"));
		break;
	case 0x02:
		OutputErrorString(_T("SCSI BUS RESET OCCURRED"));
		break;
	case 0x03:
		OutputErrorString(_T("BUS DEVICE RESET FUNCTION OCCURRED"));
		break;
	case 0x04:
		OutputErrorString(_T("DEVICE INTERNAL RESET"));
		break;
	case 0x05:
		OutputErrorString(_T("TRANSCEIVER MODE CHANGED TO SINGLE-ENDED"));
		break;
	case 0x06:
		OutputErrorString(_T("TRANSCEIVER MODE CHANGED TO LVD"));
		break;
	case 0x07:
		OutputErrorString(_T("I_T NEXUS LOSS OCCURRED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x2a(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("PARAMETERS CHANGED"));
		break;
	case 0x01:
		OutputErrorString(_T("MODE PARAMETERS CHANGED"));
		break;
	case 0x02:
		OutputErrorString(_T("LOG PARAMETERS CHANGED"));
		break;
	case 0x03:
		OutputErrorString(_T("RESERVATIONS PREEMPTED"));
		break;
	case 0x04:
		OutputErrorString(_T("RESERVATIONS RELEASED"));
		break;
	case 0x05:
		OutputErrorString(_T("REGISTRATIONS PREEMPTED"));
		break;
	case 0x06:
		OutputErrorString(_T("ASYMMETRIC ACCESS STATE CHANGED"));
		break;
	case 0x07:
		OutputErrorString(_T("IMPLICIT ASYMMETRIC ACCESS STATE TRANSITION FAILED"));
		break;
	case 0x08:
		OutputErrorString(_T("PRIORITY CHANGED"));
		break;
	case 0x09:
		OutputErrorString(_T("CAPACITY DATA HAS CHANGED"));
		break;
	case 0x0a:
		OutputErrorString(_T("ERROR HISTORY I_T NEXUS CLEARED"));
		break;
	case 0x0b:
		OutputErrorString(_T("ERROR HISTORY SNAPSHOT RELEASED"));
		break;
	case 0x10:
		OutputErrorString(_T("TIMESTAMP CHANGED"));
		break;
	case 0x14:
		OutputErrorString(_T("SA CREATION CAPABILITIES DATA HAS CHANGED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x2b(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("COPY CANNOT EXECUTE SINCE HOST CANNOT DISCONNECT"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x2c(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("COMMAND SEQUENCE ERROR"));
		break;
	case 0x03:
		OutputErrorString(_T("CURRENT PROGRAM AREA IS NOT EMPTY"));
		break;
	case 0x04:
		OutputErrorString(_T("CURRENT PROGRAM AREA IS EMPTY"));
		break;
	case 0x06:
		OutputErrorString(_T("PERSISTENT PREVENT CONFLICT"));
		break;
	case 0x07:
		OutputErrorString(_T("PREVIOUS BUSY STATUS"));
		break;
	case 0x08:
		OutputErrorString(_T("PREVIOUS TASK SET FULL STATUS"));
		break;
	case 0x09:
		OutputErrorString(_T("PREVIOUS RESERVATION CONFLICT STATUS"));
		break;
	case 0x0c:
		OutputErrorString(_T("ORWRITE GENERATION DOES NOT MATCH"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x2e(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("INSUFFICIENT TIME FOR OPERATION"));
		break;
	case 0x01:
		OutputErrorString(_T("COMMAND TIMEOUT BEFORE PROCESSING"));
		break;
	case 0x02:
		OutputErrorString(_T("COMMAND TIMEOUT DURING PROCESSING"));
		break;
	case 0x03:
		OutputErrorString(_T("COMMAND TIMEOUT DURING PROCESSING DUE TO ERROR RECOVERY"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x2f(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("COMMANDS CLEARED BY ANOTHER INITIATOR"));
		break;
	case 0x01:
		OutputErrorString(_T("COMMANDS CLEARED BY POWER LOSS NOTIFICATION"));
		break;
	case 0x02:
		OutputErrorString(_T("COMMANDS CLEARED BY DEVICE SERVER"));
		break;
	case 0x03:
		OutputErrorString(_T("SOME COMMANDS CLEARED BY QUEUING LAYER EVENT"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x30(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("INCOMPATIBLE MEDIUM INSTALLED"));
		break;
	case 0x01:
		OutputErrorString(_T("CANNOT READ MEDIUM - UNKNOWN FORMAT"));
		break;
	case 0x02:
		OutputErrorString(_T("CANNOT READ MEDIUM - INCOMPATIBLE FORMAT"));
		break;
	case 0x03:
		OutputErrorString(_T("CLEANING CARTRIDGE INSTALLED"));
		break;
	case 0x04:
		OutputErrorString(_T("CANNOT WRITE MEDIUM - UNKNOWN FORMAT"));
		break;
	case 0x05:
		OutputErrorString(_T("CANNOT WRITE MEDIUM - INCOMPATIBLE FORMAT"));
		break;
	case 0x06:
		OutputErrorString(_T("CANNOT FORMAT MEDIUM - INCOMPATIBLE MEDIUM"));
		break;
	case 0x07:
		OutputErrorString(_T("CLEANING FAILURE"));
		break;
	case 0x08:
		OutputErrorString(_T("CANNOT WRITE - APPLICATION CODE MISMATCH"));
		break;
	case 0x09:
		OutputErrorString(_T("CURRENT SESSION NOT FIXATED FOR APPEND"));
		break;
	case 0x0a:
		OutputErrorString(_T("CLEANING REQUEST REJECTED"));
		break;
	case 0x10:
		OutputErrorString(_T("MEDIUM NOT FORMATTED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x31(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("MEDIUM FORMAT CORRUPTED"));
		break;
	case 0x01:
		OutputErrorString(_T("FORMAT COMMAND FAILED"));
		break;
	case 0x02:
		OutputErrorString(_T("ZONED FORMATTING FAILED DUE TO SPARE LINKING"));
		break;
	case 0x03:
		OutputErrorString(_T("SANITIZE COMMAND FAILED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x32(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("NO DEFECT SPARE LOCATION AVAILABLE"));
		break;
	case 0x01:
		OutputErrorString(_T("DEFECT LIST UPDATE FAILURE"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x34(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("ENCLOSURE FAILURE"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x35(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("ENCLOSURE SERVICES FAILURE"));
		break;
	case 0x01:
		OutputErrorString(_T("UNSUPPORTED ENCLOSURE FUNCTION"));
		break;
	case 0x02:
		OutputErrorString(_T("ENCLOSURE SERVICES UNAVAILABLE"));
		break;
	case 0x03:
		OutputErrorString(_T("ENCLOSURE SERVICES TRANSFER FAILURE"));
		break;
	case 0x04:
		OutputErrorString(_T("ENCLOSURE SERVICES TRANSFER REFUSED"));
		break;
	case 0x05:
		OutputErrorString(_T("ENCLOSURE SERVICES CHECKSUM ERROR"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x37(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("ROUNDED PARAMETER"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x38(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x07:
		OutputErrorString(_T("THIN PROVISIONING SOFT THRESHOLD REACHED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x39(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("SAVING PARAMETERS NOT SUPPORTED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x3a(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("MEDIUM NOT PRESENT"));
		break;
	case 0x01:
		OutputErrorString(_T("MEDIUM NOT PRESENT - TRAY CLOSED"));
		break;
	case 0x02:
		OutputErrorString(_T("MEDIUM NOT PRESENT - TRAY OPEN"));
		break;
	case 0x03:
		OutputErrorString(_T("MEDIUM NOT PRESENT - LOADABLE"));
		break;
	case 0x04:
		OutputErrorString(_T("MEDIUM NOT PRESENT - MEDIUM AUXILIARY MEMORY ACCESSIBLE"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x3b(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case SCSI_SENSEQ_DESTINATION_FULL:
		OutputErrorString(_T("MEDIUM DESTINATION ELEMENT FULL"));
		break;
	case SCSI_SENSEQ_SOURCE_EMPTY:
		OutputErrorString(_T("MEDIUM SOURCE ELEMENT EMPTY"));
		break;
	case 0x0f:
		OutputErrorString(_T("END OF MEDIUM REACHED"));
		break;
	case 0x11:
		OutputErrorString(_T("MEDIUM MAGAZINE NOT ACCESSIBLE"));
		break;
	case 0x12:
		OutputErrorString(_T("MEDIUM MAGAZINE REMOVED"));
		break;
	case 0x13:
		OutputErrorString(_T("MEDIUM MAGAZINE INSERTED"));
		break;
	case 0x14:
		OutputErrorString(_T("MEDIUM MAGAZINE LOCKED"));
		break;
	case 0x15:
		OutputErrorString(_T("MEDIUM MAGAZINE UNLOCKED"));
		break;
	case 0x16:
		OutputErrorString(_T("MECHANICAL POSITIONING OR CHANGER ERROR"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x3d(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("INVALID BITS IN IDENTIFY MESSAGE"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x3e(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("LOGICAL UNIT HAS NOT SELF-CONFIGURED YET"));
		break;
	case 0x01:
		OutputErrorString(_T("LOGICAL UNIT FAILURE"));
		break;
	case 0x02:
		OutputErrorString(_T("TIMEOUT ON LOGICAL UNIT"));
		break;
	case 0x03:
		OutputErrorString(_T("LOGICAL UNIT FAILED SELF-TEST"));
		break;
	case 0x04:
		OutputErrorString(_T("LOGICAL UNIT UNABLE TO UPDATE SELF-TEST LOG"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x3f(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case SCSI_SENSEQ_TARGET_OPERATING_CONDITIONS_CHANGED:
		OutputErrorString(_T("TARGET_OPERATING_CONDITIONS_CHANGED"));
		break;
	case SCSI_SENSEQ_MICROCODE_CHANGED:
		OutputErrorString(_T("MICROCODE_CHANGED"));
		break;
	case SCSI_SENSEQ_OPERATING_DEFINITION_CHANGED:
		OutputErrorString(_T("OPERATING_DEFINITION_CHANGED"));
		break;
	case SCSI_SENSEQ_INQUIRY_DATA_CHANGED:
		OutputErrorString(_T("INQUIRY_DATA_CHANGED"));
		break;
	case SCSI_SENSEQ_COMPONENT_DEVICE_ATTACHED:
		OutputErrorString(_T("COMPONENT_DEVICE_ATTACHED"));
		break;
	case SCSI_SENSEQ_DEVICE_IDENTIFIER_CHANGED:
		OutputErrorString(_T("DEVICE_IDENTIFIER_CHANGED"));
		break;
	case SCSI_SENSEQ_REDUNDANCY_GROUP_MODIFIED:
		OutputErrorString(_T("REDUNDANCY_GROUP_MODIFIED"));
		break;
	case SCSI_SENSEQ_REDUNDANCY_GROUP_DELETED:
		OutputErrorString(_T("REDUNDANCY_GROUP_DELETED"));
		break;
	case SCSI_SENSEQ_SPARE_MODIFIED:
		OutputErrorString(_T("SPARE_MODIFIED"));
		break;
	case SCSI_SENSEQ_SPARE_DELETED:
		OutputErrorString(_T("SPARE_DELETED"));
		break;
	case SCSI_SENSEQ_VOLUME_SET_MODIFIED:
		OutputErrorString(_T("VOLUME_SET_MODIFIED"));
		break;
	case SCSI_SENSEQ_VOLUME_SET_DELETED:
		OutputErrorString(_T("VOLUME SET DELETED"));
		break;
	case SCSI_SENSEQ_VOLUME_SET_DEASSIGNED:
		OutputErrorString(_T("VOLUME SET DEASSIGNED"));
		break;
	case SCSI_SENSEQ_VOLUME_SET_REASSIGNED:
		OutputErrorString(_T("VOLUME SET REASSIGNED"));
		break;
	case SCSI_SENSEQ_REPORTED_LUNS_DATA_CHANGED:
		OutputErrorString(_T("REPORTED_LUNS_DATA_CHANGED"));
		break;
	case SCSI_SENSEQ_ECHO_BUFFER_OVERWRITTEN:
		OutputErrorString(_T("ECHO_BUFFER_OVERWRITTEN"));
		break;
	case SCSI_SENSEQ_MEDIUM_LOADABLE:
		OutputErrorString(_T("MEDIUM LOADABLE"));
		break;
	case SCSI_SENSEQ_MEDIUM_AUXILIARY_MEMORY_ACCESSIBLE:
		OutputErrorString(_T("MEDIUM_AUXILIARY_MEMORY_ACCESSIBLE"));
		break;
	case 0x12:
		OutputErrorString(_T("iSCSI IP ADDRESS ADDED"));
		break;
	case 0x13:
		OutputErrorString(_T("iSCSI IP ADDRESS REMOVED"));
		break;
	case 0x14:
		OutputErrorString(_T("iSCSI IP ADDRESS CHANGED"));
		break;
	case 0x15:
		OutputErrorString(_T("INSPECT REFERRALS SENSE DESCRIPTORS"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x40(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("RAM FAILURE"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x41(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("DATA PATH FAILURE"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x42(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("POWER-ON OR SELF-TEST FAILURE"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x43(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("MESSAGE ERROR"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x44(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("INTERNAL TARGET FAILURE"));
		break;
	case 0x01:
		OutputErrorString(_T("PERSISTENT RESERVATION INFORMATION LOST"));
		break;
	case 0x71:
		OutputErrorString(_T("ATA DEVICE FAILED SET FEATURES"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x45(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("SELECT OR RESELECT FAILURE"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x46(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("UNSUCCESSFUL SOFT RESET"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x47(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("SCSI PARITY ERROR"));
		break;
	case 0x01:
		OutputErrorString(_T("DATA PHASE CRC ERROR DETECTED"));
		break;
	case 0x02:
		OutputErrorString(_T("SCSI PARITY ERROR DETECTED DURING ST DATA PHASE"));
		break;
	case 0x03:
		OutputErrorString(_T("INFORMATION UNIT iuCRC ERROR DETECTED"));
		break;
	case 0x04:
		OutputErrorString(_T("ASYNCHRONOUS INFORMATION PROTECTION ERROR DETECTED"));
		break;
	case 0x05:
		OutputErrorString(_T("PROTOCOL SERVICE CRC ERROR"));
		break;
	case 0x06:
		OutputErrorString(_T("PHY TEST FUNCTION IN PROGRESS"));
		break;
	case 0x7f:
		OutputErrorString(_T("SOME COMMANDS CLEARED BY ISCSI PROTOCOL EVENT"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x48(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("INITIATOR DETECTED ERROR MESSAGE RECEIVED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x49(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("INVALID MESSAGE ERROR"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x4a(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("COMMAND PHASE ERROR"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x4b(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("DATA PHASE ERROR"));
		break;
	case 0x01:
		OutputErrorString(_T("INVALID TARGET PORT TRANSFER TAG RECEIVED"));
		break;
	case 0x02:
		OutputErrorString(_T("TOO MUCH WRITE DATA"));
		break;
	case 0x03:
		OutputErrorString(_T("ACK/NAK TIMEOUT"));
		break;
	case 0x04:
		OutputErrorString(_T("NAK RECEIVED"));
		break;
	case 0x05:
		OutputErrorString(_T("DATA OFFSET ERROR"));
		break;
	case 0x06:
		OutputErrorString(_T("INITIATOR RESPONSE TIMEOUT"));
		break;
	case 0x07:
		OutputErrorString(_T("CONNECTION LOST"));
		break;
	case 0x08:
		OutputErrorString(_T("DATA-IN BUFFER OVERFLOW - DATA BUFFER SIZE"));
		break;
	case 0x09:
		OutputErrorString(_T("DATA-IN BUFFER OVERFLOW - DATA BUFFER DESCRIPTOR AREA"));
		break;
	case 0x0a:
		OutputErrorString(_T("DATA-IN BUFFER ERROR"));
		break;
	case 0x0b:
		OutputErrorString(_T("DATA-OUT BUFFER OVERFLOW - DATA BUFFER SIZE"));
		break;
	case 0x0c:
		OutputErrorString(_T("DATA-OUT BUFFER OVERFLOW - DATA BUFFER DESCRIPTOR AREA"));
		break;
	case 0x0d:
		OutputErrorString(_T("DATA-OUT BUFFER ERROR"));
		break;
	case 0x0e:
		OutputErrorString(_T("PCIE FABRIC ERROR"));
		break;
	case 0x0f:
		OutputErrorString(_T("PCIE COMPLETION TIMEOUT"));
		break;
	case 0x10:
		OutputErrorString(_T("PCIE COMPLETER ABORT"));
		break;
	case 0x11:
		OutputErrorString(_T("PCIE POISONED TLP RECEIVED"));
		break;
	case 0x12:
		OutputErrorString(_T("PCIE ECRC CHECK FAILED"));
		break;
	case 0x13:
		OutputErrorString(_T("PCIE UNSUPPORTED REQUEST"));
		break;
	case 0x14:
		OutputErrorString(_T("PCIE ACS VIOLATION"));
		break;
	case 0x15:
		OutputErrorString(_T("PCIE TLP PREFIX BLOCKED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x4c(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("LOGICAL UNIT FAILED SELF-CONFIGURATION"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x4e(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("OVERLAPPED COMMANDS ATTEMPTED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x51(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("ERASE FAILURE"));
		break;
	case 0x01:
		OutputErrorString(_T("ERASE FAILURE - INCOMPLETE ERASE OPERATION DETECTED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x53(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("MEDIA LOAD OR EJECT FAILED"));
		break;
	case 0x02:
		OutputErrorString(_T("MEDIUM REMOVAL PREVENTED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x55(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x01:
		OutputErrorString(_T("SYSTEM BUFFER FULL"));
		break;
	case 0x02:
		OutputErrorString(_T("INSUFFICIENT RESERVATION RESOURCES"));
		break;
	case 0x03:
		OutputErrorString(_T("INSUFFICIENT RESOURCES"));
		break;
	case 0x04:
		OutputErrorString(_T("INSUFFICIENT REGISTRATION RESOURCES"));
		break;
	case 0x05:
		OutputErrorString(_T("INSUFFICIENT ACCESS CONTROL RESOURCES"));
		break;
	case 0x06:
		OutputErrorString(_T("AUXILIARY MEMORY OUT OF SPACE"));
		break;
	case 0x0b:
		OutputErrorString(_T("INSUFFICIENT POWER FOR OPERATION"));
		break;
	case 0x0c:
		OutputErrorString(_T("INSUFFICIENT RESOURCES TO CREATE ROD"));
		break;
	case 0x0d:
		OutputErrorString(_T("INSUFFICIENT RESOURCES TO CREATE ROD TOKEN"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x57(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("UNABLE TO RECOVER TABLE-OF-CONTENTS"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x58(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("GENERATION DOES NOT EXIST"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x59(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("UPDATED BLOCK READ"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x5a(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case SCSI_SENSEQ_STATE_CHANGE_INPUT:
		OutputErrorString(_T("OPERATOR REQUEST OR STATE CHANGE INPUT"));
		break;
	case SCSI_SENSEQ_MEDIUM_REMOVAL:
		OutputErrorString(_T("OPERATOR MEDIUM REMOVAL REQUEST"));
		break;
	case SCSI_SENSEQ_WRITE_PROTECT_ENABLE:
		OutputErrorString(_T("OPERATOR SELECTED WRITE PROTECT"));
		break;
	case SCSI_SENSEQ_WRITE_PROTECT_DISABLE:
		OutputErrorString(_T("OPERATOR SELECTED WRITE PERMIT"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x5b(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("LOG EXCEPTION"));
		break;
	case 0x01:
		OutputErrorString(_T("THRESHOLD CONDITION MET"));
		break;
	case 0x02:
		OutputErrorString(_T("LOG COUNTER AT MAXIMUM"));
		break;
	case 0x03:
		OutputErrorString(_T("LOG LIST CODES EXHAUSTED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x5c(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("RPL STATUS CHANGE"));
		break;
	case 0x01:
		OutputErrorString(_T("SPINDLES SYNCHRONIZED"));
		break;
	case 0x02:
		OutputErrorString(_T("SPINDLES NOT SYNCHRONIZED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x5d(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("FAILURE PREDICTION THRESHOLD EXCEEDED"));
		break;
	case 0x01:
		OutputErrorString(_T("MEDIA FAILURE PREDICTION THRESHOLD EXCEEDED"));
		break;
	case 0x02:
		OutputErrorString(_T("LOGICAL UNIT FAILURE PREDICTION THRESHOLD EXCEEDED"));
		break;
	case 0x03:
		OutputErrorString(_T("SPARE AREA EXHAUSTION PREDICTION THRESHOLD EXCEEDED"));
		break;
	case 0x10:
		OutputErrorString(_T("HARDWARE IMPENDING FAILURE GENERAL HARD DRIVE FAILURE"));
		break;
	case 0x11:
		OutputErrorString(_T("HARDWARE IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH"));
		break;
	case 0x12:
		OutputErrorString(_T("HARDWARE IMPENDING FAILURE DATA ERROR RATE TOO HIGH"));
		break;
	case 0x13:
		OutputErrorString(_T("HARDWARE IMPENDING FAILURE SEEK ERROR RATE TOO HIGH"));
		break;
	case 0x14:
		OutputErrorString(_T("HARDWARE IMPENDING FAILURE TOO MANY BLOCK REASSIGNS"));
		break;
	case 0x15:
		OutputErrorString(_T("HARDWARE IMPENDING FAILURE ACCESS TIMES TOO HIGH"));
		break;
	case 0x16:
		OutputErrorString(_T("HARDWARE IMPENDING FAILURE START UNIT TIMES TOO HIGH"));
		break;
	case 0x17:
		OutputErrorString(_T("HARDWARE IMPENDING FAILURE CHANNEL PARAMETRICS"));
		break;
	case 0x18:
		OutputErrorString(_T("HARDWARE IMPENDING FAILURE CONTROLLER DETECTED"));
		break;
	case 0x19:
		OutputErrorString(_T("HARDWARE IMPENDING FAILURE THROUGHPUT PERFORMANCE"));
		break;
	case 0x1a:
		OutputErrorString(_T("HARDWARE IMPENDING FAILURE SEEK TIME PERFORMANCE"));
		break;
	case 0x1b:
		OutputErrorString(_T("HARDWARE IMPENDING FAILURE SPIN-UP RETRY COUNT"));
		break;
	case 0x1c:
		OutputErrorString(_T("HARDWARE IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT"));
		break;
	case 0x20:
		OutputErrorString(_T("CONTROLLER IMPENDING FAILURE GENERAL HARD DRIVE FAILURE"));
		break;
	case 0x21:
		OutputErrorString(_T("CONTROLLER IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH"));
		break;
	case 0x22:
		OutputErrorString(_T("CONTROLLER IMPENDING FAILURE DATA ERROR RATE TOO HIGH"));
		break;
	case 0x23:
		OutputErrorString(_T("CONTROLLER IMPENDING FAILURE SEEK ERROR RATE TOO HIGH"));
		break;
	case 0x24:
		OutputErrorString(_T("CONTROLLER IMPENDING FAILURE TOO MANY BLOCK REASSIGNS"));
		break;
	case 0x25:
		OutputErrorString(_T("CONTROLLER IMPENDING FAILURE ACCESS TIMES TOO HIGH"));
		break;
	case 0x26:
		OutputErrorString(_T("CONTROLLER IMPENDING FAILURE START UNIT TIMES TOO HIGH"));
		break;
	case 0x27:
		OutputErrorString(_T("CONTROLLER IMPENDING FAILURE CHANNEL PARAMETRICS"));
		break;
	case 0x28:
		OutputErrorString(_T("CONTROLLER IMPENDING FAILURE CONTROLLER DETECTED"));
		break;
	case 0x29:
		OutputErrorString(_T("CONTROLLER IMPENDING FAILURE THROUGHPUT PERFORMANCE"));
		break;
	case 0x2a:
		OutputErrorString(_T("CONTROLLER IMPENDING FAILURE SEEK TIME PERFORMANCE"));
		break;
	case 0x2b:
		OutputErrorString(_T("CONTROLLER IMPENDING FAILURE SPIN-UP RETRY COUNT"));
		break;
	case 0x2c:
		OutputErrorString(_T("CONTROLLER IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT"));
		break;
	case 0x30:
		OutputErrorString(_T("DATA CHANNEL IMPENDING FAILURE GENERAL HARD DRIVE FAILURE"));
		break;
	case 0x31:
		OutputErrorString(_T("DATA CHANNEL IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH"));
		break;
	case 0x32:
		OutputErrorString(_T("DATA CHANNEL IMPENDING FAILURE DATA ERROR RATE TOO HIGH"));
		break;
	case 0x33:
		OutputErrorString(_T("DATA CHANNEL IMPENDING FAILURE SEEK ERROR RATE TOO HIGH"));
		break;
	case 0x34:
		OutputErrorString(_T("DATA CHANNEL IMPENDING FAILURE TOO MANY BLOCK REASSIGNS"));
		break;
	case 0x35:
		OutputErrorString(_T("DATA CHANNEL IMPENDING FAILURE ACCESS TIMES TOO HIGH"));
		break;
	case 0x36:
		OutputErrorString(_T("DATA CHANNEL IMPENDING FAILURE START UNIT TIMES TOO HIGH"));
		break;
	case 0x37:
		OutputErrorString(_T("DATA CHANNEL IMPENDING FAILURE CHANNEL PARAMETRICS"));
		break;
	case 0x38:
		OutputErrorString(_T("DATA CHANNEL IMPENDING FAILURE CONTROLLER DETECTED"));
		break;
	case 0x39:
		OutputErrorString(_T("DATA CHANNEL IMPENDING FAILURE THROUGHPUT PERFORMANCE"));
		break;
	case 0x3a:
		OutputErrorString(_T("DATA CHANNEL IMPENDING FAILURE SEEK TIME PERFORMANCE"));
		break;
	case 0x3b:
		OutputErrorString(_T("DATA CHANNEL IMPENDING FAILURE SPIN-UP RETRY COUNT"));
		break;
	case 0x3c:
		OutputErrorString(_T("DATA CHANNEL IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT"));
		break;
	case 0x40:
		OutputErrorString(_T("SERVO IMPENDING FAILURE GENERAL HARD DRIVE FAILURE"));
		break;
	case 0x41:
		OutputErrorString(_T("SERVO IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH"));
		break;
	case 0x42:
		OutputErrorString(_T("SERVO IMPENDING FAILURE DATA ERROR RATE TOO HIGH"));
		break;
	case 0x43:
		OutputErrorString(_T("SERVO IMPENDING FAILURE SEEK ERROR RATE TOO HIGH"));
		break;
	case 0x44:
		OutputErrorString(_T("SERVO IMPENDING FAILURE TOO MANY BLOCK REASSIGNS"));
		break;
	case 0x45:
		OutputErrorString(_T("SERVO IMPENDING FAILURE ACCESS TIMES TOO HIGH"));
		break;
	case 0x46:
		OutputErrorString(_T("SERVO IMPENDING FAILURE START UNIT TIMES TOO HIGH"));
		break;
	case 0x47:
		OutputErrorString(_T("SERVO IMPENDING FAILURE CHANNEL PARAMETRICS"));
		break;
	case 0x48:
		OutputErrorString(_T("SERVO IMPENDING FAILURE CONTROLLER DETECTED"));
		break;
	case 0x49:
		OutputErrorString(_T("SERVO IMPENDING FAILURE THROUGHPUT PERFORMANCE"));
		break;
	case 0x4a:
		OutputErrorString(_T("SERVO IMPENDING FAILURE SEEK TIME PERFORMANCE"));
		break;
	case 0x4b:
		OutputErrorString(_T("SERVO IMPENDING FAILURE SPIN-UP RETRY COUNT"));
		break;
	case 0x4c:
		OutputErrorString(_T("SERVO IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT"));
		break;
	case 0x50:
		OutputErrorString(_T("SPINDLE IMPENDING FAILURE GENERAL HARD DRIVE FAILURE"));
		break;
	case 0x51:
		OutputErrorString(_T("SPINDLE IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH"));
		break;
	case 0x52:
		OutputErrorString(_T("SPINDLE IMPENDING FAILURE DATA ERROR RATE TOO HIGH"));
		break;
	case 0x53:
		OutputErrorString(_T("SPINDLE IMPENDING FAILURE SEEK ERROR RATE TOO HIGH"));
		break;
	case 0x54:
		OutputErrorString(_T("SPINDLE IMPENDING FAILURE TOO MANY BLOCK REASSIGNS"));
		break;
	case 0x55:
		OutputErrorString(_T("SPINDLE IMPENDING FAILURE ACCESS TIMES TOO HIGH"));
		break;
	case 0x56:
		OutputErrorString(_T("SPINDLE IMPENDING FAILURE START UNIT TIMES TOO HIGH"));
		break;
	case 0x57:
		OutputErrorString(_T("SPINDLE IMPENDING FAILURE CHANNEL PARAMETRICS"));
		break;
	case 0x58:
		OutputErrorString(_T("SPINDLE IMPENDING FAILURE CONTROLLER DETECTED"));
		break;
	case 0x59:
		OutputErrorString(_T("SPINDLE IMPENDING FAILURE THROUGHPUT PERFORMANCE"));
		break;
	case 0x5a:
		OutputErrorString(_T("SPINDLE IMPENDING FAILURE SEEK TIME PERFORMANCE"));
		break;
	case 0x5b:
		OutputErrorString(_T("SPINDLE IMPENDING FAILURE SPIN-UP RETRY COUNT"));
		break;
	case 0x5c:
		OutputErrorString(_T("SPINDLE IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT"));
		break;
	case 0x60:
		OutputErrorString(_T("FIRMWARE IMPENDING FAILURE GENERAL HARD DRIVE FAILURE"));
		break;
	case 0x61:
		OutputErrorString(_T("FIRMWARE IMPENDING FAILURE DRIVE ERROR RATE TOO HIGH"));
		break;
	case 0x62:
		OutputErrorString(_T("FIRMWARE IMPENDING FAILURE DATA ERROR RATE TOO HIGH"));
		break;
	case 0x63:
		OutputErrorString(_T("FIRMWARE IMPENDING FAILURE SEEK ERROR RATE TOO HIGH"));
		break;
	case 0x64:
		OutputErrorString(_T("FIRMWARE IMPENDING FAILURE TOO MANY BLOCK REASSIGNS"));
		break;
	case 0x65:
		OutputErrorString(_T("FIRMWARE IMPENDING FAILURE ACCESS TIMES TOO HIGH"));
		break;
	case 0x66:
		OutputErrorString(_T("FIRMWARE IMPENDING FAILURE START UNIT TIMES TOO HIGH"));
		break;
	case 0x67:
		OutputErrorString(_T("FIRMWARE IMPENDING FAILURE CHANNEL PARAMETRICS"));
		break;
	case 0x68:
		OutputErrorString(_T("FIRMWARE IMPENDING FAILURE CONTROLLER DETECTED"));
		break;
	case 0x69:
		OutputErrorString(_T("FIRMWARE IMPENDING FAILURE THROUGHPUT PERFORMANCE"));
		break;
	case 0x6a:
		OutputErrorString(_T("FIRMWARE IMPENDING FAILURE SEEK TIME PERFORMANCE"));
		break;
	case 0x6b:
		OutputErrorString(_T("FIRMWARE IMPENDING FAILURE SPIN-UP RETRY COUNT"));
		break;
	case 0x6c:
		OutputErrorString(_T("FIRMWARE IMPENDING FAILURE DRIVE CALIBRATION RETRY COUNT"));
		break;
	case 0xff:
		OutputErrorString(_T("FAILURE PREDICTION THRESHOLD EXCEEDED (FALSE)"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x5e(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("LOW POWER CONDITION ON"));
		break;
	case 0x01:
		OutputErrorString(_T("IDLE CONDITION ACTIVATED BY TIMER"));
		break;
	case 0x02:
		OutputErrorString(_T("STANDBY CONDITION ACTIVATED BY TIMER"));
		break;
	case 0x03:
		OutputErrorString(_T("IDLE CONDITION ACTIVATED BY COMMAND"));
		break;
	case 0x04:
		OutputErrorString(_T("STANDBY CONDITION ACTIVATED BY COMMAND"));
		break;
	case 0x05:
		OutputErrorString(_T("IDLE_B CONDITION ACTIVATED BY TIMER"));
		break;
	case 0x06:
		OutputErrorString(_T("IDLE_B CONDITION ACTIVATED BY COMMAND"));
		break;
	case 0x07:
		OutputErrorString(_T("IDLE_C CONDITION ACTIVATED BY TIMER"));
		break;
	case 0x08:
		OutputErrorString(_T("IDLE_C CONDITION ACTIVATED BY COMMAND"));
		break;
	case 0x09:
		OutputErrorString(_T("STANDBY_Y CONDITION ACTIVATED BY TIMER"));
		break;
	case 0x0a:
		OutputErrorString(_T("STANDBY_Y CONDITION ACTIVATED BY COMMAND"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x63(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("END OF USER AREA ENCOUNTERED ON THIS TRACK"));
		break;
	case 0x01:
		OutputErrorString(_T("PACKET DOES NOT FIT IN AVAILABLE SPACE"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x64(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("ILLEGAL MODE FOR THIS TRACK"));
		break;
	case 0x01:
		OutputErrorString(_T("INVALID PACKET SIZE"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x65(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("VOLTAGE FAULT"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x67(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x0a:
		OutputErrorString(_T("SET TARGET PORT GROUPS COMMAND FAILED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x68(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x0a:
		OutputErrorString(_T("SUBSIDIARY LOGICAL UNIT NOT CONFIGURED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x6f(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case SCSI_SENSEQ_AUTHENTICATION_FAILURE:
		OutputErrorString(_T("COPY PROTECTION KEY EXCHANGE FAILURE - AUTHENTICATION FAILURE"));
		break;
	case SCSI_SENSEQ_KEY_NOT_PRESENT:
		OutputErrorString(_T("COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT PRESENT"));
		break;
	case SCSI_SENSEQ_KEY_NOT_ESTABLISHED:
		OutputErrorString(_T("COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT ESTABLISHED"));
		break;
	case SCSI_SENSEQ_READ_OF_SCRAMBLED_SECTOR_WITHOUT_AUTHENTICATION:
		OutputErrorString(_T("READ OF SCRAMBLED SECTOR WITHOUT AUTHENTICATION"));
		break;
	case SCSI_SENSEQ_MEDIA_CODE_MISMATCHED_TO_LOGICAL_UNIT:
		OutputErrorString(_T("MEDIA REGION CODE IS MISMATCHED TO LOGICAL UNIT REGION"));
		break;
	case SCSI_SENSEQ_LOGICAL_UNIT_RESET_COUNT_ERROR:
		OutputErrorString(_T("DRIVE REGION MUST BE PERMANENT/REGION RESET COUNT ERROR"));
		break;
	case 0x06:
		OutputErrorString(_T("INSUFFICIENT BLOCK COUNT FOR BINDING NONCE RECORDING"));
		break;
	case 0x07:
		OutputErrorString(_T("CONFLICT IN BINDING NONCE RECORDING"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x72(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("SESSION FIXATION ERROR"));
		break;
	case 0x01:
		OutputErrorString(_T("SESSION FIXATION ERROR WRITING LEAD-IN"));
		break;
	case 0x02:
		OutputErrorString(_T("SESSION FIXATION ERROR WRITING LEAD-OUT"));
		break;
	case 0x03:
		OutputErrorString(_T("SESSION FIXATION ERROR - INCOMPLETE TRACK IN SESSION"));
		break;
	case 0x04:
		OutputErrorString(_T("EMPTY OR PARTIALLY WRITTEN RESERVED TRACK"));
		break;
	case 0x05:
		OutputErrorString(_T("NO MORE TRACK RESERVATIONS ALLOWED"));
		break;
	case 0x06:
		OutputErrorString(_T("RMZ EXTENSION IS NOT ALLOWED"));
		break;
	case 0x07:
		OutputErrorString(_T("NO MORE TEST ZONE EXTENSIONS ARE ALLOWED"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x73(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x00:
		OutputErrorString(_T("CD CONTROL ERROR"));
		break;
	case SCSI_SENSEQ_POWER_CALIBRATION_AREA_ALMOST_FULL:
		OutputErrorString(_T("POWER CALIBRATION AREA ALMOST FULL"));
		break;
	case SCSI_SENSEQ_POWER_CALIBRATION_AREA_FULL:
		OutputErrorString(_T("POWER CALIBRATION AREA IS FULL"));
		break;
	case SCSI_SENSEQ_POWER_CALIBRATION_AREA_ERROR:
		OutputErrorString(_T("POWER CALIBRATION AREA ERROR"));
		break;
	case SCSI_SENSEQ_PMA_RMA_UPDATE_FAILURE:
		OutputErrorString(_T("PROGRAM MEMORY AREA UPDATE FAILURE"));
		break;
	case SCSI_SENSEQ_PMA_RMA_IS_FULL:
		OutputErrorString(_T("RMA/PMA IS ALMOST FULL"));
		break;
	case SCSI_SENSEQ_PMA_RMA_ALMOST_FULL:
		OutputErrorString(_T("CURRENT POWER CALIBRATION AREA ALMOST FULL"));
		break;
	case 0x10:
		OutputErrorString(_T("CURRENT POWER CALIBRATION AREA IS FULL"));
		break;
	case 0x11:
		OutputErrorString(_T("RDZ IS FULL"));
		break;
	case 0x17:
		OutputErrorString(_T("OTHER"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
		break;
	}
}

VOID OutputAdditionalSenseCodeQualifier0x74(
	BYTE byAscq
	)
{
	switch (byAscq) {
	case 0x08:
		OutputErrorString(_T("DIGITAL SIGNATURE VALIDATION FAILURE"));
		break;
	case 0x0c:
		OutputErrorString(_T("UNABLE TO DECRYPT PARAMETER LIST"));
		break;
	case 0x10:
		OutputErrorString(_T("SA CREATION PARAMETER VALUE INVALID"));
		break;
	case 0x11:
		OutputErrorString(_T("SA CREATION PARAMETER VALUE REJECTED"));
		break;
	case 0x12:
		OutputErrorString(_T("INVALID SA USAGE"));
		break;
	case 0x30:
		OutputErrorString(_T("SA CREATION PARAMETER NOT SUPPORTED"));
		break;
	case 0x40:
		OutputErrorString(_T("AUTHENTICATION FAILED"));
		break;
	case 0x71:
		OutputErrorString(_T("LOGICAL UNIT ACCESS NOT AUTHORIZED"));
		break;
	case 0x79:
		OutputErrorString(_T("SECURITY CONFLICT IN TRANSLATED DEVICE"));
		break;
	default:
		OutputErrorString(_T("OTHER"));
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
	)
{
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
		if (byAsc >= SCSI_ADSENSE_VENDOR_UNIQUE || byAscq >= 0x80) {
			OutputErrorString(_T(" VENDER UNIQUE ERROR)"));
		}
		else {
			OutputErrorString(_T("OTHER"));
		}
		break;
	}
	OutputErrorString(_T("\n"));
}

VOID OutputSenseKey(
	BYTE byKey
	)
{
	switch (byKey) {
	case SCSI_SENSE_NO_SENSE:
		OutputErrorString(_T("NO_SENSE"));
		break;
	case SCSI_SENSE_RECOVERED_ERROR:
		OutputErrorString(_T("RECOVERED_ERROR"));
		break;
	case SCSI_SENSE_NOT_READY:
		OutputErrorString(_T("NOT_READY"));
		break;
	case SCSI_SENSE_MEDIUM_ERROR:
		OutputErrorString(_T("MEDIUM_ERROR"));
		break;
	case SCSI_SENSE_HARDWARE_ERROR:
		OutputErrorString(_T("HARDWARE_ERROR"));
		break;
	case SCSI_SENSE_ILLEGAL_REQUEST:
		OutputErrorString(_T("ILLEGAL_REQUEST"));
		break;
	case SCSI_SENSE_UNIT_ATTENTION:
		OutputErrorString(_T("UNIT_ATTENTION"));
		break;
	case SCSI_SENSE_DATA_PROTECT:
		OutputErrorString(_T("DATA_PROTECT"));
		break;
	case SCSI_SENSE_BLANK_CHECK:
		OutputErrorString(_T("BLANK_CHECK"));
		break;
	case SCSI_SENSE_UNIQUE:
		OutputErrorString(_T("UNIQUE"));
		break;
	case SCSI_SENSE_COPY_ABORTED:
		OutputErrorString(_T("COPY_ABORTED"));
		break;
	case SCSI_SENSE_ABORTED_COMMAND:
		OutputErrorString(_T("ABORTED_COMMAND"));
		break;
	case SCSI_SENSE_EQUAL:
		OutputErrorString(_T("EQUAL"));
		break;
	case SCSI_SENSE_VOL_OVERFLOW:
		OutputErrorString(_T("VOL_OVERFLOW"));
		break;
	case SCSI_SENSE_MISCOMPARE:
		OutputErrorString(_T("MISCOMPARE"));
		break;
	case SCSI_SENSE_RESERVED:
		OutputErrorString(_T("RESERVED"));
		break;
	default:
		OutputErrorString(_T("UNKNOWN"));
		break;
	}
	OutputErrorString(_T(" - "));
}

VOID OutputSenseData(
	PSENSE_DATA pSenseData
	)
{
	OutputErrorString(
		_T("\tSenseData Key-Asc-Ascq: %02x-%02x-%02x = ")
		, pSenseData->SenseKey, pSenseData->AdditionalSenseCode
		, pSenseData->AdditionalSenseCodeQualifier);
	OutputSenseKey(pSenseData->SenseKey);
	OutputAdditionalSenseCode(
		pSenseData->AdditionalSenseCode, pSenseData->AdditionalSenseCodeQualifier);
#ifdef _DEBUG
	OutputErrorString(
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
		OutputErrorString(
			_T("                Information: %u%u%u%u\n"),
			pSenseData->Information[0],
			pSenseData->Information[1],
			pSenseData->Information[2],
			pSenseData->Information[3]);
	}
	OutputErrorString(
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
	)
{
	OutputErrorString(
		_T("\tScsiStatus: %#04x = "), byScsiStatus);
	switch (byScsiStatus) {
	case SCSISTAT_GOOD:
		OutputErrorString(_T("GOOD\n"));
		break;
	case SCSISTAT_CHECK_CONDITION:
		OutputErrorString(_T("CHECK_CONDITION\n"));
		break;
	case SCSISTAT_CONDITION_MET:
		OutputErrorString(_T("CONDITION_MET\n"));
		break;
	case SCSISTAT_BUSY:
		OutputErrorString(_T("BUSY\n"));
		break;
	case SCSISTAT_INTERMEDIATE:
		OutputErrorString(_T("INTERMEDIATE\n"));
		break;
	case SCSISTAT_INTERMEDIATE_COND_MET:
		OutputErrorString(_T("INTERMEDIATE_COND_MET\n"));
		break;
	case SCSISTAT_RESERVATION_CONFLICT:
		OutputErrorString(_T("RESERVATION_CONFLICT\n"));
		break;
	case SCSISTAT_COMMAND_TERMINATED:
		OutputErrorString(_T("COMMAND_TERMINATED\n"));
		break;
	case SCSISTAT_QUEUE_FULL:
		OutputErrorString(_T("QUEUE_FULL\n"));
		break;
	default:
		OutputErrorString(_T("UNKNOWN\n"));
		break;
	}
}

VOID OutputScsiAddress(
	PDEVICE pDevice
	)
{
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
	)
{
	OutputDriveLogA(
		OUTPUT_DHYPHEN_PLUS_STR(StorageAdapterDescriptor)
		"\t              Version: %lu\n"
		"\t                 Size: %lu\n"
		"\tMaximumTransferLength: %lu (bytes)\n"
		"\t MaximumPhysicalPages: %lu\n"
		"\t        AlignmentMask: %lu\n"
		"\t       AdapterUsesPio: %s\n"
		"\t     AdapterScansDown: %s\n"
		"\t      CommandQueueing: %s\n"
		"\t  AcceleratedTransfer: %s\n"
		"\t              BusType: "
		, pAdapterDescriptor->Version
		, pAdapterDescriptor->Size
		, pAdapterDescriptor->MaximumTransferLength
		, pAdapterDescriptor->MaximumPhysicalPages
		, pAdapterDescriptor->AlignmentMask
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
		OutputDriveLogA("BusType???\n");
		break;
	}
	OutputDriveLogA(
		"\t      BusMajorVersion: %u\n"
		"\t      BusMinorVersion: %u\n",
		pAdapterDescriptor->BusMajorVersion,
		pAdapterDescriptor->BusMinorVersion);
}

VOID OutputFloppyInfo(
	PDISK_GEOMETRY pGeom,
	DWORD dwGeomNum
	)
{
	if (dwGeomNum > 1) {
		OutputDiscLogA("SupportedMediaType\n");
	}
	else if (dwGeomNum == 1) {
		OutputDiscLogA("CurrentMediaType\n");
	}
	for (DWORD i = 0; i < dwGeomNum; i++) {
		OutputDiscLogA("\t        MediaType: ");
		switch (pGeom[i].MediaType) {
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
			OutputDiscLogA("Unknown\n");
			break;
		}

		DWORD dwDiskSize = pGeom[i].Cylinders.LowPart * pGeom[i].TracksPerCylinder *
			pGeom[i].SectorsPerTrack * pGeom[i].BytesPerSector;
		OutputDiscLogA(
			"\t        Cylinders: %lu\n"
			"\tTracksPerCylinder: %lu\n"
			"\t  SectorsPerTrack: %lu\n"
			"\t   BytesPerSector: %lu (Bytes)\n"
			"\t--------------------------\n"
			"\t         DiskSize: %lu (Bytes)\n\n",
			pGeom[i].Cylinders.LowPart,
			pGeom[i].TracksPerCylinder,
			pGeom[i].SectorsPerTrack,
			pGeom[i].BytesPerSector,
			dwDiskSize);
	}
}
