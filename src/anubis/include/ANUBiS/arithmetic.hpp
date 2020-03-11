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
static const ULL FIFTEEN = LL_MASK & ~(1u << 15);
static const ULL MASKS[] = {
    H_MASK, L_MASK
};

/***********************************************************************************************************************
 * NUM
 **********************************************************************************************************************/

static void STRIP(ULL const* a, int offset, ULL& occ) {
    int j = occ % 2 == 0 ^ offset ? 0 : 1;
    ULL const* i = a + occ / 2 + j;
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
        }
    }
}

static int GET_NUM_POS(num const n) {
    return n->meta & NUM_POS_MASK >> 32;
}

static void SET_NUM_POS(num n, int pos) {
    n->meta = n->meta & ~NUM_POS_MASK | ((ULL) pos << 32) & NUM_POS_MASK;
}

static int GET_NUM_OCC(num const n) {
    return n->meta & NUM_OCC_MASK;
}

static ULL* GET_ABS_DATA(num const n) {
    return (n + 1)->value;
}

static bool GET_NUM_SIGN(num const n) {
    return n->meta & NUM_SIGN_MASK;
}

static int GET_NUM_SIZE(num const n) {
    return n->meta & LL_MASK;
}

static void SWITCH_NUM_SIGN(num n) {
    n->meta ^= NUM_SIGN_MASK;
}

static int COMPARE_ABS(num const n_a, num const n_b) {
    int occ = GET_NUM_OCC(n_a);
    if (occ != GET_NUM_OCC(n_b)) return occ - GET_NUM_OCC(n_b);
    auto a = GET_ABS_DATA(n_a);
    auto b = GET_ABS_DATA(n_b);
    for (int i = 0; i < occ / 2; i++) {
        if (*a != *b) return *a > *b ? 1 : -1;
        a++;
        b++;
    }
    if (occ % 2 != 0) {
        if ((*a & L_MASK) != (*b & L_MASK)) return (*a & L_MASK) - (*b & L_MASK);
    }
    return 0;
}

static void SET_NUM(num a, num const& b) {
    *a = *b;
    *(a + 1) = *(b + 1);
}

struct NUM_COMPARATOR {
    bool operator()(num const& a, num const& b) const {
        if ((a->meta ^ b->meta) & NUM_SIGN_MASK) return GET_NUM_SIGN(a) ? b : a;
        if (COMPARE_ABS(a, b) < 0) {
            return !GET_NUM_SIGN(a);
        } else return GET_NUM_SIGN(a);
    }
};

struct NUM_ABS_COMPARATOR {
    bool operator()(num const& a, num const& b) const {
        return COMPARE_ABS(a, b) < 0;
    }
};

/***********************************************************************************************************************
 * NUM - (DE)ALLOCATION
 **********************************************************************************************************************/

static num NEW_NUM(ULL initial_size, bool sign, ULL value) {
    if (initial_size % 2 != 0) initial_size++;
    num i = new svec_node[2]{
            {.meta = (sign ? NUM_SIGN_MASK : 0u) | ((initial_size & LL_MASK) << 16) | 1u},
            {.value = new ULL[initial_size / 2] { }}
    };
    *((i + 1)->value) = value;
    return i;
}

static num NEW_NUM(ULL* value, int size, int occ, bool sign) {
    return new svec_node[2] {
            {.meta = (sign ? NUM_SIGN_MASK : 0u) | ((ULL) size & LL_MASK << 16) | ((ULL) occ & LL_MASK)},
            {.value = value}
    };
}

static num COPY_NUM(num const n) {
    num res = NEW_NUM(GET_NUM_OCC(n), GET_NUM_SIGN(n), GET_ABS_DATA(n)[0]);
    for (int i = 1; i < GET_NUM_OCC(n); i++) {
        GET_ABS_DATA(res)[i] = GET_ABS_DATA(n)[i];
    }
    return res;
}

static void DEL_NUM_DATA(num i) {
    delete[] (i + 1)->value;
}

static void DEL_NUM(num i) {
    delete (i + 1);
    delete i;
}

