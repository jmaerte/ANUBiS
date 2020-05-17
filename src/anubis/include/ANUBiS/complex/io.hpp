//
// Created by jmaerte on 12.05.20.
//

#ifndef ANUBIS_SUPERBUILD_IO_HPP
#define ANUBIS_SUPERBUILD_IO_HPP

#include "../complex.hpp"

namespace jmaerte {
    namespace anubis {
        namespace io {
            template<typename complex_constructor>
            auto complex_from_file(
                    complex_constructor constructor,
                    const std::string & path,
                    int sceleton,
                    const std::string & sep,
                    const std::string & set_openers,
                    const std::string & set_closers
            ) -> typename std::result_of<complex_constructor(std::string, int)>::type;

            template<typename complex_constructor>
            auto complex_from_facets(
                    complex_constructor constructor,
                    const std::vector<std::vector<unsigned int> *> &facets,
                    std::string name,
                    int sceleton
            ) -> typename std::result_of<complex_constructor(std::string, int)>::type;
        }
    }
}

#include "../../../src/complex/io.cpp"

#endif //ANUBIS_SUPERBUILD_IO_HPP
