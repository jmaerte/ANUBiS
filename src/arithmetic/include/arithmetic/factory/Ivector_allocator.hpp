//
// Created by jmaerte on 11.05.20.
//

#ifndef ANUBIS_SUPERBUILD_IVECTOR_ALLOCATOR_HPP
#define ANUBIS_SUPERBUILD_IVECTOR_ALLOCATOR_HPP

#include "../typedef.hpp"
#include <cstdlib>

namespace jmaerte {
    namespace arith {
        namespace vec {

            class vector_allocator {
            public:
                vector_allocator();

                vector_allocator(const vector_allocator&) = delete;
                void operator=(const vector_allocator&) = delete;

                // ALLOCATION
                virtual s_ap_int_vec allocate_vec(std::size_t) = 0;
                virtual void deallocate_vec(s_ap_int_vec) = 0;

                // COPY
                virtual s_ap_int_vec copy(s_ap_int_vec) = 0;

                // ENLARGE
                virtual void enlarge(s_ap_int_vec&, std::size_t, std::size_t) = 0;

                // FREE
                virtual void free() = 0;

                unsigned int get_id() {
                    return ID;
                }

            private:
                unsigned int ID;
            };
        }
    }
}

#endif //ANUBIS_SUPERBUILD_IVECTOR_ALLOCATOR_HPP