/***********************************************************************************************************************
 * NUM - ARITHMETIC
 **********************************************************************************************************************/

/**
 * @param result result memory block. But care; we are assuming that result reserves enough space to save the product!
 * @param a
 * @param b
 * @param a_occ range of occupation of a, i.e. how many digits does a have in base 2^64
 * @param b_occ range of occupation of b, i.e. how many digits does b have in base 2^64
 */
static void KMUL(ULL* result, bool offset, ULL const* a, ULL a_occ, bool a_off, ULL const* b, ULL b_occ, bool b_off) {
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
//        bool mid_off = a_mid_off != b_mid_off;
//        if (offset) mid_off = !mid_off;
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

/**
 * Adds lambda * b to result.
 * @param result
 * @param lambda
 * @param b
 */
static num MUL(const num& lambda, const num& b) {
    if (GET_NUM_OCC(lambda) > GET_NUM_OCC(b)) {
        return MUL(b, lambda);
    }
    int occ = GET_NUM_OCC(lambda) + GET_NUM_OCC(b);
    ULL* result = new ULL[occ / 2 + occ % 2];
    KMUL(result, false, (lambda + 1)->value, GET_NUM_OCC(lambda), false, (b + 1)->value, GET_NUM_OCC(b), false);
    return NEW_NUM(result, 2 * (occ / 2 + occ % 2), occ, GET_NUM_SIGN(lambda) ^ GET_NUM_SIGN(b));
}

static bool ADD_NUM(num& a, num const& b) {
    if (GET_NUM_OCC(b) > GET_NUM_OCC(a)) {

    }
    return true;
}

/**
 * Sets b to be b % a.
 * @param a Divisor
 * @param b Dividend
 */
static void MOD(num a, num const b) {

}

static num DIVIDE(num a, num b) {

}

/***********************************************************************************************************************
 * VECTOR
 **********************************************************************************************************************/

static int GET_VEC_SIZE(s_vec const v) {
    return v->meta >> 32;
}

static int GET_VEC_OCC(s_vec const v) {
    return v->meta & L_MASK;
}

static void SET_VEC_OCC(s_vec v, int occ) {
    v->meta = v->meta & H_MASK | (ULL) occ;
}

static num VEC_AT(s_vec const v, const int i) {
    return v + 1 + 2 * i;
}

static void ENLARGE_VEC(s_vec v, int size) {
    s_vec next = new svec_node[2 * size + 1];
    std::copy(v, VEC_AT(v, GET_VEC_OCC(v)), next);
    SET_NUM(v, next);
}

static void ENLARGE_VEC(s_vec& v) {
    ENLARGE_VEC(v, 2 * GET_VEC_SIZE(v));
}

static void SWITCH_VEC_SIGN(s_vec v) {
    int occ = GET_VEC_OCC(v);
    for (int i = 0; i < occ; i++) {
        SWITCH_NUM_SIGN(VEC_AT(v, i));
    }
}

static void VEC_SWAP_VALUES(s_vec& v, int i, int j) {
    ULL temp = VEC_AT(v, i)->meta;
    VEC_AT(v, i)->meta = VEC_AT(v, j)->meta & ~NUM_POS_MASK | temp & NUM_POS_MASK;
    VEC_AT(v, j)->meta = temp & ~NUM_POS_MASK | VEC_AT(v,j)->meta & NUM_POS_MASK;
    // swap data pointers
    auto a = VEC_AT(v, i) + 1;
    *(VEC_AT(v, i) + 1) = *(VEC_AT(v, j) + 1);
    *(VEC_AT(v, j) + 1) = *a;
}

static int VEC_FIND_POS(s_vec const& v, int pos) {
    int occ = GET_VEC_OCC(v);
    if (occ == 0 || GET_NUM_POS(VEC_AT(v, occ - 1)) < pos) return occ;
    if (GET_NUM_POS(VEC_AT(v, 0)) < pos) return 0;
    int min = 0;
    int max = occ;
    int compare = 0;
    while (min < max) {
        int mid = (min + max)/2;
        if (GET_NUM_POS(VEC_AT(v, mid)) < pos) min = mid + 1;
        else if (GET_NUM_POS(VEC_AT(v, mid)) > pos) max = mid;
        else return mid;
    }
    return min;
}

static void VEC_PUT(s_vec v, num const& n) {
    int k = VEC_FIND_POS(v, GET_NUM_POS(n));
    if (GET_VEC_SIZE(v) >= GET_VEC_OCC(v)) {
        ENLARGE_VEC(v);
    }
    if (k < GET_VEC_OCC(v)) {
        std::copy_backward(VEC_AT(v, k), VEC_AT(v, GET_VEC_OCC(v)), VEC_AT(v, GET_VEC_OCC(v) + 2));
    }
    *VEC_AT(v, k) = *n;
    *(VEC_AT(v, k) + 1) = *(n + 1);
}

/***********************************************************************************************************************
 * VECTOR - (DE)ALLOCATION
 **********************************************************************************************************************/

static s_vec NEW_VEC(std::vector<std::pair<ULL, std::pair<bool, ULL>>> vector) {
    s_vec v = new svec_node[1 + 2 * vector.size()];
    v[0] = {.meta = ((ULL) vector.size()) << 32 | ((ULL) vector.size()) & L_MASK};
    int i = 1;
    for (auto it = vector.begin(); it != vector.end(); it++) {
        num n = NEW_NUM(1u, it->second.first, it->second.second);
        n->meta |= (it->first & FIFTEEN) << 32;
        v[i++] = *n;
        v[i++] = *(n + 1);
    }
    return v;
}

static void DEL_VEC(s_vec& v) {
    ULL occ = v->meta & L_MASK;
    for (int i = 0; i < occ; i++) {
        DEL_NUM(VEC_AT(v, i));
    }
    delete[] v;
}

static void DEL_POS(s_vec& v, int i) {
    DEL_NUM_DATA(VEC_AT(v, i));
    std::copy(VEC_AT(v, i + 1), VEC_AT(v, GET_VEC_OCC(v)), VEC_AT(v, i));
    SET_VEC_OCC(v, GET_VEC_OCC(v) - 1);
}

/***********************************************************************************************************************
 * VECTOR - ARITHMETIC
 **********************************************************************************************************************/

/** Adds lambda * b to a.
 *
 * @param a destination sparse_vector
 * @param lambda coefficient
 * @param size_lambda number of ULLs needed to represent lambda
 * @param b addend
 */
static void ADD(s_vec& a, const num& lambda, const s_vec& b) {
    num a_end = VEC_AT(a, GET_VEC_OCC(a));
    num b_end = VEC_AT(b, GET_VEC_OCC(b));

    num it_a = VEC_AT(a, 0);
    num j = VEC_AT(b, 0);

    for (int i = 0; i != GET_VEC_OCC(a) && j != b_end;) {
        if (GET_NUM_POS(it_a) < GET_NUM_POS(j)) {
            i++;
            it_a += 2;
        } else if (GET_NUM_POS(it_a) > GET_NUM_POS(j)) {
            if (GET_VEC_OCC(a) + 2 > GET_VEC_SIZE(a)) {
                ENLARGE_VEC(a);
                it_a = VEC_AT(a, i);
                a_end = VEC_AT(a, GET_VEC_OCC(a));
            }
            // copy j and j + 1 into a.
            std::copy_backward(it_a, a_end, a_end + 2);
            SET_NUM(it_a, MUL(lambda, j));
            SET_VEC_OCC(a, GET_VEC_OCC(a) + 1);
            j += 2;
        } else {
            ADD_NUM(it_a, MUL(lambda, j));
            it_a += 2;
            i++;
            j += 2;
        }
    }

    if (j != b_end) {
        if (GET_VEC_OCC(a) + (b_end - j) / 2 > GET_VEC_SIZE(a)) {
            ENLARGE_VEC(a, GET_VEC_OCC(a) + (b_end - j) / 2);
            a_end = VEC_AT(a, GET_VEC_OCC(a));
        }
        std::copy(j, b_end, a_end);
    }
}

#endif //ANUBIS_SUPERBUILD_ARITHMETIC_HPP
