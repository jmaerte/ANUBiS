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
#include <mutex>
#include "data_types/lin/sparse.hpp"
#include <algebra/typedef.hpp>
#include <data_types/stream.hpp>
#include <ANUBIS_EXPORT.h>

using namespace jmaerte::arith;
using namespace jmaerte::algebra;

namespace jmaerte {
    namespace anubis {


        /** @brief Virtual class abstraction of a simplicial complex. It guarentees us, that non-abstract child classes
         * enable us to calculate their homology and laplacian spectrum.

        A Simplicial Complex K is a collection of simplices, such that a pair of distinct ones always share a common face
         or are disjoint.
        @author Julian Märte
        @date January 2020
        */
        class ANUBIS_EXPORT complex {
        private:

            virtual s_float_matrix   laplacian(int i) = 0;
            virtual s_int_matrix     boundary(int dim) = 0;

        protected:

            std::string name;
            const int sceleton;

            std::mutex f_mutex;
            std::vector<int> f;
            std::vector<int> h;

            complex(std::string name, int sceleton = -1);

        public:

            virtual void facet_insert(const std::vector<unsigned int> *) = 0;
            std::vector<double> laplacian_spectrum(int i);
            virtual std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR> homology(int dim) = 0;

            std::vector<int> f_vector();
        };

        class ANUBIS_EXPORT s_list : public complex {
        private:

            static const unsigned int highest_mask = (unsigned int) 1 << (8 * sizeof(unsigned int) - 1);

            std::mutex facet_mutex;
            std::vector<std::vector<unsigned int>> facets;
            std::map<unsigned int, unsigned int *> past;
            std::map<unsigned int, std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR>> smith_forms;
            int vertices;

            s_float_matrix   laplacian(int i) override;
            s_int_matrix     boundary(int dim) override;

            std::pair<int, int> bit_position(int pos);
            int get_simplex_size();

            s_list(std::string name, int sceleton): complex(name, sceleton), smith_forms() {}

        public:

            ~s_list();

            void facet_insert(const std::vector<unsigned int> *) override;

            void clear_map();
            void clear();
            void clear_dim(unsigned int i);
            unsigned int * generate(unsigned int dim);
            bool calculated(unsigned int dim);

            std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR> homology(int dim) override;

            static s_list* from_file(
                    const std::string &,
                    int sceleton = -2,
                    const std::string & sep = ",",
                    const std::string & set_openers = "\\[{",
                    const std::string & set_closers = "\\]}");
            static s_list* from_facets(std::vector<std::vector<unsigned int>*> &facets, std::string name, int sceleton = -1);
        };

        class ANUBIS_EXPORT s_tree : public complex {
        private:
                struct ANUBIS_EXPORT node {
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

            s_int_matrix     boundary(int dim) override;
            s_float_matrix   laplacian(int i) override;

            s_tree(std::string name, int sceleton = -1);

            void _insert(std::vector<int>);
            void n_print(node *, std::string);

            node* head = get_node();

        public:

            ~s_tree();

            std::mutex chains_mutex;
            std::vector<std::map<int, node *> *> chains;

            void facet_insert(const std::vector<unsigned int> *) override;
            void print();
            std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR> homology(int i) override;

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