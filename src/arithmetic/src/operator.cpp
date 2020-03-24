//
// Created by jmaerte on 24.03.20.
//

#include "arithmetic/operator.hpp"
#include "arithmetic.hpp"

namespace jmaerte {
    namespace arith {
        namespace num {
            /*
             * COMPARATORS
             */

            int COMPARE_ABS(ap_int const n_a, ap_int const n_b) {
                int occ = GET_OCC(n_a);
                if (occ != GET_OCC(n_b)) return occ - GET_OCC(n_b);
                auto a = GET_ABS_DATA(n_a) + occ;
                auto b = GET_ABS_DATA(n_b) + occ;
                return aux::COMPARE_RAW(a, b, occ);
            }

            bool comp::SIGNED_COMPARATOR::operator()(ap_int const &a, ap_int const &b) const {
                if ((a->meta ^ b->meta) & NUM_SIGN_MASK) return GET_SIGN(a) ? b : a;
                return !((COMPARE_ABS(a, b) < 0) ^ GET_SIGN(a));
            }

            bool comp::UNSIGNED_COMPARATOR::operator()(ap_int const &a, ap_int const &b) const {
                return COMPARE_ABS(a, b) < 0;
            }

            /*
             * OPERATORS
             */

            void RSHIFT(ap_int n, int shift) {
                int start = shift / 64;
                shift %= 64;
                int next_shift = 64 - shift;
                ULL *arr = GET_ABS_DATA(n);
                int end = GET_OCC(n);
                for (int i = 0; i < end - start - 1; i++) {
                    *(arr + i) = (*(arr + i + start) >> shift) | (*(arr + i + start + 1) << next_shift);
                }
                *(arr + end - start - 1) = *(arr + end - 1) >> shift;
                SET_OCC(n, GET_OCC(n) - start - (*(arr + end - start - 1) == 0ULL));
            }

            void LSHIFT(ap_int n, int shift) {
                int start = shift / 64;
                shift %= 64;
                int next_shift = 64 - shift;
                ENLARGE(n, GET_OCC(n) + start + 1);
                ULL *arr = GET_ABS_DATA(n);
                int end = GET_OCC(n);
                for (int i = end - 1; i > 0; i--) {
                    *(arr + start + i) = (*(arr + i) << shift) | (*(arr + i - 1) >> next_shift);
                    if (start != 0) *(arr + i) = 0ULL;
                }
                *(arr + start) = *arr << shift;
                if (start != 0) *arr = 0ULL;
                SET_OCC(n, GET_OCC(n) + start + (*(arr + end + start) != 0ULL));
            }

            void LSHIFT_BLOCK(ap_int n, int blocks) {
                ENLARGE(n, GET_OCC(n) + blocks);
                ULL *arr = GET_ABS_DATA(n);
                int size = GET_OCC(n);
                std::copy_backward(arr, arr + size, arr + size + blocks);
                for (int i = 0; i < blocks; i++) *(arr + i) = 0ULL;
                SET_OCC(n, GET_OCC(n) + blocks);
            }

            /*
             * ARITHMETIC
             */

            ap_int iMUL(ap_int a, ap_int b) {
                if (GET_OCC(a) > GET_OCC(b)) {
                    return iMUL(b, a);
                }
                int occ = GET_OCC(a) + GET_OCC(b);
                ULL *result = new ULL[occ];
                occ -= 1 - aux::iREC_MUL_DATA(result, GET_ABS_DATA(a), GET_OCC(a), GET_ABS_DATA(b), GET_OCC(b));
                return NEW(result, occ, occ, GET_SIGN(a) ^ GET_SIGN(b));
            }

