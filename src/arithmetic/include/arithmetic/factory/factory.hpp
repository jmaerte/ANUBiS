//
// Created by jmaerte on 08.05.20.
//

#ifndef ANUBIS_SUPERBUILD_VECTOR_FACTORY_HPP
#define ANUBIS_SUPERBUILD_VECTOR_FACTORY_HPP

#include "Ivector_allocator.hpp"
#include <map>
#include <stdexcept>
#include <string>
#include <iostream>

namespace jmaerte {
    namespace arith {
        namespace vec {

            class vector_factory;
            class vector_allocator;

            namespace factory {
                static inline unsigned int MAX_FACTORIES = 8u;

                extern vector_factory& dict;
            }

            /**
             * Singleton
             */
            class ARITHMETIC_EXPORT vector_factory {
            public:

                static vector_factory& get_instance(std::size_t capacity) {
                    static vector_factory instance {capacity};
                    return instance;
                }
                vector_factory(const vector_factory&) = delete;
                void operator=(const vector_factory&) = delete;


                vector_factory(std::size_t max_factories): max_factories(max_factories) { }

                vector_allocator* get_factory(s_ap_int_vec& v);

                vector_allocator* get_factory(unsigned int id) {
                    try {
                        return factories.at(id);
                    } catch (const std::out_of_range& e) {
                        std::cout << "[Mem] ERROR - Tried to access factory that is non-existent!" << std::endl;
                        throw e;
                    }
                }

                unsigned int push_factory(vector_allocator* alloc) {
                    for (unsigned int i = 0; i < max_factories; i++) {
                        if (factories.count(i) == 0) {
                            factories.insert({i, alloc});
                            std::cout << "[Mem] Factory of ID " << i << " created." << std::endl;
                            return i;
                        }
                    }
                    std::cout << "[Mem] Error - Factory reached maximum number " << max_factories << " of allocators." << std::endl;
                    throw std::overflow_error{"[Mem] Error - Factory reached maximum number " + std::to_string(max_factories) + " of allocators."};
                }

                void release_factory(unsigned int id) {
                    vector_allocator* alloc = factories.at(id);
                    delete alloc;
                    factories.erase(id);
                }

            private:
                std::map<unsigned int, vector_allocator*> factories;

                std::size_t max_factories;
            };
        }
    }
}

#endif //ANUBIS_SUPERBUILD_VECTOR_FACTORY_HPP
