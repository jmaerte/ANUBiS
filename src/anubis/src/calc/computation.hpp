//
// Created by jmaerte on 29.11.19.
//

#pragma once

#include <tuple>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <map>
#include "data_types/lazy/stream.hpp"
#include "data_types/lin/sparse.hpp"
#include "utils.hpp"
#include <arithmetic/typedef.hpp>
#include <arithmetic/operator.hpp>

double machine_eps = 1e-8;

/***********************************************************************************************************************
 * Auxiliary
 **********************************************************************************************************************/

double dot(const stream<double> & x, const stream<double> & y) {
    return zip(x, y, [](double a, double b) {
        return a * b;
    }).flatten([](double a, double b) {
        return a + b;
    });
}

stream<double> matmul(const stream<sparse<double>> matrix, const std::vector<double> v) {
    return transform(matrix, [v](sparse<double> s) {
        double sum = 0;
        for (auto it = s.vec.begin(); it != s.vec.end(); it++) {
            sum += v[(*it).first] * (*it).second;
        }
        return sum;
    });
}

stream<double> scale(const stream<double> & v, const double n) {
    return transform(v, [n](double d) {
        return n * d;
    });
}

/***********************************************************************************************************************
 * Eigenvalue algorithms
 **********************************************************************************************************************/

std::pair<std::vector<double>, std::vector<double>> lanczos(stream<sparse<double>> & matrix, int n) {
    stream<double> x = drand().take(n);
    double x_norm = sqrt(dot(x, x));
    stream<double> q = scale(x, 1 / x_norm);
    stream<double> r = matmul(matrix, q.vectorize());
    std::vector<double> alpha;
    std::vector<double> beta;

    alpha.push_back(dot(r, q));
    r = zip(r, scale(q, -alpha[0]), [](double a, double b) {
        return a + b;
    });
    beta.push_back(sqrt(dot(r, r)));
    for (int i = 1; i < n; i++) {
        stream<double> v = q;
        q = transform(r, [&beta, &i](double d) {
            return 1 / beta[i - 1] * d;
        });
        r = matmul(matrix, q.vectorize());
        r = zip(r, scale(v, -beta[i - 1]), [](double a, double b) {
            return a + b;
        });
        alpha.push_back(dot(q,r));
        r = zip(r, scale(q, -alpha[i]), [](double a, double b) {
            return a + b;
        });
        double b = sqrt(dot(r,r));
        if (b < machine_eps) break;
        beta.push_back(b);
    }
    return std::pair<std::vector<double>, std::vector<double>>(alpha, beta);
}

std::vector<double> QR(std::vector<double> & alpha, std::vector<double> & beta) {
    double sub = 1;
    int n = alpha.size();
    std::vector<double> a, b, c;
    while (sub > 1e-8) {
        a = alpha;
        b = beta;
        c = std::vector<double>(n - 2);
        std::vector<double> theta (n - 1);
        theta[0] = atan(- beta[0] / alpha[0]);
        double gamma = cos(theta[0]);
        double sigma = sin(theta[0]);
        a[0] = gamma * alpha[0] - sigma * beta[0];
        b[0] = gamma * beta[0] - sigma * alpha[1];
        c[0] = - sigma * beta[1];
        a[1] = sigma * beta[0] + gamma * alpha[1];
        b[1] = gamma * beta[1];
        for (int i = 1; i < n - 2; i++) {
            theta[i] = atan(- beta[i] / a[i]);
            sigma = sin(theta[i]);
            gamma = cos(theta[i]);
            a[i] = gamma * a[i] - sigma * beta[i];
            double temp = gamma * b[i] - sigma * a[i + 1];
            a[i + 1] = sigma * b[i] + gamma * a[i + 1];
            c[i] = - sigma * b[i + 1];
            b[i + 1] = gamma * b[i + 1];
            b[i] = temp;
        }
        theta[n - 2] = atan(- beta[n - 2] / a[n - 2]);
        gamma = cos(theta[n - 2]);
        sigma = sin(theta[n - 2]);
        a[n - 2] = gamma * a[n - 2] - sigma * beta[n - 2];
        double temp = gamma * b[n - 2] - sigma * a[n - 1];
        a[n - 1] = sigma * b[n - 2] + gamma * a[n - 1];
        b[n - 2] = temp;

        sub = -1;
        for (int i = 0; i < n - 1; i++) {
            gamma = cos(theta[i]);
            sigma = sin(theta[i]);
//            if (i < n - 2)
//                assert(fabs(sin(theta[i + 1]) * alpha[i] + cos(theta[i + 1]) * (sigma * a[i] + gamma * b[i]) - -sigma * a[i + 1]) < 1e-8);
            alpha[i] = gamma * a[i] - sigma * b[i];
            beta[i] = - sigma * a[i + 1];
            if (fabs(beta[i]) > sub) sub = fabs(beta[i]);
            a[i + 1] = gamma * a[i + 1];
        }
        alpha[n - 1] = a[n - 1];
    }
    return alpha;
}

std::vector<double> eigen(stream<sparse<double>> && matrix, int n) {
    std::pair<std::vector<double>, std::vector<double>> tridiag = lanczos(matrix, n);
    return QR(tridiag.first, tridiag.second);
}

