#include "register-root.hpp"

register_root::register_root(header* var_ptr) : register_variable{ var_ptr } {}

header* register_root::get_register_variable() noexcept {
    return register_variable;
}

const header* register_root::get_register_variable() const noexcept {
    return register_variable;
}

void register_root::set_register_variable(header* var_ptr) noexcept {
    register_variable = var_ptr;
}

void register_root::accept(gc_visitor& visitor) noexcept {
    visitor.visit(*this);
}