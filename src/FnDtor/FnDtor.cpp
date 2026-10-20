// Function-derived classes whose inline destructor deletes a polymorphic pointer member.
#include "compat41.h"

struct Function {
    virtual ~Function();
    virtual void vf1() = 0;
};
Function::~Function() {}   // out-of-line trivial: mov [ecx],vtbl ; ret

struct IfFn : Function {
    Function *field_4;
    virtual void vf1();
    virtual ~IfFn() { delete field_4; }
};

IfFn *ForceIfFn()
{
    return new IfFn;
}
