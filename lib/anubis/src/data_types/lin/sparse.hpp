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
};

#include "sparse.cpp"