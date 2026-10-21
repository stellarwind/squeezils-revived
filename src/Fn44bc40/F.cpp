// 0x44bc40 (ret 0xc): if the key is known, resolve its name through the name table and run the command on the given thing.
#include "String/String.h"
struct Keys { virtual int v0(); virtual int Has(const String &k); };      // slot 1
struct NameTab : Keys { String NameOf(const String &k); };                // 0x462cc0
struct Thing { int Query(int a, int b); };                                // 0x4140b0
struct Ref { int d[6]; Ref(int x, int y); };                           // 0x420900
#include "Script/CallFunction.h"
struct Runner {
    char pad[0x1b8];
    NameTab names;                                                        // 0x1b8
    int Run(const char *key, int a, Thing *t);
};
int Runner::Run(const char *key, int a, Thing *t)
{
    int missing = !names.Has(String(key));
    if (missing)
        return 0;
    String name = names.NameOf(String(key));
    Ref r(a, t->Query(a, 0));
    return CallFunction(name.c_str(), &r, 0, 0);
}
