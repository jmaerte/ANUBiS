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
constexpr ULL BASE = 1ULL << 32;

// AUX MIN and MAX functions without branching
inline int MIN(int x, int y) {
    return y ^ ((x ^ y) & -(x < y));
}

inline int MAX(int x, int y) {
    return x ^ ((x ^ y) & -(x < y));
}

namespace jmaerte {
    namespace arith {
/***********************************************************************************************************************
 * AUX
 **********************************************************************************************************************/

        namespace num {
            namespace aux {

                void            STRIP(ULL const* a, ULL& occ);
                void            SET(ap_int n, int pos, ULL val);
                ULL             GET(ap_int n, int pos);
                unsigned int    GET_LEADING(ap_int const n);
                void            REMOVE(ap_int n, int i);
                void            ADD(ap_int a, ULL lambda, int shift, ap_int const b);
                void            SUB(ap_int a, ULL lambda, int shift, ap_int const b);
                void            KMUL(ULL* result, bool offset, ULL const* a, ULL a_occ, bool a_off, ULL const* b, ULL b_occ, bool b_off);
                void            CMUL();

                ap_int iSUM_DATA(const ULL* a, int l_a, const ULL* b, int l_b);
                void SUM_DATA(ULL* a, const ULL* b, int l);
                void REC_MUL_DATA(ULL* a, int l_a, const ULL* b, int l_b);
                void iREC_MUL_DATA(ULL* result, ULL* a, int l_a, const ULL* b, int l_b);

                namespace modular {
                    void SREM(ap_int a, ap_int N, ULL N_inv);
                    void MODMUL(ap_int& a, ap_int b, ap_int N, ULL N_inv);
                }
            }
        }

/***********************************************************************************************************************
 * NUM - DECL
 **********************************************************************************************************************/
        namespace num {
            /*
             * OPERATORS
             */

            inline int GET_POS(ap_int const n) {
                return n->meta & NUM_POS_MASK >> 32;
            }

            inline int GET_OCC(ap_int const n) {
                return n->meta & NUM_OCC_MASK;
            }

            inline ULL *GET_ABS_DATA(ap_int const n) {
                return (n + 1)->value;
            }

            inline bool GET_SIGN(ap_int const n) {
                return n->meta & NUM_SIGN_MASK;
            }

            inline int GET_SIZE(ap_int const n) {
                return n->meta & LL_MASK;
            }

            inline void SET_POS(ap_int n, int pos) {
                n->meta = n->meta & ~NUM_POS_MASK | ((ULL) pos << 32) & NUM_POS_MASK;
            }

            inline void SET_SIZE(ap_int n, int size) {
                n->meta = n->meta & ~NUM_SIZE_MASK | ((size & LL_MASK) << 16);
            }

            inline void SET_OCC(ap_int n, int occ) {
                n->meta = n->meta & ~NUM_OCC_MASK | (occ & LL_MASK);
            }

            inline void SET_SIGN(ap_int n, bool sign) {
                n->meta = n->meta & ~NUM_SIGN_MASK | ((sign ? 1ULL : 0ULL) << 63);
            }

            inline void SWITCH_SIGN(ap_int n) {
                n->meta ^= NUM_SIGN_MASK;
            }

            inline void ASSIGN(ap_int a, ap_int const &b) {
                *a = *b;
                *(a + 1) = *(b + 1);
            }

            inline void OVERWRITE(ap_int a, ap_int const &b) {
                DELETE_DATA(a);
                *a = *b;
                *(a + 1) = *(b + 1);
            }

            inline void ENLARGE(ap_int &n, int size) {
                if (size <= GET_SIZE(n)) return;
                ULL *next = new ULL[size] { };
                std::copy(GET_ABS_DATA(n), GET_ABS_DATA(n) + GET_OCC(n), next);
                delete[] GET_ABS_DATA(n);
                (n + 1)->value = next;
                SET_SIZE(n, size);
            }

            inline void ENLARGE(ap_int &n) {
                ENLARGE(n, 2 * GET_SIZE(n));
            }


