//
// Created by jmaerte on 22.03.20.
//
#include "../arithmetic.hpp"
#include "../operator.hpp"
#include "hardware/hardware.hpp"

namespace jmaerte {
    namespace arith {
        namespace num {
            namespace aux {
                /**
                 * This namespace contains modular algorithms (montgomery arithmetic):
                 *  * Shift the modulus N so that it becomes odd.
                 *  * Iteratively calculate the modular inverse of N respectively R.
                 *  * Montgomery multiply, i.e. reduce (AB)R^{-1} mod N for R = B^n, where B = 2^32 and R / B <= N < R.
                 *  * Calculate scaled remainder
                 *  * Calculate power of radix R.
                 */
                namespace modular {

                    int ODDIFY(ap_int N) {
                        int shift = 0;
                        ULL* arr = ABS(N);
                        ULL value;
                        for (int i = 0; i < GET_OCC(N); i++) {
                            if(!(value = *(arr + i))) shift += 64;
                            else {
                                shift += TRAILING_ZEROS(value);
                            }
                        }
                        RSHIFT(N, shift);
                        return shift;
                    }

                    /**
                     * Dusse Kaliski
                     * @param N
                     * @return
                     */
                    ap_int MODINV(ap_int N) {
                        ULL x = GET(N, 0);

                        ULL y = 1ULL;
                        ULL c = 3ULL; // 0....011
                        // 8 correct bits for the inverse
                        for (int i = 1; i < 8; i++) {
                            if ((x * y) & c != 1) {
                                y |= 1ULL << i;
                            }
                            c = c + c + 1ULL; // shift left by 1 and fill with 1.
                        }
                        // now newton:
                        for (int i = 0; i < 4; i++) {
                            y = y * (2 - y * x);
                        }
                        // y is now accurate in all 64 bits.
                        int n = GET_OCC(N);
                        ap_int inv = NEW(n, false, y);
                        ap_int temp;
                        // every iteration step the number of correct bits doubles. => 3 iterations for full 64 bits.
                        // there we need to do 3 + log_2 (n) iteration steps, where n is the occupation of N.
                        int correct_ulls = 1;
                        int log = LOG2(n);
                        ap_int ONE = NEW(1, false, 1ULL);
                        for (int i = 0; i < log; i++) {
                            OVERWRITE(temp, iMULL(N, inv, 2 * correct_ulls));
                            SUB(temp, 2ULL, 0, ONE);
                            SWITCH_SIGN(temp);
                            MULL(inv, temp, 2 * correct_ulls);
                            correct_ulls *= 2;
                            if (correct_ulls > n) correct_ulls = n / 2 + n % 2;
                        }
                        DELETE(ONE);
                        return inv;
                    }

                    /**
                     * Let N be an modulus of n-words, i.e. BASE^{n-1} <= N < BASE^n =: R. This function calculates
                     * a representative of (ab)R^{-1} (mod N).
                     *
                     * @param a target ap_int,
                     * @param b coefficient
                     * @param N modulus assumed as ODD.
                     * @param N_inv modular inverse of N respective BASE.
                     */
                    void MODMUL(ap_int a, ap_int b, ap_int N, ap_int N_inv) {
                        int n = GET_OCC(N);
                        MUL(a, b); // a = a * b
                        ap_int hi = TRUNCATE(a, n); // a = low_n(a), hi = high_n(a)
                        MULL(a, N_inv, n);
                        MULH(a, N, n);

                        if (COMPARE_ABS(hi, a) < 0) {
                            num::SUB(a, hi);
                            SWITCH_SIGN(a);
                            num::ADD(a, N);
                        } else {
                            num::SUB(a, hi);
                            SWITCH_SIGN(a);
                        }
                    }

                    /**
                     * Calculates the scaled remainder, i.e. an s, such that
                     *      r = - s * R^n (mod b),
                     * where r is the true remainder of a (mod b).
                     *
                     * WE ASSUME a IS NON-NEGATIVE, you should handle that inside the mod function.
                     *
                     * Returns whether the division can be performed exact or not.
                     */
                    bool SREM(ap_int a, ap_int N, ap_int N_inv) {
                        int len_a = GET_OCC(a), len_N = GET_OCC(N);
                        int n = len_a / len_N; // ceil

                        ULL* data = ABS(a);

                        ULL* rem = data;
                        REC_MULL_DATA(rem, len_N, ABS(N_inv), GET_OCC(N_inv), 0, len_N);
                        REC_MULH_DATA(rem, len_N, ABS(N), GET_OCC(N), 0, len_N);

                        data += len_N;

                        bool borrow = false;
                        ULL one = 1ULL;
                        for (int i = 1; i < n; i++) {
                            borrow = COMPARE_RAW(rem + len_N, data + len_N, len_N);
                            (void)SUB_DATA_RANGE(rem, data, true, rem + len_N); // ignore borrow of this subtraction
                            REC_MULL_DATA(rem, len_N, ABS(N_inv), GET_OCC(N_inv), 0, len_N);
                            if (borrow) ADD_DATA_RANGE(rem, &one, &one + 1);
                            REC_MULH_DATA(rem, len_N, ABS(N), GET_OCC(N), 0, len_N);
                            data += len_N;
                        }
                        if (n * len_N < len_a) {
                            int r = len_a - n * len_N;
                            ULL occ = len_N;
                            STRIP(rem, occ);
                            int len = MIN(r, occ);
                            if (r != occ) borrow = (r > occ);
                            else COMPARE_RAW(rem + occ, data + r, occ);
                            ULL carry = SUB_DATA_RANGE(rem, data, true, rem + len); // ignore borrow of this subtraction
                            while (len < len_N && carry){
//                                temp = (carry < *(rem + len));
                                carry = (carry < *(rem + len));
                                *(rem + len) = carry - *(rem + len);
                                len++;
                            }
                            if (len > occ) occ = len;
                            REC_MULL_DATA(rem, occ, ABS(N_inv), GET_OCC(N_inv), 0, len_N);
                            if (borrow) {
                                carry = ADD_DATA_RANGE(rem, &one, rem + 1);
                                len = 1;
                                while (len < len_N && carry){
                                    *(rem + len) += carry;
                                    carry = (*(rem + len) < carry);
                                    len++;
                                }
                                if (len > occ) occ = len;
                            }
                            REC_MULH_DATA(rem, occ, ABS(N), GET_OCC(N), 0, len_N);
                        }
                        if (GET_OCC(a)) {
                            num::SUB(a, N);
                            SWITCH_SIGN(a);
                        }
                        return GET_OCC(a) == 0;
                    }

