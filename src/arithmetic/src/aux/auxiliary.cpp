
#include "../arithmetic.hpp"
#include "../operator.hpp"
#include "hardware/hardware.hpp"
#include <cstring>

namespace jmaerte {
    namespace arith {
        namespace num {
            namespace aux {
/***********************************************************************************************************************
 * NUM - AUX
 **********************************************************************************************************************/

                void SET(ap_int n, int pos, ULL val) {
                    if (pos >= GET_SIZE(n)) ENLARGE(n, pos + 1);
                    *(GET_ABS_DATA(n) + pos) = val;
                    ULL occ = GET_OCC(n);
                    if (pos > occ && val != 0ULL) occ = pos;
                    else STRIP(GET_ABS_DATA(n), occ);
                    SET_OCC(n, occ);
                }

                void ADD(ap_int a, ULL lambda, int shift, ap_int const b) {
                    if (GET_SIGN(a) != GET_SIGN(b)) {
                        SWITCH_SIGN(b);
                        SUB(a, lambda, shift, b);
                        SWITCH_SIGN(b);
                    } else {
                        int n = 0;
                        if (GET_OCC(a) <= GET_OCC(b) + shift) {
                            n = GET_OCC(b) + shift;
                            if (mulh(lambda, GET_LEADING(b))) n++;
                        } else {
                            n = GET_OCC(a);
                        }
                        ENLARGE(a, n + 1);
                        ULL* prod = new ULL[2];
                        ULL* dat_a = GET_ABS_DATA(a);
                        ULL carry_s = 0ULL;
                        ULL carry_t = 0ULL;
                        for (int i = 0; i < n; i++) {
                            set_mul(prod, lambda, GET(b, i - shift));
                            *(dat_a + i) += carry_t;
                            // if this happens dat_a + i = 0, thus (dat_a + i) + ... won't overflow.
                            *(prod + 1) += (*(dat_a + i) < carry_t);
                            *prod += carry_s;
                            carry_t = (*prod < carry_s);
                            // so if *(prod + 1) is maximum the overflow above happened and therefore this here can't overflow.
                            *(dat_a + i) += *prod;
                            // therefore this sum is always non-overflowing.
                            carry_s = *(prod + 1) + (*(dat_a + i) < *prod);
                        }
                        delete[] prod;
                        if (carry_s | carry_t) {
                            // we know that this can't overflow, because n is calculated as such.
                            *(dat_a + n) += carry_s + carry_t;
                            SET_OCC(a, n + 1);
                        } else SET_OCC(a, n);
                    }
                }

