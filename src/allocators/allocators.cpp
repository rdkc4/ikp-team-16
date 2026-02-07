#include "allocators.hpp"

#include <format>
#include <memory>
#include <latch>
#include <iostream>
#include <chrono>

allocators::allocators(heap_manager& heap_manager_ref, size_t thread_count) : heap_manager_ref(heap_manager_ref), alloc_thread_pool(thread_count) {}

thread_local std::mt19937 allocators::rng;

thread_local std::uniform_int_distribution<int> allocators::category_dist(0, 99);

void allocators::simulate_alloc_stress(size_t tls_count, size_t global_count, size_t register_count){
    std::cout << "Initializing stress simulation\n";
    auto start_time = std::chrono::high_resolution_clock::now();

    std::latch completion_latch(tls_count + global_count + register_count);

    for(size_t i = 0; i < tls_count; ++i){
        auto tls = create_tls_root("t" + std::to_string(i));
        alloc_thread_pool.enqueue([&, tls, i] -> void {
            std::cout << std::format("TLS {} is allocating objects...\n", i);
            simulate_tls_alloc_stress(tls);
            std::cout << std::format("TLS {} finished\n", i);

            completion_latch.count_down();
        });
    }

    for(size_t i = 0; i < global_count; ++i){
        auto global = create_global_root("g" + std::to_string(i));
        alloc_thread_pool.enqueue([&, global, i] -> void {
            std::cout << std::format("Global {} is allocating...\n", i);
            simulate_global_alloc_stress(global);
            std::cout << std::format("Global {} finished\n", i);

            completion_latch.count_down();
        });
    }

    for(size_t i = 0; i < register_count; ++i){
        auto reg = create_register_root("r" + std::to_string(i));
        alloc_thread_pool.enqueue([&, reg, i] -> void {
            std::cout << std::format("Register {} is allocating...\n", i);
            simulate_register_alloc_stress(reg);
            std::cout << std::format("Register {} finished\n", i);

            completion_latch.count_down();
        });
    }

    completion_latch.wait();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time);
    std::cout << "Total execution time: " << duration.count() << " ms\n";
    std::cout << "Total execution time: " << duration.count() / 1000.0 << " seconds\n";

    std::cout << "Cleaning up after stress simulation\n";
    // cleaning up after simulation
    heap_manager_ref.clear_roots();
    heap_manager_ref.collect_garbage();
}

void allocators::simulate_alloc_relaxed(size_t tls_count, size_t global_count, size_t register_count){
    std::cout << "Initializing relaxed simulation\n";
    auto start_time = std::chrono::high_resolution_clock::now();

    std::latch completion_latch(tls_count + global_count + register_count);

    for(size_t i = 0; i < tls_count; ++i){
        auto tls = create_tls_root("t" + std::to_string(i));
        alloc_thread_pool.enqueue([&, tls, i] -> void {
            std::cout << std::format("TLS {} is allocating objects...\n", i);
            simulate_tls_alloc_relaxed(tls);
            std::cout << std::format("TLS {} finished\n", i);

            completion_latch.count_down();
        });
    }

    for(size_t i = 0; i < global_count; ++i){
        auto global = create_global_root("g" + std::to_string(i));
        alloc_thread_pool.enqueue([&, global, i] -> void {
            std::cout << std::format("Global {} is allocating objects...\n", i);
            simulate_global_alloc_relaxed(global);
            std::cout << std::format("Global {} finished\n", i);

            completion_latch.count_down();
        });
    }

    for(size_t i = 0; i < register_count; ++i){
        auto reg = create_register_root("r" + std::to_string(i));
        alloc_thread_pool.enqueue([&, reg, i] -> void {
            std::cout << std::format("Register {} is allocating objects...\n", i);
            simulate_register_alloc_relaxed(reg);
            std::cout << std::format("Register {} finished\n", i);

            completion_latch.count_down();
        });
    }

    completion_latch.wait();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time);
    std::cout << "Total execution time: " << duration.count() << " ms\n";
    std::cout << "Total execution time: " << duration.count() / 1000.0 << " seconds\n";

    std::cout << "Cleaning up after stress simulation\n";
    // cleaning up after simulation
    heap_manager_ref.clear_roots();
    heap_manager_ref.collect_garbage();
}

