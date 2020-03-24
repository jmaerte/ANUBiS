//
// Created by jmaerte on 23.03.20.
//

#include "constants.hpp"

typedef unsigned long long ULL;

const int BYTES_PER_ULL = sizeof(ULL);
const int ULL_SIZE = 8 * BYTES_PER_ULL;
// first 32 bits
const ULL L_MASK = (1ULL << (ULL_SIZE / 2)) - 1;
// first 16 bits
const ULL LL_MASK = (1ULL << (ULL_SIZE / 4)) - 1;
// last 32 bits
const ULL H_MASK = L_MASK << (ULL_SIZE / 2);
const ULL NUM_SIGN_MASK = 1ULL << 63;
// last 32 bits without the most significant
const ULL NUM_POS_MASK = H_MASK ^ NUM_SIGN_MASK;
const ULL NUM_SIZE_MASK = L_MASK & ~LL_MASK;
const ULL NUM_OCC_MASK = LL_MASK;
const ULL FIFTEEN = LL_MASK & ~(1u << 15);
const ULL MASKS[] = {
        H_MASK, L_MASK
};
const ULL ULL_MAX = ~0ULL;
const ULL UI_MAX = (~0ULL) >> 32;
const ULL BASE = 1ULL << 32;