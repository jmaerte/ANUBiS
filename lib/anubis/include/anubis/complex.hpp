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

            complex(std::string name);

        public:

            std::vector<double> laplacian_spectrum(int i);

            std::vector<int> f_vector();
        };

        class s_tree : public complex {
        private:
            struct node {
            private:

                template<typename InputIt>
                node * _insert(InputIt it, InputIt end, node *);

            public:
                node * next;

                std::unordered_map<int, node*> children;

                template<typename InputIt>
                node* insert(InputIt it, InputIt end);
            };

            static node* get_node() {
                node* n = new node;
                n->children = {};
                n->next = nullptr;
                return n;
            }

            s_tree(std::string name);

            void _insert(std::vector<int>);
            void n_print(node *, std::string);

            node* head = get_node();

            stream<sparse<double>> laplacian(int i) override;
        public:

            std::vector<std::map<int, node*> *> chains;

            void s_insert(const std::vector<int> &, int sceleton = -1);
            void print();

            static s_tree* from_file(
                    const std::string &,
                    int sceleton = -2,
                    const std::string & sep = ",",
                    const std::string & set_openers = "\\[{",
                    const std::string & set_closers = "\\]}");
            static s_tree* from_facets(std::vector<std::vector<int>>, std::string, int sceleton = -1);
        };
    }
}