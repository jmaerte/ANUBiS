//
// Created by jmaerte on 04.12.20.
//

#include "binary.hpp"
#include "arithmetic/operator.hpp"
#include "arithmetic/typedef.hpp"
#include "../include/arithmetic/aux.hpp"

namespace jmaerte {
    namespace arith {
        namespace num {

            bool PRINT = false;

            void PRINT_COUNTERS() {
                aux::PRINT_COUNTERS();
            }

            void ADD(ap_int& a, ap_int const& b) {
                if (GET_SIGN(a) == GET_SIGN(b)) {
                    if (IS_SINGLE(a)) {
                        if (IS_SINGLE(b)) {
                            ULL sum = ((ULL) a.value) + ((ULL) b.value);
                            ULL overflow = sum >> 32;
                            if (overflow) aux::MAKE_MULTI_OVERFLOW(a, overflow);
                            *num::DATA(a) = sum;
                        } else {
                            aux::MAKE_MULTI_SIZE(a, GET_OCC(b) + 1);
                            const UL* dat = num::ABS(b);
                            ULL sum = ((ULL) a.values[0]) + ((ULL) b.values[0]);
                            a.values[0] = sum;
                            ULL overflow = sum >> 32;
                            for (int i = 1; i < GET_OCC(b) || overflow; i++) {
                                sum = ((ULL) b.values[i]) + overflow;
                                overflow = sum >> 32;
                                a.values[i] = sum;
                            }
                            unsigned int n = GET_OCC(b) + 1;
                            aux::STRIP(a.values, &n);
                            num::SET_OCC(a, n);
                        }
                    } else {
                        if (IS_SINGLE(b)) {
                            aux::C_ADD(a, b);
                        } else {
                            aux::ADD(a, b);
                        }
                    }
                } else {
                    if (IS_SINGLE(a)) {
                        if (IS_SINGLE(b)) {
                            if (a.value < b.value) {
                                num::SWITCH_SIGN(a);
                                a.value = b.value - a.value;
                            } else {
                                a.value -= b.value;
                            }
                        } else {
                            unsigned int pos = num::GET_POS(a);
                            num::ap_int x;
                            num::COPY(x, a);
                            num::COPY(a, b);
                            num::SET_POS(a, pos);
                            aux::C_SUB(a, x);
                        }
                    } else {
                        if (IS_SINGLE(b)) {
                            aux::C_SUB(a, b);
                        } else {
                            aux::SUB(a, b);
                        }
                    }
                }
            }

            int PREPARE_DIVISOR(ap_int& b) {
                if (IS_SINGLE(b)) return 0;

                unsigned int n = GET_OCC(b);

                int k;

                if (n > aux::REC_DIV_THRESHOLD) {
                    int m = 1;
                    while (m * aux::DIVISOR_LIMIT <= n) {
                        m <<= 1;
                    }
                    int j = n % m == 0 ? n / m : (n / m + 1);

                    k = j * m;
                    aux::ENLARGE(b, k + 2);
                } else {
                    k = n;
                    aux::ENLARGE(b, k + 1);
                }

                UL* v = num::DATA(b);
                UL div = v[n - 1];

                int shift = 31 - arith::util::LOG2(div);
                aux::SHIFT_LEFT(v, n, k - n, shift);

                return shift + 32 * (k - n);
            }


