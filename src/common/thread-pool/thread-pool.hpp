#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <cstddef>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <new>
#include <stdexcept>

#include "../queue/queue.hpp"

/** 
 * @class thread_pool
 * @brief manages a fixed number of worker threads that execute queued tasks.
*/
class thread_pool {
private:
    /// list of threads owned by thread pool.
    std::thread* threads;
    /// number of threads in a thread pool.
    size_t thread_count;
    /// queue of tasks.
    queue<std::function<void()>> tasks;
    /// mutex for concurrent access to tasks.
    std::mutex mtx;
    /// condition variable for waiting/notifying.
    std::condition_variable cv;
    /// flag that handles stoppage of worker threads.
    bool stop;

    void worker() {
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

public:
    /** 
     * @brief starts specified number of worker threads.
     * @param thread_count - number of worker threads that handle tasks.
    */
    thread_pool(size_t thread_count) : threads(nullptr), thread_count(thread_count), stop(false) {
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

    /** 
     * @brief stopping all worker threads.
    */
    ~thread_pool() {
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

    /** 
     * @brief adding new task to the queue
     * @param f - task that is being added to queue.
     * @throws std::runtime_error - when called after all worker threads stopped working.
    */
    template<typename T>
    void enqueue(T&& f) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            if(stop){
                throw std::runtime_error("Enqueue on stopped thread");
            }
            tasks.push(std::forward<T>(f));
        }

        cv.notify_one();
    }

};

#endif