#ifndef ALLOCATORS_HPP
#define ALLOCATORS_HPP

#include <format>
#include <memory>
#include <string>
#include <latch>
#include <random>
#include <utility>
#include <concepts>
#include <iostream>

#include "../heap-manager/heap-manager.hpp"
#include "../common/thread-pool/thread-pool.hpp"
#include "../root-set-table/thread-local-stack.hpp"
#include "../root-set-table/global-root.hpp"
#include "../root-set-table/register-root.hpp"

/// number of allocations per tls in stress mode.
size_t constexpr TLS_ALLOC_STRESS_THRESHOLD = 8192;

/// number of scopes per tls for stress mode.
size_t constexpr TLS_SCOPE_COUNT_STRESS = 8;

/// number of allocations per scope for tls in stress mode.
size_t constexpr TLS_ALLOC_STRESS_THRESHOLD_PER_SCOPE = TLS_ALLOC_STRESS_THRESHOLD / TLS_SCOPE_COUNT_STRESS;

/// capacity of the hash-map that maps tls variable to its index in stress mode.
size_t constexpr TLS_MAP_CAPACITY_STRESS = TLS_ALLOC_STRESS_THRESHOLD_PER_SCOPE << 1;

/// number of allocations per tls in relaxed mode.
size_t constexpr TLS_ALLOC_RELAXED_THRESHOLD = 1024;

/// number of scopes per tls in relaxed mode.
size_t constexpr TLS_SCOPE_COUNT_RELAXED = 8;

/// number of allocations per scope for tls in relaxed mode.
size_t constexpr TLS_ALLOC_RELAXED_THRESHOLD_PER_SCOPE = TLS_ALLOC_RELAXED_THRESHOLD / TLS_SCOPE_COUNT_RELAXED;

/// capacity of the hash-map that maps tls variable to its index in relaxed mode.
size_t constexpr TLS_MAP_CAPACITY_RELAXED = TLS_ALLOC_STRESS_THRESHOLD_PER_SCOPE << 1;

/// number of allocations per global in stress mode.
size_t constexpr GLOBAL_ALLOC_STRESS_THRESHOLD = 128;

/// number of allocations per global in relaxed mode.
size_t constexpr GLOBAL_ALLOC_RELAXED_THRESHOLD = 32;

/// number of allocations per register in stress mode.
size_t constexpr REGISTER_ALLOC_STRESS_THRESHOLD = 128;

/// number of allocations per register in relaxed mode.
size_t constexpr REGISTER_ALLOC_RELAXED_THRESHOLD = 32;

