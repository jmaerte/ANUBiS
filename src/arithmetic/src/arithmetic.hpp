//
// Created by jmaerte on 16.03.20.
//

#pragma once

#include "arithmetic/operator.hpp"
#include "arithmetic/typedef.hpp"

constexpr int ULL_SIZE = 8 * sizeof(ULL);
// first 32 bits
constexpr ULL L_MASK = (1ULL << (ULL_SIZE / 2)) - 1;
// first 16 bits
constexpr ULL LL_MASK = (1ULL << (ULL_SIZE / 4)) - 1;
// last 32 bits
constexpr ULL H_MASK = L_MASK << (ULL_SIZE / 2);
constexpr ULL NUM_SIGN_MASK = 1ULL << 63;
// last 32 bits without the most significant
constexpr ULL NUM_POS_MASK = H_MASK ^ NUM_SIGN_MASK;
constexpr ULL NUM_SIZE_MASK = L_MASK & ~LL_MASK;
constexpr ULL NUM_OCC_MASK = LL_MASK;
constexpr ULL FIFTEEN = LL_MASK & ~(1u << 15);
constexpr ULL MASKS[] = {
        H_MASK, L_MASK
};
constexpr ULL ULL_MAX = ~0ULL;
constexpr ULL UI_MAX = (~0ULL) >> 32;

/***********************************************************************************************************************
 * AUX
 **********************************************************************************************************************/

void STRIP(ULL const* a, int offset, ULL& occ);
void SET_NUM_AT(num n, int pos, ULL val);
ULL GET_NUM_AT(num n, int pos);
unsigned int GET_NUM_LEADING(num const n);
void REMOVE_WORD(num n, int i);
void KMUL(ULL* result, bool offset, ULL const* a, ULL a_occ, bool a_off, ULL const* b, ULL b_occ, bool b_off);
void ADD_NUM(num a, ULL lambda, int shift, num const b);
void SUB_NUM(num a, ULL lambda, int shift, num const b);

/***********************************************************************************************************************
 * NUM - DECL
 **********************************************************************************************************************/

/*
 * OPERATORS
 */

inline int GET_NUM_POS(num const n) {
    return n->meta & NUM_POS_MASK >> 32;
}

inline int GET_NUM_OCC(num const n) {
    return n->meta & NUM_OCC_MASK;
}

inline int GET_NUM_MEMSIZE(num const n) {
    return GET_NUM_OCC(n) / 2 + (GET_NUM_OCC(n) % 2 ? 1 : 0);
}

inline ULL* GET_ABS_DATA(num const n) {
    return (n + 1)->value;
}

inline bool GET_NUM_SIGN(num const n) {
    return n->meta & NUM_SIGN_MASK;
}

inline int GET_NUM_SIZE(num const n) {
    return n->meta & LL_MASK;
}

inline void SET_NUM_POS(num n, int pos) {
    n->meta = n->meta & ~NUM_POS_MASK | ((ULL) pos << 32) & NUM_POS_MASK;
}

inline void SET_NUM_SIZE(num n, int size) {
    n->meta = n->meta & ~NUM_SIZE_MASK | ((size & LL_MASK) << 16);
}

inline void SET_NUM_OCC(num n, int occ) {
    n->meta = n->meta & ~NUM_OCC_MASK | (occ & LL_MASK);
}

inline void SET_NUM_SIGN(num n, bool sign) {
    n->meta = n->meta & ~NUM_SIGN_MASK | ((sign ? 1ULL : 0ULL) << 63);
}

inline void SWITCH_NUM_SIGN(num n) {
    n->meta ^= NUM_SIGN_MASK;
}

inline void SET_NUM(num a, num const& b) {
    *a = *b;
    *(a + 1) = *(b + 1);
}

inline void ENLARGE_NUM(num& n, int size) {
    if (size <= GET_NUM_SIZE(n)) return;
    if (size % 2 != 0) size++;
    ULL* next = new ULL[size / 2];
    std::copy(GET_ABS_DATA(n), GET_ABS_DATA(n) + GET_NUM_OCC(n), next);
    delete[] GET_ABS_DATA(n);
    (n + 1)->value = next;
    SET_NUM_SIZE(n, size);
}

inline void ENLARGE_NUM(num& n) {
    ENLARGE_NUM(n, 2 * GET_NUM_SIZE(n));
}