                /**
                 *  subtract lambda * b from a.
                 * @param a
                 * @param lambda (int)
                 * @param shift shift lambda by beta^shift.
                 * @param b
                 */
                void SUB(ap_int a, ULL lambda, int shift, ap_int const b) {
                    if (GET_SIGN(a) != GET_SIGN(b)) {
                        SWITCH_SIGN(b);
                        ADD(a, lambda, shift, b);
                        SWITCH_SIGN(b);
                    } else {
                        ULL n = 0ULL;
                        bool sign = false;
                        if (GET_OCC(a) <= GET_OCC(b) + shift) {
                            n = GET_OCC(b) + shift;
                            if (mulh(lambda, GET_LEADING(b))) n++;
                        } else {
                            n = GET_OCC(a);
                        }
                        for (; n > 0 && GET(a, n - 1) == lambda * GET(b, n - 1 - shift) + mulh(lambda, GET(b, n - 2 - shift)); ) {
                            REMOVE(a, --n);
                        }
                        sign = GET(a, n - 1) < lambda * GET(b, n - 1 - shift) + mulh(lambda, GET(b, n - 2 - shift));
                        ENLARGE(a, n);
                        ULL* dat_a = GET_ABS_DATA(a);
                        ULL* prod = new ULL[2];
                        ULL carry_t, carry_s = 0ULL;
                        if (sign) {
                            for (int i = 0; i < n; i++) {
                                set_mul(prod, lambda, GET(b, i - shift));
                                *(prod + 1) += (*(dat_a + i) < carry_t);
                                *(dat_a + i) -= carry_t;
                                *prod += carry_s;
                                carry_t = (*prod < carry_s);
                                carry_s = *(prod + 1) + (*(dat_a + i) > *prod);
                                *(dat_a + i) = *prod - *(dat_a + i);
                            }
                        } else {
                            for (int i = 0; i < n; i++) {
                                set_mul(prod, lambda, GET(b, i - shift));
                                *(prod + 1) += (*(dat_a + i) < carry_t);
                                *(dat_a + i) -= carry_t;
                                *prod += carry_s;
                                carry_t = (*prod < carry_s);
                                carry_s = *(prod + 1) + (*(dat_a + i) < *prod);
                                *(dat_a + i) -= *prod;
                            }
                        }
                        delete[] prod;
                        if (carry_s | carry_t) {
                            // we know that this can't overflow, because n and sign are calculated as such.
                            if (sign) {
                                *(dat_a + n) = carry_s + carry_t - *(dat_a + n);
                            } else {
                                *(dat_a + n) -= carry_s + carry_t;
                            }
                        }
                        STRIP(dat_a, n);
                        SET_OCC(a, n);
                        SET_SIGN(a, sign ^ GET_SIGN(a));
                    }
                }

                ap_int iADD_DATA(ULL* a, int l_a, ULL* b, int l_b) {
                    int l = MAX(l_a, l_b);
                    ULL* data = new ULL[l + 1] { };
                    ULL* it = data;
                    ULL* end = it + l;
                    int occ = l;
                    ULL v_b;
                    ULL carry = 0ULL;
                    for ( ; it != end; ) {
                        v_b = *b++ + carry;
                        *it += *a++ + v_b;
                        carry = (v_b < carry || *it++ < v_b);
                    }
                    while (carry) {
                        *end += carry;
                        carry = (*end++ < carry);
                        occ++;
                    }
                    return NEW(data, l + 1, occ, false);
                }

                /**
                 * Adds the raw data blocks pointed to by a and b.
                 *
                 * @param a pointer to destination block
                 * @param b pointer to addend block
                 * @param n occupation of a
                 * @param l occupation of b
                 * @return
                 */
                int ADD_DATA(ULL* a, ULL* b, int n, int l) {
                    ULL carry = ADD_DATA_RANGE(a, b, a + l);
                    int overflow = n - l;
                    while (carry) {
                        *a += carry;
                        carry = (*a++ < carry);
                        overflow++;
                    }
                    return MAX(overflow, 0);
                }

                bool SUB_DATA(ULL* a, ULL* b, int n, int l) {
                    bool sign;
                    ULL* end_a = a + l;
                    if (n == l) {
                        ULL* end_b = b + l;
                        while (end_a != a && *--end_a == *--end_b) {
                            *end_a = 0ULL;
                            l--;
                        }
                        if (end_a == a) return false;
                        sign = *end_a < *end_b;
                        end_a++;
                    } else sign = n > l;
                    ULL carry = SUB_DATA_RANGE(a, b, sign, end_a);
                    // only in case n > l we need to perform the carry...
                    ULL temp = 0ULL;
                    while (carry) {
                        if (sign) {
                            temp = (carry < *a);
                            *a = carry - *a;
                            a++;
                        } else {
                            temp = (*a < carry);
                            *a++ -= carry;
                        }
                        carry = temp;
                    }
                    return sign;
                }

                int COMPARE_RAW(ULL* a, ULL* b, int length) {
                    ULL* stop = a - length - 1;
                    for (; a != stop; ) {
                        if (*--a != *--b) return *a > *b ? 1 : -1;
                    }
                    return 0;
                }
            }
        }
    }
}