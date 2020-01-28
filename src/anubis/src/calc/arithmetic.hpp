//
// Created by jmaerte on 28.01.2020.
//

#ifndef ANUBIS_SUPERBUILD_ARITHMETIC_HPP
#define ANUBIS_SUPERBUILD_ARITHMETIC_HPP

typedef unsigned long long ULL;

static const ULL THRSH_KARATSUBA = 50;
static const ULL THRSH_TOOM_CROOK = 75;

/**
 * This union has 64 bit.
 * In svec we have one leading ULL for metadata:
 *      +--32--+--32--+
 *      | size |  occ |
 *      +------+------+
 * After that we have a pair of
 */
union svec_node {
    ULL meta;
    ULL * value;
};

// A number is a pair of svec_nodes. One is a meta node and the other points to the actual number.
typedef svec_node* num;

// A s_vec is an array of svec_nodes. It is always of odd length; one is a meta node and all the others are pairs, i.e. nums.
typedef svec_node* s_vec;

static const int ULL_SIZE = 8 * sizeof(ULL);
// first 32 bits
static const ULL L_MASK = (1ULL << (ULL_SIZE / 2)) - 1;
// first 16 bits
static const ULL LL_MASK = (1ULL << (ULL_SIZE / 4)) - 1;
// last 32 bits
static const ULL H_MASK = L_MASK << (ULL_SIZE / 2);
static const ULL NUM_SIGN_MASK = 1ULL << 63;
// last 32 bits without the most significant
static const ULL NUM_POS_MASK = H_MASK ^ NUM_SIGN_MASK;
static const ULL NUM_OCC_MASK = LL_MASK;

static ULL HIGH(const ULL& a) {
    return a >> (ULL_SIZE / 2);
}

static ULL LOW(const ULL& a) {
    return a & L_MASK;
}

static void STRIP(ULL const* a, ULL& occ) {
    if (occ % 2 != 0) occ++;
    for (ULL const* i = a + occ / 2 - 1; i != a; i--) {
        if ((*i & H_MASK) != 0) {
            return;
        }
        occ--;
        if ((*i & L_MASK) != 0) {
            return;
        }
        occ--;
    }
    occ = 0;
}

/**
 *
 * @param result result memory block
 * @param a
 * @param b
 * @param a_occ range of occupation of a, i.e. how many digits does a have in base 2^64
 * @param b_occ range of occupation of b, i.e. how many digits does b have in base 2^64
 */
static void KMUL(ULL* result, bool offset, ULL const* a, ULL const* b, ULL a_occ, bool a_off, ULL b_occ, bool b_off) {
    STRIP(a, a_occ);
    STRIP(b, b_occ);
    if (a_occ == 0 || b_occ == 0) return;
    if (a_occ < b_occ) {
        std::swap(a, b);
        std::swap(a_occ, b_occ);
    }
    if (a_occ > 1) {
        ULL n = a_occ + a_occ % 2;
        ULL split = n / 2;
        bool a_mid_off = (n % 2 != 0) != a_off;
        bool b_mid_off = (n % 2 != 0) != b_off;
        bool mid_off = a_off != b_off;
        if (offset) mid_off = !mid_off;
        KMUL(result, offset, a, b, n, a_off, n, b_off);
        KMUL(result + split, mid_off, a + split, b, a_occ - n, a_mid_off, b_occ, b_off);
        if (b_occ > n) {
            KMUL(result + split, mid_off, a, b + split, a_occ, a_off, b_occ - n, b_off);
            KMUL(result + n, offset, a + split, b + split, a_occ - n, a_mid_off, b_occ - n, b_mid_off);
        }
    } else {
        ULL prod = (a_off ? (*a >> 32) : (*a & L_MASK)) * (b_off ? (*b >> 32) : (*b & L_MASK));
        if (offset) {
            if (prod & H_MASK) {
                *(result + 1)+= prod >> 32;
            }
            *result += prod << 32;
        } else {
            *result += prod;
        }
    }
}

static void TCMUL(num& result, const num& a, const num& b) {

}