            void DIV(ap_int& quot, ap_int& a, ap_int& b, int s) {
                //std::cout << "DIV" << std::endl;
                bool sign = num::GET_SIGN(a);

                if (IS_SINGLE(b)) {
                    // Divide by constant
                    NEW(quot, GET_POS(a), GET_SIGN(a) ^ GET_SIGN(b), 0ULL);
                    if (num::IS_SINGLE(a)) {
                        if(a.value < b.value) return;
                        else {
                            quot.value = a.value / b.value;
                            num::NEW(a, GET_POS(a), false, a.value % b.value);
                        }
                    } else {
                        unsigned int n = num::GET_OCC(a);

                        aux::MAKE_MULTI_SIZE(quot, n);

                        ULL carry = 0ULL;
                        ULL q;
                        ULL curr;
                        ULL rem;
                        const UL* a_dat = num::ABS(a);
                        UL* res = num::DATA(quot);
                        for (int i = n - 1; i >= 0; i--) {
                            curr = ((((ULL) carry) << 32) + *(a_dat + i));
                            *(res + i) = curr / b.value;
                            carry = curr % b.value;
                        }
                        num::aux::STRIP(res, &n);
                        num::SET_OCC(quot, n);

                        num::DELETE_DATA(a);
                        num::NEW(a, GET_POS(a), false, carry);
                    }
                    //std::cout << "QUOT: " << num::STRINGIFY(quot) << "; REMAINDER: " << num::STRINGIFY(a) << std::endl;
                } else {
                    unsigned int m = GET_OCC(a);
                    unsigned int n = GET_OCC(b);

                    if (n > m) {
                        NEW(quot, GET_POS(a), false, 0u);
                        return;
                    }
                    aux::ENLARGE(a, m + s / 32 + 2);

                    num::NEW(quot, num::GET_POS(a), num::GET_SIGN(a) ^ num::GET_SIGN(b), 0ULL);
                    aux::MAKE_MULTI_SIZE(quot, m - n + 1);

                    UL* u = a.values;
                    UL* v = b.values;
                    UL* res = quot.values;

                    aux::DIV(res, u, m, v, n, s);

                    // remainder is always the smallest positive (thus unique) remainder.
                    if (num::GET_SIGN(a)) num::SWITCH_SIGN(a);
                    
                    unsigned int occ = n;
                    aux::STRIP(ABS(a), &occ);
                    num::SET_OCC(a, occ);

                    occ = m - n + 1;
                    aux::STRIP(ABS(quot), &occ);
                    num::SET_OCC(quot, occ);
                }

                if (sign) {
                    num::SWITCH_SIGN(a);
                }
            }

            /*
            * We assume b >= a.
            * 
            * Sets a to be the gcd of a and b.
            * x = a / gcd, y = b / gcd.
            * s and t are such that 
            * x = s a + t b.
            */
            void GCD(ap_int& s, ap_int& t, ap_int& q_a, ap_int& q_b, ap_int& a, ap_int& b) {
                //std::cout << "GCD " << num::STRINGIFY(a) << " " << num::STRINGIFY(b) << std::endl;

                bool sign_a = num::GET_SIGN(a);
                bool sign_b = num::GET_SIGN(b);

                if (sign_a) num::SWITCH_SIGN(a);
                if (sign_b) num::SWITCH_SIGN(b);

                int n = GET_OCC(a);
                int m = GET_OCC(b);

                num::NEW(s, 0, false, 1u);
                num::NEW(t, 0, false, 0u);

                num::NEW(q_a, 0, false, 1u);
                num::NEW(q_b, 0, false, 0u);

                num::ap_int quot;
                num::ap_int lambda;

                while (GET_OCC(b) > 1) {
                    n = GET_OCC(a);
                    m = GET_OCC(b);

                    long long x = n > 1 ? a.values[n - 1] : a.value;
                    long long y = (m == n ? b.values[m - 1] : 0ll);

                    long long A = 1, B = 0, C = 0, D = 1;

                    while ((y + C) != 0 && (y + D) != 0) {
                        long long q1 = (x + A) / (y + C);
                        long long q2 = (x + B) / (y + D);

                        if (q1 == q2) {
                            long long t = A - q1 * C;
                            A = C;
                            C = t;
                            t = B - q1 * D;
                            B = D;
                            D = t;
                            t = x - q1 * y;
                            x = y;
                            y = t;
                        } else {
                            break;
                        }
                        if (B == 0) break;
                    }
                    if (B == 0) {
                        // do one normal iteration of euclidean algorithm
                        int shift = num::PREPARE_DIVISOR(b);
                        num::DIV(quot, a, b, shift);
                        num::DENORMALIZE_DIVISOR(b, shift);

                        num::SWAP_VALUES(a, b);
                        SWAP_VALUES(s, q_b);
                        num::i_MUL(lambda, quot, s);
                        num::SWITCH_SIGN(lambda);
                        num::ADD(q_b, lambda);

                        num::DELETE_DATA(lambda);

                        SWAP_VALUES(t, q_a);
                        num::i_MUL(lambda, quot, t);
                        num::SWITCH_SIGN(lambda);
                        num::ADD(q_a, lambda);

                        num::DELETE_DATA(lambda);
                        num::DELETE_DATA(quot);
                    } else {
                        aux::LEHMER_SWAP(a, b, A, B, C, D);
                        aux::LEHMER_SWAP(s, q_b, A, B, C, D);
                        aux::LEHMER_SWAP(t, q_a, A, B, C, D);
                    }
                }

                while (!num::IS_ZERO(b)) {
                    //std::cout << num::STRINGIFY(a) << " " << num::STRINGIFY(b) << " " << num::STRINGIFY(s) << " " << num::STRINGIFY(t) << std::endl;
                    int shift = PREPARE_DIVISOR(b);
                    DIV(quot, a, b, shift);
                    DENORMALIZE_DIVISOR(b, shift);

                    SWAP_VALUES(a, b);

                    SWAP_VALUES(s, q_b);
                    num::i_MUL(lambda, quot, s);
                    num::SWITCH_SIGN(lambda);
                    num::ADD(q_b, lambda);

                    num::DELETE_DATA(lambda);

                    SWAP_VALUES(t, q_a);
                    num::i_MUL(lambda, quot, t);
                    num::SWITCH_SIGN(lambda);
                    num::ADD(q_a, lambda);

                    num::DELETE_DATA(lambda);

                    num::DELETE_DATA(quot);
                }
                
                num::SET_SIGN(q_a, sign_a);
                num::SET_SIGN(q_b, sign_b);

                num::SET_SIGN(s, sign_a ^ num::GET_SIGN(s));
                num::SET_SIGN(t, sign_b ^ num::GET_SIGN(t));
            }

