//
// Created by jmaerte on 23.03.20.
//
#include "../arithmetic.hpp"
#include "../operator.hpp"
#include "hardware/hardware.hpp"
#include <limits>

namespace jmaerte {
    namespace arith {
        namespace num {
            namespace aux {

                /**
                 * Mutable Implementation of Knuth's algorithm D.
                 * @param a
                 * @param b
                 * @return
                 */
                ap_int E_DIV(ap_int a, ap_int b) {
                    // normalize
                    int m = GET_OCC(a);
                    int n = GET_OCC(b);
                    std::cout << n << " " << m << std::endl;

                    ULL* u = num::ABS(a);
                    ULL* v = num::ABS(b);
                    ULL div = v[n-1];
                    int s = 63 - aux::modular::LOG2(div);

                    std::cout << s << " " << div << std::endl;

                    for (int i = n - 1; i > 0; i--) {
                        v[i] = (v[i] << s) | (v[i - 1] >> (64 - s));
                    }
                    v[0] = v[0] << s;
                    num::ENLARGE(a, num::GET_OCC(a) + 1);
                    u[m] = u[m-1] >> (64 - s);
                    for (int i = m - 1; i > 0; i--) {
                        u[i] = (u[i] << s) | (u[i - 1] >> (64 - s));
                    }
                    u[0] = u[0] << s;

                    ap_int quot = num::NEW(m - n + 1, num::GET_SIGN(a) ^ num::GET_SIGN(b), 0ULL);

                    ULL q;
                    ULL rem;
                    ULL ls_mul;
                    ULL overflow;
                    ULL t, k, p;
                    char overflown;
                    for (int j = m - n; j >= 0; j--) {
                        q = udiv(u[j + n], u[j + n - 1], v[n-1], &rem);
                        overflown = 0;
                        do {
                            ls_mul = mul(q, v[n-2], &overflow);
                            if ((q == 0 && u[j + n] != 0) || // the operation overflowed, that is: u[j + n] = MAX_VALUE_ULL
                                (overflow > rem || (overflow == rem && ls_mul > u[j + n - 2]))) {
                                q -= 1;
                                overflown = adc(0, rem, v[n - 1], &rem);
                            }
                        } while (overflown == 0);
                        k = 0;
                        overflown = 0;
                        char o = 0;
                        for (int i = 0; i < n; i++) {
                            p = mul(q, v[i], &overflow);
                            o = adc(0, k, p, &t);
                            overflown = adc(0, u[i + j], - t, &t);

                            u[i + j] = t;
                            k = overflow + (overflown + o - 1);
                        }
                        o = adc(0, u[j + n], -k, &t);
                        u[j + n] = t;

                        num::ABS(quot)[j] = q;
                        if (o == 0) {// if t < 0.
                            num::ABS(quot)[j] -= 1;
                            k = 0;
                            for (int i = 0; i < n; i++) {
                                o = adc(0, v[i], k, &t);
                                o = adc(o, u[i + j], t, &t);
                                u[i + j] = t;
                                k = o;
                            }
                            adc(0, u[j + n], k, u + j + n);
                        }
                    }
                    // de-normalize
                    if (num::GET_SIGN(a)) num::SWITCH_SIGN(a);
                    for (int i = 0; i < n - 1; i++) {
                        u[i] = (u[i] >> s) | (u[i + 1] << (64 - s));
                    }
                    u[n - 1] = u[n-1] >> s;

                    ULL occ = n;
                    STRIP(ABS(a), occ);
                    num::SET_OCC(a, occ);

                    return quot;
                }
            }
        }
    }
}