            void RSHIFT(ap_int &n, int shift) {
                int start = shift / 64;
                shift %= 64;
                int next_shift = 64 - shift;
                ULL *arr = GET_ABS_DATA(n);
                int end = GET_OCC(n);
                for (int i = 0; i < end - start - 1; i++) {
                    *(arr + i) = (*(arr + i + start) >> shift) | (*(arr + i + start + 1) << next_shift);
                }
                *(arr + end - start - 1) = *(arr + end - 1) >> shift;
                SET_OCC(n, GET_OCC(n) - start - (*(arr + end - start - 1) == 0ULL));
            }

            void LSHIFT(ap_int &n, int shift) {
                int start = shift / 64;
                shift %= 64;
                next_shift = 64 - shift;
                ENLARGE(n, GET_OCC(n) + start + 1);
                ULL *arr = GET_ABS_DATA(n);
                int end = GET_OCC(n);
                for (int i = end - 1; i > 0; i--) {
                    *(arr + start + i) = (*(arr + i) << shift) | (*(arr + i - 1) >> next_shift);
                    if (start != 0) *(arr + i) = 0ULL;
                }
                *(arr + start) = *arr << shift;
                if (start != 0) *arr = 0ULL;
                SET_OCC(n, GET_OCC(n) + start + (*(arr + end + start) != 0ULL));
            }

            // DEPRECATED:
//            void LSHIFT_BLOCK(ap_int &n, int blocks) {
//                if (blocks % 2) {
//                    ENLARGE(n, GET_OCC(n) + blocks);
//                    int start = blocks / 2;
//                    ULL *arr = GET_ABS_DATA(n);
//                    int end = GET_OCC(n) / 2 + GET_OCC(n) % 2;
//                    if (GET_OCC(n) % 2 == 0) {
//                        *(arr + start + end) = *(arr + end) >> 32;
//                    }
//                    for (int i = end - 1; i > 0; i--) {
//                        *(arr + start + i) = (*(arr + i) << 32) | (*(arr + i - 1) >> 32);
//                        if (i < start) *(arr + i) = 0ULL;
//                    }
//                    *(arr + start) = *arr << 32;
//                    if (start > 0) *arr = 0ULL;
//                    SET_OCC(n, GET_OCC(n) + blocks);
//                } else LSHIFT_ULL(n, blocks / 2);
//            }

            void LSHIFT_BLOCK(ap_int &n, int blocks) {
                ENLARGE(n, GET_OCC(n) + ulls);
                ULL *arr = GET_ABS_DATA(n);
                int size = GET_OCC(n);
                std::copy_backward(arr, arr + size, arr + size + ulls);
                for (int i = 0; i < ulls; i++) *(arr + i) = 0ULL;
                SET_OCC(n, GET_OCC(n) + ulls);
            }

            /*
             * COMPARATORS
             */

            int COMPARE_ABS(ap_int const n_a, ap_int const n_b) {
                int occ = GET_OCC(n_a);
                if (occ != GET_OCC(n_b)) return occ - GET_OCC(n_b);
                auto a = GET_ABS_DATA(n_a);
                auto b = GET_ABS_DATA(n_b);
                for (int i = 0; i < occ; i++) {
                    if (*a != *b) return *a++ > *b++ ? 1 : -1;
                }
                return 0;
            }

            bool comp::SIGNED_COMPARATOR::operator()(ap_int const &a, ap_int const &b) const {
                if ((a->meta ^ b->meta) & NUM_SIGN_MASK) return GET_SIGN(a) ? b : a;
                return !((COMPARE_ABS(a, b) < 0) ^ GET_SIGN(a));
            }

            bool comp::UNSIGNED_COMPARATOR::operator()(ap_int const &a, ap_int const &b) const {
                return COMPARE_ABS(a, b) < 0;
            }

            /*
             * (DE-)ALLOCATION
             */

