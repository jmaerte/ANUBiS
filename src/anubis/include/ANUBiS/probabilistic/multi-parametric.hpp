//
// Created by jmaerte on 03.10.20.
//

#ifndef ANUBIS_MULTI_PARAMETRIC_HPP
#define ANUBIS_MULTI_PARAMETRIC_HPP

#include "../complex.hpp"
#include <vector>

namespace jmaerte {
    namespace anubis {
        namespace probabilistic {

            class ANUBIS_EXPORT mp_experiment {
            public:
            struct node {
                complex* complex;
                double prob;
                bool is_P;
                std::vector<node*> children;
                node* parent;
            };
            private:

                int depth = -1;
                int n;
                node* root = nullptr;

            public:

                std::function<bool(complex*)> P;
                std::function<double(int)> p;

                mp_experiment(std::function<bool(complex*)> P, std::function<double(int)> p, int n): P(P), p(p), n(n) {};

                void generate_poset();
                void render_poset();
            };

        }
    }
}

#endif //ANUBIS_MULTI_PARAMETRIC_HPP
