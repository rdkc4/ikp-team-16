#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <cstddef>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

#include "../queue/queue.hpp"

/** 
 * @class thread_pool
 * @brief manages a fixed number of worker threads that execute queued tasks.
*/
class thread_pool {
private:
    /// mutex for concurrent access to tasks.
    std::mutex mtx;

    /// condition variable for waiting/notifying.
    std::condition_variable cv;

    /// flag that handles stoppage of worker threads.
    bool stop;

    /// queue of tasks.
    queue<std::function<void()>> tasks;

    /// list of threads owned by thread pool.
    std::thread* threads;
    
    /// number of threads in a thread pool.
    size_t thread_count;

    /**
     * @brief threads execute tasks from the queue.
    */
    void worker();

public:
    /** 
     * @brief starts specified number of worker threads.
     * @param thread_count - number of worker threads that handle tasks.
    */
    thread_pool(size_t thread_count);

    /** 
     * @brief stopping all worker threads.
    */
    ~thread_pool();

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