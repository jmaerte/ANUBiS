/*
 * main.cpp
 *
 *  Created on: 24.11.2019
 *      Author: jmaerte
 */

#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <ANUBiS/complex.hpp>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
static const std::string slash="\\";
#else
static const std::string slash="/";
#endif

int main() {
    auto start = std::chrono::high_resolution_clock::now();
//    auto c = jmaerte::anubis::s_tree::from_file(getenv("HOME") + slash + "Downloads" + slash + "c88");
//
//
    auto c = jmaerte::anubis::s_list::from_file("W:\\Downloads\\c88");

//
//    std::vector<double> spec = c->laplacian_spectrum(0);
//    c->s_insert(std::vector<int>{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20});
//    c->print();
    c->generate(0);
    c->generate(1);
    c->generate(2);
    c->generate(3);
    c->generate(4);
    c->generate(5);
    c->generate(6);
    c->generate(7);

//    c->homology(1);

    for (auto i : c->f_vector()) {
        std::cout << i << std::endl;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::cout << "Time elapsed: " << (finish - start).count() / 1e9 << "s" << std::endl;
    delete c;
    system("pause");
    return 0;
}