            inline ap_int NEW(ULL initial_size, bool sign, ULL value) {
                ap_int i = new svec_node[2]{
                        {.meta = (sign ? NUM_SIGN_MASK : 0ULL) | ((initial_size & LL_MASK) << 16) |
                                 (value ? 1ULL : 0ULL)},
                        {.value = new ULL[initial_size]{ }}
                };
                *((i + 1)->value) = value;
                return i;
            }

            inline ap_int NEW(ULL* value, ULL size, ULL occ, bool sign) {
                return new svec_node[2]{
                        {.meta = (sign ? NUM_SIGN_MASK : 0u) | (size & LL_MASK << 16) | (occ & LL_MASK)},
                        {.value = value}
                };
            }

            inline ap_int COPY(ap_int const n) {
                ap_int res = NEW(GET_OCC(n), GET_SIGN(n), 0ULL);
                std::copy(GET_ABS_DATA(n), GET_ABS_DATA(n) + GET_OCC(n), GET_ABS_DATA(res));
                return res;
            }

            inline void DELETE_DATA(ap_int i) {
                delete[] (i + 1)->value;
            }

            inline void DELETE(ap_int i) {
                DELETE_DATA(i);
                delete (i + 1);
                delete i;
            }

            inline ap_int operator ""ap_int(ULL n) {
                return NEW(1, false, n);
            }

            /*
             * ARITHMETIC
             */

            ap_int iMUL(const ap_int &lambda, const ap_int &b) {
                if (GET_OCC(lambda) > GET_OCC(b)) {
                    return iMUL(b, lambda);
                }
                int occ = GET_OCC(lambda) + GET_OCC(b);
                ULL *result = new ULL[occ];
                aux::iREC_MUL_DATA(result, (lambda + 1)->value, GET_OCC(lambda), (b + 1)->value, GET_OCC(b));
                return NEW(result, occ, occ, GET_SIGN(lambda) ^ GET_SIGN(b));
            }

            void MUL(ap_int a, ap_int b) {
                int n_a = GET_OCC(a);
                int n_b = GET_OCC(b);

                ENLARGE(a, n_a + n_b);

                ULL *dat_a = GET_ABS_DATA(a);
                ULL *dat_b = GET_ABS_DATA(b);

                n_a += aux::REC_MUL_DATA(dat_a, n_a, dat_b, n_b, 0);

                SET_SIGN(a, GET_SIGN(a) ^ GET_SIGN(b));
                SET_OCC(a, n_a + n_b);
            }

            ap_int iMULL(ap_int a, ap_int b, int n) {
                if (GET_OCC(a) > GET_OCC(b)) {
                    return iMUL(b, a);
                }
                ULL *result = new ULL[n];
                aux::iREC_MULL_DATA(result, (lambda + 1)->value, GET_OCC(lambda), (b + 1)->value, GET_OCC(b), n);
                return NEW(result, n, n, GET_SIGN(a) ^ GET_SIGN(b));
            }

            void MULL(ap_int a, ap_int b, int n) {
                int n_a = GET_OCC(a);
                int n_b = GET_OCC(b);

                ENLARGE(a, n);

                ULL *dat_a = GET_ABS_DATA(a);
                ULL *dat_b = GET_ABS_DATA(b);

                aux::REC_MULL_DATA(dat_a, s_a, dat_b, s_b, n);

                SET_SIGN(a, GET_SIGN(a) ^ GET_SIGN(b));
                SET_OCC(a, n);
            }

            inline void ADD(ap_int &a, ap_int const &b) {
                if (GET_SIGN(a) != GET_SIGN(b)) SUB(a, b);
                else {
                    ENLARGE(a, MAX(GET_OCC(a), GET_OCC(b)) + 1);
                    SET_OCC(a, GET_OCC(a) + aux::ADD_DATA(GET_ABS_DATA(a), GET_ABS_DATA(b), GET_OCC(a), GET_OCC(b)));
                }
            }

