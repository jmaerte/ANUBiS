//
// Created by jmaerte on 16.03.20.
//

#ifndef ANUBIS_ARITHMETIC_SUPERBUILD_OPERATOR_PUBLIC_HPP
#define ANUBIS_ARITHMETIC_SUPERBUILD_OPERATOR_PUBLIC_HPP

#include "typedef.hpp"
#include <vector>
#include <tuple>
#include <iostream>
#include <ARITHMETIC_EXPORT.h>
#include <string>
#include <chrono>

namespace jmaerte {
    namespace arith {
/***********************************************************************************************************************
 * NUM
 **********************************************************************************************************************/

        ARITHMETIC_EXPORT extern bool PRINT;
        ARITHMETIC_EXPORT extern double ELAPSED;

        namespace num {

            // INLINE
            static std::string STRINGIFY(ap_int const n);

            static unsigned int   GET_POS(ap_int const n);
            static unsigned int   GET_OCC(ap_int const n);
            static ULL*  ABS(ap_int const n);
            static bool  GET_SIGN(ap_int const n);
            static bool  IS_NA(ap_int a);
            static bool  IS_SINGLE(ap_int const n);
            static unsigned int   GET_SIZE(ap_int const n);

            static void  SET_POS(ap_int n, int pos);
            static void  SET_SIZE(ap_int n, int size);
            static void  SET_OCC(ap_int n, int occ);
            static void  MAKE_MULTI(ap_int a, int initial_size);
            static void  MAKE_SINGLE(ap_int a);
            static void  SET_SIGN(ap_int n, bool sign);
            static void  SWITCH_SIGN(ap_int n);

            static void  ASSIGN(ap_int a, ap_int const& b);
            static void  OVERWRITE(ap_int a, ap_int const& b);

            static void  ENLARGE(ap_int n, int size);
            static void  ENLARGE(ap_int n);

            // NOT INLINE
            ARITHMETIC_EXPORT void  RSHIFT(ap_int n, int shift);
            ARITHMETIC_EXPORT void  LSHIFT(ap_int n, int shift);
            ARITHMETIC_EXPORT void  LSHIFT_BLOCK(ap_int n, int blocks);
        }

/***********************************************************************************************************************
 * NUM - COMPARATORS
 **********************************************************************************************************************/

        namespace num {

            // NOT INLINE
            ARITHMETIC_EXPORT int COMPARE_ABS(ap_int const n_a, ap_int const n_b);
            ARITHMETIC_EXPORT bool COMPARE(ap_int const a, ap_int const b);
            ARITHMETIC_EXPORT bool IS_ZERO(ap_int const a);

        }

/***********************************************************************************************************************
 * NUM - (DE)ALLOCATION
 **********************************************************************************************************************/

        namespace num {

            // INLINE
            static ap_int    NEW(ULL initial_size, bool sign, ULL value);
            static ap_int    NEW(bool sign, ULL value);
            static void      NEW(ap_int dest, bool sign, ULL value);
            static ap_int    NEW(ULL* value, ULL size, ULL occ, bool sign);
            static ap_int    COPY(ap_int const n);
            static void      DELETE_DATA(ap_int i);
            static void      DELETE(ap_int i);
        }

/***********************************************************************************************************************
 * NUM - ARITHMETIC
 **********************************************************************************************************************/

        namespace num {

            // INLINE
            static void      ADD(ap_int a, ap_int b);
            static void      SUB(ap_int a, ap_int b);
            static void      MOD(ap_int a, ap_int b, ap_int R_POW, ap_int inv);
            static ap_int    iMUL(ap_int a, ap_int b);

            // NOT INLINE
            /**
             * Returns lambda * b.
             * @param result
             * @param lambda
             * @param b
             */
            ARITHMETIC_EXPORT ap_int    iMUL(ap_int a, ap_int b);
            ARITHMETIC_EXPORT void      iMUL(ap_int dest, ap_int a, ap_int b);
            ARITHMETIC_EXPORT void      MUL(ap_int a, ap_int b);
            ARITHMETIC_EXPORT ap_int    iMULL(ap_int a, ap_int b, int n);
            ARITHMETIC_EXPORT void      MULL(ap_int a, ap_int b, ULL n);
            ARITHMETIC_EXPORT void      MULH(ap_int a, ap_int b, ULL n);
            ARITHMETIC_EXPORT void      SQR(ap_int a);
            ARITHMETIC_EXPORT ap_int    iSQR(ap_int a);

