#include "thread-pool.hpp"

#include <new>

thread_pool::thread_pool(size_t thread_count) : threads(nullptr), thread_count(thread_count), stop(false) {
    if (thread_count == 0) {
        throw std::invalid_argument("Thread count must be greater than zero");
    }

    threads = static_cast<std::thread*>(
        ::operator new(sizeof(std::thread) * thread_count)
    );

    size_t i = 0;
    try {
        for (; i < thread_count; ++i) {
            new (threads + i) std::thread([this] { worker(); });
        }
    } catch (...) {
        for (size_t j = 0; j < i; ++j) {
            threads[j].join();
            threads[j].~thread();
        }
        ::operator delete(threads);
        throw;
    }
}

thread_pool::~thread_pool() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        stop = true;
    }
    cv.notify_all();

    for (size_t i = 0; i < thread_count; ++i) {
        if (threads[i].joinable()) {
            threads[i].join();
        }
        threads[i].~thread();
    }

    ::operator delete(threads);
}

void thread_pool::worker() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] {
                return stop || !tasks.empty();
            });

            if (stop && tasks.empty()) {
                return;
            }

            task = tasks.pop();
        }
        task();
    }
}