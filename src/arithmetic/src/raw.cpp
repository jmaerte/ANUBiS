//
// Created by jmaerte on 04.12.20.
//

#include "arithmetic/factory/factory.hpp"
#include "../include/arithmetic/aux.hpp"
#include "arithmetic/operator.hpp"
#include "arithmetic/constants.hpp"
#include <math.h>

namespace jmaerte {
    namespace arith {
        namespace num {
            namespace aux {

                boost::mutex allocation_mutex {};

                int ACCESSIBLE = 0;
                int USED = 0;
                int FREED = 0;

                int REC_DIV_THRESHOLD = 32;
                int REC_MUL_THRESHOLD = 16;
                int REC_MUL33_THRESHOLD = 10000000;
                int REC_MUL55_THRESHOLD = 10000000;
                int DIVISOR_LIMIT = 8;

                std::vector<std::vector<UL>> V33 = {
                    {},
                    {},
                    {},
                    {},
                    {}
                };

                std::vector<std::vector<UL>> V55 = {
                    {},
                    {},
                    {},
                    {},
                    {},
                    {},
                    {},
                    {},
                    {}
                }; 

                //std::map<unsigned int, std::vector<std::vector<UL>>> REC_MUL_VANDERMONDE { };

                std::string STRINGIFY(UL* a, int occ) {
                    std::string res = "";
                    bool set = false;
                    UL pos;
                    for (int i = occ - 1; i >= 0; i--) {
                        pos = 1u << 31;
                        while (pos != 0) {
                            if (set) {
                                res += (a[i] & pos) != 0 ? "1" : "0";
                            } else {
                                if (a[i] & pos) {
                                    set = true;
                                    res += "1";
                                }
                            }
                            pos >>= 1;
                        }
                    }
                    if (!set) res += "0";
                    return res;
                }

                void INCREMENT_COUNTERS(int a, int u, int f) {
                    ACCESSIBLE += a;
                    USED += u;
                    FREED += f;
                }

                void PRINT_COUNTERS() {
                    std::cout << "Bytes - " << std::endl;
                    std::cout << "\t" << "accessible: " << ACCESSIBLE << "; used: " << USED << "; freed: " << FREED << std::endl;
                }

                void STRIP(const UL* dat, unsigned int* occ) {
                    for (; *occ > 0; (*occ)--) {
                        if (*(dat + *occ - 1) != 0) return;
                    }
                }

                int COMPARE_RAW(const UL* a, const UL* b, int length) {
                    const UL* stop = a - length;
                    for (; a != stop; ) {
                        if (*--a != *--b) return *a > *b ? 1 : -1;
                    }
                    return 0;
                }

                void MAKE_MULTI_OVERFLOW(ap_int& n, UL overflow) {
                    if (!IS_SINGLE(n)) throw std::runtime_error("Trying to make a number overflow that is already multi.");
                    //n.meta = ((GET_SIGN(n) ? 1ULL : 0ULL) << num::SIGN_SHIFT) | (((ULL) GET_POS(n)) << num::POS_SHIFT) | (2ULL << num::SIZE_SHIFT) | ((overflow != 0 ? 2ULL : ((ULL) GET_OCC(n))) << num::OCC_SHIFT);
                    n.meta = ((GET_SIGN(n) ? 1ULL : 0ULL) << num::SIGN_SHIFT) | (((ULL) GET_POS(n)) << num::POS_SHIFT) | (2ULL << num::SIZE_SHIFT) | ((overflow != 0 ? 2ULL : ((ULL) GET_OCC(n))) << num::OCC_SHIFT);
                    UL val = n.value;

                    allocation_mutex.lock();
                    n.values = new UL[2];
                    allocation_mutex.unlock();

                    n.values[0] = val;
                    n.values[1] = overflow;
                }

                void MAKE_MULTI_SIZE(ap_int& n, std::size_t size) {
                    if (!IS_SINGLE(n)) throw std::runtime_error("Trying to make a number multi size that is already multi.");
                    n.meta = ((GET_SIGN(n) ? 1ULL : 0ULL) << num::SIGN_SHIFT) | (((ULL) GET_POS(n)) << num::POS_SHIFT) | (((ULL) size) << num::SIZE_SHIFT) | ((n.value != 0 ? 1ULL : 0ULL) << num::OCC_SHIFT);
                    UL val = n.value;
//                    std::cout << "multi size " << size << std::endl;

                    allocation_mutex.lock();
                    n.values = new UL[size] { };
                    allocation_mutex.unlock();

                    n.values[0] = val;
                }

                void MAKE_SINGLE(ap_int& n) {
                    if (GET_OCC(n) > 1) throw std::runtime_error("Trying to make a number single that has higher order non-zero terms.");
                    if (IS_SINGLE(n)) return;
                    UL val = n.values[0];

                    allocation_mutex.lock();
                    delete[] n.values;
                    allocation_mutex.unlock();

                    n.meta = (1ULL << num::SINGLE_SHIFT) | ((GET_SIGN(n) ? 1ULL : 0ULL) << num::SIGN_SHIFT) | (((ULL) GET_POS(n)) << 2);
                    n.value = val;
                }

                void ENLARGE(ap_int& n) {
                    ENLARGE(n, 2 * GET_SIZE(n));
                    //ENLARGE(n, ceil(PHI * GET_SIZE(n)));
                }

