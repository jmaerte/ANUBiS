//
// Created by jmaerte on 27.11.19.
//

#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include "../../src/data_types/lin/sparse.hpp"

template<class T>
class stream;

namespace jmaerte {
    namespace anubis {

        class complex {
        private:

            virtual stream<sparse<double>> laplacian(int i) = 0;

        protected:

            std::string name;

            std::vector<int> f;

            complex();

        public:

            std::vector<double> laplacian_spectrum(int i);
        };

        class s_tree : public complex {
        private:
            struct node {
            private:

                template<typename InputIt>
                bool _insert(InputIt it, InputIt end, bool inserted);

            public:
                std::unordered_map<int, node*> children;

                template<typename InputIt>
                bool insert(InputIt it, InputIt end);
            };

            static node* get_node() {
                node* n = new node;
                n->children = {};
                return n;
            }

            s_tree();

            void _insert(std::vector<int>);
            void n_print(node *, std::string);

            node* head = get_node();
            stream<sparse<double>> laplacian(int i) override;
        public:

            void s_insert(std::vector<int>, int sceleton = -2);
            void print();

            static s_tree* from_file(std::string, int sceleton = -2, std::string sep = ",", std::string set_openers = "\\[{", std::string set_closers = "\\]}");
            static s_tree* from_facets(std::vector<std::vector<int>>, int sceleton = -2);
        };
    }
}
