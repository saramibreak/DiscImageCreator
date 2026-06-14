/**
 * Copyright 2025 sarami@ChatGPT
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* CD+G subchannel Reed-Solomon (GF(2^6)) toolkit
 *
 * Field	 : GF(64) with primitive poly p(x) = x^6 + x + 1  (0x43)
 * Roots	 : start exponent = 0  => g(x) = ƒ®_{i=0..m-1} (x - ƒ¿^i)
 * Direction : Public API exposes "High-degree" syndrome (r(x)=r0*x^{n-1}+...)
 *
 * Public (high-degree API):
 *   void cdg_rs6_init(void);
 *   int  rs6_syndromes_highdeg(const unsigned char *code, int n, int m, unsigned char *S); // logging
 *
 *   // Q (RS(4,2), t=1): robust single-error decoder using high-degree S
 *   // code4 = [cmd, ins, Q0, Q1] (each 6-bit)
 *   int  rs6_q_correct(unsigned char code4[4]);   // 0:ok(no change), 1:fixed, -1:fail
 *
 *   // P (RS(24,20), t=2): normal decoder (Berlekamp-Massey + Chien + Forney)
 *   // code24 = [cmd,ins,Q0,Q1,data[16],P0..P3] (each 6-bit)
 *   int  rs6_p_correct(unsigned char code24[24]); // >=0: #fixed, -1:fail
 *
 *   // P with erasures (e.g., erasures={20,21,22,23} for parity-only damage)
 *   int  rs6_p_correct_with_erasures(unsigned char code24[24], const int *eras, int e);
 *
 * Notes:
 *   - All functions mask symbols to 6 bits internally.
 *   - High-degree API here is stable: for P decoding we reverse to ascending-power core internally.
 *   - Q decoder uses a robust analytical 1-error method (position brute-force) with high-degree syndromes.
 */

#if defined(__linux__)
#include "defineForLinux.h"
#endif

 /* ===== GF(2^6) ===== */
#define GF_PRIM6  0x43
#define GF_SIZE   64
#define GF_ORD	63

static unsigned char gf_exp_tbl[GF_ORD * 2];
static unsigned char gf_log_tbl[GF_SIZE];

static unsigned char gf6_mul(unsigned char a, unsigned char b)
{
	if (!a || !b) {
		return 0;
	}
	return gf_exp_tbl[gf_log_tbl[a] + gf_log_tbl[b]];
}

static unsigned char gf6_div(unsigned char a, unsigned char b)
{
	if (!a) {
		return 0;
	}
	int e = gf_log_tbl[a] - gf_log_tbl[b];
	if (e < 0) {
		e += GF_ORD;
	}
	return gf_exp_tbl[e];
}

static unsigned char gf6_pow_alpha(int e)
{
	while (e < 0) {
		e += GF_ORD;
	}
	return gf_exp_tbl[e % GF_ORD];
}

static void rs6_mask6(unsigned char* a, int n)
{
	for (int i = 0; i < n; i++) {
		a[i] &= 0x3F;
	}
}

static void rs6_reverse(unsigned char* a, int n)
{
	for (int i = 0; i < n / 2; i++) {
		unsigned char t = a[i];
		a[i] = a[n - 1 - i];
		a[n - 1 - i] = t;
	}
}

void cdg_rs6_init(void)
{
	// Build exp/log tables for GF(64), primitive poly 0x43, ƒ¿ is element 2^0=1 step left with reduction.
	unsigned char x = 1;
	for (int i = 0; i < GF_ORD; i++) {
		gf_exp_tbl[i] = x;
		gf_log_tbl[x] = (unsigned char)i;
		x = (unsigned char)((x << 1) & 0xFF);
		if (x & 0x40) {
			x ^= GF_PRIM6; // mod p(x)
		}
	}
	for (int i = GF_ORD; i < GF_ORD * 2; i++) {
		gf_exp_tbl[i] = gf_exp_tbl[i - GF_ORD];
	}
	gf_log_tbl[0] = 0; // unused
}

