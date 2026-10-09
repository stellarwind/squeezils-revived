// Function hierarchy: base dtor + the small derived ctors.
#include "Function/Function.h"

Function::~Function()
{
}

void Function::vf1() {}
void Function::vf2() {}
void Function::vf3() {}
void Function::vf4() {}

ExecuteFunction::ExecuteFunction()
{
    field_4 = 0;
}

void ExecuteFunction::vf1() {}
void ExecuteFunction::vf2() {}
void ExecuteFunction::vf3() {}
void ExecuteFunction::vf4() {}

IfFunction::IfFunction()
{
    field_4 = 0;
}

void IfFunction::vf1() {}
void IfFunction::vf2() {}
void IfFunction::vf3() {}
void IfFunction::vf4() {}

DelayedExecuteFunction::DelayedExecuteFunction()
{
    field_4 = 0;
    field_8 = 0;
}

void DelayedExecuteFunction::vf1() {}
void DelayedExecuteFunction::vf2() {}
void DelayedExecuteFunction::vf3() {}
void DelayedExecuteFunction::vf4() {}
