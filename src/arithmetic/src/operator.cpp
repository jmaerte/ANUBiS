//
// Created by jmaerte on 24.03.20.
//

#include "arithmetic/operator.hpp"
#include "arithmetic.hpp"
#include "../include/arithmetic/constants.hpp"
#include <cstdlib>

namespace jmaerte {
    namespace arith {

        double ELAPSED = 0.0;

        namespace num {
            /*
             * COMPARATORS
             */

            int COMPARE_ABS(ap_int const n_a, ap_int const n_b) {
                if (num::IS_SINGLE(n_a)) {
                    if (num::IS_SINGLE(n_b)) {
                        if ((n_a+1)->value != (n_b+1)->value) {
                            return (n_a+1)->value > (n_b+1)->value ? 1 : -1;
                        }
                        return 0;
                    } else return -1;
                } else {
                    if (num::IS_SINGLE(n_b)) return 1;
                    else {
                        int occ = GET_OCC(n_a);
                        if (occ != GET_OCC(n_b)) return occ - GET_OCC(n_b);
                        auto a = ABS(n_a) + occ;
                        auto b = ABS(n_b) + occ;
                        return aux::COMPARE_RAW(a, b, occ);
                    }
                }
            }

            bool comp::SIGNED_COMPARATOR::operator()(ap_int const &a, ap_int const &b) const {
                if ((a->single ^ b->single) & num::SIGN_MASK) return GET_SIGN(a) ? b : a;
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
                ULL *arr = ABS(n);
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
                ULL *arr = ABS(n);
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
                ULL *arr = ABS(n);
                int size = GET_OCC(n);
                std::copy_backward(arr, arr + size, arr + size + blocks);
                for (int i = 0; i < blocks; i++) *(arr + i) = 0ULL;
                SET_OCC(n, GET_OCC(n) + blocks);
            }

            /*
             * ARITHMETIC
             */

            ap_int iMUL(ap_int a, ap_int b) {
//                if (GET_OCC(a) > GET_OCC(b)) {
//                    return iMUL(b, a);
//                }
//                std::cout << "a occ: " << GET_OCC(a) << " b occ: " << GET_OCC(b) << std::endl;
//                std::cout << "a: ";
//                for (int i = 0; i < GET_OCC(a); i++) {
//                    std::cout << *(GET_ABS_DATA(a) + i) << " ";
//                }
//                std::cout << std::endl;
//                std::cout << "b: ";
//                for (int i = 0; i < GET_OCC(b); i++) {
//                    std::cout << *(GET_ABS_DATA(b) + i) << " ";
//                }
//                std::cout << std::endl;

                int occ = GET_OCC(a) + GET_OCC(b);
                ULL *result = new ULL[occ] { };
                occ -= 1 - aux::iREC_MUL_DATA(result, ABS(a), GET_OCC(a), ABS(b), GET_OCC(b));
                return NEW(result, GET_OCC(a) + GET_OCC(b), occ, GET_SIGN(a) ^ GET_SIGN(b));
            }

            void iMUL(ap_int dest, ap_int a, ap_int b) {
                int occ = GET_OCC(a) + GET_OCC(b);
                ULL *result = new ULL[occ] { };
                occ -= 1 - aux::iREC_MUL_DATA(result, ABS(a), GET_OCC(a), ABS(b), GET_OCC(b));
                *dest = (svec_node){.single = ((GET_SIGN(a) ^ GET_SIGN(b)) ? num::SIGN_MASK : 0ULL) | (((GET_OCC(a) + GET_OCC(b)) & constants::LL_MASK) << 16) | (ULL) occ};
                *(dest + 1) = (svec_node){.value = result};
            }

            void MUL(ap_int a, ap_int b) {
                int n_a = GET_OCC(a);
                int n_b = GET_OCC(b);

                ENLARGE(a, n_a + n_b);

                ULL *dat_a = ABS(a);
                ULL *dat_b = ABS(b);

                n_a -= 1 - aux::REC_MUL_DATA(dat_a, n_a, dat_b, n_b, 0);

                SET_SIGN(a, GET_SIGN(a) ^ GET_SIGN(b));
                SET_OCC(a, n_a + n_b);
            }

            void SQR(ap_int a) {
                int n = GET_OCC(a);
                ENLARGE(a);
                ULL *dat = ABS(a);

                n -= 1 - aux::REC_SQR_DATA(dat, n, 0);

                SET_SIGN(a, false);
                SET_OCC(a, n + GET_OCC(a));
            }

