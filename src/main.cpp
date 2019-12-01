/*
 * main.cpp
 *
 *  Created on: 24.11.2019
 *      Author: jmaerte
 */

#include <anubis/complex.hpp>
#include <iostream>
#include <vector>

int main() {
    auto * c = jmaerte::anubis::s_tree::from_file("/Downloads/c88", 3);
//    std::vector<double> spec = c->laplacian_spectrum(0);
//    c->s_insert(std::vector<int>{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20});
//    c->print();
    return 0;
}

