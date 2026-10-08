// Script: see Script.h. Reconstructed from SQZ.EXE 0x422ce0 (and 0x423130, which only it calls).
#include "Script.h"
#include "Container.h"
#include "GameString.h"
#include "Original.h"
#include "VirtualCall.h"

typedef Table<String, Function *, Array<Function *>, 0> FunctionTable;

static Array<FunctionTable *> *const kFunctionStack = (Array<FunctionTable *> *)0x4c7090;
static const char *const kNoSuchFunction = (const char *)0x4bb21c;   // "ERROR: couldn't find function %s.\n"

// @00423130 searches the stack from the top; NULL when no table has the name
static Function *FindFunction(const char *name)
{
    for (int i = kFunctionStack->count - 1; i >= 0; i--) {
        FunctionTable *table = kFunctionStack->At(i);
        if (table->Contains(String(name)))
            return table->Get(String(name));
    }
    return NULL;
}

// @00422ce0
int PostFunction(const char *name, void *args, int extra, int local)
{
    Function *function = NULL;
    if (!local) {
        function = FindFunction(name);
    } else {
        FunctionTable *top = kFunctionStack->At(kFunctionStack->count - 1);
        if (top->Contains(String(name)))
            function = top->Get(String(name));
    }
    if (!function) {
        GameLog(kNoSuchFunction, name);
        return 0;
    }
    CallVirtual<void>(function, FUNCTION_EXECUTE, args, extra);
    return 1;
}
