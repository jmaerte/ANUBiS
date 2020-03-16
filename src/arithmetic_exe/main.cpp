//
// Created by jmaerte on 16.03.20.
//

#include <arithmetic/typedef.hpp>
#include <arithmetic/operator.hpp>
#include <iostream>

int main() {
    num n = NEW_NUM(1, false, 1ULL << 63 | 1ULL << 62 | 1ULL << 30);
    std::cout << GET_NUM_OCC(n) << " " << *GET_ABS_DATA(n) << std::endl;
    num m = NEW_NUM(1, false, 1ULL << 30 | 1ULL << 29);
    std::cout << GET_NUM_OCC(m) << " " << *GET_ABS_DATA(m) << std::endl;
    SUB_NUM(n, 2, 1, m);
    std::cout << GET_NUM_OCC(n) << " " << *GET_ABS_DATA(n) << std::endl;
}