#include "arithmetic.hpp"
#include "arithmetic/operator.hpp"

/***********************************************************************************************************************
 * NUM - AUX
 **********************************************************************************************************************/

void STRIP(ULL const* a, int offset, ULL& occ) {
    int j = (occ % 2) ^ offset ? 0 : 1;
    ULL const* i = a + occ / 2 - (offset ? 0 : 1 - (occ  % 2));
    if (offset && occ % 2 != 0) j--;
    for (; i != a; ) {
        if(!(*i & MASKS[j--])) {
            occ--;
        } else return;
        if (j < 0) {
            i--;
            j = 1;
        }
    }
    if (!(*i & H_MASK)) {
        occ--;
    }
    if (offset) {
        occ = 0;
    } else {
        if (!(*i & L_MASK)) {
            occ = 0;
        } else occ = 1;
    }
}

void SET_NUM_AT(num n, int pos, ULL val) {
    if (pos > GET_NUM_SIZE(n)) ENLARGE_NUM(n, pos + 1);
    int mask_index = pos % 2 ? 1 : 0;
    int shift = mask_index * 32;
    *(GET_ABS_DATA(n) + pos / 2) = (*(GET_ABS_DATA(n) + pos / 2) & MASKS[mask_index]) | ((val & MASKS[1 - mask_index]) << shift);
    ULL occ = GET_NUM_OCC(n);
    if (pos > occ) occ = pos;
    else STRIP(GET_ABS_DATA(n), false, occ);
    SET_NUM_OCC(n, occ);
}

ULL GET_NUM_AT(num n, int pos) {
    if (pos < 0 || pos >= GET_NUM_OCC(n)) return 0ULL;
    int mask_index = pos % 2 ? 0 : 1;
    int shift = (1 - mask_index) * 32;
    return (*(GET_ABS_DATA(n) + pos / 2) & MASKS[mask_index]) >> shift;
}

unsigned int GET_NUM_LEADING(num const n) {
    int k = GET_NUM_OCC(n);
    int shift = 0;
    if (k % 2 != 0) k++;
    else shift = 32;
    k /= 2;
    return (*(GET_ABS_DATA(n) + k) >> shift) & L_MASK;
}

void REMOVE_WORD(num n, int i) {
    SET_NUM_AT(n, i, 0ULL);
    // SET_NUM_AT does this already
//    if (i == GET_NUM_OCC(n) - 1) SET_NUM_OCC(n, GET_NUM_OCC(n) - 1);
}

/**
 * @param result result memory block. But care; we are assuming that result reserves enough space to save the product!
 * @param a
 * @param b
 * @param a_occ range of occupation of a, i.e. how many digits does a have in base 2^64
 * @param b_occ range of occupation of b, i.e. how many digits does b have in base 2^64
 */
void KMUL(ULL* result, bool offset, ULL const* a, ULL a_occ, bool a_off, ULL const* b, ULL b_occ, bool b_off) {
    STRIP(a, a_off, a_occ);
    STRIP(b, b_off, b_occ);
    std::cout << offset << " " << *a << " " << a_occ << " " << a_off << " " << *b << " " << b_occ << " " << b_off << std::endl;
    if (a_occ == 0 || b_occ == 0) return;
    if (a_occ < b_occ) {
        std::swap(a, b);
        std::swap(a_occ, b_occ);
        std::swap(a_off, b_off);
    }
    if (a_occ > 1) {
        ULL r = a_occ + a_occ % 2;
        ULL n = r / 2;
        ULL split = n / 2;
        if (a_occ % 2 && a_off) split++;
        bool a_mid_off = (n % 2 != 0) != a_off;
        bool b_mid_off = (n % 2 != 0) != b_off;
        KMUL(result, offset, a, n, a_off, b, b_occ < n ? b_occ : n, b_off);
        KMUL(result + split, a_mid_off != b_off, a + split, a_occ - n, a_mid_off, b, b_occ, b_off);
        if (b_occ > n) {
            KMUL(result + n, offset, a + split, a_occ - n, a_mid_off, b + split, b_occ - n, b_mid_off);
            KMUL(result + split, b_mid_off != a_off, a, a_occ, a_off, b + split, b_occ - n, b_mid_off);
        }
    } else {
        ULL prod = (a_off ? (*a >> 32) : (*a & L_MASK)) * (b_off ? (*b >> 32) : (*b & L_MASK));
        std::cout << prod << std::endl;
        if (offset) {
            if (prod & H_MASK) {
                *(result + 1) += prod >> 32;
            }
            *result += prod << 32;
        } else {
            *result += prod;
        }
    }
}