            ARITHMETIC_EXPORT ap_int    DIV(ap_int a, ap_int b);
            ARITHMETIC_EXPORT ULL       C_DIV(ap_int a, ULL b);
            ARITHMETIC_EXPORT ap_int    iC_DIV(ap_int a, ULL b);
        }

/***********************************************************************************************************************
 * VECTOR
 **********************************************************************************************************************/

        namespace vec {

            // INLINE
            static int   GET_SIZE(s_ap_int_vec v);
            static int   GET_OCC(s_ap_int_vec v);

            static unsigned int GET_FACTORY_ID(s_ap_int_vec v);

            static bool  GET_IS_ALL_SINGLE(s_ap_int_vec vec);

            static void  SET_OCC(s_ap_int_vec v, int occ);
            static void  SET_SIZE(s_ap_int_vec v, int size);

            static void  SET_FACTORY_ID(s_ap_int_vec v, unsigned int id);

            static num::ap_int   AT(s_ap_int_vec v, const int i);
            static void          SET(s_ap_int_vec v, int k, num::ap_int n);

            static void  SWITCH_SIGNS(s_ap_int_vec v);
            static void  SWAP_VALUES(s_ap_int_vec v, int i, int j);

            static void  ENLARGE_RANGE(s_ap_int_vec& v, int size, int copy_range);
            static void  ENLARGE_RANGE(s_ap_int_vec& v, int copy_range);
            static void  ENLARGE(s_ap_int_vec& v, int size);
            static void  ENLARGE(s_ap_int_vec& v);

            // NOT INLINE
            ARITHMETIC_EXPORT int   FIND_POS(s_ap_int_vec v, int pos);
            ARITHMETIC_EXPORT void  PUT(s_ap_int_vec& v, num::ap_int n);
        }

/***********************************************************************************************************************
 * VECTOR - (DE)ALLOCATION
 **********************************************************************************************************************/

        namespace vec {

            namespace factory {
                template<typename factory_type>
                static unsigned int REGISTER();
                static void RELEASE(unsigned int id);
            }

            // INLINE
            static s_ap_int_vec NEW(unsigned int factory_id, std::vector<std::pair<ULL, std::pair<bool, ULL>>> vector);
            static void DELETE(s_ap_int_vec& v);
            static s_ap_int_vec COPY(unsigned int factory_id, s_ap_int_vec v);
            static void DELETE_POS(s_ap_int_vec v, int i);

        }

/***********************************************************************************************************************
 * VECTOR - ARITHMETIC
 **********************************************************************************************************************/

        namespace vec {

            // INLINE

            static void  REDUCE(s_ap_int_vec& a, s_ap_int_vec trivial, int i);

            // NOT INLINE

            /** Adds lambda * b to a.
             *
             * @param a destination sparse_vector
             * @param lambda coefficient
             * @param size_lambda number of ULLs needed to represent lambda
             * @param b addend
             */
            ARITHMETIC_EXPORT void  ADD(s_ap_int_vec& a, int start_a, num::ap_int lambda, s_ap_int_vec b, int start_b);
            ARITHMETIC_EXPORT void  ADD_ALL_SINGLE(s_ap_int_vec& a, int start_a, num::ap_int lambda_num, s_ap_int_vec b, int start_b);
            ARITHMETIC_EXPORT void  ADD_SINGLE(s_ap_int_vec& a, int start_a, num::ap_int lambda, s_ap_int_vec b, int start_b);
            ARITHMETIC_EXPORT void  ADD_SINGLE_SCALAR(s_ap_int_vec& a, int start_a, num::ap_int lambda, s_ap_int_vec b, int start_b);
            ARITHMETIC_EXPORT void  MUL(s_ap_int_vec v, num::ap_int const n);
            ARITHMETIC_EXPORT void  MOD(s_ap_int_vec v);
        }
    }
}

#include "../../src/operator.hpp"
#include "../../src/arithmetic.hpp"

#endif //ANUBIS_ARITHMETIC_SUPERBUILD_OPERATOR_PUBLIC_HPP
