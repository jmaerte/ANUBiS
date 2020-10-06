//
// Created by jmaerte on 08.05.20.
//

#ifndef ANUBIS_SUPERBUILD_STD_FACTORY_HPP
#define ANUBIS_SUPERBUILD_STD_FACTORY_HPP

#include "../operator.hpp"

namespace jmaerte {
    namespace arith {
        namespace vec {
            class ARITHMETIC_EXPORT std_factory : public vector_allocator {
            public:

                // ALLOCATION
                virtual s_ap_int_vec allocate_vec(std::size_t size) {
                    svec_node* res = new svec_node[1 + 2 * size];
                    SET_FACTORY_ID(res, get_id());
                    return res;
                }

                virtual void deallocate_vec(s_ap_int_vec v) {
                    ULL occ = GET_OCC(v);
                    for (int i = 0; i < occ; i++) {
                        num::DELETE_DATA(AT(v, i));
                    }
                    delete[] v;
                }

                // COPY
                virtual s_ap_int_vec copy(s_ap_int_vec v) {
                    svec_node* vec = allocate_vec(vec::GET_OCC(v));
                    *vec = (svec_node){.single = v->single};
                    SET_FACTORY_ID(vec, get_id());
                    SET_SIZE(vec, vec::GET_OCC(v));
                    for (int i = 0; i < GET_OCC(v); i++) {
                        num::ASSIGN(AT(vec, i), num::COPY(AT(v, i)));
                    }
                    return vec;
                }

                // ENLARGE
                virtual void enlarge(s_ap_int_vec& v, std::size_t new_size, std::size_t copy_range) {
                    svec_node* next = allocate_vec(new_size);
                    std::copy(v, AT(v, copy_range), next);
                    deallocate_vec(v);
                    v = next;
                    SET_SIZE(v, new_size);
                }

                // FREE
                virtual void free() { }
            };
        }
    }
}

#endif //ANUBIS_SUPERBUILD_STD_FACTORY_HPP
