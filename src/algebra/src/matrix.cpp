//
// Created by jmaerte on 24.03.20.
//

#include "algebra/matrix.hpp"

namespace jmaerte {
    namespace algebra {

        s_int_matrix NEW(std::vector<arith::vec::s_ap_int_vec> arr) {
            return typings::transform(typings::ints_from(0).take(arr.size()), [&arr](int i) {
                return arr[i];
            });
        }

    }
}