/**
 * @enum simulation_mode
 * @brief defines the type of the simulation.
*/
enum class simulation_mode { stress, relaxed };

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

    /// distribution for small object size.
    static thread_local std::uniform_int_distribution<uint32_t> small_dist;

    /// distribution for medium object size.
    static thread_local std::uniform_int_distribution<uint32_t> medium_dist;

    /// distribution for large object size.
    static thread_local std::uniform_int_distribution<uint32_t> large_dist;

    /**
     * @brief simulates allocation of a thread, stress mode.
     * @param tls - pointer to a thread local stack.
     * @param scope_count - number of scopes.
     * @param allocs_per_scope - number of allocations per scope.
    */
    void simulate_tls_alloc(thread_local_stack* tls, size_t scope_count, size_t allocs_per_scope);

    /**
     * @brief simulates allocation of a global variable, stress mode.
     * @param global - pointer to a global root.
     * @param global_allocs - number of allocations for global variable.
    */
    void simulate_global_alloc(global_root* global, size_t global_allocs);

    /**
     * @brief simulates allocation of a register variable, stress mode.
     * @param register - pointer to a register root.
     * @param register_allocs - number of allocations for register.
    */
    void simulate_register_alloc(register_root* reg, size_t register_allocs);

    /**
     * @brief creates the root for root-set-table.
     * @tparam root - type of the root.
     * @tparam ...args - types of arguments for root construction.
     * @param key - key of the root.
     * @param arguments - arguments for the construction.
     * @returns pointer to a root-set-table element. 
    */
    template <typename root, typename... args>
    requires std::derived_from<root, root_set_base>
    root* create_root(const std::string& key, args&&... arguments){
        auto root_ptr = std::make_unique<root>(std::forward<args>(arguments)...);
        heap_manager_ref.add_root(key, std::move(root_ptr));
        return static_cast<root*>(heap_manager_ref.get_root(key));
    }

    /**
     * @brief adds simulation to queue.
     * @tparam fn - type of the function.
     * @param label - name of caller.
     * @param index - index of the caller.
     * @param simulate - simulation function.
     * @param completion_latch - synchronization for simulation.
    */
    template <typename fn>
    void enqueue_simulation(const std::string& label, size_t index, fn&& simulate, std::latch& completion_latch){
        alloc_thread_pool.enqueue([label, index, simulate = std::forward<fn>(simulate), &completion_latch]{
            std::cout << std::format("{} {} is allocating...\n", label, index);
            simulate();
            std::cout << std::format("{} {} finished\n", label, index);
            
            completion_latch.count_down();
        });
    }

    /**
     * @brief getter for the name of the mode of simulation.
     * @param mode - simulation mode.
     * @returns name of the mode.
    */
    static constexpr const char* simulation_mode_name(simulation_mode mode) {
        switch (mode) {
            case simulation_mode::stress: return "stress";
            case simulation_mode::relaxed: return "relaxed";
        }
        std::unreachable();
    }

    /**
     * @brief getter for tls scope count.
     * @param mode - mode of the simulation.
     * @returns number of scopes for tls.
    */
    static constexpr size_t tls_scope_count(simulation_mode mode){
        switch(mode){
            case simulation_mode::stress: return TLS_SCOPE_COUNT_STRESS;
            case simulation_mode::relaxed: return TLS_SCOPE_COUNT_RELAXED;
        }
        std::unreachable();
    }

    /**
     * @brief getter for tls allocation count per scope.
     * @param mode - mode of the simulation.
     * @returns number of allocation for tls per scope.
    */
    static constexpr size_t tls_allocs_per_scope(simulation_mode mode){
        switch(mode){
            case simulation_mode::stress: return TLS_ALLOC_STRESS_THRESHOLD_PER_SCOPE;
            case simulation_mode::relaxed: return TLS_ALLOC_RELAXED_THRESHOLD_PER_SCOPE;
        }
        std::unreachable();
    }

    /**
     * @brief getter for the initial capacity of the hash-map that maps tls variable to its index.
     * @param mode - mode of the simulation.
     * @returns initial capacity of the hash-map.
    */
    static constexpr size_t tls_map_capacity(simulation_mode mode){
        switch(mode){
            case simulation_mode::stress: return TLS_MAP_CAPACITY_STRESS;
            case simulation_mode::relaxed: return TLS_MAP_CAPACITY_RELAXED;
        }
        std::unreachable();
    }

    /**
     * @brief getter for global allocation count.
     * @param mode - mode of the simulation.
     * @returns number of allocation for global variable.
    */
    static constexpr size_t global_alloc_count(simulation_mode mode){
        switch(mode){
            case simulation_mode::stress: return GLOBAL_ALLOC_STRESS_THRESHOLD;
            case simulation_mode::relaxed: return GLOBAL_ALLOC_RELAXED_THRESHOLD;
        }
        std::unreachable();
    }

    /**
     * @brief getter for register allocation count.
     * @param mode - mode of the simulation.
     * @returns number of allocation for register.
    */
    static constexpr size_t register_alloc_count(simulation_mode mode){
        switch(mode){
            case simulation_mode::stress: return REGISTER_ALLOC_STRESS_THRESHOLD;
            case simulation_mode::relaxed: return REGISTER_ALLOC_RELAXED_THRESHOLD;
        }
        std::unreachable();
    }

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
     * @param mode - mode of the simulation.
    */
    void simulate_alloc(size_t tls_count, size_t global_count, size_t register_count, simulation_mode mode);

};

#endif