static void MUL(num& result, const num& lambda, const num& b) {
    // set result to lambda * b
    ULL lambda_length = lambda->meta & NUM_OCC_MASK >> 32;
    ULL b_length = b->meta & NUM_OCC_MASK >> 32;
    if (lambda_length > b_length) {
        MUL(result, b, lambda);
        return;
    }
    if (lambda_length < THRSH_KARATSUBA || b_length < THRSH_KARATSUBA) {
        // perform standard multiplication
        bool lambda_sign = (bool) (lambda->meta & NUM_SIGN_MASK);
        bool b_sign = (bool) (b->meta & NUM_SIGN_MASK);
        result = new svec_node {0};
        if (lambda_sign != b_sign) {
            result->meta |= (1ULL << 63);
        }
        result->meta |= (b->meta & NUM_POS_MASK);
        ULL LEN = lambda_length + b_length;
        ULL* prod = new ULL[LEN / 2 + LEN % 2](); // round up N/2.
        ULL* j = (b + 1)->value;
        ULL* l = (lambda + 1)->value;
        ULL* k = prod;
        ULL carry = 0ULL;
//        for (; ; ) {
//            carry =
//        }
    } else {
        if (lambda_length < THRSH_TOOM_CROOK && b_length < THRSH_TOOM_CROOK) {
            KMUL((result + 1)->value, false, (lambda + 1)->value, (b + 1)->value, lambda_length, false, b_length, false);
        } else {
            TCMUL(result, lambda, b);
        }
    }
    ULL occ = lambda_length + b_length;
    STRIP((result + 1)->value, occ);
}

static bool ADD_NUM(num& a, const num& lambda, const num& b) {
    ULL a_meta = a->meta;
    ULL a_size = (a_meta & L_MASK) >> 16;
    ULL a_occupation = (a_meta & LL_MASK);
    ULL* a_start = (a + 1)->value;
    ULL* a_end = a_start + a_occupation;
    bool a_negative = (bool) (a_meta & NUM_SIGN_MASK);

    ULL b_meta = b->meta;
    ULL b_size = (b_meta & L_MASK) >> 16;
    ULL b_occupation = (b_meta & LL_MASK);
    ULL* b_start = (b + 1)->value;
    ULL* b_end = b_start + b_occupation;
    bool b_negative = (bool) (b_meta & NUM_SIGN_MASK);

    // return if a is zero now.

}

/** Adds lambda * b to a.
 *
 * @param a destination sparse_vector
 * @param lambda coefficient
 * @param size_lambda number of ULLs needed to represent lambda
 * @param b addend
 */
static void ADD(s_vec& a, const num& lambda, const s_vec& b) {
    ULL a_meta = a->meta;
    ULL a_size = (a_meta >> 32) - 1ULL;
    ULL a_occupation = a_meta & L_MASK;
    svec_node* a_start = (a + 1);
    svec_node* a_end = a_start + a_occupation;

    ULL b_meta = b->meta;
    ULL b_size = (b_meta >> 32) - 1ULL;
    ULL b_occupation = b_meta & L_MASK;
    svec_node* b_start = (b + 1);
    svec_node* b_end = b_start + b_occupation;

    svec_node* i = a_start;
    svec_node* j = b_start;

    ULL a_pos, b_pos;
    for (; i != a_end && j != b_end; ) {
        if ((a_pos = i->meta & NUM_POS_MASK) < (b_pos = j->meta & NUM_POS_MASK)) {
            i += 2;
        } else if (a_pos > b_pos) {
            if (a_occupation + 2 > a_size) {
                s_vec* next = new svec_node[a_size * 2 + 1];
                std::copy(a, a_end, next);
                a = next;
                a_size *= 2;
            }
            // copy j and j + 1 into a.
            std::copy_backward(i, a_end, a_end + 2);
            // new number = lambda * (*j)
            KMUL(i, lambda, j);
            a_end += 2;
            a_occupation += 2;
            i += 2;
            j += 2;
        } else {
            if(ADD_NUM(i, lambda, j)) {
                std::copy(i + 2, a_end, i);
                a_end -= 2;
            } else i += 2;
            j += 2;
        }
    }

    if (j != b_end) {
        if (a_occupation + b_end - j > a_size) {
            s_vec* next = new svec_node[a_size * 2 + 1];
            std::copy(a, a_end, next);
            a = next;
            a_size *= 2;
        }
        std::copy(j, b_end, a_end);
    }
    a->meta = (a_size << 32) | a_occupation;
}

#endif //ANUBIS_SUPERBUILD_ARITHMETIC_HPP
