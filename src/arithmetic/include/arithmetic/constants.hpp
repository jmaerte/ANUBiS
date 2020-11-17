//
// Created by jmaerte on 24.03.20.
//

#ifndef ANUBIS_SUPERBUILD_CONSTANTS_HPP
#define ANUBIS_SUPERBUILD_CONSTANTS_HPP

#include <ARITHMETIC_EXPORT.h>

typedef unsigned long long ULL;

namespace jmaerte {
    namespace arith {
        namespace constants {
            extern unsigned int HEAP_FACTORY_ID;

            extern const ARITHMETIC_EXPORT int BYTES_PER_ULL;
            extern const ARITHMETIC_EXPORT int ULL_SIZE;
            extern const ARITHMETIC_EXPORT ULL L_MASK;
            extern const ARITHMETIC_EXPORT ULL LL_MASK;
            extern const ARITHMETIC_EXPORT ULL H_MASK;
            extern const ARITHMETIC_EXPORT ULL ULL_MAX;
            extern const ARITHMETIC_EXPORT ULL UI_MAX;
            extern const ARITHMETIC_EXPORT ULL BASE;
            extern const ARITHMETIC_EXPORT ULL FIFTEEN;
        }
        namespace num {
            extern const ARITHMETIC_EXPORT ULL SIGN_MASK;
            extern const ARITHMETIC_EXPORT ULL SINGLE_MASK;
            extern const ARITHMETIC_EXPORT ULL POS_MASK;
            extern const ARITHMETIC_EXPORT ULL SIZE_MASK;
            extern const ARITHMETIC_EXPORT ULL OCC_MASK;
            extern const ARITHMETIC_EXPORT ULL MASKS[2];
        }

        namespace vec {
            extern const ARITHMETIC_EXPORT ULL ALL_SINGLE_MASK;
            extern const ARITHMETIC_EXPORT ULL FACTORY_ID_MASK;
            extern const ARITHMETIC_EXPORT ULL SIZE_MASK;
            extern const ARITHMETIC_EXPORT ULL OCC_MASK;
        }
    }
}


#endif //ANUBIS_SUPERBUILD_CONSTANTS_HPP
