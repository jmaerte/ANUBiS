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

/***********************************************************************************************************************
 * NUM
 **********************************************************************************************************************/

// INLINE
void ADD_NUM(num a, ULL lambda, int shift, num const b);
void SUB_NUM(num a, ULL lambda, int shift, num const b);

ARITHMETIC_EXPORT int GET_NUM_POS(num const n);
ARITHMETIC_EXPORT int GET_NUM_OCC(num const n);
ARITHMETIC_EXPORT int GET_NUM_MEMSIZE(num const n);
ARITHMETIC_EXPORT ULL* GET_ABS_DATA(num const n);
ARITHMETIC_EXPORT bool GET_NUM_SIGN(num const n);
ARITHMETIC_EXPORT int GET_NUM_SIZE(num const n);

ARITHMETIC_EXPORT void SET_NUM_POS(num n, int pos);
ARITHMETIC_EXPORT void SET_NUM_SIZE(num n, int size);
ARITHMETIC_EXPORT void SET_NUM_OCC(num n, int occ);
ARITHMETIC_EXPORT void SET_NUM_SIGN(num n, bool sign);
ARITHMETIC_EXPORT void SWITCH_NUM_SIGN(num n);
ARITHMETIC_EXPORT void SET_NUM(num a, num const& b);

ARITHMETIC_EXPORT void ENLARGE_NUM(num& n, int size);
ARITHMETIC_EXPORT void ENLARGE_NUM(num& n);

// NOT INLINE

ARITHMETIC_EXPORT num PREPARE_SMOD_DIVISOR(num divisor, num denominators, int arr_size);
ARITHMETIC_EXPORT num PREPARE_NUM_SVOBODA(num const n);

/***********************************************************************************************************************
 * NUM - COMPARATORS
 **********************************************************************************************************************/

// NOT INLINE

ARITHMETIC_EXPORT int COMPARE_ABS(num const n_a, num const n_b);

/***********************************************************************************************************************
 * NUM - (DE)ALLOCATION
 **********************************************************************************************************************/

// INLINE

ARITHMETIC_EXPORT num NEW_NUM(ULL initial_size, bool sign, ULL value);
ARITHMETIC_EXPORT num NEW_NUM(ULL* value, int size, int occ, bool sign);
ARITHMETIC_EXPORT num COPY_NUM(num const n);
ARITHMETIC_EXPORT void DEL_NUM_DATA(num i);
ARITHMETIC_EXPORT void DEL_NUM(num i);

/***********************************************************************************************************************
 * NUM - ARITHMETIC
 **********************************************************************************************************************/

// NOT INLINE

/**
 * Returns lambda * b.
 * @param result
 * @param lambda
 * @param b
 */
ARITHMETIC_EXPORT num MUL(const num& lambda, const num& b);

ARITHMETIC_EXPORT bool ADD_NUM(num& a, num const& b);

/**
 * Sets b to be b % a.
 * @param a Divisor
 * @param b Dividend
 */
ARITHMETIC_EXPORT void SMOD(num a, num const b, num const pre);

/**
 * SVOBODA
 * @param a
 * @param b
 * @return
 */
ARITHMETIC_EXPORT num SDIV(num const a, num const b, num const pre);

/***********************************************************************************************************************
 * VECTOR
 **********************************************************************************************************************/

// INLINE

ARITHMETIC_EXPORT int GET_VEC_SIZE(s_vec const v);
ARITHMETIC_EXPORT int GET_VEC_OCC(s_vec const v);
ARITHMETIC_EXPORT void SET_VEC_OCC(s_vec v, int occ);
ARITHMETIC_EXPORT num VEC_AT(s_vec const v, const int i);
ARITHMETIC_EXPORT void ENLARGE_VEC(s_vec& v, int size);
ARITHMETIC_EXPORT void ENLARGE_VEC(s_vec& v);
ARITHMETIC_EXPORT void SWITCH_VEC_SIGN(s_vec v);
ARITHMETIC_EXPORT void VEC_SWAP_VALUES(s_vec& v, int i, int j);
ARITHMETIC_EXPORT void VEC_SET(s_vec v, int k, num const& n);

// NOT INLINE

ARITHMETIC_EXPORT int VEC_FIND_POS(s_vec const& v, int pos);
ARITHMETIC_EXPORT void VEC_PUT(s_vec v, num const& n);

/***********************************************************************************************************************
 * VECTOR - (DE)ALLOCATION
 **********************************************************************************************************************/

// INLINE

ARITHMETIC_EXPORT s_vec NEW_VEC(std::vector<std::pair<ULL, std::pair<bool, ULL>>> vector);
ARITHMETIC_EXPORT void DEL_VEC(s_vec& v);
ARITHMETIC_EXPORT void DEL_POS(s_vec& v, int i);

/***********************************************************************************************************************
 * VECTOR - ARITHMETIC
 **********************************************************************************************************************/

// NOT INLINE

/** Adds lambda * b to a.
 *
 * @param a destination sparse_vector
 * @param lambda coefficient
 * @param size_lambda number of ULLs needed to represent lambda
 * @param b addend
 */
ARITHMETIC_EXPORT void ADD(s_vec& a, int start_a, const num& lambda, const s_vec& b, int start_b);
ARITHMETIC_EXPORT void MUL_VEC(s_vec v, num const n);


#include "../../src/arithmetic.hpp"

#endif //ANUBIS_SUPERBUILD_OPERATOR_HPP
