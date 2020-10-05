//
// Created by jmaerte on 23.03.20.
//

#ifndef ANUBIS_SUPERBUILD_MODULAR_INL
#define ANUBIS_SUPERBUILD_MODULAR_INL

#include "../hardware/hardware.hpp"

static const unsigned long long t[6] = {
        0xFFFFFFFF00000000ull,
        0x00000000FFFF0000ull,
        0x000000000000FF00ull,
        0x00000000000000F0ull,
        0x000000000000000Cull,
        0x0000000000000002ull
};

namespace jmaerte {
    namespace arith {
        namespace num {
            namespace aux {
                namespace modular {
                    // ceils log2(x)
                    static inline int LOG2(ULL x) {
                        int y = (((x & (x - 1)) == 0) ? 0 : 1);
                        int j = 32;
                        int i;

                        for (i = 0; i < 6; i++) {
                            int k = (((x & t[i]) == 0) ? 0 : j);
                            y += k;
                            x >>= k;
                            j >>= 1;
                        }

                        return y;
                    }

                    static inline int TRAILING_ZEROS(ULL v) {
                        int c = 64;
                        v &= -signed(v);
                        if (v) c--; // if there is a one, subtract one from the maximum zeros already.
                        if (v & 0x0000'0000'FFFF'FFFFull) c -= 32;
                        if (v & 0x0000'FFFF'0000'FFFFull) c -= 16;
                        if (v & 0x00FF'00FF'00FF'00FFull) c -= 8;
                        if (v & 0x0F0F'0F0F'0F0F'0F0Full) c -= 4;
                        if (v & 0x3333'3333'3333'3333ull) c -= 2;
                        if (v & 0x5555'5555'5555'5555ull) c--;
                        return c;
                    }
                }
            }
        }
    }
}

#endif //ANUBIS_SUPERBUILD_MODULAR_INL
