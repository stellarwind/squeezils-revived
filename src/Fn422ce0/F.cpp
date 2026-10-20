// SQZ.EXE 0x422ce0 (cdecl): run a script function by name, from the top table only when asked, else from any table; 0 if not found.
#include "String/String.h"

struct CallArg;
class RunFunction {
public:
    virtual void slot0();
    virtual void Run(CallArg *args, int extra);
};
struct RunTable {
    virtual void slot0();
    virtual int Contains(const String &name);
    RunFunction *Get(const String &name);                  // 0x46c430
};
struct RunStack {
    void *vtable;
    int count, capacity;
    RunTable **data;
    RunTable *At(int index) { return data[index]; }
};
extern RunStack gFunctionStack;                          // 0x4c7090
RunFunction *FindFunction(const char *name);             // 0x423130 (cdecl)
int DebugLogAll(const char *format, ...);                  // 0x419290
extern const char kNoFunction[];                         // 0x4bb21c

int CallFunction(const char *name, CallArg *args, int extra, int topOnly)
{
    RunFunction *fn = 0;
    if (!topOnly)
        fn = FindFunction(name);
    else if (gFunctionStack.At(gFunctionStack.count - 1)->Contains(String(name)))
        fn = gFunctionStack.At(gFunctionStack.count - 1)->Get(String(name));
    if (!fn) {
        DebugLogAll(kNoFunction, name);
        return 0;
    }
    fn->Run(args, extra);
    return 1;
}
