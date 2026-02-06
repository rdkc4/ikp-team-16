#ifndef THREAD_LOCAL_STACK_HPP
#define THREAD_LOCAL_STACK_HPP

#include <string>

#include "../common/indexed-stack/indexed-stack.hpp"
#include "../common/hash-map/hash-map.hpp"
#include "../common/header/header.hpp"
#include "../common/root-set/root-set-base.hpp"
#include "../common/gc/gc-visitor.hpp"
#include "../common/root-set/thread-local-stack-entry.hpp"

/**
 * @class thread_local_stack
 * @brief local memory of a thread.
*/
class thread_local_stack final : public root_set_base {
private:
    /// id of the last pushed scope.
    size_t scope;

    /// stack of initialized variables.
    indexed_stack<thread_local_stack_entry> thread_stack;

    /// mapping the variable name to its index inside of the thread_stack.
    hash_map<std::string, size_t> var_to_idx;

public:
    /**
     * @brief creates the instance of the thread_local_stack.
     * @details scope defaults to 1.
    */
    thread_local_stack();
    
    /**
     * @brief creates the instance of the thread_local_stack.
     * @param hash_map_capacity - number of buckets in the hash_map.
     * @details scope defaults to 1.
    */
    thread_local_stack(size_t hash_map_capacity);

    /**
     * @brief deletes the thread_local_stack.
     * @details frees final scope if not freed manually.
    */
    ~thread_local_stack();

    /// deleted copy constructor.
    thread_local_stack(const thread_local_stack&) = delete;

    /// deleted assignment operator.
    thread_local_stack& operator=(const thread_local_stack&) = delete;

    /**
     * @brief constructs the thread_local_stack instance from an existing one.
     * @param other - rvalue of the existing thread_local_stack.
     * @details moves the ownership of the scope, thread_stack and var_to_idx map from other to this.
    */
    thread_local_stack(thread_local_stack&& other) noexcept;

    /**
     * @brief constructs new thread_local_stack by assigning it an existing one.
     * @param other - rvalue of the existing thread_local_stack.
     * @details moves the ownership of the scope, thread_stack and var_to_idx map from other to this.
    */
    thread_local_stack& operator=(thread_local_stack&& other) noexcept;

    /**
     * @brief initializes new variable.
     * @param variable_name - name of the variable.
     * @param heap_ptr - pointer to the value of the variable on the heap.
     * @throws std::invalid_argument when variable already exists.
    */
    void init(std::string variable_name, header* heap_ptr = nullptr);

    /**
     * @brief assigns new value to a variable.
     * @param variable_name - name of the variable.
     * @param new_ref_to - pointer to a new value on the heap.
     * @throws std::invalid_argument when variable_name is not previously initialized.
    */
    void reassign_ref(const std::string& variable_name, header* new_ref_to);

    /**
     * @brief removes the reference to a value on the heap.
     * @param variable_name - name of the variable.
     * @throws std::invalid_argument when variable_name is not previously initialized.
    */
    void remove_ref(const std::string& variable_name);

    /**
     * @brief simulates entering new scope.
     * @note simulation purposes.
    */
    void push_scope() noexcept;

    /**
     * @brief simulates exiting scope.
     * @param destr - flag if pop_scope is called by destructor, defaults to false.
     * @note simulation purposes.
    */
    void pop_scope(bool destr = false);

    /**
     * @brief getter for the thread_stack.
     * @returns reference to thread_stack.
    */
    indexed_stack<thread_local_stack_entry>& get_thread_stack() noexcept;

    /**
     * @brief getter for the thread_stack.
     * @returns const reference to thread_stack.
    */
    const indexed_stack<thread_local_stack_entry>& get_thread_stack() const noexcept;

    /** 
     * @brief getter for the bottom element of the stack.
     * @returns the const pointer to the bottom element of the stack.
    */
    const thread_local_stack_entry* begin() const noexcept;

    /** 
     * @brief getter for the end of the stack.
     * @returns the const pointer to the end of the stack.
    */
    const thread_local_stack_entry* end() const noexcept;

    /**
     * @brief accepts the gc visitor.
     * @param visitor - reference to a gc visitor.
     * Calls marking on the gc visitor for thread-local elements.
    */
    virtual void accept(gc_visitor& visitor) noexcept override;

};

#endif