            void DENORMALIZE_DIVISOR(ap_int& b, int s) {
                unsigned int n = GET_OCC(b);
                UL* v = num::DATA(b);
                aux::SHIFT_RIGHT(v, n, s / 32, s % 32);
            }

            void MUL(ap_int& a, ap_int const& b) {
                if (IS_SINGLE(a)) {
                    if (IS_SINGLE(b)) {
                        ULL prod = ((ULL) a.value) * ((ULL) b.value);
                        num::SET_SIGN(a, num::GET_SIGN(a) ^ num::GET_SIGN(b));

                        UL overflow = prod >> 32;

                        a.value = prod;

                        if (overflow) aux::MAKE_MULTI_OVERFLOW(a, overflow);
                    } else {
                        UL val = a.value;
                        bool sign = num::GET_SIGN(a);
                        unsigned int n = GET_OCC(b);

                        num::SET_SIGN(a, sign ^ num::GET_SIGN(b));

                        num::aux::MAKE_MULTI_SIZE(a, n + 1);

                        ULL prod = 0;
                        UL carry = 0;
                        for (int i = 0; i < n; i++) {
                            prod = ((ULL) b.values[i]) * ((ULL) val) + carry;
                            a.values[i] = prod;
                            carry = prod >> 32;
                        }
                        if (carry) {
                            a.values[n] = carry;
                        }
                        unsigned int occ = n + 1;
                        num::aux::STRIP(ABS(a), &occ);
                        num::SET_OCC(a, occ);
                    }
                } else {
                    if (IS_SINGLE(b)) {
                        unsigned int n = num::GET_OCC(a);
                        num::aux::ENLARGE(a, n + 1);

                        num::SET_SIGN(a, num::GET_SIGN(a) ^ num::GET_SIGN(b));

                        ULL prod = 0;
                        UL carry = 0;
                        for (int i = 0; i < n; i++) {
                            prod = ((ULL) a.values[i]) * ((ULL) b.value) + carry;
                            a.values[i] = prod;
                            carry = prod >> 32;
                        }
                        if (carry) {
                            a.values[n] = carry;
                            num::SET_OCC(a, n + 1);
                        }
                    } else {
                        unsigned int n = num::GET_OCC(a);
                        unsigned int m = num::GET_OCC(b);

                        UL* curr = new UL[n + m] { };

                        ULL prod = 0ULL;
                        ULL sum = 0ULL;
                        ULL overflow;
                        ULL base = (1ULL << 32) - 1;
                        for (int i = 0; i < n; i++) {
                            overflow = 0ULL;
                            for (int j = 0; j < m || overflow; j++) {
                                prod = ((ULL) a.values[i]) * (j < m ? (ULL) b.values[j] : 0ULL) + overflow; // does never overflow itself
                                sum = ((ULL) curr[i + j]) + (prod & base);
                                curr[i + j] = sum & base;
                                overflow = (sum >> 32) + (prod >> 32);
                            }
                        }
                        unsigned int occ = n + m;
                        aux::STRIP(curr, &occ);
                        delete[] a.values;
                        a.values = curr;
                        num::SET_OCC(a, occ);
                        num::SET_SIZE(a, n + m);
                    }
                }
            }

