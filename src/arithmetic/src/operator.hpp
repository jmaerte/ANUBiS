#ifndef ANUBIS_ARITHMETIC_SUPERBUILD_SRC_OPERATOR_PRIVATE_HPP
#define ANUBIS_ARITHMETIC_SUPERBUILD_SRC_OPERATOR_PRIVATE_HPP

//
// Created by jmaerte on 23.03.20.
//

#include "arithmetic/operator.hpp"
#include "arithmetic.hpp"
#include "constants.hpp"
#include <algorithm>
#include <vector>
#include <cstdlib>

namespace jmaerte {
    namespace arith {
/***********************************************************************************************************************
 * NUM - DECL
 **********************************************************************************************************************/
        namespace num {
            /*
             * OPERATORS
             */

            static inline std::string STRINGIFY(ap_int const n) {
                std::string res = std::string("(") + (GET_SIGN(n) ? "-" : "");
                bool set = false;
                ULL pos;
                for (int i = GET_OCC(n) - 1; i >= 0; i--) {
                    pos = 1ULL << 63;
                    while (pos != 0) {
                        if (set) {
                            res += (aux::GET(n, i) & pos) != 0 ? "1" : "0";
                        } else {
                            if (aux::GET(n, i) & pos) {
                                set = true;
                                res += "1";
                            }
                        }
                        pos >>= 1;
                    }
                }
                if (!set) res += "0";
                res += ")_2";
                return res;
            }

            static inline int GET_POS(ap_int const n) {
                return (n->meta & NUM_POS_MASK) >> 32;
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
                return (n->meta & NUM_SIZE_MASK) >> 16;
            }

            static inline void SET_POS(ap_int n, int pos) {
                n->meta = (n->meta & ~NUM_POS_MASK) | (((ULL) pos) << 32);
            }

            static inline void SET_SIZE(ap_int n, int size) {
                n->meta = n->meta & ~NUM_SIZE_MASK | ((((ULL) size) & LL_MASK) << 16);
            }

            static inline void SET_OCC(ap_int n, int occ) {
                n->meta = n->meta & ~NUM_OCC_MASK | (((ULL) occ) & LL_MASK);
            }

            static inline void SET_SIGN(ap_int n, bool sign) {
                n->meta = n->meta & ~NUM_SIGN_MASK | ((sign ? 1ULL : 0ULL) << 63);
            }

            static inline void SWITCH_SIGN(ap_int n) {
                n->meta ^= NUM_SIGN_MASK;
            }

            static inline void ASSIGN(ap_int a, ap_int const &b) {
                a->meta = b->meta;
                (a + 1)->value = (b + 1)->value;
            }

            static inline void OVERWRITE(ap_int a, ap_int const &b) {
                if (!IS_NA(a)) DELETE_DATA(a);
                *a = *b;
                *(a + 1) = *(b + 1);
            }

            static inline void ENLARGE(ap_int n, int size) {
                if (size <= GET_SIZE(n)) return;
                ULL* next = new ULL[size] { };
                std::copy(GET_ABS_DATA(n), GET_ABS_DATA(n) + GET_OCC(n), next);
                delete[] (n + 1)->value;
                (n + 1)->value = next;
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
                        {.value = new ULL[initial_size] { }}
                };
                *((i + 1)->value) = value;
                return i;
            }

            static inline ap_int NEW(ULL* value, ULL size, ULL occ, bool sign) {
                return new svec_node[2]{
                        {.meta = (sign ? NUM_SIGN_MASK : 0u) | (size & LL_MASK << 16) | (occ & LL_MASK)},
                        {.value = value}
                };
            }

            static inline ap_int COPY(ap_int const n) {
                ap_int res = NEW(GET_OCC(n), GET_SIGN(n), 0ULL);
                *res = *n;
                std::copy(GET_ABS_DATA(n), GET_ABS_DATA(n) + GET_OCC(n), GET_ABS_DATA(res)); // was std::copy before.
                return res;
            }

            static inline void DELETE_DATA(ap_int i) {
//                std::free((i + 1)->value);
                delete[] (i + 1)->value;
            }

            static inline void DELETE(ap_int i) {
                DELETE_DATA(i);
                delete[] i;
            }

            /*
             * ARITHMETIC
             */

            static inline void ADD(ap_int a, ap_int b) {
                if (GET_SIGN(a) != GET_SIGN(b)) {
                    SWITCH_SIGN(a);
                    SUB(a, b);
                    SWITCH_SIGN(a);
                } else {
//                    std::cout << "Adding" << std::endl;
                    ENLARGE(a, MAX(GET_OCC(a), GET_OCC(b)) + 1);
//                    std::cout << "enlarged: occ = " << GET_OCC(a) << " size = " << GET_SIZE(a) << std::endl;
//                    std::cout << "a = " << num::STRINGIFY(a) << std::endl;
//                    std::cout << "b = " << num::STRINGIFY(b) << std::endl;
                    SET_OCC(a, MAX(GET_OCC(a), GET_OCC(b)) + aux::ADD_DATA(GET_ABS_DATA(a), GET_ABS_DATA(b), GET_OCC(a), GET_OCC(b)));
//                    std::cout << "sum = " << num::STRINGIFY(a) << std::endl;
                }
            }

