//
// Created by jmaerte on 24.03.20.
//

#ifndef ANUBIS_SUPERBUILD_CONSTANTS_HPP
#define ANUBIS_SUPERBUILD_CONSTANTS_HPP

#include <ARITHMETIC_EXPORT.h>

#include <cinttypes>

typedef unsigned long long ULL;
typedef std::uint32_t UL;

namespace jmaerte {
    namespace arith {
        namespace constants {
            extern unsigned int HEAP_FACTORY_ID;


        
            extern float PHI;

            extern const ARITHMETIC_EXPORT ULL base;
            extern const ARITHMETIC_EXPORT ULL L32;

            extern const ARITHMETIC_EXPORT int BYTES_PER_ULL;
            extern const ARITHMETIC_EXPORT int ULL_SIZE;
            extern const ARITHMETIC_EXPORT ULL L;
            extern const ARITHMETIC_EXPORT ULL LL;
            extern const ARITHMETIC_EXPORT ULL H;
            extern const ARITHMETIC_EXPORT ULL ULL_MAX;
            extern const ARITHMETIC_EXPORT ULL UL_MAX;
            //extern const ARITHMETIC_EXPORT ULL BASE;
            extern const ARITHMETIC_EXPORT ULL L15;
            extern const ARITHMETIC_EXPORT ULL L30;
            extern const ARITHMETIC_EXPORT ULL UL_LEFTMOST;
        }
        namespace num {
            extern const ARITHMETIC_EXPORT ULL SIGN;
            extern const ARITHMETIC_EXPORT ULL SINGLE;
            extern const ARITHMETIC_EXPORT ULL POS;
            extern const ARITHMETIC_EXPORT ULL SIZE;
            extern const ARITHMETIC_EXPORT ULL OCC;
            extern const ARITHMETIC_EXPORT ULL MASKS[2];

            extern const ARITHMETIC_EXPORT int SIGN_SHIFT;
            extern const ARITHMETIC_EXPORT int SINGLE_SHIFT;
            extern const ARITHMETIC_EXPORT int POS_SHIFT;
            extern const ARITHMETIC_EXPORT int SIZE_SHIFT;
            extern const ARITHMETIC_EXPORT int OCC_SHIFT;
            
        }

        namespace vec {
            //extern const ARITHMETIC_EXPORT ULL ALL_SINGLE;
            extern const ARITHMETIC_EXPORT ULL FACTORY_ID;
            extern const ARITHMETIC_EXPORT ULL SIZE;
            extern const ARITHMETIC_EXPORT ULL OCC;

            extern const ARITHMETIC_EXPORT int FACTORY_ID_SHIFT;
            extern const ARITHMETIC_EXPORT int SIZE_SHIFT;
            extern const ARITHMETIC_EXPORT int OCC_SHIFT;
        }
    }
}


#endif //ANUBIS_SUPERBUILD_CONSTANTS_HPP
