#ifndef THREAD_LOCAL_STACK_HPP
#define THREAD_LOCAL_STACK_HPP

#include <stdexcept>
#include <string>

#include "../common/indexed-stack/indexed-stack.hpp"
#include "../common/hash-map/hash-map.hpp"
#include "../common/header/header.hpp"
#include "../common/root-set/root-set-base.hpp"
#include "../common/gc/gc-visitor.hpp"

/**
 * @class thread_local_stack
 * @brief local memory of a thread.
*/
class thread_local_stack final : public root_set_base {
private:
    /**
     * @struct thread_local_stack_entry
     * @brief element of the thread_local_stack.
    */
    struct thread_local_stack_entry {
        /// name of the variable (unique).
        std::string variable_name;
        /// id of the scope in which variable was initialized (simulation purposes).
        size_t scope;
        /// pointer to the header of the block containing the data of the variable on the heap.
        header* ref_to;
    };

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
    thread_local_stack() : scope(1) {}
    
    /**
     * @brief creates the instance of the thread_local_stack.
     * @param hash_map_capacity - number of buckets in the hash_map.
     * @details scope defaults to 1.
    */
    thread_local_stack(size_t hash_map_capacity) : scope(1), var_to_idx(hash_map_capacity) {}

    /**
     * @brief deletes the thread_local_stack.
     * @details frees final scope if not freed manually.
    */
    ~thread_local_stack() {
        pop_scope(true);
    }

    /// deleted copy constructor.
    thread_local_stack(const thread_local_stack&) = delete;

    /// deleted assignment operator.
    thread_local_stack& operator=(const thread_local_stack&) = delete;

    /**
     * @brief constructs the thread_local_stack instance from an existing one.
     * @param other - rvalue of the existing thread_local_stack.
     * @details moves the ownership of the scope, thread_stack and var_to_idx map from other to this.
    */
    thread_local_stack(thread_local_stack&& other) noexcept :
        scope(std::exchange(other.scope, 0)),
        thread_stack(std::move(other.thread_stack)),
        var_to_idx(std::move(other.var_to_idx)) {}

    /**
     * @brief constructs new thread_local_stack by assigning it an existing one.
     * @param other - rvalue of the existing thread_local_stack.
     * @details moves the ownership of the scope, thread_stack and var_to_idx map from other to this.
    */
    thread_local_stack& operator=(thread_local_stack&& other) noexcept {
        if(this != &other){
            scope = std::exchange(other.scope, 0);
            thread_stack = std::move(other.thread_stack);
            var_to_idx = std::move(other.var_to_idx);
        }
        return *this;
    }

    /**
     * @brief initializes new variable.
     * @param variable_name - name of the variable.
     * @param heap_ptr - pointer to the value of the variable on the heap.
     * @throws std::invalid_argument when variable already exists.
    */
    void init(const std::string& variable_name, header* heap_ptr = nullptr){
        if(var_to_idx.contains(variable_name)){
            throw std::invalid_argument("Variable already exists");
        }
        thread_stack.push({variable_name, scope, heap_ptr});
        var_to_idx.insert(variable_name, thread_stack.get_size() - 1);
    }

    /**
     * @brief assigns new value to a variable.
     * @param variable_name - name of the variable.
     * @param new_ref_to - pointer to a new value on the heap.
     * @throws std::invalid_argument when variable_name is not previously initialized.
    */
    void reassign_ref(const std::string& variable_name, header* new_ref_to){
        if(!var_to_idx.contains(variable_name)){
            throw std::invalid_argument("Variable doesn't exist");
        }
        size_t idx = var_to_idx[variable_name];
        thread_stack[idx].ref_to = new_ref_to;
    }

    /**
     * @brief removes the reference to a value on the heap.
     * @param variable_name - name of the variable.
     * @throws std::invalid_argument when variable_name is not previously initialized.
    */
    void remove_ref(const std::string& variable_name){
        if(!var_to_idx.contains(variable_name)){
            throw std::invalid_argument("Variable doesn't exist");
        }
        size_t idx = var_to_idx[variable_name];
        thread_stack[idx].ref_to = nullptr;
    }

    /**
     * @brief simulates entering new scope.
     * @note simulation purposes.
    */
    void push_scope() noexcept {
        ++scope;
    }

    /**
     * @brief simulates exiting scope.
     * @param destr - flag if pop_scope is called by destructor, defaults to false.
     * @note simulation purposes.
    */
    void pop_scope(bool destr = false){
        if((scope <= 1 && !destr) || scope == 0){
            return;
        }

        while(!thread_stack.empty() && thread_stack.peek().scope == scope){
            var_to_idx.erase(thread_stack.peek().variable_name);
            thread_stack.pop();
        }
        --scope;
    }

    /**
     * @brief getter for the thread_stack.
     * @returns const reference to thread_stack.
    */
    const indexed_stack<thread_local_stack_entry>& get_thread_stack() const noexcept {
        return thread_stack;
    }

    /** 
     * @brief getter for the bottom element of the stack.
     * @returns the const pointer to the bottom element of the stack.
    */
    const thread_local_stack_entry* begin() const noexcept {
        return thread_stack.begin();
    }

    /** 
     * @brief getter for the end of the stack.
     * @returns the const pointer to the end of the stack.
    */
    const thread_local_stack_entry* end() const noexcept {
        return thread_stack.end();
    }

    virtual void accept(gc_visitor& visitor) noexcept override {
        visitor.visit(*this);
    }
};

#endif