// Function-derived class whose inline dtor deletes a polymorphic pointer member (field_8)
// and has a refcounted String member (field_4) destroyed implicitly.
#include "compat41.h"

struct Function {
    virtual ~Function();
    virtual void vf1() = 0;
};
Function::~Function() {}

struct String {
    void *rep;
    ~String();
};

struct ExecWithPropFn : Function {
    String field_4;     // +4
    Function *field_8;  // +8
    virtual void vf1();
    virtual ~ExecWithPropFn() { delete field_8; }
};

ExecWithPropFn *ForceEwp()
{
    return new ExecWithPropFn;
}
