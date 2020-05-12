//
// Created by jmaerte on 11.05.20.
//

#include "arithmetic/factory/Ivector_allocator.hpp"
#include "arithmetic/factory/factory.hpp"

namespace jmaerte {
    namespace arith {
        namespace vec {

            vector_allocator::vector_allocator() {
                ID = factory::dict.push_factory(this);
            }

        }
    }
}