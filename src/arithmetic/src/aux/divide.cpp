//
// Created by jmaerte on 23.03.20.
//
#include "../arithmetic.hpp"
#include "../operator.hpp"
#include "hardware/hardware.hpp"

namespace jmaerte {
    namespace arith {
        namespace num {
            namespace aux {

                ap_int E_DIV(ap_int a, ap_int b) {
                    bool sign_a = GET_SIGN(a);
                    bool sign_b = GET_SIGN(b);
                    if (sign_a) SWITCH_SIGN(a);
                    if (sign_b) SWITCH_SIGN(b);
                    // normalization
                    ULL rem;
                    ULL div = aux::GET_LEADING(a) + 1ULL;
                    ULL d;
                    if (div) {
                        d = udiv(1ULL, 0ULL, d, &rem);
                        ENLARGE(b, GET_OCC(b) + 1);
                        CMUL(b, d);
                        CMUL(a, d); // this maybe introduces a new position in a.
                    }
                    int m = GET_OCC(a) - GET_OCC(b);
                    int n = GET_OCC(b);
                    ap_int Q = NEW(m, false, 0ULL);
                    ULL leading = aux::GET_LEADING(b);
                    ULL second = aux::GET(b, GET_OCC(b) - 2);

                    ULL q;
                    ULL* prod_leading = new ULL[2];
                    ULL* prod_second = new ULL[2];

                    ULL curr_a_leading;
                    ULL curr_a_second;
                    for (int j = m; j > 0; j--) {
                        curr_a_leading = aux::GET(a, j);
                        curr_a_second = aux::GET(a, j - 1);
                        if (curr_a_leading == leading) q = ULL_MAX;
                        else q = udiv(curr_a_leading, curr_a_second, leading, &rem);

                        if (DIV_TEST(prod_leading, prod_second, leading, second, q, curr_a_leading, curr_a_second, aux::GET(a, j - 2))) {
                            q--;
                            if (DIV_TEST(prod_leading, prod_second, leading, second, q, curr_a_leading, curr_a_second, aux::GET(a, j - 2))) {
                                q--;
                            }
                        }

                        aux::SUB(a, q, j, b);
                        if (GET_SIGN(a)) {
                            q--;
                            aux::ADD(a, 1ULL, j, b);
                        }
                        aux::SET(Q, j, q);
                    }
                    delete[] prod_leading;
                    delete[] prod_second;
                    // unnormalize
                    if (div) C_DIV(a, d);
                    SET_SIGN(Q, sign_a ^ sign_b);
                    if (sign_a) SWITCH_SIGN(a);
                    if (sign_b) SWITCH_SIGN(b);
                    return Q;
                }
            }
        }
    }
}