//
// Created by jmaerte on 27.11.19.
//

#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include "../../src/multi_thread/rw_mutex.hpp"
#include "../../src/data_types/lin/sparse.hpp"

template<class T>
class stream;

namespace jmaerte {
    namespace anubis {

        class complex {
        private:

            virtual stream<sparse<double>> laplacian(int i) = 0;
            virtual stream<sparse<int>> boundary(int dim) = 0;

        protected:

            std::string name;
            const int sceleton;

            std::mutex f_mutex;
            std::vector<int> f;

            complex(std::string name, int sceleton = -1);

        public:

            virtual void facet_insert(const std::vector<unsigned int> *) = 0;
            std::vector<double> laplacian_spectrum(int i);
//            std::map<int, int> homology();

            std::vector<int> f_vector();
        };

        class s_list : public complex {
        private:

            static const unsigned int highest_mask = (unsigned int) 1 << (8 * sizeof(unsigned int) - 1);

            std::mutex facet_mutex;
            std::vector<std::vector<unsigned int>> facets;
            std::map<unsigned int, unsigned int *> past;
            int vertices;

            stream<sparse<double>> laplacian(int i) override;

            std::pair<int, int> bit_position(int pos);
            int get_simplex_size();

            stream<sparse<int>> boundary(int dim) override;

            s_list(std::string name, int sceleton): complex(name, sceleton) {}

        public:

            ~s_list();

            void facet_insert(const std::vector<unsigned int> *) override;

            void clear_map();
            void clear();
            void clear_dim(unsigned int i);
            unsigned int * generate(unsigned int dim);
            bool calculated(unsigned int dim);

            static s_list* from_file(
                    const std::string &,
                    int sceleton = -2,
                    const std::string & sep = ",",
                    const std::string & set_openers = "\\[{",
                    const std::string & set_closers = "\\]}");
            static s_list* from_facets(std::vector<std::vector<unsigned int>*> &facets, std::string name, int sceleton = -1);
        };

        class s_tree : public complex {
        private:
            struct node {
            private:

                template<typename InputIt>
                node * _insert(InputIt it, InputIt end, node *);

            public:
                node * next;

                std::mutex mutex;
                std::unordered_map<int, node*> children;

                ~node();

                template<typename InputIt>
                node* insert(InputIt it, InputIt end);
            };

            static node* get_node() {
                node* n = new node;
                n->children = {};
                n->next = nullptr;
                return n;
            }

            stream<sparse<int>> boundary(int dim) override;

            s_tree(std::string name, int sceleton = -1);

            void _insert(std::vector<int>);
            void n_print(node *, std::string);

            node* head = get_node();

            stream<sparse<double>> laplacian(int i) override;
        public:

            ~s_tree();

            std::mutex chains_mutex;
            std::vector<std::map<int, node *> *> chains;

            void facet_insert(const std::vector<unsigned int> *) override;
            void print();

            static s_tree * from_file(
                    const std::string &,
                    int sceleton = -2,
                    const std::string & sep = ",",
                    const std::string & set_openers = "\\[{",
                    const std::string & set_closers = "\\]}");
            static s_tree * from_facets(std::vector<std::vector<unsigned int>*> &facets, std::string name, int sceleton = -1);
        };
    }
}