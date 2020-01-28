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

std::map<int, unsigned int> smith(stream<sparse_vector>&& stream) {
    sparse_vector[] matrix; // array of pointers to sparse vector positions
    std::map<sparse_vector, unsigned int> result;
    while(!stream.is_empty()) {
        sparse_vector vec = stream.get();
        
    }





    std::vector<sparse<int>> remainder;
    std::map<int, unsigned int> result;
    {
        std::vector<sparse<int>> trivial;
        std::vector<int> first;
        int count = 0;
        // Preprocessing
        while (!matrix.is_empty()) {
            sparse<int> vec = matrix.get();
            matrix = matrix.pop_front();
            int k = 0;
            for (int i = 0; i < vec.non_zero();) {
                std::cout << vec.non_zero() << std::endl;
                k = binary_search(first, vec[i], k, first.size(), compare_ints);
                if (k < first.size() && first[k] == vec[i]) {
                    add(vec, - vec(i), trivial[k]);
                } else i++;
            }
            if (vec.non_zero() == 0) continue; // vector was linear combination of trivial ones.
            if (vec(0) == 1 || vec(0) == -1) {
                k = binary_search(first, vec[0], 0, first.size(), compare_ints);
                trivial.insert(trivial.begin() + k, vec);
                first.insert(first.begin() + k, vec[0]);
                for (sparse<int>& v : remainder) {
                    int j = v.index(vec[0]);
                    if (j < v.non_zero() && v(j) == vec[0]) add(v, - v(j) * vec(0), vec);
                }
            }
        }
        result = {{1, first.size()}};
    }

    // Smith
    int n = remainder.size();
    for (int i = 0; i < n; i++) {
        if (i % 100 == 0) std::cout << "\rCalculating Smith Normalform " << i << " / " << remainder.size();
        int j = -1;
        sparse<int> temp;
        std::vector<int> indices;
        for (int row = i; row < remainder.size(); row++) {
            if (remainder[row].non_zero() == 0) {
                temp = remainder[row];
                remainder[row] = remainder[--n];
                remainder[n] = temp;
            } else if (remainder[row][0] < j || j < 0) {
                indices.clear();
                indices.push_back(row);
                j = remainder[row][0];
            } else if (remainder[row][0] == j) {
                indices.push_back(row);
            }
        }
        if (j < 0) break;
        // is true if we reduce column, false if we reduce row
        bool col = true;
        while (true) {
            if (col) {
                // Pivot
                int k = -1; // row index of pivot
                int h = 0; // index in indices where the pivot lays
                for (int l = 0; l < indices.size(); l++) {
                    int row = indices[l];
                    if (k < 0 || abs(remainder[k](0)) > abs(remainder[row](0))) {
                        k = row;
                        h = l;
                    }
                }
                if (k < 0) return result;
                temp = remainder[i];
                remainder[i] = remainder[k];
                remainder[k] = temp;
                if (indices[0] != i) indices.erase(indices.begin() + h);
                if (indices[0] == i) indices.erase(indices.begin());
                std::vector<int> next_indices;
                next_indices.reserve(indices.size() - 1);
                int occ = 0;
                for (int l = 0; l < indices.size(); l++) {
                    int row = indices[l];
                    int lambda = remainder[row](0) / remainder[i](0);
                    add(remainder[row], - lambda, remainder[i]);
                    if (remainder[row][0] == remainder[i][0]) next_indices[occ++] = row;
                }
                indices = next_indices;
                if (next_indices.size() > 0) {
                    if (remainder[i].non_zero() > 0) indices.push_back(i);
                    col = true;
                } else col = false;
            } else {
                for (int l = 1; l < remainder[i].non_zero(); l++) {
                    remainder[i].get(l) %= remainder[i](0);
                    if (remainder[i].get(l) == 0) {
                        remainder[i].remove(l);
                        l--;
                    }
                }
                if (remainder[i].non_zero() == 1) {
                    auto it = result.find(abs(remainder[i](0)));
                    if (it == result.end()) result.emplace(abs(remainder[i](0)), 1);
                    else (*it).second = (*it).second + 1;
                    break;
                } else {
                    int k = -1;
                    for (int row = 0; row < remainder[i].non_zero(); row++) {
                        if (k < 0 || abs(remainder[i](k)) > abs(remainder[i](row))) k = i;
                    }
                    int curr = remainder[i](0);
                    remainder[i].get(0) = remainder[i](k);
                    remainder[i].get(k) = curr;
                    for(int row = i + 1; row < n; row++) {
                        int l = remainder[row].index(remainder[i](k));
                        if (l < remainder[row].non_zero() && remainder[row][l] == remainder[row][k]) {
                            curr = remainder[row](l);
                            remainder[row].remove(l);
                            remainder[row].insert(0, remainder[i][0], curr);
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