//
// Created by jmaerte on 08.05.20.
//

#ifndef ANUBIS_SUPERBUILD_STD_FACTORY_HPP
#define ANUBIS_SUPERBUILD_STD_FACTORY_HPP

namespace jmaerte {
    namespace arith {
        namespace vec {
            class ARITHMETIC_EXPORT std_factory {
            public:
                // ALLOCATION
                virtual s_ap_int_vec allocate_vec(std::size_t size) {
                    return new svec_node[1 + 2 * size];
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
                    svec_node* vec = allocate_vec(GET_OCC(v));
                    *vec = {.single = v->single};
                    for (int i = 0; i < GET_OCC(v); i++) {
                        num::ASSIGN(AT(vec, i), num::COPY(AT(v, i)));
                    }
                    return vec;
                }

                // ENLARGE
                virtual void enlarge(s_ap_int_vec&, std::size_t, std::size_t) {
                    svec_node* next = allocate_vec(size);
                    std::copy(v, AT(v, copy_range), next);
                    deallocate_vec(v);
                    v = next;
                    SET_SIZE(v, size);
                }

                // FREE
                virtual void free() { }
            };
        }
    }
}

#endif //ANUBIS_SUPERBUILD_STD_FACTORY_HPP
