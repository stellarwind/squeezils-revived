// Function: root of the scripting-function hierarchy (SQZ.EXE vtbl 0x49dabc).
// Porting the leaf ctor/dtor of the base and the small derived ctors first.
#pragma once
#include "compat41.h"

class Function {
public:
    Function() {}
    virtual ~Function() {}
    virtual void vf1() = 0;
    virtual void vf2() = 0;
    virtual void vf3() = 0;
    virtual void vf4() = 0;
};

// 0x49e8e4 (0x444770): field_4 only.
class ExecuteFunction : public Function {
public:
    ExecuteFunction();
    virtual void vf1();
    virtual void vf2();
    virtual void vf3();
    virtual void vf4();
    int field_4;
};

// 0x49e874 (0x4459e0): field_4 only.
class IfFunction : public Function {
public:
    IfFunction();
    virtual void vf1();
    virtual void vf2();
    virtual void vf3();
    virtual void vf4();
    int field_4;
};

// 0x49e914 (0x444ca0): field_4 and field_8.
class DelayedExecuteFunction : public Function {
public:
    DelayedExecuteFunction();
    virtual void vf1();
    virtual void vf2();
    virtual void vf3();
    virtual void vf4();
    int field_4;
    int field_8;
};

// 0x4a0e3c (0x46f800, CreatePropFunction): field_4, field_8 and field_C.
class Fnc3Field : public Function {
public:
    Fnc3Field();
    virtual void vf1();
    virtual void vf2();
    virtual void vf3();
    virtual void vf4();
    int field_4;
    int field_8;
    int field_C;
};
