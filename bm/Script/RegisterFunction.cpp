// 0x422be0: replace an existing script function, then register its arguments.
#include <stdarg.h>
#include "String/String.h"

struct RegisteredFunction { virtual ~RegisteredFunction(); };
struct RegistrationTable {
    virtual void Add(const String &name, va_list arguments, int count);
    virtual int Contains(const String &name);
    RegisteredFunction *Get(const String &name); // 0x46c430
};
RegistrationTable *TopRegistrationTable();       // 0x423220

void RegisterFunction(const char *name, ...)
{
    if (TopRegistrationTable()->Contains(String(name))) {
        delete TopRegistrationTable()->Get(String(name));
    }
    va_list arguments;
    va_start(arguments, name);
    TopRegistrationTable()->Add(String(name), arguments, -1);
    va_end(arguments);
}
