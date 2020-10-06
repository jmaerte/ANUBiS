//
// Created by jmaerte on 24.03.20.
//

#ifndef ANUBIS_SUPERBUILD_MATRIX_HPP
#define ANUBIS_SUPERBUILD_MATRIX_HPP

#include <ALGEBRA_EXPORT.h>
#include <vector>
#include "typedef.hpp"

namespace jmaerte {
    namespace algebra {
        ALGEBRA_EXPORT s_int_matrix NEW(std::vector<arith::vec::s_ap_int_vec> arr);
    }
}

#endif //ANUBIS_SUPERBUILD_MATRIX_HPP