            void CMUL(ap_int& a, long long x) {
                if (x < 0) {
                    num::SWITCH_SIGN(a);
                    x = -x;
                }
                if (x >> 32) throw std::runtime_error("SCALAR IS NOT SINGLE LIMB");

                ULL lambda = x;

                if (IS_SINGLE(a)) {
                    ULL prod = lambda * a.value;
                    a.value = prod;
                    if (prod >> 32) aux::MAKE_MULTI_OVERFLOW(a, prod >> 32);
                } else {
                    ULL prod;
                    UL carry = 0;
                    int n = GET_OCC(a);
                    for (int i = 0; i < n; i++) {
                        prod = lambda * ((ULL) a.values[i]) + carry;
                        a.values[i] = prod;
                        carry = prod >> 32;
                    }
                    if (carry) {
                        aux::ENLARGE(a, n + 1);
                        a.values[n] = carry;
                        num::SET_OCC(a, n + 1);
                    }
                }
            }

            void i_MUL(ap_int& res, ap_int const& a, ap_int const& b) {
                if (IS_SINGLE(a)) {
                    if (IS_SINGLE(b)) {
                        ULL prod = ((ULL) a.value) * ((ULL) b.value);
                        UL overflow = prod >> 32;
                        NEW(res, GET_POS(a), GET_SIGN(a) ^ GET_SIGN(b), prod);
                        if (overflow != 0) {
                            aux::MAKE_MULTI_OVERFLOW(res, overflow);
                        }
                    } else {
                        ULL lambda = a.value;
                        unsigned int n = GET_OCC(b);
                        NEW(res, GET_POS(a), GET_SIGN(a) ^ GET_SIGN(b), 0ULL);
                        aux::MAKE_MULTI_SIZE(res, n + 1);

                        ULL prod;
                        const UL* dat = b.values;
                        UL* tar = res.values;
                        UL overflow = 0ULL;
                        for (int i = 0; i < n; i++, dat++, tar++) {
                            prod = lambda * ((ULL) *dat) + ((ULL) overflow);
                            overflow = prod >> 32;
                            *tar = prod;
                        }
                        if (overflow != 0) {
                            *tar = overflow;
                        }
                        unsigned int occ = n + 1;
                        num::aux::STRIP(ABS(res), &occ);
                        num::SET_OCC(res, occ);
                        if (PRINT) {
                            std::cout << "MUL: " << GET_OCC(res) << " " << GET_SIZE(res) << " " << num::STRINGIFY(res) << std::endl;
                            std::cout << num::STRINGIFY(a) << " " << num::STRINGIFY(b) << std::endl;
                        }
                    }
                } else {
                    if (IS_SINGLE(b)) {
                        i_MUL(res, b, a);
                    } else {
                        aux::KARATSUBA(res, a, b);
                    }
                }
            }

            int COMPARE_ABS(ap_int const& n_a, ap_int const& n_b) {
                if (num::IS_SINGLE(n_a)) {
                    if (num::IS_SINGLE(n_b)) {
                        if (n_a.value != n_b.value) {
                            return n_a.value > n_b.value ? 1 : -1;
                        }
                        return 0;
                    } else return -1;
                } else {
                    if (num::IS_SINGLE(n_b)) return 1;
                    else {
                        unsigned int occ = GET_OCC(n_a);
                        if (occ != GET_OCC(n_b)) return ((int) occ) - ((int) GET_OCC(n_b));
                        const UL* a = ABS(n_a) + occ;
                        const UL* b = ABS(n_b) + occ;
                        return aux::COMPARE_RAW(a, b, occ);
                    }
                }
            }

