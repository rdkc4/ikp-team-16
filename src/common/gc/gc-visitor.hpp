#ifndef GC_VISITOR_HPP
#define GC_VISITOR_HPP

class thread_local_stack;
class global_root;
class register_root;

class gc_visitor {
public:
    virtual ~gc_visitor() = default;
    
    virtual void visit(thread_local_stack& stack) = 0;
    virtual void visit(global_root& global) = 0;
    virtual void visit(register_root& reg) = 0;
};

#endif