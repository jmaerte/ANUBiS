//
// Created by jmaerte on 16.03.20.
//

#ifndef ANUBIS_ARITHMETIC_SUPERBUILD_OPERATOR_PUBLIC_HPP
#define ANUBIS_ARITHMETIC_SUPERBUILD_OPERATOR_PUBLIC_HPP

#include "typedef.hpp"
#include "factory/factory.hpp"

#include <vector>
#include <tuple>
#include <iostream>
#include <ARITHMETIC_EXPORT.h>
#include <string>
#include <chrono>

namespace jmaerte {
    namespace arith {
        namespace num {


            extern bool PRINT;

            void PRINT_COUNTERS();


            /*
             * OPERATORS
             */

            // GETTERS
            static std::string          STRINGIFY(ap_int const& n);
            static unsigned int         GET_POS(ap_int const& n);
            static unsigned int         GET_OCC(ap_int const& n);
            static unsigned int         GET_SIZE(ap_int const& n);
            static bool                 IS_SINGLE(ap_int const& n);
            static bool                 GET_SIGN(ap_int const& n);
            static bool                 IS_ZERO(ap_int const& n);

            static const UL*            ABS(ap_int const& n);
            static UL*                  DATA(ap_int& n);

            // MUTATORS
            static void                 SET_SIZE(ap_int& n, unsigned int size);
            static void                 SET_POS(ap_int& n, unsigned int pos);
            static void                 SET_OCC(ap_int& n, unsigned int occ);
            static void                 SET_SIGN(ap_int& n, bool sign);
            static void                 SWITCH_SIGN(ap_int& n);
            static void                 SWAP_VALUES(ap_int& a, ap_int& b);


            // ALLOCATORS
            static void COPY(ap_int& m, ap_int const& n);
            static void DELETE_DATA(ap_int& n);
            static void NEW(ap_int& n, int pos, bool sign, int value);
            static void NEW_MULTI(ap_int& n, int pos, bool sign, int occ, int size, UL* value);

            /*
             * ARITHMETIC
             */

            ARITHMETIC_EXPORT void      ADD(ap_int& a, ap_int const& b);
            ARITHMETIC_EXPORT int       PREPARE_DIVISOR(ap_int& b);
            ARITHMETIC_EXPORT void      DIV(ap_int& quot, ap_int& a, ap_int& b, int s);
            ARITHMETIC_EXPORT void      GCD(ap_int& s, ap_int& t, ap_int& x, ap_int& y, ap_int& a, ap_int& b);
            ARITHMETIC_EXPORT void      DENORMALIZE_DIVISOR(ap_int& b, int s);
            ARITHMETIC_EXPORT void      MUL(ap_int& a, ap_int const& b);
            ARITHMETIC_EXPORT void      CMUL(ap_int& a, long long x);
            ARITHMETIC_EXPORT void      i_MUL(ap_int& res, ap_int const& a, ap_int const& b);
            ARITHMETIC_EXPORT void      i_LC(ap_int& res, unsigned int lambda, ap_int const& a, unsigned int mu, ap_int const& b);

            ARITHMETIC_EXPORT int       COMPARE_ABS(ap_int const& a, ap_int const& b);
            ARITHMETIC_EXPORT int       COMPARE(ap_int const& a, ap_int const& b);
        }

        namespace vec {

            extern bool PACK;

            /*
             * OPERATORS
             */

            // GETTERS
            static bool IS_ZERO(s_ap_int_vec const& vec);
            static unsigned int GET_OCC(s_ap_int_vec const& vec);
            static unsigned int GET_FACTORY_ID(s_ap_int_vec const& vec);
            static unsigned int FIND_POS(s_ap_int_vec const& vec, unsigned int pos);

            // MUTATORS
            static void SET_SIZE(s_ap_int_vec& vec, unsigned int size);
            static void SET_OCC(s_ap_int_vec& vec, unsigned int occ);
            static void SET_FACTORY_ID(s_ap_int_vec& vec, unsigned int factory_id);
            static void DELETE_POS(s_ap_int_vec& vec, unsigned int pos);
            static void SWAP_VALUES(s_ap_int_vec& vec, int pos1, int pos2);

