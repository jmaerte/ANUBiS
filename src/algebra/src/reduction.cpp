//
// Created by jmaerte on 24.03.20.
//

#include <output/logger.hpp>
#include "algebra/reduction.hpp"
#include <arithmetic/operator.hpp>
#include <arithmetic/factory/heap_allocator.hpp>
#include "util/search.hpp"
#include <cstdlib>
#include <malloc.h>
#include <chrono>
#include <algorithm>
#include <exception>
#include <data_types/thread_pool.hpp>
#include <boost/thread.hpp>

namespace jmaerte {
    namespace algebra {
        namespace reduction {

            using namespace jmaerte::arith;

            auto comparator = [](const vec::s_ap_int_vec& a, const vec::s_ap_int_vec& b) {
                if (vec::IS_ZERO(a)) {
                    return true;
                } else {
                    if (vec::IS_ZERO(b)) return false;
                    else if (num::GET_POS(vec::AT(a, 0)) != num::GET_POS(vec::AT(b, 0))) return num::GET_POS(vec::AT(a, 0)) < num::GET_POS(vec::AT(b, 0));
                    else return num::COMPARE_ABS(vec::AT(a, 0), vec::AT(b, 0)) < 0;
                }
            };

            int(*comparator_int)(const vec::s_ap_int_vec&, const vec::s_ap_int_vec&) = [](const vec::s_ap_int_vec& a, const vec::s_ap_int_vec& b) {
                if (vec::IS_ZERO(a)) {
                    return -1;
                } else {
                    if (vec::IS_ZERO(b)) return 1;
                    else if (num::GET_POS(vec::AT(a, 0)) != num::GET_POS(vec::AT(b, 0))) return ((int) num::GET_POS(vec::AT(a, 0))) - ((int) num::GET_POS(vec::AT(b, 0)));
                    else return num::COMPARE(vec::AT(a, 0), vec::AT(b, 0));
                }
            };

            inline void reduce(arith::arith_context* context, vec::s_ap_int_vec& vec, std::map<int, vec::s_ap_int_vec>& trivial,
                    std::vector<vec::s_ap_int_vec>& remainder, std::vector<int>& first_remainder,
                    unsigned int& remainder_factory_id, bool is_remainder) {
                int pos = num::GET_POS(vec::AT(vec, 0));
//                        int k = util::binary_search_ints(first, pos, 0, first.size());
                auto it = trivial.find(pos);

                // DEBUGGING
                /*std::string str_vec = "";
                for (int i = 0; i < vec::GET_OCC(vec); i++) {
                    str_vec += std::to_string(num::GET_POS(vec::AT(vec, i))) + " -> " + num::STRINGIFY(vec::AT(vec, i)) + " ;  ";
                }
                std::vector<vec::s_ap_int_vec> reducing_vectors = {};
                std::vector<std::string> vec_strings = {};*/


//                        while (k < first.size() && first[k] == pos) {
                while (it != trivial.end()) {
//                            vec::REDUCE(vec, trivial[k], 0);
                    //std::cout << num::GET_POS(vec::AT(vec, 0)) << " " << num::GET_POS(vec::AT(it->second, 0)) << std::endl;
                    context->vec_reduce(vec, it->second, 0);
                    /*reducing_vectors.push_back(it->second);
                    std::string str_vec_c = "";
                    for (int i = 0; i < vec::GET_OCC(vec); i++) {
                        str_vec_c += std::to_string(num::GET_POS(vec::AT(vec, i))) + " -> " + num::STRINGIFY(vec::AT(vec, i)) + " ;  ";
                    }
                    vec_strings.push_back(str_vec_c);*/

                    if (vec::IS_ZERO(vec)) {
                        break;
                    }
                    pos = num::GET_POS(vec::AT(vec, 0));
//                            k = util::binary_search_ints(first, pos, k, first.size());
                    it = trivial.find(pos);
                }

                if (vec::IS_ZERO(vec)) {
                    vec::DELETE(vec);
                } else if (context->is_single(vec::AT(vec, 0)) && vec::AT(vec, 0).value == 1UL) {
                    if (!num::GET_SIGN(vec::AT(vec, 0))) vec::SWITCH_SIGNS(vec);
                    pos = num::GET_POS(vec::AT(vec, 0));

                    trivial[pos] = vec;
                } else {
                    remainder.push_back(vec::COPY(remainder_factory_id, vec));
                    vec::DELETE(vec);


                    /*std::cout << "REMAINDER REDUCED THIS WAY: " << std::endl;
                    std::cout << "\t ORIGINAL VECTOR: " << str_vec << std::endl;
                    std::cout << "\t REDUCING VECTORS: " << std::endl;

                    for (int j = 0; j < reducing_vectors.size(); j++) {
                        std::cout << "\t\t" << j << " TRIVIAL: ";
                        for (int l = 0; l < vec::GET_OCC(reducing_vectors[j]); l++) {
                            std::cout << std::to_string(num::GET_POS(vec::AT(reducing_vectors[j], l))) << " -> " << num::STRINGIFY(vec::AT(reducing_vectors[j], l)) << " ;  ";
                        }
                        std::cout << std::endl;
                        std::cout << "\t\t" << j << " VEC: " << vec_strings[j] << std::endl;
                    }*/
                }
            }

