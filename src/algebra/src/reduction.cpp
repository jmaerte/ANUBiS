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

namespace jmaerte {
    namespace algebra {
        namespace reduction {

            using namespace jmaerte::arith;

            void reduce(double& search_time, vec::s_ap_int_vec& vec, std::map<int, vec::s_ap_int_vec>& trivial,
                    std::vector<vec::s_ap_int_vec>& remainder, std::vector<int>& first_remainder,
                    unsigned int& remainder_factory_id) {
                int pos = num::GET_POS(vec::AT(vec, 0));
                auto time = std::chrono::steady_clock::now();
//                        int k = util::binary_search_ints(first, pos, 0, first.size());
                auto it = trivial.find(pos);
                search_time += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - time).count();

//                        while (k < first.size() && first[k] == pos) {
                while (it != trivial.end()) {
//                            vec::REDUCE(vec, trivial[k], 0);
                    vec::REDUCE(vec, it->second, 0);

                    if (vec::GET_OCC(vec) == 0) {
                        break;
                    }
                    pos = num::GET_POS(vec::AT(vec, 0));
                    time = std::chrono::steady_clock::now();
//                            k = util::binary_search_ints(first, pos, k, first.size());
                    it = trivial.find(pos);
                    search_time += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - time).count();
                }

                if (vec::GET_OCC(vec) == 0) {
                    vec::DELETE(vec);
                } else if (num::IS_SINGLE(vec::AT(vec, 0)) && (vec::AT(vec, 0) + 1)->single == 1ULL) {
                    // assert that the leading entry is -1.
                    if (!num::GET_SIGN(vec::AT(vec, 0))) vec::SWITCH_SIGNS(vec);
                    pos = num::GET_POS(vec::AT(vec, 0));

                    // REDUCED ROW ECHELON FORM
//                            for (vec::s_ap_int_vec& v : trivial) {
//
//                                int j = vec::FIND_POS(v, pos);
//                                if (j < vec::GET_OCC(v) && num::GET_POS(vec::AT(v, j)) == pos) {
//                                    vec::REDUCE(v, vec, j);
//                                }
//                            }
//
//                            k = util::binary_search(first, pos, 0, first.size(), util::compare_ints);

//                            trivial.insert(trivial.begin() + k, vec);
//                            first.insert(first.begin() + k, pos);
                    trivial[pos] = vec;

                    // this can be done multi-threaded
                    for (int l = 0; l < remainder.size(); l++) {
                        auto& v = remainder[l];
//                                if (v == nullptr) continue;
                        int j = vec::FIND_POS(v, pos);
                        if (j < vec::GET_OCC(v) && num::GET_POS(vec::AT(v, j)) == pos) {

                            std::cout << "REMAINDER: FIRST: " << first_remainder[l] << " : ";

                            for (int i = 0; i < vec::GET_OCC(v); i++) {
                                std::cout << num::GET_POS(vec::AT(v, i)) << " " << num::STRINGIFY(vec::AT(v, i)) << " ; ";
                            }
                            std::cout << std::endl;

                            vec::REDUCE(v, vec, j);

                            if (j == 0) {
                                std::cout << "l : " << l << std::endl;
                                std::cout << "remainder.size : " << remainder.size() << std::endl;
                                remainder.erase(remainder.begin() + l);
                                first_remainder.erase(first_remainder.begin() + l);
                                for (int i = 0; i < vec::GET_OCC(v); i++) {
                                    std::cout << num::GET_POS(vec::AT(v, i)) << " " << num::STRINGIFY(vec::AT(v, i)) << " ; ";
                                }
                                std::cout << std::endl;

                                if (vec::GET_OCC(v) == 0) {
                                    std::cout << "deleting v" << std::endl;
                                    vec::DELETE(v);
                                    std::cout << "deleted v" << std::endl;
                                } else {
                                    reduce(search_time, v, trivial, remainder, first_remainder, remainder_factory_id);
                                    std::cout << "after reduction : ";
                                    for (int i = 0; i < vec::GET_OCC(v); i++) {
                                        std::cout << num::GET_POS(vec::AT(v, i)) << " " << num::STRINGIFY(vec::AT(v, i)) << " ; ";
                                    }
                                    std::cout << std::endl;
                                }

                                if (l == remainder.size() || remainder[l] != v) l--;
                                std::cout << "l = " << l << std::endl;
                                std::cout << "remainder.size = " << remainder.size() << std::endl;
                            }
                        }
                    }
                } else {
//                            pos = num::GET_POS(vec::AT(vec, 0));
                    int k = util::binary_search(first_remainder, pos, 0, first_remainder.size(), util::compare_ints);
                    remainder.insert(remainder.begin() + k, vec::COPY(remainder_factory_id, vec));
                    vec::DELETE(vec);
                    first_remainder.insert(first_remainder.begin() + k, pos);
                }
            }


//            template<typename Allocator = std::allocator<jmaerte::arith::svec_node>>
            std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR> smith(s_int_matrix matrix) {

                using namespace jmaerte::arith;

                unsigned int channel_id = jmaerte::output::LOGGER.register_channel("Smith", std::cout);

                double time_elapsed;
                double allocation_generation_time = 0;
                double search_time = 0;

                auto start_time = std::chrono::steady_clock::now();

                unsigned int remainder_factory_id = arith::vec::factory::REGISTER<jmaerte::arith::vec::std_factory>();

                std::vector<vec::s_ap_int_vec> remainder;
                std::vector<int> first_remainder;
                std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR> result;
                {
                    std::map<int, vec::s_ap_int_vec> trivial;
//                    std::vector<vec::s_ap_int_vec> trivial;
//                    std::vector<int> first;
                    int count = 0;

                    while (!matrix.is_empty()) {
                        auto time = std::chrono::steady_clock::now();
                        vec::s_ap_int_vec vec = matrix.get();
                        allocation_generation_time += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - time).count();

//                        for (int i = 0; i < vec::GET_OCC(vec); i++) {
//                            num::ap_int j = vec::AT(vec, i);
//                            std::cout << num::GET_POS(j) << " " << num::STRINGIFY(j) << std::endl;
//                        }

                        if (++count % 1000 == 0) std::cout << count << " / " << matrix.rows() << std::endl;
//                        matrix = matrix.pop_front();

                        // REDUCED ROW ECHELON FORM
//
//                        int k = 0;
//                        int pos = 0;
//
//                        std::vector<int> addition_indices;
//                        std::vector<int> indices;
//                        // SINCE EVERY TRIVIAL VECTOR HAS ENTRY 0 WHERE ANY OTHER TRIVIAL VECTOR HAS
//                        // LEADING 1 THE NEEDED ADDITIONS ARE ALREADY GIVEN BY THE INITIAL VECTOR STRUCTURE.
//                        for (int i = 0; i < vec::GET_OCC(vec); i++) {
//                            pos = num::GET_POS(vec::AT(vec, i));
//                            k = util::binary_search(first, pos, k, first.size(), util::compare_ints);
//                            if (k < first.size() && first[k] == pos) {
//                                indices.push_back(pos);
//                                addition_indices.push_back(k);
//                            }
//                        }
//                        for (int j = 0; j < indices.size(); j++) {
//                            vec::REDUCE(vec, trivial[addition_indices[j]], vec::FIND_POS(vec, indices[j]));
//                        }

                        // NON REDUCED ROW ECHELON
                        reduce(search_time, vec, trivial, remainder, first_remainder, remainder_factory_id);

                    }
                    result.emplace(num::NEW(1, false, 1ULL), trivial.size());

//                    for (int j = 0; j < trivial.size(); j++) {
//                        vec::DELETE(trivial[j]);
//                    }
//                    trivial.clear();
                }

