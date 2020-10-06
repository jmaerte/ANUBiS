//
// Created by jmaerte on 11.05.20.
//

#include "arithmetic/factory/factory.hpp"
#include <iostream>
#include "arithmetic/operator.hpp"

namespace jmaerte {
    namespace arith {
        namespace vec {
            namespace factory {
                vector_factory& dict = vector_factory::get_instance(MAX_FACTORIES);
            }

            vector_allocator* vector_factory::get_factory(s_ap_int_vec& v)  {
                try {
                    return factories.at(GET_FACTORY_ID(v));
                } catch (const std::out_of_range& e) {
                    std::cout << "[Mem] ERROR - Tried to access factory " << GET_FACTORY_ID(v) << " that is non-existent!" << std::endl;
                    throw e;
                }
            }
        }
    }
}