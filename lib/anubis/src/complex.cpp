//
// Created by jmaerte on 27.11.19.
//

#include <cassert>
#include <fstream>
#include <cstdlib>
#include <regex>
#include "anubis/complex.hpp"
#include "data_types/lin/sparse.hpp"
#include "calc/computation.hpp"
#include "data_types/potence/potence.hpp"
#include "multi_thread/thread_pool.hpp"

namespace jmaerte {
    namespace anubis {

        std::vector<double> complex::laplacian_spectrum(int i) {
            return eigen(laplacian(i), f[i]);
        }

        complex::complex(std::string name) : name(name) {}

        std::vector<int> complex::f_vector() {
            return f;
        }

        stream<sparse<double>> s_tree::laplacian(int dim) {
            assert(0 <= dim && dim < f.size());
            return dim == 0 ? transform(ints_from(0).take(f[0]), [this](int i) {
                return sparse<double>();
            }) : transform(ints_from(0).take(f[dim]), [dim, this](int i) {
                return sparse<double>();
            });
        }

        void s_tree::_insert(std::vector<int> v) {
            head->insert(v.begin(), v.end());
        }

        void s_tree::s_insert(const std::vector<int> & v, int sceleton) {
            int dim = sceleton < 0 ? (v.size() - 1) : (sceleton < v.size() ? sceleton : (v.size() - 1));
            if (dim + 1 > f.size()) {
                f.resize(dim + 1, 0);
                chains.resize(dim + 1, new std::map<int, node *>());
            }
            potence<int> pot {v};
            node* n;
            while(!pot.done()) {
                if (dim + 2 == pot.order()) return;
                n = head->insert(pot.begin(), pot.end());
                if (n) {
                    f[pot.order() - 1]++;
                    int el = pot.get(pot.order() - 1);
                    auto find = chains[pot.order() - 1]->find(el);
                    if (find != chains[pot.order() - 1]->end()) {
                        node * head_chain = chains[pot.order() - 1]->at(el);
                        node * curr = head_chain->next;
                        while (curr->next != head_chain) {
                            curr = curr->next;
                        }
                        curr->next = n;
                        n->next = head_chain;
                    } else {
                        chains[pot.order() - 1]->operator[](el) = n;
                        n->next = n;
                    }
                    chains[pot.order() - 1]->operator[](el) = n;
                }
                ++pot;
            }
        }

        void s_tree::print() {
            std::cout << "f-vector: [ ";
            for (auto it = f.begin(); it != f.end(); it++) std::cout << *it << " ";
            std::cout << "]" << std::endl;
            n_print(head, "");
        }

        void s_tree::n_print(s_tree::node * n, std::string tabs) {
            for (auto elem : n->children) {
                std::cout << tabs << elem.first << ": [" << std::endl;
                n_print(elem.second, tabs + "\t");
                std::cout << tabs << "]" << std::endl;
            }
        }

        s_tree::s_tree(std::string name) : complex(name) {}

        s_tree *s_tree::from_facets(std::vector<std::vector<int>> facets, std::string name, int sceleton) {
            auto * tree = new s_tree(name);
            for (const auto & facet : facets) {
                tree->s_insert(facet, sceleton);
            }
            return tree;
        }

        s_tree *s_tree::from_file(const std::string & path,
                int sceleton,
                const std::string & sep,
                const std::string & set_openers,
                const std::string & set_closers) {

            std::cout << "Opening file..." << std::endl;
            std::ifstream file;
            file.open(path);
            std::string content;
            if (file.is_open()) {
                content = {(std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>())};
            } else {
                std::cout << "[IO] Error: failed to open file!" << std::endl;
            }
            file.close();
            std::cout << "Read file..." << std::endl;
            size_t pos = content.find(" ");

            while(pos != std::string::npos) {
                content.replace(pos, 1, "");
                pos = content.find(" ", pos);
            }

            std::string name;

            pos = content.find(":=");
            if (pos != std::string::npos) {
                name = content.substr(0, pos);
                content = content.substr(pos + 2, content.size());
            } else {
                pos = content.find('=');
                if (pos != std::string::npos) {
                    name = content.substr(0, pos);
                    content = content.substr(pos + 1, content.size());
                } else {
                    throw std::invalid_argument("Simplicial Complex file must have a name followed by '=' or ':='.");
                }
            }

            std::cout << "Found name: " << name << std::endl;

            content = content.substr(1, content.size() - 2);

            std::regex simplex("[" + set_openers + "](([\\[{][\\w" + sep + "]*[\\]}])|([\\w" + sep + "]*))*[" + set_closers + "]");

            std::smatch matcher;
            std::string::const_iterator search (content.cbegin());

            std::map<std::string, int> labels;
            int i_labels = 0;

            auto * tree = new s_tree(name);

            thread_pool workers (1);

            while (regex_search(search, content.cend(), matcher, simplex)) {
                search = matcher.suffix().first;
                std::string s_facet = matcher[0];
                s_facet = s_facet.substr(1, s_facet.size() - 2);


                std::regex element("[" + set_openers + "][\\w,]*[" + set_closers + "]|[\\w]+");
                std::smatch element_matcher;
                std::string::const_iterator element_search (s_facet.cbegin());

                std::vector<int> facet;

                while (regex_search(element_search, s_facet.cend(), element_matcher, element)) {
                    element_search = element_matcher.suffix().first;
                    std::string curr = element_matcher[0];
                    if (labels.find(curr) == labels.end()) labels[curr] = i_labels++;
                    facet.push_back(labels[curr]);
                }

                std::cout << "Pushing simplex " << s_facet << std::endl;

                workers.add_work(
                        [&tree](const std::vector<int> & f, int s) {
                            tree->s_insert(f, s);
                        }, facet, sceleton);
            }
            return tree;
        }

        template<class InputIt>
        s_tree::node* s_tree::node::insert(InputIt it, InputIt end) {
            return _insert(it, end, nullptr);
        }

        template<class InputIt>
        s_tree::node * s_tree::node::_insert(InputIt it, InputIt end, node * curr) {
            static_assert(std::is_convertible<typename std::iterator_traits<InputIt>::value_type, int>::value, "Can only insert integral words into a simplex tree.");
            if (it == end) return curr;
            if (children.find(*it) == children.end()) {
                children[*it] = get_node();
                return children[*it]->_insert(++it, end, children[*it]);
            }
            return children[*it]->_insert(++it, end, nullptr);
        }
    }
};