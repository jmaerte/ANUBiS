//
// Created by maertej on 29.09.19.
//

#include <vector>
#include <array>
#include "../util/index_list.hpp"

#ifndef ANUBIS_SIMPLICIAL_HPP
#define ANUBIS_SIMPLICIAL_HPP

namespace anubis {
    class complex {
    public:
        virtual index_list homology() = 0;
    };

    /**
     * A class that represents a simplicial complex.
     * The simplicial structure is implemented as a simplex tree.
     */
    class simplex_tree : public complex {
        int dim;
    public:
        simplex_tree(std::vector<std::vector<int>>);

        index_list homology();
    };

    class simplex_iterator : public complex {
        std::vector<std::vector<int>> top_facets;
    public:
        simplex_iterator(std::vector<std::vector<int>> top_facets) : top_facets(std::move(top_facets)) {}

        index_list homology();
    };
}

#endif //ANUBIS_SIMPLICIAL_HPP
