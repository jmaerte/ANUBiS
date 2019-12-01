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

namespace jmaerte {
    namespace anubis {

        std::vector<double> complex::laplacian_spectrum(int i) {
            return eigen(laplacian(i), f[i]);
        }

        complex::complex() {}

        stream<sparse<double>> s_tree::laplacian(int i) {
            assert(0 <= i && i <= f.size());
            return transform(ints_from(0).take(5), [](int j) {
                switch(j) {
                    case 0: return sparse<double>({{0, 3}, {2, 4}, {3, 6}});
                    case 1: return sparse<double>({{1, 2}, {4, 2}});
                    case 2: return sparse<double>({{0, 4}, {3, 2}});
                    case 3: return sparse<double>({{0, 6}, {2, 2}, {3, 5}});
                    case 4: return sparse<double>({{1, 2}, {4, 2}});
                }
            });
        }

        void s_tree::_insert(std::vector<int> v) {
            head->insert(v.begin(), v.end());
        }

        void s_tree::s_insert(std::vector<int> v, int sceleton) {
            if (v.size() > f.size()) {
                f.resize(v.size(), 0);
            }
            potence<int> pot {v};
            while(!pot.done()) {
                if (sceleton == pot.order() - 1) return;
                if (head->insert(pot.begin(), pot.end()))
                    f[pot.order() - 1]++;
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

        s_tree::s_tree() {}

        s_tree *s_tree::from_facets(std::vector<std::vector<int>> facets, int sceleton) {
            s_tree* tree;
            for (auto facet : facets) {
                tree->s_insert(facet, sceleton);
            }
            return tree;
        }

        s_tree *s_tree::from_file(std::string home_path, int sceleton, std::string sep, std::string set_openers, std::string set_closers) {
            std::ifstream file;
            file.open(std::getenv("HOME") + home_path);
            std::string content;
            if (file.is_open()) {
                content = {(std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>())};
            } else {
                std::cout << "[IO] Error: failed to open file!" << std::endl;
            }
            file.close();
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
                pos = content.find("=");
                if (pos != std::string::npos) {
                    name = content.substr(0, pos);
                    content = content.substr(pos + 1, content.size());
                } else {
                    throw std::invalid_argument("Simplicial Complex file must have a name followed by '=' or ':='.");
                }
            }

            content = content.substr(1, content.size() - 2);

            std::regex simplex("[" + set_openers + "](([\\[{][\\w" + sep + "]*[\\]}])|([\\w" + sep + "]*))*[" + set_closers + "]");

            std::smatch matcher;
            std::string::const_iterator search (content.cbegin());
            std::vector<std::string> s_facets;
            while (regex_search(search, content.cend(), matcher, simplex)) {
                search = matcher.suffix().first;
                std::string curr = matcher[0];
                curr = curr.substr(1, curr.size() - 2);
                s_facets.push_back(curr);
                std::cout << curr << " " << (search - content.cbegin()) << std::endl;
            }
            std::vector<std::vector<int>> facets;
            std::map<std::string, int> labels;
            int i_labels = 0;
            for (auto const & s_facet : s_facets) {
//                std::cout << s_facet << std::endl;
                std::regex element("[" + set_openers + "][\\w,]*[" + set_closers + "]|[\\w]*");
                std::smatch element_matcher;
                std::string::const_iterator element_search (s_facet.cbegin());

                std::vector<int> facet;

                while (regex_search(element_search, s_facet.cend(), element_matcher, element)) {
                    element_search = matcher.suffix().first;
                    std::string curr = element_matcher[0];
//                    std::cout << curr << std::endl;
                    if (labels.find(curr) == labels.end()) labels[curr] = i_labels++;
                    facet.push_back(labels[curr]);
                }

//                std::cout << s_facet << " ";
//                for (auto el : facet) {
//                    std::cout << el << " ";
//                }
//                std::cout << std::endl;

                facets.push_back(facet);
            }
            s_tree * result = s_tree::from_facets(facets);
            result->name = name;
            return s_tree::from_facets(facets, sceleton);
        }

        template<class InputIt>
        bool s_tree::node::insert(InputIt it, InputIt end) {
            return _insert(it, end, false);
        }

        template<class InputIt>
        bool s_tree::node::_insert(InputIt it, InputIt end, bool inserted) {
            static_assert(std::is_convertible<typename std::iterator_traits<InputIt>::value_type, int>::value, "Can only insert integral words into a simplex tree.");
            if (it == end) return inserted;
            if (children.find(*it) == children.end()) {
                children[*it] = get_node();
                return children[*it]->_insert(++it, end, true);
            }
            return children[*it]->_insert(++it, end, false);
        }
    }
}