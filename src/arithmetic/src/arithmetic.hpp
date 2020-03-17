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

namespace jmaerte {
    namespace arith {
/***********************************************************************************************************************
 * AUX
 **********************************************************************************************************************/

        namespace num {
            namespace aux {

                void STRIP(ULL const* a, int offset, ULL& occ);
                void SET(ap_int n, int pos, ULL val);
                ULL GET(ap_int n, int pos);
                unsigned int GET_LEADING(ap_int const n);
                void REMOVE(ap_int n, int i);
                void ADD(ap_int a, ULL lambda, int shift, ap_int const b);
                void SUB(ap_int a, ULL lambda, int shift, ap_int const b);
                void KMUL(ULL* result, bool offset, ULL const* a, ULL a_occ, bool a_off, ULL const* b, ULL b_occ, bool b_off);

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

            inline int GET_MEMSIZE(ap_int const n) {
                return GET_OCC(n) / 2 + (GET_OCC(n) % 2 ? 1 : 0);
            }

            inline ULL* GET_ABS_DATA(ap_int const n) {
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

            inline void ASSIGN(ap_int a, ap_int const& b) {
                *a = *b;
                *(a + 1) = *(b + 1);
            }

            inline void ENLARGE(ap_int& n, int size) {
                if (size <= GET_SIZE(n)) return;
                if (size % 2 != 0) size++;
                ULL* next = new ULL[size / 2];
                std::copy(GET_ABS_DATA(n), GET_ABS_DATA(n) + GET_OCC(n), next);
                delete[] GET_ABS_DATA(n);
                (n + 1)->value = next;
                SET_SIZE(n, size);
            }

            inline void ENLARGE(ap_int& n) {
                ENLARGE(n, 2 * GET_SIZE(n));
            }

            ap_int PREPARE_SMOD_DIVISOR(ap_int divisor, ap_int denominators, int arr_size) {
                int n = -1;
                for (int i = 0; i < arr_size; i++) {
                    if (n < 0 || n > GET_OCC(denominators)) {
                        n = GET_OCC(denominators);
                    }
                    denominators += 2;
                }
                n = n / 2 + n % 2;

                // TODO: modify divisor accordingly.

                return nullptr;
            }

            ap_int PREPARE_NUM_SVOBODA(ap_int const n) {
                ULL b = aux::GET_LEADING(n);
                ap_int k = NEW(2, false, UI_MAX / b + (UI_MAX % b != 0));
                *GET_ABS_DATA(k) = *GET_ABS_DATA(k) << 32;
                return MUL(k, n);
            }

            /*
             * COMPARATORS
             */

            int COMPARE_ABS(ap_int const n_a, ap_int const n_b) {
                int occ = GET_OCC(n_a);
                if (occ != GET_OCC(n_b)) return occ - GET_OCC(n_b);
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

            bool comp::SIGNED_COMPARATOR::operator()(ap_int const& a, ap_int const& b) const {
                if ((a->meta ^ b->meta) & NUM_SIGN_MASK) return GET_SIGN(a) ? b : a;
                if (COMPARE_ABS(a, b) < 0) {
                    return !GET_SIGN(a);
                } else return GET_SIGN(a);
            }

            bool comp::UNSIGNED_COMPARATOR::operator()(ap_int const& a, ap_int const& b) const {
                return COMPARE_ABS(a, b) < 0;
            }

            /*
             * (DE-)ALLOCATION
             */

            inline ap_int NEW(ULL initial_size, bool sign, ULL value) {
                if (initial_size % 2 != 0) initial_size++;
                ap_int i = new svec_node[2]{
                        {.meta = (sign ? NUM_SIGN_MASK : 0ULL) | ((initial_size & LL_MASK) << 16) | (value ? (value & H_MASK ? 2ULL : 1ULL): 0ULL)},
                        {.value = new ULL[initial_size / 2] { }}
                };
                *((i + 1)->value) = value;
                return i;
            }

            inline ap_int NEW(ULL* value, int size, int occ, bool sign) {
                return new svec_node[2] {
                        {.meta = (sign ? NUM_SIGN_MASK : 0u) | ((ULL) size & LL_MASK << 16) | ((ULL) occ & LL_MASK)},
                        {.value = value}
                };
            }

            inline ap_int COPY(ap_int const n) {
                ap_int res = NEW(GET_OCC(n), GET_SIGN(n), GET_ABS_DATA(n)[0]);
                for (int i = 1; i < GET_OCC(n); i++) {
                    GET_ABS_DATA(res)[i] = GET_ABS_DATA(n)[i];
                }
                return res;
            }

            inline void DELETE_DATA(ap_int i) {
                delete[] (i + 1)->value;
            }

            inline void DELETE(ap_int i) {
                delete (i + 1);
                delete i;
            }

            /*
             * ARITHMETIC
             */

            ap_int MUL(const ap_int& lambda, const ap_int& b) {
                if (GET_OCC(lambda) > GET_OCC(b)) {
                    return MUL(b, lambda);
                }
                int occ = GET_OCC(lambda) + GET_OCC(b);
                ULL* result = new ULL[occ / 2 + occ % 2];
                aux::KMUL(result, false, (lambda + 1)->value, GET_OCC(lambda), false, (b + 1)->value, GET_OCC(b), false);
                return NEW(result, 2 * (occ / 2 + occ % 2), occ, GET_SIGN(lambda) ^ GET_SIGN(b));
            }

            bool ADD(ap_int& a, ap_int const& b) {
                if (GET_OCC(b) > GET_OCC(a)) {

                }
                return true;
            }

            void SMOD(ap_int a, ap_int const b, ap_int const pre) {

            }

            ap_int SDIV(ap_int const a, ap_int const b, ap_int const pre) {
                int m = GET_OCC(a) - GET_OCC(b);
                int n = GET_OCC(b);
                ap_int Q = NEW(m, GET_SIGN(a) ^ GET_SIGN(b), 0ULL);
                for (int j = m - 1; j > 0; j--) {
                    aux::SET(Q, j, aux::GET(a, n + j));
                    aux::SUB(a, aux::GET(Q, j), j - 1, pre);
                    if (GET_SIGN(a)) {
                        aux::SET(Q, j, aux::GET(Q, j) - 1);
                        aux::ADD(a, 1ULL, j - 1, pre);
                    }
                }
                aux::SET(Q, 0, (aux::GET(a, GET_OCC(a) - 1) << 32 | aux::GET(a, GET_OCC(a) - 2)) / aux::GET_LEADING(b));
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
                        if (GET_OCC(a) + 2 > GET_SIZE(a)) {
                            ENLARGE(a);
                            it_a = AT(a, i);
                            a_end = AT(a, GET_OCC(a));
                        }
                        // copy j and j + 1 into a.
                        std::copy_backward(it_a, a_end, a_end + 2);
                        num::ASSIGN(it_a, num::MUL(lambda, j));
                        SET_OCC(a, GET_OCC(a) + 1);
                        j += 2;
                    } else {
                        num::ap_int x = num::MUL(lambda, j);
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
                    num::DELETE_DATA(AT(v, i));
                    SET(v, i, n);
                }
            }
        }
    }
}