            void MUL(ap_int a, ap_int b) {
                int n_a = GET_OCC(a);
                int n_b = GET_OCC(b);

                ENLARGE(a, n_a + n_b);

                ULL *dat_a = GET_ABS_DATA(a);
                ULL *dat_b = GET_ABS_DATA(b);

                n_a -= 1 - aux::REC_MUL_DATA(dat_a, n_a, dat_b, n_b, 0);

                SET_SIGN(a, GET_SIGN(a) ^ GET_SIGN(b));
                SET_OCC(a, n_a + n_b);
            }

            void SQR(ap_int a) {
                int n = GET_OCC(a);
                ENLARGE(a);
                ULL *dat = GET_ABS_DATA(a);

                n -= 1 - aux::REC_SQR_DATA(dat, n, 0);

                SET_SIGN(a, false);
                SET_OCC(a, n + GET_OCC(a));
            }

            ap_int iSQR(ap_int a) {
                int n = GET_OCC(a);
                ULL *result = new ULL[2 * n];
                ULL *dat = GET_ABS_DATA(a);
                int occ = 2 * n - 1 + aux::iREC_SQR_DATA(result, dat, n);
                return NEW(result, 2 * n, occ, false);
            }

            ap_int iMULL(ap_int a, ap_int b, int n) {
                if (GET_OCC(a) > GET_OCC(b)) {
                    return iMUL(b, a);
                }
                ULL *result = new ULL[n];
                aux::iREC_MULL_DATA(result, GET_ABS_DATA(a), GET_OCC(a), GET_ABS_DATA(b), GET_OCC(b), 0, n);
                ULL occ = n;
                aux::STRIP(result, occ);
                return NEW(result, n, occ, GET_SIGN(a) ^ GET_SIGN(b));
            }

            void MULL(ap_int a, ap_int b, ULL n) {
                int n_a = GET_OCC(a);
                int n_b = GET_OCC(b);

                ENLARGE(a, n);

                ULL *dat_a = GET_ABS_DATA(a);
                ULL *dat_b = GET_ABS_DATA(b);

                aux::REC_MULL_DATA(dat_a, n_a, dat_b, n_b, 0, n);

                SET_SIGN(a, GET_SIGN(a) ^ GET_SIGN(b));
                aux::STRIP(dat_a, n);
                SET_OCC(a, n);
            }

            void MULH(ap_int a, ap_int b, ULL n) {
                int n_a = GET_OCC(a);
                int n_b = GET_OCC(b);

                ENLARGE(a, n);

                ULL *dat_a = GET_ABS_DATA(a);
                ULL *dat_b = GET_ABS_DATA(b);

                aux::REC_MULH_DATA(dat_a, n_a, dat_b, n_b, 0, n);

                SET_SIGN(a, GET_SIGN(a) ^ GET_SIGN(b));
                aux::STRIP(dat_a, n);
                SET_OCC(a, n);
            }

            ap_int DIV(ap_int a, ap_int b) {
                return aux::E_DIV(a, b);
            }
        }

        namespace vec {

            int FIND_POS(s_vec v, int pos) {
                int occ = GET_OCC(v);
                if (occ == 0 || num::GET_POS(AT(v, occ - 1)) < pos) return occ;
                if (num::GET_POS(AT(v, 0)) < pos) return 0;
                int min = 0;
                int max = occ;
                int compare = 0;
                while (min < max) {
                    int mid = (min + max) / 2;
                    if (num::GET_POS(AT(v, mid)) < pos) min = mid + 1;
                    else if (num::GET_POS(AT(v, mid)) > pos) max = mid;
                    else return mid;
                }
                return min;
            }

            void PUT(s_vec v, num::ap_int n) {
                int k = FIND_POS(v, num::GET_POS(n));
                if (GET_SIZE(v) <= GET_OCC(v)) {
                    ENLARGE(v);
                }
                if (k < GET_OCC(v)) {
                    std::copy_backward(AT(v, k), AT(v, GET_OCC(v)), AT(v, GET_OCC(v) + 2));
                }
                SET(v, k, n);
            }

            /*
             * ARITHMETIC
             */

