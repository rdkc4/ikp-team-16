#ifndef REGISTER_ROOT_HPP
#define REGISTER_ROOT_HPP

#include <mutex>

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
    /// used for register synchronization.
    mutable std::mutex register_mutex;

    /// pointer to a header of the register variable on the heap.
    header* register_variable;

    /**
     * @brief getter for the variable.
     * @warning must be called when lock is held already.
     * @returns pointer to a header of the variable.
    */
    header* get_register_variable_unlocked() noexcept;

    /// allowing gc to access getter for the variable.
    friend class garbage_collector;

public:
    /**
     * @brief creates the instance of the register variable
     * @param var_ptr - pointer to a header of the register variable on the heap.
    */
    register_root(header* var_ptr);

    /**
     * @brief setter for the register variable
     * @param var_ptr - pointer to a header of the variable on the heap.
     * @returns void
    */
    void set_register_variable(header* var_ptr) noexcept;

    /**
     * @brief accepts the gc visitor.
     * @param visitor - reference to a gc visitor.
     * Calls marking on the gc visitor for register variable element.
    */
    virtual void accept(gc_visitor& visitor) noexcept override;

};

#endif