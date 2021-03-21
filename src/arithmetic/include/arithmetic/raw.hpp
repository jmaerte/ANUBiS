//
// Created by jmaerte on 04.12.20.
//

#ifndef ANUBIS_SUPERBUILD_AUX_HPP
#define ANUBIS_SUPERBUILD_AUX_HPP

#include "arithmetic/typedef.hpp"
#include <cinttypes>
#include <boost/thread.hpp>
#include <vector>
#include <map>

namespace jmaerte {
    namespace arith {
        namespace num {
            namespace aux {

                extern boost::mutex allocation_mutex;
                extern int REC_DIV_THRESHOLD, REC_MUL_THRESHOLD, REC_MUL33_THRESHOLD;//, REC_MUL55_THRESHOLD;
                extern int DIVISOR_LIMIT;

                extern std::vector<std::vector<UL>> V33;
                extern std::vector<std::vector<UL>> V55;
                //extern std::map<unsigned int, std::vector<std::vector<UL>>> REC_MUL_VANDERMONDE;

                std::string STRINGIFY(UL* a, int length);
                void    INCREMENT_COUNTERS(int a, int u, int f);
                void    PRINT_COUNTERS();

                void    STRIP(const UL* dat, unsigned int* occ);
                int     COMPARE_RAW(const UL* a, const UL* b, int occ);

                void    MAKE_MULTI_OVERFLOW(ap_int& n, UL overflow);
                void    MAKE_MULTI_SIZE(ap_int& n, std::size_t size);
                void    MAKE_SINGLE(ap_int& n);
                void    ENLARGE(ap_int& n);
                void    ENLARGE(ap_int& n, int length);

                bool    DECREMENT(UL* a, int n);
                void    INCREMENT(UL* a, int n);
                UL      SUB_RAW(UL* dest, UL* a, int n, UL* b, int m, UL carry);
                UL      ADD_RAW(UL* dest, UL* a, int n, UL* b, int m, UL carry);
                bool    CMP(UL* a, int n, UL* b, int m);

                void    ADD(ap_int& a, ap_int const& b);
                void    C_ADD(ap_int& a, ap_int const& b);

                void    SUB(ap_int& a, ap_int const& b);
                void    C_SUB(ap_int& a, ap_int const& b);

                void    KARATSUBA(ap_int& res, ap_int const& a, ap_int const& b);
                namespace toom {
                    extern "C" void    REC_MUL(UL* dest, UL* a, int n, UL* b, int m);
                }

                extern "C" int     i_LC(UL* dest, long long lambda, UL* a, int n, long long mu, UL* b, int m);
                void    LEHMER_SWAP(num::ap_int& a, num::ap_int& b, long long A, long long B, long long C, long long D);

                extern "C" void    SHIFT_RIGHT(UL* v, int n, int words, int s);
                extern "C" void    SHIFT_LEFT(UL* v, int n, int words, int s);

                extern "C" void    BASECASE_32_DIV(UL* dest, UL* u, int size_u, UL* v, int n);
                extern "C" void    BASECASE_21_DIV(UL* dest, UL* u, int size_u, UL* v, int n);

                extern "C" void    DIV(UL* dest, UL* a, int m, UL* b, int n, int s);
                
            }
        }
    }
}

#endif //ANUBIS_SUPERBUILD_AUX_HPP
