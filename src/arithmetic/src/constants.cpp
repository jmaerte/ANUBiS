//
// Created by jmaerte on 23.03.20.
//

#include "../include/arithmetic/constants.hpp"

typedef unsigned long long ULL;

namespace jmaerte {
    namespace arith {
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
            const ULL SIGN_MASK = 1ULL << 1;
            const ULL SINGLE_MASK = 1ULL;
            const ULL POS_MASK = constants::H_MASK;
            const ULL SIZE_MASK = constants::FIFTEEN << 2;
            const ULL OCC_MASK = constants::FIFTEEN << 17;

            const ULL MASKS[] = {
                    constants::H_MASK, constants::L_MASK
            };
        }

        namespace vec {
            const ULL ALL_SINGLE_MASK = 1ULL; // 0...01
            const ULL FACTORY_ID_MASK = 7ULL << 1; // 0...01110
            const ULL SIZE_MASK = (constants::L_MASK >> 2) << 4; // 0...01...10000
            const ULL OCC_MASK = (constants::L_MASK >> 2) << 34; // 1...10...0
        }
    }
}