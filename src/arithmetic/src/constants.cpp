//
// Created by jmaerte on 23.03.20.
//

#include "constants.hpp"

typedef unsigned long long ULL;

const int ULL_SIZE = ;
// first 32 bits
const ULL L_MASK = ;
// first 16 bits
const ULL LL_MASK = ;
// last 32 bits
const ULL H_MASK = ;
const ULL NUM_SIGN_MASK = ;
// last 32 bits without the most significant
const ULL NUM_POS_MASK = ;
const ULL NUM_SIZE_MASK = ;
const ULL NUM_OCC_MASK = ;
const ULL FIFTEEN = LL_MASK & ~(1u << 15);
const ULL
const ULL ULL_MAX = ;
const ULL UI_MAX = ;
const ULL BASE = ;


namespace jmaerte {
    namespace arithmetic {
        namespace constants {
            const int BYTES_PER_ULL = sizeof(ULL);
            const int ULL_SIZE = 8 * BYTES_PER_ULL;
            const ULL L_MASK = (1ULL << (ULL_SIZE / 2)) - 1;
            const ULL LL_MASK = (1ULL << (ULL_SIZE / 4)) - 1;
            const ULL H_MASK = L_MASK << (ULL_SIZE / 2);
            const ULL ULL_MAX = ~0ULL;
            const ULL UI_MAX = (~0ULL) >> 32;
            const ULL BASE = 1ULL << 32;
            const ULL FIFTEEN = L_MASK ^ (1ULL << 15);
        }

        namespace num {
            namespace constants {
                const ULL SIGN_MASK 1ULL << 1;
                const ULL SINGLE_MASK = 1ULL;
                const ULL POS_MASK = H_MASK;
                const ULL SIZE_MASK = FIFTEEN << 2;
                const ULL OCC_MASK = FIFTEEN << 17;

                const ULL MASKS[] = {
                    H_MASK, L_MASK
                };
            }
        }

        namespace vec {
            namespace constants {
                const ULL ALL_SINGLE_MASK = 1ULL;
                const ULL SIZE_MASK = L_MASK << 1;
                const ULL OCC_MASK = L_MASK << 33;
            }
        }
    }
}