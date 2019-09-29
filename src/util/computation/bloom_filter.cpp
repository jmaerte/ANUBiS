//
// Created by maertej on 29.09.19.
//

#include <array>
#include "bloom_filter.hpp"

template<typename Key, typename Hash>
bool bloom_filter<Key, Hash>::has(Key key) {
    return false;
}

template<typename Key, typename Hash>
uint64_t bloom_filter<Key, Hash>::hash(uint8_t k, std::pair<uint64_t, uint64_t> hashes) {
    return (hashes.first + k * hashes.second) % n_bins;
}

static uint64_t ROL(uint64_t a, uint64_t b) {
    return (a << b) | (a >> (64 - b));
}

template<typename Key, typename Hash>
std::pair<uint64_t, uint64_t> bloom_filter<Key, Hash>::hash(Key *key, std::size_t len) {
    uint64_t hash1 = 0;
    uint64_t hash2 = 0;
    const uint16_t* raw = (const uint16_t*)key;
    const int n_blocks = len / 32;
    uint64_t c1 = 0xcc9e2d51;
    uint64_t c2 = 0x1b873593;
    uint64_t r1 = 15;
    uint64_t r2 = 13;
    uint64_t m = 5;
    uint64_t n = 0xe6546b64;

    for (int i = 0; i < n_blocks; i++) {
        uint8_t k1 = raw[2*i];
        uint8_t k2 = raw[2*i + 1];

        k1 = k1 * c1; k2 = k2 * c1;
        k1 = ROL(k1, r1); k2 = ROL(k2, r1);
        k1 = k1 * c2; k2 = k2 * c2;
        hash1 ^= k1; hash2 ^= k2;
        hash1 = ROL(hash1, r2); hash2 = ROL(hash2, r2);
        hash1 = hash1 * m + n; hash2 = hash2 * m + n;
    }



    return {hash1, hash2};
}