            inline void SUB(ap_int &a, ap_int const &b) {
                if (GET_SIGN(a) != GET_SIGN(b)) ADD(a, b);
                else {
                    int size = MAX(GET_OCC(a), GET_OCC(b)) + 1;
                    ENLARGE(a, size); // this sets the last place to 0.
                    SET_SIGN(a, aux::SUB_DATA(GET_ABS_DATA(a), GET_ABS_DATA(b), GET_OCC(a), GET_OCC(b)));
                    aux::STRIP(GET_ABS_DATA(a), size);
                    SET_OCC(a, size);
                }
            }

            inline void MOD(ap_int a, ap_int b, ap_int R_POW, ULL inv) {
                aux::modular::SREM(a, b, inv);
                if (GET_OCC(a)) aux::modular::MODMUL(a, R_POW);
            }

            ap_int SDIV(ap_int const a, ap_int const b, ap_int const pre) {
                int m = GET_OCC(a) - GET_OCC(b);
                int n = GET_OCC(b);
                ap_int Q = NEW(m, GET_SIGN(a) ^ GET_SIGN(b), 0ULL);
                for (int j = m - 1; j > 0; j--) {
                    aux::SET(Q, j, aux::GET(a, n + j));
                    aux::SUB(a, aux::GET(Q, j), j - 1, pre);
                    if (GET_SIGN(a)) {
                        aux::SET(Q, j, aux::GET(Q, j) - 1ULL);
                        aux::ADD(a, 1ULL, j - 1, pre);
                    }
                }
                // TODO: REFACTOR THIS
                aux::SET(Q, 0, aux::GET(a, GET_OCC(a) - 1) / aux::GET_LEADING(b));
                aux::SUB(a, aux::GET(Q, 0), 0, b);
                if (GET_SIGN(a)) {
                    aux::SET(Q, 0, aux::GET(Q, 0) - 1);
                    aux::ADD(a, 1ULL, 0, pre);
                }
                return Q;
            }
        }

/***********************************************************************************************************************
 * VEC - AUX
 **********************************************************************************************************************/

        namespace vec {}

/***********************************************************************************************************************
 * VEC - DECL
 **********************************************************************************************************************/

        namespace vec {
            /*
             * OPERATORS
             */

            inline int GET_SIZE(s_vec const v) {
                return v->meta >> 32;
            }

            inline int GET_OCC(s_vec const v) {
                return v->meta & L_MASK;
            }

            inline void SET_OCC(s_vec v, int occ) {
                v->meta = v->meta & H_MASK | (ULL) occ;
            }

            inline num::ap_int AT(s_vec const v, const int i) {
                return v + 1 + 2 * i;
            }

            inline void ENLARGE(s_vec& v, int size) {
                s_vec next = new svec_node[2 * size + 1];
                std::copy(v, AT(v, GET_OCC(v)), next);
                delete[] v;
                v = next;
            }

            inline void ENLARGE(s_vec& v) {
                ENLARGE(v, 2 * GET_SIZE(v));
            }

            inline void SWITCH_SIGNS(s_vec v) {
                int occ = GET_OCC(v);
                for (int i = 0; i < occ; i++) {
                    num::SWITCH_SIGN(AT(v, i));
                }
            }

            inline void SWAP_VALUES(s_vec& v, int i, int j) {
                ULL temp = AT(v, i)->meta;
                AT(v, i)->meta = AT(v, j)->meta & ~NUM_POS_MASK | temp & NUM_POS_MASK;
                AT(v, j)->meta = temp & ~NUM_POS_MASK | AT(v,j)->meta & NUM_POS_MASK;
                // swap data pointers
                auto a = AT(v, i) + 1;
                *(AT(v, i) + 1) = *(AT(v, j) + 1);
                *(AT(v, j) + 1) = *a;
            }

