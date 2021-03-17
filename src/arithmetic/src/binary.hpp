//
// Created by jmaerte on 06.12.20.
//

#ifndef ANUBIS_SUPERBUILD_UTIL_HPP
#define ANUBIS_SUPERBUILD_UTIL_HPP

#include <cstdint>

namespace jmaerte {
    namespace arith {
        namespace util {
            static const char table[256] = {
                    #define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
                    -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
                    LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
                    LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
            };

            static inline int LOG2(std::uint32_t v) {
                register unsigned int t, tt;
                if (tt = v >> 16) {
                    return (t = tt >> 8) ? 24 + table[t] : 16 + table[tt];
                } else {
                    return (t = v >> 8) ? 8 + table[t] : table[v];
                }
            }
        }
    }
}

#endif //ANUBIS_SUPERBUILD_UTIL_HPP
