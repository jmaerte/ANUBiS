//
// Created by jmaerte on 24.03.20.
//

#include "algebra/reduction.hpp"
#include <arithmetic/operator.hpp>
#include "util/search.hpp"

namespace jmaerte {
    namespace algebra {
        namespace reduction {

            using namespace jmaerte::arith;

            std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR> smith(s_int_matrix matrix) {

                using namespace jmaerte::arith;

                std::vector<vec::s_ap_int_vec> remainder;
                std::vector<int> first_remainder;
                std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR> result;
                {
                    std::vector<vec::s_ap_int_vec> trivial;
                    std::vector<int> first;
                    int count = 0;
                    while (!matrix.is_empty()) {
                        vec::s_ap_int_vec vec = matrix.get();
                        if (++count % 1000 == 0) std::cout << count << std::endl;
                        matrix = matrix.pop_front();
                        int k = 0;
                        int pos = 0;
                        for (int i = 0; i < vec::GET_OCC(vec);) {
//                            std::cout << i << std::endl;
                            pos = num::GET_POS(vec::AT(vec, i));
                            k = util::binary_search(first, pos, k, first.size(), util::compare_ints);
                            if (k < first.size() && first[k] == pos) {
//                                std::cout << "pos: " << pos << ", k: " << k << std::endl;
                                vec::REDUCE(&vec, trivial[k], i);
                            } else i++;
                        }
//                        std::cout << "deciding what to do with vector" << std::endl;
                        if (!vec::GET_OCC(vec)) {
                            vec::DELETE(&vec);
                        } else if (num::GET_OCC(vec::AT(vec, 0)) == 1 && *((vec + 2)->value) == 1ULL) {
                            // assert that the leading entry is -1.
                            if (!num::GET_SIGN(vec::AT(vec, 0))) vec::SWITCH_SIGNS(vec);
                            pos = num::GET_POS(vec::AT(vec, 0));
                            k = util::binary_search(first, pos, 0, first.size(), util::compare_ints);
                            trivial.insert(trivial.begin() + k, vec);
                            first.insert(first.begin() + k, pos);
                            for (vec::s_ap_int_vec& v : remainder) {
                                int j = vec::FIND_POS(v, pos);
                                if (j < vec::GET_OCC(v) && num::GET_POS(vec::AT(v, j)) == pos) {
//                                    std::cout << "adding to remainder" << std::endl;
//                                    vec::ADD(&v, j + 1, num::COPY(vec::AT(v, j)), vec, 1);
//                                    std::cout << "out of addition" << std::endl;
//                                    vec::DELETE_POS(v, j);
                                    vec::REDUCE(&v, vec, j);
                                }
                            }
                        } else {
//                            std::cout << "adding vector to remainder list" << std::endl;
                            pos = num::GET_POS(vec::AT(vec, 0));
                            k = util::binary_search(first_remainder, pos, 0, first_remainder.size(), util::compare_ints);
                            remainder.insert(remainder.begin() + k, vec);
                            first_remainder.insert(first_remainder.begin() + k, pos);
                        }
                    }
                    result.emplace(num::NEW(1, false, 1ULL), first.size());
                    for (vec::s_ap_int_vec& n : trivial) {
//                        for (int i = 0; i < vec::GET_OCC(n); i++) {
//                            std::cout << num::STRINGIFY(vec::AT(n, i)) << " ";
//                        }
//                        std::cout << std::endl;
                        vec::DELETE(&n);
                    }
                }
                std::cout << std::endl;

                int n = remainder.size();// amount of non-trivial vectors in remainder.
                std::cout << "Remainder matrix has " << n << " rows." << std::endl;
                for (int i = 0; i < n; i++) {
                    if (i % 100 == 0) std::cout << "\rCalculating Smith Normalform " << i << " / " << remainder.size();
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
                    for (; block_size < first_remainder.size(); block_size++) {
                        if (first_remainder[block_size] != pos) break;
                    }
//        if (j < 0) break;
                    // is true if we reduce column, false if we reduce row
                    bool col = true;
                    while (true) {
                        if (col) {
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
                            int occ = 0;
//                num::ap_int pre = num::PRECOMPUTE_SDIV_DIVISOR(vec::AT(remainder[i], 0));
                            for (int row = 1; row < block_size; row++) {
                                num::ap_int lambda = num::DIV(vec::AT(remainder[row], 0), vec::AT(remainder[i], 0));
                                num::SWITCH_SIGN(lambda);
                                vec::ADD(&remainder[row], 1, lambda, remainder[i], 1);
                                int next_pos;
                                if (vec::GET_OCC(remainder[row]) == 0) {
                                    // vector got reduced to 0
                                    vec::DELETE(&remainder[row]);
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
//                num::DELETE(pre);
                            col = block_size <= 1;
                        } else {
                            temp = remainder[0];
                            vec::MOD(temp);
                            if (vec::GET_OCC(temp) == 1) {
                                auto it = result.find(vec::AT(temp, 0));
                                if (it == result.end()) {
                                    result.emplace(num::COPY(vec::AT(temp, 0)), 1u);
                                }
                                else (*it).second = (*it).second + 1u;
                                vec::DELETE(&temp);
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
                                        vec::PUT(&temp_row, vec::AT(temp_row, l));
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
                return result;
            }
        }
    }
}