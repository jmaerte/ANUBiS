#ifndef ANUBIS_SUPERBUILD_SRC_OPERATOR_HPP
#define ANUBIS_SUPERBUILD_SRC_OPERATOR_HPP

//
// Created by jmaerte on 23.03.20.
//

#include "arithmetic/operator.hpp"
#include "arithmetic.hpp"
#include "constants.hpp"
#include <cstring>
#include <vector>

namespace jmaerte {
    namespace arith {
/***********************************************************************************************************************
 * NUM - DECL
 **********************************************************************************************************************/
        namespace num {
            /*
             * OPERATORS
             */

            static inline int GET_POS(ap_int const n) {
                return n->meta & NUM_POS_MASK >> 32;
            }

            static inline int GET_OCC(ap_int const n) {
                return n->meta & NUM_OCC_MASK;
            }

            static inline ULL *GET_ABS_DATA(ap_int const n) {
                return (n + 1)->value;
            }

            static inline bool GET_SIGN(ap_int const n) {
                return n->meta & NUM_SIGN_MASK;
            }

            static inline bool IS_NA(ap_int a) {
                return a == nullptr;
            }

            static inline int GET_SIZE(ap_int const n) {
                return n->meta & LL_MASK;
            }

            static inline void SET_POS(ap_int n, int pos) {
                n->meta = n->meta & ~NUM_POS_MASK | ((ULL) pos << 32) & NUM_POS_MASK;
            }

            static inline void SET_SIZE(ap_int n, int size) {
                n->meta = n->meta & ~NUM_SIZE_MASK | ((size & LL_MASK) << 16);
            }

            static inline void SET_OCC(ap_int n, int occ) {
                n->meta = n->meta & ~NUM_OCC_MASK | (occ & LL_MASK);
            }

            static inline void SET_SIGN(ap_int n, bool sign) {
                n->meta = n->meta & ~NUM_SIGN_MASK | ((sign ? 1ULL : 0ULL) << 63);
            }

            static inline void SWITCH_SIGN(ap_int n) {
                n->meta ^= NUM_SIGN_MASK;
            }

            static inline void ASSIGN(ap_int a, ap_int const &b) {
                *a = *b;
                *(a + 1) = *(b + 1);
            }

            static inline void OVERWRITE(ap_int a, ap_int const &b) {
                if (!IS_NA(a)) DELETE_DATA(a);
                *a = *b;
                *(a + 1) = *(b + 1);
            }

            static inline void ENLARGE(ap_int n, int size) {
                if (size <= GET_SIZE(n)) return;
                (n + 1)->value = (ULL *) realloc((n + 1)->value, size * BYTES_PER_ULL);
                SET_SIZE(n, size);
            }

            static inline void ENLARGE(ap_int n) {
                ENLARGE(n, 2 * GET_SIZE(n));
            }

            /*
             * (DE-)ALLOCATION
             */

            static inline ap_int NEW(ULL initial_size, bool sign, ULL value) {
                ap_int i = new svec_node[2]{
                        {.meta = (sign ? NUM_SIGN_MASK : 0ULL) | ((initial_size & LL_MASK) << 16) |
                                 (value ? 1ULL : 0ULL)},
                        {.value = (ULL *) calloc(initial_size, BYTES_PER_ULL)}
                };
                *((i + 1)->value) = value;
                return i;
            }

            static inline ap_int NEW(ULL *value, ULL size, ULL occ, bool sign) {
                return new svec_node[2]{
                        {.meta = (sign ? NUM_SIGN_MASK : 0u) | (size & LL_MASK << 16) | (occ & LL_MASK)},
                        {.value = value}
                };
            }

            static inline ap_int COPY(ap_int const n) {
                ap_int res = NEW(GET_OCC(n), GET_SIGN(n), 0ULL);
                std::memcpy(GET_ABS_DATA(n), GET_ABS_DATA(res), GET_OCC(n) * BYTES_PER_ULL); // was std::copy before.
                return res;
            }

            static inline void DELETE_DATA(ap_int i) {
                free((i + 1)->value);
//                delete[] (i + 1)->value;
            }

            static inline void DELETE(ap_int i) {
                DELETE_DATA(i);
                delete (i + 1);
                delete i;
            }

