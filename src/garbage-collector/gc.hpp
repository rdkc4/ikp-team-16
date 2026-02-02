#ifndef GARBAGE_COLLECTOR_HPP
#define GARBAGE_COLLECTOR_HPP

#include "../common/gc/gc-visitor.hpp"
#include "../root-set-table/root-set-table.hpp"
#include "../root-set-table/thread-local-stack.hpp"
#include "../root-set-table/global-root.hpp"
#include "../root-set-table/register-root.hpp"
#include "../heap/heap.hpp"

/**
 * @class garbage_collector
 * @brief implementation of the mark-sweep gc
*/
class garbage_collector final : public gc_visitor {
private:
    /**
     * @brief marks all objects that are reachable from the root-set-table.
     * @param root_set - reference to a root-set-table
    */
    void mark(root_set_table& root_set) noexcept {
        auto& roots_table = root_set.get_roots();

        auto** buckets = roots_table.get_buckets();
        size_t capacity = roots_table.get_capacity();

        for(size_t i = 0; i < capacity; ++i) {
            auto* root = buckets[i];

            while(root) {
                if(root->value){
                    root->value->accept(*this);
                }
                root = root->next;
            }
        }
    }

    /**
     * @brief sweeps objects from a segment.
     * @param seg - reference to a segment.
    */
    void sweep_segment(segment& seg) noexcept {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(&seg);
        const uint8_t* endptr = ptr + SEGMENT_SIZE;
        while(ptr < endptr) {
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

    /**
     * @brief sweeps the unmarked objects from heap.
     * @param heap_memory - reference to a heap.
    */
    void sweep(heap& heap_memory) noexcept {
        for(size_t i = 0; i < SMALL_OBJECT_SEGMENTS; ++i) {
            sweep_segment(heap_memory.get_small_object_segment(i));
        }
        for(size_t i = 0; i < MEDIUM_OBJECT_SEGMENTS; ++i) {
            sweep_segment(heap_memory.get_medium_object_segment(i));
        }
        for(size_t i = 0; i < LARGE_OBJECT_SEGMENTS; ++i) {
            sweep_segment(heap_memory.get_large_object_segment(i));
        }
    }

public:
    /**
     * @brief collects the garbage from the heap.
     * @param root_set - reference to a root-set-table.
     * @param heap_memory - reference to a heap.
    */
    void collect(root_set_table& root_set, heap& heap_memory) noexcept {
        mark(root_set);
        sweep(heap_memory);
    }

    /**
     * @brief marks the objects on the stack.
     * @param stack - reference to a thread local stack.
    */
    void visit(thread_local_stack& stack) override final {
        auto& stack_data = stack.get_thread_stack();
        for(thread_local_stack_entry& entry : stack_data) {
            if(entry.ref_to){
                entry.ref_to->set_marked(true);
            }
        }
    }

    /**
     * @brief marks the global object.
     * @param global - reference to a global root.
    */
    void visit(global_root& global) override final {
        header* gvar = global.get_global_variable();
        if(gvar){
            gvar->set_marked(true);
        }
    }

    /**
     * @brief marks the register object.
     * @param reg - reference to a register root.
    */
    void visit(register_root& reg) override final {
        header* reg_var = reg.get_register_variable();
        if(reg_var){
            reg_var->set_marked(true);
        }
    }

};

#endif