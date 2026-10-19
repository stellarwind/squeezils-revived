// SQZ.EXE 0x421f50: parse one script command using the registered function prototypes.
#include "FunctionArgs/FunctionArgs.h"

struct ArgList {
    virtual String ReadString(int flag);   // slot 0
    virtual void slot1();
    virtual void Skip(int n);              // slot 2
};
struct FunctionPrototypeRegistry {
    int Contains(const String &name);      // 0x46c390
    Function *Get(const String &name);     // 0x46c430
};
extern FunctionPrototypeRegistry gFunctionPrototypes; // 0x4c70a0
extern const char kUnrecognizedFunctionFormat[];      // 0x4bb15c

Function *ParseFunction(ArgList *list)
{
    String name = list->ReadString(0);
    if (gFunctionPrototypes.Contains(name)) {
        list->Skip(0);
        return gFunctionPrototypes.Get(name)->Create(list);
    }
    DebugLog(kUnrecognizedFunctionFormat, name.c_str());
    return 0;
}