//  RS generator: g(x) = ƒ®_{i=0..m-1} (x - ƒ¿^(start+i))
static void make_generator_start(int m, int start, unsigned char* gen/*len m+1*/)
{
	memset(gen, 0, (size_t)(m + 1));
	gen[0] = 1;
	for (int i = 0; i < m; i++) {
		unsigned char next[8] = { 0 };
		for (int j = 0; j <= i; j++) {
			next[j + 1] ^= gen[j];  // x * current
			unsigned char term = gf6_mul(gen[j], gf_exp_tbl[start + i]); // ƒ¿^(start+i)
			next[j] ^= term;	// -(ƒ¿^k) == +(ƒ¿^k) in GF(2)
		}
		memcpy(gen, next, (size_t)(m + 1));
	}
}

// Systematic encoder: compute m parity symbols for k info symbols.
static void rs_encode_parity(const unsigned char* msg, int k, const unsigned char* gen, int m, unsigned char* parity)
{
	unsigned char reg[6] = { 0 };
	for (int i = 0; i < k; i++) {
		unsigned char fb = (unsigned char)(msg[i] ^ reg[0]);
		for (int j = 0; j < m - 1; j++) {
			unsigned char mult = gf6_mul(fb, gen[m - 1 - j]); // gen[m-1]..gen[1]
			reg[j] = (unsigned char)(reg[j + 1] ^ mult);
		}
		reg[m - 1] = gf6_mul(fb, gen[0]);
	}
	memcpy(parity, reg, (size_t)m);
}

// ========== CD+G packet check ==========
// pkt[24] = [cmd][ins][Q0][Q1][data x16][P0..P3]
int cdg_check_packet_RS(const unsigned char pkt[24])
{
	const unsigned char cmd = (unsigned char)(pkt[0] & 0x3F);
	const unsigned char ins = (unsigned char)(pkt[1] & 0x3F);
	const unsigned char Qr[2] = { (unsigned char)(pkt[2] & 0x3F), (unsigned char)(pkt[3] & 0x3F) };
	unsigned char data16[16];
	for (int i = 0; i < 16; i++) {
		data16[i] = (unsigned char)(pkt[4 + i] & 0x3F);
	}
	const unsigned char Pr[4] = {
		(unsigned char)(pkt[20] & 0x3F),(unsigned char)(pkt[21] & 0x3F),
		(unsigned char)(pkt[22] & 0x3F),(unsigned char)(pkt[23] & 0x3F)
	};

	unsigned char genQ[3], genP[5];
	make_generator_start(2, 0, genQ);   // RS(4,2): parity m=2
	make_generator_start(4, 0, genP);   // RS(24,20): parity m=4

	// --- 1) Q FRS(4,2) ---
	unsigned char msgQ[2] = { cmd, ins };
	unsigned char Qx[2];
	rs_encode_parity(msgQ, 2, genQ, 2, Qx);
	if (Qx[0] != Qr[0] || Qx[1] != Qr[1]) {
		return -1; // Q unmatch
	}

	// --- 2) P FRS(24,20) on [cmd,ins,Qx0,Qx1,data16] ---
	unsigned char msgP[20];
	msgP[0] = cmd;
	msgP[1] = ins;
	msgP[2] = Qx[0];
	msgP[3] = Qx[1];
	memcpy(&msgP[4], data16, 16);

	unsigned char Px[4];
	rs_encode_parity(msgP, 20, genP, 4, Px);

	if (Px[0] != Pr[0] || Px[1] != Pr[1] || Px[2] != Pr[2] || Px[3] != Pr[3]) {
		return -2; // P unmatch
	}
	return 0; // match
}

/* ===== High-degree syndromes for logging / verification =====
 * r(x) = r0*x^{n-1} + r1*x^{n-2} + ... + r_{n-1}
 * S_i  = r(ƒ¿^i)  for i=0..m-1
 */
