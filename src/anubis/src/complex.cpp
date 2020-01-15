//
// Created by jmaerte on 27.11.19.
//

#include <cassert>
#include "ANUBiS/complex.hpp"
#include "data_types/lin/sparse.hpp"
#include "calc/computation.hpp"
#include "data_types/potence/potence.hpp"
#include "multi_thread/thread_pool.hpp"
#include <boost/regex.hpp>

static const char LogTable256[256] = {
    #define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
                -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
                LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
                LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
};


namespace jmaerte {
    namespace anubis {

        static const int UINT_SIZE = 8 * sizeof(unsigned int);



        template<typename complex_constructor>
        auto complex_from_file(
                complex_constructor constructor,
                const std::string & path,
                int sceleton,
                const std::string & sep,
                const std::string & set_openers,
                const std::string & set_closers
        ) -> typename std::result_of<complex_constructor(std::string, int)>::type {
            using complex_type = typename std::result_of<complex_constructor(std::string, int)>::type;
            static_assert(std::is_base_of<jmaerte::anubis::complex, typename std::remove_pointer<complex_type>::type>::value, "complex_type must be derived from complex.");
            std::cout << "Opening file..." << std::endl;
            std::ifstream file;
            file.open(path);
            std::string content;
            if (file.is_open()) {
                content = {(std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>())};
            } else {
                throw std::invalid_argument("[IO] Error: failed to open file!");
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

            boost::regex simplex("[" + set_openers + "](([\\[{][\\w" + sep + "]*[\\]}])|([\\w" + sep + "]*))*[" + set_closers + "]");
            boost::sregex_token_iterator iter(content.begin(), content.end(), simplex, 0);
            boost::sregex_token_iterator end;

            std::map<std::string, unsigned int> labels;
            unsigned int i_labels = 0;

            complex_type complex = constructor(name, sceleton);

            {
                thread_pool workers(-1);

                while (iter != end) {
                    std::string s_facet = *iter;
                    s_facet = s_facet.substr(1, s_facet.size() - 2);

                    boost::regex element("[" + set_openers + "][\\w,]*[" + set_closers + "]|[\\w]+");
                    boost::sregex_token_iterator element_iter(s_facet.begin(), s_facet.end(), element, 0);
                    boost::sregex_token_iterator element_end;

                    auto facet = new std::vector<unsigned int>();

                    while (element_iter != element_end) {
                        std::string curr = *element_iter;
                        if (labels.find(curr) == labels.end()) labels[curr] = i_labels++;
                        facet->push_back(labels[curr]);
                        element_iter++;
                    }

//                    std::cout << "Pushing " << s_facet << std::endl;

//                    tree->s_insert(facet, sceleton);
                    std::function<void()> fn = [complex, facet, sceleton]() {
                        complex->facet_insert(facet);
                    };

                    workers.add_work(fn);
                    iter++;
                }
                std::cout << "pushed all" << std::endl;
            }
            std::cout << "processed all" << std::endl;

            return complex;
        }

        template<typename complex_constructor>
        auto complex_from_facets(
                complex_constructor constructor,
                std::vector<std::vector<unsigned int> *> &facets,
                std::string name,
                int sceleton
        ) -> typename std::result_of<complex_constructor(std::string, int)>::type {

            using complex_type = typename std::result_of<complex_constructor(std::string, int)>::type;
            static_assert(std::is_base_of<jmaerte::anubis::complex, typename std::remove_pointer<complex_type>::type>::value, "complex_type must be derived from complex.");

            complex_type complex = constructor(name, sceleton);
            for (const auto & facet : facets) {
                complex->facet_insert(facet);
            }
            return complex;
        }


        /***************************************************************************************************************
         * Helper-Methods for generate
         **************************************************************************************************************/

        void set(unsigned int * list, unsigned int * simplex, int pos, int SIMPLEX_SIZE) {
            for (int i = 0; i < SIMPLEX_SIZE; i++) {
                list[pos * SIMPLEX_SIZE + i] = simplex[i];
            }
        }

        bool equals(unsigned int * a, unsigned int * b, int length) {
            for (int i = 0; i < length; i++) {
                if (a[i] != b[i]) return false;
            }
            return true;
        }

        unsigned int log_2(unsigned int v) {
            unsigned int r;
            unsigned int t, tt;

            if ((tt = v >> 16) != 0) {
                r = (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
            } else {
                r = (t = v >> 8) ? 8 + LogTable256[t] : LogTable256[v];
            }
            return r;
        }

        int compare_to(unsigned int * a, unsigned int * b, int length) {
            unsigned int c;
            for (int i = 0; i < length; i++) {
                if ((c = a[i] ^ b[i]) != 0) {
                    unsigned int mask = ((c - 1) | c) ^ (c - 1);
                    return (a[i] & mask) == 0 ? 1 : -1;
                }
            }
            return 0;
        }

        int binary_search(unsigned int * list, unsigned int * simplex, int list_size, int SIMPLEX_SIZE) {
            if (list_size == 0 || compare_to(list + (list_size - 1) * SIMPLEX_SIZE, simplex, SIMPLEX_SIZE) < 0) return list_size;
            if (compare_to(simplex, list, SIMPLEX_SIZE) < 0) return 0;
            int min = 0;
            int max = list_size;
            int compare = 0;
            while (min < max) {
                int mid = (min + max)/2;
                if ((compare = compare_to(list + mid * SIMPLEX_SIZE, simplex, SIMPLEX_SIZE)) < 0) min = mid + 1;
                else if (compare > 0) max = mid;
                else return mid;
            }
            return min;
        }

        int insert(unsigned int * a, int pos, unsigned int *& list, int occupation, int list_size, int SIMPLEX_SIZE) {
            if (occupation >= list_size) {
                auto * resized = new unsigned int[list_size * SIMPLEX_SIZE * 2];
                std::copy(list, list + pos * SIMPLEX_SIZE, resized);
                if (pos != list_size) {
                    std::copy(list + pos * SIMPLEX_SIZE, list + occupation * SIMPLEX_SIZE, resized + (pos + 1) * SIMPLEX_SIZE);
                }

                delete[] list;
                list = resized;
                list_size *= 2;
            } else {
                if (pos < occupation){
                    std::copy_backward(list + pos * SIMPLEX_SIZE, list + occupation * SIMPLEX_SIZE, list + (occupation + 1) * SIMPLEX_SIZE);
                }
            }
            std::copy(a, a + SIMPLEX_SIZE, list + pos * SIMPLEX_SIZE);
            return list_size;
        }


        /***************************************************************************************************************
         * Complex implementations
         **************************************************************************************************************/


        std::vector<double> complex::laplacian_spectrum(int i) {
            return eigen(laplacian(i), f[i]);
        }

        complex::complex(std::string name, int sceleton) : name(name), sceleton(sceleton) {}

        std::vector<int> complex::f_vector() {
            return f;
        }


        /***************************************************************************************************************
         * s_list implementations
         **************************************************************************************************************/

        stream<sparse<double>> s_list::laplacian(int i) {
            return stream<sparse<double>>();
        }

        void s_list::facet_insert(const std::vector<unsigned int> * v) {
            std::lock_guard<std::mutex> lockGuard(facet_mutex);
            clear();
            if (v->size() > f.size()) {
                f.resize(v->size());
            }
            facets.push_back(*v);
            if (v->at(v->size() - 1) + 1 > vertices)
                vertices = v->at(v->size() - 1) + 1;
        }

        void s_list::clear() {
            clear_map();
            for (int i = 0; i < f.size(); i++) {
                f[i] = 0;
            }
        }

        void s_list::clear_map() {
            for (auto it = past.begin(); it != past.end(); it++) {
                delete[] it->second;
            }
            past.clear();
        }

        void s_list::clear_dim(unsigned int i) {
            if (past.find(i) == past.end()) return;
            unsigned int *& el = past.at(i);
            delete[] el;
            past.erase(i);
        }

        unsigned int * s_list::generate(unsigned int dim) {
            if (past.find(dim) != past.end()) return past[dim];
            int SIMPLEX_SIZE = get_simplex_size();

            int size = (f.size() <= dim || f[dim] == 0) ? 10 :
                f[dim];
            auto * list = new unsigned int[size * SIMPLEX_SIZE];
            int filled = 0;
            for (auto& facet : facets) {
                if (facet.size() < dim + 1) continue;
                potence<unsigned int> pot {facet, (int) dim + 1};
                while (!pot.done() && pot.order() == dim + 1) {
                    auto * simplex = new unsigned int[SIMPLEX_SIZE];
                    for (int i = 0; i < SIMPLEX_SIZE; i++) {
                        simplex[i] = 0u;
                    }
                    for (auto it = pot.begin(); it != pot.end(); ++it) {
                        simplex[*it / UINT_SIZE] |= 1u << (*it % UINT_SIZE);
                    }
                    int k = binary_search(list, simplex, filled, SIMPLEX_SIZE);
                    if (k == filled || !equals(list + k * SIMPLEX_SIZE, simplex, SIMPLEX_SIZE)) {
                        size = insert(simplex, k, list, filled, size, SIMPLEX_SIZE);
                        filled++;
                    }
                    delete[] simplex;
                    ++pot;
                }
            }

//            for (int j = 0; j < filled; j++) {
//                unsigned int * simplex = list + SIMPLEX_SIZE * j;
//                for (int i = 0; i < SIMPLEX_SIZE; i++) std::cout << std::bitset<32>(simplex[SIMPLEX_SIZE - i - 1]) << " ";
//                std::cout << std::endl;
//            }
            f[dim] = filled;
            past[dim] = list;
            return past[dim];
        }

        std::pair<int, int> s_list::bit_position(int pos) {
            int index = vertices * pos / (8 * sizeof(unsigned int));
            return {index, (vertices * pos) % (8 * sizeof(unsigned int))};
        }

        bool s_list::calculated(unsigned int dim) {
            return past.find(dim) != past.end();
        }

        s_list* s_list::from_file(
                    const std::string &path,
                    int sceleton,
                    const std::string &sep,
                    const std::string &set_openers,
                    const std::string &set_closers
                ) {
            return complex_from_file([](std::string name, int sceleton) {
                return new s_list(std::move(name), sceleton);
            }, path, sceleton, sep, set_openers, set_closers);
        }

        s_list* s_list::from_facets(std::vector<std::vector<unsigned int> *> &facets, std::string name, int sceleton) {
            return complex_from_facets([](std::string name, int sceleton) {
                return new s_list(std::move(name), sceleton);
            }, facets, std::move(name), sceleton);
        }


        s_list::~s_list() {
            clear_map();
        }

        stream<sparse<int>> s_list::boundary(int dim) {
            assert(0 <= dim && dim < f.size());
            if(f[dim] == 0) generate(dim);
            if (dim > 0) {
                if(f[dim - 1] == 0) generate(dim - 1);
            }
            int SIMPLEX_SIZE = get_simplex_size();
            return dim == 0 ? transform(ints_from(0).take(f[dim]), [this](int i) {
                return sparse<int>({{0, 1}}); // reduced boundary
            }) : transform(ints_from(0).take(f[dim]), [this, dim, SIMPLEX_SIZE](int i) {
                std::vector<std::pair<int, int>> vec {static_cast<std::size_t>(dim + 1)};
                unsigned int * simplex = this->past[dim] + i * SIMPLEX_SIZE;
                int curr_index = 0;
                unsigned int curr = simplex[0];
                unsigned int leading;
                unsigned int temp;
                for (int pos = 0; pos < dim + 1; pos++) {
                    while (curr == 0) curr = simplex[++curr_index];
                    leading = ((curr - 1) | curr) ^ (curr - 1);
                    curr = curr ^ leading; // remove this leading bit from curr.
                    temp = simplex[curr_index];
                    simplex[curr_index] ^= leading;
                    vec[pos] = {
                            binary_search(this->past[dim - 1], simplex, f[dim - 1], SIMPLEX_SIZE),
                            pos % 2 == 0 ? 1 : -1
                    };
                    simplex[curr_index] = temp;
                }
                return sparse<int>(vec);
            });
        }

        int s_list::get_simplex_size() {
            return vertices / UINT_SIZE + (vertices % UINT_SIZE == 0 ? 0 : 1);
        }

        /***************************************************************************************************************
         * s_tree implementations
         **************************************************************************************************************/

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

        void s_tree::facet_insert(const std::vector<unsigned int> * v) {
            int dim = sceleton < 0 ? (v->size() - 1) : (sceleton < v->size() ? sceleton : (v->size() - 1));
            {
                std::lock_guard<std::mutex> chainsGuard(chains_mutex);
                if (dim + 1 > f.size()) {
                    f.resize(dim + 1, 0);
                    chains.resize(dim + 1, new std::map<int, node *>());
                }
            }
            potence<unsigned int> pot {*v};
            node* n;
            while(!pot.done()) {
                if (dim + 2 == pot.order()) return;
                n = head->insert(pot.begin(), pot.end());
                if (n) {
                    std::lock_guard<std::mutex> lockGuard(chains_mutex);
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
//                    chains_mutex.lock_write();
//                    chains[pot.order() - 1]->operator[](el) = n;
//                    chains_mutex.unlock_write();
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

        s_tree::s_tree(std::string name, int sceleton) : complex(name, sceleton) {}

        s_tree::~s_tree() {
            delete head;
        }

        s_tree * s_tree::from_file(
                    const std::string &path,
                    int sceleton,
                    const std::string &sep,
                    const std::string &set_openers,
                    const std::string &set_closers
                ) {
            return complex_from_file([](std::string name, int sceleton) {
                return new s_tree(name, sceleton);
            }, path, sceleton, sep, set_openers, set_closers);
        }

        s_tree *s_tree::from_facets(std::vector<std::vector<unsigned int> *> &facets, std::string name, int sceleton) {
            return complex_from_facets([](std::string name, int sceleton) {
                return new s_tree(name, sceleton);
            }, facets, name, sceleton);
        }

        stream<sparse<int>> s_tree::boundary(int dim) {
            return stream<sparse<int>>();
        }

        /***********************************************************************************************************
         * s_tree nodes implementations
         **********************************************************************************************************/

        s_tree::node::~node() {
            for (auto element : children) {
                delete element.second;
            }
        }

        template<class InputIt>
        s_tree::node* s_tree::node::insert(InputIt it, InputIt end) {
            static_assert(std::is_convertible<typename std::iterator_traits<InputIt>::value_type, int>::value, "Can only insert integral words into a simplex tree.");
            return _insert(it, end, nullptr);
        }

        template<class InputIt>
        s_tree::node * s_tree::node::_insert(InputIt it, InputIt end, node * curr) {
            if (it == end) return curr;
            std::unique_lock<std::mutex> lockGuard(mutex);
            if (children.find(*it) == children.end()) {
                children[*it] = get_node();
                lockGuard.unlock();
                /** Alternative way (lock-free):
                 * children an array instead of a map.
                 * each process holds his own pointer to a new node.
                 * When a process inserts a new child it uses new_node = std::atomic_compare_exchange_weak(children[i], nullptr, new_node)
                 */
                return children[*it]->_insert(++it, end, children[*it]);
            }
            lockGuard.unlock();
            return children[*it]->_insert(++it, end, nullptr);
        }
    }
};