void ADD_NUM(num a, ULL lambda, int shift, num const b) {
    if (GET_NUM_SIGN(a) != GET_NUM_SIGN(b)) {
        SWITCH_NUM_SIGN(b);
        SUB_NUM(a, lambda, shift, b);
        SWITCH_NUM_SIGN(b);
    } else {
        int n = 0;
        if (GET_NUM_OCC(a) <= GET_NUM_OCC(b) + shift) {
            n = GET_NUM_OCC(b) + shift;
            if ((lambda * GET_NUM_LEADING(b)) & H_MASK) n++;
        } else {
            n = GET_NUM_OCC(a);
        }
        ENLARGE_NUM(a, n);
        SET_NUM_OCC(a, n);
        ULL carry_lambda = 0ULL;
        ULL val = 0ULL;
        ULL d = 0ULL;
        for (int i = 0; i < n; i++) {
            val = GET_NUM_AT(a, i);
            carry_lambda += lambda * GET_NUM_AT(b, i - shift);
            SET_NUM_AT(a, i, val + (carry_lambda & L_MASK) + d);
            d = (val + (carry_lambda & L_MASK) + d) & H_MASK;
            carry_lambda >> 32;
        }
    }
}

/**
 *  subtract lambda * b from a.
 * @param a
 * @param lambda (int)
 * @param shift shift lambda by beta^shift.
 * @param b
 */
void SUB_NUM(num a, ULL lambda, int shift, num const b) {
    if (GET_NUM_SIGN(a) != GET_NUM_SIGN(b)) {
        SWITCH_NUM_SIGN(b);
        ADD_NUM(a, lambda, shift, b);
        SWITCH_NUM_SIGN(b);
    } else {
        int n = 0;
        bool sign = false;
        if (GET_NUM_OCC(a) <= GET_NUM_OCC(b) + shift) {
            n = GET_NUM_OCC(b) + shift;
            if ((lambda * GET_NUM_LEADING(b)) & H_MASK) n++;
        } else {
            n = GET_NUM_OCC(a);
        }
        std::cout << n << std::endl;
        for (; n > 0 && GET_NUM_AT(a, n - 1) == ((lambda * GET_NUM_AT(b, n - 1 - shift)) & L_MASK) + ((lambda * GET_NUM_AT(b, n - 2 - shift)) >> 32); ) {
            std::cout << GET_NUM_AT(a, n - 1) << std::endl;
            REMOVE_WORD(a, --n);
        }
        std::cout << n << std::endl;
        sign = GET_NUM_AT(a, n - 1) > ((lambda * GET_NUM_AT(b, n - 1 - shift)) & L_MASK) + ((lambda * GET_NUM_AT(b, n - 2 - shift)) >> 32) == GET_NUM_SIGN(a);
        ENLARGE_NUM(a, n);
        SET_NUM_OCC(a, n);
        ULL val = 0ULL;
        ULL carry_lambda = 0ULL;
        ULL d = 0ULL;
        if (sign == GET_NUM_SIGN(a)) {
            for (int i = 0; i < n; i++) {
                val = GET_NUM_AT(a, i);
                carry_lambda += lambda * GET_NUM_AT(b, i - shift);
                SET_NUM_AT(a, i, val - (carry_lambda & L_MASK) - d);
                d = val < carry_lambda + d;
                carry_lambda >> 32;
            }
        } else {
            for (int i = 0; i < n; i++) {
                val = GET_NUM_AT(a, i);
                carry_lambda += lambda * GET_NUM_AT(b, i - shift);
                SET_NUM_AT(a, i, - val + (carry_lambda & L_MASK) + d);
                d = val > carry_lambda + d;
                carry_lambda >> 32;
            }
        }
        SET_NUM_SIGN(a, sign);
    }
}
