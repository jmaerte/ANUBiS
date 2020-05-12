//
// Created by jmaerte on 12.05.20.
//

#ifndef ANUBIS_SUPERBUILD_IO_HPP
#define ANUBIS_SUPERBUILD_IO_HPP

#include "../complex.hpp"

namespace jmaerte {
    namespace anubis {
        namespace io {

            namespace s_list {
                static s_list * from_file(
                        const std::string &,
                        int sceleton = -2,
                        const std::string & sep = ",",
                        const std::string & set_openers = "\\[{",
                        const std::string & set_closers = "\\]}");
                static s_list* from_facets(std::vector<std::vector<unsigned int>*> &facets, std::string name, int sceleton = -1);
            }

            namespace s_tree {
                static s_tree * from_file(
                        const std::string &,
                        int sceleton = -2,
                        const std::string & sep = ",",
                        const std::string & set_openers = "\\[{",
                        const std::string & set_closers = "\\]}");
                static s_tree * from_facets(std::vector<std::vector<unsigned int>*> &facets, std::string name, int sceleton = -1);
            }
        }
    }
}

#endif //ANUBIS_SUPERBUILD_IO_HPP