            void ADD(s_vec a, int start_a, num::ap_int lambda, s_vec b, int start_b) {
                num::ap_int a_end = AT(a, GET_OCC(a));
                num::ap_int b_end = AT(b, GET_OCC(b));

                num::ap_int it_a = AT(a, start_a);
                num::ap_int j = AT(b, start_b);

                for (int i = start_a; i != GET_OCC(a) && j != b_end;) {
                    if (num::GET_POS(it_a) < num::GET_POS(j)) {
                        i++;
                        it_a += 2;
                    } else if (num::GET_POS(it_a) > num::GET_POS(j)) {
                        if (GET_OCC(a) + 1 > GET_SIZE(a)) {
                            ENLARGE(a);
                            it_a = AT(a, i);
                            a_end = AT(a, GET_OCC(a));
                        }
                        // copy j and j + 1 into a.
                        std::copy_backward(it_a, a_end, a_end + 2);
                        num::ASSIGN(it_a, num::iMUL(lambda, j));
                        SET_OCC(a, GET_OCC(a) + 1);
                        j += 2;
                    } else {
                        num::ap_int x = num::iMUL(lambda, j);
                        num::ADD(it_a, x);
                        num::DELETE(x);
                        it_a += 2;
                        i++;
                        j += 2;
                    }
                }

                if (j != b_end) {
                    if (GET_OCC(a) + (b_end - j) / 2 > GET_SIZE(a)) {
                        ENLARGE(a, GET_OCC(a) + (b_end - j) / 2);
                        a_end = AT(a, GET_OCC(a));
                    }
                    std::copy(j, b_end, a_end);
                }
            }

            void MUL(s_vec v, num::ap_int n) {
                for (int i = 0; i < GET_OCC(v); i++) {
                    num::MUL(AT(v, i), n);
                }
            }

            void MOD(s_vec v) {
                num::ap_int modulus = AT(v, 0);
                int shift = num::aux::modular::ODDIFY(modulus);
                int shift_blocks = shift / 64;
                ULL mask = (1ULL << (shift % 64)) - 1;
                num::ap_int inv = num::aux::modular::MODINV(modulus);
                num::ap_int pow = num::aux::modular::RADIX_SQ(modulus);
                std::map<int, num::ap_int> powers;
                powers.emplace(std::make_pair(2, pow));
                ULL *remainder = new ULL[shift_blocks + (mask != 0)];
                for (int i = 1; i < GET_OCC(v); i++) {
                    if (GET_OCC(AT(v, i)) < GET_OCC(modulus)) continue;
                    int n = GET_OCC(AT(v, i)) / GET_OCC(modulus) + GET_OCC(AT(v, i)) % GET_OCC(modulus);
                    auto it = powers.find(n);

                    // Shift the denominator
                    if (shift_blocks) std::memcpy(num::GET_ABS_DATA(AT(v, i)), remainder, shift_blocks);
                    if (mask) *(remainder + shift_blocks) = num::aux::GET(AT(v, i), shift_blocks) & mask;
                    num::RSHIFT(AT(v, i), shift);

                    num::aux::modular::SREM(AT(v, i), modulus, inv); // scaled remainder
                    if (it == powers.end()) {
                        powers.emplace(
                                std::make_pair(n, (pow = num::aux::modular::RADIX_POW(powers[2], n, modulus, inv))));
                    } else pow = it->second;
                    num::aux::modular::MODMUL(AT(v, i), pow, modulus, inv);

                    // redo shifting
                    num::LSHIFT(AT(v, i), shift);
                    if (shift_blocks) std::memcpy(remainder, num::GET_ABS_DATA(AT(v, i)), shift_blocks);
                    if (mask) *(num::GET_ABS_DATA(AT(v, i)) + shift_blocks) |= *(remainder + shift_blocks) & mask;
                }
                delete[] remainder;
                num::LSHIFT(modulus, shift);
            }
        }
    }
}