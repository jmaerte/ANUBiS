#include "arithmetic.hpp"
#include "arithmetic/operator.hpp"

static const unsigned long long t[6] = {
        0xFFFFFFFF00000000ull,
        0x00000000FFFF0000ull,
        0x000000000000FF00ull,
        0x00000000000000F0ull,
        0x000000000000000Cull,
        0x0000000000000002ull
};

#define HAVE_FAST_mul64 1

#ifdef __SIZEOF_INT128__     // GNU C
    static const unsigned __int128 ULL_MASK = (unsigned __int128) ULL_MAX;
    static inline void add_mul(ULL* mem, ULL a, ULL b) {
         unsigned __int128 prod =  a * (unsigned __int128)b;
         *mem++ += prod & ULL_MAX;
         *mem += prod >> 64;
    }
    static inline void set_mul(ULL* mem, ULL a, ULL b) {
         unsigned __int128 prod =  a * (unsigned __int128)b;
         *mem++ = prod & ULL_MAX;
         *mem = prod >> 64;
    }
    static inline ULL mulh(ULL a, ULL b) {
         unsigned __int128 prod =  a * (unsigned __int128)b;
         return prod >> 64;
    }

#elif defined(_M_X64) || defined(_M_ARM64)
    #include <intrin.h>

    static inline void add_mul(ULL* mem, ULL a, ULL b) {
        *mem++ += a * b;
        *mem += mulh(a, b);
    }
    static inline void set_mul(ULL* mem, ULL a, ULL b) {
        *mem++ = a * b;
        *mem = mulh(a, b);
    }
    #define mulh __umulh

#elif defined(_M_IA64)
    #include <intrin.h>
    static inline void add_mul(ULL* mem, ULL a, ULL b) {
        *mem += _umul128(a, b, mem + 1);
    }
    static inline void set_mul(ULL* mem, ULL a, ULL b) {
        *mem = _umul128(a, b, mem + 1);
    }
    static inline ULL mulh(ULL a, ULL b) {
        ULL res;
        (void)_umul128(a, b, &res);
        return res;
    }

#else
    # undef HAVE_FAST_mul64
    static void set_mul(ULL* mem, ULL a, ULL b) {
        ULL u = a & L_MASK;
        ULL temp = (a & L_MASK) * (b & L_MASK);
        ULL w3 = temp & L_MASK;
        ULL k = temp >> 32;

        a >>= 32;
        temp = a * (b & L_MASK) + k;
        k = temp & L_MASK;
        ULL w1 = temp >> 32;

        b >>= 32;
        temp = u * b + k;
        k = temp >> 32;

        *mem++ = (temp << 32) + w3;
        *mem = a * b + w1 + k;
    }
    static void add_mul(ULL* mem, ULL a, ULL b) {
        ULL u = a & L_MASK;
        ULL temp = (a & L_MASK) * (b & L_MASK);
        ULL w3 = temp & L_MASK;
        ULL k = temp >> 32;

        a >>= 32;
        temp = a * (b & L_MASK) + k;
        k = temp & L_MASK;
        ULL w1 = temp >> 32;

        b >>= 32;
        temp = u * b + k;
        k = temp >> 32;

        *mem++ += (temp << 32) + w3;
        *mem += a * b + w1 + k;
    }
    static ULL mulh(ULL a, ULL b) {
        ULL a_lo = a & L_MASK;
        ULL b_lo = b & L_MASK;
        ULL a_hi = a >> 32;
        ULL a_lo = b >> 32;

        ULL mid1 = a_lo * b_hi;
        ULL mid2 = b_lo * a_hi;

        ULL carry = ((mid1 & L_MASK) + (mid2 & L_MASK) + (a_lo * b_lo >> 32)) >> 32;
        return a_hi * b_hi + (mid1 >> 32) + (mid2 >> 32) + carry;
    }
#endif

namespace jmaerte {
    namespace arith {
        namespace num {
            namespace aux {
/***********************************************************************************************************************
 * NUM - AUX
 **********************************************************************************************************************/

                inline void STRIP(ULL const* a, ULL& occ) {
                    const ULL* i = a + occ - 1;
                    for (; occ > 0; occ--) {
                        if (*(a + occ - 1)) return;
                    }
                }

                void SET(ap_int n, int pos, ULL val) {
                    if (pos >= GET_SIZE(n)) ENLARGE(n, pos + 1);
                    *(GET_ABS_DATA(n) + pos) = val;
                    ULL occ = GET_OCC(n);
                    if (pos > occ && val != 0ULL) occ = pos;
                    else STRIP(GET_ABS_DATA(n), occ);
                    SET_OCC(n, occ);
                }