                    /**
                     * Calculates R^2 (mod N) by iterated modmul with 2^64 or 2^32.
                     * I.e. block left shifts.
                     * @param N
                     * @return
                     */
                    ap_int RADIX_SQ(ap_int N) {
                        int n = GET_OCC(N);
                        int k = n + 1;
                        ap_int c = NEW(n + 1, false, 1ULL);
                        LSHIFT_BLOCK(c, n - 1);
                        for (int i = 0; i < k; i++) {
                            LSHIFT_BLOCK(c, 1);
                            if (COMPARE_ABS(c, N) > 0) SUB(c, 1ULL, 0, N);
                        }
                        return c;
                    }

                    void MODSQR(ap_int& a, ap_int N, ap_int inv) {
                        int n = GET_OCC(N);
                        SQR(a);
                        ap_int hi = TRUNCATE(a, n); // a = low_n(a), hi = high_n(a)
                        MULL(a, inv, n);
                        MULH(a, N, n);

                        if (COMPARE_ABS(hi, a) < 0) {
                            num::SUB(a, hi);
                            SWITCH_SIGN(a);
                            num::ADD(a, N);
                        } else {
                            num::SUB(a, hi);
                            SWITCH_SIGN(a);
                        }
                    }

                    void iMODSQR(ap_int dest, ap_int a, ap_int N, ap_int N_inv) {
                        int n = GET_OCC(N);
                        OVERWRITE(dest, iSQR(a));
                        ap_int hi = TRUNCATE(dest, n); // a = low_n(a), hi = high_n(a)
                        MULL(dest, N_inv, n);
                        MULH(dest, N, n);

                        if (COMPARE_ABS(hi, dest) < 0) {
                            num::SUB(dest, hi);
                            SWITCH_SIGN(dest);
                            num::ADD(dest, N);
                        } else {
                            num::SUB(dest, hi);
                            SWITCH_SIGN(dest);
                        }
                    }

                    void MODRED(ap_int& a, ap_int N, ap_int inv) {
                        int n = GET_OCC(N);
                        MULL(a, inv, n);
                        MULH(a, N, n);
                        if(GET_OCC(a)) {
                            num::SUB(a, N);
                            SWITCH_SIGN(a);
                        }
                    }

                    void iMODRED(ap_int dest, ap_int a, ap_int N, ap_int inv) {
                        int n = GET_OCC(N);
                        OVERWRITE(dest, iMULL(a, inv, n));
                        MULH(dest, N, n);
                        if(GET_OCC(dest)) {
                            num::SUB(dest, N);
                            SWITCH_SIGN(dest);
                        }
                    }

                    /**
                     * Calculates R^n (mod N).
                     * @param SQ R^2 (mod N)
                     */
                    ap_int RADIX_POW(ap_int SQ, int n, ap_int N, ap_int N_inv) {
                        ap_int pow = COPY(SQ);
                        if (n == 3) {
                            MODSQR(pow, N, N_inv);
                        } else {
                            ap_int ptmp = COPY(SQ);
                            MODSQR(ptmp, N, N_inv);
                            int p = n;
                            int bm = 0;
                            unsigned int bit = 1ULL;
                            while (p > 5) {
                                if (p % 2 == 0) bm |= bit;
                                p = p / 2 + 1;
                                bit << 1;
                            }
                            if (p == 4) MODMUL(pow, ptmp, N, N_inv);
                            else if (p == 5) iMODSQR(pow, ptmp, N, N_inv);
                            bit >>= 1;
                            while (bit != 0) {
                                if (bit & bm) {
                                    iMODRED(ptmp, pow, N, N_inv);
                                    MODMUL(pow, ptmp, N, N_inv);
                                } else MODSQR(pow, N, N_inv);
                                bit >>= 1;
                            }
                        }
                        return pow;
                    }
                }
            }
        }
    }
}