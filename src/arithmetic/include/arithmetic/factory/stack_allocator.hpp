//
// Created by jmaerte on 11.05.20.
//

#ifndef ANUBIS_SUPERBUILD_STACK_ALLOCATOR_HPP
#define ANUBIS_SUPERBUILD_STACK_ALLOCATOR_HPP

#include "Ivector_allocator.hpp"
#include "../operator.hpp"
#include <cstdlib>
#include <sstream>

namespace jmaerte {
    namespace arith {
        namespace vec {

            /**
             * An allocator that successively allocates vector space.
             * These are saved in blocks of size block_size.
             *
             * @tparam block_size how many svec_nodes fit in there.
             */
            template<std::size_t block_size>
            class stack_allocator : public vector_allocator {
            public:
                ~stack_allocator() {
                    this->free();
                }


                // ALLOCATION
                virtual s_ap_int_vec allocate_vec(std::size_t size) {
                    if (1 + 2 * size > block_size) {
                        jmaerte::output::LOGGER.err(channel_id,
                            "Vector got larger than page size!",
                            jmaerte::output::LOGGER.BAD_ALLOC);
                        throw;
                    }
                    if (alloc == nullptr || (1 + 2 * size) + (alloc - blocks[page]) > block_size) {
                        if (page == blocks.size() - 1) {
                            blocks.push_back(static_cast<svec_node*>(malloc(type_size * block_size)));
                            if (blocks.back() == nullptr || blocks.back() == NULL) {
                                jmaerte::output::LOGGER.err(channel_id,
                                        "Allocation of a memory block of " + std::to_string(type_size * block_size) + " bytes failed.",
                                        jmaerte::output::LOGGER.BAD_ALLOC);
                            }
                            std::ostringstream address;
                            std::ostringstream address_end;
                            address << static_cast<const void*>(blocks.back());
                            address_end << static_cast<const void*>(blocks.back() + block_size);
                            jmaerte::output::LOGGER.log(channel_id, "Allocated memory block of size " + std::to_string(type_size * block_size) + " bytes. Starting in " +
                                     address.str() + " reaching up to " + address_end.str() + ".");
                            if (alloc != nullptr) aligned.push_back(block_size - (alloc - blocks[page]));
                        } else aligned[page] = block_size - (alloc - blocks[page]);
                        alloc = blocks[++page];
                    }
                    s_ap_int_vec v = alloc;
                    alloc += (1 + 2 * size);
                    SET_FACTORY_ID(v, get_id());
                    return v;
                }

                /**
                 * This function can only be performed on
                 * the last vector allocated.
                 */
                virtual void deallocate_vec(s_ap_int_vec v) {
                    for (int i = 0; i < GET_OCC(v); i++) {
                        num::DELETE_DATA(AT(v, i));
                    }
                    reset_alloc(v);
                }

                // COPY
                virtual s_ap_int_vec copy(s_ap_int_vec v) {
                    s_ap_int_vec res = allocate_vec(GET_OCC(v));
                    *res = {.single = v->single};
                    SET_FACTORY_ID(res, get_id());
                    SET_SIZE(res, GET_OCC(v));
                    for (int i = 0; i < GET_OCC(v); i++) {
                        num::ASSIGN(AT(res, i), num::COPY(AT(v, i)));
                    }
                    return res;
                }

                // ENLARGE
                virtual void enlarge(s_ap_int_vec& v, std::size_t new_size, std::size_t copy_range) {
                    reset_alloc(v);
                    s_ap_int_vec next = allocate_vec(new_size);
                    if (next != v) {
                        // page got changed because v got too large to safe it in the remaining page space
                        std::copy(v, AT(v, copy_range), next);
                    }
                    v = next;
                    SET_SIZE(v, new_size);
                }

                // FREE
                virtual void free() {
                    for (s_ap_int_vec v : blocks) {
                        std::free(static_cast<void*>(v));
                    }
                }

                void print() override {
                    for(int i = 0; i < blocks.size(); i++) {
                        std::cout << blocks[i] << " ";
                    }
                    std::cout << std::endl;
                }

            private:

                void reset_alloc(s_ap_int_vec v) {
                    alloc = v;
                    if (page > 0 && alloc == blocks.back()) {
                        alloc = blocks[--page] + block_size - aligned[page];
                    }
                }

                svec_node* alloc;
                int page = -1;
                std::vector<svec_node*> blocks;
                std::vector<std::size_t> aligned;
                std::size_t type_size = sizeof(svec_node);
            };
        }
    }
}

#endif //ANUBIS_SUPERBUILD_STACK_ALLOCATOR_HPP
