#include <iostream>

#include "src/allocators/allocators.hpp"
#include "src/heap-manager/heap-manager.hpp"

int main() {
    constexpr size_t hm_thread_count = 8;
    constexpr size_t gc_thread_count = 8;
    heap_manager heap_mng(hm_thread_count, gc_thread_count);

    size_t alloc_thread_counts[] = {1, 2, 5, 10};
    const size_t len = sizeof(alloc_thread_counts) / sizeof(size_t);

    constexpr size_t tls_count = 5;
    constexpr size_t global_count = 5;
    constexpr size_t register_count = 5;

    for(size_t i = 0; i < len; ++i){
        std::cout << std::format("Allocators using {} threads in stress mode: \n", alloc_thread_counts[i]);
        allocators allocator(heap_mng, alloc_thread_counts[i]);
        allocator.simulate_alloc(tls_count, global_count, register_count, simulation_mode::stress);
        std::cout << "\n";
    }

    for(size_t i = 0; i < len; ++i){
        std::cout << std::format("Allocators using {} threads in relaxed mode: \n", alloc_thread_counts[i]);
        allocators allocator(heap_mng, alloc_thread_counts[i]);
        allocator.simulate_alloc(tls_count, global_count, register_count, simulation_mode::relaxed);
        std::cout << "\n";
    }
    
    return 0;
}
