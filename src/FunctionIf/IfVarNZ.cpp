// IfVariableNonZeroFunction (vtable 0x49ea2c): true when the variable is not zero.
#include "FunctionIf/IfVar.h"

class IfVariableNonZeroFunction : public IfVariableFunction {
public:
    virtual const char *Name();                          // 0x4465b0
    virtual IfFunction *Make();                          // 0x4465c0
    virtual int Check(int v, ScriptCall *args, int extra);   // 0x4465a0
};

const char *IfVariableNonZeroFunction::Name() { return "If"; }
IfFunction *IfVariableNonZeroFunction::Make() { return new IfVariableNonZeroFunction; }
int IfVariableNonZeroFunction::Check(int v, ScriptCall *args, int extra) { return v != 0; }
