// 0x45a850: constructor of a body-like object: registers a handle for its name with the global registry, tells the named service about it.
#include "String/String.h"
struct Service { virtual void v0(); virtual void v1(); virtual void v2(); virtual void Bind(int handle); };
Service *FindService(String name, int a, int b);     // 0x42c8b0 (cdecl)
struct Registry { int NewHandle(); void Set(int handle, int owner); };   // 0x476f40, 0x477040
extern Registry *g_reg;                              // 0x4c75e0
class BaseB {
public:
    BaseB(int owner);                                // 0x41fb90
    virtual ~BaseB();                                // 0x40bae0
    virtual void s1();
    char pad[0x44 - 4];
};
class NamedBody : public BaseB {
public:
    NamedBody(int owner, const char *name);
    virtual ~NamedBody();
    int a44, a48, a4c;
    char pad2[0xc0 - 0x50];
    int handle;                                      // 0xc0
};
NamedBody::NamedBody(int owner, const char *name) : BaseB(owner)
{
    if (name) {
        handle = g_reg->NewHandle();
        g_reg->Set(handle, owner);
        FindService(String(name), 0, 0)->Bind(handle);
    }
    a44 = 0;
    a48 = 0;
    a4c = 0;
}
