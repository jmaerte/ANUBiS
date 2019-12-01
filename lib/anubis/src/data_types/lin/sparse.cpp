//
// Created by jmaerte on 29.11.19.
//

#include "sparse.hpp"


template<typename T>
sparse<T>::sparse() {}

template<typename T>
sparse<T>::sparse(std::vector<std::pair<int, T>> v): vec(v) {}

template class sparse<double>;
template class sparse<int>;