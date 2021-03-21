//
// Created by jmaerte on 23.03.20.
//

#include "../include/arithmetic/constants.hpp"
#include "arithmetic/factory/factory.hpp"
#include "arithmetic/operator.hpp"
#include "arithmetic/factory/allocators.hpp"

#include <math.h>

typedef unsigned long long ULL;
typedef std::uint32_t UL;

namespace jmaerte {
    namespace arith {
        namespace constants {
        
            float PHI = (1 + sqrt(5)) / 2;

            unsigned int HEAP_FACTORY_ID = vec::factory::REGISTER<jmaerte::arith::vec::std_factory>();

            const ULL base = 1ULL << 32;
            const ULL L32 = (1ULL << 32) - 1;

            const int BYTES_PER_ULL = sizeof(ULL);
            const int ULL_SIZE = 8 * BYTES_PER_ULL;
            const ULL L = (1ULL << (ULL_SIZE / 2)) - 1;
            const ULL LL = (1ULL << (ULL_SIZE / 4)) - 1;
            const ULL H = L << (ULL_SIZE / 2);
            const ULL ULL_MAX = ~0ULL;
            const ULL UL_MAX = (~0ULL) >> 32;
            //const ULL BASE = 1ULL << 32;
            const ULL L15 = ((1ULL << 15) - 1);
            const ULL L30 = (L >> 2);
            const ULL UL_LEFTMOST = 1ULL << 31;

            const int ABS_SHIFT = sizeof(long long) * 8 - 1;
        }

        namespace num {

            // CONSTANTS FOR MULTI NUMBERS

            const ULL SIGN = 1ULL << 1; // 0....010
            const ULL SINGLE = 1ULL; // 0....01
            const ULL POS = constants::H; // 1....10....0
            const ULL SIZE = constants::L15 << 2; // 0....01....100
            const ULL OCC = constants::L15 << 17; // 0...01...10...0

            const ULL MASKS[] = {
                    constants::H, constants::L
            };


            const int SIGN_SHIFT = 1;
            const int SINGLE_SHIFT = 0;
            const int POS_SHIFT = 32;
            const int SIZE_SHIFT = 2;
            const int OCC_SHIFT = 17;
        }

        namespace vec {
            //const ULL ALL_SINGLE_MASK = 1ULL; // 0...01
            const ULL FACTORY_ID = 15ULL; // 0...01111
            const ULL SIZE = constants::L30 << 4; // 0...01...10000
            const ULL OCC = constants::L30 << 34; // 1...10...0

            const int FACTORY_ID_SHIFT = 0;
            const int SIZE_SHIFT = 4;
            const int OCC_SHIFT = 34;
        }
    }
}