            inline void reduce2(arith::arith_context* context, std::vector<vec::s_ap_int_vec>& remainder, std::map<int, vec::s_ap_int_vec>& trivial) {
                vec::s_ap_int_vec pivot = remainder.back();
                remainder.pop_back();

                int pos = num::GET_POS(vec::AT(pivot, 0));
                auto it = trivial.find(pos);

                while (it != trivial.end()) {
                    vec::s_ap_int_vec& vec = it->second;
                    if (context->is_single(vec::AT(vec, 0)) && vec::AT(vec, 0).value == 1UL) {
                        context->vec_reduce(pivot, vec, 0);
                    } else if (num::COMPARE_ABS(vec::AT(pivot, 0), vec::AT(vec, 0)) == 0) {
                        num::ap_int a = vec::GET(vec, 0);
                        num::ap_int b = vec::GET(pivot, 0);

                        bool sign_a = num::GET_SIGN(a);
                        bool sign_b = num::GET_SIGN(b);

                        num::ap_int lambda;
                        num::NEW(lambda, 0, !(sign_a ^ sign_b), 1u);

                        context->vec_add(pivot, 1, lambda, vec, 1, 0);
                    } else {
                        num::ap_int s, t;
                        num::ap_int x, y;

                        num::ap_int& a = vec::GET(vec, 0);
                        num::ap_int& b = vec::GET(pivot, 0);

                        bool sign_a = num::GET_SIGN(a);
                        bool sign_b = num::GET_SIGN(b);

                        context->gcd(s, t, x, y, a, b);

                        if (num::IS_ZERO(s)) {
                            vec::GET(pivot, 0) = a;
                            if (sign_b) {
                                num::SWITCH_SIGN(vec::GET(pivot, 0));
                                vec::SWITCH_SIGNS(pivot);
                            }
                            /*std::copy(vec.values + 1, vec.values + vec::GET_OCC(vec), vec.values);
                            vec::SET_OCC(vec, vec::GET_OCC(vec) - 1);*/

                            num::SWITCH_SIGN(x);

                            context->vec_add(vec, 1, x, pivot, 1, 0);

                            vec::s_ap_int_vec temp = vec;
                            trivial[pos] = pivot;
                            pivot = temp;
                        } else if (num::IS_ZERO(t)) {
                            if (sign_a) {
                                num::SWITCH_SIGN(vec::GET(vec, 0));
                                vec::SWITCH_SIGNS(vec);
                            }

                            num::SWITCH_SIGN(y);

                            num::DELETE_DATA(vec::GET(pivot, 0));
                            context->vec_add(pivot, 1, y, vec, 1, 0);
                        } else {
                            num::SWITCH_SIGN(y);
                            num::DELETE_DATA(vec::GET(pivot, 0));
                            context->vec_combine(vec, pivot, s, t, y, x, 1, 1, 1, 0);
                            //vec::DELETE_POS(pivot, 0);
                        }
                        num::DELETE_DATA(s);
                        num::DELETE_DATA(t);
                        num::DELETE_DATA(x);
                        num::DELETE_DATA(y);

                        if (context->is_single(vec::AT(vec, 0)) && vec::AT(vec, 0).value == 1UL) {
                            if (!num::GET_SIGN(vec::AT(vec, 0))) vec::SWITCH_SIGNS(vec);
                        }
                    }
                    if (vec::IS_ZERO(pivot)) break;
                    pos = num::GET_POS(vec::AT(pivot, 0));
                    it = trivial.find(pos);
                }

                if (vec::IS_ZERO(pivot)) {
                    vec::DELETE(pivot);
                } else {
                    if (context->is_single(vec::AT(pivot, 0)) && vec::AT(pivot, 0).value == 1UL) {
                        if (!num::GET_SIGN(vec::AT(pivot, 0))) vec::SWITCH_SIGNS(pivot);
                    }
                    trivial[pos] = pivot;
                }
            }

