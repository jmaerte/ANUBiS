//
// Created by jmaerte on 04.12.20.
//

#include "arithmetic/operator.hpp"
#include "arithmetic/aux.hpp"
#include "arithmetic/constants.hpp"
#include <algorithm>
#include <math.h>

namespace jmaerte {
    namespace arith {
        namespace vec {

            bool PACK = false;

            /**
            * ADD FUNCTION FOR A STACK ALLOCATED VECTOR
            */
            void ADD_STACK(s_ap_int_vec& a, int start_a, const num::ap_int& coeff, s_ap_int_vec& b, int start_b) {
                unsigned int occ = GET_OCC(a);
                unsigned int occ_b = GET_OCC(b);

                num::ap_int* it = a.values + start_a;

                unsigned int size_cpy = (occ - start_a);

                num::ap_int* a_cpy = (size_cpy != 0) ? new num::ap_int[size_cpy] : nullptr;

                if (size_cpy != 0) std::copy(it, a.values + occ, a_cpy);

                num::ap_int lambda;
                num::COPY(lambda, coeff);

                SET_OCC(a, start_a);

                vec::ENLARGE(a, start_a, occ + occ_b);

                occ = start_a;

                unsigned int pos_a, pos_b;
                int i = 0, j = start_b;
                ULL overflow;

                num::ap_int curr;

                for (; i < size_cpy && j < occ_b; occ++) {
                    //if (occ >= GET_SIZE(a)) ENLARGE(a, occ);
                    if ((pos_a = num::GET_POS(a_cpy[i])) < (pos_b = num::GET_POS(b.values[j]))) {
                        a.values[occ] = a_cpy[i];
                        i++;
                    } else if (pos_a > pos_b) {
                        num::i_MUL(curr, lambda, b.values[j]);
                        num::SET_POS(curr, pos_b);
                        a.values[occ] = curr;

                        j++;
                    } else {
                        num::i_MUL(curr, lambda, b.values[j]);
                        num::SET_POS(curr, pos_a);
                        num::ADD(curr, a_cpy[i]);

                        if (num::IS_ZERO(curr)) occ--;
                        else a.values[occ] = curr;

                        num::DELETE_DATA(a_cpy[i]);
                        i++;
                        j++;
                    }
                }

                while(i < size_cpy) {
                    if (occ >= GET_SIZE(a)) ENLARGE(a, occ);
                    a.values[occ] = a_cpy[i];
                    i++;
                    occ++;
                }

                while(j < occ_b) {
                    if (occ >= GET_SIZE(a)) ENLARGE(a, occ);
                    pos_b = num::GET_POS(b.values[j]);
                    num::i_MUL(curr, lambda, b.values[j]);
                    num::SET_POS(curr, pos_b);
                    a.values[occ] = curr;
                    j++;
                    occ++;
                }

                if (size_cpy != 0) delete[] a_cpy;
                num::DELETE_DATA(lambda);

                // update vector flags:
                SET_OCC(a, occ);
                if (PACK) {
                    PACK_VEC(a);
                }
            }

            void ADD(s_ap_int_vec& a, int start_a, const num::ap_int& coeff, s_ap_int_vec& b, int start_b, int copy_a) {
                unsigned int occ = GET_OCC(a);
                unsigned int occ_b = GET_OCC(b);

                num::ap_int* a_old = a.values;

                unsigned int size = vec::GET_SIZE(a);
                while (size < occ + occ_b) size = ceil(constants::PHI * size);
                num::ap_int* a_new = new num::ap_int[size];

                unsigned int size_cpy = (occ - start_a);

                num::ap_int* a_cpy = a_old + start_a;

                if (start_a != 0) std::copy(a_old, a_old + copy_a, a_new);

                num::ap_int lambda;
                num::COPY(lambda, coeff);

                occ = copy_a;

                unsigned int pos_a, pos_b;
                int i = 0, j = start_b;

                num::ap_int curr;

                for (; i < size_cpy && j < occ_b; occ++) {
                    //if (occ >= GET_SIZE(a)) ENLARGE(a, occ);
                    if ((pos_a = num::GET_POS(a_cpy[i])) < (pos_b = num::GET_POS(b.values[j]))) {
                        a_new[occ] = a_cpy[i];
                        i++;
                    } else if (pos_a > pos_b) {
                        num::i_MUL(curr, lambda, b.values[j]);
                        num::SET_POS(curr, pos_b);
                        a_new[occ] = curr;

                        j++;
                    } else {
                        num::i_MUL(curr, lambda, b.values[j]);
                        num::SET_POS(curr, pos_a);
                        num::ADD(curr, a_cpy[i]);

                        if (num::IS_ZERO(curr)) occ--;
                        else a_new[occ] = curr;

                        num::DELETE_DATA(a_cpy[i]);
                        i++;
                        j++;
                    }
                }

                while(i < size_cpy) {
                    //if (occ >= GET_SIZE(a)) ENLARGE(a, occ);
                    a_new[occ] = a_cpy[i];
                    i++;
                    occ++;
                }

                while(j < occ_b) {
                    //if (occ >= GET_SIZE(a)) ENLARGE(a, occ);
                    pos_b = num::GET_POS(b.values[j]);
                    num::i_MUL(curr, lambda, b.values[j]);
                    num::SET_POS(curr, pos_b);
                    a_new[occ] = curr;
                    j++;
                    occ++;
                }

                num::DELETE_DATA(lambda);

                delete[] a_old;
                a.values = a_new;

                // update vector flags:
                SET_SIZE(a, size);
                SET_OCC(a, occ);
            }

