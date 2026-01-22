#ifndef ROOT_SET_BASE_HPP
#define ROOT_SET_BASE_HPP

#include "../gc/gc-visitor.hpp"

/**
 * @class root_set_base
 * @brief parent class of root-set table entries.
*/
class root_set_base {
public:
    /**
     * @brief deletes the root_set_base object.
    */
    virtual ~root_set_base() = default;
    
    /**
     * @brief abstract method for accepting visitor.
     * @param visitor - reference to a gc visitor.
     * @returns void
    */
    virtual void accept(gc_visitor& visitor) noexcept = 0;
};

#endif