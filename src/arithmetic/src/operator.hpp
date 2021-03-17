//
// Created by jmaerte on 04.12.20.
//

#ifndef ANUBIS_SUPERBUILD_OPERATOR_HPP
#define ANUBIS_SUPERBUILD_OPERATOR_HPP

#include "arithmetic/operator.hpp"
#include "arithmetic/constants.hpp"
#include "arithmetic/aux.hpp"
#include <output/logger.hpp>
#include <algorithm>
#include <math.h>

namespace jmaerte {
    namespace arith {
        namespace num {


            /*
             * OPERATORS
             */

            static inline std::string STRINGIFY(ap_int const& n) {
                if (IS_SINGLE(n)) return (GET_SIGN(n) ? "-" : "") + std::to_string(n.value);
                std::string res = std::string("(") + (GET_SIGN(n) ? "-" : "");
                res += aux::STRINGIFY(n.values, num::GET_OCC(n));
                res += ")_2";
                return res;
            }

            static inline unsigned int GET_POS(ap_int const& n) {
                return IS_SINGLE(n) ? (n.meta >> 2) : ((n.meta & num::POS) >> num::POS_SHIFT);
            }

            static inline unsigned int GET_OCC(ap_int const& n) {
                return IS_SINGLE(n) ? (n.value == 0 ? 0ULL : 1ULL) : ((n.meta & num::OCC) >> num::OCC_SHIFT);
            }

            static inline unsigned int GET_SIZE(ap_int const& n) {
                return IS_SINGLE(n) ? 1 : ((n.meta & num::SIZE) >> num::SIZE_SHIFT);
            }

            static inline bool IS_SINGLE(ap_int const& n) {
                return (n.meta & num::SINGLE) >> num::SINGLE_SHIFT;
            }

            static inline bool GET_SIGN(ap_int const& n) {
                return (n.meta & num::SIGN) >> num::SIGN_SHIFT;
            }

            static inline bool IS_ZERO(ap_int const& n) {
                if (IS_SINGLE(n)) return n.value == 0;
                //return GET_OCC(n) == 0;
                for (int i = GET_OCC(n) - 1; i >= 0; i--) {
                    if (n.values[i] != 0) return false;
                }
                return true;
            }


            static inline const UL* ABS(ap_int const& n) {
                return IS_SINGLE(n) ? &(n.value) : n.values;
            }
            static inline UL* DATA(ap_int& n) {
                return IS_SINGLE(n) ? &(n.value) : n.values;
            }

            // MUTATORS
            static void SET_SIZE(ap_int& n, unsigned int size) {
                if (IS_SINGLE(n)) throw std::runtime_error("Tried to set size of single number");
                n.meta = (n.meta & ~num::SIZE) | (((ULL) size) << num::SIZE_SHIFT);
            }

            static void SET_POS(ap_int& n, unsigned int pos) {
                if (IS_SINGLE(n)) n.meta = (n.meta & 3ULL) | (((ULL) pos) << 2);
                else n.meta = (n.meta & ~num::POS) | (((ULL) pos) << num::POS_SHIFT);
            }

            static inline void SET_OCC(ap_int& n, unsigned int occ) {
                if (IS_SINGLE(n)) throw std::runtime_error("Tried to set occ of single number");

                n.meta = (n.meta & ~num::OCC) | (((ULL) occ) << num::OCC_SHIFT);

                if (occ <= 1) aux::MAKE_SINGLE(n);
            }

            static inline void SET_SIGN(ap_int& n, bool sign) {
                n.meta = (n.meta & ~num::SIGN) | ((sign ? 1ULL : 0ULL) << num::SIGN_SHIFT);
            }

            static inline void SWITCH_SIGN(ap_int& n) {
                n.meta ^= num::SIGN;
            }

            static inline void SWAP_VALUES(ap_int& a, ap_int& b) {
                ap_int temp = b;
                b = a;
                a = temp;
            }

            // ALLOCATORS
            static inline void COPY(ap_int& m, ap_int const& n) {
                m.meta = n.meta;
                if (IS_SINGLE(n)) m.value = n.value;
                else {

                    aux::allocation_mutex.lock();
                    m.values = new UL[GET_SIZE(n)] { };
                    aux::allocation_mutex.unlock();

                    std::copy(n.values, n.values + GET_OCC(n), m.values);
                }
            }