            /*
             * ARITHMETIC
             */

            static inline void ADD(ap_int a, ap_int b) {
                if (GET_SIGN(a) != GET_SIGN(b)) SUB(a, b);
                else {
                    ENLARGE(a, MAX(GET_OCC(a), GET_OCC(b)) + 1);
                    SET_OCC(a, GET_OCC(a) + aux::ADD_DATA(GET_ABS_DATA(a), GET_ABS_DATA(b), GET_OCC(a), GET_OCC(b)));
                }
            }

            static inline void SUB(ap_int a, ap_int b) {
                if (GET_SIGN(a) != GET_SIGN(b)) ADD(a, b);
                else {
                    ULL size = MAX(GET_OCC(a), GET_OCC(b)) + 1;
                    ENLARGE(a, size); // this sets the last place to 0.
                    SET_SIGN(a, aux::SUB_DATA(GET_ABS_DATA(a), GET_ABS_DATA(b), GET_OCC(a), GET_OCC(b)));
                    aux::STRIP(GET_ABS_DATA(a), size);
                    SET_OCC(a, size);
                }
            }

            /**
             * Calculates a (mod b) where b is an odd modulus. Some precomputation is necessary.
             * @param a dividend
             * @param b divisor/modulus
             * @param R_POW R^n (mod b), where R is the radix of b, i.e. R = r^m, r = 2^64 and r^{n-1} <= b < r^n.
             *          and n is such that R^{n-1} <= a < R^n.
             * @param inv mod-inverse of b, i.e. b * inv = 1 (mod R)
             */
            static inline void MOD(ap_int a, ap_int b, ap_int R_POW, ap_int inv) {
                aux::modular::SREM(a, b, inv);
                if (GET_OCC(a)) aux::modular::MODMUL(a, R_POW, b, inv);
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

            static inline int GET_SIZE(s_vec v) {
                return v->meta >> 32;
            }

            static inline int GET_OCC(s_vec v) {
                return v->meta & L_MASK;
            }

            static inline void SET_OCC(s_vec v, int occ) {
                v->meta = v->meta & H_MASK | (ULL) occ;
            }

            static inline num::ap_int AT(s_vec v, int i) {
                return v + 1 + 2 * i;
            }

            static inline void ENLARGE(s_vec &v, int size) {
                s_vec next = new svec_node[2 * size + 1];
                std::copy(v, AT(v, GET_OCC(v)), next);
                delete[] v;
                v = next;
            }

            static inline void ENLARGE(s_vec &v) {
                ENLARGE(v, 2 * GET_SIZE(v));
            }

            static inline void SWITCH_SIGNS(s_vec v) {
                int occ = GET_OCC(v);
                for (int i = 0; i < occ; i++) {
                    num::SWITCH_SIGN(AT(v, i));
                }
            }

            static inline void SWAP_VALUES(s_vec v, int i, int j) {
                ULL temp = AT(v, i)->meta;
                AT(v, i)->meta = AT(v, j)->meta & ~NUM_POS_MASK | temp & NUM_POS_MASK;
                AT(v, j)->meta = temp & ~NUM_POS_MASK | AT(v, j)->meta & NUM_POS_MASK;
                // swap data pointers
                auto a = AT(v, i) + 1;
                *(AT(v, i) + 1) = *(AT(v, j) + 1);
                *(AT(v, j) + 1) = *a;
            }

            static inline void SET(s_vec v, int k, num::ap_int n) {
                num::ASSIGN(AT(v, k), n);
            }

            /*
             * (DE-)ALLOCATION
             */

            static inline s_vec NEW(std::vector < std::pair < ULL, std::pair < bool, ULL >> > vector) {
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

            static inline void DELETE(s_vec &v) {
                ULL occ = v->meta & L_MASK;
                for (int i = 0; i < occ; i++) {
                    num::DELETE(AT(v, i));
                }
                delete[] v;
            }

            static inline void DELETE_POS(s_vec &v, int i) {
                num::DELETE_DATA(AT(v, i));
                std::copy(AT(v, i + 1), AT(v, GET_OCC(v)), AT(v, i));
                SET_OCC(v, GET_OCC(v) - 1);
            }
        }
    }
}

#endif //ANUBIS_SUPERBUILD_SRC_OPERATOR_HPP