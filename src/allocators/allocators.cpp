#include "allocators.hpp"

#include <chrono>

allocators::allocators(heap_manager& heap_manager_ref, size_t thread_count) : heap_manager_ref(heap_manager_ref), alloc_thread_pool(thread_count) {}

thread_local std::mt19937 allocators::rng{std::random_device{}() + std::hash<std::thread::id>{}(std::this_thread::get_id())};

thread_local std::uniform_int_distribution<int> allocators::category_dist(0, 99);

thread_local std::uniform_int_distribution<uint32_t> allocators::small_dist(1, SMALL_OBJECT_THRESHOLD);

thread_local std::uniform_int_distribution<uint32_t> allocators::medium_dist(SMALL_OBJECT_THRESHOLD + 1, MEDIUM_OBJECT_THRESHOLD);

thread_local std::uniform_int_distribution<uint32_t> allocators::large_dist(MEDIUM_OBJECT_THRESHOLD + 1, LARGE_OBJECT_THRESHOLD);

void allocators::simulate_alloc(size_t tls_count, size_t global_count, size_t register_count, simulation_mode mode){
    std::cout << std::format("Initializing {} simulation\n", simulation_mode_name(mode));
    const auto start_time = std::chrono::high_resolution_clock::now();

    std::latch completion_latch(tls_count + global_count + register_count);
    
    const size_t tls_scopes = tls_scope_count(mode);
    const size_t tls_allocs = tls_allocs_per_scope(mode);
    const size_t global_allocs = global_alloc_count(mode);
    const size_t reg_allocs = register_alloc_count(mode);

    for(size_t i = 0; i < tls_count; ++i){
        auto tls = create_root<thread_local_stack>("t" + std::to_string(i), 1024);
        enqueue_simulation("TLS", i, [this, tls, tls_scopes, tls_allocs] -> void {
            simulate_tls_alloc(tls, tls_scopes, tls_allocs);
        }, completion_latch);
    }

    for(size_t i = 0; i < global_count; ++i){
        auto global = create_root<global_root>("g" + std::to_string(i), nullptr);
        enqueue_simulation("Global", i, [this, global, global_allocs] -> void {
            simulate_global_alloc(global, global_allocs);
        }, completion_latch);
    }

    for(size_t i = 0; i < register_count; ++i){
        auto reg = create_root<register_root>("r" + std::to_string(i), nullptr);
        enqueue_simulation("Register", i, [this, reg, reg_allocs] -> void {
            simulate_register_alloc(reg, reg_allocs);
        }, completion_latch);
    }

    completion_latch.wait();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time);
    std::cout<< std::format("Total execution time: {} ms ({} s)\n", duration.count(), duration.count() / 1000.0);

    std::cout << "Cleaning up after simulation\n";
    heap_manager_ref.clear_roots();
    heap_manager_ref.collect_garbage();
}

void allocators::simulate_tls_alloc(thread_local_stack* tls, size_t scope_count, size_t allocs_per_scope){
    if(!tls) return;
    for(size_t scope = 0; scope < scope_count; ++scope){
        tls->push_scope();
        for(size_t i = 0; i < allocs_per_scope; ++i){
            header* obj = heap_manager_ref.allocate(generate_random_size());
            tls->init(std::to_string(scope) + "_" + std::to_string(i), obj);
        }
        tls->pop_scope();
    }
}

void allocators::simulate_global_alloc(global_root* global, size_t global_allocs){
    if(!global) return;
    for(size_t i = 0; i < global_allocs; ++i){
        global->set_global_variable(i & 1 ? nullptr : heap_manager_ref.allocate(generate_random_size()));
    }
}

void allocators::simulate_register_alloc(register_root* reg, size_t register_allocs){
    if(!reg) return;
    for(size_t i = 0; i < register_allocs; ++i){
        reg->set_register_variable(i & 1 ? nullptr : heap_manager_ref.allocate(generate_random_size()));
    }
}

uint32_t allocators::generate_random_size() {
    int category = category_dist(rng);

    if (category < 80) {
        return small_dist(rng);
    } else if (category < 99) {
        return medium_dist(rng);
    } else {
        return large_dist(rng);
    }
}