            void COMBINE_STACK(s_ap_int_vec& a, s_ap_int_vec& b, num::ap_int& lambda_a, num::ap_int& tau_a, num::ap_int& lambda_b, num::ap_int& tau_b, int start_a, int start_b) {
                unsigned int occ_a = GET_OCC(a);
                unsigned int occ_b = GET_OCC(b);

                num::ap_int* it_a = a.values + start_a;
                num::ap_int* it_b = b.values + start_b;

                unsigned int size_a = (occ_a - start_a);
                unsigned int size_b = (occ_b - start_b);

                num::ap_int* a_cpy = (size_a != 0) ? new num::ap_int[size_a] : nullptr;
                num::ap_int* b_cpy = (size_b != 0) ? new num::ap_int[size_b] : nullptr;

                if (size_a != 0) std::copy(it_a, a.values + occ_a, a_cpy);
                if (size_b != 0) std::copy(it_b, b.values + occ_b, b_cpy);
                

                SET_OCC(a, start_a);
                SET_OCC(b, start_b);

                vec::ENLARGE(a, start_a, occ_a + occ_b);
                vec::ENLARGE(b, start_b, occ_a + occ_b);

                occ_a = start_a;
                occ_b = start_b;

                unsigned int pos_a, pos_b;
                int i = 0, j = 0;

                num::ap_int curr;
                num::ap_int temp;

                for (; i < size_a && j < size_b;) {
                    //if (occ_a >= GET_SIZE(a)) ENLARGE(a, occ_a);
                    //if (occ_b >= GET_SIZE(b)) ENLARGE(b, occ_b);

                    if ((pos_a = num::GET_POS(a_cpy[i])) < (pos_b = num::GET_POS(b_cpy[j]))) {
                        
                        num::i_MUL(curr, a_cpy[i], lambda_a);
                        num::COPY(a.values[occ_a], curr);
                        num::SET_POS(a.values[occ_a++], pos_a);

                        num::DELETE_DATA(curr);

                        num::i_MUL(curr, a_cpy[i], lambda_b);
                        num::COPY(b.values[occ_b], curr);
                        num::SET_POS(b.values[occ_b++], pos_a);

                        num::DELETE_DATA(curr);

                        num::DELETE_DATA(a_cpy[i++]);
                    } else if (pos_a > pos_b) {

                        num::i_MUL(curr, b_cpy[j], tau_a);
                        num::COPY(a.values[occ_a], curr);
                        num::SET_POS(a.values[occ_a++], pos_b);

                        num::DELETE_DATA(curr);

                        num::i_MUL(curr, b_cpy[j], tau_b);
                        num::COPY(b.values[occ_b], curr);
                        num::SET_POS(b.values[occ_b++], pos_b);

                        num::DELETE_DATA(curr);

                        num::DELETE_DATA(b_cpy[j++]);
                    } else {
                        
                        num::ap_int curr2;
                        num::ap_int temp2;

                        num::i_MUL(curr, lambda_a, a_cpy[i]);
                        num::i_MUL(curr2, tau_a, b_cpy[j]);

                        num::i_MUL(temp, lambda_b, a_cpy[i]);
                        num::i_MUL(temp2, tau_b, b_cpy[j]);
                        
                        num::COPY(a.values[occ_a], curr);
                        num::ADD(a.values[occ_a], curr2);

                        if (num::IS_ZERO(a.values[occ_a])) num::DELETE_DATA(a.values[occ_a]);
                        else num::SET_POS(a.values[occ_a++], pos_a);

                        num::DELETE_DATA(curr);
                        num::DELETE_DATA(curr2);

                        num::COPY(b.values[occ_b], temp);
                        num::ADD(b.values[occ_b], temp2);

                        if (num::IS_ZERO(b.values[occ_b])) num::DELETE_DATA(b.values[occ_b]);
                        else num::SET_POS(b.values[occ_b++], pos_b);

                        num::DELETE_DATA(temp);
                        num::DELETE_DATA(temp2);
                        num::DELETE_DATA(a_cpy[i++]);
                        num::DELETE_DATA(b_cpy[j++]);
                    }
                }

                while(i < size_a) {
                    if (occ_a >= GET_SIZE(a)) ENLARGE(a, occ_a);
                    if (occ_b >= GET_SIZE(b)) ENLARGE(b, occ_b);

                    pos_a = num::GET_POS(a_cpy[i]);

                    num::i_MUL(a.values[occ_a], a_cpy[i], lambda_a);
                    num::SET_POS(a.values[occ_a++], pos_a);

                    num::i_MUL(b.values[occ_b], a_cpy[i], lambda_b);
                    num::SET_POS(b.values[occ_b++], pos_a);

                    num::DELETE_DATA(a_cpy[i++]);
                }

                while(j < size_b) {
                    if (occ_a >= GET_SIZE(a)) ENLARGE(a, occ_a);
                    if (occ_b >= GET_SIZE(b)) ENLARGE(b, occ_b);

                    pos_b = num::GET_POS(b_cpy[j]);

                    num::i_MUL(a.values[occ_a], b_cpy[j], tau_a);
                    num::SET_POS(a.values[occ_a++], pos_b);

                    num::i_MUL(b.values[occ_b], b_cpy[j], tau_b);
                    num::SET_POS(b.values[occ_b++], pos_b);

                    num::DELETE_DATA(b_cpy[j++]);
                }

                if (size_a != 0) delete[] a_cpy;
                if (size_b != 0) delete[] b_cpy;

                // update vector flags:
                SET_OCC(a, occ_a);
                SET_OCC(b, occ_b);

                if (PACK) {
                    PACK_VEC(b);
                }
            }

