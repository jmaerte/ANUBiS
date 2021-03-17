//
// Created by jmaerte on 16.03.20.
//

#include <arithmetic/typedef.hpp>
#include <arithmetic/operator.hpp>
#include <arithmetic/aux.hpp>
#include <iostream>

using namespace jmaerte::arith;

int main() {
    num::ap_int x, y, s, t;

    num::ap_int a, b;

    num::NEW(a, 0, true, 3);
    num::NEW(b, 0, false, 3);

    num::GCD(s, t, x, y, a, b);

    std::cout << num::STRINGIFY(x) << " " << num::STRINGIFY(y) << " " << num::STRINGIFY(s) << " " << num::STRINGIFY(t) << std::endl;

    return 0;
}