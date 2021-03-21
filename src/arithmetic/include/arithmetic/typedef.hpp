//
// Created by jmaerte on 16.03.20.
//

#ifndef ANUBIS_ARITHMETIC_SUPERBUILD_TYPEDEF_HPP
#define ANUBIS_ARITHMETIC_SUPERBUILD_TYPEDEF_HPP

#include <ARITHMETIC_EXPORT.h>
#include <cinttypes>

typedef unsigned long long ULL;
typedef std::uint32_t UL;

/**
 * This union has 64 bits. It can serve either as an pointer to a memory block of ULLs or as an ULL of meta-data itself.
 */
namespace jmaerte {
    namespace arith {

        /**
         * A number is a memory block of two 64 bit integers:
         *
         *  64      64
         * +-------+---------+
         * | meta  | value/s |
         * +-------+---------+
         *
         * where meta is of the form
         *
         *  32    15     15     1       1
         * +-----+------+-----+------+--------+
         * | pos | size | occ | sign | single |
         * +-----+------+-----+------+--------+
         *
         * and value points to an array of size @code{size}-many ULLs, where the first @code{occ} 32-bit block elements
         * are a word of the expansion of the number in base 2^32.
         */

        struct num_node {
            ULL meta;

            union {
                UL value;
                UL* values;
            };
        };


        /**
         * A sparse vector is a memory block of 2 64 bit integers.
         *
         *
         *  64      64
         * +-------+--------+
         * | meta  | values |
         * +-------+--------+
         *
         * Where values points to an array of num_nodes of size "size". The degree of fill-in is
         * saved in occ and thus we always have to preserve that occ <= size.
         *
         *  30     30
         * +------+-----+------------+
         * | size | occ | factory_id |
         * +------+-----+------------+
         *
         * where factory_id is the id of the factory that produces that vector and holds its pointer reference.
         *
         */

        struct svec {
            ULL meta;
            num_node* values;
        };
    }
}

namespace jmaerte {
    namespace arith {
        namespace num {
            typedef num_node ap_int;
        }
    }
}

namespace jmaerte {
    namespace arith {
        namespace vec {
            typedef svec s_ap_int_vec;
            typedef svec s_ap_float_vec;
        }
    }
}

namespace jmaerte {
    namespace arith {
        /**
        * An instance of the current state of arithmetics, i.e. this changes when an overflow occurs.
        */
        class context {
        public:
            virtual bool i_mul(num::ap_int& curr, num::ap_int& a, num::ap_int& b) = 0;
            virtual bool add(num::ap_int& a, num::ap_int& b) = 0;
            virtual bool mul(num::ap_int& a, num::ap_int& b) = 0;
            virtual bool gcd(num::ap_int& s, num::ap_int& t, num::ap_int& q_a, num::ap_int& q_b, num::ap_int& a, num::ap_int& b) = 0;
            virtual bool is_single(num::ap_int const& a) = 0;

            virtual bool vec_reduce(vec::s_ap_int_vec& v, vec::s_ap_int_vec& trivial, int k) = 0;
            virtual bool vec_add(vec::s_ap_int_vec& a, int start_a, const num::ap_int& coeff, vec::s_ap_int_vec& b, int start_b, int copy_a) = 0;
            virtual bool vec_combine(vec::s_ap_int_vec& a, vec::s_ap_int_vec& b, num::ap_int& lambda_a, num::ap_int& tau_a, num::ap_int& lambda_b, num::ap_int& tau_b, int start_a, int start_b, int copy_a, int copy_b) = 0;
        };
    }
}

namespace jmaerte {
    namespace arith {
        namespace num {
            namespace comp {
                struct ARITHMETIC_EXPORT SIGNED_COMPARATOR {
                        bool operator()(ap_int const& a, ap_int const& b);
                };

                struct ARITHMETIC_EXPORT UNSIGNED_COMPARATOR {
                        bool operator()(ap_int const& a, ap_int const& b);
                };
            }
        }
    }
}

#endif //ANUBIS_SUPERBUILD_TYPEDEF_HPP