num PREPARE_SMOD_DIVISOR(num divisor, num denominators, int arr_size) {
    int n = -1;
    for (int i = 0; i < arr_size; i++) {
        if (n < 0 || n > GET_NUM_OCC(denominators)) {
            n = GET_NUM_OCC(denominators);
        }
        denominators += 2;
    }
    n = n / 2 + n % 2;

    // TODO: modify divisor accordingly.

    return nullptr;
}

num PREPARE_NUM_SVOBODA(num const n) {
    ULL b = GET_NUM_LEADING(n);
    num k = NEW_NUM(2, false, UI_MAX / b + (UI_MAX % b != 0));
    *GET_ABS_DATA(k) = *GET_ABS_DATA(k) << 32;
    return MUL(k, n);
}

/*
 * COMPARATORS
 */

int COMPARE_ABS(num const n_a, num const n_b) {
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

bool NUM_COMPARATOR::operator()(num const& a, num const& b) const {
    if ((a->meta ^ b->meta) & NUM_SIGN_MASK) return GET_NUM_SIGN(a) ? b : a;
    if (COMPARE_ABS(a, b) < 0) {
        return !GET_NUM_SIGN(a);
    } else return GET_NUM_SIGN(a);
}

bool NUM_ABS_COMPARATOR::operator()(num const& a, num const& b) const {
    return COMPARE_ABS(a, b) < 0;
}

/*
 * (DE-)ALLOCATION
 */

inline num NEW_NUM(ULL initial_size, bool sign, ULL value) {
    if (initial_size % 2 != 0) initial_size++;
    num i = new svec_node[2]{
            {.meta = (sign ? NUM_SIGN_MASK : 0ULL) | ((initial_size & LL_MASK) << 16) | (value ? (value & H_MASK ? 2ULL : 1ULL): 0ULL)},
            {.value = new ULL[initial_size / 2] { }}
    };
    *((i + 1)->value) = value;
    return i;
}

inline num NEW_NUM(ULL* value, int size, int occ, bool sign) {
    return new svec_node[2] {
            {.meta = (sign ? NUM_SIGN_MASK : 0u) | ((ULL) size & LL_MASK << 16) | ((ULL) occ & LL_MASK)},
            {.value = value}
    };
}

inline num COPY_NUM(num const n) {
    num res = NEW_NUM(GET_NUM_OCC(n), GET_NUM_SIGN(n), GET_ABS_DATA(n)[0]);
    for (int i = 1; i < GET_NUM_OCC(n); i++) {
        GET_ABS_DATA(res)[i] = GET_ABS_DATA(n)[i];
    }
    return res;
}

inline void DEL_NUM_DATA(num i) {
    delete[] (i + 1)->value;
}

inline void DEL_NUM(num i) {
    delete (i + 1);
    delete i;
}

/*
 * ARITHMETIC
 */

num MUL(const num& lambda, const num& b) {
    if (GET_NUM_OCC(lambda) > GET_NUM_OCC(b)) {
        return MUL(b, lambda);
    }
    int occ = GET_NUM_OCC(lambda) + GET_NUM_OCC(b);
    ULL* result = new ULL[occ / 2 + occ % 2];
    KMUL(result, false, (lambda + 1)->value, GET_NUM_OCC(lambda), false, (b + 1)->value, GET_NUM_OCC(b), false);
    return NEW_NUM(result, 2 * (occ / 2 + occ % 2), occ, GET_NUM_SIGN(lambda) ^ GET_NUM_SIGN(b));
}

bool ADD_NUM(num& a, num const& b) {
    if (GET_NUM_OCC(b) > GET_NUM_OCC(a)) {

    }
    return true;
}

void SMOD(num a, num const b, num const pre) {

}

num SDIV(num const a, num const b, num const pre) {
    int m = GET_NUM_OCC(a) - GET_NUM_OCC(b);
    int n = GET_NUM_OCC(b);
    num Q = NEW_NUM(m, GET_NUM_SIGN(a) ^ GET_NUM_SIGN(b), 0ULL);
    for (int j = m - 1; j > 0; j--) {
        SET_NUM_AT(Q, j, GET_NUM_AT(a, n + j));
        SUB_NUM(a, GET_NUM_AT(Q, j), j - 1, pre);
        if (GET_NUM_SIGN(a)) {
            SET_NUM_AT(Q, j, GET_NUM_AT(Q, j) - 1);
            ADD_NUM(a, 1ULL, j - 1, pre);
        }
    }
    SET_NUM_AT(Q, 0, (GET_NUM_AT(a, GET_NUM_OCC(a) - 1) << 32 | GET_NUM_AT(a, GET_NUM_OCC(a) - 2)) / GET_NUM_LEADING(b));
    SUB_NUM(a, GET_NUM_AT(Q, 0), 0, b);
    if (GET_NUM_SIGN(a)) {
        SET_NUM_AT(Q, 0, GET_NUM_AT(Q, 0) - 1);
        ADD_NUM(a, 1ULL, 0, pre);
    }
    return Q;
}

/***********************************************************************************************************************
 * VEC - AUX
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * VEC - DECL
 **********************************************************************************************************************/

/*
 * OPERATORS
 */

inline int GET_VEC_SIZE(s_vec const v) {
    return v->meta >> 32;
}

inline int GET_VEC_OCC(s_vec const v) {
    return v->meta & L_MASK;
}

inline void SET_VEC_OCC(s_vec v, int occ) {
    v->meta = v->meta & H_MASK | (ULL) occ;
}

inline num VEC_AT(s_vec const v, const int i) {
    return v + 1 + 2 * i;
}

inline void ENLARGE_VEC(s_vec& v, int size) {
    s_vec next = new svec_node[2 * size + 1];
    std::copy(v, VEC_AT(v, GET_VEC_OCC(v)), next);
    delete[] v;
    v = next;
}

inline void ENLARGE_VEC(s_vec& v) {
    ENLARGE_VEC(v, 2 * GET_VEC_SIZE(v));
}

inline void SWITCH_VEC_SIGN(s_vec v) {
    int occ = GET_VEC_OCC(v);
    for (int i = 0; i < occ; i++) {
        SWITCH_NUM_SIGN(VEC_AT(v, i));
    }
}

inline void VEC_SWAP_VALUES(s_vec& v, int i, int j) {
    ULL temp = VEC_AT(v, i)->meta;
    VEC_AT(v, i)->meta = VEC_AT(v, j)->meta & ~NUM_POS_MASK | temp & NUM_POS_MASK;
    VEC_AT(v, j)->meta = temp & ~NUM_POS_MASK | VEC_AT(v,j)->meta & NUM_POS_MASK;
    // swap data pointers
    auto a = VEC_AT(v, i) + 1;
    *(VEC_AT(v, i) + 1) = *(VEC_AT(v, j) + 1);
    *(VEC_AT(v, j) + 1) = *a;
}

int VEC_FIND_POS(s_vec const& v, int pos) {
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

inline void VEC_SET(s_vec v, int k, num const& n) {
    SET_NUM(VEC_AT(v, k), n);
}

void VEC_PUT(s_vec v, num const& n) {
    int k = VEC_FIND_POS(v, GET_NUM_POS(n));
    if (GET_VEC_SIZE(v) >= GET_VEC_OCC(v)) {
        ENLARGE_VEC(v);
    }
    if (k < GET_VEC_OCC(v)) {
        std::copy_backward(VEC_AT(v, k), VEC_AT(v, GET_VEC_OCC(v)), VEC_AT(v, GET_VEC_OCC(v) + 2));
    }
    VEC_SET(v, k, n);
}

/*
 * (DE-)ALLOCATION
 */

inline s_vec NEW_VEC(std::vector<std::pair<ULL, std::pair<bool, ULL>>> vector) {
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

inline void DEL_VEC(s_vec& v) {
    ULL occ = v->meta & L_MASK;
    for (int i = 0; i < occ; i++) {
        DEL_NUM(VEC_AT(v, i));
    }
    delete[] v;
}

inline void DEL_POS(s_vec& v, int i) {
    DEL_NUM_DATA(VEC_AT(v, i));
    std::copy(VEC_AT(v, i + 1), VEC_AT(v, GET_VEC_OCC(v)), VEC_AT(v, i));
    SET_VEC_OCC(v, GET_VEC_OCC(v) - 1);
}

/*
 * ARITHMETIC
 */

void ADD(s_vec& a, int start_a, const num& lambda, const s_vec& b, int start_b) {
    num a_end = VEC_AT(a, GET_VEC_OCC(a));
    num b_end = VEC_AT(b, GET_VEC_OCC(b));

    num it_a = VEC_AT(a, start_a);
    num j = VEC_AT(b, start_b);

    for (int i = start_a; i != GET_VEC_OCC(a) && j != b_end;) {
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
            num x = MUL(lambda, j);
            ADD_NUM(it_a, x);
            DEL_NUM(x);
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

void MUL_VEC(s_vec v, num const n) {
    for (int i = 0; i < GET_VEC_OCC(v); i++) {
        num res = MUL(VEC_AT(v, i), n);
        DEL_NUM_DATA(VEC_AT(v, i));
        VEC_SET(v, i, n);
    }
}