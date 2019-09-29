//
// Created by maertej on 29.09.19.
//

#include <cstddef>

#ifndef ANUBIS_SHIFTER_HPP
#define ANUBIS_SHIFTER_HPP
namespace anubis {
    class shifter {
        int k;
        int N; // k is the number of current set bits, N the number of
    };

    class shifter_binary {
        std::size_t k;
        std::size_t N;
        unsigned long int curr;
        unsigned long int stop;
        bool checked_once = false;
    public:
        shifter_binary(std::size_t k, std::size_t N): k(k), N(N), curr(0), stop(0) {
            for (int i = 0; i < k; i++) {
                curr |= 1u << i;
                stop |= 1u << (N - i - 1);
            }
        }

        void shift() {
            if (curr == stop) {
                checked_once = true;
                return;
            }
            unsigned int t = curr | (curr - 1);
            curr = (t + 1) | (((~t & -~t) - 1) >> (__builtin_ctz(curr) + 1));
        }
        unsigned int get() {
            return curr;
        }
        bool has_next() {
            return !checked_once;
        }
    };
}
#endif //ANUBIS_SHIFTER_HPP
