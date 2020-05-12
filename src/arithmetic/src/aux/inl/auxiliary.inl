//
// Created by jmaerte on 23.03.20.
//

#ifndef ANUBIS_SUPERBUILD_AUXILIARY_INL
#define ANUBIS_SUPERBUILD_AUXILIARY_INL

#include "../../operator.hpp"
#include "../hardware/hardware.hpp"
#include <cstring>

namespace jmaerte {
    namespace arith {
        namespace num {
            namespace aux {
/***********************************************************************************************************************
 * NUM - AUX
 **********************************************************************************************************************/

                static inline void STRIP(ULL* a, ULL& occ) {
                    const ULL* i = a + occ - 1;
                    for (; occ > 0; occ--) {
                        if (*(a + occ - 1)) return;
                    }
                }

                static inline ULL GET(ap_int n, int pos) {
                    if (pos < 0 || pos >= GET_OCC(n)) return 0ULL;
                    return *(ABS(n) + pos);
                }

                static inline ULL GET_LEADING(ap_int const n) {
                    return *(ABS(n) + GET_OCC(n) - 1);
                }

                static inline void REMOVE(ap_int n, int i) {
                    SET(n, i, 0ULL);
                }

                static inline ULL ADD_DATA_RANGE(ULL* a, ULL* b, ULL* end) {
                    ULL v_b;
                    ULL carry = 0ULL;
                    for ( ; a != end; ) {
//                        std::cout << "a: " << *a << ", b: " << *b << ", carry: " << carry << std::endl;
                        v_b = *b++ + carry;
                        carry = (v_b < carry);
                        *a += v_b;
//                        std::cout << "a_sum = " << *a;
                        carry += (*a++ < v_b);
//                        std::cout << ", carry: " << carry << std::endl;
                    }
                    return carry;
                }

                /**
                 * Performs subtract on two ranges of ULLs and returns the last carry.
                 * @param a
                 * @param b
                 * @param end
                 * @return
                 */
                static inline ULL SUB_DATA_RANGE(ULL*& a, const ULL* b, bool sign, ULL* end) {
                    ULL v_b;
                    ULL v_a;
                    ULL carry = 0ULL;
                    for ( ; a != end; ) {
//                        std::cout << "a: " << *a << ", b: " << *b << ", carry: " << carry << ", sign: " << sign << std::endl;
                        v_b = *b++;
                        v_a = *a;
                        if (sign) {
                            v_a += carry; // v_a or v_b += carry?
                            carry = (v_a < carry || v_b < v_a); // this is actually exclusive due to carry \in \{0,1\}
                            *a++ = v_b - v_a;
//                            std::cout << "a_diff: " << *(a - 1) << ", borrow: " << carry << std::endl;
                        } else {
                            v_b += carry;
                            carry = (v_b < carry || v_a < v_b);
                            *a++ = v_a - v_b;
//                            std::cout << "a_diff: " << *(a - 1) << ", borrow: " << carry << std::endl;
                        }
                    }
                    return carry;
                }

                /**
                 * Truncates an ap integer at place n. The integer itself becomes the low part and the high part
                 * gets returned.
                 */
                static inline ap_int TRUNCATE(ap_int a, ULL n) {
                    ap_int hi = NEW(GET_OCC(a) - n, GET_SIGN(a), 0ULL);
                    std::memcpy(ABS(a) + n, ABS(hi), GET_OCC(a) - n);
                    (a + 1)->value = (ULL*) realloc(ABS(a), n * constants::BYTES_PER_ULL);
                    STRIP(ABS(a), n);
                    SET_OCC(a, n);
                }
            }
        }
    }
}

#endif //ANUBIS_SUPERBUILD_AUXILIARY_INL