                inline ULL GET(ap_int n, int pos) {
                    if (pos < 0 || pos >= GET_OCC(n)) return 0ULL;
                    return *(GET_ABS_DATA(n) + pos);
                }

                inline ULL GET_LEADING(ap_int const n) {
                    return *(GET_ABS_DATA(n) + GET_OCC(n) - 1);
                }

                inline void REMOVE(ap_int n, int i) {
                    SET(n, i, 0ULL);
                }

                /**
                 * @param result result memory block. But care; we are assuming that result reserves enough space to save the product!
                 * @param a
                 * @param b
                 * @param a_occ range of occupation of a, i.e. how many digits does a have in base 2^64
                 * @param b_occ range of occupation of b, i.e. how many digits does b have in base 2^64
                 */
                // DEPRECATED
//                void KMUL(ULL* result, bool offset, ULL const* a, ULL a_occ, bool a_off, ULL const* b, ULL b_occ, bool b_off) {
//                    STRIP(a, a_off, a_occ);
//                    STRIP(b, b_off, b_occ);
//                    std::cout << offset << " " << *a << " " << a_occ << " " << a_off << " " << *b << " " << b_occ << " " << b_off << std::endl;
//                    if (a_occ == 0 || b_occ == 0) return;
//                    if (a_occ < b_occ) {
//                        std::swap(a, b);
//                        std::swap(a_occ, b_occ);
//                        std::swap(a_off, b_off);
//                    }
//                    if (a_occ > 1) {
//                        ULL r = a_occ + a_occ % 2;
//                        ULL n = r / 2;
//                        ULL split = n / 2;
//                        if (a_occ % 2 && a_off) split++;
//                        bool a_mid_off = (n % 2 != 0) != a_off;
//                        bool b_mid_off = (n % 2 != 0) != b_off;
//                        KMUL(result, offset, a, n, a_off, b, b_occ < n ? b_occ : n, b_off);
//                        KMUL(result + split, a_mid_off != b_off, a + split, a_occ - n, a_mid_off, b, b_occ, b_off);
//                        if (b_occ > n) {
//                            KMUL(result + n, offset, a + split, a_occ - n, a_mid_off, b + split, b_occ - n, b_mid_off);
//                            KMUL(result + split, b_mid_off != a_off, a, a_occ, a_off, b + split, b_occ - n, b_mid_off);
//                        }
//                    } else {
//                        ULL prod = (a_off ? (*a >> 32) : (*a & L_MASK)) * (b_off ? (*b >> 32) : (*b & L_MASK));
//                        std::cout << prod << std::endl;
//                        if (offset) {
//                            if (prod & H_MASK) {
//                                *(result + 1) += prod >> 32;
//                            }
//                            *result += prod << 32;
//                        } else {
//                            *result += prod;
//                        }
//                    }
//                }

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
                        int n = 0;
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
                        ULL carry_lambda = 0ULL;
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
                            // we know that this can't overflow, because n is calculated as such.
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

