#ifndef GC_VISITOR_HPP
#define GC_VISITOR_HPP

class thread_local_stack;
class global_root;
class register_root;

/**
 * @class gc_visitor
 * @brief interface for GC's marking phase.
*/
class gc_visitor {
public:
    /**
     * @brief deletes the gc_visitor object
    */
    virtual ~gc_visitor() = default;
    
    /**
     * @brief virtual function for marking elements of a thread stack.
     * @param stack - reference to a thread local stack.
     * @returns void
    */
    virtual void visit(thread_local_stack& stack) = 0;

    /**
     * @brief virtual function for marking elements of a global root.
     * @param global - reference to a global variable.
     * @returns void
    */
    virtual void visit(global_root& global) = 0;

    /**
     * @brief virtual function for marking elements of a register root
     * @param reg - reference to a register variable.
     * @returns void
    */
    virtual void visit(register_root& reg) = 0;
};

#endif