//
// Created by jmaerte on 24.03.20.
//

#ifndef ANUBIS_ALGEBRA_SUPERBUILD_TYPEDEF_HPP
#define ANUBIS_ALGEBRA_SUPERBUILD_TYPEDEF_HPP

#include <data_types/stream.hpp>
#include <arithmetic/operator.hpp>
#include <arithmetic/factory/Ivector_allocator.hpp>

namespace jmaerte {
    namespace algebra {
        using namespace jmaerte::arith;
        using namespace jmaerte::typings;

        class s_int_matrix {
        private:
            int i = 0;
            int max;
            unsigned int factory_id;
            std::function<vec::s_ap_int_vec(int, unsigned int)> f;

        public:
            s_int_matrix(unsigned int factory_id, int max, std::function<vec::s_ap_int_vec(int, unsigned int)> f) : factory_id(factory_id), max(max), f(f) {}

            vec::s_ap_int_vec get() {
                return f(i++, factory_id);
            }

            bool is_empty() {
                return i >= max;
            }

            unsigned int get_factory_id() {
                return factory_id;
            }
        };

        typedef stream<vec::s_ap_float_vec> s_float_matrix;
    }
}

#endif //ANUBIS_ALGEBRA_SUPERBUILD_TYPEDEF_HPP
