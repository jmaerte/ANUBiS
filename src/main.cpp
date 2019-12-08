/*
 * main.cpp
 *
 *  Created on: 24.11.2019
 *      Author: jmaerte
 */

#include <anubis/complex.hpp>
#include <iostream>
#include <vector>
#include <chrono>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
static const std::string slash="\\";
#else
static const std::string slash="/";
#endif

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    auto * c = jmaerte::anubis::s_tree::from_file("C:" + slash + "Users" + slash + "Ina" + slash + "Downloads" + slash + "c88");
    auto finish = std::chrono::high_resolution_clock::now();
    std::cout << "Time elapsed: " << (finish - start).count() / 1e9 << "s" << std::endl;
//    std::vector<double> spec = c->laplacian_spectrum(0);
//    c->s_insert(std::vector<int>{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20});
//    c->print();
    for (auto i : c->f_vector()) {
        std::cout << i << std::endl;
    }
    delete c;
    return 0;
}

