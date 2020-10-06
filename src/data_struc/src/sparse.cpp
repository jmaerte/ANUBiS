//
// Created by jmaerte on 29.11.19.
//

#include "../include/data_types/sparse.hpp"
#include "utils.hpp"


template<typename T>
sparse<T>::sparse() {}

template<typename T>
sparse<T>::sparse(std::vector<std::pair<int, T>> v): vec(v) {}

template<typename T>
int sparse<T>::non_zero() const {
    return vec.size();
}

// Aux
template<typename T>
int compare_pair_int(const std::pair<int, T>& pair, const int& i) {
    return pair.first - i;
}

template<typename T>
int sparse<T>::index(int i) const {
    return binary_search(vec, i, 0, vec.size(), compare_pair_int);
}

template class sparse<double>;
template class sparse<int>;