                void ENLARGE(ap_int& n, int length) {
                    if (IS_SINGLE(n)) throw std::runtime_error("Trying to enlarge single number.");
                    if (length < GET_SIZE(n)) return;

                    allocation_mutex.lock();
                    UL* next = new UL[length] { };
                    allocation_mutex.unlock();

                    std::copy(n.values, n.values + GET_OCC(n), next);
                    
                    allocation_mutex.lock();
                    delete[] n.values;
                    allocation_mutex.unlock();

                    n.values = next;
                    num::SET_SIZE(n, length);
                }

                void INCREMENT(UL* a, int n) {
                    ULL sum;
                    UL carry = 1;
                    for (int l = 0; l < n; l++) {
                        sum = ((ULL) a[l]) + carry;
                        a[l] = sum & constants::L32;
                        carry = sum >> 32;
                        if (carry == 0) break;
                    }
                    if (carry != 0) a[n] = carry;
                }

                bool DECREMENT(UL* a, int n) {
                    for (int l = 0; l < n; l++) {
                        if (a[l] > 0) {
                            a[l]--;
                            return false;
                        }
                    }
                    return true;
                }

                UL ADD_RAW(UL* dest, UL* a, int n, UL* b, int m, UL carry) {
                    ULL sum;
                    for (int i = 0; i < n; i++) {
                        sum = (((ULL) a[i]) + (i < m ? b[i] : 0ULL)) + carry;
                        dest[i] = (sum & constants::L32);
                        carry = sum >> 32;
                    }
                    a[n] = carry;
                    return carry;
                }

                UL SUB_RAW(UL* dest, UL* a, int n, UL* b, int m, UL carry) {
                    ULL sum = 0;
                    for (int l = 0; l < n; l++) {
                        sum = (l < m ? b[l] : 0ULL) + carry;
                        carry = (sum >> 32) + (a[l] < (sum & constants::L32)) ? 1 : 0;
                        dest[l] = a[l] - (sum & constants::L32);
                    }
                    return carry;
                }

                // returns true iff a <= b.
                bool CMP(UL* a, int n, UL* b, int m) {
                    if (n != m) return n < m;
                    for (int i = 0; i < n; i++) {
                        if (a[i] != b[i]) return a[i] < b[i];
                    }
                    return true;
                }

                void ADD(ap_int& a, ap_int const& b) {
                    unsigned int n = GET_OCC(a);
                    unsigned int m = GET_OCC(b);

                    unsigned int max = n < m ? m : n;

                    if (num::GET_SIZE(a) < max) aux::ENLARGE(a, max + 1);

                    UL* a_dat = num::DATA(a);
                    const UL* b_dat = num::ABS(b);
                    ULL add = 0ULL;
                    ULL carry = 0;
                    for (int i = 0; i < max; i++) {
                        add = ((i < n) ? ((ULL) a_dat[i]) : 0ULL) + ((i < m) ? ((ULL) b_dat[i]) : 0ULL) + carry;
                        carry = add >> 32;
                        a_dat[i] = add;
                    }
                    if (carry) {
                        aux::ENLARGE(a, max + 1);
                        a.values[max] = carry;
                        num::SET_OCC(a, max + 1);
                    } else num::SET_OCC(a, max);
                }

                void C_ADD(ap_int& a, ap_int const& b) {
                    int n = GET_OCC(a);
                    //aux::ENLARGE(a, GET_OCC(a) + 1);
                    ULL sum = ((ULL) a.values[0]) + ((ULL) b.value);
                    ULL overflow = sum >> 32;
                    a.values[0] = sum;
                    for (int i = 1; i < n; i++) {
                        sum = ((ULL) a.values[i]) + overflow;
                        overflow = sum >> 32;
                        a.values[i] = sum;
                    }
                    if (overflow) {
                        ENLARGE(a, n + 1);
                        a.values[n] = overflow;
                        num::SET_OCC(a, n + 1);
                    }
                    /*unsigned int n = GET_OCC(a) + 1;
                    aux::STRIP(a.values, &n);
                    SET_OCC(a, n);*/
                }

                void SUB(ap_int& a, ap_int const& b) {
                    const UL* first;
                    unsigned int size_f;
                    const UL* sec;
                    unsigned int size_s;

                    int cmp = num::COMPARE_ABS(a, b);
                    if (cmp < 0) {
                        first = ABS(b);
                        size_f = num::GET_OCC(b);

                        num::SWITCH_SIGN(a);
                        if (num::IS_SINGLE(a)) num::aux::MAKE_MULTI_SIZE(a, size_f);
                        else aux::ENLARGE(a, size_f);

                        sec = ABS(a);
                        size_s = num::GET_OCC(a);
                    } else if (cmp == 0) {
                        num::DELETE_DATA(a);
                        num::NEW(a, num::GET_POS(a), false, 0ULL);
                        //std::cout << "SUBTRACTION OF MULTIS RETURNED 0" << std::endl;
                        return;
                    } else {
                        first = ABS(a);
                        size_f = num::GET_OCC(a);
                        sec = ABS(b);
                        size_s = num::GET_OCC(b);
                    }
                    UL* a_dat = DATA(a);

                    long long diff;
                    long long borrow = 0;
                    for (int i = 0; i < size_s || borrow != 0; i++) {
                        diff = *(first + i) - ((i < size_s) ? ((long long) *(sec + i)) : 0ll) - borrow;
                        borrow = (diff < 0 ? -1 : (diff >> 32));
                        *(a_dat + i) = diff;
                    }

                    /*ULL sub = 0ULL;
                    unsigned char carry = 0;
                    for (int i = 0; i < size_s || carry != 0; i++) {
                        sub = carry + ((i < size_s) ? ((ULL) *(sec + i)) : 0ULL);
                        carry = sub >> 32;
                        sub &= ((1ULL << 32) - 1);
                        if (*(first + i) < sub) carry++;
                        *(a_dat + i) = *(first + i) - sub;
                    }*/
                    unsigned int k = size_f;
                    num::aux::STRIP(a_dat, &k);
                    num::SET_OCC(a, k);
                }

