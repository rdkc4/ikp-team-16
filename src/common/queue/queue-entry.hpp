#ifndef QUEUE_ENTRY_HPP
#define QUEUE_ENTRY_HPP

#include <utility>
#include <type_traits>

/**
 * @struct queue_entry
 * @brief element of the queue.
 * @tparam T - type of the element.
*/
template<typename T>
struct queue_entry {
    /// value of the element in the queue.
    T value;
    
    /// pointer to the next element in the queue.
    queue_entry* next;

    /**
     * @brief creates the instance of the queue element.
     * @tparam TT - type of element in the queue.
     * @param value - value of the element in the queue.
    */
    template<typename TT>
    requires std::is_constructible_v<T, TT&&>
    queue_entry(TT&& value) : value(std::forward<TT>(value)), next(nullptr) {}
};

#endif