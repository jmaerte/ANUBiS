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

    typename std::vector<std::pair<int, T>>::iterator begin() const {
        return vec.begin();
    }

    typename std::vector<std::pair<int, T>>::iterator end() const {
        return vec.end();
    }

    int operator [](int i) const    {
        return vec[i].first;
    }

    T operator ()(int i) const    {
        return vec[i].second;
    }
};