                void C_SUB(ap_int& a, ap_int const& b) {
                    //std::cout << a.meta << " " << num::STRINGIFY(a) << " " << b.meta << " " << num::STRINGIFY(b) << std::endl;
                    unsigned char overflow = (a.values[0] < b.value) ? 1 : 0;
                    a.values[0] -= b.value;
                    for (int i = 1; overflow != 0; i++) {
                        overflow = (a.values[i] == 0) ? 1 : 0;
                        a.values[i]--;
                    }
                    unsigned int k = GET_OCC(a);
                    num::aux::STRIP(a.values, &k);
                    num::SET_OCC(a, k);
                    //std::cout << num::STRINGIFY(a) << std::endl;
                }

                void KARATSUBA(ap_int& res, ap_int const& a, ap_int const& b) {
                    unsigned int n = GET_OCC(a);
                    unsigned int m = GET_OCC(b);
                    NEW(res, GET_POS(a), GET_SIGN(a) ^ GET_SIGN(b), 0ULL);
                    aux::MAKE_MULTI_SIZE(res, n + m);

                    toom::REC_MUL(res.values, a.values, n, b.values, m);

                    unsigned int occ = n + m;
                    aux::STRIP(res.values, &occ);
                    num::SET_OCC(res, occ);
                }

                namespace toom {

                    struct multi {
                        bool sign;
                        int occ;
                        UL* data;
                    };

                    int REC_MUL_IMMUTABLE_ADD(UL* dest, UL* a, int br, UL* b, int lim) {
                        ULL sum;
                        ULL carry = 0;
                        for (int i = 0; i < br; i++) {
                            sum = ((ULL) a[i]) + (i < lim ? ((ULL) b[i]) : 0ULL) + carry;
                            carry = sum >> 32;
                            dest[i] = sum;
                        }
                        int l = br;
                        if (carry) {
                            dest[br] = carry;
                            l = br + 1;
                        }
                        return l;
                    }

                    int REC_MUL_MUTABLE_ADD(UL* a, UL* b, int lim) {
                        ULL sum;
                        ULL carry = 0;
                        int i = 0;
                        for (; i < lim; i++) {
                            sum = ((ULL) a[i]) + ((ULL) b[2 * lim + i]) + carry;
                            carry = sum >> 32;
                            a[i] = sum;
                        }
                        while (carry) {
                            sum = ((ULL) a[i]) + carry;
                            carry = sum >> 32;
                            a[i++] = sum;
                        }
                    }

                    bool REC_MUL_IMMUTABLE_SUB(UL* dest, UL* a, int l, UL* b, int br) {
                        bool sign = l == br;
                        if (sign) sign = aux::COMPARE_RAW(a + br, b + br, br) < 0;

                        long long diff;
                        long long borrow = 0;
                        if (sign) {
                            for (int i = 0; i < br; i++) {
                                diff = ((long long) b[i]) - (long long) a[i] + borrow;
                                borrow = diff < 0 ? -1 : 0;
                                dest[i] = diff;
                            }
                        } else {
                            for (int i = 0; i < br; i++) {
                                diff = ((long long) a[i]) - (long long) b[i] + borrow;
                                borrow = diff < 0 ? -1 : 0;
                                dest[i] = diff;
                            }
                            dest[br] = ((long long) a[br]) + borrow;
                        }
                        return sign;
                    }

                    bool REC_MUL_MUTABLE_SUB(UL* a, int l, UL* b, int lim) {
                        bool sign = l <= lim;
                        if (l == lim) sign = aux::COMPARE_RAW(a + l, b + l, l) < 0;

                        long long diff;
                        long long borrow = 0;
                        if (sign) {
                            for (int i = 0; i < lim; i++) {
                                diff = ((long long) b[i]) - (long long) a[i] + borrow;
                                borrow = diff < 0 ? -1 : 0;
                                a[i] = diff;
                            }
                            a[lim] = -((long long) a[lim]) + borrow;
                        } else {
                            int i = 0;
                            for (; i < lim; i++) {
                                diff = ((long long) a[i]) - (long long) b[i] + borrow;
                                borrow = diff < 0 ? -1 : 0;
                                a[i] = diff;
                            }
                            while (borrow) {
                                diff = ((long long) a[i]) + borrow;
                                borrow = diff < 0 ? -1 : 0;
                                a[i] = diff;
                            }
                        }
                        return sign;
                    }

