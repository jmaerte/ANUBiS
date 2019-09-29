//
// Created by maertej on 29.09.19.
//

#include "anubis/util/shifter.hpp"
#include <chrono>
#include <iostream>

int main() {
    auto start = std::chrono::system_clock::now();
    shifter_binary bin (16, 32);
    while (bin.has_next()) {
        //std::cout << bin.get() << std::endl;
        bin.shift();
    }
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start).count() << std::endl;
    return 0;
}