int rs6_syndromes_highdeg(const unsigned char* code, int n, int m, unsigned char* S)
{
	int nz = 0;
	for (int i = 0; i < m; i++) {
		unsigned char a = gf6_pow_alpha(i);
		unsigned char y = 0;
		for (int j = 0; j < n; j++) {
			// Horner: y = y*a + r_j
			y = (unsigned char)(gf6_mul(y, a) ^ (code[j] & 0x3F));
		}
		S[i] = y;
		if (y) {
			nz = 1;
		}
	}
	return nz; // 0 means already a valid codeword
}

/* ===== Ascending-power core (internal) =====
 * S_i(asc) = ƒ° r_j * (ƒ¿^i)^j, Berlekamp-Massey, Chien, Forney
 */
static int rs6_syndromes_asc(const unsigned char* r, int n, int m, unsigned char* S)
{
	int nz = 0;
	for (int i = 0; i < m; i++) {
		unsigned char si = 0;
		unsigned char a = gf6_pow_alpha(i);
		unsigned char p = 1;
		for (int j = 0; j < n; j++) {
			if (r[j]) {
				si = (unsigned char)((si ^ gf6_mul(r[j], p)) & 0xFF);
			}
			p = gf6_mul(p, a);
		}
		S[i] = si;
		if (si) {
			nz = 1;
		}
	}
	return nz;
}

static int bm_solve_locator(const unsigned char* S, int m, unsigned char* Lambda, int* Lout)
{
	unsigned char C[8] = { 0 };
	unsigned char B[8] = { 0 };
	C[0] = 1;
	B[0] = 1;
	int L = 0;
	int mstep = 1;
	unsigned char b = 1;
	for (int n = 0; n < m; n++) {
		unsigned char d = S[n];
		for (int i = 1; i <= L; i++) {
			if (C[i] && S[n - i]) {
				d = (unsigned char)((d ^ gf6_mul(C[i], S[n - i])) & 0xFF);
			}
		}
		if (d) {
			unsigned char T[8];
			memcpy(T, C, sizeof(T));
			unsigned char coef = gf6_div(d, b);
			for (int i = 0; i < 8 - mstep; i++) {
				if (B[i]) {
					C[i + mstep] = (unsigned char)((C[i + mstep] ^ gf6_mul(coef, B[i])) & 0xFF);
				}
			}
			if (2 * L <= n) {
				L = n + 1 - L;
				memcpy(B, T, sizeof(B));
				b = d;
				mstep = 1;
			}
			else {
				mstep++;
			}
		}
		else {
			mstep++;
		}
	}
	memcpy(Lambda, C, (size_t)(m + 1));
	*Lout = L;
	return 0;
}

static int chien_search_asc(const unsigned char* Lambda, int L, int n, int* err_pos)
{
	int cnt = 0;
	for (int j = 0; j < n; j++) {
		unsigned char xinv = gf6_pow_alpha(-j);
		unsigned char val = 0;
		unsigned char p = 1;
		for (int i = 0; i <= L; i++) {
			if (Lambda[i]) {
				val = (unsigned char)((val ^ gf6_mul(Lambda[i], p)) & 0xFF);
			}
			p = gf6_mul(p, xinv);
		}
		if (val == 0 && cnt < L) {
			err_pos[cnt++] = j;
		}
	}
	return cnt;
}

static void build_omega(const unsigned char* S, int m, const unsigned char* Lambda, int L, unsigned char* Omega)
{
	memset(Omega, 0, (size_t)m);
	for (int i = 0; i < m; i++) {
		if (!S[i]) {
			continue;
		}
		for (int j = 0; j <= L && i + j < m; j++) {
			if (Lambda[j]) {
				Omega[i + j] = (unsigned char)((Omega[i + j] ^ gf6_mul(S[i], Lambda[j])) & 0xFF);
			}
		}
	}
}

static unsigned char eval_poly_asc(const unsigned char* poly, int deg, unsigned char x)
{
	unsigned char y = 0;
	unsigned char p = 1;
	for (int i = 0; i <= deg; i++) {
		if (poly[i]) {
			y = (unsigned char)((y ^ gf6_mul(poly[i], p)) & 0xFF);
		}
		p = gf6_mul(p, x);
	}
	return y;
}

