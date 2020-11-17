//
// Created by jmaerte on 08.05.20.
//

#ifndef ANUBIS_SUPERBUILD_VECTOR_FACTORY_HPP
#define ANUBIS_SUPERBUILD_VECTOR_FACTORY_HPP

#include "Ivector_allocator.hpp"
#include <output/logger.hpp>
#include <map>
#include <stdexcept>
#include <string>
#include <iostream>
#include <exception>
#include "arithmetic/constants.hpp"

namespace jmaerte {
    namespace arith {
        namespace vec {

            class vector_factory;
            class vector_allocator;

            namespace factory {
                extern ARITHMETIC_EXPORT unsigned int MAX_FACTORIES;

                extern ARITHMETIC_EXPORT vector_factory& dict;
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

                vector_allocator* get_factory(s_ap_int_vec& v);

                vector_allocator* get_factory(unsigned int id) {
                    try {
                        return factories.at(id);
                    } catch (const std::out_of_range& e) {
                        jmaerte::output::LOGGER.err(jmaerte::output::MEM_CHANNEL,
                                                     "Tried to access factory " + std::to_string(id) + " that is non-existent!",
                                                     jmaerte::output::LOGGER.INVALID_ARG);
//                        std::cout << "[Mem] ERROR - Tried to access factory that is non-existent!" << std::endl;
                        throw e;
                    }
                }

                unsigned int push_factory(vector_allocator* alloc) {
                    for (unsigned int i = 0; i < max_factories; i++) {
                        if (factories.count(i) == 0) {
                            factories.insert({i, alloc});
                            jmaerte::output::LOGGER.log(jmaerte::output::MEM_CHANNEL,
                                                         "Factory of ID " + std::to_string(i) + " created.");
                            jmaerte::output::LOGGER.log(jmaerte::output::MEM_CHANNEL,
                                                            "Current ");
//                            std::cout << "[Mem] Factory of ID " << i << " created." << std::endl;
                            return i;
                        }
                    }
                    jmaerte::output::LOGGER.err(jmaerte::output::MEM_CHANNEL,
                            "Factory reached maximum number " + std::to_string(max_factories) + " of allocators.",
                            jmaerte::output::LOGGER.BAD_ALLOC);
                    throw std::runtime_error("");
//                    std::cout << "[Mem] Error - Factory reached maximum number " << max_factories << " of allocators." << std::endl;
//                    throw std::overflow_error{"[Mem] Error - Factory reached maximum number " + std::to_string(max_factories) + " of allocators."};
                }

                void release_factory(unsigned int id) {
                    vector_allocator* alloc = factories.at(id);
                    delete alloc;
                    factories.erase(id);
                }

                ~vector_factory() {
                    for (unsigned int i = 0; i < max_factories; i++) {
                        auto it = factories.find(i);
                        if (it != factories.end()) {
                            delete it->second;
                            factories.erase(it);
                        }
                    }
                }

            private:
                std::map<unsigned int, vector_allocator*> factories;
                unsigned int output_channel_id;

                vector_factory(std::size_t max_factories): max_factories(max_factories) {}

                std::size_t max_factories;
            };
        }
    }
}

#endif //ANUBIS_SUPERBUILD_VECTOR_FACTORY_HPP