/***********************************************************************************************************************
 * Elimination algorithms
 **********************************************************************************************************************/

int compare_to(const sparse<int>& a, const sparse<int>& b) {
    return a[0] - b[0];
};

int compare_ints(const int& a, const int& b) {
    return a - b;
}

void add (sparse<int>& a, int lambda, sparse<int>& b) {
    std::vector<std::pair<int, int>> res;
    auto it_a = a.begin();
    auto it_b = b.begin();
    std::pair<int, int> tmp;
    int calc = 0;
    while (it_a != a.end() && it_b != b.end()) {
        if (it_a->first < it_b->first) {
            res.push_back(*it_a);
        } else if (it_a -> first > it_b->first) {
            tmp = *it_b;
            tmp.second = lambda * tmp.second;
            res.push_back(tmp);
        } else {
            if ((calc = it_a->second + lambda * it_b->second) != 0) {
                res.emplace_back(it_a->first, calc);
            }
        }
    }
    while (it_a != a.end()) {
        res.push_back(*it_a);
    }
    while (it_b != b.end()) {
        res.emplace_back(it_b->first, lambda * it_b->second);
    }
    a.set(res);
}

/***********************************************************************************************************************
 * Smith normalform
 **********************************************************************************************************************/
std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR> smith(stream<vec::s_vec>&& matrix) {
    return smith(std::forward<stream<vec::s_vec>>(matrix));
}

std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR> smith(stream<vec::s_vec>& matrix) {
    //    std::cout << "hom" << std::endl;
//    ULL* a = new unsigned long long;
//    *a = ((1ULL << 31) + (1ULL << 32));
//    std::cout << *a << std::endl;
//    ULL* b = new unsigned long long;
//    *b = (1ULL << 63) | 1ULL;
//    ULL* c = new unsigned long long[2];
//    *c = (0ULL);
//    KMUL(c, false, a, 2, false, b, 2, false);
//    std::cout << *c << " " << *(c + 1) << std::endl;
//    delete a;
//    delete b;
//    delete[] c;

    using namespace jmaerte::arith;

    std::vector<vec::s_vec> remainder;
    std::vector<int> first_remainder;
    std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR> result;
    {
        std::vector<vec::s_vec> trivial;
        std::vector<int> first;
        int count = 0;
        while (!matrix.is_empty()) {
            vec::s_vec vec = matrix.get();
            matrix = matrix.pop_front();
            int k = 0;
            int pos = 0;
            for (int i = 0; i < vec::GET_OCC(vec);) {
                pos = num::GET_POS(vec::AT(vec, i));
                std::cout << vec::GET_OCC(vec) << std::endl;
                k = binary_search(first, pos, k, first.size(), compare_ints);
                if (k < first.size() && first[k] == pos) {
                    vec::ADD(vec, 0, vec::AT(vec, i), trivial[k], 0);
                } else i++;
            }
            if (!vec::GET_OCC(vec)) {
                vec::DELETE(vec);
                continue;
            }
            if (num::GET_OCC(vec::AT(vec, 0)) == 1 && *((vec + 2)->value) == 1ULL) {
                // assert that the leading entry is -1.
                if (!num::GET_SIGN(vec::AT(vec, 0))) vec::SWITCH_SIGNS(vec);
                pos = num::GET_POS(vec::AT(vec, 0));
                k = binary_search(first, pos, 0, first.size(), compare_ints);
                trivial.insert(trivial.begin() + k, vec);
                first.insert(first.begin() + k, pos);
                for (vec::s_vec& v : remainder) {
                    int j = vec::FIND_POS(v, pos);
                    if (j < vec::GET_OCC(v) && num::GET_POS(vec::AT(v, j)) == pos) {
                        vec::ADD(v, 0, vec::AT(v, j), vec, 0);
                    }
                }
            } else {
                pos = num::GET_POS(vec::AT(vec, 0));
                k = binary_search(first_remainder, pos, 0, first_remainder.size(), compare_ints);
                remainder.insert(remainder.begin() + k, vec);
                first_remainder.insert(first_remainder.begin() + k, pos);
            }
        }
        result.emplace(num::NEW(1, false, 1ULL), first.size());
    }

    int n = remainder.size();// amount of non-trivial vectors in remainder.
    for (int i = 0; i < n; i++) {
        if (i % 100 == 0) std::cout << "\rCalculating Smith Normalform " << i << " / " << remainder.size();
//        int j = -1;
        vec::s_vec temp;
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
                    vec::ADD(remainder[row], 1, lambda, remainder[i], 1);
                    int next_pos;
                    if (vec::GET_OCC(remainder[row]) == 0) {
                        // vector got reduced to 0
                        vec::DELETE(remainder[row]);
                        remainder.erase(remainder.begin() + row);
                        first_remainder.erase(first_remainder.begin() + row);
                        row--;
                        block_size--;
                    } else {
                        if ((next_pos = num::GET_POS(vec::AT(remainder[row], 0))) != pos) {
                            temp = remainder[row];
                            k = binary_search(first_remainder, next_pos, block_size, first_remainder.size(), compare_ints);
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
                    vec::s_vec temp_row;
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
    return result;
}