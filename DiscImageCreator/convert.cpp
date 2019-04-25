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
#include "convert.h"

/*
	<dst>
		lpRowSubcode[ 0-11] -> P channel
		lpRowSubcode[12-23] -> Q channel
		lpRowSubcode[24-35] -> R channel
		lpRowSubcode[36-47] -> S channel
		lpRowSubcode[48-59] -> T channel
		lpRowSubcode[60-71] -> U channel
		lpRowSubcode[72-83] -> V channel
		lpRowSubcode[84-95] -> W channel
	<src>
		lpColumnSubcode[2352-2447] & 0x80 -> P channel
		lpColumnSubcode[2352-2447] & 0x40 -> Q channel
		lpColumnSubcode[2352-2447] & 0x20 -> R channel
		lpColumnSubcode[2352-2447] & 0x10 -> S channel
		lpColumnSubcode[2352-2447] & 0x08 -> T channel
		lpColumnSubcode[2352-2447] & 0x04 -> U channel
		lpColumnSubcode[2352-2447] & 0x02 -> V channel
		lpColumnSubcode[2352-2447] & 0x01 -> W channel
*/
/*
	//p(0x80)	//0x80  0	//0x40 L1	//0x20 L2	//0x10 L3	//0x08 L4	//0x04 L5	//0x02 L6	//0x01 L7
	//q(0x40)	//0x80 R1	//0x40 0	//0x20 L1	//0x10 L2	//0x08 L3	//0x04 L4	//0x02 L5	//0x01 L6
	//r(0x20)	//0x80 R2	//0x40 R1	//0x20 0	//0x10 L1	//0x08 L2	//0x04 L3	//0x02 L4	//0x01 L5
	//s(0x10)	//0x80 R3	//0x40 R2	//0x20 R1	//0x10 0	//0x08 L1	//0x04 L2	//0x02 L3	//0x01 L4
	//t(0x08)	//0x80 R4	//0x40 R3	//0x20 R2	//0x10 R1	//0x08 0	//0x04 L1	//0x02 L2	//0x01 L3
	//u(0x04)	//0x80 R5	//0x40 R4	//0x20 R3	//0x10 R2	//0x08 R1	//0x04 0	//0x02 L1	//0x01 L2
	//v(0x02)	//0x80 R6	//0x40 R5	//0x20 R4	//0x10 R3	//0x08 R2	//0x04 R1	//0x02 0	//0x01 L1
	//w(0x01)	//0x80 R7	//0x40 R6	//0x20 R5	//0x10 R4	//0x08 R3	//0x04 R2	//0x02 R1	//0x01 0
*/
BOOL AlignRowSubcode(
	LPBYTE lpRowSubcode,
	LPBYTE lpColumnSubcode
) {
	ZeroMemory(lpRowSubcode, CD_RAW_READ_SUBCODE_SIZE);
	INT nRow = 0;
	for (INT bitNum = 0; bitNum < CHAR_BIT; bitNum++) {
		for (INT nColumn = 0; nColumn < CD_RAW_READ_SUBCODE_SIZE; nRow++) {
			UINT nMask = 0x80;
			for (INT nShift = 0; nShift < CHAR_BIT; nShift++, nColumn++) {
				INT n = nShift - bitNum;
				if (n > 0) {
					lpRowSubcode[nRow] |= (BYTE)((lpColumnSubcode[nColumn] >> n) & nMask);
				}
				else {
					lpRowSubcode[nRow] |= (BYTE)((lpColumnSubcode[nColumn] << abs(n)) & nMask);
				}
				nMask >>= 1;
			}
		}
	}
	return TRUE;
}

