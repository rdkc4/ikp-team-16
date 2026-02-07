#ifndef GLOBAL_ROOT_HPP
#define GLOBAL_ROOT_HPP

#include <mutex>

#include "../common/header/header.hpp"
#include "../common/root-set/root-set-base.hpp"
#include "../common/gc/gc-visitor.hpp"

/**
 * @class global_root
 * @brief defines the structure of the global variable in the root-set table.
 * Inherits from root_set_base.
*/
class global_root final : public root_set_base {
private:
    /// used for global variable synchronization.
    mutable std::mutex global_mutex;

    /// pointer to a header of the variable on the heap
    header* global_variable_ptr;

    /**
     * @brief getter for the variable.
     * @warning must be called when lock is held already.
     * @returns pointer to a header of the variable.
    */
    header* get_global_variable_unlocked() noexcept;

    /// allowing gc to access getter for the variable.
    friend class garbage_collector;

public:
    /**
     * @brief creates the instance of the global variable
     * @param var_ptr - pointer to a header of the global variable on the heap
    */
    global_root(header* var_ptr);

    /**
     * @brief setter for the global variable.
     * @param var_ptr - pointer to the header of the global variable on the heap
    */
    void set_global_variable(header* var_ptr) noexcept;
    
    /**
     * @brief accepts the gc visitor.
     * @param visitor - reference to a gc visitor.
     * Calls marking on the gc visitor for global variable element.
    */
    virtual void accept(gc_visitor& visitor) noexcept override;

};

#endif