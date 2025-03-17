/*
Copyright(C) 2003, Y.Kanechika. All rights reserved.
Copyright(C) 2006, C-yan. All rights reserved.
ソースコード及びバイナリ形式の利用及び再配布は、それらの改変に関わらず、以下の条件を満たす限り、許可する。

1.ソースコードの再配布は、上記の著作権表示、この条約リスト、および以下の免責事項を明記した文章を伴って、行わなければならない。
2.バイナリ形式の再配布は、上記の著作権表示、この条約リスト、及び以下の免責事項を、配布物に付属する説明書、又は書類へ明記しなければならない。
3.ソースコード及びバイナリ形式の再配布において、配布に必要な物理的コストを上回る代価の要求、又はそれに類するライセンス行為を行ってはならない。（商業利用は禁止）

このソフトウェアの利用から発生した、いかなる問題においても、この著作権者は、損害に対し責任はない。
*/
#pragma once

BYTE* GetEccP();
BYTE* GetEccQ();
DWORD CalcEDC(BYTE* Buffer, DWORD size);
void CalcEccP(BYTE *buffer);
void CalcEccQ(BYTE *buffer);
int MemCmp(BYTE* Mem1, BYTE* Mem2, DWORD Count);