            ap_int iSQR(ap_int a) {
                int n = GET_OCC(a);
                ULL *result = new ULL[2 * n] { };
                ULL *dat = ABS(a);
                int occ = 2 * n - 1 + aux::iREC_SQR_DATA(result, dat, n);
                return NEW(result, 2 * n, occ, false);
            }

            ap_int iMULL(ap_int a, ap_int b, int n) {
                if (GET_OCC(a) > GET_OCC(b)) {
                    return iMUL(b, a);
                }
                ULL *result = new ULL[n] { };
                aux::iREC_MULL_DATA(result, ABS(a), GET_OCC(a), ABS(b), GET_OCC(b), 0, n);
                ULL occ = n;
                aux::STRIP(result, occ);
                return NEW(result, n, occ, GET_SIGN(a) ^ GET_SIGN(b));
            }

            void MULL(ap_int a, ap_int b, ULL n) {
                int n_a = GET_OCC(a);
                int n_b = GET_OCC(b);

                ENLARGE(a, n);

                ULL *dat_a = ABS(a);
                ULL *dat_b = ABS(b);

                aux::REC_MULL_DATA(dat_a, n_a, dat_b, n_b, 0, n);

                SET_SIGN(a, GET_SIGN(a) ^ GET_SIGN(b));
                aux::STRIP(dat_a, n);
                SET_OCC(a, n);
            }

            void MULH(ap_int a, ap_int b, ULL n) {
                int n_a = GET_OCC(a);
                int n_b = GET_OCC(b);

                ENLARGE(a, n);

                ULL *dat_a = ABS(a);
                ULL *dat_b = ABS(b);

                aux::REC_MULH_DATA(dat_a, n_a, dat_b, n_b, 0, n);

                SET_SIGN(a, GET_SIGN(a) ^ GET_SIGN(b));
                aux::STRIP(dat_a, n);
                SET_OCC(a, n);
            }

            ap_int DIV(ap_int a, ap_int b) {
                return aux::E_DIV(a, b);
            }

            ULL C_DIV(ap_int a, ULL b) {
                return aux::C_DIV(a, b);
            }

            ap_int iC_DIV(ap_int a, ULL b) {
                return aux::iC_DIV(a, b);
            }
        }

        namespace vec {