            static inline void SUB(ap_int a, ap_int b) {
                if (GET_SIGN(a) != GET_SIGN(b)) {
                    SWITCH_SIGN(a);
                    ADD(a, b);
                    SWITCH_SIGN(a);
                } else {
//                    std::cout << "Subtracting" << std::endl;
                    ULL size = MAX(GET_OCC(a), GET_OCC(b)) + 1;
                    ENLARGE(a, size); // this sets the last place to 0.
//                    std::cout << "enlarged: occ = " << GET_OCC(a) << " size = " << GET_SIZE(a) << std::endl;
//                    std::cout << "a = " << num::STRINGIFY(a) << std::endl;
//                    std::cout << "b = " << num::STRINGIFY(b) << std::endl;
                    SET_SIGN(a, GET_SIGN(a) ^ aux::SUB_DATA(GET_ABS_DATA(a), GET_ABS_DATA(b), GET_OCC(a), GET_OCC(b)));
                    aux::STRIP(GET_ABS_DATA(a), size);
                    SET_OCC(a, size);
//                    std::cout << "diff = " << num::STRINGIFY(a) << std::endl;
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

            static inline int GET_SIZE(s_ap_int_vec v) {
                return v->meta >> 32;
            }

            static inline int GET_OCC(s_ap_int_vec v) {
                return v->meta & L_MASK;
            }

            static inline void SET_SIZE(s_ap_int_vec* v, int size) {
                (*v)->meta = (*v)->meta & L_MASK | (((ULL) size) << 32);
            }

            static inline void SET_OCC(s_ap_int_vec* v, int occ) {
                (*v)->meta = (*v)->meta & H_MASK | (ULL) occ;
            }

            static inline num::ap_int AT(s_ap_int_vec v, int i) {
                return v + 1 + 2 * i;
            }

            static inline void ENLARGE(s_ap_int_vec* v, int size) {
                svec_node* next = new svec_node[2 * size + 1] { };
                std::copy(*v, AT(*v, GET_OCC(*v)), next);
                delete[] *v;
                *v = next;
                SET_SIZE(v, size);
            }

            static inline void ENLARGE(s_ap_int_vec* v) {
                ENLARGE(v, 2 * GET_SIZE(*v));
            }

            static inline void SWITCH_SIGNS(s_ap_int_vec v) {
                int occ = GET_OCC(v);
                for (int i = 0; i < occ; i++) {
                    num::SWITCH_SIGN(AT(v, i));
                }
            }

            static inline void SWAP_VALUES(s_ap_int_vec v, int i, int j) {
                ULL temp = AT(v, i)->meta;
                AT(v, i)->meta = AT(v, j)->meta & ~NUM_POS_MASK | temp & NUM_POS_MASK;
                AT(v, j)->meta = temp & ~NUM_POS_MASK | AT(v, j)->meta & NUM_POS_MASK;
                // swap data pointers
                auto a = AT(v, i) + 1;
                *(AT(v, i) + 1) = *(AT(v, j) + 1);
                *(AT(v, j) + 1) = *a;
            }

            static inline void SET(s_ap_int_vec* v, int k, num::ap_int n) {
                num::ASSIGN(AT(*v, k), n); // OVERWRITE?
            }

            /*
             * (DE-)ALLOCATION
             */

            static inline s_ap_int_vec NEW(std::vector < std::pair < ULL, std::pair < bool, ULL > > > vector) {
                svec_node* v = new svec_node[1 + 2 * vector.size()] { };
                *v = {.meta = (((ULL) vector.size()) << 32) | (((ULL) vector.size()) & L_MASK)};
                int i = 1;
                for (auto it = vector.begin(); it != vector.end(); it++) {
                    num::ap_int n = num::NEW(1ULL, it->second.first, it->second.second);
                    num::SET_POS(n, it->first);
                    *(v + i++) = *n;
                    *(v + i++) = *(n + 1);
                    delete[] n;
                }
                return v;
            }

            static inline void DELETE(s_ap_int_vec* v) {
                ULL occ = GET_OCC(*v);
                for (int i = 0; i < occ; i++) {
                    num::DELETE_DATA(AT(*v, i));
                }
                delete[] *v;
//                std::free(v);
            }

            static inline void DELETE_POS(s_ap_int_vec v, int i) {
                num::DELETE_DATA(AT(v, i));
                std::copy(AT(v, i + 1), AT(v, GET_OCC(v)), AT(v, i));
                SET_OCC(&v, GET_OCC(v) - 1);
            }
        }
    }
}

#endif //ANUBIS_ARITHMETIC_SUPERBUILD_SRC_OPERATOR_PRIVATE_HPP