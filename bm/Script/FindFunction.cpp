// SQZ.EXE 0x423130: search function tables from the top of the script stack.
#include "String/String.h"

class Function;
struct ScriptFunctionTable {
    virtual void slot0();
    virtual int Contains(const String &name);
    Function *Get(const String &name);                  // 0x46c430
};
struct ScriptFunctionStack {
    void *vtable;
    int count, capacity;
    ScriptFunctionTable **data;
    ScriptFunctionTable *At(int index) { return data[index]; }
};
extern ScriptFunctionStack gFunctionStack;               // 0x4c7090

Function *FindFunction(const char *name)
{
    Function *result = 0;
    for (int index = gFunctionStack.count - 1; index >= 0; --index) {
        if (gFunctionStack.At(index)->Contains(String(name))) {
            result = gFunctionStack.At(index)->Get(String(name));
            break;
        }
    }
    return result;
}