/*
	<dst>
		lpColumnSubcode[0-96] & 0x80 -> P channel
		lpColumnSubcode[0-96] & 0x40 -> Q channel
		lpColumnSubcode[0-96] & 0x20 -> R channel
		lpColumnSubcode[0-96] & 0x10 -> S channel
		lpColumnSubcode[0-96] & 0x08 -> T channel
		lpColumnSubcode[0-96] & 0x04 -> U channel
		lpColumnSubcode[0-96] & 0x02 -> V channel
		lpColumnSubcode[0-96] & 0x01 -> W channel
	<src>
		lpRowSubcode[ 0-11] -> P channel
		lpRowSubcode[12-23] -> Q channel
		lpRowSubcode[24-35] -> R channel
		lpRowSubcode[36-47] -> S channel
		lpRowSubcode[48-59] -> T channel
		lpRowSubcode[60-71] -> U channel
		lpRowSubcode[72-83] -> V channel
		lpRowSubcode[84-95] -> W channel
*/
/*
	//p(0x80)	//0x80  0	//0x40 L1	//0x20 L2	//0x10 L3	//0x08 L4	//0x04 L5	//0x02 L6	//0x01 L7
	//q(0x40)	//0x80 R1	//0x40 0	//0x20 L1	//0x10 L2	//0x08 L3	//0x04 L4	//0x02 L5	//0x01 L6
	//r(0x20)	//0x80 R2	//0x40 R1	//0x20 0	//0x10 L1	//0x08 L2	//0x04 L3	//0x02 L4	//0x01 L5
	//s(0x10)	//0x80 R3	//0x40 R2	//0x20 R1	//0x10 0	//0x08 L1	//0x04 L2	//0x02 L3	//0x01 L4
	//t(0x08)	//0x80 R4	//0x40 R3	//0x20 R2	//0x10 R1	//0x08 0	//0x04 L1	//0x02 L2	//0x01 L3
	//u(0x04)	//0x80 R5	//0x40 R4	//0x20 R3	//0x10 R2	//0x08 R1	//0x04 0	//0x02 L1	//0x01 L2
	//v(0x02)	//0x80 R6	//0x40 R5	//0x20 R4	//0x10 R3	//0x08 R2	//0x04 R1	//0x02 0	//0x01 L1
	//w(0x01)	//0x80 R7	//0x40 R6	//0x20 R5	//0x10 R4	//0x08 R3	//0x04 R2	//0x02 R1	//0x01 0
*/
BOOL AlignColumnSubcode(
	LPBYTE lpColumnSubcode,
	LPBYTE lpRowSubcode
) {
	INT nRow = 0;
	UINT nMask = 0x80;
	for (INT bitNum = 0; bitNum < CHAR_BIT; bitNum++) {
		for (INT nColumn = 0; nColumn < CD_RAW_READ_SUBCODE_SIZE; nRow++) {
			for (INT nShift = 0; nShift < CHAR_BIT; nShift++, nColumn++) {
				INT n = nShift - bitNum;
				if (n > 0) {
					lpColumnSubcode[nColumn] |= (BYTE)((lpRowSubcode[nRow] << n) & nMask);
				}
				else {
					lpColumnSubcode[nColumn] |= (BYTE)((lpRowSubcode[nRow] >> abs(n)) & nMask);
				}
			}
		}
		nMask >>= 1;
	}
	return TRUE;
}

BYTE BcdToDec(
	BYTE bySrc
) {
	return (BYTE)(((bySrc >> 4) & 0x0f) * 10 + (bySrc & 0x0f));
}

BYTE DecToBcd(
	BYTE bySrc
) {
	INT m = 0;
	INT n = bySrc;
	m += n / 10;
	n -= m * 10;
	return (BYTE)(m << 4 | n);
}

// 00:00:00 <= MSF <= 89:59:74
// 90:00:00 <= MSF <= 99:59:74 is TODO
INT MSFtoLBA(
	BYTE byMinute,
	BYTE bySecond, 
	BYTE byFrame
) {
	return (byMinute * 60 + bySecond) * 75 + byFrame;
}

// -451150 <= LBA <= -151 is TODO
// -150 <= LBA <= 404849
VOID LBAtoMSF(
	INT nLBA, 
	LPBYTE byMinute,
	LPBYTE bySecond, 
	LPBYTE byFrame
) {
	*byFrame = (BYTE)(nLBA % 75);
	nLBA /= 75;
	*bySecond = (BYTE)(nLBA % 60);
	nLBA /= 60;
	*byMinute = (BYTE)(nLBA);
}

VOID LittleToBig(
	LPWCH pOut,
	LPWCH pIn,
	INT nCnt
) {
	for (INT a = 0; a < nCnt; a++) {
		REVERSE_BYTES_SHORT(&pOut[a], &pIn[a]);
	}
}

// http://msdn.microsoft.com/ja-jp/library/83ythb65.aspx
// http://senbee.seesaa.net/article/19124170.html
LPVOID ConvParagraphBoundary(
	PDEVICE pDevice,
	LPBYTE pv
) {
#ifdef _WIN32
	return (LPVOID)(((UINT_PTR)pv + pDevice->AlignmentMask) & ~pDevice->AlignmentMask);
#else
	UNREFERENCED_PARAMETER(pDevice);
	return (LPVOID)pv;
#endif
}

UINT PadSizeForVolDesc(
	UINT uiSize
) {
	INT nPadding = DISC_RAW_READ_SIZE - (INT)uiSize;
	// uiSize isn't 2048 byte
	if (nPadding != 0) {
		// uiSize is smaller than 2048 byte
		if (nPadding > 0){
			// Generally, directory size is per 2048 byte
			// Exception:
			//  Codename - Outbreak (Europe) (Sold Out Software)
			//  Commandos - Behind Enemy Lines (Europe) (Sold Out Software)
			// and more
			uiSize += nPadding;
		}
		// uiSize is larger than 2048 byte
		else {
			nPadding = (INT)uiSize % DISC_RAW_READ_SIZE;
			// uiSize isn't 4096, 6144, 8192 etc byte
			if (nPadding != 0) {
				nPadding = DISC_RAW_READ_SIZE - nPadding;
				uiSize += nPadding;
			}
		}
	}
	return uiSize;
}
