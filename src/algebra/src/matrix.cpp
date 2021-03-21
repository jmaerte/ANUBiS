//
// Created by jmaerte on 24.03.20.
//

#include "algebra/matrix.hpp"
#include <arithmetic/factory/stack_allocator.hpp>
#include <arithmetic/operator.hpp>

namespace jmaerte {
    namespace algebra {

        s_int_matrix NEW(std::vector<arith::vec::s_ap_int_vec> arr) {
            unsigned int factory_id = arith::vec::factory::REGISTER<arith::vec::stack_allocator<2048>>();
            return s_int_matrix(factory_id, arr.size(), arith::get_context(), [&arr](int i, unsigned int factory_id) {
                return arr[i];
            });
        }

    }
}