                time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start_time).count();
                output::print_time_resume("Matrix Reduction", time_elapsed, {{"allocation and generation", allocation_generation_time}, {"searching in position list", search_time}});

                arith::vec::factory::RELEASE(matrix.get_factory_id());

                int n = remainder.size();// amount of non-trivial vectors in remainder.
                jmaerte::output::LOGGER.log(channel_id, "Remainder matrix has " + std::to_string(n) + " rows.");

                for (int i = 0; i < n; i++) {

                    if (i % 100 == 0) jmaerte::output::LOGGER.log(channel_id, "Calculating Normalform " + std::to_string(i) + " / " + std::to_string(remainder.size()));

                    std::cout << num::GET_POS(vec::AT(remainder[i], 0)) << " " << num::STRINGIFY(vec::AT(remainder[i], 0)) << std::endl;
//        int j = -1;
                    vec::s_ap_int_vec temp;
//        std::vector<int> indices;
//        int pos = 0;
//        for (int row = i; row < n; row++) {
//            temp = remainder[row];
//            if (vec::GET_OCC(temp) == 0) {
//                remainder[row] = remainder[--n];
//                remainder[n] = temp;
//                vec::DELETE(temp);
//            } else {
//                pos = num::GET_POS(vec::AT(temp, 0));
//                if (pos < j || j < 0) {
//                    indices.clear();
//                    indices.push_back(row);
//                    j = pos;
//                } else if (pos == j) {
//                    indices.push_back(row);
//                }
//            }
//        }
                    if (first_remainder.size() == 0) break;
                    int block_size = 0;
                    int pos = first_remainder[0];
                    // we know that first_remainder is ordered and thus can cut off the beginning:
                    for (; block_size < first_remainder.size(); block_size++) {
                        if (first_remainder[block_size] != pos) break;
                    }
//        if (j < 0) break;
                    // is true if we reduce column, false if we reduce row
                    bool col = true;
                    while (true) {
                        if (col) {
                            std::cout << "col" << std::endl;
                            // Pivotize
                            int k = -1; // row index of pivot
                            for (int row = 0; row < block_size; row++) {
                                if (k < 0 || num::COMPARE_ABS(vec::AT(temp, 0), vec::AT(remainder[row], 0)) > 0) {
                                    temp = remainder[row];
                                    k = row;
                                }
                            }

//                            for (int i = 0; i < vec::GET_OCC(temp); i++) {
//                                std::cout << num::GET_POS(vec::AT(temp, i)) << " " << num::STRINGIFY(vec::AT(temp, i)) << " ";
//                            }
//                            std::cout << std::endl;

                            if (k < 0) return result;
                            remainder[k] = remainder[0];
                            remainder[0] = temp;
                            int occ = 0;
//                num::ap_int pre = num::PRECOMPUTE_SDIV_DIVISOR(vec::AT(remainder[i], 0));
                            for (int row = 1; row < block_size; row++) {

//                                for (int i = 0; i < vec::GET_OCC(remainder[row]); i++) {
//                                    std::cout << num::GET_POS(vec::AT(remainder[row], i)) << " " << num::STRINGIFY(vec::AT(remainder[row], i)) << " ";
//                                }
//                                std::cout << std::endl;

                                if (vec::GET_IS_ALL_SINGLE(remainder[0])) {
                                    if (num::IS_SINGLE(vec::AT(remainder[row], 0))) {
                                        num::ap_int lambda = num::NEW(
                                                !(num::GET_SIGN(vec::AT(remainder[row], 0)) ^
                                                  num::GET_SIGN(vec::AT(remainder[0], 0))),
                                                *num::ABS(vec::AT(remainder[row], 0)) / *num::ABS(vec::AT(remainder[0], 0))
                                        );
                                        if (vec::GET_IS_ALL_SINGLE(remainder[row])) {
                                            vec::ADD_ALL_SINGLE(remainder[row], 0, lambda, remainder[0], 0);
                                        } else {
                                            vec::ADD_SINGLE(remainder[row], 0, lambda, remainder[0], 0);
                                        }
                                    } else {
                                        num::ap_int lambda = num::iC_DIV(vec::AT(remainder[row], 0), *num::ABS(vec::AT(remainder[0], 0)));
                                        if (!num::GET_SIGN(vec::AT(remainder[0], 0))) num::SWITCH_SIGN(lambda);
                                        // vector remainder[row] can't be all single...
                                        vec::ADD_SINGLE(remainder[row], 1, lambda, remainder[0], 1);
                                    }
                                } else {
                                    if (num::IS_SINGLE(vec::AT(remainder[0], 0))) {
                                        if (num::IS_SINGLE(vec::AT(remainder[row], 0))) {
                                            num::ap_int lambda = num::NEW(
                                                    !(num::GET_SIGN(vec::AT(remainder[row], 0)) ^
                                                      num::GET_SIGN(vec::AT(remainder[0], 0))),
                                                    *num::ABS(vec::AT(remainder[row], 0)) / *num::ABS(vec::AT(remainder[0], 0))
                                            );
                                            if (vec::GET_IS_ALL_SINGLE(remainder[row])) {
                                                vec::ADD_SINGLE_SCALAR(remainder[row], 0, lambda, remainder[0], 0);
                                            } else {
                                                vec::ADD(remainder[row], 0, lambda, remainder[0], 0);
                                            }
                                        } else {
                                            num::ap_int lambda = num::iC_DIV(vec::AT(remainder[row], 0), *num::ABS(vec::AT(remainder[0], 0)));
                                            if (!num::GET_SIGN(vec::AT(remainder[0], 0))) num::SWITCH_SIGN(lambda);
                                            vec::ADD(remainder[row], 1, lambda, remainder[0], 1);
                                        }
                                    } else {
                                        // the first of the others can't be single neither..
                                        num::ap_int lambda = num::DIV(vec::AT(remainder[row], 0), vec::AT(remainder[0], 0));
                                        num::SWITCH_SIGN(lambda);
                                        vec::ADD(remainder[row], 1, lambda, remainder[0], 1);
                                    }
                                }
                                int next_pos;
                                if (vec::GET_OCC(remainder[row]) == 0) {
                                    vec::DELETE(remainder[row]);
                                    remainder.erase(remainder.begin() + row);
                                    first_remainder.erase(first_remainder.begin() + row);
                                    row--;
                                    block_size--;
                                } else {
                                    if ((next_pos = num::GET_POS(vec::AT(remainder[row], 0))) != pos) {
                                        temp = remainder[row];
                                        k = util::binary_search(first_remainder, next_pos, block_size, first_remainder.size(), util::compare_ints);
                                        std::copy(first_remainder.begin() + row + 1, first_remainder.begin() + k, first_remainder.begin() + row);
                                        std::copy(remainder.begin() + row + 1, remainder.begin() + k, remainder.begin() + row);
                                        first_remainder[k] = next_pos;
                                        remainder[k] = temp;
                                        block_size--;
                                        row--;
                                    }
                                }
                            }
//                            for (int row = 1; row < block_size; row++) {
//                                num::ap_int lambda = num::DIV(vec::AT(remainder[row], 0), vec::AT(remainder[i], 0));
//                                num::SWITCH_SIGN(lambda);
//                                vec::ADD(remainder[row], 1, lambda, remainder[i], 1);
//                                int next_pos;
//                                if (vec::GET_OCC(remainder[row]) == 0) {
//                                    // vector got reduced to 0
//                                    vec::DELETE(remainder[row]);
//                                    remainder.erase(remainder.begin() + row);
//                                    first_remainder.erase(first_remainder.begin() + row);
//                                    row--;
//                                    block_size--;
//                                } else {
//                                    if ((next_pos = num::GET_POS(vec::AT(remainder[row], 0))) != pos) {
//                                        temp = remainder[row];
//                                        k = util::binary_search(first_remainder, next_pos, block_size, first_remainder.size(), util::compare_ints);
//                                        std::copy(first_remainder.begin() + row + 1, first_remainder.begin() + k, first_remainder.begin() + row);
//                                        std::copy(remainder.begin() + row + 1, remainder.begin() + k, remainder.begin() + row);
//                                        first_remainder[k] = next_pos;
//                                        remainder[k] = temp;
//                                        block_size--;
//                                        row--;
//                                    }
//                                }
//                            }
//                num::DELETE(pre);
                            col = block_size > 1;
                        } else {
                            std::cout << "row" << std::endl;
                            temp = remainder[0];
                            if  (vec::GET_IS_ALL_SINGLE(temp)) {
                                for (int i = 1; i < vec::GET_OCC(temp); i++) {
                                    (vec::AT(temp, i) + 1)->single %= (vec::AT(temp, 0) + 1)->single;
                                    if ((vec::AT(temp, i) + 1)->single == 0ULL) vec::DELETE_POS(temp, i--);
                                }
                            } else {
                                vec::MOD(temp);
                            }
                            if (vec::GET_OCC(temp) == 1) {
                                auto it = result.find(vec::AT(temp, 0));
                                if (it == result.end()) {
                                    result.emplace(num::COPY(vec::AT(temp, 0)), 1u);
                                }
                                else (*it).second = (*it).second + 1u;
                                vec::DELETE(temp);
                                remainder.erase(remainder.begin());
                                first_remainder.erase(first_remainder.begin());
                                n--;
                                break;
                            } else {
                                int k = -1;
                                for (int row = 0; row < vec::GET_OCC(temp); row++) {
                                    if (k < 0 || num::COMPARE_ABS(vec::AT(temp, k), vec::AT(temp, row)) > 0) k = row;
                                }
                                vec::SWAP_VALUES(temp, 0, k);
                                vec::s_ap_int_vec temp_row;
                                for(int row = 1; row < n; row++) {
                                    temp_row = remainder[row];
                                    int l = vec::FIND_POS(temp_row, pos);
                                    if (l < vec::GET_OCC(temp_row) && num::GET_POS(vec::AT(temp_row, l)) == num::GET_POS(vec::AT(temp_row, k))) {
                                        num::SET_POS(vec::AT(temp_row, l), pos);
                                        vec::PUT(temp_row, vec::AT(temp_row, l));
                                        vec::DELETE_POS(temp_row, l + 1);
                                        // get this vector to the front
                                        std::copy_backward(remainder.begin() + 1, remainder.begin() + row - 1, remainder.begin() + row);
                                        std::copy_backward(first_remainder.begin() + 1, first_remainder.begin() + row - 1, first_remainder.begin() + row);
                                        remainder[1] = temp_row;
                                        first_remainder[1] = pos;
                                        block_size++;
                                    }
                                }
                                col = block_size > 1;
                            }
                        }
                    }
                }
                arith::vec::factory::RELEASE(remainder_factory_id);
                jmaerte::output::LOGGER.release_channel(channel_id);

                return result;
            }
        }
    }
}