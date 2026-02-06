#include "thread-local-stack.hpp"

#include <stdexcept>
#include <utility>

thread_local_stack::thread_local_stack() : scope(1) {}

thread_local_stack::thread_local_stack(size_t hash_map_capacity) : scope(1), var_to_idx(hash_map_capacity) {}

thread_local_stack::~thread_local_stack() {
    pop_scope(true);
}

thread_local_stack::thread_local_stack(thread_local_stack&& other) noexcept : scope(std::exchange(other.scope, 0)),
    thread_stack(std::move(other.thread_stack)), var_to_idx(std::move(other.var_to_idx)) {}

thread_local_stack& thread_local_stack::operator=(thread_local_stack&& other) noexcept {
    if(this != &other){
        scope = std::exchange(other.scope, 0);
        thread_stack = std::move(other.thread_stack);
        var_to_idx = std::move(other.var_to_idx);
    }
    return *this;
}

void thread_local_stack::init(std::string variable_name, header* heap_ptr){
    if(var_to_idx.contains(variable_name)){
        throw std::invalid_argument("Variable already exists");
    }
    var_to_idx.insert(variable_name, thread_stack.get_size());
    thread_stack.push(thread_local_stack_entry{std::move(variable_name), scope, heap_ptr});
}

void thread_local_stack::reassign_ref(const std::string& variable_name, header* new_ref_to){
    if(!var_to_idx.contains(variable_name)){
        throw std::invalid_argument("Variable doesn't exist");
    }
    size_t idx = var_to_idx[variable_name];
    thread_stack[idx].ref_to = new_ref_to;
}

void thread_local_stack::remove_ref(const std::string& variable_name){
    if(!var_to_idx.contains(variable_name)){
        throw std::invalid_argument("Variable doesn't exist");
    }
    size_t idx = var_to_idx[variable_name];
    thread_stack[idx].ref_to = nullptr;
}

void thread_local_stack::push_scope() noexcept {
    ++scope;
}

void thread_local_stack::pop_scope(bool destr){
    if((scope <= 1 && !destr) || scope == 0){
        return;
    }

    while(!thread_stack.empty() && thread_stack.peek().scope == scope){
        var_to_idx.erase(thread_stack.peek().variable_name);
        thread_stack.pop();
    }
    --scope;
}

indexed_stack<thread_local_stack_entry>& thread_local_stack::get_thread_stack() noexcept {
    return thread_stack;
}

const indexed_stack<thread_local_stack_entry>& thread_local_stack::get_thread_stack() const noexcept {
    return thread_stack;
}

const thread_local_stack_entry* thread_local_stack::begin() const noexcept {
    return thread_stack.begin();
}

const thread_local_stack_entry* thread_local_stack::end() const noexcept {
    return thread_stack.end();
}

void thread_local_stack::accept(gc_visitor& visitor) noexcept {
    visitor.visit(*this);
}