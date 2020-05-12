//
// Created by jmaerte on 06.04.20.
//

#ifndef ANUBIS_SUPERBUILD_BLOCK_ALLOCATOR_HPP
#define ANUBIS_SUPERBUILD_BLOCK_ALLOCATOR_HPP

#include <DATA_STRUCTURES_EXPORT.h>
#include <utility>

namespace jmaerte {
    namespace data_type {

        template<typename T, std::size_t page_size>
        class serial_block_allocator;

        template<typename T, std::size_t page_size, serial_block_allocator<T, page_size> alloc>
        struct serial_pointer {
            T* address;
            int page;
        };

        template<typename T, std::size_t page_size, serial_block_allocator<T, page_size> alloc>
        serial_pointer<T, page_size, alloc> operator+(const serial_pointer<T, page_size>& ptr, int n) {
            if ((address - alloc.pages[ptr.page]) / sizeof(T) > page_size) 
        }

        /**
         *
         * @tparam block_size number of chunks per block
         * @tparam chunk_size number of bytes per chunk
         */
        template<typename T, std::size_t page_size>
        class DATA_STRUCTURES_EXPORT serial_block_allocator {
        public:
            serial_pointer<T, page_size, *this> allocate(std::size_t num);
            void  deallocate(serial_pointer<T, page_size, *this> ptr, std::size_t size);

            block_allocator(const block_allocator&) = default;
            block_allocator(block_allocator&&) = default;
            block_allocator& operator=(const block_allocator& a) {
                if (this != &a) {
                    page_size = a.page_size;
                    chunk_size = a.chunk_size;
                    pages = a.pages;
                    alloc_page = a.alloc_page;
                    alloc = a.alloc;
                }

                return *this;
            }
            block_allocator& operator=(block_allocator&&) {
                if (this != &a) {
                    page_size = a.page_size;
                    chunk_size = a.chunk_size;
                    pages = a.pages;
                    alloc_page = a.alloc_page;
                    alloc = a.alloc;
                }

                return *this;
            }


            std::vector<void*> pages;
            int alloc_page = -1;
            void* alloc = nullptr;

            void* allocate_page();
        };
    }
}

#include "../../../src/memory/pool_allocator.cpp"

#endif //ANUBIS_SUPERBUILD_BLOCK_ALLOCATOR_HPP
