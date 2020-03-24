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
                 * result is just a part of a itself.
                 * New occupation of a is l_a + l_b + result of this method call.
                 */
                int REC_MUL_DATA(ULL* a, int l_a, ULL* b, int l_b, int shift) {
                    int n = MAX(l_a, l_b);
                    if (n <= 20) {
                        // do long mult
                        ULL* cpy = new ULL[l_a];
                        std::copy(cpy, cpy + l_a, a);

                        ULL* prod = new ULL[2];
                        int i = 0;
                        ULL carry;
                        for (ULL* it = cpy; it != cpy + l_a; it++, i++) {
                            carry = aux_mul(a + shift + i, *it, b, l_b, prod);
                        }
                        delete[] cpy;
                        delete[] prod;
                        return carry != 0;
                    } else {
                        unsigned int split = n / 2 + n % 2;
                        ap_int sum_a = iADD_DATA(a, split, a + split, l_a - split);
                        ap_int sum_b = iADD_DATA(b, split, b + split, l_b - split);
                        SET_OCC(sum_a, GET_OCC(sum_a) + GET_OCC(sum_b) + REC_MUL_DATA(GET_ABS_DATA(sum_a), GET_OCC(sum_a), GET_ABS_DATA(sum_b), GET_OCC(sum_b), 0));
                        DELETE(sum_b);

                        REC_MUL_DATA(a, split, b, split, shift);
                        int carry = REC_MUL_DATA(a + split, l_a, b + split, l_b, shift + split);

                        SUB_DATA(GET_ABS_DATA(sum_a), a, GET_OCC(sum_a), 2 * split);
                        SUB_DATA(GET_ABS_DATA(sum_a), a + 2 * split, GET_OCC(sum_a), l_a + l_b - 1 + carry - 2 * split);

                        ADD_DATA(a + split, GET_ABS_DATA(sum_a), 2 * split, GET_OCC(sum_a));
                        DELETE(sum_a);
                        return carry;
                    }
                }

                int iREC_MUL_DATA(ULL* result, ULL* a, int l_a, ULL* b, int l_b) {
                    if (l_a < 0 || l_b < 0) return 0;
                    int n = MAX(l_a, l_b);
                    if (n <= 20) {
                        // do long mult
                        ULL* prod = new ULL[2];
                        ULL carry;
                        int i = 0;
                        for (ULL* it_a = a; it_a != a + l_a; it_a++, i++) {
                            carry = aux_mul(result + i, *it_a, b, l_b, prod);
                        }
                        delete[] prod;
                        return carry != 0;
                    } else {
                        int split = n / 2 + n % 2;
                        ap_int sum_a = iADD_DATA(a, split, a + split, l_a - split);
                        ap_int sum_b = iADD_DATA(b, split, b + split, l_b - split);
                        SET_OCC(sum_a, GET_OCC(sum_a) + GET_OCC(sum_b) + REC_MUL_DATA(GET_ABS_DATA(sum_a), GET_OCC(sum_a), GET_ABS_DATA(sum_b), GET_OCC(sum_b), 0));
                        DELETE(sum_b);

                        iREC_MUL_DATA(result, a, split, b, split);
                        int carry = iREC_MUL_DATA(result + 2 * split, a + split, l_a, b + split, l_b);

                        SUB_DATA(GET_ABS_DATA(sum_a), a, GET_OCC(sum_a), 2 * split);
                        SUB_DATA(GET_ABS_DATA(sum_a), a + 2 * split, GET_OCC(sum_a), l_a + l_b - 1 + carry - 2 * split);

                        ADD_DATA(a + split, GET_ABS_DATA(sum_a), 2 * split, GET_OCC(sum_a));
                        DELETE(sum_a);
                        return carry;
                    }
                }

                int REC_MULL_DATA(ULL* a, int l_a, ULL* b, int l_b, int shift, int m) {
                    if (shift >= m) return 0;
                    int n_a = MIN(l_a, m);
                    int n_b = MIN(l_b, m);
                    int n = MAX(l_a, l_b);
                    if (n <= 20) {
                        // do long mult
                        ULL* cpy = new ULL[n_a];
                        std::copy(cpy, cpy + n_a, a);
                        ULL* prod = new ULL[2];
                        ULL carry;
                        int i = 0;
                        for (ULL* it = cpy; it != cpy + n_a; it++, i++) {
                            carry = aux_mul(a + shift + i, *it, b, n_b - i, prod);
                        }
                        delete[] cpy;
                        delete[] prod;
                        return carry != 0;
                    } else {
                        int split = n / 2 + n % 2;
                        ap_int sum_a = iADD_DATA(a, split, a + split, l_a - split);
                        ap_int sum_b = iADD_DATA(b, split, b + split, l_b - split);
                        SET_OCC(sum_a, GET_OCC(sum_a) + GET_OCC(sum_b) + REC_MULL_DATA(GET_ABS_DATA(sum_a), GET_OCC(sum_a), GET_ABS_DATA(sum_b), GET_OCC(sum_b), 0, m - shift - split));
                        DELETE(sum_b);

                        REC_MULL_DATA(a, split, b, split, shift, m);
                        int carry = REC_MULL_DATA(a + split, l_a, b + split, l_b, shift + split, m);

                        SUB_DATA(GET_ABS_DATA(sum_a), a, GET_OCC(sum_a), 2 * split);
                        SUB_DATA(GET_ABS_DATA(sum_a), a + 2 * split, GET_OCC(sum_a), l_a + l_b - 1 + carry - 2 * split);

                        ADD_DATA(a + split, GET_ABS_DATA(sum_a), 2 * split, GET_OCC(sum_a));
                        DELETE(sum_a);
                        return carry;
                    }
                }

                int REC_MULH_DATA(ULL* a, int l_a, ULL* b, int l_b, int shift, int m) {
                    if (l_a + l_b < m - shift) return 0;
                    int n = MAX(l_a, l_b);
                    if (n <= 20) {
                        // do long mult
                        ULL* cpy = new ULL[l_a];
                        std::copy(cpy, cpy + l_a, a);
                        ULL* prod = new ULL[2];
                        ULL carry;
                        int i = 0;
                        for (ULL* it = cpy; it != a + l_a; it++, i++) {
                            carry = aux_mul(a + shift + i, *it, b + m - shift, l_b - m + shift, prod);
                        }
                        delete[] cpy;
                        delete[] prod;
                        return carry != 0;
                    } else {
                        int split = n / 2 + n % 2;
                        ap_int sum_a = iADD_DATA(a, split, a + split, l_a - split);
                        ap_int sum_b = iADD_DATA(b, split, b + split, l_b - split);
                        SET_OCC(sum_a, GET_OCC(sum_a) + GET_OCC(sum_b) + REC_MULH_DATA(GET_ABS_DATA(sum_a), GET_OCC(sum_a), GET_ABS_DATA(sum_b), GET_OCC(sum_b), 0, m - shift - split));
                        DELETE(sum_b);

                        REC_MULH_DATA(a, split, b, split, shift, m);
                        int carry = REC_MULH_DATA(a + split, l_a, b + split, l_b, shift + split, m);

                        SUB_DATA(GET_ABS_DATA(sum_a), a, GET_OCC(sum_a), 2 * split);
                        SUB_DATA(GET_ABS_DATA(sum_a), a + 2 * split, GET_OCC(sum_a), l_a + l_b - 1 + carry - 2 * split);

                        ADD_DATA(a + split, GET_ABS_DATA(sum_a), 2 * split, GET_OCC(sum_a));
                        DELETE(sum_a);
                        return carry;
                    }
                }

                int iREC_MULL_DATA(ULL* result, ULL* a, int l_a, ULL* b, int l_b, int shift, int m) {
                    if (m <= 0) return 0;
                    if (l_a < 0 || l_b < 0) return 0;
                    int n_a = MIN(l_a, m);
                    int n_b = MIN(l_b, m);
                    int n = MAX(l_a, l_b);
                    if (n <= 20) {
                        // do long mult
                        ULL* prod = new ULL[2];
                        ULL carry;
                        int i = 0;
                        for (ULL* it_a = a; it_a != a + n_a; it_a++, i++) {
                            carry = aux_mul(a + i + shift, *it_a, b, n_b - i, prod);
                        }
                        delete[] prod;
                        return carry != 0;
                    } else {
                        int split = n / 2 + n % 2;
                        ap_int sum_a = iADD_DATA(a, split, a + split, l_a - split);
                        ap_int sum_b = iADD_DATA(b, split, b + split, l_b - split);
                        SET_OCC(sum_a, GET_OCC(sum_a) + GET_OCC(sum_b) + REC_MULL_DATA(GET_ABS_DATA(sum_a), GET_OCC(sum_a), GET_ABS_DATA(sum_b), GET_OCC(sum_b), 0, m - shift - split));
                        DELETE(sum_b);

                        iREC_MULL_DATA(result, a, split, b, split, shift, m);
                        int carry = iREC_MULL_DATA(result + 2 * split, a + split, l_a, b + split, l_b, shift + 2 * split, m);

                        SUB_DATA(GET_ABS_DATA(sum_a), a, GET_OCC(sum_a), 2 * split);
                        SUB_DATA(GET_ABS_DATA(sum_a), a + 2 * split, GET_OCC(sum_a), l_a + l_b - 1 + carry - 2 * split);

                        ADD_DATA(a + split, GET_ABS_DATA(sum_a), 2 * split, GET_OCC(sum_a));
                        DELETE(sum_a);
                        return carry;
                    }
                }

                int REC_SQR_DATA(ULL* dat, int n, int shift) {
                    if (n <= 20) {
                        // do long mult
                        ULL* cpy = new ULL[n];
                        std::copy(cpy, cpy + n, dat);

                        ULL* prod = new ULL[2];
                        int i = 0;
                        ULL carry1;
                        ULL carry2;
                        for (ULL* it = cpy; it != cpy + n; it++, i++) {
                            int j = i + 1;

                            set_mul(prod, *it, *it);

                            *(dat + 2 * i + shift) += *prod;
                            carry1 = *(prod + 1) + (*(dat + 2 * i + shift) < *prod);

                            for (ULL* it_b = cpy + j; it_b != cpy + n; it_b++, j++) {
                                set_mul(prod, *it, *it_b);

                                ULL val1 = *prod;
                                val1 += carry1;
                                carry1 = (val1 < carry1);
                                *prod += val1;
                                carry1 += (*prod < val1);
                                carry1 += carry2;
                                carry2 = (carry1 < carry2);
                                carry1 += *(prod + 1);
                                carry2 += (carry1 < *(prod + 1));

                                *(dat + i + j + shift) += *prod;
                                if (*(dat + i + j + shift) < *prod) {
                                    carry1++;
                                    carry2 += (carry1 == 0);
                                }
                            }
                            *(dat + i + n + shift) = carry1;
                            *(dat + i + n + shift + 1) = carry2;
                        }
                        delete[] cpy;
                        delete[] prod;
                        return (carry1 != 0) + (carry2 != 0);
                    } else {
                        unsigned int k = n / 2 + n % 2;
                        ap_int sum = iADD_DATA(dat, k, dat + k, n - k);
                        SET_OCC(sum, 2 * GET_OCC(sum) + REC_SQR_DATA(GET_ABS_DATA(sum), GET_OCC(sum), 0));

                        REC_SQR_DATA(dat, k, shift);
                        int carry = REC_SQR_DATA(dat + k, n, shift + k);

                        SUB_DATA(GET_ABS_DATA(sum), dat, GET_OCC(sum), 2 * k);
                        SUB_DATA(GET_ABS_DATA(sum), dat + 2 * k, GET_OCC(sum), 2 * n - 1 + carry - 2 * k);

                        ADD_DATA(dat + k, GET_ABS_DATA(sum), 2 * k, GET_OCC(sum));
                        DELETE(sum);
                        return carry;
                    }
                }

                int iREC_SQR_DATA(ULL* result, ULL* dat, int n) {
                    if (n <= 20) {
                        // do long mult
                        ULL* cpy = new ULL[n];
                        std::copy(cpy, cpy + n, dat);

                        ULL* prod = new ULL[2];
                        int i = 0;
                        ULL carry1;
                        ULL carry2;
                        for (ULL* it = cpy; it != cpy + n; it++, i++) {
                            int j = i + 1;

                            set_mul(prod, *it, *it);

                            *(result + 2 * i) += *prod;
                            carry1 = *(prod + 1) + (*(result + 2 * i) < *prod);

                            for (ULL* it_b = cpy + j; it_b != cpy + n; it_b++, j++) {
                                set_mul(prod, *it, *it_b);

                                ULL val1 = *prod;
                                val1 += carry1;
                                carry1 = (val1 < carry1);
                                *prod += val1;
                                carry1 += (*prod < val1);
                                carry1 += carry2;
                                carry2 = (carry1 < carry2);
                                carry1 += *(prod + 1);
                                carry2 += (carry1 < *(prod + 1));

                                *(result + i + j) += *prod;
                                if (*(result + i + j) < *prod) {
                                    carry1++;
                                    carry2 += (carry1 == 0);
                                }
                            }
                            *(result + i + n) = carry1;
                            if (carry2) *(result + i + n + 1) = carry2; // the last carry2 is always 0.
                        }
                        delete[] cpy;
                        delete[] prod;
                        return (carry1 != 0);
                    } else {
                        unsigned int k = n / 2 + n % 2;
                        ap_int sum = iADD_DATA(dat, k, dat + k, n - k);
                        SET_OCC(sum, 2 * GET_OCC(sum) - 1 + REC_SQR_DATA(GET_ABS_DATA(sum), GET_OCC(sum), 0));

                        iREC_SQR_DATA(result, dat, n);
                        int carry = iREC_SQR_DATA(result, dat + k, n);

                        SUB_DATA(GET_ABS_DATA(sum), dat, GET_OCC(sum), 2 * k);
                        SUB_DATA(GET_ABS_DATA(sum), dat + 2 * k, GET_OCC(sum), 2 * n - 1 + carry - 2 * k);

                        ADD_DATA(dat + k, GET_ABS_DATA(sum), 2 * k, GET_OCC(sum));
                        DELETE(sum);
                        return carry;
                    }
                }
            }
        }
    }
}