            void COMBINE(s_ap_int_vec& a, s_ap_int_vec& b, num::ap_int& lambda_a, num::ap_int& tau_a, num::ap_int& lambda_b, num::ap_int& tau_b, int start_a, int start_b, int copy_a, int copy_b) {
                unsigned int occ_a = GET_OCC(a);
                unsigned int occ_b = GET_OCC(b);

                unsigned int size_a = (occ_a - start_a);
                unsigned int size_b = (occ_b - start_b);

                num::ap_int* a_old = a.values;
                num::ap_int* b_old = b.values;

                num::ap_int* a_cpy = a_old + start_a;
                num::ap_int* b_cpy = b_old + start_b;

                unsigned int size = vec::GET_SIZE(a);
                while (size < occ_a + occ_b) size = ceil(constants::PHI * size);

                num::ap_int* a_new = new num::ap_int[size];
                num::ap_int* b_new = new num::ap_int[size];

                if (start_a != 0) std::copy(a_old, a_old + copy_a, a_new);
                if (start_b != 0) std::copy(b_old, b_old + copy_b, b_new);

                occ_a = copy_a;
                occ_b = copy_b;

                unsigned int pos_a, pos_b;
                int i = 0, j = 0;

                num::ap_int curr;
                num::ap_int temp;

                for (; i < size_a && j < size_b;) {
                    //if (occ_a >= GET_SIZE(a)) ENLARGE(a, occ_a);
                    //if (occ_b >= GET_SIZE(b)) ENLARGE(b, occ_b);

                    if ((pos_a = num::GET_POS(a_cpy[i])) < (pos_b = num::GET_POS(b_cpy[j]))) {
                        
                        num::i_MUL(curr, a_cpy[i], lambda_a);
                        num::COPY(a_new[occ_a], curr);
                        num::SET_POS(a_new[occ_a++], pos_a);

                        num::DELETE_DATA(curr);

                        num::i_MUL(curr, a_cpy[i], lambda_b);
                        num::COPY(b_new[occ_b], curr);
                        num::SET_POS(b_new[occ_b++], pos_a);

                        num::DELETE_DATA(curr);

                        num::DELETE_DATA(a_cpy[i++]);
                    } else if (pos_a > pos_b) {

                        num::i_MUL(curr, b_cpy[j], tau_a);
                        num::COPY(a_new[occ_a], curr);
                        num::SET_POS(a_new[occ_a++], pos_b);

                        num::DELETE_DATA(curr);

                        num::i_MUL(curr, b_cpy[j], tau_b);
                        num::COPY(b_new[occ_b], curr);
                        num::SET_POS(b_new[occ_b++], pos_b);

                        num::DELETE_DATA(curr);

                        num::DELETE_DATA(b_cpy[j++]);
                    } else {
                        
                        num::ap_int curr2;
                        num::ap_int temp2;

                        num::i_MUL(curr, lambda_a, a_cpy[i]);
                        num::i_MUL(curr2, tau_a, b_cpy[j]);

                        num::i_MUL(temp, lambda_b, a_cpy[i]);
                        num::i_MUL(temp2, tau_b, b_cpy[j]);
                        
                        num::COPY(a_new[occ_a], curr);
                        num::ADD(a_new[occ_a], curr2);

                        if (num::IS_ZERO(a_new[occ_a])) num::DELETE_DATA(a_new[occ_a]);
                        else num::SET_POS(a_new[occ_a++], pos_a);

                        num::DELETE_DATA(curr);
                        num::DELETE_DATA(curr2);

                        num::COPY(b_new[occ_b], temp);
                        num::ADD(b_new[occ_b], temp2);

                        if (num::IS_ZERO(b_new[occ_b])) num::DELETE_DATA(b_new[occ_b]);
                        else num::SET_POS(b_new[occ_b++], pos_b);

                        num::DELETE_DATA(temp);
                        num::DELETE_DATA(temp2);
                        num::DELETE_DATA(a_cpy[i++]);
                        num::DELETE_DATA(b_cpy[j++]);
                    }
                }

                while(i < size_a) {
                    //if (occ_a >= GET_SIZE(a)) ENLARGE(a, occ_a);
                    //if (occ_b >= GET_SIZE(b)) ENLARGE(b, occ_b);

                    pos_a = num::GET_POS(a_cpy[i]);

                    num::i_MUL(a_new[occ_a], a_cpy[i], lambda_a);
                    num::SET_POS(a_new[occ_a++], pos_a);

                    num::i_MUL(b_new[occ_b], a_cpy[i], lambda_b);
                    num::SET_POS(b_new[occ_b++], pos_a);

                    num::DELETE_DATA(a_cpy[i++]);
                }

                while(j < size_b) {
                    //if (occ_a >= GET_SIZE(a)) ENLARGE(a, occ_a);
                    //if (occ_b >= GET_SIZE(b)) ENLARGE(b, occ_b);

                    pos_b = num::GET_POS(b_cpy[j]);

                    num::i_MUL(a_new[occ_a], b_cpy[j], tau_a);
                    num::SET_POS(a_new[occ_a++], pos_b);

                    num::i_MUL(b_new[occ_b], b_cpy[j], tau_b);
                    num::SET_POS(b_new[occ_b++], pos_b);

                    num::DELETE_DATA(b_cpy[j++]);
                }

                delete[] a_old;
                delete[] b_old;
                a.values = a_new;
                b.values = b_new;

                // update vector flags:
                SET_SIZE(a, size);
                SET_SIZE(b, size);

                SET_OCC(a, occ_a);
                SET_OCC(b, occ_b);
            }

