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
#include "../../include/ANUBiS/arithmetic.hpp"

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
std::map<num, unsigned int, NUM_COMPARATOR> smith(stream<s_vec>&& matrix) {
    return smith(std::forward<stream<s_vec>>(matrix));
}

std::map<num, unsigned int, NUM_COMPARATOR> smith(stream<s_vec>& matrix) {
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
    std::vector<s_vec> remainder;
    std::map<num, unsigned int, NUM_COMPARATOR> result;
    {
        std::vector<s_vec> trivial;
        std::vector<int> first;
        int count = 0;
        while (!matrix.is_empty()) {
            s_vec vec = matrix.get();
            matrix = matrix.pop_front();
            int k = 0;
            int pos = 0;
            for (int i = 0; i < GET_VEC_OCC(vec);) {
                pos = GET_NUM_POS(VEC_AT(vec, i));
                std::cout << GET_VEC_OCC(vec) << std::endl;
                k = binary_search(first, pos, k, first.size(), compare_ints);
                if (k < first.size() && first[k] == pos) {
                    ADD(vec, VEC_AT(vec, i), trivial[k]);
                } else i++;
            }
            if (!GET_VEC_OCC(vec)) {
                DEL_VEC(vec);
                continue;
            }
            if (GET_NUM_OCC(VEC_AT(vec, 0)) == 1 && *((vec + 2)->value) == 1ULL) {
                // assert that the leading entry is -1.
                if (!GET_NUM_SIGN(VEC_AT(vec, 0))) SWITCH_VEC_SIGN(vec);
                pos = GET_NUM_POS(VEC_AT(vec, 0));
                k = binary_search(first, pos, 0, first.size(), compare_ints);
                trivial.insert(trivial.begin() + k, vec);
                first.insert(first.begin() + k, pos);
                for (s_vec& v : remainder) {
                    int j = VEC_FIND_POS(v, pos);
                    if (j < GET_VEC_OCC(v) && GET_NUM_POS(VEC_AT(v, j)) == pos) {
                        ADD(v, VEC_AT(v, j), vec);
                    }
                }
            } else {
                remainder.push_back(vec);
            }
        }
        result.emplace(NEW_NUM(1, false, 1ULL), first.size());
    }

    int n = remainder.size();// amount of non-trivial vectors in remainder.
    for (int i = 0; i < n; i++) {
        if (i % 100 == 0) std::cout << "\rCalculating Smith Normalform " << i << " / " << remainder.size();
        int j = -1;
        s_vec temp;
        std::vector<int> indices;
        int pos = 0;
        for (int row = i; row < n; row++) {
            temp = remainder[row];
            if (GET_VEC_OCC(temp) == 0) {
                remainder[row] = remainder[--n];
                remainder[n] = temp;
                DEL_VEC(temp);
            } else {
                pos = GET_NUM_POS(VEC_AT(temp, 0));
                if (pos < j || j < 0) {
                    indices.clear();
                    indices.push_back(row);
                    j = pos;
                } else if (pos == j) {
                    indices.push_back(row);
                }
            }
        }
        if (j < 0) break;
        // is true if we reduce column, false if we reduce row
        bool col = true;
        while (true) {
            if (col) {
                // Pivotize
                int k = -1; // row index of pivot
                int h = 0; // index in indices where the pivot lays
                for (int l = 0; l < indices.size(); l++) {
                    int row = indices[l];
                    if (k < 0 || COMPARE_ABS(VEC_AT(temp, 0), VEC_AT(remainder[row], 0)) > 0) {
                        temp = remainder[row];
                        k = row;
                        h = l;
                    }
                }
                if (k < 0) return result;
                remainder[k] = remainder[i];
                remainder[i] = temp;
                if (indices[0] != i) indices.erase(indices.begin() + h);
                if (indices[0] == i) indices.erase(indices.begin());
                std::vector<int> next_indices;
                next_indices.reserve(indices.size() - 1);
                int occ = 0;
                for (int l = 0; l < indices.size(); l++) {
                    int row = indices[l];
                    num lambda = DIVIDE(VEC_AT(remainder[row], 0), VEC_AT(remainder[i], 0));
                    SWITCH_NUM_SIGN(lambda);
                    ADD(remainder[row], lambda, remainder[i]);
                    if (GET_NUM_POS(VEC_AT(remainder[row], 0)) == GET_NUM_POS(VEC_AT(remainder[i], 0))) next_indices[occ++] = row;
                }
                indices = next_indices;
                if (next_indices.size() > 0) {
                    if (GET_VEC_OCC(remainder[i]) > 0) indices.push_back(i);
                    col = true;
                } else col = false;
            } else {
                temp = remainder[i];
                for (int l = 1; l < GET_VEC_OCC(temp); l++) {
                    MOD(VEC_AT(temp, l), VEC_AT(temp, 0));
                    if (GET_NUM_OCC(VEC_AT(temp, l)) == 0) {
                        DEL_POS(temp, l);
                        l--;
                    }
                }
                if (GET_VEC_OCC(temp) == 1) {
                    auto it = result.find(VEC_AT(temp, 0));
                    if (it == result.end()) {
                        result.emplace(COPY_NUM(VEC_AT(temp, 0)), 1u);
                    }
                    else (*it).second = (*it).second + 1u;
                    DEL_VEC(temp);
                    break;
                } else {
                    int k = -1;
                    for (int row = 0; row < GET_VEC_OCC(remainder[i]); row++) {
                        if (k < 0 || COMPARE_ABS(VEC_AT(remainder[i], k), VEC_AT(remainder[i], row)) > 0) k = i;
                    }
                    VEC_SWAP_VALUES(remainder[i], 0, k);
                    for(int row = i + 1; row < n; row++) {
                        int l = VEC_FIND_POS(remainder[row], GET_NUM_POS(VEC_AT(remainder[i], k)));
                        if (l < GET_VEC_OCC(remainder[row]) && GET_NUM_POS(VEC_AT(remainder[row], l)) == GET_NUM_POS(VEC_AT(remainder[row], k))) {
                            SET_NUM_POS(VEC_AT(remainder[row], l), GET_NUM_POS(VEC_AT(remainder[i], 0)));
                            VEC_PUT(remainder[row], VEC_AT(remainder[row], l));
                            DEL_POS(remainder[row], l + 1);
                            indices.push_back(row);
                        }
                    }
                    if (indices.size() == 0) col = false;
                    else {
                        indices.push_back(i);
                        col = true;
                    }
                }
            }
        }
    }
    return result;
}