#include "gc.hpp"

#include <latch>

garbage_collector::garbage_collector(size_t thread_count) : gc_thread_pool(thread_count) {}

void garbage_collector::collect(root_set_table& root_set, heap& heap_memory) noexcept {
    mark(root_set);
    sweep(heap_memory);
}

void garbage_collector::visit(thread_local_stack& stack){
    auto& stack_data = stack.get_thread_stack();
    for(thread_local_stack_entry& entry : stack_data) {
        if(entry.ref_to){
            entry.ref_to->set_marked(true);
        }
    }
}

void garbage_collector::visit(global_root& global){
    header* gvar = global.get_global_variable();
    if(gvar){
        gvar->set_marked(true);
    }
}

void garbage_collector::visit(register_root& reg){
    header* reg_var = reg.get_register_variable();
    if(reg_var){
        reg_var->set_marked(true);
    }
}

void garbage_collector::mark(root_set_table& root_set) noexcept {
    const size_t total = root_set.get_root_count();
    if(total == 0) return;

    std::latch completion_latch(static_cast<std::ptrdiff_t>(total));

    auto& roots_table = root_set.get_roots();
    auto** buckets = roots_table.get_buckets();
    const size_t capacity = roots_table.get_capacity();

    for(size_t i = 0; i < capacity; ++i) {
        for(auto* root = buckets[i]; root; root = root->next){
            if(!root->value) continue;

            gc_thread_pool.enqueue([&, root_value = root->value.get()]{
                root_value->accept(*this);
                completion_latch.count_down();
            });
            
        }
    }

    completion_latch.wait();
}

void garbage_collector::sweep_segment(segment& seg) noexcept {
    uint8_t* ptr = seg.segment_memory;
    const uint8_t* endptr = seg.segment_memory + SEGMENT_SIZE;
    
    while(ptr + sizeof(header) <= endptr) {
        header* hdr = reinterpret_cast<header*>(ptr);

        if(hdr->is_marked()) {
            hdr->set_marked(false);
        }
        else {
            hdr->set_free(true);
        }

        ptr += sizeof(header) + static_cast<size_t>(hdr->size);
    }
}

void garbage_collector::sweep(heap& heap_memory) noexcept {
    if constexpr (TOTAL_SEGMENTS == 0) return;
    
    std::latch completion_latch(TOTAL_SEGMENTS);

    auto enqueue_segment_sweep = [&](segment& segment) -> void {
        gc_thread_pool.enqueue([&, seg=&segment] -> void {
            sweep_segment(*seg);
            completion_latch.count_down();
        });
    };

    for(size_t i = 0; i < SMALL_OBJECT_SEGMENTS; ++i) {
        enqueue_segment_sweep(heap_memory.get_small_object_segment(i));
    }

    for(size_t i = 0; i < MEDIUM_OBJECT_SEGMENTS; ++i) {
        enqueue_segment_sweep(heap_memory.get_medium_object_segment(i));
    }

    for(size_t i = 0; i < LARGE_OBJECT_SEGMENTS; ++i) {
        enqueue_segment_sweep(heap_memory.get_large_object_segment(i));
    }

    completion_latch.wait();
}