                    void REC_MUL_SHIFT_DOUBLE(UL* a, int br) {
                        if (constants::UL_LEFTMOST & a[br]) throw std::runtime_error("OVERFLOW OCCURS WHEN SHIFTING!");
                        for (int i = br; i > 0; i++) {
                            a[i] = (a[i] << 1) | (a[i - 1] >> 31);
                        }
                        a[0] <<= 1;
                    }

                    void REC_MUL_SHIFT_HALF(UL* a, int br) {
                        for (int i = 0; i < br - 1; i++) {
                            a[i] = (a[i] >> 1) | (a[i + 1] << 31); 
                        }
                        a[br - 1] >>= 1;
                    }

                    void REC_MUL_EXACT_DIV_BY_3(UL* a, int lim) {
                        UL carry = 0;
                        ULL curr;
                        for (int i = lim - 1; i >= 0; i--) {
                            curr = ((((ULL) carry) << 32) + *(a + i));
                            *(a + i) = curr / 3;
                            carry = curr % 3;
                        }
                    }

                    extern "C" void REC_MUL(UL* dest, UL* a, int n, UL* b, int m) {
                        //std::cout << "KARATSUBA " << n << " " << m << std::endl;
                        if (n <= 0 || m <= 0) return;
                        register unsigned int max = n < m ? m : n;
                        if (max < REC_MUL_THRESHOLD) {

                            register ULL prod = 0ULL;
                            register ULL sum = 0ULL;
                            register ULL overflow = 0ULL;
                            for (int i = 0; i < n; i++) {
                                overflow = 0ULL;
                                for (int j = 0; j < m || overflow; j++) {
                                    prod = ((ULL) a[i]) * (j < m ? (ULL) b[j] : 0ULL) + overflow; // does never overflow itself
                                    sum = ((ULL) dest[i + j]) + (prod & constants::L32);
                                    dest[i + j] = sum;
                                    overflow = (sum >> 32) + (prod >> 32);
                                }
                            }
                        } else if (max < REC_MUL33_THRESHOLD) {
                            // MUL22
                            // TODO: IMPLEMENT UNBALANCED CASE
                            register unsigned int br = max % 2 == 0 ? max / 2 : (max / 2 + 1);

                            if (n < br || m < br) {
                                REC_MUL(dest, a, n < br ? n : br, b, m < br ? m : br);
                                REC_MUL(dest + br, a + (n < br ? 0 : br), n < br ? n : (n - br), b + (m < br ? 0 : br), m < br ? m : (m - br));
                            } else {
                                UL low1[br + 1];
                                UL low2[br + 1];
                                register ULL overflow_a = 0ULL, overflow_b = 0ULL;
                                register ULL sum;
                                for (int i = 0; i < br; i++) {
                                    sum = (i < n ? ((ULL) a[i]) : 0ULL) + ((br + i) < n ? ((ULL) a[br + i]) : 0ULL) + overflow_a;
                                    low1[i] = sum;
                                    overflow_a = sum >> 32;

                                    sum = (i < m ? ((ULL) b[i]) : 0ULL) + ((br + i) < m ? ((ULL) b[br + i]) : 0ULL) + overflow_b;
                                    low2[i] = sum;
                                    overflow_b = sum >> 32;
                                }
                                low1[br] = overflow_a;
                                low2[br] = overflow_b;

                                unsigned int k = br + 1;
                                unsigned int l = br + 1;
                                num::aux::STRIP(low1, &k);
                                num::aux::STRIP(low2, &l);

                                UL res2[2 * br + 2] = {0};
                                REC_MUL(dest, a, br < n ? br : n, b, br < m ? br : m); // res1
                                REC_MUL(res2, low1, k, low2, l);
                                REC_MUL(dest + 2 * br, a + br, n - br, b + br, m - br);// res3

                                // subtract the first and second half from res2, i.e. res2 = (a2 + a1)(b2+b1) - a1b1 - a2b2
                                register long long t;
                                register ULL carry = 0ULL;
                                for (int i = 0; i < 2 * br || carry; i++) {
                                    sum = (i < 2 * br ? dest[i] : 0ULL) + carry;
                                    t = res2[i] - (sum & constants::L32);
                                    res2[i] = t;
                                    carry = (sum >> 32) + (t < 0 ? 1 : 0);
                                }
                                for (int i = 2 * br; i < n + m || carry; i++) {
                                    sum = (i < n + m ? dest[i] : 0ULL) + carry;
                                    t = res2[i - 2 * br] - (sum & constants::L32);
                                    res2[i - 2 * br] = t;
                                    carry = (sum >> 32) + (t < 0 ? 1 : 0);
                                }

                                sum = 0ULL;
                                register ULL overflow = 0ULL;

                                for (int i = 0; (i < n + m - br && i < 2 * br + 2) || overflow; i++) {
                                    sum = ((ULL) dest[br + i]) + (i < 2 * br + 2 ? ((ULL) res2[i]) : 0ULL) + overflow;
                                    dest[br + i] = sum;
                                    overflow = sum >> 32;
                                }
                            }
                        } else if (max < REC_MUL55_THRESHOLD) {
                            // MUL33
                            /*register unsigned int br = max % 3 == 0 ? max / 3 : (max / 3 + 1);

                            multi w4 {false, 0, nullptr}, w3 {false, 0, nullptr}, w2 {false, 0, nullptr}, w0 {false, 0, nullptr};


                            UL curr1_a[br + 1] = {0};
                            UL curr2_a[br + 1] = {0};
                            UL curr1_b[br + 1] = {0};
                            UL curr2_b[br + 1] = {0};

                            w3.data = curr2_a;
                            w0.data = curr1_a;
                            w2.data = curr2_b;
                            w4.data = curr1_b;

                            UL* a0 = a, *a1 = a + br, *a2 = a + 2 * br;
                            UL* b0 = b, *b1 = b + br, *b2 = b + 2 * br;

                            int lim_a = n > 2 * br ? n - 2 * br : 0;
                            int lim_b = m > 2 * br ? m - 2 * br : 0;

                            REC_MUL(dest, a0, br, b0, br);
                            REC_MUL(dest + 4 * br, a2, lim_a, b2, lim_b);

                            // curr1_a = a2 + a0
                            // curr1_b = b2 + b0
                            REC_MUL_IMMUTABLE_ADD(w3, {false, br, a0}, {false, lim_a, a2});
                            REC_MUL_IMMUTABLE_ADD(w2, {false, br, b0}, {false, lim_b, b2});

                            // curr1_a = curr2_a - a1
                            // curr1_b = curr2_b - b1
                            REC_MUL_IMMUTABLE_SUB(w0, w3, {false, br, a1});
                            REC_MUL_IMMUTABLE_SUB(w4, w2, {false, br, b1});

                            // curr2_a = curr2_a + a1
                            // curr2_b = curr2_b + b1
                            REC_MUL_MUTABLE_ADD(w3, {false, br, a1});
                            REC_MUL_MUTABLE_ADD(w2, {false, br, b1});

                            multi w1 {false, 0, nullptr};

                            UL prod_a[2 * br + 2] = {0};
                            UL prod_b[2 * br + 2] = {0};

                            REC_MUL(prod_a, w3.data, w3.occ, w2.data, w2.occ);
                            REC_MUL(prod_b, w0.data, w0.occ, w4.data, w4.occ);
                            

                            if (!sign_a) {
                                REC_MUL_MUTABLE_ADD(curr1_a, a2, lim_a);
                            } else {
                                sign_a = !REC_MUL_MUTABLE_SUB(curr1_a, l_a, a2, lim_a);
                            }
                            if (!sign_b) {
                                REC_MUL_MUTABLE_ADD(curr1_b, b2, lim_b);
                            } else {
                                sign_b = !REC_MUL_MUTABLE_SUB(curr1_b, l_b, b2, lim_b);
                            }

                            REC_MUL_SHIFT_DOUBLE(curr1_a, br);
                            REC_MUL_SHIFT_DOUBLE(curr1_b, br);

                            if (sign_a) {
                                REC_MUL_MUTABLE_ADD(curr1_a, a0, br);
                            } else {
                                sign_a = REC_MUL_MUTABLE_SUB(curr1_a, curr1_a[br] ? br + 1 : br, a0, br);
                            }
                            if (sign_b) {
                                REC_MUL_MUTABLE_ADD(curr1_b, b0, br);
                            } else {
                                sign_b = REC_MUL_MUTABLE_SUB(curr1_b, curr1_b[br] ? br + 1 : br, b0, br);
                            }

                            UL prod[2 * br + 2] = {0};
                            bool sign = sign_a ^ sign_b;

                            REC_MUL(prod, curr1_a, curr1_a[br] ? br + 1 : br, curr2_a, curr2_a[br] ? br + 1 : br);

                            if (sign) {
                                REC_MUL_MUTABLE_ADD(prod, prod_a, 2 * br + 2);
                            } else {
                                REC_MUL_MUTABLE_SUB(prod, 2 * br + 2, prod_a, 2 * br + 2);
                            }

                            REC_MUL_EXACT_DIV_BY_3(prod, 2 * br + 2);

                            bool sign_prod_a = REC_MUL_MUTABLE_SUB(prod_a, 2 * br + 2, prod_b, 2 * br + 2);

                            REC_MUL_SHIFT_HALF(prod_a, 2 * br + 2);

                            bool sign_prod_b = REC_MUL_MUTABLE_SUB(prod_b, 2 * br + 2, dest, 2 * br);*/



                        } else {
                            // MUL55
                            throw std::runtime_error("HIGHER MULTIPLICATION NOT IMPLEMENTED");
                        }
                    }
                }