            // returns true if a < b; i.e. if a and b are in the right order.
            int COMPARE(ap_int const& a, ap_int const& b) {
                if ((a.meta ^ b.meta) & num::SIGN) return GET_SIGN(a) ? 1 : -1;
                int cp = COMPARE_ABS(a, b);
                if (cp == 0) return 0;
                return cp * (GET_SIGN(a) ? 1 : -1);
            }


            bool comp::SIGNED_COMPARATOR::operator()(ap_int const& a, ap_int const& b) {
                return COMPARE(a, b) < 0;
            }

            bool comp::UNSIGNED_COMPARATOR::operator()(ap_int const &a, ap_int const &b) {
                return COMPARE_ABS(a, b) < 0;
            }
        }




        bool sp_context::i_mul(num::ap_int& curr, num::ap_int& a, num::ap_int& b) {
            ULL prod = ((ULL) a.value) * ((ULL) b.value);
            num::NEW(curr, num::GET_POS(a), num::GET_SIGN(a) ^ num::GET_SIGN(b), prod);
            UL overflow = prod >> 32;
            if (overflow) {
                num::aux::MAKE_MULTI_OVERFLOW(curr, overflow);
                return true;
            }
            return false;
        }

        bool sp_context::add(num::ap_int& a, num::ap_int& b) {
            if (num::GET_SIGN(a) == num::GET_SIGN(b)) {
                ULL sum = ((ULL) a.value) + ((ULL) b.value);
                UL overflow = sum >> 32;
                a.value = sum;
                if (overflow) {
                    num::aux::MAKE_MULTI_OVERFLOW(a, overflow);
                    return true;
                }
                return false;
            }
        }

        bool sp_context::mul(num::ap_int& a, num::ap_int& b) {
            ULL prod = ((ULL) a.value) * ((ULL) b.value);
            UL overflow = prod >> 32;
            a.value = prod;
            if (num::GET_SIGN(b)) num::SWITCH_SIGN(a);
            if (overflow) {
                num::aux::MAKE_MULTI_OVERFLOW(a, overflow);
                return true;
            }
            return false;
        }


        bool sp_context::gcd(num::ap_int& s, num::ap_int& t, num::ap_int& q_a, num::ap_int& q_b, num::ap_int& a, num::ap_int& b) {
            long long a_val = a.value;
            long long b_val = b.value;

            long long s_val = 1;
            long long t_val = 0;
            long long q_a_val = 1;
            long long q_b_val = 0;

            long long temp;
            while (b_val != 0) {
                long long q = a_val / b_val;
                
                temp = b_val;
                b_val = a_val - q * b_val;
                a_val = temp;

                temp = q_b_val;
                q_b_val = s_val - q * q_b_val;
                s_val = temp;

                temp = q_a_val;
                q_a_val = t_val - q * q_a_val;
                t_val = temp;
            }

            bool sign_a = num::GET_SIGN(a);
            bool sign_b = num::GET_SIGN(b);

            num::NEW(s, 0, (s_val < 0) ^ sign_a, s_val < 0 ? -s_val : s_val);
            num::NEW(t, 0, (t_val < 0) ^ sign_b, t_val < 0 ? -t_val : t_val);

            num::NEW(q_a, 0, sign_a, q_a_val < 0 ? -q_a_val : q_a_val);
            num::NEW(q_b, 0, sign_b, q_b_val < 0 ? -q_b_val : q_b_val);

            num::NEW(a, num::GET_POS(a), false, a_val);
            num::NEW(b, num::GET_POS(b), false, 0);

            // We know this cant overflow because |q_a| <= |a|, |q_b| <= |b|, |s| <= |b/2|, |t| <= |a/2| (if gcd != a,b) 
            return false;
        }


        arith_context* get_context() {
            return new arith_context(sp_context::instance());
        }

    }
}