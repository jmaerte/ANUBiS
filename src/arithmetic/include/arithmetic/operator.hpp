//
// Created by jmaerte on 16.03.20.
//

#ifndef ANUBIS_SUPERBUILD_OPERATOR_HPP
#define ANUBIS_SUPERBUILD_OPERATOR_HPP

#include "typedef.hpp"
#include <vector>
#include <tuple>
#include <iostream>
#include <ARITHMETIC_EXPORT.h>

// TEMP - BELONGS TO arithmetic.hpp
namespace jmaerte {
    namespace arith {
        namespace num {
            namespace aux {
                void ADD(ap_int a, ULL lambda, int shift, ap_int const b);
                void SUB(ap_int a, ULL lambda, int shift, ap_int const b);
            }
        }
    }
}

namespace jmaerte {
    namespace arith {
/***********************************************************************************************************************
 * NUM
 **********************************************************************************************************************/

        namespace num {

            // INLINE

            ARITHMETIC_EXPORT int   GET_POS(ap_int const n);
            ARITHMETIC_EXPORT int   GET_OCC(ap_int const n);
            ARITHMETIC_EXPORT int   GET_MEMSIZE(ap_int const n);
            ARITHMETIC_EXPORT ULL*  GET_ABS_DATA(ap_int const n);
            ARITHMETIC_EXPORT bool  GET_SIGN(ap_int const n);
            ARITHMETIC_EXPORT int   GET_SIZE(ap_int const n);

            ARITHMETIC_EXPORT void  SET_POS(ap_int n, int pos);
            ARITHMETIC_EXPORT void  SET_SIZE(ap_int n, int size);
            ARITHMETIC_EXPORT void  SET_OCC(ap_int n, int occ);
            ARITHMETIC_EXPORT void  SET_SIGN(ap_int n, bool sign);
            ARITHMETIC_EXPORT void  SWITCH_SIGN(ap_int n);
            ARITHMETIC_EXPORT void  ASSIGN(ap_int a, ap_int const& b);

            ARITHMETIC_EXPORT void  ENLARGE(ap_int& n, int size);
            ARITHMETIC_EXPORT void  ENLARGE(ap_int& n);

            // NOT INLINE

            ARITHMETIC_EXPORT ap_int   PREPARE_SMOD_DIVISOR(ap_int divisor, ap_int denominators, int arr_size);
            ARITHMETIC_EXPORT ap_int   PREPARE_NUM_SVOBODA(ap_int const n);
        }

/***********************************************************************************************************************
 * NUM - COMPARATORS
 **********************************************************************************************************************/

        namespace num {

            // NOT INLINE

            ARITHMETIC_EXPORT int COMPARE_ABS(ap_int const n_a, ap_int const n_b);
        }

/***********************************************************************************************************************
 * NUM - (DE)ALLOCATION
 **********************************************************************************************************************/

        namespace num {

            // INLINE

            ARITHMETIC_EXPORT ap_int   NEW(ULL initial_size, bool sign, ULL value);
            ARITHMETIC_EXPORT ap_int   NEW(ULL* value, int size, int occ, bool sign);
            ARITHMETIC_EXPORT ap_int   COPY(ap_int const n);
            ARITHMETIC_EXPORT void  DELETE_DATA(ap_int i);
            ARITHMETIC_EXPORT void  DELETE(ap_int i);
        }

/***********************************************************************************************************************
 * NUM - ARITHMETIC
 **********************************************************************************************************************/

        namespace num {

            // NOT INLINE

            /**
             * Returns lambda * b.
             * @param result
             * @param lambda
             * @param b
             */
            ARITHMETIC_EXPORT ap_int   MUL(const ap_int& lambda, const ap_int& b);

            ARITHMETIC_EXPORT bool  ADD(ap_int& a, ap_int const& b);

            /**
             * Sets b to be b % a.
             * @param a Divisor
             * @param b Dividend
             */
            ARITHMETIC_EXPORT void  SMOD(ap_int a, ap_int const b, ap_int const pre);

            /**
             * SVOBODA
             * @param a
             * @param b
             * @return
             */
            ARITHMETIC_EXPORT ap_int   SDIV(ap_int const a, ap_int const b, ap_int const pre);
        }

/***********************************************************************************************************************
 * VECTOR
 **********************************************************************************************************************/

        namespace vec {

            // INLINE

            ARITHMETIC_EXPORT int   GET_SIZE(s_vec const v);
            ARITHMETIC_EXPORT int   GET_OCC(s_vec const v);

            ARITHMETIC_EXPORT void  SET_OCC(s_vec v, int occ);

            ARITHMETIC_EXPORT num::ap_int   AT(s_vec const v, const int i);
            ARITHMETIC_EXPORT void  SET(s_vec v, int k, num::ap_int const& n);

            ARITHMETIC_EXPORT void  SWITCH_SIGNS(s_vec v);
            ARITHMETIC_EXPORT void  SWAP_VALUES(s_vec& v, int i, int j);

            ARITHMETIC_EXPORT void  ENLARGE(s_vec& v, int size);
            ARITHMETIC_EXPORT void  ENLARGE(s_vec& v);

            // NOT INLINE

            ARITHMETIC_EXPORT int FIND_POS(s_vec const& v, int pos);
            ARITHMETIC_EXPORT void PUT(s_vec v, num::ap_int const& n);
        }

/***********************************************************************************************************************
 * VECTOR - (DE)ALLOCATION
 **********************************************************************************************************************/

        namespace vec {

            // INLINE

            ARITHMETIC_EXPORT s_vec NEW(std::vector<std::pair<ULL, std::pair<bool, ULL>>> vector);
            ARITHMETIC_EXPORT void DELETE(s_vec& v);
            ARITHMETIC_EXPORT void DELETE_POS(s_vec& v, int i);
        }

/***********************************************************************************************************************
 * VECTOR - ARITHMETIC
 **********************************************************************************************************************/

        namespace vec {

            // NOT INLINE

            /** Adds lambda * b to a.
             *
             * @param a destination sparse_vector
             * @param lambda coefficient
             * @param size_lambda number of ULLs needed to represent lambda
             * @param b addend
             */
            ARITHMETIC_EXPORT void  ADD(s_vec& a, int start_a, const num::ap_int& lambda, const s_vec& b, int start_b);
            ARITHMETIC_EXPORT void  MUL(s_vec v, num::ap_int const n);
        }
    }
}


#include "../../src/arithmetic.hpp"

#endif //ANUBIS_SUPERBUILD_OPERATOR_HPP
