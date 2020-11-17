//
// Created by jmaerte on 11.05.20.
//

#include "arithmetic/factory/Ivector_allocator.hpp"
#include "arithmetic/factory/factory.hpp"

namespace jmaerte {
    namespace arith {
        namespace vec {

            vector_allocator::vector_allocator() {
                ID = vector_factory::get_instance(factory::MAX_FACTORIES).push_factory(this);
                channel_id = jmaerte::output::LOGGER.register_channel("MEM | Factory " + std::to_string(ID), std::cout);
            }

        }
    }
}