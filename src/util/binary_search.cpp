//
// Created by maertej on 29.09.19.
//

#include <vector>
#include <algorithm>

template<typename T>
static typename std::vector<T>::iterator binary_search(typename std::vector<T>::iterator begin, typename std::vector<T>::iterator end, T& element) {
    if(begin == end || *(end - 1) < element) return end;

    int min = begin;
    int max = end;
    while(min < max) {
        typename std::vector<T>::iterator mid = (min + max) / 2;
        if(*mid < element) min = mid + 1;
        else if(*mid > element) max = mid;
        else return mid;
    }
    return min;
}