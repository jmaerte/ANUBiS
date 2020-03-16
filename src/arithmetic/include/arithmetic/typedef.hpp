//
// Created by jmaerte on 16.03.20.
//

#ifndef ANUBIS_SUPERBUILD_TYPEDEF_HPP
#define ANUBIS_SUPERBUILD_TYPEDEF_HPP

#include <ARITHMETIC_EXPORT.h>

typedef unsigned long long ULL;

/**
 * This union has 64 bits. It can serve either as an pointer to a memory block of ULLs or as an ULL of meta-data itself.
 */
union svec_node {
    ULL meta;
    ULL * value;
};

/**
 * A number is a memory block of two svec_nodes:
 *
 *  64      64
 * +-------+-------+
 * | meta  | value |
 * +-------+-------+
 *
 * where meta is of the form
 *
 *  1      31    16     16
 * +------+-----+------+-----+
 * | sign | pos | size | occ |
 * +------+-----+------+-----+
 *
 * and value points to an array of size @code{size}-many ULLs, where the first @code{occ} 32-bit block elements
 * are a word of the expansion of the number in base 2^32.
 */
typedef svec_node* num;

/**
 * A sparse vector is an array of svec_nodes of length 2*n + 1. n is thereby the number of non-zero elements that
 * can be saved.
 *
 * +------+-------+-------+-...-+-------+-------+
 * | meta | num_1 : num_1 | ... | num_k : num_k |
 * +------+-------+-------+-...-+-------+-------+
 *
 * where meta is of the form
 *
 *  32     32
 * +------+-----+
 * | size | occ |
 * +------+-----+.
 *
 */
typedef svec_node* s_vec;

struct ARITHMETIC_EXPORT NUM_COMPARATOR {
        bool operator()(num const& a, num const& b) const;
};

struct ARITHMETIC_EXPORT NUM_ABS_COMPARATOR {
        bool operator()(num const& a, num const& b) const;
};

#endif //ANUBIS_SUPERBUILD_TYPEDEF_HPP
