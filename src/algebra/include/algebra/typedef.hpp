//
// Created by jmaerte on 24.03.20.
//

#ifndef ANUBIS_ALGEBRA_SUPERBUILD_TYPEDEF_HPP
#define ANUBIS_ALGEBRA_SUPERBUILD_TYPEDEF_HPP

#include <arithmetic/typedef.hpp>
#include <data_types/stream.hpp>

namespace jmaerte {
    namespace algebra {
        using namespace jmaerte::arith;
        using namespace jmaerte::typings;

        typedef stream<vec::s_ap_int_vec> s_int_matrix;
        typedef stream<vec::s_ap_float_vec> s_float_matrix;
    }
}

#endif //ANUBIS_ALGEBRA_SUPERBUILD_TYPEDEF_HPP