            void MOD(s_ap_int_vec& vec) {
                num::ap_int& modulus = vec.values[0];
                num::ap_int quot;
                unsigned int occ = 1;

                int shift = num::PREPARE_DIVISOR(modulus);

                for (int i = 1; i < vec::GET_OCC(vec); i++) {
                    num::DIV(quot, vec.values[i], modulus, shift);
                    num::DELETE_DATA(quot);

                    if (!num::IS_ZERO(vec.values[i])) {
                        vec.values[occ++] = vec.values[i];
                    } else num::DELETE_DATA(vec.values[i]);
                }

                num::DENORMALIZE_DIVISOR(modulus, shift);
                vec::SET_OCC(vec, occ);
            }

        }

        bool sp_context::vec_add(vec::s_ap_int_vec& a, int start_a, const num::ap_int& coeff, vec::s_ap_int_vec& b, int start_b, int copy_a) {
            bool overflown = false;
            unsigned int occ = vec::GET_OCC(a);
            unsigned int occ_b = vec::GET_OCC(b);

            num::ap_int* a_old = a.values;

            unsigned int size = vec::GET_SIZE(a);
            while (size < occ + occ_b) size = ceil(constants::PHI * size);
            num::ap_int* a_new = new num::ap_int[size];

            unsigned int size_cpy = (occ - start_a);

            num::ap_int* a_cpy = a_old + start_a;

            if (start_a != 0) std::copy(a_old, a_old + copy_a, a_new);

            num::ap_int* curr = a_new + copy_a;

            num::ap_int* a_curr = a_cpy, * b_curr = b.values + start_b;
            num::ap_int* a_end = a_old + occ, * b_end = b.values + occ_b;

            unsigned int pos_a, pos_b;
            /*int i = 0, j = start_b;*/

            long long sign_lambda = coeff.meta & num::SIGN;
            long long lambda = coeff.value;

            if (sign_lambda) {
                // subtract
                //if (lambda != 1ULL) {
                    if (a_curr != a_end && b_curr != b_end) {
                        do {
                            if ((pos_a = num::GET_POS(*a_curr)) == (pos_b = num::GET_POS(*b_curr))) {
                                if (num::SIGN & (b_curr->meta ^ a_curr->meta)) {
                                    ULL res = a_curr->value + lambda * b_curr->value;
                                    curr->meta = a_curr->meta;
                                    curr->value = res;
                                    UL overflow = res >> 32;
                                    if (overflow) {
                                        num::aux::MAKE_MULTI_OVERFLOW(*curr, overflow);
                                        overflown = true;
                                    }
                                    ++curr;
                                } else {
                                    long long res = ((long long) a_curr->value) - ((long long) lambda) * b_curr->value;
                                    if (res != 0) {
                                        long long mask = res >> constants::ABS_SHIFT;
                                        res = (mask + res) ^ mask;
                                        curr->value = res;
                                        UL overflow = res >> 32;
                                        curr->meta = a_curr->meta ^ (mask & num::SIGN);
                                        if (overflow) {
                                            num::aux::MAKE_MULTI_OVERFLOW(*curr, overflow);
                                            overflown = true;
                                        }
                                        ++curr;
                                    }
                                }

                                ++a_curr;
                                ++b_curr;

                                if (a_curr == a_end || b_curr == b_end) break;
                            } else if (pos_a < pos_b) {
                                *curr = *a_curr;
                                ++a_curr;
                                ++curr;

                                if (a_curr == a_end) break;
                            } else {
                                ULL res = lambda * b_curr->value;
                                curr->meta = b_curr->meta ^ num::SIGN;
                                curr->value = res;
                                UL overflow = res >> 32;
                                if (overflow) {
                                    num::aux::MAKE_MULTI_OVERFLOW(*curr, overflow);
                                    overflown = true;
                                }

                                ++b_curr;
                                ++curr;

                                if (b_curr == b_end) break;
                            }
                        } while (true);
                    }

                    while(a_curr != a_end) {
                        *curr = *a_curr;
                        ++a_curr;
                        ++curr;
                    }

                    while(b_curr != b_end) {
                        ULL res = lambda * b_curr->value;
                        UL overflow = res >> 32;
                        curr->meta = b_curr->meta ^ num::SIGN;
                        curr->value = res;
                        if (overflow) {
                            num::aux::MAKE_MULTI_OVERFLOW(*curr, overflow);
                            overflown = true;
                        }

                        ++b_curr;
                        ++curr;
                    }
                /*} else {
                    if (a_curr != a_end && b_curr != b_end) {
                        do {
                            if ((pos_a = num::GET_POS(*a_curr)) == (pos_b = num::GET_POS(*b_curr))) {
                                if (num::SIGN & (b_curr->meta ^ a_curr->meta)) {
                                    ULL res = ((ULL) a_curr->value) + b_curr->value;
                                    curr->value = res;
                                    curr->meta = a_curr->meta;
                                    UL overflow = res >> 32;
                                    if (overflow) {
                                        num::aux::MAKE_MULTI_OVERFLOW(*curr, overflow);
                                        overflown = true;
                                    }
                                    ++curr;
                                } else {
                                    long long res = ((long long) a_curr->value) - ((long long) b_curr->value);
                                    if (res != 0) {
                                        long long mask = res >> constants::ABS_SHIFT;
                                        res = (mask + res) ^ mask;

                                        curr->value = res;
                                        UL overflow = res >> 32;
                                        curr->meta = a_curr->meta ^ (mask & num::SIGN);
                                        if (overflow) {
                                            num::aux::MAKE_MULTI_OVERFLOW(*curr, overflow);
                                            overflown = true;
                                        }
                                        ++curr;
                                    }
                                }

                                ++a_curr;
                                ++b_curr;

                                if (a_curr == a_end || b_curr == b_end) break;
                            } else if (pos_a < pos_b) {
                                *curr = *a_curr;
                                ++a_curr;
                                ++curr;

                                if (a_curr == a_end) break;
                            } else {
                                *curr = *b_curr;
                                curr->meta ^= num::SIGN;
                                //num::SWITCH_SIGN(*curr);
                                ++b_curr;
                                ++curr;

                                if (b_curr == b_end) break;
                            }
                        } while (true);
                    }

                    while(a_curr != a_end) {
                        *curr = *a_curr;
                        ++a_curr;
                        ++curr;
                    }

                    while(b_curr != b_end) {
                        curr->value = b_curr->value;
                        curr->meta = b_curr->meta ^ num::SIGN;
                        ++b_curr;
                        ++curr;
                    }
                }*/
            } else {
                // add
                /*if (lambda == 1ULL) {
                    if (a_curr != a_end && b_curr != b_end) {
                        do {
                            //if (occ >= GET_SIZE(a)) ENLARGE(a, occ);
                            if ((pos_a = num::GET_POS(*a_curr)) == (pos_b = num::GET_POS(*b_curr))) {
                                if (num::SIGN & ~(b_curr->meta ^ a_curr->meta)) {
                                    ULL res = ((ULL) a_curr->value) + b_curr->value;
                                    curr->meta = a_curr->meta;
                                    curr->value = res;
                                    UL overflow = res >> 32;
                                    if (overflow) {
                                        num::aux::MAKE_MULTI_OVERFLOW(*curr, overflow);
                                        overflown = true;
                                    }
                                    ++curr;
                                } else {
                                    long long res = ((long long) a_curr->value) - ((long long) b_curr->value);
                                    if (res != 0) {
                                        long long mask = res >> constants::ABS_SHIFT;
                                        res = (mask + res) ^ mask;

                                        curr->value = res;
                                        UL overflow = res >> 32;

                                        curr->meta = a_curr->meta ^ (mask & num::SIGN);
                                        if (overflow) {
                                            num::aux::MAKE_MULTI_OVERFLOW(*curr, overflow);
                                            overflown = true;
                                        }
                                        ++curr;
                                    }
                                }

                                ++a_curr;
                                ++b_curr;

                                if (a_curr == a_end || b_curr == b_end) break;
                            } else if (pos_a < pos_b) {
                                *curr = *a_curr;
                                ++a_curr;
                                ++curr;

                                if (a_curr == a_end) break;
                            } else {
                                *curr = *b_curr;
                                ++b_curr;
                                ++curr;

                                if (b_curr == b_end) break;
                            }
                        } while (true);
                    }

                    while(a_curr != a_end) {
                        //if (occ >= GET_SIZE(a)) ENLARGE(a, occ);
                        *curr = *a_curr;
                        ++a_curr;
                        ++curr;
                    }

                    while(b_curr != b_end) {
                        //if (occ >= GET_SIZE(a)) ENLARGE(a, occ);
                        *curr = *b_curr;
                        
                        ++b_curr;
                        ++curr;
                    }
                } else {*/
                    if (a_curr != a_end && b_curr != b_end) {
                        do {
                            if ((pos_a = num::GET_POS(*a_curr)) == (pos_b = num::GET_POS(*b_curr))) {
                                if (num::SIGN & ~(b_curr->meta ^ a_curr->meta)) {
                                    ULL res = a_curr->value + lambda * b_curr->value;
                                    curr->value = res;
                                    curr->meta = a_curr->meta;
                                    UL overflow = res >> 32;
                                    if (overflow) {
                                        num::aux::MAKE_MULTI_OVERFLOW(*curr, overflow);
                                        overflown = true;
                                    }
                                    ++curr;
                                } else {
                                    long long res = ((long long) a_curr->value) - ((long long) lambda) * b_curr->value;
                                    if (res != 0) {
                                        long long mask = res >> constants::ABS_SHIFT;
                                        res = (mask + res) ^ mask;

                                        curr->value = res;
                                        UL overflow = res >> 32;

                                        curr->meta = a_curr->meta ^ (mask & num::SIGN);
                                        if (overflow) {
                                            num::aux::MAKE_MULTI_OVERFLOW(*curr, overflow);
                                            overflown = true;
                                        }
                                        ++curr;
                                    }
                                }

                                ++a_curr;
                                ++b_curr;

                                if (a_curr == a_end || b_curr == b_end) break;
                            } else if (pos_a < pos_b) {
                                *curr = *a_curr;
                                ++a_curr;
                                ++curr;

                                if (a_curr == a_end) break; 
                            } else {
                                ULL res = lambda * b_curr->value;
                                curr->meta = b_curr->meta;
                                curr->value = res;
                                UL overflow = res >> 32;
                                if (overflow) {
                                    num::aux::MAKE_MULTI_OVERFLOW(*curr, overflow);
                                    overflown = true;
                                }

                                ++b_curr;
                                ++curr;
                                
                                if (b_curr == b_end) break;
                            }
                        } while (true);
                    }

                    while(a_curr != a_end) {
                        //if (occ >= GET_SIZE(a)) ENLARGE(a, occ);
                        *curr = *a_curr;
                        ++a_curr;
                        ++curr;
                    }

                    while(b_curr != b_end) {
                        ULL res = lambda * b_curr->value;
                        UL overflow = res >> 32;
                        curr->value = res;
                        curr->meta = b_curr->meta;
                        if (overflow) {
                            num::aux::MAKE_MULTI_OVERFLOW(*curr, overflow);
                            overflown = true;
                        }
                        
                        ++b_curr;
                        ++curr;
                    }
                //}
            }

            delete[] a_old;
            a.values = a_new;

            // update vector flags:
            vec::SET_SIZE(a, size);
            vec::SET_OCC(a, curr - a_new);

            return overflown;
        }

