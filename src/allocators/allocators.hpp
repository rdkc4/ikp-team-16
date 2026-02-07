#ifndef ALLOCATORS_HPP
#define ALLOCATORS_HPP

#include <string>
#include <random>

#include "../heap-manager/heap-manager.hpp"
#include "../common/thread-pool/thread-pool.hpp"
#include "../root-set-table/thread-local-stack.hpp"
#include "../root-set-table/global-root.hpp"
#include "../root-set-table/register-root.hpp"

/// number of allocations per tls in stress mode.
size_t constexpr TLS_ALLOC_STRESS_THRESHOLD = 10000;

/// number of scopes per tls for stress mode.
size_t constexpr TLS_STACK_COUNT_STRESS = 10;

/// number of allocations per scope for tls in stress mode.
size_t constexpr TLS_ALLOC_STRESS_THRESHOLD_PER_STACK = TLS_ALLOC_STRESS_THRESHOLD / TLS_STACK_COUNT_STRESS;

/// number of allocations per tls in relaxed mode.
size_t constexpr TLS_ALLOC_RELAXED_THRESHOLD = 1000;

/// number of scopes per tls in relaxed mode.
size_t constexpr TLS_STACK_COUNT_RELAXED = 10;

/// number of allocations per scope for tls in relaxed mode.
size_t constexpr TLS_ALLOC_RELAXED_THRESHOLD_PER_STACK = TLS_ALLOC_RELAXED_THRESHOLD / TLS_STACK_COUNT_RELAXED;

/// number of allocations per global in stress mode.
size_t constexpr GLOBAL_ALLOC_STRESS_THRESHOLD = 100;

/// number of allocations per global in relaxed mode.
size_t constexpr GLOBAL_ALLOC_RELAXED_THRESHOLD = 20;

/// number of allocations per register in stress mode.
size_t constexpr REGISTER_ALLOC_STRESS_THRESHOLD = 100;

/// number of allocations per register in relaxed mode.
size_t constexpr REGISTER_ALLOC_RELAXED_THRESHOLD = 20;

/**
 * @class allocators
 * @brief simulates the allocations on the heap.
*/
class allocators {
private:
    /// reference to a heap manager used by allocators.
    heap_manager& heap_manager_ref;

    /// allocators thread pool.
    thread_pool alloc_thread_pool;

    /// random number generator.
    static thread_local std::mt19937 rng;

    /// distribution for object size category.
    static thread_local std::uniform_int_distribution<int> category_dist;

    /**
     * @brief creates the thread local stack root.
     * @param tls_root_key - key for the thread local stack.
     * @returns pointer to a thread local stack.
    */
    thread_local_stack* create_tls_root(std::string tls_root_key);

    /**
     * @brief creates the global root.
     * @param global_root_key - key for the global root.
     * @returns pointer to a global root.
    */
    global_root* create_global_root(std::string global_root_key);

    /**
     * @brief creates the register root.
     * @param register_root_key - key for the register root.
     * @returns pointer to a register root.
    */
    register_root* create_register_root(std::string register_root_key);

    /**
     * @brief simulates allocation of a thread, stress mode.
     * @param tls - pointer to a thread local stack.
    */
    void simulate_tls_alloc_stress(thread_local_stack* tls);

    /**
     * @brief simulates allocation of a thread, relaxed mode.
     * @param tls - pointer to a thread local stack.
    */
    void simulate_tls_alloc_relaxed(thread_local_stack* tls);

    /**
     * @brief simulates allocation of a global variable, stress mode.
     * @param global - pointer to a global root.
    */
    void simulate_global_alloc_stress(global_root* global);

    /**
     * @brief simulates allocation of a global variable, relaxed mode.
     * @param global - pointer to a global root.
    */
    void simulate_global_alloc_relaxed(global_root* global);

    /**
     * @brief simulates allocation of a register variable, stress mode.
     * @param register - pointer to a register root.
    */
    void simulate_register_alloc_stress(register_root* reg);

    /**
     * @brief simulates allocation of a register variable, relaxed mode.
     * @param register - pointer to a register root.
    */
    void simulate_register_alloc_relaxed(register_root* reg);

    /**
     * @brief generates the size of the object.
     * @returns amount of bytes object needs for allocation.
    */
    uint32_t generate_random_size();

public:
    /**
     * @brief creates the instance of the allocators.
     * @param heap_manager_ref - reference to a heap manager.
     * @param thread_count - number of thread in allocators thread pool.
    */
    allocators(heap_manager& heap_manager_ref, size_t thread_count);

    /**
     * @brief deletes the instance of the allocators.
    */
    ~allocators() = default;

    /**
     * @brief starts the allocation simulation, stress mode.
     * @param tls_count - number of threads that are allocating objects.
     * @param global_count - number of global variables that are referencing objects.
     * @param register_count - number of register variables that are referencing objects.
    */
    void simulate_alloc_stress(size_t tls_count, size_t global_count, size_t register_count);

    /**
     * @brief starts the allocation simulation, relaxed mode.
     * @param tls_count - number of threads that are allocating objects.
     * @param global_count - number of global variables that are allocating objects.
     * @param register_count - number of register variables that are allocating objects.
    */
    void simulate_alloc_relaxed(size_t tls_count, size_t global_count, size_t register_count);

};

#endif