                ap_int iADD_DATA(const ULL* a, int l_a, const ULL* b, int l_b) {
                    int l = MAX(l_a, l_b);
                    ULL* data = new ULL[l + 1] { };
                    it = data;
                    end = it + l;
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
                int ADD_DATA(ULL* a, const ULL* b, int n, int l) {
                    ULL v_b;
                    ULL* end = a + l;
                    ULL carry = 0ULL;
                    for ( ; a != end; ) {
                        v_b = *b++ + carry;
                        carry = (v_b < carry);
                        *a += v_b;
                        carry += (*a++ < v_b);
                    }
                    int overflow = n - l;
                    while (carry) {
                        *a += carry;
                        carry = (*a++ < carry);
                        overflow++;
                    }
                    return MAX(overflow, 0);
                }

                bool SUB_DATA(ULL* a, const ULL* b, int n, int l) {
                    if (n != l) {
                        ULL* end_a = a + l;
                        ULL* end_b = b + l;
                        while (end_a != a && *--end_a == *--end_b) {
                            *end_a = 0ULL;
                            l--;
                        }
                        if (end_a == a) return;
                        bool sign = *end_a < *end_b;
                    } else sign = n < l;

                    ULL v_b;
                    ULL v_a;
                    ULL* end = end_a + 1;
                    ULL carry = 0ULL;
                    for ( ; a != end; ) {
                        v_b = *b++;
                        v_a = *a;
                        if (sign) {
                            v_b += carry; // v_a += carry? TODO: Prove this mathematically..
                            carry = (v_b < carry || v_b < v_a); // this is actually exclusive due to carry \in \{0,1\}
                            *a++ = v_b - v_a;
                        } else {
                            v_b += carry;
                            carry = (v_b < carry || v_a < v_b);
                            *a++ = v_a - v_b;
                        }
                    }
                    // only in case n > l we need to perform the carry...
                    ULL temp = 0ULL;
                    while (carry) {
                        if (sign) {
                            temp = (carry < *a);
                            *a = carry - *a;
                        } else {
                            temp = (*a < carry);
                            *a++ -= carry;
                        }
                        carry = temp;
                    }
                    return sign;
                }

                /**
                 * New occupation of a is l_a + l_b + result of this method call.
                 */
                int REC_MUL_DATA(ULL* a, int l_a, const ULL* b, int l_b, int shift) {
                    int n = MAX(l_a, l_b);
                    if (n <= 20) {
                        // do long mult
                        ULL* cpy = new ULL[l_a];
                        ULL* prod = new ULL[2];
                        std::copy(cpy, cpy + l_a, a);
                        ULL carry;
                        int i = 0;
                        for (ULL* it_a = cpy; it_a != a + l_a; it_a++, i++) {
                            int j = 0;
                            for (ULL* it_b = b; it_b != b + l_b; it_b++, j++) {
                                set_mul(prod, *it_a, *it_b);

                                *prod += carry;
                                carry = *(prod + 1) + (*prod < carry); // never overflows

                                *(a + i + j + shift) += *prod;
                            }
                            *(a + i + l_b + shift) = carry;
                        }
                        delete[] cpy;
                        delete[] prod;
                        return carry != 0;
                    } else {
                        // DO KARATSUBA
                        int k = n / 2 + n % 2; // k = ceil(n / 2)
                        ap_int sum_a = iSUM_DATA(a, k, a + k, l_a - k);
                        ap_int sum_b = iSUM_DATA(b, k, b + k, l_b - k);
                        SET_OCC(sum_a, GET_OCC(sum_a) + GET_OCC(sum_b) + REC_MUL_DATA(sum_a, GET_OCC(sum_a), sum_b, GET_OCC(sum_b), 0));
                        DELETE(sum_b);

                        REC_MUL_DATA(a, k, b, k, shift);
                        int carry = REC_MUL_DATA(a + k, l_a, b + k, l_b, shift + 2 * k);

                        SUB_DATA(GET_ABS_DATA(sum_a), a, 2 * k);
                        SUB_DATA(GET_ABS_DATA(sum_a), a + 2 * k, l_a + l_b + carry - 2 * k);

                        ADD_DATA(a + k, GET_ABS_DATA(sum_a), GET_OCC(sum_a));
                        DELETE(sum_a);
                        return carry;
                    }
                }

                void iREC_MUL_DATA(ULL* result, ULL* a, int l_a, const ULL* b, int l_b) {
                    if (l_a < 0 || l_b < 0) return;
                    int n = MAX(l_a, l_b);
                    if (n <= 20) {
                        // do long mult
                        ULL* prod = new ULL[2];
                        ULL carry;
                        int i = 0;
                        for (ULL* it_a = a; it_a != a + l_a; it_a++, i++) {
                            int j = 0;
                            for (ULL* it_b = b; it_b != b + l_b; it_b++, j++) {
                                set_mul(prod, *it_a, *it_b);

                                *prod += carry;
                                carry = *(prod + 1) + (*prod < carry); // never overflows

                                *(result + i + j) += *prod;
                            }
                            *(result + i + l_b) = carry;
                        }
                        delete[] prod;
                    } else {
                        // DO KARATSUBA
                        int k = n / 2 + n % 2;
                        ap_int sum_a = iSUM_DATA(a, k, a + k, l_a - k);
                        ap_int sum_b = iSUM_DATA(b, k, b + k, l_b - k);
                        SET_OCC(sum_a, GET_OCC(sum_a) + GET_OCC(sum_b) + REC_MUL_DATA(sum_a, GET_OCC(sum_a), sum_b, GET_OCC(sum_b)));
                        DELETE(sum_b);

                        iREC_MUL_DATA(result, a, k, b, k);
                        iREC_MUL_DATA(result + 2 * k, a + k, l_a - k, b + k, l_b - k);
                        SUB_DATA(GET_ABS_DATA(sum_a), result, 2 * k);
                        SUB_DATA(GET_ABS_DATA(sum_a), result + 2 * k, n - 2 * k);
                        ADD_DATA(result + k, GET_ABS_DATA(sum_a), GET_OCC(sum_a));
                        DELETE(sum_a);
                    }
                }

                void REC_MULL_DATA(ULL* a, int l_a, const ULL* b, int l_b, int shift, int trunc) {
                    if (shift >= trunc) return;
                    int n_a = MIN(l_a, trunc);
                    int n_b = MIN(l_b, trunc);
                    int n = MAX(l_a, l_b);
                    if (n <= 20) {
                        // do long mult
                        ULL* cpy = new ULL[n_a];
                        ULL* prod = new ULL[2];
                        std::copy(cpy, cpy + n_a_a, a);
                        ULL carry;
                        int i = 0;
                        for (ULL* it_a = cpy; it_a != a + n_a; it_a++, i++) {
                            int j = 0;
                            for (ULL* it_b = b; it_b != b + n_b - i; it_b++, j++) {
                                set_mul(prod, *it_a, *it_b);

                                *prod += carry;
                                carry = *(prod + 1) + (*prod < carry); // never overflows

                                *(a + i + j + shift) += *prod;
                            }
                            *(a + i + n_b + shift) = carry;
                        }
                        delete[] cpy;
                        delete[] prod;
//                        return carry != 0;
                    } else {
                        // DO KARATSUBA
                        int k = n / 2 + n % 2;
                        int next_trunc = trunc - k;
                        ap_int sum_a = iSUM_DATA(a, k, a + k, l_a - k);
                        ap_int sum_b = iSUM_DATA(b, k, b + k, l_b - k);
                        SET_OCC(sum_a, GET_OCC(sum_a) + GET_OCC(sum_b) + REC_MULL_DATA(sum_a, GET_OCC(sum_a), sum_b, GET_OCC(sum_b), 0, next_trunc));
                        DELETE(sum_b);

                        REC_MULL_DATA(a, k, b, k, shift, trunc);
                        REC_MULL_DATA(a + k, n_a, b + k, n_b, shift + 2 * k, trunc);

                        SUB_DATA(GET_ABS_DATA(sum_a), a, 2 * k);
                        SUB_DATA(GET_ABS_DATA(sum_a), a + 2 * k, trunc - shift - 2 * k);

                        ADD_DATA(a + k, GET_ABS_DATA(sum_a), GET_OCC(sum_a));
                        DELETE(sum_a);
//                        return carry;
                    }
                }

                void iREC_MULL_DATA(ULL* result, ULL* a, int l_a, const ULL* b, int l_b, int trunc) {
                    if (trunc <= 0) return;
                    if (l_a < 0 || l_b < 0) return;
                    int n_a = MIN(l_a, trunc);
                    int n_b = MIN(l_b, trunc);
                    int n = MAX(l_a, l_b);
                    if (n <= 20) {
                        // do long mult
                        ULL* prod = new ULL[2];
                        ULL carry;
                        int i = 0;
                        for (ULL* it_a = a; it_a != a + n_a; it_a++, i++) {
                            int j = 0;
                            for (ULL* it_b = b; it_b != b + n_b - i; it_b++, j++) {
                                set_mul(prod, *it_a, *it_b);

                                *prod += carry;
                                carry = *(prod + 1) + (*prod < carry); // never overflows

                                *(result + i + j) += *prod;
                            }
                            *(result + i + n_b) = carry;
                        }
                        delete[] prod;
                    } else {
                        // DO KARATSUBA
                        int k = n / 2 + n % 2;
                        ap_int sum_a = iSUM_DATA(a, k, a + k, n_a - k);
                        ap_int sum_b = iSUM_DATA(b, k, b + k, n_b - k);
                        SET_OCC(sum_a, GET_OCC(sum_a) + GET_OCC(sum_b) + REC_MUL_DATA(sum_a, GET_OCC(sum_a), sum_b, GET_OCC(sum_b), 0));
                        DELETE(sum_b);

                        iREC_MUL_DATA(result, a, k, b, k);
                        iREC_MUL_DATA(result + 2 * k, a + k, n_a - k, b + k, n_b - k);
                        SUB_DATA(GET_ABS_DATA(sum_a), result, 2 * k);
                        SUB_DATA(GET_ABS_DATA(sum_a), result + 2 * k, trunc - 2 * k);
                        ADD_DATA(result + k, GET_ABS_DATA(sum_a), GET_OCC(sum_a));
                        DELETE(sum_a);
                    }
                }

                /**
                 * This namespace contains modular algorithms (montgomery arithmetic):
                 *  * Shift the modulus N so that it becomes odd.
                 *  * Iteratively calculate the modular inverse of N respectively R.
                 *  * Montgomery multiply, i.e. reduce (AB)R^{-1} mod N for R = B^n, where B = 2^32 and R / B <= N < R.
                 *  * Calculate scaled remainder
                 *  * Calculate power of radix R.
                 */
                namespace modular {

                    inline int LOG2(ULL x) {
                        int y = (((x & (x - 1)) == 0) ? 0 : 1);
                        int j = 32;
                        int i;

                        for (i = 0; i < 6; i++) {
                            int k = (((x & t[i]) == 0) ? 0 : j);
                            y += k;
                            x >>= k;
                            j >>= 1;
                        }

                        return y;
                    }

                    inline int TRAILING_ZEROS(ULL v) {
                        int c = 64;
                        v &= -signed(v);
                        if (v) c--; // if there is a one, subtract one from the maximum zeros already.
                        if (v & 0x0000'0000'FFFF'FFFFull) c -= 32;
                        if (v & 0x0000'FFFF'0000'FFFFull) c -= 16;
                        if (v & 0x00FF'00FF'00FF'00FFull) c -= 8;
                        if (v & 0x0F0F'0F0F'0F0F'0F0Full) c -= 4;
                        if (v & 0x3333'3333'3333'3333ull) c -= 2;
                        if (v & 0x5555'5555'5555'5555ull) c--;
                        return c;
                    }

                    int ODDIFY(ap_int N) {
                        int shift = 0;
                        ULL* arr = GET_ABS_DATA(N);
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
                        int n = GET_OCC(N);
                        ap_int inv = NEW(n, false, y);
                        ap_int temp;
                        // every iteration step the number of correct bits doubles. => 3 iterations for full 64 bits.
                        // there we need to do 3 + log_2 (n) iteration steps, where n is the occupation of N.
                        for (int i = 0; i < 4 + LOG2(n); i++) {
                            OVERWRITE(temp, iMULL(N, inv, n));
                            SUB(temp, 2ULL, 0, ONE);
                            SWITCH_SIGN(temp);
                            MULL(inv, temp, n);
                        }
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
                    void MODMUL(ap_int& a, ap_int b, ap_int N, ULL N_inv) {
                        int n = GET_OCC(a);
                        ap_int c = NEW(GET_OCC(B) + 1, false, 0ULL);
                        ULL q;
                        for (int i = 0; i < n; i++) {
                            ADD(c, GET(a, i), 0, b);
                            q = ((N_inv & L_MASK) * (GET(c, 0) & L_MASK)) & L_MASK;
                            ADD(c, q, 0, N);
                            RSHIFT(c, 32);
                        }
                        DELETE(a);
                        if (COMPARE_ABS(c, N) >= 0) SUB(c, 1ULL, 0, N);
                        a = c;
                    }

                    /**
                     * Calculates the scaled remainder, i.e. an s, such that
                     *      r = - s * R^n (mod b),
                     * where r is the true remainder of a (mod b).
                     */
                    void SREM(ap_int a, ap_int N, ULL N_inv) {

                    }

                    /**
                     * Calculates R^2 (mod N) by iterated modmul with 2^64 or 2^32.
                     * I.e. block left shifts.
                     * @param N
                     * @return
                     */
                    ap_int RADIX_SQ(ap_int N) {
                        int n = GET_OCC(N);
                        int k = (n + 1) / 2;
                        ap_int c = NEW(n + 1, false, 1ULL);
                        LSHIFT_BLOCK(c, n - 1);
                        for (int i = 0; i < k; i++) {
                            LSHIFT_BLOCK(c, 2);
                            if (COMPARE_ABS(c, N) > 0) SUB(c, 1ULL, 0, N);
                        }
                        if ((n + 1) % 2) {
                            LSHIFT_BLOCK(c, 1);
                            if (COMPARE_ABS(c, N) > 0) SUB(c, 1ULL, 0, N);
                        }
                        return c;
                    }

                    void MODSQ(ap_int& a, ap_int N, ap_int inv) {
                        ap_int a_sq = SQR(a);

                    }

                    void MODRED(ap_int& a, ap_int N, ap_int inv) {

                    }

                    /**
                     * Calculates R^n (mod N)
                     * @param SQ R^2 (mod N)
                     */
                    ap_int RADIX_POW(ap_int SQ, ap_int N, ap_int N_inv) {

                    }
                }
            }
        }
    }
}