            int FIND_POS(s_ap_int_vec v, int pos) {
                int occ = GET_OCC(v);
                if (occ == 0 || num::GET_POS(AT(v, occ - 1)) < pos) return occ;
                if (num::GET_POS(AT(v, 0)) > pos) return 0;
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

            void PUT(s_ap_int_vec& v, num::ap_int n) {
                int k = FIND_POS(v, num::GET_POS(n));
                if (GET_SIZE(v) <= GET_OCC(v)) {
                    ENLARGE(v);
                }
                if (k < GET_OCC(v)) {
                    std::copy_backward(AT(v, k), AT(v, GET_OCC(v)), AT(v, GET_OCC(v) + 2));
                }
                SET(v, k, n);
                SET_OCC(v, GET_OCC(v) + 1);
            }

            /*
             * ARITHMETIC
             */

            /**
             * Addition function exclusively for vectors where every entry is a single.
             * @param a
             * @param start_a
             * @param lambda_num
             * @param b
             * @param start_b
             */
            void ADD_ALL_SINGLE(s_ap_int_vec& a, int start_a, num::ap_int lambda_num, s_ap_int_vec b, int start_b) {
//                std::cout << "add all single " << GET_OCC(b) << std::endl;

                int occ = GET_OCC(a);
                int occ_b = GET_OCC(b);

                num::ap_int it = AT(a, start_a);


//                std::cout << "a: " << a << std::endl;
//                std::cout << "a_end: " << (a + 1 + 2 * GET_SIZE(a)) << std::endl;

                int size_cpy = (occ - start_a);

//                svec_node* a_cpy = new svec_node[2 * size_cpy];
                svec_node* a_cpy = static_cast<svec_node*>(malloc(2 * size_cpy * sizeof(svec_node)));

                memcpy(static_cast<void*>(a_cpy), static_cast<void*>(a + 1 + 2 * start_a), 2 * size_cpy * sizeof(svec_node));

//                std::copy(it, AT(a, occ), a_cpy);

                SET_OCC(a, start_a);
                occ = start_a;

                bool multi = false;

                ULL lambda = (lambda_num + 1)->single;
                bool lambda_sign = num::GET_SIGN(lambda_num);

                ULL pos_a, pos_b;
                int i = 0, j = start_b;
                ULL overflow;

                for (; i < size_cpy && j != occ_b; occ++) {
//                    std::cout << "it: " << it << std::endl;
//                    std::cout << "it_cpy: " << it_cpy << std::endl;
                    if (occ >= GET_SIZE(a)) {
                        ENLARGE_RANGE(a, occ);
                        it = AT(a, occ);
//                        std::cout << "a is now " << a << " and it is " << it << std::endl;
                    }
//                    std::cout << i << " " << (b_end - j) / 2 << std::endl;
                    if ((pos_a = num::GET_POS(a_cpy + 2 * i)) < (pos_b = num::GET_POS(b + 1 + 2 * j))) {
                        a[1 + 2 * occ] = a_cpy[2 * i];
                        a[2 + 2 * occ] = a_cpy[2 * i + 1];
                        i++;
                    } else if (pos_a > pos_b) {
//                        std::copy_backward(it, a_end, a_end + 2);
                        a[1 + 2 * occ] = (svec_node){.single = 1ULL | (num::GET_SIGN(b + 1 + 2 * j) ^ lambda_sign ? 1ULL : 0ULL) << 1 | pos_b << 2};
                        a[2 + 2 * occ] = (svec_node){.single = mul((b + 2 + 2 * j)->single, lambda, &overflow)};
//                        *it = (svec_node){.single = 1ULL | (num::GET_SIGN(j) ^ lambda_sign ? 1ULL : 0ULL) << 1 | pos_b << 2};
//                        *(it + 1) = (svec_node){.single = mul((j + 1)->single, lambda, &overflow)};
                        if (overflow) {
                            num::MAKE_MULTI(it, overflow);
                            multi = true;
                        }
                        j++;
                    } else {
                        ULL x = mul(lambda, (b + 2 + 2 * j)->single, &overflow);
                        bool sign = lambda_sign ^ num::GET_SIGN(b + 1 + 2 * j);
                        if (sign ^ num::GET_SIGN(a_cpy + 2 * i)) {
                            if (overflow) {
                                num::SWITCH_SIGN(a_cpy + 2 * i);
                                if (x < (a_cpy + 2 * i + 1)->single) {
                                    overflow--;
                                }
                                (a_cpy + 2 * i + 1)->single = x - (a_cpy + 2 * i + 1)->single;
                            } else {
                                if ((a_cpy + 2 * i + 1)->single < x) {
                                    num::SWITCH_SIGN(a_cpy + 2 * i);
                                    (a_cpy + 2 * i + 1)->single = x - (a_cpy + 2 * i + 1)->single;
                                } else (a_cpy + 2 * i + 1)->single -= x;
                            }
                        } else {
                            overflow += adc(0, (a_cpy + 2 * i + 1)->single, x, &(a_cpy + 2 * i + 1)->single); // can't overflow itself
                        }
                        if (overflow) {
                            num::MAKE_MULTI(a_cpy + 2 * i, overflow);
                            multi = true;
                        } else if ((a_cpy + 2 * i + 1)->single == 0ULL) {
                            occ--;
                            i++;
                            j++;
                            continue;
                        }
                        a[1 + 2 * occ] = a_cpy[2 * i];
                        a[2 + 2 * occ] = a_cpy[2 * i + 1];
                        i++;
                        j++;
                    }
                }

                while(i < size_cpy) {
//                    std::cout << "cpy - it: " << it << std::endl;
//                    std::cout << "it_cpy: " << it_cpy << std::endl;
                    if (occ >= GET_SIZE(a)) {
                        ENLARGE_RANGE(a, occ);
                        it = AT(a, occ);
//                        std::cout << "a is now " << a << " and it is " << it << std::endl;
                    }
                    a[1 + 2 * occ] = a_cpy[2 * i];
                    a[2 + 2 * occ] = a_cpy[2 * i + 1];
                    i++;
                    occ++;
                }
                while(j < occ_b) {
//                    std::cout << "it: " << it << std::endl;
//                    std::cout << "j: " << j << std::endl;
                    if (occ >= GET_SIZE(a)) {
                        ENLARGE_RANGE(a, occ);
                        it = AT(a, occ);
                    }
                    pos_b = ((b + 1 + 2 * j)->single >> 2);
                    a[1 + 2 * occ] = (svec_node){.single = 1ULL | (num::GET_SIGN(b + 1 + 2 * j) ^ lambda_sign ? 1ULL : 0ULL) << 1 | pos_b << 2};
                    a[2 + 2 * occ] = (svec_node){.single = mul((b + 2 + 2 * j)->single, lambda, &overflow)};
                    if (overflow) {
                        num::MAKE_MULTI(it, overflow);
                        multi = true;
                    }
                    j++;
                    occ++;
                }

//                delete[] a_cpy;
                free(static_cast<void*>(a_cpy));

                // update vector flags:
                SET_OCC(a, occ);
                if (multi) a->single &= ~vec::ALL_SINGLE_MASK;
            }

            /**
             * Addition function exclusively for b being a vector where every entry is a single.
             * @param a
             * @param start_a
             * @param lambda_num
             * @param b
             * @param start_b
             */
            void ADD_SINGLE(s_ap_int_vec& a, int start_a, num::ap_int lambda, s_ap_int_vec b, int start_b) {
                num::ap_int a_end = AT(a, GET_OCC(a));
                num::ap_int b_end = AT(b, GET_OCC(b));

                num::ap_int it_a = AT(a, start_a);
                num::ap_int j = AT(b, start_b);

                ULL pos_a, pos_b;
                ULL occ = GET_OCC(a);
                for (int i = start_a; i < occ && j != b_end;) {
                    if ((pos_a = num::GET_POS(it_a)) < (pos_b = j->single >> 2)) {
                        i++;
                        it_a += 2;
                    } else if (pos_a > pos_b) {
                        if (occ + 1 > GET_SIZE(a)) {
                            ENLARGE(a);
                            it_a = AT(a, i);
                            a_end = AT(a, occ);
                        }
                        // copy j and j + 1 into a.
                        std::copy_backward(it_a, a_end, a_end + 2);
                        num::aux::iC_MUL(j, lambda, &it_a);
                        num::SET_POS(it_a, pos_b);
                        occ++;
                        a_end += 2;
                        j += 2;
                    } else {
                        num::aux::iC_A_MUL(j, lambda, &it_a);
                        if (num::GET_OCC(it_a) == 0) {
                            num::DELETE_DATA(it_a);
                            std::copy(AT(a, i + 1), AT(a, occ--), AT(a, i));
                            a_end -= 2;
                        } else {
                            it_a += 2;
                            i++;
                        }
                        j += 2;
                    }
                }

                if (occ + (b_end - j) / 2 > GET_SIZE(a)) {
                    ENLARGE(a, occ + (b_end - j) / 2);
                }
                it_a = AT(a, occ);
                occ += (b_end - j) / 2;
                while (j != b_end) {
                    num::aux::iC_MUL(j, lambda, &it_a);
                    num::SET_POS(it_a, num::GET_POS(j));
                    j += 2;
                    it_a += 2;
                }

                // update vector flags
                SET_OCC(a, occ);
            }

            /**
             * Addition for two multi number vectors with a single number scalar.
             * @param a
             * @param start_a
             * @param lambda_num
             * @param b
             * @param start_b
             */
            void ADD_SINGLE_SCALAR(s_ap_int_vec& a, int start_a, num::ap_int lambda, s_ap_int_vec b, int start_b) {
                num::ap_int a_end = AT(a, GET_OCC(a));
                num::ap_int b_end = AT(b, GET_OCC(b));

                num::ap_int it_a = AT(a, start_a);
                num::ap_int j = AT(b, start_b);

                ULL pos_a, pos_b;
                ULL occ = GET_OCC(a);
                for (int i = start_a; i < occ && j != b_end;) {
                    if ((pos_a = num::GET_POS(it_a)) < (pos_b = j->single >> 2)) {
                        i++;
                        it_a += 2;
                    } else if (pos_a > pos_b) {
                        if (occ + 1 > GET_SIZE(a)) {
                            ENLARGE(a);
                            it_a = AT(a, i);
                            a_end = AT(a, occ);
                        }
                        // copy j and j + 1 into a.
                        std::copy_backward(it_a, a_end, a_end + 2);
                        num::aux::iC_MUL(lambda, j, &it_a);
                        num::SET_POS(it_a, pos_b);
                        occ++;
                        a_end += 2;
                        j += 2;
                    } else {
                        if (num::aux::iC_A_MUL(lambda, j, &it_a)) {
                            if (num::GET_OCC(it_a) == 0) {
                                num::DELETE_DATA(it_a);
                                std::copy(AT(a, i + 1), AT(a, occ--), AT(a, i));
                                a_end -= 2;
                            } else {
                                it_a += 2;
                                i++;
                            }
                        } else {
                            if (!(it_a + 1)->single) {
                                std::copy(AT(a, i + 1), AT(a, occ--), AT(a, i));
                                a_end -= 2;
                            } else {
                                it_a += 2;
                                i++;
                            }
                        }
                        j += 2;
                    }
                }

                if (occ + (b_end - j) / 2 > GET_SIZE(a)) {
                    ENLARGE(a, occ + (b_end - j) / 2);
                }
                it_a = AT(a, occ);
                occ += (b_end - j) / 2;
                while (j != b_end) {
                    num::aux::iC_MUL(lambda, j, &it_a);
                    num::SET_POS(it_a, num::GET_POS(j));
                    j += 2;
                    it_a += 2;
                }

                // update vector flags
                SET_OCC(a, occ);
            }

            void ADD(s_ap_int_vec& a, int start_a, num::ap_int lambda, s_ap_int_vec b, int start_b) {
                num::ap_int a_end = AT(a, GET_OCC(a));
                num::ap_int b_end = AT(b, GET_OCC(b));

                num::ap_int it_a = AT(a, start_a);
                num::ap_int j = AT(b, start_b);
                ULL pos_a, pos_b;
                ULL occ = GET_OCC(a);
                for (int i = start_a; i < occ && j != b_end;) {
                    if ((pos_a = num::GET_POS(it_a)) < (pos_b = num::GET_POS(j))) {
                        i++;
                        it_a += 2;
                    } else if (pos_a > pos_b) {
                        if (occ + 1 > GET_SIZE(a)) {
                            ENLARGE(a);
                            it_a = AT(a, i);
                            a_end = AT(a, occ);
                        }
                        // copy j and j + 1 into a.
                        std::copy_backward(it_a, a_end, a_end + 2);
                        num::ap_int x = num::iMUL(lambda, j);
                        num::SET_POS(x, pos_b);
                        num::ASSIGN(it_a, x);
                        delete[] x;
                        occ++;
                        a_end += 2;
                        j += 2;
                    } else {
                        num::ap_int x = num::iMUL(lambda, j);
                        num::ADD(it_a, x);
                        num::DELETE(x);
                        if (num::GET_OCC(it_a) == 0) {
                            num::DELETE_DATA(it_a);
                            std::copy(AT(a, i + 1), AT(a, occ--), AT(a, i));
                            a_end -= 2;
                        } else {
                            it_a += 2;
                            i++;
                        }
                        j += 2;
                    }
                }
                if (occ + (b_end - j) / 2 > GET_SIZE(a)) {
                    ENLARGE(a, occ + (b_end - j) / 2);
                }
                it_a = AT(a, occ);
                occ += (b_end - j) / 2;
                while (j != b_end) {
                    num::ap_int x = num::iMUL(lambda, j);
                    num::SET_POS(x, num::GET_POS(j));
                    num::ASSIGN(it_a, x);
                    delete[] x;
                    j += 2;
                    it_a += 2;
                }
                // update vector flags
                SET_OCC(a, occ);
            }

            void MUL(s_ap_int_vec v, num::ap_int n) {
                for (int i = 0; i < GET_OCC(v); i++) {
                    num::MUL(AT(v, i), n);
                }
            }

            void MOD(s_ap_int_vec v) {
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
                    if (num::GET_OCC(AT(v, i)) < num::GET_OCC(modulus)) continue;
                    int n = num::GET_OCC(AT(v, i)) / num::GET_OCC(modulus) + num::GET_OCC(AT(v, i)) % num::GET_OCC(modulus);
                    auto it = powers.find(n);

                    // Shift the denominator
                    if (shift_blocks) std::memcpy(num::ABS(AT(v, i)), remainder, shift_blocks);
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
                    if (shift_blocks) std::memcpy(remainder, num::ABS(AT(v, i)), shift_blocks);
                    if (mask) *(num::ABS(AT(v, i)) + shift_blocks) |= *(remainder + shift_blocks) & mask;
                }
                delete[] remainder;
                num::LSHIFT(modulus, shift);
            }
        }
    }
}