/* ===== Q (4,2) robust single-error decoder (high-degree) =====
 * code4 = [cmd, ins, Q0, Q1] (6-bit each)
 */
int rs6_q_correct(unsigned char code[4])
{
	rs6_mask6(code, 4);

	unsigned char S[2];
	rs6_syndromes_highdeg(code, 4, 2, S);
	if (S[0] == 0 && S[1] == 0) {
		return 0; // already valid
	}

	// Assume single error: e = S0, and S1 == e * ƒ¿^{n-1 - j}
	unsigned char e = S[0];
	for (int j = 0; j < 4; j++) {
		unsigned char expected = gf6_mul(e, gf6_pow_alpha(3 - j));
		if (expected == S[1]) {
			unsigned char fixed[4];
			memcpy(fixed, code, 4);
			fixed[j] ^= e;
			fixed[j] &= 0x3F;
			unsigned char Sck[2];
			rs6_syndromes_highdeg(fixed, 4, 2, Sck);
			if (Sck[0] == 0 && Sck[1] == 0) {
				memcpy(code, fixed, 4);
				return 1;
			}
		}
	}
	return -1; // not a single-error pattern (or orientation mismatch upstream)
}

/* ===== P (24,20) normal correction (t=2) - high-degree wrapper ===== ===== */
int rs6_p_correct(unsigned char code24[24])
{
	rs6_mask6(code24, 24);
	unsigned char tmp[24];
	memcpy(tmp, code24, 24);
	rs6_reverse(tmp, 24); // high-degree -> ascending-power core

	unsigned char S[4];
	if (!rs6_syndromes_asc(tmp, 24, 4, S)) {
		rs6_reverse(tmp, 24);
		memcpy(code24, tmp, 24);
		return 0;
	}

	unsigned char Lambda[8] = { 0 };
	int L = 0;
	bm_solve_locator(S, 4, Lambda, &L);
	if (L == 0 || L > 2) {
		return -1;
	}

	int err_pos[4] = { 0 };
	int nerr = chien_search_asc(Lambda, L, 24, err_pos);
	if (nerr != L) {
		return -1;
	}

	unsigned char Omega[6] = { 0 };
	build_omega(S, 4, Lambda, L, Omega);

	for (int t = 0; t < nerr; t++) {
		int j = err_pos[t];
		unsigned char X = gf6_pow_alpha(-j);

		// ƒ©'(X): odd terms only
		unsigned char LpX = 0;
		unsigned char p = 1; // X^0
		for (int i = 1; i <= L; i += 2) {
			if (Lambda[i]) {
				LpX = (unsigned char)((LpX ^ gf6_mul(Lambda[i], p)) & 0xFF);
			}
			p = gf6_mul(p, gf6_mul(X, X)); // step by X^2
		}
		if (!LpX) {
			return -1;
		}

		unsigned char OmegaX = eval_poly_asc(Omega, 3, X);
		unsigned char e = gf6_div(OmegaX, LpX);

		tmp[j] ^= e;
		tmp[j] &= 0x3F;
	}

	unsigned char S2[4];
	if (rs6_syndromes_asc(tmp, 24, 4, S2)) {
		return -1;
	}

	rs6_reverse(tmp, 24);
	memcpy(code24, tmp, 24);
	return nerr;
}

/* ===== P with erasures (e.g., parity-only damage) - high-degree wrapper =====
 * erasures: array of erasure positions in high-degree order (0..23)
 * Returns: >=0 (#unknown-errors corrected), -1 on failure
 */
