#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <cstddef>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "./queue-entry.hpp"

/**
 * @class queue
 * @brief implementation of the queue.
 * @tparam T - type of the elements in the queue.
*/
template<typename T>
class queue {
private:
    using queue_entry = queue_entry<T>;
    /// pointer to the front of the queue.
    queue_entry* head;
    /// pointer to the back of the queue.
    queue_entry* tail;
    /// size of the queue.
    size_t size;

public:
    /**
     * @brief creates the instance of the queue.
    */
    queue() : head(nullptr), tail(nullptr), size(0) {}

    /**
     * @brief deletes the instance of the queue.
     * @details frees all the elements from the queue.
    */
    ~queue() {
        clear();
    }

    /// deleted copy constructor.
    queue(const queue&) = delete;
    /// deleted assignment operator.
    queue& operator=(const queue&) = delete;

    /**
     * @brief creates the instance of the queue from an existing queue.
     * @param other - rvalue of the existing queue.
     */
    queue(queue&& other) noexcept : 
        head(std::exchange(other.head, nullptr)),
        tail(std::exchange(other.tail, nullptr)),
        size(std::exchange(other.size, 0)) {}

    /**
     * @brief assigns the instance of the queue from an existing queue.
     * @param other - rvalue of the existing queue.
    */
    queue& operator=(queue&& other) noexcept {
        if(this != &other){
            clear();
            head = std::exchange(other.head, nullptr);
            tail = std::exchange(other.tail, nullptr);
            size = std::exchange(other.size, 0);
        }

        return *this;
    }

    /**
     * @brief enqueues an element at the end of the queue.
     * @tparam TT - type of the element.
     * @param value - value of the element.
    */
    template<typename TT>
    requires std::is_constructible_v<T, TT&&>
    void push(TT&& value){
        queue_entry* new_element = static_cast<queue_entry*>(::operator new(sizeof(queue_entry)));
        try {
            new (new_element) queue_entry(std::forward<TT>(value));
        } catch (...) {
            ::operator delete(new_element);
            throw;
        }

        if(!tail){
            head = tail = new_element;
        }
        else {
            tail->next = new_element;
            tail = new_element;
        }
        ++size;
    }

    /**
     * @brief pops the element from the front of the queue.
     * @returns T - the value of the popped element.
     * @throws std::out_of_range if queue is empty.
    */
    T pop(){
        if(!head){
            throw std::out_of_range("Queue is empty");
        }

        queue_entry* old_head = head;
        T value = std::move(old_head->value);
        
        head = head->next;
        if(!head){
            tail = nullptr;
        }

        old_head->~queue_entry();
        ::operator delete(old_head);

        --size;
        return value;
    }

    /**
     * @brief peeks at the front element.
     * @returns reference to the front element.
     * @throws std::out_of_range if queue is empty.
    */
    T& front(){
        if(!head){
            throw std::out_of_range("Queue is empty");
        }
        return head->value;
    }

    /**
     * @brief peeks at the front element.
     * @returns const reference to the front element.
     * @throws std::out_of_range if queue is empty.
    */
    const T& front() const {
        if(!head){
            throw std::out_of_range("Queue is empty");
        }
        return head->value;
    }

    /**
     * @brief checks if the queue is empty.
     * @returns true if queue is empty, false otherwise.
    */
    bool empty() const noexcept {
        return size == 0;
    }

    /**
     * @brief getter for the size of the queue.
     * @returns size of the queue.
    */
    size_t get_size() const noexcept {
        return size;
    }

    /**
     * @brief clears all elements from the queue.
    */
    void clear() {
        while (head) {
            queue_entry* tmp = head;
            head = head->next;
            tmp->~queue_entry();
            ::operator delete(tmp);
        }
        tail = nullptr;
        size = 0;
    }

};

#endif