//
// Created by jmaerte on 29.11.19.
//

#pragma once

#include <vector>
#include <tuple>

template<typename T>
class sparse {
public:

    std::vector<std::pair<int, T>> vec;
    sparse();
    sparse(std::vector<std::pair<int, T>>);

    int non_zero() const;

    int index(int i) const;
    void set(std::vector<std::pair<int, T>>& n_vec) {
        vec = n_vec;
    }

    typename std::vector<std::pair<int, T>>::const_iterator begin() const {
        return vec.begin();
    }

    typename std::vector<std::pair<int, T>>::const_iterator end() const {
        return vec.end();
    }

    int operator [](int i) const    {
        return vec[i].first;
    }

    T operator ()(int i) const    {
        return vec[i].second;
    }
};