            // GETTERS WITH MUTABLE RETURN VALUES
            static const num::ap_int& AT(const s_ap_int_vec& vec, int i);
            static num::ap_int& GET(s_ap_int_vec& vec, int i);

            // ALLOCATORS
            static void DELETE(s_ap_int_vec& vec);
            static s_ap_int_vec COPY(int factory_id, s_ap_int_vec& vec);
            static s_ap_int_vec NEW(unsigned int factory_id, std::vector<std::pair<UL, std::pair<bool, UL>>> vector);

            static void ENLARGE(s_ap_int_vec& vec);
            static void ENLARGE(s_ap_int_vec& vec, unsigned int occ);
            static void ENLARGE(s_ap_int_vec& vec, unsigned int occ, unsigned int size);

            static void PACK_VEC(s_ap_int_vec& vec);


            namespace factory {
                template<typename factory_type>
                static unsigned int REGISTER();
                static void RELEASE(unsigned int id);
            }


            // MUTATORS
            static void SWITCH_SIGNS(s_ap_int_vec& vec);
            static void DELETE_INDEX(s_ap_int_vec& vec, int index);


            /*
             * ARITHMETIC
             */


            /**
             * Assume trivial starts with -1 and the k-th entry of a is at the same pos as the first of trivial.
             */
            static void REDUCE(s_ap_int_vec& v, s_ap_int_vec& trivial, int k);

            ARITHMETIC_EXPORT void ADD(s_ap_int_vec& a, int start_a, const num::ap_int& lambda, s_ap_int_vec& b, int start_b, int copy_a);
            ARITHMETIC_EXPORT void COMBINE(s_ap_int_vec& a, s_ap_int_vec& b, num::ap_int& lambda_a, num::ap_int& tau_a, num::ap_int& lambda_b, num::ap_int& tau_b, int start_a, int start_b, int copy_a, int copy_b);
            ARITHMETIC_EXPORT void MOD(s_ap_int_vec& vec);
        }

        class ap_context : public context {
        private:

            ap_context() { }

        public:

            ap_context(ap_context const&) = delete;
            void operator=(ap_context const&) = delete;

            static ap_context* instance() {
                static ap_context i;
                return &i;
            }

            virtual bool i_mul(num::ap_int& curr, num::ap_int& a, num::ap_int& b) {
                num::i_MUL(curr, a, b);
                return false;
            }

            virtual bool add(num::ap_int& a, num::ap_int& b) {
                num::ADD(a, b);
                return false;
            }

            virtual bool mul(num::ap_int& a, num::ap_int& b) {
                num::MUL(a, b);
                return false;
            }

            virtual bool gcd(num::ap_int& s, num::ap_int& t, num::ap_int& q_a, num::ap_int& q_b, num::ap_int& a, num::ap_int& b) {
                num::GCD(s, t, q_a, q_b, a, b);
                return false;
            }

            virtual bool is_single(num::ap_int const& a) {
                return num::IS_SINGLE(a);
            }



            virtual bool vec_reduce(vec::s_ap_int_vec& v, vec::s_ap_int_vec& trivial, int k) {
                vec::REDUCE(v, trivial, k);
                return false;
            }

            virtual bool vec_add(vec::s_ap_int_vec& a, int start_a, const num::ap_int& coeff, vec::s_ap_int_vec& b, int start_b, int copy_a) {
                vec::ADD(a, start_a, coeff, b, start_b, copy_a);
                return false;
            }

            virtual bool vec_combine(vec::s_ap_int_vec& a, vec::s_ap_int_vec& b, num::ap_int& lambda_a, num::ap_int& tau_a, num::ap_int& lambda_b, num::ap_int& tau_b, int start_a, int start_b, int copy_a, int copy_b) {
                vec::COMBINE(a, b, lambda_a, tau_a, lambda_b, tau_b, start_a, start_b, copy_a, copy_b);
                return false;
            }
        };