static void rs6_build_gamma_from_eras_asc(const int* eras, int e, unsigned char* Gamma /*len e+1*/)
{
	memset(Gamma, 0, (size_t)(e + 1));
	Gamma[0] = 1;
	for (int t = 0; t < e; t++) {
		unsigned char Xi = gf6_pow_alpha(-eras[t]); // ƒ¿^{-j}
		unsigned char nxt[8] = { 0 };
		for (int i = 0; i <= t; i++) {
			nxt[i] ^= Gamma[i];
			nxt[i + 1] = (unsigned char)((nxt[i + 1] ^ gf6_mul(Gamma[i], Xi)) & 0xFF); // +Xi*x  (minus==plus)
		}
		memcpy(Gamma, nxt, (size_t)(e + 1));
	}
}

static void rs6_apply_gamma_to_s(unsigned char* S, int m, const unsigned char* Gamma, int e)
{
	unsigned char tmp[8] = { 0 };
	for (int i = 0; i < m; i++) {
		if (!S[i]) {
			continue;
		}
		for (int j = 0; j <= e && i + j < m; j++) {
			if (Gamma[j]) {
				tmp[i + j] = (unsigned char)((tmp[i + j] ^ gf6_mul(S[i], Gamma[j])) & 0xFF);
			}
		}
	}
	memcpy(S, tmp, (size_t)m);
}

int rs6_p_correct_with_erasures(unsigned char code24[24], const int* eras, int e)
{
	rs6_mask6(code24, 24);
	unsigned char tmp[24];
	memcpy(tmp, code24, 24);
	rs6_reverse(tmp, 24); // high-degree -> ascending

	unsigned char S[4];
	if (!rs6_syndromes_asc(tmp, 24, 4, S)) {
		rs6_reverse(tmp, 24);
		memcpy(code24, tmp, 24);
		return 0;
	}

	// Build ƒ¡(x) from erasures (positions are given in high-degree indexing)
	int eras_asc[8];
	for (int i = 0; i < e; i++) {
		eras_asc[i] = 23 - eras[i]; // map to ascending index
	}
	unsigned char Gamma[8] = { 0 };
	rs6_build_gamma_from_eras_asc(eras_asc, e, Gamma);
	rs6_apply_gamma_to_s(S, 4, Gamma, e);

	// Run BM on adjusted syndromes to get ƒ©'(x), then ƒ©=ƒ¡*ƒ©'
	unsigned char Lp_[8] = { 0 };
	int Lp = 0;
	bm_solve_locator(S, 4, Lp_, &Lp);

	unsigned char Lambda[8] = { 0 };
	for (int i = 0; i <= e; i++) {
		if (!Gamma[i]) {
			continue;
		}
		for (int j = 0; j <= Lp && i + j < 8; j++) {
			if (Lp_[j]) {
				Lambda[i + j] = (unsigned char)((Lambda[i + j] ^ gf6_mul(Gamma[i], Lp_[j])) & 0xFF);
			}
		}
	}
	int L = e + Lp;
	if (L > 4) {
		return -1;
	}

	int err_pos[4] = { 0 };
	int nerr = chien_search_asc(Lambda, L, 24, err_pos);
	if (nerr != L) {
		return -1;
	}

	unsigned char Omega[6] = { 0 };
	build_omega(S, 4, Lambda, L, Omega);

	for (int t = 0; t < nerr; t++) {
		int j = err_pos[t];
		unsigned char X = gf6_pow_alpha(-j);

		unsigned char LpX = 0, p = 1;
		for (int i = 1; i <= L; i += 2) {
			if (Lambda[i]) {
				LpX = (unsigned char)((LpX ^ gf6_mul(Lambda[i], p)) & 0xFF);
			}
			p = gf6_mul(p, gf6_mul(X, X));
		}
		if (!LpX) {
			return -1;
		}

		unsigned char OmegaX = eval_poly_asc(Omega, 3, X);
		unsigned char e_mag = gf6_div(OmegaX, LpX);

		tmp[j] ^= e_mag;
		tmp[j] &= 0x3F;
	}

	unsigned char S2[4];
	if (rs6_syndromes_asc(tmp, 24, 4, S2)) {
		return -1;
	}

	rs6_reverse(tmp, 24);
	memcpy(code24, tmp, 24);
	// return #unknown-errors corrected (not counting known erasures)
	return (L - e);
}
