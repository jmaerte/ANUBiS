//
// Created by jmaerte on 06.04.20.
//

namespace jmaerte {
    namespace data_type {

        template<typename T, std::size_t block_size>
        void* serial_block_allocator::allocate(std::size_t size) {
            if (alloc == nullptr || size > block_size - (alloc - pages[alloc_page]) / sizeof(T)) {
                if (alloc_page + 1 == pages.size()) {
                    alloc = alloc_block();
                    pages.push_back(alloc);
                } else {
                    alloc = pages[alloc_page + 1];
                }
                alloc_page++;
            }
            alloc += sizeof(T) * size;
            return alloc - sizeof(T) * size;
        }

        template<typename T, std::size_t block_size>
        void serial_block_allocator::deallocate(void* ptr, std::size_t size) {
            if (sizeof(T) * size > (alloc - pages[alloc_page])) {
                size -= (alloc - pages[alloc_page]) / sizeof(T);
                alloc = pages[--alloc_page] + block_size * sizeof(T);
            }
            alloc -= sizeof(T) * size;
        }


        template<typename T, std::size_t block_size>
        chunk* serial_block_allocator::allocate_block() {
            std::cout << "[Mem] Allocating block of size " << (sizeof(T) * block_size) << " bytes." << std::endl;
            return malloc(block_size);
        }
    }
}