        class sp_context : public context {
        private:

            sp_context() { }

        public:

            sp_context(sp_context const&) = delete;
            void operator=(sp_context const&) = delete;

            static sp_context* instance() {
                static sp_context i;
                return &i;
            }

            virtual bool i_mul(num::ap_int& curr, num::ap_int& a, num::ap_int& b);
            virtual bool add(num::ap_int& a, num::ap_int& b);
            virtual bool mul(num::ap_int& a, num::ap_int& b);
            virtual bool gcd(num::ap_int& s, num::ap_int& t, num::ap_int& q_a, num::ap_int& q_b, num::ap_int& a, num::ap_int& b);
            virtual bool is_single(num::ap_int const& a) {
                return true;
            }

            virtual bool vec_reduce(vec::s_ap_int_vec& v, vec::s_ap_int_vec& trivial, int k) {
                return vec_add(v, k + 1, vec::AT(v, k), trivial, 1, k);
            }
            virtual bool vec_add(vec::s_ap_int_vec& a, int start_a, const num::ap_int& coeff, vec::s_ap_int_vec& b, int start_b, int copy_a);
            virtual bool vec_combine(vec::s_ap_int_vec& a, vec::s_ap_int_vec& b, num::ap_int& lambda_a, num::ap_int& tau_a, num::ap_int& lambda_b, num::ap_int& tau_b, int start_a, int start_b, int copy_a, int copy_b);
        };

        class arith_context : public context {
        private:
            context* c;

        public:
            
            arith_context(context* c) : c(c) { }

            virtual bool i_mul(num::ap_int& curr, num::ap_int& a, num::ap_int& b) {
                if (c->i_mul(curr, a, b)) {
                    make_ap();
                }
                return false;
            }

            virtual bool add(num::ap_int& a, num::ap_int& b) {
                if (c->add(a, b)) {
                    make_ap();
                }
                return false;
            }

            virtual bool mul(num::ap_int& a, num::ap_int& b) {
                if (c->mul(a, b)) {
                    make_ap();
                }
                return false;
            }

            virtual bool gcd(num::ap_int& s, num::ap_int& t, num::ap_int& q_a, num::ap_int& q_b, num::ap_int& a, num::ap_int& b) {
                if (c->gcd(s, t, q_a, q_b, a, b)) {
                    make_ap();
                }
                return false;
            }

            virtual bool is_single(num::ap_int const& a) {
                return c->is_single(a);
            }




            virtual bool vec_reduce(vec::s_ap_int_vec& v, vec::s_ap_int_vec& trivial, int k) {
                if (c->vec_reduce(v, trivial, k)) {
                    make_ap();
                }
                return false;
            }

            virtual bool vec_add(vec::s_ap_int_vec& a, int start_a, const num::ap_int& coeff, vec::s_ap_int_vec& b, int start_b, int copy_a) {
                if (c->vec_add(a, start_a, coeff, b, start_b, copy_a)) {
                    make_ap();
                }
                return false;
            }

            virtual bool vec_combine(vec::s_ap_int_vec& a, vec::s_ap_int_vec& b, num::ap_int& lambda_a, num::ap_int& tau_a, num::ap_int& lambda_b, num::ap_int& tau_b, int start_a, int start_b, int copy_a, int copy_b) {
                if (c->vec_combine(a, b, lambda_a, tau_a, lambda_b, tau_b, start_a, start_b, copy_a, copy_b)) {
                    make_ap();
                }
                return false;
            }

            inline void make_ap() {
                c = ap_context::instance();
                std::cout << "SWAPPED TO MULTI PRECISION" << std::endl;
            }
        };

        arith_context* get_context();
    }
}

#include "../../src/operator.hpp"

#endif //ANUBIS_ARITHMETIC_SUPERBUILD_OPERATOR_PUBLIC_HPP
