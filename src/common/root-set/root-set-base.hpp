#ifndef ROOT_SET_BASE_HPP
#define ROOT_SET_BASE_HPP

#include "../gc/gc-visitor.hpp"

class root_set_base {
public:
    virtual ~root_set_base() = default;
    
    virtual void accept(gc_visitor& visitor) noexcept = 0;
};

#endif