                extern "C" int i_LC(UL* dest, long long lambda, UL* a, int n, long long mu, UL* b, int m) {
                    int max = n < m ? m : n;
                    if (max == 0) return 0;
                    bool sign_x = lambda < 0;
                    bool sign_y = mu < 0;

                    if (sign_x == sign_y) {
                        // just add both.
                        ULL x = (ULL) (sign_x ? -lambda : lambda);
                        ULL y = (ULL) (sign_y ? -mu : mu);

                        if ((x < 0 ? -x : x) >> 32 || (y < 0 ? -y : y) >> 32) throw std::runtime_error("COEFFICIENTS OF LINEAR COMBINATION ARE NOT SINGLE LIMB! " + std::to_string(x) + " " + std::to_string(y)); 

                        ULL p1, p2;
                        ULL carry = 0;

                        for (int i = 0; i < max || carry; i++) {
                            p1 =  i < n ? (x * a[i]) : 0ULL;
                            p2 = (i < m ? (y * b[i]) : 0ULL) + (carry & constants::L32) + (p1 & constants::L32); // can't overflow
                            carry = (carry >> 32) + (p1 >> 32) + (p2 >> 32);

                            dest[i] = p2;
                        }
                        unsigned int occ = max + 3;
                        aux::STRIP(dest, &occ);
                        return sign_x ? -occ : occ;
                    } else {
                        
                        ULL x = (ULL) (sign_x ? -lambda : lambda);
                        ULL y = (ULL) (sign_y ? -mu : mu);

                        bool cmp = false;
                        int i = max - 1;

                        ULL p1 = (i < n) ? (x * a[i]) : 0ULL;
                        ULL p2 = (i < m) ? (y * b[i]) : 0ULL;
                        if ((p1 >> 32) != (p2 >> 32)) cmp = (p1 >> 32) < (p2 >> 32);
                        else {
                            p1 = p1 << 32;
                            p2 = p2 << 32;
                            ULL c1, c2;

                            for (i--; i >= 0; i--) {
                                p1 += i < n ? x * a[i] : 0ULL;
                                p2 += i < m ? y * b[i] : 0ULL;
                                if (p1 != p2) {
                                    cmp = p1 < p2;
                                    break;
                                }
                                p1 = p1 << 32;
                                p2 = p2 << 32;
                            }
                            if (i <= 0) {
                                //all previous were equal
                                if (p1 == p2) {
                                    return 0;
                                } else {
                                    cmp = p1 < p2;
                                }
                            }
                        }
                        
                        // cmp = true iff a < b.
                        if (cmp) {
                            std::swap(x, y);
                            std::swap(a, b);
                            std::swap(n, m);
                        }

                        bool sign = cmp ^ sign_x;

                        if (x >> 32 || y >> 32) throw std::runtime_error("COEFFICIENTS OF LINEAR COMBINATION ARE NOT SINGLE LIMB! " + std::to_string(x) + " " + std::to_string(y)); 

                        long long carry = 0;
                        long long curr;

                        for (int i = 0; i < max || carry; i++) {
                            p1 = i < n ? (x * a[i]) : 0ULL;
                            p2 = i < m ? (y * b[i]) : 0ULL; // can't overflow
                            curr = (p1 & constants::L32) - (p2 & constants::L32) + carry;
                            dest[i] = curr;
                            carry = (curr < 0 ? -1 : 0) + (carry < 0 ? -((-carry) >> 32) : (carry >> 32));
                            carry += (long long)(p1 >> 32) - (long long)(p2 >> 32);
                        }
                        unsigned int occ = max + 3;
                        aux::STRIP(dest, &occ);
                        return sign ? -occ : occ;
                    }
                }

