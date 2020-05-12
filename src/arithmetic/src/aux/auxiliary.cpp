
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
                    *(ABS(n) + pos) = val;
                    ULL occ = GET_OCC(n);
                    if (pos > occ && val != 0ULL) occ = pos;
                    else STRIP(ABS(n), occ);
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
                        ULL* dat_a = ABS(a);
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

                void C_ADD(ap_int a, ULL b) {
                    int n = GET_OCC(a);
                    unsigned char carry = 0;
                    for (auto it = ABS(a); it < ABS(a) + n; it++) {
                        carry = adc(carry, *it, b, it);
                    }
                    if (carry) {
                        ENLARGE(a, n);
                        *(ABS(a) + n) = 1ULL;
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
                        ULL* dat_a = ABS(a);
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

                /**
                 * WE ASSUME THAT a IS NOT A SINGLE AND THAT a IS POSITIVE, i.e. ABSOLUTE SUBTRACTION.
                 */
                void C_SUB(ap_int a, ULL b) {
                    int n = GET_OCC(a);
                    for (auto it = ABS(a); it < ABS(a) + n; it++) {
                        if (*it < b) {
                            *it -= b;
                            b = 1ULL;
                        } else *it -= b;
                    }
                    // We know that b is 0 now because b > BASE = 2^64.
                }

                /**
                 * Immutable multiplication with a single. We assume dest to be an empty place to store our product.
                 * @param coeff
                 * @param a
                 * @param dest
                 * @returns if dest is now multi.
                 */
                bool iC_MUL(ap_int coeff, ap_int a, ap_int* dest) {
                    bool multi = false;
                    ULL sign = (GET_SIGN(coeff) ^ GET_SIGN(a)) ? 1ULL : 0ULL;
                    ULL lambda = (coeff + 1)->single;
                    ULL overflow;
                    if (IS_SINGLE(a)) {
                        ULL prod = mul(lambda, (a + 1)->single, &overflow);
                        if (overflow) {
                            *(*dest + 1) = (svec_node){.value = new ULL[2] {prod, overflow}};
                            *(*dest) = (svec_node){.single = sign << 1 | 2ULL << 2 | 2ULL << 17}; // multi with sign sign and 2 occ, 2 size.
                            multi = true;
                        } else {
                            *(*dest + 1) = (svec_node){.single = prod};
                            *(*dest) = (svec_node){.single = 1ULL | sign << 1};
                        }
                    } else {
                        int n = GET_OCC(a);
                        num::ASSIGN(*dest, NEW(n + 1, sign, 0ULL));
                        ULL* it = (a + 1)->value;
                        ULL* res = (*dest + 1)->value;
                        ULL next_overflow;
                        unsigned char carry;
                        for (int i = 0; i < n; i++, it++, res++) {
                            *res = mul(lambda, *it, &next_overflow);
                            carry = adc(carry, *res, overflow, res);
                            overflow = next_overflow;
                        }
                        if (overflow) {
                            SET_OCC(*dest, n + 1);
                            *res = overflow;
                        } else SET_OCC(*dest, n);
                        multi = true;
                    }
                    return multi;
                }

                bool iC_A_MUL(ap_int coeff, ap_int a, ap_int* dest) {
                    bool multi = false;
                    ULL sign = (GET_SIGN(coeff) ^ GET_SIGN(a)) ? 1ULL : 0ULL;
                    ULL lambda = (coeff + 1)->single;
                    ULL overflow = 0ULL;
                    if (sign == GET_SIGN(*dest)) {
                        if (IS_SINGLE(*dest)) {
                            if (IS_SINGLE(a)) {
                                ULL prod = mul(lambda, (a + 1)->single, &overflow);
                                overflow += adc(0, (*dest + 1)->single, prod, &((*dest + 1)->single));
                                if (overflow) {
                                    MAKE_MULTI(*dest, overflow);
                                    multi = true;
                                }
                            } else {
                                multi = true;
                                int n = GET_OCC(a);
                                MAKE_MULTI(*dest, n + 1);
                                ULL next_overflow;
                                ULL* res = (*dest + 1)->value;
                                ULL* it = (a + 1)->value;
                                for (int i = 1; i < n; i++, res++, it++) {
                                    *res = mul(lambda, *it, &next_overflow);
                                    next_overflow += adc(0, overflow, *res, res); // can't overflow
                                    overflow = next_overflow;
                                }
                                if (overflow) {
                                    *res = overflow;
                                    SET_OCC(*dest, n + 1);
                                } else SET_OCC(*dest, n);
                            }
                        } else {
                            multi = true;
                            if (IS_SINGLE(a)) {
                                ULL prod = mul(lambda, (a + 1)->single, &overflow);
                                overflow += adc(0, *((*dest + 1)->value), prod, (*dest + 1)->value);
                                if (overflow) {
                                    unsigned char carry = adc(0, overflow, *((*dest + 1)->value + 1), (*dest + 1)->value + 1);
                                    int n = GET_OCC(*dest);
                                    int i = 2;
                                    while (i < n && carry) {
                                        carry = adc(0, carry, *((*dest + 1)->value + i), (*dest + 1)->value + i);
                                    }
                                    if (carry) {
                                        ENLARGE(*dest, n + 1);
                                        *((*dest + 1)->value + n) = carry;
                                    }
                                } else {
                                    C_ADD(*dest, prod);
                                }
                            } else {
                                ADD(*dest, lambda, 0, a);
                            }
                        }
                    } else {
                        if (IS_SINGLE(*dest)) {
                            if (IS_SINGLE(a)) {
                                ULL prod = mul(lambda, (a + 1)->single, &overflow);
                                if (overflow) {
                                    SWITCH_SIGN(*dest);
                                    if (prod < (*dest + 1)->single) overflow--;
                                    (*dest + 1)->single = prod - (*dest + 1)->single;
                                    MAKE_MULTI(*dest, overflow);
                                    multi = true;
                                } else {
                                    if ((*dest + 1)->single < prod) {
                                        SWITCH_SIGN(*dest);
                                        (*dest + 1)->single = prod - (*dest + 1)->single;
                                    } else (*dest + 1)->single -= prod;
                                }
                            } else {
                                multi = true;
                                int n = GET_OCC(a);
                                SWITCH_SIGN(*dest);
                                overflow = (*dest + 1)->single;
                                MAKE_MULTI(*dest, n + 1);
                                ULL* res = (*dest + 1)->value;
                                ULL* it = (a + 1)->value;
                                ULL next_overflow;

                                *res = mul(lambda, *it, &next_overflow);
                                if (*res < overflow) next_overflow--;
                                *res -= overflow;
                                overflow = next_overflow;
                                it++;
                                res++;

                                for (int i = 1; i < n; i++, res++, it++) {
                                    *res = mul(lambda, *it, &next_overflow);
                                    next_overflow += adc(0, *res, overflow, res);
                                    overflow = next_overflow;
                                }
                                if (overflow) {
                                    *res = overflow;
                                    SET_OCC(*dest, n + 1);
                                } else SET_OCC(*dest, n);
                            }
                        } else {
                            multi = true;
                            if (IS_SINGLE(a)) {
                                ULL prod = mul(lambda, (a + 1)->single, &overflow);
                                if (overflow) {
                                    if (*((*dest + 1)->value) < prod) overflow++; // can't overflow
                                    *((*dest + 1)->value) -= prod;
                                    int n = GET_OCC(*dest);
                                    ULL next_overflow = 0ULL;
                                    for (int i = 1; i < n && overflow; i++) {
                                        next_overflow = *((*dest + 1)->value + i) < overflow;
                                        *((*dest + 1)->value + i) -= overflow;
                                        overflow = next_overflow;
                                    }
                                    // can't overflow because every n-word number, n > 1, is larger than every single.
                                } else {
                                    C_SUB(*dest, prod);
                                }
                            } else {
                                SUB(*dest, lambda, 0, a);
                                if (GET_OCC(*dest) == 1) {
                                    multi = true;
                                    MAKE_SINGLE(*dest);
                                }
                            }
                        }
                    }
                    return multi;
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
                    ULL* end_a = a + l - 1;
                    if (n == l) {
                        ULL* end_b = b + l - 1;
                        while (l > 0 && *end_a == *end_b) {
                            *end_a = 0ULL;
                            end_a--;
                            end_b--;
                            l--;
                        }
                        if (l > 0) {
                            sign = *end_a < *end_b;
                            end_a++;
                        } else return false;
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