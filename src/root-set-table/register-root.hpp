#ifndef REGISTER_ROOT_HPP
#define REGISTER_ROOT_HPP

#include "../common/header/header.hpp"
#include "../common/root-set/root-set-base.hpp"
#include "../common/gc/gc-visitor.hpp"

class register_root final : public root_set_base {
private:
    header* register_variable;

public:
    register_root(header* var_ptr) : register_variable{ var_ptr } {}

    header* get_register_variable() noexcept {
        return register_variable;
    }

    const header* get_register_variable() const noexcept {
        return register_variable;
    }

    void set_register_variable(header* var_ptr) noexcept {
        register_variable = var_ptr;
    }

    virtual void accept(gc_visitor& visitor) noexcept override {
        visitor.visit(*this);
    }
};

#endif