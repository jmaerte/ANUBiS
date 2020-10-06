//
// Created by jmaerte on 16.03.20.
//

#pragma once

#include "arithmetic/typedef.hpp"
#include <tuple>
#include <utility>
#include <map>

// AUX MIN and MAX functions without branching
static inline int MIN(int x, int y) {
    return y ^ ((x ^ y) & -(x < y));
}

static inline int MAX(int x, int y) {
    return x ^ ((x ^ y) & -(x < y));
}

namespace jmaerte {
    namespace arith {
/***********************************************************************************************************************
 * AUX
 **********************************************************************************************************************/

        namespace num {
            namespace aux {

                // auxiliary.inl
                static void            STRIP(ULL* a, ULL& occ);
                static ULL             GET(ap_int n, int pos);
                static ULL             GET_LEADING(ap_int const n);
                static void            REMOVE(ap_int n, int i);
                static ULL             ADD_DATA_RANGE(ULL* a, ULL* b, ULL* end);
                static ULL             SUB_DATA_RANGE(ULL*& a, const ULL* b, bool sign, ULL* end);
                static ap_int          TRUNCATE(ap_int a, ULL n);
                //auxiliary.cpp
                void     SET(ap_int n, int pos, ULL val);
                void     ADD(ap_int a, ULL lambda, int shift, ap_int const b);
                void     C_ADD(ap_int a, ULL b);
                void     C_SUB(ap_int a, ULL b);
                bool     iC_MUL(ap_int coeff, ap_int a, ap_int* dest);
                bool     iC_A_MUL(ap_int coeff, ap_int a, ap_int* dest);
                void     SUB(ap_int a, ULL lambda, int shift, ap_int const b);
                ap_int   iADD_DATA(ULL* a, int l_a, ULL* b, int l_b);
                int      ADD_DATA(ULL* a, ULL* b, int n, int l);
                bool     SUB_DATA(ULL* a, ULL* b, int n, int l);
                int      COMPARE_RAW(ULL* a, ULL* b, int length);

                // divide.inl
                static ULL             C_DIV(ap_int a, ULL b);
                // divide.cpp
                ap_int   E_DIV(ap_int a, ap_int b);

                // multiply.inl
                static ULL             aux_mul(ULL* result, ULL val, ULL* a, int l_a, ULL* prod);
                static void            CMUL(ap_int a, ULL b);
                // multiply.cpp
                int      REC_MUL_DATA(ULL* a, int l_a, ULL* b, int l_b, int shift);
                int      iREC_MUL_DATA(ULL* result, ULL* a, int l_a, ULL* b, int l_b);
                int      REC_MULL_DATA(ULL* a, int l_a, ULL* b, int l_b, int shift, int m);
                int      REC_MULH_DATA(ULL* a, int l_a, ULL* b, int l_b, int shift, int m);
                int      iREC_MULL_DATA(ULL* result, ULL* a, int l_a, ULL* b, int l_b, int shift, int m);
                int      REC_SQR_DATA(ULL* dat, int n, int shift);
                int      iREC_SQR_DATA(ULL* result, ULL* dat, int n);

                namespace modular {

                    // modular.inl
                    static int             LOG2(ULL x);
                    static int             TRAILING_ZEROS(ULL v);
                    // modular.cpp
                    int      ODDIFY(ap_int N);
                    ap_int   MODINV(ap_int N);
                    void     MODMUL(ap_int a, ap_int b, ap_int N, ap_int N_inv);
                    bool     SREM(ap_int a, ap_int N, ap_int N_inv);
                    ap_int   RADIX_SQ(ap_int N);
                    void     MODSQR(ap_int& a, ap_int N, ap_int inv);
                    void     iMODSQR(ap_int dest, ap_int a, ap_int N, ap_int N_inv);
                    void     MODRED(ap_int& a, ap_int N, ap_int inv);
                    void     iMODRED(ap_int dest, ap_int a, ap_int N, ap_int inv);
                    ap_int   RADIX_POW(ap_int SQ, int n, ap_int N, ap_int N_inv);
                }
            }
        }
    }
}

#include "misc/inl/auxiliary.inl"
#include "misc/inl/divide.inl"
#include "misc/inl/modular.inl"
#include "misc/inl/multiply.inl"