            static inline void DELETE_DATA(ap_int& n) {
                if (!IS_SINGLE(n)) {

                    aux::allocation_mutex.lock();
                    delete[] n.values;
                    aux::allocation_mutex.unlock();
                }
            }

            static inline void NEW(ap_int& n, int pos, bool sign, int value) {
                n.meta = 1ULL | ((sign ? 1ULL : 0ULL) << num::SIGN_SHIFT) | (((ULL) pos) << 2);
                n.value = value;
            }

            static inline void NEW_MULTI(ap_int& n, int pos, bool sign, int occ, int size, UL* value) {
                n.meta = ((sign ? 1ULL : 0ULL) << num::SIGN_SHIFT) | (((ULL) pos) << num::POS_SHIFT) | (((ULL) size) << num::SIZE_SHIFT) | (((ULL) occ) << num::OCC_SHIFT);
                n.values = value;
                if (occ <= 1) aux::MAKE_SINGLE(n);
            }

            /*
             * ARITHMETIC
             */

        }

        namespace vec {

            /*
             * OPERATORS
             */

            static inline bool IS_ZERO(s_ap_int_vec const& vec) {
                return GET_OCC(vec) == 0;
            }

            static inline unsigned int GET_OCC(s_ap_int_vec const& vec) {
                return (vec.meta & vec::OCC) >> vec::OCC_SHIFT;
            }

            static inline unsigned int GET_SIZE(s_ap_int_vec const& vec) {
                return (vec.meta & vec::SIZE) >> vec::SIZE_SHIFT;
            }

            static inline unsigned int GET_FACTORY_ID(s_ap_int_vec const& vec) {
                return (vec.meta & vec::FACTORY_ID) >> vec::FACTORY_ID_SHIFT;
            }

            static inline unsigned int FIND_POS(s_ap_int_vec const& v, unsigned int pos) {
                int occ = GET_OCC(v);
                if (occ == 0 || num::GET_POS(AT(v, occ - 1)) < pos) return occ;
                if (num::GET_POS(AT(v, 0)) > pos) return 0;
                int min = 0;
                int max = occ;
                int compare = 0;
                while (min < max) {
                    int mid = (min + max) / 2;
                    if (num::GET_POS(AT(v, mid)) < pos) min = mid + 1;
                    else if (num::GET_POS(AT(v, mid)) > pos) max = mid;
                    else return mid;
                }
                return min;
            }

            // MUTATORS
            static inline void SET_SIZE(s_ap_int_vec& vec, unsigned int size) {
                vec.meta = (vec.meta & ~vec::SIZE) | (((ULL) size) << vec::SIZE_SHIFT);
            }

            static inline void SET_OCC(s_ap_int_vec& vec, unsigned int occ) {
                vec.meta = (vec.meta & ~vec::OCC) | (((ULL) occ) << vec::OCC_SHIFT);
            }

            static inline void SWITCH_SIGNS(s_ap_int_vec& vec) {
                for (int i = 0; i < vec::GET_OCC(vec); i++) num::SWITCH_SIGN(vec.values[i]);
            }

            static inline void SET_FACTORY_ID(s_ap_int_vec& vec, unsigned int factory_id) {
                vec.meta = (vec.meta & ~vec::FACTORY_ID) | (((ULL) factory_id) << vec::FACTORY_ID_SHIFT);
            }

            static inline void DELETE_POS(s_ap_int_vec& vec, unsigned int pos) {
                num::DELETE_DATA(vec.values[pos]);
                std::copy(vec.values + pos + 1, vec.values + vec::GET_OCC(vec), vec.values + pos);
                vec::SET_OCC(vec, vec::GET_OCC(vec) - 1);
            }

