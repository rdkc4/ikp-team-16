#include "register-root.hpp"

#include <mutex>

register_root::register_root(header* var_ptr) : register_variable{ var_ptr } {}

void register_root::set_register_variable(header* var_ptr) noexcept {
    std::lock_guard<std::mutex> register_lock(register_mutex);
    register_variable = var_ptr;
}

void register_root::accept(gc_visitor& visitor) noexcept {
    std::lock_guard<std::mutex> register_lock(register_mutex);
    visitor.visit(*this);
}

header* register_root::get_register_variable_unlocked() noexcept {
    return register_variable;
}