                void LEHMER_SWAP(num::ap_int& a, num::ap_int& b, long long A, long long B, long long C, long long D) {
                    unsigned int n = GET_OCC(a);
                    unsigned int m = GET_OCC(b);

                    unsigned int max = n < m ? m : n;

                    UL* dest_a = new UL[max + 3] { };
                    UL* dest_b = new UL[max + 3] { };
                    int occ_a = aux::i_LC(dest_a, GET_SIGN(a) ? -A : A, DATA(a), n, GET_SIGN(b) ? -B : B, DATA(b), m);
                    int occ_b = aux::i_LC(dest_b, GET_SIGN(a) ? -C : C, DATA(a), n, GET_SIGN(b) ? -D : D, DATA(b), m);

                    bool neg_a = occ_a < 0;
                    bool neg_b = occ_b < 0;

                    num::DELETE_DATA(a);
                    num::DELETE_DATA(b);

                    num::NEW_MULTI(a, num::GET_POS(a), neg_a, neg_a ? -occ_a : occ_a, max + 3, dest_a);
                    num::NEW_MULTI(b, num::GET_POS(b), neg_b, neg_b ? -occ_b : occ_b, max + 3, dest_b);
                }

                extern "C" void SHIFT_LEFT(UL* v, int n, int words, int s) {
                    if (s > 0) {
                        if (v[n - 1] >> (32 - s)) {
                            v[n + words] = v[n - 1] >> (32 - s);
                        }
                        for (int i = n + words - 1; i > words; i--) {
                            v[i] = (v[i - words] << s) | (v[i - words - 1] >> (32 - s));
                        }
                        v[words] = v[0] << s;
                    } else {
                        for (int i = n - 1; i >= 0; i--) {
                            v[i + words] = v[i];
                        }
                    }
                    for (int i = 0; i < words; i++) {
                        v[i] = 0;
                    }
                }

                extern "C" void SHIFT_RIGHT(UL* v, int n, int words, int s) {
                    if (s > 0) {
                        for (int i = words; i < n + words; i++) {
                            v[i - words] = (v[i] >> s) | (v[i + 1] << (32 - s));
                        }
                        v[n] = v[n + words] >> s;
                    } else {
                        for (int i = 0; i < n; i++) {
                            v[i] = v[i + words];
                        }
                    }
                    for (int i = n; i < n + words + 1; i++) {
                        v[i] = 0;
                    }
                }

