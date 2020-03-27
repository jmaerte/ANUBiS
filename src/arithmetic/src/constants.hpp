//
// Created by jmaerte on 24.03.20.
//

#ifndef ANUBIS_SUPERBUILD_CONSTANTS_HPP
#define ANUBIS_SUPERBUILD_CONSTANTS_HPP

typedef unsigned long long ULL;

namespace jmaerte {
    namespace arithmetic {
        namespace constants {
            extern const int BYTES_PER_ULL;
            extern const int ULL_SIZE;
            extern const ULL L_MASK;
            extern const ULL LL_MASK;
            extern const ULL H_MASK;
            extern const ULL ULL_MAX;
            extern const ULL UI_MAX;
            extern const ULL BASE;
            extern const ULL FIFTEEN
        }
        namespace num {

            namespace constants {
                extern const ULL SIGN_MASK;
                extern const ULL SINGLE_MASK;
                extern const ULL POS_MASK;
                extern const ULL SIZE_MASK;
                extern const ULL OCC_MASK;
                extern const ULL MASKS[2];
            }
        }

        namespace vec {
            namespace constants {
                extern const ULL ALL_SINGLE_MASK;
                extern const ULL SIZE_MASK;
                extern const ULL OCC_MASK;
            }
        }
    }
}


#endif //ANUBIS_SUPERBUILD_CONSTANTS_HPP
