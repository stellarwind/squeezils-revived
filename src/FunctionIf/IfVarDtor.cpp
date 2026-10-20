// Destructors of the IfVariable classes (SQZ.EXE 0x4464b0, 0x446520, 0x446700): defined inline in the original class, so the
// plain and the scalar-deleting copies both carry the body. Kept in their own unit (classes renamed with a D suffix, rest of IfVar.cpp
// included for the same symbol context) because the inline definitions change the register choices of Parse in IfVar.cpp.
#define IFVAR_DTORS
#define IfVariableFunction IfVariableFunctionD
#define IfVariableValueFunction IfVariableValueFunctionD
#define IfVariableEqualFunction IfVariableEqualFunctionD
#define IfVariableLesserFunction IfVariableLesserFunctionD
#define IfVariableGreaterFunction IfVariableGreaterFunctionD
#include "FunctionIf/IfVar.cpp"
void ForceIfVarDtors(IfVariableFunction *a, IfVariableValueFunction *b)
{
    delete a;
    delete b;
}