            int FIND_POS(s_vec const& v, int pos) {
                int occ = GET_OCC(v);
                if (occ == 0 || num::GET_POS(AT(v, occ - 1)) < pos) return occ;
                if (num::GET_POS(AT(v, 0)) < pos) return 0;
                int min = 0;
                int max = occ;
                int compare = 0;
                while (min < max) {
                    int mid = (min + max)/2;
                    if (num::GET_POS(AT(v, mid)) < pos) min = mid + 1;
                    else if (num::GET_POS(AT(v, mid)) > pos) max = mid;
                    else return mid;
                }
                return min;
            }

            inline void SET(s_vec v, int k, num::ap_int const& n) {
                num::ASSIGN(AT(v, k), n);
            }

            void PUT(s_vec v, num::ap_int const& n) {
                int k = FIND_POS(v, num::GET_POS(n));
                if (GET_SIZE(v) <= GET_OCC(v)) {
                    ENLARGE(v);
                }
                if (k < GET_OCC(v)) {
                    std::copy_backward(AT(v, k), AT(v, GET_OCC(v)), AT(v, GET_OCC(v) + 2));
                }
                SET(v, k, n);
            }

            /*
             * (DE-)ALLOCATION
             */

            inline s_vec NEW(std::vector<std::pair<ULL, std::pair<bool, ULL>>> vector) {
                s_vec v = new svec_node[1 + 2 * vector.size()];
                v[0] = {.meta = ((ULL) vector.size()) << 32 | ((ULL) vector.size()) & L_MASK};
                int i = 1;
                for (auto it = vector.begin(); it != vector.end(); it++) {
                    num::ap_int n = num::NEW(1u, it->second.first, it->second.second);
                    n->meta |= (it->first & FIFTEEN) << 32;
                    v[i++] = *n;
                    v[i++] = *(n + 1);
                }
                return v;
            }

            inline void DELETE(s_vec& v) {
                ULL occ = v->meta & L_MASK;
                for (int i = 0; i < occ; i++) {
                    num::DELETE(AT(v, i));
                }
                delete[] v;
            }

            inline void DELETE_POS(s_vec& v, int i) {
                num::DELETE_DATA(AT(v, i));
                std::copy(AT(v, i + 1), AT(v, GET_OCC(v)), AT(v, i));
                SET_OCC(v, GET_OCC(v) - 1);
            }

            /*
             * ARITHMETIC
             */

            void ADD(s_vec& a, int start_a, const num::ap_int& lambda, const s_vec& b, int start_b) {
                num::ap_int a_end = AT(a, GET_OCC(a));
                num::ap_int b_end = AT(b, GET_OCC(b));

                num::ap_int it_a = AT(a, start_a);
                num::ap_int j = AT(b, start_b);

                for (int i = start_a; i != GET_OCC(a) && j != b_end;) {
                    if (num::GET_POS(it_a) < num::GET_POS(j)) {
                        i++;
                        it_a += 2;
                    } else if (num::GET_POS(it_a) > num::GET_POS(j)) {
                        if (GET_OCC(a) + 1 > GET_SIZE(a)) {
                            ENLARGE(a);
                            it_a = AT(a, i);
                            a_end = AT(a, GET_OCC(a));
                        }
                        // copy j and j + 1 into a.
                        std::copy_backward(it_a, a_end, a_end + 2);
                        num::ASSIGN(it_a, num::iMUL(lambda, j));
                        SET_OCC(a, GET_OCC(a) + 1);
                        j += 2;
                    } else {
                        num::ap_int x = num::iMUL(lambda, j);
                        num::ADD(it_a, x);
                        num::DELETE(x);
                        it_a += 2;
                        i++;
                        j += 2;
                    }
                }

                if (j != b_end) {
                    if (GET_OCC(a) + (b_end - j) / 2 > GET_SIZE(a)) {
                        ENLARGE(a, GET_OCC(a) + (b_end - j) / 2);
                        a_end = AT(a, GET_OCC(a));
                    }
                    std::copy(j, b_end, a_end);
                }
            }

            void MUL(s_vec v, num::ap_int const n) {
                for (int i = 0; i < GET_OCC(v); i++) {
                    num::ap_int res = num::MUL(AT(v, i), n);
                }
            }
        }
    }
}