            static inline void SWAP_VALUES(s_ap_int_vec& vec, int pos1, int pos2) {
                std::cout << "UNSWAPPED VALUES: " << num::GET_POS(vec.values[pos1]) << " " << num::STRINGIFY(vec.values[pos1]) << " " << num::GET_POS(vec.values[pos2]) << " " << num::STRINGIFY(vec.values[pos2]) << std::endl;

                int p1 = num::GET_POS(vec.values[pos1]);
                int p2 = num::GET_POS(vec.values[pos2]);

                num::ap_int temp = vec.values[pos1];
                vec.values[pos1] = vec.values[pos2];
                vec.values[pos2] = temp;

                num::SET_POS(vec.values[pos1], p1);
                num::SET_POS(vec.values[pos2], p2);

                std::cout << "SWAPPED VALUES: " << num::GET_POS(vec.values[pos1]) << " " << num::STRINGIFY(vec.values[pos1]) << " " << num::GET_POS(vec.values[pos2]) << " " << num::STRINGIFY(vec.values[pos2]) << std::endl;
            }

            // GETTERS WITH MUTABLE RETURN VALUES
            static inline const num::ap_int& AT(const s_ap_int_vec& vec, int i) {
                if (i < 0 || i >= GET_OCC(vec)) throw std::runtime_error("Out of bounds access of vector elements!");
                return vec.values[i];
            }

            static inline num::ap_int& GET(s_ap_int_vec& vec, int i) {
                if (i < 0 || i >= GET_OCC(vec)) throw std::runtime_error("Out of bounds access of vector elements!");
                return vec.values[i];
            }

            // ALLOCATORS
            static inline void DELETE(s_ap_int_vec& vec) {
                factory::dict.get_factory(GET_FACTORY_ID(vec))->deallocate_vec(vec);
            }

            static inline s_ap_int_vec COPY(int factory_id, s_ap_int_vec& vec) {
                return factory::dict.get_factory(factory_id)->copy(vec);
            }

            static inline s_ap_int_vec NEW(unsigned int factory_id, std::vector<std::pair<UL, std::pair<bool, UL>>> vector) {
                s_ap_int_vec v = factory::dict.get_factory(factory_id)->allocate_vec(vector.size());
                v.meta = (((ULL) vector.size()) << vec::SIZE_SHIFT) | (((ULL) vector.size()) << vec::OCC_SHIFT) | (factory_id << vec::FACTORY_ID_SHIFT);
                int i = 0;
                for (auto it = vector.begin(); it != vector.end(); ++it) {
                    num::NEW(v.values[i], it->first, it->second.first, it->second.second);
                    i++;
                }
                return v;
            }

            static inline void ENLARGE(s_ap_int_vec& vec) {
                factory::dict.get_factory(GET_FACTORY_ID(vec))->enlarge(vec, ceil(constants::PHI * GET_SIZE(vec)), GET_OCC(vec));
            }

            static inline void ENLARGE(s_ap_int_vec& vec, unsigned int occ) {
                factory::dict.get_factory(GET_FACTORY_ID(vec))->enlarge(vec, ceil(constants::PHI * GET_SIZE(vec)), occ);
            }

            static inline void ENLARGE(s_ap_int_vec& vec, unsigned int occ, unsigned int size) {
                factory::dict.get_factory(GET_FACTORY_ID(vec))->enlarge(vec, size, occ);
            }

            static inline void PACK_VEC(s_ap_int_vec& vec) {
                std::cout << "saved " << (4 * (vec::GET_SIZE(vec) - vec::GET_OCC(vec))) << " bytes of space" << std::endl;
                factory::dict.get_factory(GET_FACTORY_ID(vec))->enlarge(vec, GET_OCC(vec), GET_OCC(vec));
            }


            /*
             * ARITHMETIC
             */

            static inline void REDUCE(s_ap_int_vec& v, s_ap_int_vec& trivial, int k) {
                num::ap_int lambda = AT(v, k);
                ADD(v, k + 1, lambda, trivial, 1, k);
                num::DELETE_DATA(lambda);
            }

            namespace factory {

                template<typename factory_type>
                static inline unsigned int REGISTER() {
                    output::LOGGER.log(output::MEM_CHANNEL, "Registering factory of type " + std::string(typeid(factory_type).name()) + ".");
                    auto fact = new factory_type();
                    return fact->get_id();
                }

                static inline void RELEASE(unsigned int id) {
                    jmaerte::output::LOGGER.release_channel(factory::dict.get_factory(id)->get_channel_id());
                    factory::dict.release_factory(id);
                    output::LOGGER.log(output::MEM_CHANNEL, "Released factory " + std::to_string(id) + " successfully!");
                }
            }

        }
    }
}

#endif //ANUBIS_SUPERBUILD_OPERATOR_HPP
