//
// Created by maertej on 29.09.19.
//

#ifndef ANUBIS_SPARSE_VECTOR_HPP
#define ANUBIS_SPARSE_VECTOR_HPP

#include <utility>
#include <vector>

template<typename T>
class sparse_vector {

    std::vector<unsigned int> indices;
    std::vector<T> values;

public:
    explicit sparse_vector(std::vector<unsigned int> indices = {}, std::vector<T> values = {}): indices(std::move(indices)), values(std::move(values)) {}

    int first_index() const;

    T first_val() const;

    bool operator<(const sparse_vector<T> &v) const;

    bool operator>(const sparse_vector<T> &v) const;

    template<typename S>
    friend sparse_vector<S> operator*(const S &factor, const sparse_vector<S> &v);

    void operator-=(const sparse_vector &v);
};


#endif //ANUBIS_SPARSE_VECTOR_HPP
