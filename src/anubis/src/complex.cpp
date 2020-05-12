//
// Created by jmaerte on 27.11.19.
//

#include <cassert>
#include "ANUBiS/complex.hpp"
#include "calc/computation.hpp"
#include "data_types/potence/potence.hpp"
#include <data_types/thread_pool.hpp>
#include <boost/regex.hpp>
#include <utility>
#include <arithmetic/operator.hpp>
#include <algebra/typedef.hpp>
#include <algebra/matrix.hpp>
#include <algebra/reduction.hpp>
#include <arithmetic/factory/stack_allocator.hpp>

using namespace jmaerte::arith;

static const std::size_t BLOCK_SIZE = 1u << 27;

static const char LogTable256[256] = {
    #define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
                -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
                LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
                LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
};

static unsigned int LOG2(unsigned int v) {
    unsigned r;
    register unsigned int t, tt;

    if (tt = v >> 16)
    {
        r = (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
    }
    else
    {
        r = (t = v >> 8) ? 8 + LogTable256[t] : LogTable256[v];
    }
}


using namespace jmaerte::algebra;

namespace jmaerte {
    namespace anubis {

        static const int UINT_SIZE = 8 * sizeof(unsigned int);
        static const bool ALWAYS_BOUNDARY = true;

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

        int binary_search(unsigned int * list, unsigned int * simplex, int start, int end, int SIMPLEX_SIZE) {
            if (end == 0 || compare_to(list + (end - 1) * SIMPLEX_SIZE, simplex, SIMPLEX_SIZE) < 0) return end;
            if (compare_to(simplex, list, SIMPLEX_SIZE) < 0) return 0;
            int min = start;
            int max = end;
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
            return {};
//            return eigen(laplacian(i), f[i]);
        }

        complex::complex(std::string name, int sceleton) : name(name), sceleton(sceleton) {}

        std::vector<int> complex::f_vector() {
            return f;
        }


        /***************************************************************************************************************
         * s_list implementations
         **************************************************************************************************************/

        s_float_matrix s_list::laplacian(int i) {
            return {};
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
                    auto * simplex = new unsigned int[SIMPLEX_SIZE] { };
                    for (auto it = pot.begin(); it != pot.end(); ++it) {
                        simplex[*it / UINT_SIZE] |= 1u << (*it % UINT_SIZE);
                    }
                    int k = binary_search(list, simplex, 0, filled, SIMPLEX_SIZE);
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
            std::cout << "Generated faces of dimension " << (dim) << "; f_" << (dim) << " = " << f[dim] << "." << std::endl;
            return past[dim];
        }

        std::pair<int, int> s_list::bit_position(int pos) {
            int index = vertices * pos / (8 * sizeof(unsigned int));
            return {index, (vertices * pos) % (8 * sizeof(unsigned int))};
        }

        bool s_list::calculated(unsigned int dim) {
            return past.find(dim) != past.end();
        }

        s_list::~s_list() {
            clear_map();
        }

        stream<int> s_list::boundary_first(unsigned int dim) {
            assert(0 < dim && dim < f.size());
            unsigned int * top = generate(dim);
            unsigned int * low = generate(dim - 1);
            int SIMPLEX_SIZE = get_simplex_size();
            return transform(ints_from(0).take(f[dim]), [this, top, low, dim, SIMPLEX_SIZE](int i) {
                unsigned int * simplex = top + i * SIMPLEX_SIZE;
                int curr_index = SIMPLEX_SIZE - 1;
                unsigned int curr = simplex[curr_index];
                while (curr == 0) curr = *(simplex + --curr_index);
                unsigned int leading = 1 << log_2(curr);
                *(simplex + curr_index) ^= leading;
                int k = binary_search(low, simplex, 0, this->f[dim - 1], SIMPLEX_SIZE);
                *(simplex + curr_index) |= leading;
                return k;
            });
        }

        s_int_matrix s_list::boundary(unsigned int dim) {
            assert(0 <= dim && dim < f.size());
            unsigned int * top = generate(dim);
            unsigned int * low;
            if (dim > 0) {
                low = generate(dim - 1);
            }
            int SIMPLEX_SIZE = get_simplex_size();

            unsigned int factory_id = arith::vec::factory::REGISTER<arith::vec::stack_allocator<BLOCK_SIZE>>();

            return dim == 0 ? s_int_matrix(factory_id, f[dim], [this](int i, unsigned int factory_id) {
                return vec::NEW(factory_id, {
                    {0ULL, {false, 1ULL}}
                }); // reduced boundary
            }) : s_int_matrix(factory_id, f[dim], [this, top, dim, SIMPLEX_SIZE](int i, unsigned int factory_id) {
                std::vector<std::pair<ULL, std::pair<bool, ULL>>> vec {static_cast<std::size_t>(dim + 1)};
                unsigned int * simplex = top + i * SIMPLEX_SIZE;
                int curr_index = 0;
                unsigned int curr = simplex[0];
                unsigned int leading;
                unsigned int k = f[dim - 1];

                for (int pos = dim; pos >= 0; pos--) {
                    while (curr == 0) curr = *(simplex + ++curr_index);
                    leading = ((curr - 1) | curr) ^ (curr - 1);
                    curr = curr ^ leading; // remove this leading bit from curr.
                    *(simplex + curr_index) ^= leading;
                    vec[pos] = std::make_pair(
                            (ULL) (k = binary_search(this->past[dim - 1], simplex, 0, k, SIMPLEX_SIZE)),
                            std::make_pair(pos % 2, 1ULL)
                    );
                    *(simplex + curr_index) |= leading;
                }
                return vec::NEW(factory_id, vec);
            });
        }

        s_int_matrix s_list::coboundary(unsigned int dim) {
            assert(0 <= dim && dim < f.size());
            unsigned int * low = generate(dim);
            int SIMPLEX_SIZE = get_simplex_size();

            unsigned int * top;
            if (dim + 1 < f.size()) {
                top = generate(dim + 1);
            }

            unsigned int factory_id = arith::vec::factory::REGISTER<arith::vec::stack_allocator<BLOCK_SIZE>>();

            return dim + 1 == f.size() ? s_int_matrix(factory_id, f[dim], [this](int i, unsigned int factory_id) {
                return vec::NEW(factory_id, {
                    {0ULL, {false, 0ULL}}
                });
            }) : s_int_matrix(factory_id, f[dim], [this, low, top, dim, SIMPLEX_SIZE](int i, unsigned int factory_id) {
//                std::vector<int> indices {};
//                indices.reserve(dim + 3);
                std::vector<unsigned int> lead {};
                lead.reserve(dim + 3);
                std::vector<unsigned int> curr_indices {};
                curr_indices.reserve(dim + 3);
                unsigned int * simplex = low + i * SIMPLEX_SIZE;

                int curr_index = 0;
                unsigned int leading;
                unsigned int curr = simplex[curr_index];
                unsigned int * query = new unsigned int[SIMPLEX_SIZE] { };

//                std::cout << std::endl;
//                std::cout << "SIMPLEX : ";
//                for (int k = SIMPLEX_SIZE - 1; k >= 0; k--) {
//                    std::cout << std::bitset<32>(*(simplex + k)) << " ";
//                }
//                std::cout << std::endl;

                unsigned int last = 0;
//                indices[0] = 0;
                lead[0] = 0;
                curr_indices[0] = -1;
                for (int i = 0; i <= dim; i++) {
                    while (curr == 0) curr = simplex[++curr_index];
//                    unsigned int c = curr;
//                    leading = 1u;
//                    while (c != 1u) {
//                        c >>= 1;
//                        leading <<= 1;
//                    }
//                    curr ^= leading;
                    leading = ((curr - 1) | curr) ^ (curr - 1);
                    curr ^= leading;
                    query[curr_index] |= leading;

//                    std::cout << "QUERY: ";
//                    for (int k = SIMPLEX_SIZE - 1; k >= 0; k--) {
//                        std::cout << std::bitset<32>(*(query + k)) << " ";
//                    }
//                    std::cout << std::endl;
//                    std::cout << std::bitset<32>(leading) << std::endl;
//                    indices[i + 1] = (last = binary_search(top, query, last, f[dim + 1], SIMPLEX_SIZE));
//                    std::cout << "CLOSE: ";
//                    for (int k = SIMPLEX_SIZE - 1; k >= 0; k--) {
//                        std::cout << std::bitset<32>(*(top + last * SIMPLEX_SIZE + k)) << " ";
//                    }
//                    std::cout << "INDEX: " << indices[dim - i + 1] << std::endl;
                    lead[i + 1] = leading;
                    curr_indices[i + 1] = curr_index;
                }
                lead[dim + 2] = 1u;
                curr_indices[dim + 2] = SIMPLEX_SIZE;
//                indices[dim + 2] = f[dim + 1];
//                curr_index = 0;
//                leading = 1u;
                last = 0;

                std::vector<std::pair<ULL, std::pair<bool, ULL>>> vec;
//                for (int i = 0; i < SIMPLEX_SIZE; i++) query[i] = simplex[i];

                for (int i = 0; i <= dim + 1; i++) {
//                    last = indices[i + 1];
//                    std::cout << "INDEX: " << indices[i] << std::endl;
                    leading = lead[i] << 1;
                    curr_index = curr_indices[i];
                    if (leading == 0) {
                        leading = 1u;
                        curr_index++;
                    }
                    while(!(leading == lead[i + 1] && curr_index == curr_indices[i + 1])) {
//                        std::cout << leading << " " << curr_index << std::endl;
//                        std::cout << lead[i + 1] << " " << curr_indices[i + 1] << std::endl;
//                        std::cout << indices[i] << " " << last << " " << indices[i + 1] << " " << std::bitset<32>(leading) << " " << std::bitset<32>(lead[i]) << " " << curr_index << " " << curr_indices[i] << std::endl;
//                        if (curr_index > 2) std::_Exit(0);
//                        if (i > 0 && leading == lead[i - 1] && curr_index == curr_indices[i - 1]) {
//                            leading <<= 1;
//                        } else {
                        query[curr_index] |= leading;
                        last = binary_search(top, query, last, f[dim + 1], SIMPLEX_SIZE);
//                            if (last == indices[i + 1]) {
//                                query[curr_index] ^= leading;
//                                leading = lead[i];
//                                curr_index = curr_indices[i];
//                                std::cout << "BREAK" << std::endl;
//                                break;
//                            }

//                            std::cout << "QUERY: ";
//                            for (int k = SIMPLEX_SIZE - 1; k >= 0; k--) {
//                                std::cout << std::bitset<32>(*(query + k)) << " ";
//                            }
//                            std::cout << std::endl;
//                            std::cout << "CLOSE: ";
//                            for (int k = SIMPLEX_SIZE - 1; k >= 0; k--) {
//                                std::cout << std::bitset<32>(*(top + last * SIMPLEX_SIZE + k)) << " ";
//                            }
//                            std::cout << std::endl;

                        bool equals = true;
                        for (int j = SIMPLEX_SIZE - 1; j >= 0; j--) {
                            if (query[j] != *(top + SIMPLEX_SIZE * last + j)) {
                                equals = false;
                                break;
                            }
                        }
                        if (equals) {
//                            std::cout << "IS COBOUNDARY" << std::endl;
                            vec.push_back(std::make_pair(
                                    (ULL) last,
                                    std::make_pair(i % 2, 1ULL)
                            ));
                        }
                        query[curr_index] ^= leading;
                        leading <<= 1;
//                        }
                        if (leading == 0) {
                            leading = 1u;
                            curr_index++;
                        }
                    }
                }
                delete[] query;
                return vec::NEW(factory_id, vec);
            });
        }

        int s_list::get_simplex_size() {
            return vertices / UINT_SIZE + (vertices % UINT_SIZE == 0 ? 0 : 1);
        }

        std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR> s_list::homology(unsigned int dim) {
            auto it_dim = smith_forms.find(dim);
            if (it_dim == smith_forms.end()) {
                if (dim > 0 && f[dim - 1] == 0) generate(dim - 1);
                if (f[dim] == 0) generate(dim);
                if (dim == 0 || f[dim] < f[dim - 1] || ALWAYS_BOUNDARY) {
                    std::cout << "Calculating smith normal form of boundary." << std::endl;
                    smith_forms.emplace(dim, reduction::smith(boundary(dim)));
                } else {
                    std::cout << "Calculating smith normal form of coboundary." << std::endl;
                    smith_forms.emplace(dim, reduction::smith(coboundary(dim - 1)));
                }
                it_dim = smith_forms.find(dim);
            }
            auto d_map = it_dim->second;
            unsigned int kernel_rank = f[dim];

            auto it = d_map.begin();
            while (it != d_map.end()) {
                // DEBUGGING
//                std::cout << num::STRINGIFY(it->first) << " -> " << it->second << "." << std::endl;
                // END DEBUGGING
                kernel_rank -= it->second;
                it++;
            }
            std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR> result {};
            if (dim + 1 < f.size()) {
                it_dim = smith_forms.find(dim + 1);
                if (it_dim == smith_forms.end()) {
                    if (f[dim + 1] == 0) generate(dim + 1);

                    // check if the rank is already its own bound
                    stream<int> first = boundary_first(dim + 1);
                    int curr_first = -1;
                    int rank_lower_bound = 0;
                    while (!first.is_empty()) {
                        if (first.get() != curr_first) {
                            curr_first = first.get();
                            rank_lower_bound++;
                        }
                        first = first.pop_front();
                    }

                    std::cout << rank_lower_bound << " " << kernel_rank << std::endl;
                    if (rank_lower_bound >= kernel_rank) {
                        std::cout << "Rank of partial_" << (dim + 1) << " determined by trivial rank criterium, rank is " << kernel_rank << "." << std::endl;
                        std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR> rk = {{jmaerte::arith::num::NEW(1, false, 1ULL), kernel_rank}};
                        smith_forms.emplace(dim + 1, rk);
                    } else {
                        if (f[dim + 1] < f[dim] || ALWAYS_BOUNDARY) {
                            std::cout << "Calculating smith normal form of boundary." << std::endl;
                            smith_forms.emplace(dim + 1, reduction::smith(boundary(dim + 1)));
                        } else {
                            std::cout << "Calculating smith normal form of coboundary of dimension " << (dim) << " -> " << (dim + 1) << "." << std::endl;
                            smith_forms.emplace(dim + 1, reduction::smith(coboundary(dim)));
                        }
                    }
                    it_dim = smith_forms.find(dim + 1);
                }
                auto d_m_map = it_dim->second;
                for (auto kv : d_m_map) {
//                    std::cout << num::STRINGIFY(kv.first) << " -> " << kv.second << "." << std::endl;
                    if (!(num::IS_SINGLE(kv.first) == 1 && *(num::ABS(kv.first)) == 1ULL)) result.emplace(kv.first, kv.second);
                    kernel_rank -= kv.second;
                }
            }
            result.emplace(num::NEW(1, false, 0ULL), kernel_rank);
            return result;
        }

        /***************************************************************************************************************
         * s_tree implementations
         **************************************************************************************************************/

        s_float_matrix s_tree::laplacian(int dim) {
            return {};
//            assert(0 <= dim && dim < f.size());
//            return dim == 0 ? transform(ints_from(0).take(f[0]), [this](int i) {
//                return sparse<double>();
//            }) : transform(ints_from(0).take(f[dim]), [dim, this](int i) {
//                return sparse<double>();
//            });
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

        s_int_matrix s_tree::boundary(unsigned int dim) {
            return jmaerte::algebra::NEW({});
        }

        s_int_matrix s_tree::coboundary(unsigned int dim) {
            return jmaerte::algebra::NEW({});
        }

        std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR> s_tree::homology(unsigned int i) {
            return std::map<num::ap_int, unsigned int, num::comp::SIGNED_COMPARATOR>();
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