            int get_block_size(std::vector<vec::s_ap_int_vec>& remainder, int offset) {
                int pos = num::GET_POS(vec::AT(remainder[offset], 0));
                int block_size = 1;
                // we know that first_remainder is ordered and thus can cut off the beginning:
                for (; offset + block_size < remainder.size(); block_size++) {
                    if (num::GET_POS(vec::AT(remainder[offset + block_size], 0)) != pos) break;
                }
                return block_size;
            }

            int get_block_size(std::vector<vec::s_ap_int_vec>& remainder) {
                return get_block_size(remainder, 0);
            }

            void col_div2_reduce(std::vector<vec::s_ap_int_vec>& remainder, int offset, int block_size) {
                if (vec::IS_ZERO(remainder[offset])) return;
                int k = 0;

                while (block_size > 1) {
                    std::cout << num::STRINGIFY(vec::GET(remainder[offset], 0)) << " " << num::STRINGIFY(vec::GET(remainder[offset + 1], 0)) << std::endl;
                    num::ap_int quot;
                    int shift = num::PREPARE_DIVISOR(vec::GET(remainder[offset], 0));
                    num::DIV(quot, vec::GET(remainder[offset + 1], 0), vec::GET(remainder[offset], 0), shift);
                    num::DENORMALIZE_DIVISOR(vec::GET(remainder[offset], 0), shift);

                    num::SWITCH_SIGN(quot);
                    int start = num::IS_ZERO(vec::AT(remainder[offset + 1], 0)) ? 0 : 1;

                    vec::ADD(remainder[offset + 1], 1, quot, remainder[offset], 1, start);
                    num::DELETE_DATA(quot);

                    if (start == 0) {
                        // TODO: SORT IN VECTOR BY BINARY SEARCH
                        remainder.erase(std::remove_if(remainder.begin(), remainder.end(), [](const vec::s_ap_int_vec& v) {return v.values == nullptr;}), remainder.end());
                        std::sort(remainder.begin() + offset, remainder.end(), comparator);
                        block_size--;
                    } else {
                        vec::s_ap_int_vec temp = remainder[offset];
                        remainder[offset] = remainder[offset + 1];
                        remainder[offset + 1] = temp;
                    }
                }

                remainder.erase(std::remove_if(remainder.begin(), remainder.end(), [](const vec::s_ap_int_vec& v) {return v.values == nullptr;}), remainder.end());
                std::sort(remainder.begin() + offset, remainder.end(), comparator);
            }


//            template<typename Allocator = std::allocator<jmaerte::arith::svec_node>>
            std::map<num::ap_int, unsigned int, num::comp::UNSIGNED_COMPARATOR> smith(s_int_matrix matrix) {
                using namespace jmaerte::arith;

                unsigned int channel_id = jmaerte::output::LOGGER.register_channel("Smith", std::cout);

                //double time_elapsed;
                double allocation_generation_time = 0;
                double search_time = 0;

                auto start_time = std::chrono::steady_clock::now();

                unsigned int remainder_factory_id = arith::vec::factory::REGISTER<jmaerte::arith::vec::std_factory>();

                arith::arith_context* context = matrix.get_context();

                std::vector<vec::s_ap_int_vec> remainder;
                std::vector<int> first_remainder;
                std::map<num::ap_int, unsigned int, num::comp::UNSIGNED_COMPARATOR> result;

                num::ap_int one;
                num::NEW(one, 0, false, 1ULL);

                num::ap_int det; // maximal minor determinant.
                {
                    std::map<int, vec::s_ap_int_vec> trivial;
//                    std::vector<vec::s_ap_int_vec> trivial;
//                    std::vector<int> first;
                    int count = 0;

                    while (!matrix.is_empty()) {
                        vec::s_ap_int_vec vec = matrix.get();

                        if (count % 1000 == 0) std::cout << count << " / " << matrix.rows() << " current torsion-free rank: " << trivial.size() << "; number of deferred vectors " << remainder.size() << std::endl;
                        count++;
                        reduce(context, vec, trivial, remainder, first_remainder, remainder_factory_id, false);
                    }

                    std::cout << "current torsion-free rank: " << trivial.size() << std::endl;

/*
                    // bring into echelon form
                    int kernels = boost::thread::hardware_concurrency() - 1;

                    boost::thread threads[kernels];
                    for (int j = 0; j < kernels; j++) {
                        threads[j] = boost::thread ([&remainder, &trivial, j, kernels]() {
                            for (int l = 0; l < remainder.size(); l++) {
                                if (l % kernels != j) continue;
                                for (int i = 0; i < vec::GET_OCC(remainder[l]);) {
                                    auto it = trivial.find(num::GET_POS(vec::AT(remainder[l], i)));
                                    if (it != trivial.end()) {
                                        vec::REDUCE(remainder[l], it->second, i);
                                    } else i++;
                                }
                                if (vec::IS_ZERO(remainder[l])) {
                                    vec::DELETE(remainder[l]);
                                    remainder[l].values = nullptr;
                                }
                            }
                        });
                    }
                    for (int i = 0; i < kernels; i++) threads[i].join();

                    remainder.erase(std::remove_if(remainder.begin(), remainder.end(), [](const vec::s_ap_int_vec& v) {return v.values == nullptr;}), remainder.end());*/

                    int remainder_size = remainder.size();
                    while (remainder.size() > 0) {
                        reduce2(context, remainder, trivial);
                    }

                    num::NEW(det, 0, false, 1u);
                    for (auto it = trivial.begin(); it != trivial.end(); ++it) {
                        if (context->is_single(vec::AT(it->second, 0)) && vec::AT(it->second, 0).value == 1UL) {
                            if (vec::GET_FACTORY_ID(it->second) == remainder_factory_id) {
                                vec::DELETE(it->second);
                            }
                        } else {
                            vec::s_ap_int_vec vec = it->second;
                            num::MUL(det, vec::AT(vec, 0));
                            for (int i = 1; i < vec::GET_OCC(vec);) {
                                auto it_triv = trivial.find(num::GET_POS(vec::AT(vec, i)));
                                if (it_triv == trivial.end()) {
                                    i++;
                                } else {
                                    vec::s_ap_int_vec triv = it_triv->second;
                                    if (context->is_single(vec::AT(triv, 0)) && vec::AT(triv, 0).value == 1UL) {
                                        context->vec_reduce(vec, triv, i);
                                    } else i++;
                                }
                            }
                            remainder.push_back(vec);
                        }
                    }
                    if (num::GET_SIGN(det)) num::SWITCH_SIGN(det);

                    result.emplace(one, trivial.size() - remainder.size());

                    /*int kernels = boost::thread::hardware_concurrency() - 1;

                    boost::thread threads[kernels];
                    for (int j = 0; j < kernels; j++) {
                        threads[j] = boost::thread ([&remainder, &trivial, j, kernels]() {
                            for (int l = 0; l < remainder.size(); l++) {
                                if (l % kernels != j) continue;
                                auto it = trivial.find(num::GET_POS(vec::AT(remainder[l], 0)));
                                while (it != trivial.end()) {
                                    vec::REDUCE(remainder[l], it->second, 0);
                                    if (vec::IS_ZERO(remainder[l])) {
                                        vec::DELETE(remainder[l]);
                                        remainder[l].values = nullptr;
                                        it = trivial.end();
                                    } else it = trivial.find(num::GET_POS(vec::AT(remainder[l], 0)));
                                }
                            }
                        });
                    }
                    for (int i = 0; i < kernels; i++) threads[i].join();

                    remainder.erase(std::remove_if(remainder.begin(), remainder.end(), [](const vec::s_ap_int_vec& v) {return v.values == nullptr;}), remainder.end());
                    */
                }

                //time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start_time).count();
                //output::print_time_resume("Matrix Reduction", time_elapsed, {{"allocation and generation", allocation_generation_time}, {"searching in position list", search_time}});

                int n = remainder.size();// amount of non-trivial vectors in remainder.

                std::sort(remainder.begin(), remainder.end(), comparator);

                n = remainder.size();// amount of non-trivial vectors in remainder.

                jmaerte::output::LOGGER.log(channel_id, "TORSION-FREE RANK: " + std::to_string(result.at(one)));
                jmaerte::output::LOGGER.log(channel_id, "SEARCHING FOR TORSION OF RANK: " + std::to_string(n));

                arith::vec::factory::RELEASE(matrix.get_factory_id());
                

                for (int i = 0; i < remainder.size(); i++) {
                    std::cout << "REMAINDER " << i << " " << num::GET_POS(vec::GET(remainder[i], 0)) << " " << num::STRINGIFY(vec::GET(remainder[i], 0)) << " ";
                    num::ap_int max = vec::GET(remainder[i], 0);
                    for (int j = 1; j < vec::GET_OCC(remainder[i]); j++) {
                        if (num::COMPARE_ABS(vec::GET(remainder[i], j), max) > 0) {
                            max = vec::GET(remainder[i], j);
                        }
                    }
                    std::cout << num::STRINGIFY(max) << std::endl;
                }

                //jmaerte::arith::vec::PACK = true;
                std::cout << "MAXIMAL MINOR DETERMINANT IS " << num::STRINGIFY(det) << std::endl;

                while (remainder.size() > 0) {

                    jmaerte::output::LOGGER.log(channel_id, "Calculating torsion " + std::to_string(n - remainder.size() + 1) + " / " + std::to_string(n));
                    vec::s_ap_int_vec temp;

                    int pos = num::GET_POS(vec::AT(remainder[0], 0));
                    int block_size = get_block_size(remainder);

                    // is true if we reduce column, false if we reduce row
                    bool col = true;
                    while (true) {

                        if (col) {
                            std::cout << "col " << block_size << std::endl;

                            std::sort(remainder.begin(), remainder.end(), comparator);
                            
                            block_size = get_block_size(remainder, 0);
                            col_div2_reduce(remainder, 0, block_size);

                            //vec::PACK_VEC(remainder[0]);
                            block_size = 1;
                            col = false;

                            /*
                            // Pivotize

                            int k = -1; // row index of pivot
                            for (int row = 0; row < block_size; row++) {
                                if (k < 0 || num::COMPARE_ABS(vec::AT(temp, 0), vec::AT(remainder[row], 0)) > 0) {
                                    temp = remainder[row];
                                    k = row;
                                }
                            }

                            if (k < 0) return result;
                            remainder[k] = remainder[0];
                            remainder[0] = temp;

                            //std::cout << num::STRINGIFY(vec::AT(remainder[0], 0)) << std::endl;

                            int occ = 0;
//                num::ap_int pre = num::PRECOMPUTE_SDIV_DIVISOR(vec::AT(remainder[i], 0));
                            int shift = num::PREPARE_DIVISOR(vec::GET(remainder[0], 0));
                            for (int row = 1; row < block_size; row++) {
                                num::ap_int lambda;
                                
                                //std::cout << num::STRINGIFY(vec::AT(remainder[row], 0)) << std::endl;
                                
                                num::DIV(lambda, vec::GET(remainder[row], 0), vec::GET(remainder[0], 0), shift);
                                
                                //std::cout << "DIV: " << num::STRINGIFY(lambda) << " " << num::STRINGIFY(vec::AT(remainder[row], 0)) << std::endl;

                                num::SWITCH_SIGN(lambda);
                                int start = num::IS_ZERO(vec::AT(remainder[row], 0)) ? 0 : 1;
                                if (start == 0) {
                                    vec::DELETE_POS(remainder[row], 0);
                                }

                                vec::ADD(remainder[row], start, lambda, remainder[0], 1);

                                num::DELETE_DATA(lambda);

//                                std::cout << "out of addition" << std::endl;
//                                std::cout << vec::GET_OCC(remainder[row]) << std::endl;
                                int next_pos;
                                if (vec::IS_ZERO(remainder[row])) {
                                    vec::DELETE(remainder[row]);
                                    remainder.erase(remainder.begin() + row);
                                    row--;
                                    block_size--;
                                } else {
                                    if ((next_pos = num::GET_POS(vec::AT(remainder[row], 0))) != pos) {
                                        temp = remainder[row];

//                                        for (int i = 0; i < remainder.size(); i++) std::cout << num::GET_POS(vec::AT(remainder[i], 0)) << std::endl;

                                        // TODO: REPLACE THE FOLLOWING BY BINARY SEARCH AGAIN
//                                        k = util::binary_search(remainder, temp, block_size, (int)remainder.size(), comparator_int);
                                        for (k = block_size; k < remainder.size(); k++) {
                                            if (num::GET_POS(vec::AT(remainder[k], 0)) > num::GET_POS(vec::AT(temp, 0))) {
                                                break;
                                            }
                                        }

                                        if (k > row) std::copy(remainder.begin() + row + 1, remainder.begin() + k, remainder.begin() + row);
                                        remainder[k - 1] = temp;
                                        block_size--;
                                        row--;
                                    }
                                }
                            }
                            num::DENORMALIZE_DIVISOR(vec::GET(remainder[0], 0), shift);

                            col = block_size > 1;

                            */


                        } else {
                            std::cout << "row" << std::endl;

                            if (context->is_single(vec::AT(remainder[0], 0)) && *num::ABS(vec::AT(remainder[0], 0)) == 1UL) {
                                auto it = result.find(vec::AT(remainder[0], 0));
                                if (it != result.end()) (*it).second = (*it).second + 1u;
                                else {
                                    if (num::GET_SIGN(vec::AT(remainder[0], 0))) num::SWITCH_SIGN(vec::GET(remainder[0], 0));
                                    num::ap_int copy;
                                    num::COPY(copy, vec::AT(remainder[0], 0));
                                    result.emplace(copy, 1u);
                                }
//                                (*it).second = (*it).second + 1u;
                                vec::DELETE(remainder[0]);
                                remainder.erase(remainder.begin());
                                break;
                            } else {

                                vec::MOD(remainder[0]);

                                if (vec::GET_OCC(remainder[0]) == 1) {
                                    auto it = result.find(vec::AT(remainder[0], 0));
//                                    std::cout << num::STRINGIFY(vec::AT(temp, 0)) << std::endl;
                                    if (it == result.end()) {
                                        if (num::GET_SIGN(vec::AT(remainder[0], 0))) num::SWITCH_SIGN(vec::GET(remainder[0], 0));
                                        num::ap_int copy;
                                        num::COPY(copy, vec::AT(remainder[0], 0));
                                        result.emplace(copy, 1u);
                                    } else (*it).second = (*it).second + 1u;
                                    vec::DELETE(remainder[0]);
                                    remainder.erase(remainder.begin());
//                                    n--;
//                                    i--;
                                    break;
                                } else {
                                    int k = 0;
                                    for (int row = 1; row < vec::GET_OCC(remainder[0]); row++) {
                                        if (num::COMPARE_ABS(vec::AT(remainder[0], k), vec::AT(remainder[0], row)) > 0) k = row;
                                    }
                                    int next_pos = num::GET_POS(vec::AT(remainder[0], k));

                                    vec::SWAP_VALUES(remainder[0], 0, k);

                                    for(int row = 1; row < remainder.size(); row++) {
                                        int l = vec::FIND_POS(remainder[row], next_pos);
                                        if (l < vec::GET_OCC(remainder[row]) && num::GET_POS(vec::AT(remainder[row], l)) == next_pos) {
//                                            num::SET_POS(vec::AT(temp_row, l), pos);
                                            num::ap_int el = vec::AT(remainder[row], l);
                                            num::SET_POS(el, num::GET_POS(vec::AT(remainder[0], 0)));

                                            if (l < vec::GET_OCC(remainder[row]) - 1) std::copy(remainder[row].values + l + 1, remainder[row].values + vec::GET_OCC(remainder[row]), remainder[row].values + l);
                                            std::copy_backward(remainder[row].values, remainder[row].values + vec::GET_OCC(remainder[row]) - 1, remainder[row].values + vec::GET_OCC(remainder[row]));
                                            remainder[row].values[0] = el;

                                            vec::s_ap_int_vec temp = remainder[row];
                                            std::copy_backward(remainder.begin() + 1, remainder.begin() + row, remainder.begin() + row + 1);
                                            remainder[1] = temp;
                                            block_size++;
                                        }
                                    }
                                    col = block_size > 1;
                                }
                            }
                        }
                    }
                }
                num::DELETE_DATA(det);
                //jmaerte::arith::num::PRINT = false;
                arith::vec::factory::RELEASE(remainder_factory_id);
                jmaerte::output::LOGGER.release_channel(channel_id);

                return result;
            }
        }
    }
}