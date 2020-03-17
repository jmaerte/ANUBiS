//
// Created by jmaerte on 16.03.20.
//

#include <arithmetic/typedef.hpp>
#include <arithmetic/operator.hpp>
#include <iostream>

using namespace jmaerte::arith::num;

int main() {
    ap_int n = NEW(1, false, 1ULL << 63 | 1ULL << 62 | 1ULL << 30);
    std::cout << GET_OCC(n) << " " << *GET_ABS_DATA(n) << std::endl;
    ap_int m = NEW(1, false, 1ULL << 30 | 1ULL << 29);
    std::cout << GET_OCC(m) << " " << *GET_ABS_DATA(m) << std::endl;
    aux::SUB(n, 2, 1, m);
    std::cout << GET_OCC(n) << " " << *GET_ABS_DATA(n) << std::endl;
}