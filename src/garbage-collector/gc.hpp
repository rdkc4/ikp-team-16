#ifndef GARBAGE_COLLECTOR_HPP
#define GARBAGE_COLLECTOR_HPP

#include <cstddef>

#include "../common/gc/gc-visitor.hpp"
#include "../root-set-table/root-set-table.hpp"
#include "../root-set-table/thread-local-stack.hpp"
#include "../root-set-table/global-root.hpp"
#include "../root-set-table/register-root.hpp"
#include "../heap/heap.hpp"
#include "../common/thread-pool/thread-pool.hpp"

/**
 * @class garbage_collector
 * @brief implementation of the mark-sweep gc
*/
class garbage_collector final : public gc_visitor {
private:
    /// thread pool for concurrent marking and sweeping.
    thread_pool gc_thread_pool;

    /**
     * @brief marks all objects that are reachable from the root-set-table.
     * @param root_set - reference to a root-set-table
    */
    void mark(root_set_table& root_set) noexcept;

    /**
     * @brief sweeps objects from a segment.
     * @param seg - reference to a segment.
    */
    void sweep_segment(segment& seg) noexcept;

    /**
     * @brief sweeps the unmarked objects from heap.
     * @param heap_memory - reference to a heap.
    */
    void sweep(heap& heap_memory) noexcept;

public:
    /**
     * @brief creates the instance of the garbage collector.
     * @param thread_count - number of threads in a thread pool, defaults to 1.
    */
    garbage_collector(size_t thread_count = 1);

    /**
     * @brief deletes the instance of the garbage collector.
    */
    ~garbage_collector() = default;

    /**
     * @brief collects the garbage from the heap.
     * @param root_set - reference to a root-set-table.
     * @param heap_memory - reference to a heap.
    */
    void collect(root_set_table& root_set, heap& heap_memory) noexcept;

    /**
     * @brief marks the objects on the stack.
     * @param stack - reference to a thread local stack.
    */
    void visit(thread_local_stack& stack) override final;

    /**
     * @brief marks the global object.
     * @param global - reference to a global root.
    */
    void visit(global_root& global) override final;

    /**
     * @brief marks the register object.
     * @param reg - reference to a register root.
    */
    void visit(register_root& reg) override final;

};

#endif