        bool sp_context::vec_combine(vec::s_ap_int_vec& a, vec::s_ap_int_vec& b, num::ap_int& lambda_a, num::ap_int& tau_a, num::ap_int& lambda_b, num::ap_int& tau_b, int start_a, int start_b, int copy_a, int copy_b) {
            bool overflown = false;

            unsigned int occ_a = vec::GET_OCC(a);
            unsigned int occ_b = vec::GET_OCC(b);

            unsigned int size_a = (occ_a - start_a);
            unsigned int size_b = (occ_b - start_b);

            num::ap_int* a_old = a.values;
            num::ap_int* b_old = b.values;

            num::ap_int* a_cpy = a_old + start_a;
            num::ap_int* b_cpy = b_old + start_b;

            unsigned int size = vec::GET_SIZE(a);
            while (size < occ_a + occ_b) size = ceil(constants::PHI * size);

            num::ap_int* a_new = new num::ap_int[size];
            num::ap_int* b_new = new num::ap_int[size];

            if (start_a != 0) std::copy(a_old, a_old + copy_a, a_new);
            if (start_b != 0) std::copy(b_old, b_old + copy_b, b_new);

            occ_a = copy_a;
            occ_b = copy_b;

            unsigned int pos_a, pos_b;
            int i = 0, j = 0;

            long long res;
            ULL res_u;
            UL overflow;

            ULL l_a = lambda_a.value;
            ULL l_b = lambda_b.value;
            ULL t_a = tau_a.value;
            ULL t_b = tau_b.value;

            bool sign_l_a = num::GET_SIGN(lambda_a);
            bool sign_l_b = num::GET_SIGN(lambda_b);
            bool sign_t_a = num::GET_SIGN(tau_a);
            bool sign_t_b = num::GET_SIGN(tau_b);

            for (; i < size_a && j < size_b;) {
                //if (occ_a >= GET_SIZE(a)) ENLARGE(a, occ_a);
                //if (occ_b >= GET_SIZE(b)) ENLARGE(b, occ_b);

                if ((pos_a = num::GET_POS(a_cpy[i])) < (pos_b = num::GET_POS(b_cpy[j]))) {
                    res_u = l_a * a_cpy[i].value;
                    overflow = res_u >> 32;
                    num::NEW(a_new[occ_a], pos_a, sign_l_a ^ num::GET_SIGN(a_cpy[i]), res_u);
                    if (overflow) {
                        num::aux::MAKE_MULTI_OVERFLOW(a_new[occ_a], overflow);
                        overflown = true;
                    }

                    res_u = l_b * a_cpy[i].value;
                    overflow = res_u >> 32;
                    num::NEW(b_new[occ_b], pos_a, sign_l_b ^ num::GET_SIGN(a_cpy[i]), res_u);
                    if (overflow) {
                        num::aux::MAKE_MULTI_OVERFLOW(b_new[occ_b], overflow);
                        overflown = true;
                    }

                    occ_a++;
                    occ_b++;
                    i++;
                } else if (pos_a > pos_b) {
                    res_u = t_a * b_cpy[j].value;
                    overflow = res_u >> 32;
                    num::NEW(a_new[occ_a], pos_b, sign_t_a ^ num::GET_SIGN(b_cpy[j]), res_u);
                    if (overflow) {
                        num::aux::MAKE_MULTI_OVERFLOW(a_new[occ_a], overflow);
                        overflown = true;
                    }

                    res_u = t_b * b_cpy[j].value;
                    overflow = res_u >> 32;
                    num::NEW(b_new[occ_b], pos_b, sign_t_b ^ num::GET_SIGN(b_cpy[j]), res_u);
                    if (overflow) {
                        num::aux::MAKE_MULTI_OVERFLOW(b_new[occ_b], overflow);
                        overflown = true;
                    }

                    occ_a++;
                    occ_b++;
                    j++;
                } else {
                    bool sign;
                    if ((sign = sign_l_a ^ num::GET_SIGN(a_cpy[i])) == (sign_t_a ^ num::GET_SIGN(b_cpy[j]))) {
                        res_u = l_a * a_cpy[i].value + t_a * b_cpy[j].value;
                        overflow = res_u >> 32;
                        num::NEW(a_new[occ_a], pos_a, sign, res_u);
                        if (overflow) {
                            num::aux::MAKE_MULTI_OVERFLOW(a_new[occ_a], overflow);
                            overflown = true;
                        }
                        occ_a++;
                    } else {
                        res = ((long long) l_a) * a_cpy[i].value - ((long long) t_a) * b_cpy[j].value;
                        if (res != 0) {
                            num::NEW(a_new[occ_a], pos_a, sign ^ (res < 0), res < 0 ? -res : res);
                            overflow = (res < 0 ? -res : res) >> 32;
                            if (overflow) {
                                num::aux::MAKE_MULTI_OVERFLOW(a_new[occ_a], overflow);
                                overflown = true;
                            }
                            occ_a++;
                        }
                    }

                    if ((sign = sign_l_b ^ num::GET_SIGN(a_cpy[i])) == (sign_t_b ^ num::GET_SIGN(b_cpy[j]))) {
                        res_u = l_b * a_cpy[i].value + t_b * b_cpy[j].value;
                        overflow = res_u >> 32;
                        num::NEW(b_new[occ_b], pos_a, sign, res_u);
                        if (overflow) {
                            num::aux::MAKE_MULTI_OVERFLOW(b_new[occ_b], overflow);
                            overflown = true;
                        }
                        occ_b++;
                    } else {
                        res = ((long long) l_b) * a_cpy[i].value - ((long long) t_b) * b_cpy[j].value;
                        if (res != 0) {
                            num::NEW(b_new[occ_b], pos_a, sign ^ (res < 0), res < 0 ? -res : res);
                            overflow = (res < 0 ? -res : res) >> 32;
                            if (overflow) {
                                num::aux::MAKE_MULTI_OVERFLOW(b_new[occ_b], overflow);
                                overflown = true;
                            }
                            occ_b++;
                        }
                    }

                    i++;
                    j++;
                }
            }

            while(i < size_a) {
                //if (occ_a >= GET_SIZE(a)) ENLARGE(a, occ_a);
                //if (occ_b >= GET_SIZE(b)) ENLARGE(b, occ_b);

                pos_a = num::GET_POS(a_cpy[i]);

                res_u = l_a * a_cpy[i].value;
                num::NEW(a_new[occ_a], pos_a, sign_l_a ^ num::GET_SIGN(a_cpy[i]), res_u);
                overflow = res_u >> 32;
                if (overflow) {
                    num::aux::MAKE_MULTI_OVERFLOW(a_new[occ_a], overflow);
                    overflown = true;
                }

                res_u = l_b * a_cpy[i].value;
                num::NEW(b_new[occ_b], pos_a, sign_l_b ^ num::GET_SIGN(a_cpy[i]), res_u);
                overflow = res_u >> 32;
                if (overflow) {
                    num::aux::MAKE_MULTI_OVERFLOW(b_new[occ_b], overflow);
                    overflown = true;
                }

                occ_a++;
                occ_b++;
                i++;
            }

            while(j < size_b) {
                //if (occ_a >= GET_SIZE(a)) ENLARGE(a, occ_a);
                //if (occ_b >= GET_SIZE(b)) ENLARGE(b, occ_b);

                pos_b = num::GET_POS(b_cpy[j]);

                res_u = t_a * b_cpy[j].value;
                num::NEW(a_new[occ_a], pos_b, sign_t_a ^ num::GET_SIGN(b_cpy[j]), res_u);
                overflow = res_u >> 32;
                if (overflow) {
                    num::aux::MAKE_MULTI_OVERFLOW(a_new[occ_a], overflow);
                    overflown = true;
                }

                res_u = t_b * b_cpy[j].value;
                num::NEW(b_new[occ_b], pos_b, sign_t_b ^ num::GET_SIGN(b_cpy[j]), res_u);
                overflow = res_u >> 32;
                if (overflow) {
                    num::aux::MAKE_MULTI_OVERFLOW(b_new[occ_b], overflow);
                    overflown = true;
                }

                occ_a++;
                occ_b++;
                j++;
            }

            /*if (overflown) {
                std::cout << "COMBINE OVERFLOWED:" << std::endl;
                std::cout << "a: ";
                for (int i = 0; i < size_a; i++) {
                    std::cout << num::GET_POS(a_cpy[i]) << " -> " << num::STRINGIFY(a_cpy[i]) << " ; ";
                }
                std::cout << std::endl << "b: ";
                for (int j = 0; j < size_b; j++) {
                    std::cout << num::GET_POS(b_cpy[j]) << " -> " << num::STRINGIFY(b_cpy[j]) << " ; ";
                }
                std::cout << std::endl << "Coefficients: " << num::STRINGIFY(lambda_a) << " " << num::STRINGIFY(tau_a) << " " << num::STRINGIFY(lambda_b) << " " << num::STRINGIFY(tau_b) << std::endl;
                std::cout << "Results:" << std::endl;
                std::cout << "a: ";
                for (int i = 0; i < occ_a; i++) {
                    std::cout << num::GET_POS(a_new[i]) << " -> " << num::STRINGIFY(a_new[i]) << " ; ";
                }
                std::cout << std::endl << "b: ";
                for (int j = 0; j < occ_b; j++) {
                    std::cout << num::GET_POS(b_new[j]) << " -> " << num::STRINGIFY(b_new[j]) << " ; ";
                }
                std::cout << std::endl;
                throw std::runtime_error("");
            }*/

            delete[] a_old;
            delete[] b_old;
            a.values = a_new;
            b.values = b_new;

            // update vector flags:
            vec::SET_SIZE(a, size);
            vec::SET_SIZE(b, size);

            vec::SET_OCC(a, occ_a);
            vec::SET_OCC(b, occ_b);

            return overflown;
        }
    }
}