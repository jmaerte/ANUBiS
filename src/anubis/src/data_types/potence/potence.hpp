//
// Created by jmaerte on 30.11.19.
//

#ifndef ANUBIS_PROJECT_POTENCE_HPP
#define ANUBIS_PROJECT_POTENCE_HPP

#include <vector>

template<typename T>
class potence {
private:
    std::vector<int> mask;
    std::vector<T> masked;
    bool _done = false;
    int k, N;
public:

    class iterator : public std::iterator<std::forward_iterator_tag, T> {
        int i;
        potence * p;

    public:
        iterator(potence * p, int i): p(p), i(i) {}
        iterator& operator++() {
            i++;
            return *this;
        }
        bool operator==(const iterator& rhs) {
            return i == rhs.i && p == rhs.p;
        }
        bool operator!=(const iterator& rhs) {
            return i != rhs.i || p != rhs.p;
        }
        T& operator*() {
            return p->masked[p->mask[i]];
        }
    };

    explicit potence(const std::vector<T> &, int k = 0);

    bool done();
    void operator++();

    int order() {
        return k;
    }

    T get(int i) {
        return masked[mask[i]];
    }

    iterator begin() {
        return iterator(this, 0);
    }

    iterator end() {
        return iterator(this, k);
    }
};

#endif //ANUBIS_PROJECT_POTENCE_HPP
