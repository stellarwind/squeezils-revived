// Emit the complete-object-destructor (??1) of a Function-derived class whose inline dtor
// deletes a polymorphic pointer member, via a derived class whose dtor calls it.
#include "compat41.h"

struct Function {
    virtual ~Function();
    virtual void vf1() = 0;
};

struct IfFn : Function {
    Function *field_4;
    virtual void vf1();
    virtual ~IfFn() { delete field_4; }
};

struct IfFn2 : IfFn {
    virtual void vf1() {}
    virtual ~IfFn2() {}
};

IfFn2 *ForceIfFn2()
{
    return new IfFn2;
}
