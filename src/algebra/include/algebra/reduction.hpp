//
// Created by jmaerte on 24.03.20.
//

#ifndef ANUBIS_SUPERBUILD_REDUCTION_HPP
#define ANUBIS_SUPERBUILD_REDUCTION_HPP

#include "typedef.hpp"
#include <map>
#include <ALGEBRA_EXPORT.h>

namespace jmaerte {
    namespace algebra {
        namespace reduction {
            using namespace jmaerte::arith;

            ALGEBRA_EXPORT std::map<num::ap_int, unsigned int, num::comp::UNSIGNED_COMPARATOR> smith(s_int_matrix matrix);

        }
    }
}

#endif //ANUBIS_SUPERBUILD_REDUCTION_HPP
