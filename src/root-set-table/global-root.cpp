#include "global-root.hpp"

global_root::global_root(header* var_ptr) : global_variable_ptr{ var_ptr } {}

header* global_root::get_global_variable() noexcept {
    return global_variable_ptr;
}

const header* global_root::get_global_variable() const noexcept {
    return global_variable_ptr;
}

void global_root::set_global_variable(header* var_ptr) noexcept {
    global_variable_ptr = var_ptr;
}

void global_root::accept(gc_visitor& visitor) noexcept {
    visitor.visit(*this);
}