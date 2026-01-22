#ifndef REGISTER_ROOT_HPP
#define REGISTER_ROOT_HPP

#include "../common/header/header.hpp"
#include "../common/root-set/root-set-base.hpp"
#include "../common/gc/gc-visitor.hpp"

/**
 * @class register_root
 * @brief defines the structure of the register variable in the root-set table.
 * Inherits from the root_set_base.
*/
class register_root final : public root_set_base {
private:
    /// pointer to a header of the register variable on the heap.
    header* register_variable;

public:
    /**
     * @brief creates the instance of the register variable
     * @param var_ptr - pointer to a header of the register variable on the heap.
    */
    register_root(header* var_ptr) : register_variable{ var_ptr } {}

    /**
     * @brief getter for the register variable
     * @returns pointer to a header of the register variable on the heap.
    */
    header* get_register_variable() noexcept {
        return register_variable;
    }

    /**
     * @brief getter for the register variable
     * @returns const pointer to a header of the register variable on the heap.
    */
    const header* get_register_variable() const noexcept {
        return register_variable;
    }

    /**
     * @brief setter for the register variable
     * @param var_ptr - pointer to a header of the variable on the heap.
     * @returns void
    */
    void set_register_variable(header* var_ptr) noexcept {
        register_variable = var_ptr;
    }

    /**
     * @brief accepts the gc visitor.
     * Calls marking on the gc visitor for register variable element.
    */
    virtual void accept(gc_visitor& visitor) noexcept override {
        visitor.visit(*this);
    }
};

#endif