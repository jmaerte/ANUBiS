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

                num::ap_int* allocate_data(std::size_t size) {
                    num::ap_int* res;
                    //allocation_mutex.lock();
                    res = new num::ap_int[size];
                    //allocation_mutex.unlock();
                    if (std::find(curr_vectors.begin(), curr_vectors.end(), res) != curr_vectors.end()) throw std::runtime_error("Allocated same vector twice!");
                    //curr_vectors.push_back(res);
                    return res;
                }

                // ALLOCATION
                virtual s_ap_int_vec allocate_vec(std::size_t size) {

                    s_ap_int_vec v;
                    v.meta = 0ULL;
                    v.values = allocate_data(size);

                    // check if address is valid
//                    for (int i = 0; i < curr_vectors.size(); i++) {
//                        std::ostringstream ss;
//                        if (curr_vectors[i] < res) {
//                            ss << "allocated memory doesn't respect the already taken space; new: " << res << "; old: " << curr_vectors[i] << " with size " << std::to_string(GET_SIZE(curr_vectors[i]));
//                            if (GET_SIZE(curr_vectors[i]) > res - curr_vectors[i]) throw std::runtime_error(ss.str());
//                        } else if (curr_vectors[i] > res) {
//                            ss << "place is not reserved; new: " << res << " with size " << std::to_string(1 + 2 * size) << "; old: " << curr_vectors[i];
//                            if (1 + 2 * size > curr_vectors[i] - res) throw std::runtime_error(ss.str());
//                        } else {
//                            ss << "Same address (" << res << ") was taken twice.";
//                            throw std::runtime_error(ss.str());
//                        }
//                    }
//
//                    curr_vectors.push_back(res);


                    SET_FACTORY_ID(v, get_id());
                    SET_SIZE(v, size);

                    return v;
                }

                virtual void deallocate_vec(s_ap_int_vec& v) {
                    if (vec::GET_FACTORY_ID(v) != get_id()) {
                        throw std::runtime_error("Factory " + std::to_string(get_id()) + " tried to deallocate vector that was created by factory " + std::to_string(vec::GET_FACTORY_ID(v)) + "!");
                    }
                    ULL occ = GET_OCC(v);
                    for (int i = 0; i < occ; i++) {
                        num::DELETE_DATA(v.values[i]);
                    }
                    //curr_vectors.erase(std::remove(curr_vectors.begin(), curr_vectors.end(), v.values), curr_vectors.end());
                    delete[] v.values;
                }

                // COPY
                virtual s_ap_int_vec copy(s_ap_int_vec& v) {
                    s_ap_int_vec vec = allocate_vec(vec::GET_SIZE(v));
                    //if (vec == v) throw std::runtime_error("Allocated the same memory twice!");
                    vec.meta = v.meta;
                    SET_FACTORY_ID(vec, get_id());
                    //SET_SIZE(vec, vec::GET_SIZE(v));
                    for (int i = 0; i < GET_OCC(v); i++) {
                         num::COPY(vec.values[i], v.values[i]);
                    }
                    return vec;
                }

                // ENLARGE
                virtual void enlarge(s_ap_int_vec& v, std::size_t new_size, std::size_t copy_range) {
                    num::ap_int* next = allocate_data(new_size);
                    if (copy_range > 0) std::copy(v.values, v.values + copy_range, next);
//                    deallocate_vec(v);
                    //curr_vectors.erase(std::remove(curr_vectors.begin(), curr_vectors.end(), v.values), curr_vectors.end());
                    delete[] v.values;
                    v.values = next;
                    SET_SIZE(v, new_size);
                }

                // FREE
                virtual void free() { }

            private:
                std::vector<num::ap_int*> curr_vectors;
                boost::mutex allocation_mutex;
            };
        }
    }
}

#endif //ANUBIS_SUPERBUILD_STD_FACTORY_HPP
