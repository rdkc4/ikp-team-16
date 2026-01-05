#ifndef INDEXED_STACK_HPP
#define INDEXED_STACK_HPP

#include <cassert>
#include <cstddef>
#include <new>
#include <utility>

/// initial stack capacity.
constexpr size_t DEFAULT_STACK_CAPACITY = 8;

/**
 * @class indexed_stack
 * @brief hybrid implementation of an array and stack.
 * T - type of the data stored on the stack.
*/
template<typename T>
class indexed_stack {
private:
    /// pointer to the bottom element of the stack.
    T* data;
    /// number of the elements on the stack.
    size_t size;
    /// allocated space for the stack.
    size_t capacity;

    /** 
     * @brief frees elements on the stack that are in certain range.
     * @param start - starting index.
     * @param end - ending index (excluded).
     * @throws assert when start index is bigger than end, or when end is bigger than size. 
    */
    void delete_range(size_t start, size_t end) {
        assert(start <= end && end <= size);
        for(size_t i = start; i < end; ++i){
            data[i].~T();
        }
    }

    /** 
     * @brief updates the capacity of the stack based on a size.
     * @param new_capacity - new capacity of the stack, can't go below DEFAULT_STACK_CAPACITY.
     * @throws std::bad_alloc when ::operator new fails to allocate memory.
     * @throws assert if new_capacity is below DEFAULT_STACK_CAPACITY
    */
    void resize(size_t new_capacity) {
        assert(new_capacity >= DEFAULT_STACK_CAPACITY);
        T* new_data = static_cast<T*>(::operator new(sizeof(T) * new_capacity));
        
        size_t i = 0;
        try {
            for(; i < size; ++i){
                new (new_data + i) T(std::move(data[i]));
            }
        } 
        catch (...) {
            for(size_t j = 0; j < i; ++j){
                new_data[j].~T();
            }
            ::operator delete(new_data);
            throw;
        }

        delete_range(0, size);
        ::operator delete(data);

        data = new_data;
        capacity = new_capacity;
    }

public:
    /** 
     * @brief creates the instance of the indexed_stack.
     * @details preallocates the memory for DEFAULT_STACK_CAPACITY elements; default size 0.
    */
    indexed_stack() : 
        data(static_cast<T*>(::operator new(sizeof(T) * DEFAULT_STACK_CAPACITY))), 
        size(0),
        capacity(DEFAULT_STACK_CAPACITY) {}

    /** 
     * @brief destroys the indexed_stack object.
     * @details frees all elements on the stack, frees the memory allocated for the stack.
    */
    ~indexed_stack() {
        delete_range(0, size);
        ::operator delete(data);
    }

    /// deleted copy constructor.
    indexed_stack(const indexed_stack&) = delete;

    /// deleted assignment constructor.
    indexed_stack& operator=(const indexed_stack&) = delete;

    /** 
     * @brief constructs new indexed_stack from an existing one.
     * @details moves ownership of the data, size and capacity from other to this.
    */
    indexed_stack(indexed_stack&& other) noexcept : 
        data(std::exchange(other.data, nullptr)), 
        size(std::exchange(other.size, 0)), 
        capacity(std::exchange(other.capacity, 0)) {}
    
    /** 
     * @brief constructs new indexed_stack by assigning it an existing one.
     * @details moves ownership of the data, size and capacity from other to this.
    */
    indexed_stack& operator=(indexed_stack&& other) noexcept {
        if (this != &other) {
            delete_range(0, size);
            ::operator delete(data);

            data = std::exchange(other.data, nullptr);
            size = std::exchange(other.size, 0);
            capacity = std::exchange(other.capacity, 0);
        }
        return *this;
    }
    
    /** 
     * @brief pushes new value to the top of the stack.
     * @param value - const reference object being added to the stack.
     * @details if size reaches capacity, capacity is doubled.
    */
    void push(const T& value) {
        if (size == capacity){
            resize(capacity << 1);
        }
        new (data + size) T(value);
        ++size;
    }

    /** 
     * @brief pushes new value to the top of the stack.
     * @param value - object being moved to the stack.
     * @details if size reaches capacity, capacity is doubled.
    */
    void push(T&& value) {
        if(size == capacity) {
            resize(capacity << 1);
        }
        new (data + size) T(std::move(value));
        ++size;
    }

    /** 
     * @brief removes the element from the top of the stack.
     * @details if 75% of capacity is free, stack is reallocated and capacity is half the previous capacity;
     * doesn't go below DEFAULT_STACK_CAPACITY.
     * @throws assert when stack is empty.
    */
    void pop() {
        assert(size > 0);
        data[--size].~T();

        if(size <= capacity >> 2 && capacity >> 1 >= DEFAULT_STACK_CAPACITY){
            resize(capacity >> 1);
        }
    }

    /** 
     * @brief getter for the element on the top of the stack.
     * @returns the reference to the top element on the stack.
     * @throws assert when stack is empty.
    */
    T& peek() noexcept {
        assert(size > 0);
        return data[size - 1];
    }

    /** 
     * @brief getter for the element on the top of the stack.
     * @returns the const reference to the top element on the stack.
     * @throws assert when stack is empty.
    */
    const T& peek() const noexcept {
        assert(size > 0);
        return data[size - 1];
    }

    /** 
     * @brief checks if the stack is empty.
     * @returns true if stack is empty; false otherwise.
    */
    bool empty() const noexcept {
        return size == 0;
    }

    /** 
     * @brief operator for direct indexing.
     * @param i - position of the element on the stack; calculated from the bottom.
     * @returns reference to an element on the stack.
     * @throws assert when index is bigger than or equal to size.
    */
    T& operator[](size_t i) noexcept {
        assert(i < size);
        return data[i];
    }

    /** 
     * @brief operator for direct indexing.
     * @param i - position of the element on the stack; calculated from the bottom.
     * @returns const reference to an element on the stack.
     * @throws assert when index is bigger than or equal to size.
    */    
    const T& operator[](size_t i) const noexcept {
        assert(i < size);
        return data[i];
    }

    /** 
     * @brief getter for the size of the stack.
     * @returns number of elements on the stack.
    */
    size_t get_size() const noexcept {
        return size;
    }

    /** 
     * @brief getter for the bottom element of the stack.
     * @returns the pointer to the bottom element of the stack.
    */
    T* begin() { return data; }

    /** 
     * @brief getter for the end of the stack.
     * @returns the pointer to the end of the stack.
    */
    T* end() { return data + size; }

    /** 
     * @brief getter for the bottom element of the stack.
     * @returns the const pointer to the bottom element of the stack.
    */
    const T* begin() const { return data; }

    /** 
     * @brief getter for the end of the stack.
     * @returns the const pointer to the end of the stack.
    */
    const T* end() const { return data + size; }
};

#endif