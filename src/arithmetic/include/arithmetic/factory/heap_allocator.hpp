//
// Created by jmaerte on 08.05.20.
//

#ifndef ANUBIS_SUPERBUILD_STD_FACTORY_HPP
#define ANUBIS_SUPERBUILD_STD_FACTORY_HPP

#include "../operator.hpp"
#include "Ivector_allocator.hpp"
#include <exception>
#include <algorithm>
#include <vector>
#include <sstream>
#include <boost/thread.hpp>

namespace jmaerte {
    namespace arith {
        namespace vec {
            
            class ARITHMETIC_EXPORT std_factory : public vector_allocator {
            public:

                std_factory();

                // ALLOCATION
                virtual s_ap_int_vec allocate_vec(std::size_t size) {
                    allocation_mutex.lock();
                    svec_node* res = new svec_node[1 + 2 * size];

                    // check if address is valid
                    for (int i = 0; i < curr_vectors.size(); i++) {
                        std::ostringstream ss;
                        if (curr_vectors[i] < res) {
                            ss << "allocated memory doesn't respect the already taken space; new: " << res << "; old: " << curr_vectors[i] << " with size " << std::to_string(GET_SIZE(curr_vectors[i]));
                            if (GET_SIZE(curr_vectors[i]) > res - curr_vectors[i]) throw std::runtime_error(ss.str());
                        } else if (curr_vectors[i] > res) {
                            ss << "place is not reserved; new: " << res << " with size " << std::to_string(1 + 2 * size) << "; old: " << curr_vectors[i];
                            if (1 + 2 * size > curr_vectors[i] - res) throw std::runtime_error(ss.str());
                        } else {
                            ss << "Same address (" << res << ") was taken twice.";
                            throw std::runtime_error(ss.str());
                        }
                    }

                    curr_vectors.push_back(res);

                    SET_FACTORY_ID(res, get_id());
                    allocation_mutex.unlock();
                    return res;
                }

                virtual void deallocate_vec(s_ap_int_vec v) {
                    if (vec::GET_FACTORY_ID(v) != get_id()) {
                        throw std::runtime_error("Factory " + std::to_string(get_id()) + " tried to deallocate vector that was created by factory " + std::to_string(vec::GET_FACTORY_ID(v)) + "!");
                    }
                    ULL occ = GET_OCC(v);
                    for (int i = 0; i < occ; i++) {
                        num::DELETE_DATA(AT(v, i));
                    }
                    curr_vectors.erase(std::remove(curr_vectors.begin(), curr_vectors.end(), v), curr_vectors.end());
                    delete[] v;
                }

                // COPY
                virtual s_ap_int_vec copy(s_ap_int_vec v) {
                    svec_node* vec = allocate_vec(vec::GET_OCC(v));
                    if (vec == v) throw std::runtime_error("Allocated the same memory twice!");
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

            private:
                std::vector<s_ap_int_vec> curr_vectors;
                boost::mutex allocation_mutex;
            };
        }
    }
}

#endif //ANUBIS_SUPERBUILD_STD_FACTORY_HPP
