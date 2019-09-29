//
// Created by maertej on 29.09.19.
//

#ifndef ANUBIS_BLOOM_FILTER_HPP
#define ANUBIS_BLOOM_FILTER_HPP

#include <vector>
#include <math.h>
#include <cstdint>

template<typename Key, typename Hash = std::hash<Key>>
class bloom_filter {
public:
    bloom_filter(double p, std::size_t expected_size) : p(p),
                n_bins(- 2 * expected_size * log(p) / pow(log(2), 2)),
                n_hash_fns(n_bins / expected_size * log(2)) {}

    void put(Key key);
    bool has(Key key);
private:
    double p;
    std::size_t n_elements;
    std::size_t n_bins;
    std::size_t n_hash_fns;
    std::vector<bool> bins;

    std::pair<uint64_t, uint64_t> hash(Key *key, std::size_t len);
    inline uint64_t hash(uint8_t k, std::pair<uint64_t, uint64_t> hashes);
};


#endif //ANUBIS_BLOOM_FILTER_HPP
