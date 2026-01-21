#ifndef GLOBAL_ROOT_HPP
#define GLOBAL_ROOT_HPP

#include "../common/header/header.hpp"
#include "../common/root-set/root-set-base.hpp"
#include "../common/gc/gc-visitor.hpp"

class global_root final : public root_set_base {
private:
    header* global_variable_ptr;

public:
    global_root(header* var_ptr) : global_variable_ptr{ var_ptr } {}

    header* get_global_variable() noexcept {
        return global_variable_ptr;
    }

    const header* get_global_variable() const noexcept {
        return global_variable_ptr;
    }

    void set_global_variable(header* var_ptr) noexcept {
        global_variable_ptr = var_ptr;
    }

    virtual void accept(gc_visitor& visitor) noexcept {
        visitor.visit(*this);
    }

};

#endif