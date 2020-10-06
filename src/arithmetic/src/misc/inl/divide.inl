//
// Created by jmaerte on 23.03.20.
//

#ifndef ANUBIS_SUPERBUILD_DIVIDE_INL
#define ANUBIS_SUPERBUILD_DIVIDE_INL

#include "../hardware/hardware.hpp"
#include "../../operator.hpp"

namespace jmaerte {
    namespace arith {
        namespace num {
            namespace aux {
                /**
                 * Saves the quotient a / b in a.
                 * @param a
                 * @param b
                 * @return
                 */
                static inline ULL C_DIV(ap_int a, ULL b) {
                    int n = GET_OCC(a);
                    ULL remainder = aux::GET_LEADING(a) % b;
                    ULL quot = aux::GET_LEADING(a) / b;
                    aux::SET(a, n - 1, quot);
                    for (int i = n - 2; i >= 0; i--) {
                        quot = udiv(remainder, aux::GET(a, i), b, &remainder);
                        aux::SET(a, i, quot);
                    }
                    return remainder;
                }

                /**
                 * Sets a to be the remainder and returns the quotient
                 * @param a
                 * @param b
                 * @return
                 */
                static inline ap_int iC_DIV(ap_int a, ULL b) {
                    int n = GET_OCC(a);
                    ap_int res = NEW(n, GET_SIGN(a), 0ULL);
                    ULL remainder = aux::GET_LEADING(a) % b;
                    ULL quot = aux::GET_LEADING(a) / b;
                    aux::SET(res, n - 1, quot);
                    for (int i = n - 2; i >= 0; i--) {
                        quot = udiv(remainder, aux::GET(a, i), b, &remainder);
                        aux::SET(res, i, quot);
                    }
                    // replace a by remainder.
                    ap_int x = NEW(GET_SIGN(a), remainder);
                    OVERWRITE(a, x);
                    DELETE(x);
                    return res;
                }

                static inline bool DIV_TEST(ULL* prod_leading, ULL* prod_second, ULL leading, ULL second, ULL q, ULL curr_a_leading, ULL curr_a_second, ULL curr_a_third) {
                    set_mul(prod_leading, leading, q);
                    set_mul(prod_second, second, q);
                    if (*(prod_leading + 1) == curr_a_leading) {
                        if (*(prod_second + 1) > curr_a_second - *prod_leading) return true;
                        else if (*(prod_second + 1) == curr_a_second - *prod_leading) {
                            if (*prod_second > curr_a_third) return true;
                        }
                    }
                    return false;
                }

            }
        }
    }
}

#endif //ANUBIS_SUPERBUILD_DIVIDE_INL