                extern "C" void KNUTH_DIV(UL* dest, UL* u, int m, UL* v, int n) {
                    if (m < 0) return;
                    ULL qhat, rhat, p, k;
                    long long t;

                    for (int j = m - n; j >= 0; j--) {
                        ULL curr = (((ULL) u[j + n]) << 32) | ((ULL) u[j + n - 1]);
                        qhat = curr / v[n - 1];
                        rhat = curr % v[n - 1];

                        test_remainder:
                        if (qhat >= constants::base || // the operation overflowed, that is: u[j + n] = MAX_VALUE_ULL
                            qhat * (ULL) v[n - 2] > constants::base * rhat + (ULL) u[j + n - 2]) {
                            qhat--;
                            rhat += (ULL) v[n - 1];
                            if (rhat < constants::base) goto test_remainder;
                        }

                        k = 0;
                        for (int i = 0; i < n; i++) {
                            p = qhat * (ULL) v[i] + k; // <= (b-1)^2 = (b^2 - 1) - 2 (b - 1)
                            t = u[i + j] - (p & constants::L32);
                            u[i + j] = t;
                            k = (p >> 32) + (t < 0 ? 1 : 0);
                        }
                        t = u[j + n] - k;
                        u[j + n] = t;

                        dest[j] = qhat;
                        if (t < 0) {
                            dest[j] = dest[j] - 1;
                            k = 0;
                            for (int i = 0; i < n; i++) {
                                p = (ULL) u[i + j] + (ULL) v[i] + k;
                                u[i + j] = p & constants::L32;
                                k = p >> 32;
                            }
                            u[j + n] = u[j + n] + k;
                        }
                    }
                }

                /** WE ASSUME v TO BE NORMALIZED.
                */
                extern "C" void BASECASE_21_DIV(UL* dest, UL* u, int size_u, UL* v, int n) {
                    if (size_u < n) return;
                    if (n % 2 != 0 || n < REC_DIV_THRESHOLD) {
                        KNUTH_DIV(dest, u, (size_u < 2 * n ? size_u : 2 * n) - 1, v, n); // no shift needed - performed before.
                    } else {
                        BASECASE_32_DIV(dest + n / 2, u + n / 2, size_u - n / 2, v, n / 2);
                        BASECASE_32_DIV(dest, u, size_u, v, n / 2);
                    }
                }

                extern "C" void BASECASE_32_DIV(UL* dest, UL* u, int size_u, UL* v, int n) {
                    if (size_u < 2 * n) return;
                    UL* u1 = u + 2 * n, *u2 = u + n, *u3 = u;
                    UL* v1 = v + n, *v2 = v;

                    int lim = size_u - 2 * n < n ? size_u - 2 * n : n;

                    int cmp = lim - n;
                    if (cmp == 0) cmp = COMPARE_RAW(u1 + n, v1 + n, n);
                    if (cmp < 0) {
                        BASECASE_21_DIV(dest, u2, size_u - n, v1, n);
                    } else {
                        // thus lim = n.
                        for (int i = 0; i < n; i++) {
                            dest[i] = constants::UL_MAX;
                        }
                        // u2 = (u1,u2) - (B1,0) + (0,B1) = (A1,A2) - QB1
                        ULL carry = 0;
                        ULL sum;
                        for (int i = 0; i < n; i++) {
                            // u2 - (0, B1)
                            sum = ((ULL) u2[i]) + ((ULL) v1[i]) + carry;
                            u2[i] = sum;
                            carry = sum >> 32;
                        }
                        long long diff;
                        long long borrow = carry;
                        int i = 0;
                        for (; i < n; i++) {
                            diff = ((long long) u1[i]) - ((long long) v1[i]) + borrow;
                            borrow = (diff < 0 ? -1 : (diff >> 32));
                            u1[i] = diff;
                        }
                        // we know that this can't underflow.
                        while (borrow) {
                            diff = ((long long) u1[i]) + borrow;
                            u1[i++] = diff;
                            borrow = (diff < 0 ? -1 : (diff >> 32));
                        }
                    }
                    UL d[n + lim] = { 0 };
                    toom::REC_MUL(d, dest, lim, v2, n);
                    long long borrow = 0ll;
                    long long diff;
                    int i = 0;
                    for (; i < n + lim; i++) {
                        diff = ((long long) u3[i]) - ((long long) d[i]) - borrow;
                        borrow = diff < 0 ? 1 : 0;
                        u3[i] = diff;
                    }
                    for (; i < size_u && borrow != 0; i++) {
                        diff = ((long long) u3[i]) - borrow;
                        borrow = diff < 0 ? 1 : 0;
                        u3[i] = diff;
                    }
                    if (borrow) {
                        ULL carry = 0;
                        ULL sum;
                        while (carry == 0) {
                            for (int j = 0; j < 2 * n; j++) {
                                sum = ((ULL) u3[j]) + ((ULL) v2[j]) + carry;
                                u3[j] = sum;
                                carry = sum >> 32;
                            }
                            for (int j = 0; j < lim && carry; j++) {
                                sum = ((ULL) u1[j]) + carry;
                                u1[j] = sum;
                                carry = sum >> 32;
                            }
                            DECREMENT(dest, n);
                        }
                    }
                }

