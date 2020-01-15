//
// Created by jmaerte on 30.11.19.
//

#include "potence.hpp"

template<typename T>
potence<T>::potence(const std::vector<T> & v, int k): mask(v.size()), k(k), N(v.size()), masked(v) {
    for (int i = 0; i < k; i++) {
        mask[i] = i;
    }
}

template<typename T>
void potence<T>::operator++() {
    if (k == 0) {
        k++;
    } else if (k == N) {
        _done = true;
    } else {
        int l = -1;
        for (int i = 0; i < k - 1; i++) {
            if (mask[i + 1] - mask[i] > 1) {
                mask[i]++;
                l = i;
                break;
            }
        }
        if (l < 0) {
            if (mask[k - 1] < N - 1) {
                mask[k - 1]++;
                l = k - 1;
            } else {
                k++;
                l = k;
            }
        }
        for (int i = 0; i < l; i++) {
            mask[i] = i;
        }
    }
}

template<typename T>
bool potence<T>::done() {
    return _done;
}



/**
* Compiler requires the explicit statement for compiling these template specializations:
*/

template class potence<int>;
template class potence<unsigned int>;