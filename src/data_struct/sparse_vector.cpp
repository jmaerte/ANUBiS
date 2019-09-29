//
// Created by maertej on 29.09.19.
//

#include "sparse_vector.hpp"


template<typename T>
sparse_vector<T> operator*(const T &factor, const sparse_vector<T> &v) {
    if (factor == 0) return sparse_vector<T>();
    std::vector<unsigned int> new_indices = v.indices;
    std::vector<T> new_vals = v.values;
    for (auto &val : new_vals) {
        val = factor * val;
    }
    return sparse_vector<T>(new_indices, new_vals);
}

template<typename T>
void sparse_vector<T>::operator-=(const sparse_vector &v) {
    auto &a = indices.begin();
    auto &a_val = values.begin();
    auto &b = v.indices.begin();
    auto &b_val = v.values.begin();
    for (; a != indices.end() && b != v.indices.end(); ) {
        if (*a > *b) {
            indices.insert(a, *b);
            values.insert(a_val, -*b_val);
            a++;
            a_val++;
            b++;
            b_val++;
        } else if (*a < *b) {
            a++;
            a_val++;
        } else {
            *a_val -= *b_val;
            a++;
            a_val++;
            b++;
            b_val++;
        }
    }
    while (b != v.indices.end()) {
        indices.push_back(-*b_val);
        b++;
        b_val++;
    }
}

template<typename T>
bool sparse_vector<T>::operator>(const sparse_vector<T> &v) const {
    return first_index() > v.first_index();
}

template<typename T>
bool sparse_vector<T>::operator<(const sparse_vector<T> &v) const {
    return first_index() < v.first_index();
}

template<typename T>
int sparse_vector<T>::first_index() const {
    if (indices.size() > 0) return indices.front();
    else return -1;
}

template<typename T>
T sparse_vector<T>::first_val() const {
    if (values.size() > 0) return values.front();
    else return 0;
}