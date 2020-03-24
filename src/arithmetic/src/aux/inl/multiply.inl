//
// Created by jmaerte on 23.03.20.
//

#ifndef ANUBIS_SUPERBUILD_MULTIPLY_INL
#define ANUBIS_SUPERBUILD_MULTIPLY_INL

#include "../hardware/hardware.hpp"
#include "../../operator.hpp"
#include "../../arithmetic.hpp"

namespace jmaerte {
    namespace arith {
        namespace num {
            namespace aux {

                static inline ULL aux_mul(ULL* result, ULL val, ULL* a, int l_a, ULL* prod) {
                    ULL carry;
                    int j = 0;
                    for (ULL* it = a; it != a + l_a; it++, j++) {
                        set_mul(prod, val, *it);

                        *prod += carry;

                        *(result + j) += *prod;
                        carry = *(prod + 1) + (*prod < carry) + (*(result + j) < *prod); // never overflows twice, i.e. the conditions are excluding each other
                    }
                    *(result + l_a) = carry;
                    return carry;
                }

                static inline void CMUL(ap_int a, ULL b) {
                    int n = GET_OCC(a);
                    ULL carry = 0ULL;
                    ULL* prod = new ULL[2];
                    for (int i = 0; i < n; i++) {
                        set_mul(prod, GET(a, i), b);

                        *prod += carry;
                        carry = *(prod + 1) + (*prod < carry);
                        SET(a, i, *prod);
                    }
                    delete[] prod;
                }

            }
        }
    }
}

#endif //ANUBIS_SUPERBUILD_MULTIPLY_INL