thread_local_stack* allocators::create_tls_root(std::string tls_root_key) {
    auto tls = std::make_unique<thread_local_stack>(1024);
    heap_manager_ref.add_root(tls_root_key, std::move(tls));
    return static_cast<thread_local_stack*>(heap_manager_ref.get_root(tls_root_key));
}

global_root* allocators::create_global_root(std::string global_root_key){
    auto global = std::make_unique<global_root>(nullptr);
    heap_manager_ref.add_root(global_root_key, std::move(global));
    return static_cast<global_root*>(heap_manager_ref.get_root(global_root_key));
}

register_root* allocators::create_register_root(std::string register_root_key){
    auto reg = std::make_unique<register_root>(nullptr);
    heap_manager_ref.add_root(register_root_key, std::move(reg));
    return static_cast<register_root*>(heap_manager_ref.get_root(register_root_key));
}

void allocators::simulate_tls_alloc_stress(thread_local_stack* tls){
    for(size_t stack_count = 0; stack_count < TLS_STACK_COUNT_STRESS; ++stack_count){
        tls->push_scope();

        for(size_t i = 1; i < TLS_ALLOC_STRESS_THRESHOLD_PER_STACK; ++i){
            const uint32_t bytes{generate_random_size()};
            header* obj = heap_manager_ref.allocate(bytes);

            std::string var_name = std::to_string(stack_count) + "_" + std::to_string(i);
            tls->init(std::move(var_name), obj);
        }

        tls->pop_scope();
    }
}

void allocators::simulate_tls_alloc_relaxed(thread_local_stack* tls){
    for(size_t stack_count = 0; stack_count < TLS_STACK_COUNT_RELAXED; ++stack_count){
        tls->push_scope();

        for(size_t i = 1; i < TLS_ALLOC_RELAXED_THRESHOLD_PER_STACK; ++i){
            const uint32_t bytes{generate_random_size()};
            header* obj = heap_manager_ref.allocate(bytes);

            std::string var_name = std::to_string(stack_count) + "_" + std::to_string(i);
            tls->init(std::move(var_name), obj);
        }

        tls->pop_scope();
    }
}

void allocators::simulate_global_alloc_stress(global_root* global){
    for(size_t i = 0; i < GLOBAL_ALLOC_STRESS_THRESHOLD; ++i){
        global->set_global_variable(i & 1 ? nullptr : heap_manager_ref.allocate(generate_random_size()));
    }
}

void allocators::simulate_global_alloc_relaxed(global_root* global){
    for(size_t i = 0; i < GLOBAL_ALLOC_RELAXED_THRESHOLD; ++i){
        global->set_global_variable(i & 1 ? nullptr : heap_manager_ref.allocate(generate_random_size()));
    }
}

void allocators::simulate_register_alloc_stress(register_root* reg){
    for(size_t i = 0; i < REGISTER_ALLOC_STRESS_THRESHOLD; ++i){
        reg->set_register_variable(i & 1 ? nullptr : heap_manager_ref.allocate(generate_random_size()));
    }
}

void allocators::simulate_register_alloc_relaxed(register_root* reg){
    for(size_t i = 0; i < REGISTER_ALLOC_RELAXED_THRESHOLD; ++i){
        reg->set_register_variable(i & 1 ? nullptr : heap_manager_ref.allocate(generate_random_size()));
    }
}

uint32_t allocators::generate_random_size() {
    int category = category_dist(rng) % 100;

    if (category < 80) {
        return rng() % SMALL_OBJECT_THRESHOLD;
    } else if (category < 99) {
        return SMALL_OBJECT_THRESHOLD + (rng() % (MEDIUM_OBJECT_THRESHOLD - SMALL_OBJECT_THRESHOLD));
    } else {
        return MEDIUM_OBJECT_THRESHOLD + (rng() % (LARGE_OBJECT_THRESHOLD - MEDIUM_OBJECT_THRESHOLD));
    }
}