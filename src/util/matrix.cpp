//
// Created by maertej on 29.09.19.
//

#include <vector>
#include "../data_struct/sparse_vector.hpp"
#include "computation/thread_pool.hpp"
#include "computation/rw_mutex.hpp"
#include "binary_search.cpp"
#include "computation/bloom_filter.hpp"

#ifndef ANUBIS_MATRIX_HPP
#define ANUBIS_MATRIX_HPP

template<typename T>
class generator {
    virtual bool has_next() = 0;
    virtual sparse_vector<T> next() = 0;
    virtual int size();
};

template<typename T>
static void reduce_gaussian(std::vector<sparse_vector<T>> &matrix, sparse_vector<T> &v, rw_mutex &mutex, bloom_filter<unsigned int> &filter) {
    mutex.lock_read();
    typename std::vector<sparse_vector<T>>::iterator k = matrix.begin();
    mutex.unlock_read();
    while (true) {
        mutex.lock_read();
        k = binary_search(k, matrix.end(), v);
        if (k <= matrix.end() && (*k).first_index() == v.first_index()) {
            v -= (v.first_val() / (*k).first_val()) * (*k);
        }
        mutex.unlock_read();
        if (v.first_index() < 0) return;
        mutex.lock_write();
        bool is_unique;
        if (!filter.has(v.first_index())) {
            is_unique = true;
        } else { // could be false positive!
            k = binary_search(k, matrix.end(), v);
            is_unique = !(k <= matrix.end() && (*k).first_index() == v.first_index());
        }
        if (is_unique) {
            matrix.insert(v);
            filter.put(v.first_index());
        }
        mutex.unlock_write();
        if (is_unique) return;
    }

}

template<typename T>
static std::vector<sparse_vector<T>> gaussian(generator<T> matrix_generator) {
    thread_pool workers (20);
    rw_mutex matrix_mutex;
    std::vector<sparse_vector<T>> matrix;
    bloom_filter<unsigned int> filter (0.05, matrix_generator.size() / 10);

    while (matrix_generator.has_next()) {
        workers.add_work(reduce_gaussian<T>, matrix, matrix_generator.next(), matrix_mutex, filter);
    }

    return matrix;
}

#endif //ANUBIS_MATRIX_HPP