                extern "C" void DIV(UL* dest, UL* u, int m, UL* v, int n, int s) {
                    SHIFT_LEFT(u, m, s / 32, s % 32);

                    /*if (s > 0) {
                        u[m] = u[m - 1] >> (32 - s);
                        for (int i = m - 1; i > 0; i--) {
                            u[i] = (u[i] << s) | (u[i - 1] >> (32 - s));
                        }
                        u[0] = u[0] << s;
                    }*/
                    if (/*n == m || */n <= REC_DIV_THRESHOLD) {
                        /**
                         * KNUTHS ALGORITHM D
                         */

                        KNUTH_DIV(dest, u, m, v, n);
                    } else {

                        n += s / 32;

                        int blocks;
                        int M = m + s / 32 + 1;
                        if (M % n) {
                            blocks = M / n + 1;
                        } else {
                            blocks = M / n;
                            if (u[M - 1] & constants::UL_LEFTMOST) blocks++;
                        }

                        for (int i = blocks - 2; i >= 0; i--) {
                            BASECASE_21_DIV(dest + i * n, u + i * n, M - i * n/* - 1*/, v, n);
                        }

                        /* OLD RECURSIVE APPROACH*/
                        // by calling the prepare function on the divisor first we assured that it has an even number of allocated data and is shifted properly.

                        // recursively determine the division by illustrating the numbers in base p = ceil(n / 2).
                        /*int p = n % 2 == 0 ? n / 2 : (n / 2 + 1);

                        // normalize u
                        if (n % 2 == 0) {
                            if (s > 0) {
                                u[m] = u[m - 1] >> (32 - s);
                                for (int i = m - 1; i > 0; i--) {
                                    u[i] = (u[i] << s) | (u[i - 1] >> (32 - s));
                                }
                                u[0] = u[0] << s;
                            }
                        } else {
                            if (s > 0) {
                                u[m + 1] = u[m - 1] >> (32 - s);
                                for (int i = m; i > 1; i--) {
                                    u[i] = (u[i - 1] << s) | (u[i - 2] >> (32 - s));
                                }
                                u[1] = u[0] << s;
                                u[0] = 0;
                            } else {
                                for (int i = m; i > 0; i--) {
                                    u[i] = u[i - 1];
                                }
                                u[0] = 0;
                            }
                        }

                        int m_rec = m % p == 0 ? m / p : (m / p + 1);
                        int n_rec = 2;

                        UL* div = v + p;

                        for (int j = m_rec - n_rec; j >= 0; j--) {
                            UL qhat[p + 1] { };
                            UL* rhat = u + p * (j + n_rec - 1);

                            unsigned int rhat_size = (j == m_rec - n_rec) ? m - p * (j + n_rec - 1) : 2 * p;

                            aux::DIV(qhat, rhat, rhat_size, div, p, 0);

                            unsigned int qhat_size = p + 1;
                            aux::STRIP(qhat, &qhat_size);
                            aux::STRIP(rhat, &rhat_size);

                            UL prod[qhat_size + p] { };
                            aux::REC_MUL(prod, qhat, qhat_size, v, p);
                            unsigned int prod_size = qhat_size + p;
                            aux::STRIP(prod, &prod_size);

                            UL* prev = u + p * (j + n_rec - 2);

                            test_remainder_rec:

                            bool sub = qhat_size > p;
                            if (!sub) {
                                // compare qhat * v[n-2] > constants::base * rhat + u[j + n - 2]
                                if (prod_size > 2 * p) sub = true;
                                else {
                                    int i = 2 * p - 1;
                                    for (; i >= p; i--) {
                                        if (prod[i] != rhat[i - p]) {
                                            sub = prod[i] > rhat[i - p];
                                            break;
                                        }
                                    }
                                    if (!sub && i < p) {
                                        for (; i >= 0; i--) {
                                            if (prod[i] != prev[i]) {
                                                sub = prod[i] > prev[i];
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                            if (sub) {
                                DECREMENT(qhat, qhat_size);
                                if (qhat[qhat_size - 1] == 0) qhat_size--;
                                if (ADD_RAW(rhat, rhat, rhat_size, v + p, p, 0) != 0) {
                                    rhat_size++;
                                }
                                if (rhat_size < p) goto test_remainder_rec;
                            }

                            UL curr[2 * p + qhat_size + 1] { };
                            UL carry = 0;
                            for (int i = 0; i < n_rec; i++) {
                                aux::REC_MUL(curr + i * p, qhat, qhat_size, v + i * p, p);
                                carry = SUB_RAW(u + (i + j) * p, u + (i + j) * p, p, curr + i * p, p, carry);
                            }
                            bool borrow = carry != 0 ? DECREMENT(u + (j + n) * p, p) : false;
                            ADD_RAW(dest + j * p, dest + j * p, qhat_size, qhat, qhat_size, 0);

                            if (borrow) {
                                DECREMENT(dest + j * p, p);

                                carry = 0;
                                for (int i = 0; i < n_rec; i++) {
                                    carry = ADD_RAW(u + (i + j) * p, u + (i + j) * p, p, v + i * p, p, carry);
                                }
                                if (carry != 0) INCREMENT(u + (j + n) * p, p);
                            }
                        }*/
                    }
                    SHIFT_RIGHT(u, m, s / 32, s % 32);
                    /*if (s > 0) {
                        // de-normalize
                        for (int i = 0; i < n - 1; i++) {
                            u[i] = (u[i] >> s) | (u[i + 1] << (32 - s));
                        }
                        u[n - 1] = u[n - 1] >> s;
                    }*/
                }
            }
        }
    }
}
