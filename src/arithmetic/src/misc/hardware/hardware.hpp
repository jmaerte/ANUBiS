//
// Created by jmaerte on 24.03.20.
//

#ifndef ANUBIS_SUPERBUILD_HARDWARE_HPP
#define ANUBIS_SUPERBUILD_HARDWARE_HPP

#include "../../../include/arithmetic/constants.hpp"

typedef unsigned long long ULL;

#define HAVE_HARDWARE_INSTRUC 1

#ifdef __SIZEOF_INT128__     // GNU C
#include <immintrin.h>
    static const unsigned __int128 ULL_MASK = (unsigned __int128) (-1ULL);

    static inline unsigned char adc(unsigned char c_in, ULL a, ULL b, ULL* res) {
        unsigned char c_out;
        b += c_in;
        *res = a + b;
        return (b < c_in) || (*res < b);
    }
    static inline ULL mul(ULL a, ULL b, ULL* overflow) {
        unsigned __int128 prod =  a * (unsigned __int128)b;
        *overflow = prod >> 64;
        return (ULL) prod;
    }
    static inline void add_mul(ULL* mem, ULL a, ULL b) {
         unsigned __int128 prod =  a * (unsigned __int128)b;
         *mem++ += prod & ULL_MASK;
         *mem += prod >> 64;
    }
    static inline void set_mul(ULL* mem, ULL a, ULL b) {
         unsigned __int128 prod =  a * (unsigned __int128)b;
         *mem++ = prod & ULL_MASK;
         *mem = prod >> 64;
    }
    static inline ULL mulh(ULL a, ULL b) {
         unsigned __int128 prod =  a * (unsigned __int128)b;
         return prod >> 64;
    }
    static inline void set_div(ULL* mem, ULL a_hi, ULL a_lo, ULL b) {
         unsigned __int128 div = (((unsigned __int128)a_hi) << 64 | ((unsigned __int128)a_lo)) / b;
        *mem = div & ULL_MASK;
    }
    static inline ULL udiv(ULL high, ULL low, ULL div, ULL* remainder) {
        unsigned __int128 denom = (((unsigned __int128)high) << 64 | ((unsigned __int128)low));
        *remainder = denom % div;
        return denom / div;
    }

#elif defined(_M_X64) || defined(_M_ARM64)
#include <intrin.h>
    #define adc _addcarry_u64
    #define mulh __umulh
    #define udiv _udiv128
    #define mul _umul128
    static inline void add_mul(ULL* mem, ULL a, ULL b) {
        *mem++ += a * b;
        *mem += mulh(a, b);
    }
    static inline void set_mul(ULL* mem, ULL a, ULL b) {
        *mem++ = a * b;
        *mem = mulh(a, b);
    }
    static inline void set_div(ULL* mem, ULL a_hi, ULL a_lo, ULL b) {
        ULL remainder = 0ULL;
        *mem = _udiv128(a_hi, a_lo, b, &remainder);
    }

#elif defined(_M_IA64)
#include <intrin.h>
    #define adc _addcarry_u64
    #define udiv _udiv128
    #define mul _umul128
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
    static inline void set_div(ULL* mem, ULL a_hi, ULL a_lo, ULL b) {
        ULL remainder = 0ULL;
        *mem = _udiv128(a_hi, a_lo, b, &remainder);
    }

#else
# undef HAVE_HARDWARE_INSTRUC
static char adc(char c, ULL a, ULL b, ULL* res) {
    if (c) {
        *res = a + b;
        return (*res < a);
    } else {
        b += c;
        *res = a + b;
        c = (b < c || *res < b);
    }
}
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

#endif //ANUBIS_SUPERBUILD_HARDWARE_HPP
