//
// Created by jmaerte on 05.10.2020.
//

#include <arithmetic/factory/heap_allocator.hpp>
#include <arithmetic/factory/stack_allocator.hpp>

namespace jmaerte {
    namespace arith {
        namespace vec {

            std_factory::std_factory() : vector_allocator() {}

            template<std::size_t block_size>
            stack_allocator<block_size>::stack_allocator() : vector_allocator() {}

            template class stack_allocator<2048>;
            template class stack_allocator<134217728>;
        }
    }
}