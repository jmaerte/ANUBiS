//
// Created by maertej on 29.09.19.
//
#include <vector>

#ifndef ANUBIS_INDEX_LIST_HPP
#define ANUBIS_INDEX_LIST_HPP

namespace anubis {
    class index_list {
    private:
        std::vector<int> keys;
        std::vector<int> counter;

    public:
        index_list(int n) : keys(n), counter(1) {}

        void count(int key, int amount);

        int get(int key);

        int size();

        bool operator==(int &i) {
            return this == index_list(i);
        }

        bool operator==(index_list &list) {
            if (list.size() != size()) return false;
            auto a = keys.begin();
            auto b = list.keys.begin();
            auto aval = counter.begin();
            auto bval = list.counter.begin();
            for (; a != keys.end(); a++, b++, aval++, bval++) {
                if (*a != *b || *aval != *bval) return false;
            }
            return true;
        }
    };
}

#endif //ANUBIS_INDEX_LIST_HPP
