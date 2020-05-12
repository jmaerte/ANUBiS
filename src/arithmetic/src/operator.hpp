#ifndef ANUBIS_ARITHMETIC_SUPERBUILD_SRC_OPERATOR_PRIVATE_HPP
#define ANUBIS_ARITHMETIC_SUPERBUILD_SRC_OPERATOR_PRIVATE_HPP

//
// Created by jmaerte on 23.03.20.
//

#include "arithmetic/operator.hpp"
#include "arithmetic.hpp"
#include "aux/hardware/hardware.hpp"
#include "constants.hpp"
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <bitset>
#include <stdexcept>
#include <string>
#include "arithmetic/factory/factory.hpp"

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
                if (IS_SINGLE(n)) return (GET_SIGN(n) ? "-" : "") + std::to_string((n+1)->single);
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

            static inline bool IS_SINGLE(ap_int const n) {
                return n->single & num::SINGLE_MASK;
            }

            static inline int GET_POS(ap_int const n) {
                return IS_SINGLE(n) ? (n->single >> 2) : (n->single >> 32);
            }

            static inline int GET_OCC(ap_int const n) {
                return (n->single & constants::L_MASK) >> 17;
            }

            static inline ULL* ABS(ap_int const n) {
                return n->single & num::SINGLE_MASK ? &((n + 1)->single) : (n + 1)->value;
            }

            static inline bool GET_SIGN(ap_int const n) {
                return (n->single & num::SIGN_MASK) >> 1;
            }

            static inline bool IS_NA(ap_int a) {
                return a == nullptr;
            }

            static inline int GET_SIZE(ap_int const n) {
                return (n->single & num::SIZE_MASK) >> 2;
            }

            static inline void SET_POS(ap_int n, int pos) {
                if (IS_SINGLE(n)) n->single = (n->single & 3ULL) | (((ULL) pos) << 2);
                else n->single = (n->single & ~num::POS_MASK) | (((ULL) pos) << 32);
            }

            static inline void SET_SIZE(ap_int n, int size) {
                n->single = n->single & ~num::SIZE_MASK | (((ULL) size) << 2);
            }

            static inline void SET_OCC(ap_int n, int occ) {
                n->single = n->single & ~num::OCC_MASK | ((ULL) occ) << 17;
            }

            static inline void SET_SIGN(ap_int n, bool sign) {
                n->single = n->single & ~num::SIGN_MASK | ((sign ? 1ULL : 0ULL) << 1);
            }

            static inline void MAKE_MULTI(ap_int a, int initial_size) {
                a->single = (a->single & ~3ULL) << 30 | (1ULL << 17) | ((ULL) initial_size << 2) | (a->single & 2ULL);
                ULL val = (a + 1)->single;
                *(a + 1) = (svec_node){.value = new ULL[initial_size] { }};
                *((a + 1)->value) = val;
            }

            static inline void MAKE_MULTI(ap_int a, ULL value) {
                MAKE_MULTI(a, 2);
                if (value) SET_OCC(a, 2);
                *((a + 1)->value + 1) = value;
            }

            static inline void MAKE_SINGLE(ap_int a) {
                a->single = 1ULL | (a->single & 2ULL) | (a->single >> 32);
                ULL val = *((a + 1)->value);
                delete[] (a + 1)->value;
                *(a + 1) = (svec_node){.single = val};
            }

            static inline void SWITCH_SIGN(ap_int n) {
                n->single ^= num::SIGN_MASK;
            }

            static inline void ASSIGN(ap_int a, ap_int const &b) {
                a->single = b->single;
                if (IS_SINGLE(a)) (a + 1)->single = (b + 1)->single;
                else (a + 1)->value = (b + 1)->value;
            }

            static inline void OVERWRITE(ap_int a, ap_int const &b) {
                if (!IS_NA(a)) DELETE_DATA(a);
                *a = *b;
                *(a + 1) = *(b + 1);
            }

            static inline void ENLARGE(ap_int n, int size) {
                if (size <= GET_SIZE(n)) return;
                ULL* next = new ULL[size] { };
                std::copy(ABS(n), ABS(n) + GET_OCC(n), next);
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
                if (initial_size == 1) return NEW(sign, value);
                ap_int i = new svec_node[2]{
                        {.single = (sign ? num::SIGN_MASK : 0ULL) | ((initial_size & constants::LL_MASK) << 2) |
                                 (value ? 1ULL : 0ULL) << 17},
                        {.value = new ULL[initial_size] { }}
                };
                *((i + 1)->value) = value;
                return i;
            }

            static inline ap_int NEW(bool sign, ULL value) {
                return new svec_node[2]{
                    {.single = (sign ? 1ULL : 0ULL) << 1 | 1ULL},
                    {.single = value}
                };
            }

            static inline void NEW(ap_int dest, bool sign, ULL value) {
                *dest = {.single = ((sign ? 1ULL : 0ULL) << 1) | 1ULL};
                *(dest + 1) = {.single = value};
            }

            static inline ap_int NEW(ULL* value, ULL size, ULL occ, bool sign) {
                return new svec_node[2]{
                        {.single = (sign ? num::SIGN_MASK : 0u) | (size << 2) | (occ << 17)},
                        {.value = value}
                };
            }

            static inline ap_int COPY(ap_int const n) {
                if (IS_SINGLE(n)) {
                    ap_int res = NEW(GET_SIGN(n), *ABS(n));
                    SET_POS(res, GET_POS(n));
                    return res;
                }
                ap_int res = NEW(GET_OCC(n), GET_SIGN(n), 0ULL);
                *res = *n;
                std::copy(ABS(n), ABS(n) + GET_OCC(n), ABS(res)); // was std::copy before.
                return res;
            }

            static inline void DELETE_DATA(ap_int i) {
//                std::free((i + 1)->value);
                if (!IS_SINGLE(i)) delete[] (i + 1)->value;
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
                    if (!IS_SINGLE(b)) {
                        if (IS_SINGLE(a)) {
                            MAKE_MULTI(a, MAX(GET_OCC(a), GET_OCC(b)) + 1);
                        } else ENLARGE(a, MAX(GET_OCC(a), GET_OCC(b)) + 1);
                        SET_OCC(a, MAX(GET_OCC(a), GET_OCC(b)) + aux::ADD_DATA(ABS(a), ABS(b), GET_OCC(a), GET_OCC(b)));
                    } else {
                        if (IS_SINGLE(a)) {
                            unsigned char c = adc(0, *ABS(a), *ABS(b), ABS(a));
                            if (c) {
                                MAKE_MULTI(a, 2);
                                *(ABS(a) + 1) = 1ULL;
                            }
                        } else {
                            aux::C_ADD(a, *ABS(b));
                        }
                    }
                }
            }

            static inline void SUB(ap_int a, ap_int b) {
                if (GET_SIGN(a) != GET_SIGN(b)) {
                    SWITCH_SIGN(a);
                    ADD(a, b);
                    SWITCH_SIGN(a);
                } else {
                    if (!IS_SINGLE(b)) {
                        ULL size = MAX(GET_OCC(a), GET_OCC(b)) + 1;
                        if (IS_SINGLE(a)) {
                            MAKE_MULTI(a, size);
                        } else ENLARGE(a, size);
                        SET_SIGN(a, GET_SIGN(a) ^ aux::SUB_DATA(ABS(a), ABS(b), GET_OCC(a), GET_OCC(b)));
                        aux::STRIP(ABS(a), size);
                        if (size <= 1) {
                            MAKE_SINGLE(a);
                        } else SET_OCC(a, size);
                    } else {
                        if (IS_SINGLE(a)) {
                            ULL va = *ABS(a), vb = *ABS(b);
                            *ABS(a) = va < vb ? vb - va : va - vb;
                            SET_SIGN(a, GET_SIGN(a) ^ va < vb);
                        } else {
                            aux::C_SUB(a, *ABS(b));
                        }
                    }
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

            static inline bool GET_IS_ALL_SINGLE(s_ap_int_vec vec) {
                return vec->single & vec::ALL_SINGLE_MASK;
            }

            static inline int GET_SIZE(s_ap_int_vec v) {
                return (v->single & vec::SIZE_MASK) >> 4;
            }

            static inline int GET_OCC(s_ap_int_vec v) {
                return v->single >> 34;
            }

            static inline unsigned int GET_FACTORY_ID(s_ap_int_vec v) {
                return (v->single & vec::FACTORY_ID_MASK) >> 1;
            }

            static inline void SET_SIZE(s_ap_int_vec v, int size) {
                v->single = v->single & ~vec::SIZE_MASK | ((ULL) size << 4);
            }

            static inline void SET_OCC(s_ap_int_vec v, int occ) {
                v->single = v->single & ~vec::OCC_MASK | (((ULL) occ) << 34);
            }

            static inline void SET_FACTORY_ID(s_ap_int_vec v, unsigned int id) {
                if (id > factory::MAX_FACTORIES) throw std::out_of_range("[Mem] ERROR - Factory id " + std::to_string(id) + " is invalid!");
                v->single = v->single & ~vec::FACTORY_ID_MASK | (id << 1);
            }

            static inline num::ap_int AT(s_ap_int_vec v, int i) {
                return v + 1 + 2 * i;
            }

            static inline void ENLARGE_RANGE(s_ap_int_vec& v, int size, int copy_range) {
                factory::dict.get_factory(v)->enlarge(v, size, copy_range);
            }

            static inline void ENLARGE_RANGE(s_ap_int_vec& v, int copy_range) {
                ENLARGE_RANGE(v, 2 * GET_SIZE(v), copy_range);
            }

            static inline void ENLARGE(s_ap_int_vec& v, int size) {
                ENLARGE_RANGE(v, size, GET_OCC(v));
            }

            static inline void ENLARGE(s_ap_int_vec& v) {
                ENLARGE(v, 2 * GET_SIZE(v));
            }

            static inline void SWITCH_SIGNS(s_ap_int_vec v) {
                int occ = GET_OCC(v);
                for (int i = 0; i < occ; i++) {
                    num::SWITCH_SIGN(AT(v, i));
                }
            }

            static inline void SWAP_VALUES(s_ap_int_vec v, int i, int j) {
                ULL temp = AT(v, i)->single;
                AT(v, i)->single = AT(v, j)->single & ~num::POS_MASK | temp & num::POS_MASK;
                AT(v, j)->single = temp & ~num::POS_MASK | AT(v, j)->single & num::POS_MASK;
                // swap data pointers
                auto a = AT(v, i) + 1;
                *(AT(v, i) + 1) = *(AT(v, j) + 1);
                *(AT(v, j) + 1) = *a;
            }

            static inline void SET(s_ap_int_vec v, int k, num::ap_int n) {
                num::ASSIGN(AT(v, k), n); // OVERWRITE?
            }

            /*
             * (DE-)ALLOCATION
             */

            namespace factory {

                template<typename factory_type>
                static inline unsigned int REGISTER() {
                    std::cout << "[Mem] Registering factory of type " << typeid(factory_type).name() << "." << std::endl;
                    auto fact = new factory_type();
                    return fact->get_id();
                }

                static inline void RELEASE(unsigned int id) {
                    factory::dict.release_factory(id);
                    std::cout << "[Mem] Released factory " << id << " successfully!" << std::endl;
                }
            }

            static inline s_ap_int_vec NEW(unsigned int factory_id, std::vector < std::pair < ULL, std::pair < bool, ULL > > > vector) {
                svec_node* v = factory::dict.get_factory(factory_id)->allocate_vec(vector.size());
                *v = {.single = 1ULL | (((ULL) vector.size()) << 34) | (((ULL) vector.size()) << 4) | (factory_id << 1)};
                int i = 1;
                for (auto it = vector.begin(); it != vector.end(); it++) {
                    num::NEW(v + i, it->second.first, it->second.second);
                    num::SET_POS(v + i, it->first);
                    i += 2;
                }
                return v;
            }

            static inline s_ap_int_vec COPY(unsigned int factory_id, s_ap_int_vec v) {
                return factory::dict.get_factory(factory_id)->copy(v);
            }

            static inline void DELETE(s_ap_int_vec& v) {
                factory::dict.get_factory(v)->deallocate_vec(v);
            }

            static inline void DELETE_POS(s_ap_int_vec v, int i) {
                num::DELETE_DATA(AT(v, i));
                std::copy(AT(v, i + 1), AT(v, GET_OCC(v)), AT(v, i));
                SET_OCC(v, GET_OCC(v) - 1);
            }

            /*
             * ARITHMETIC
             */

            static inline void REDUCE(s_ap_int_vec& a, s_ap_int_vec b, int k) {
                if (GET_IS_ALL_SINGLE(b)) {
                    if (GET_IS_ALL_SINGLE(a)) {
                        ADD_ALL_SINGLE(a, k + 1, AT(a, k), b, 1);
                    } else {
                        ADD_SINGLE(a, k + 1, AT(a, k), b, 1);
                    }
                } else {
                    if(GET_IS_ALL_SINGLE(a)) {
                        a->single &= ~vec::ALL_SINGLE_MASK; // there exists multi
                        ADD_SINGLE_SCALAR(a, k + 1, AT(a, k), b, 1);
                    } else {
                        ADD(a, k + 1, AT(a, k), b, 1);
                    }
                }
                vec::DELETE_POS(a, k);
            }
        }
    }
}

#endif //ANUBIS_ARITHMETIC_SUPERBUILD_SRC_OPERATOR_PRIVATE_HPP