// 0x40f610: look an item up by id, take its property name for the part string, and return the registered value (0 if none).
#include "String/String.h"
struct Props { String NameOf(String &k); };          // 0x462cc0
struct Item { Props *GetProperties(); };             // 0x40d520
struct ListC { virtual Item *Get(int id); };
struct Entry { int a; int value; };
struct Reg { virtual int v0(); virtual int Has(String &k); Entry *Get(String &k); };   // slot 1 (+4); 0x46d2a0
struct Holder7 {
    char pad[0x134];
    ListC c;                                         // 0x134
    char pad2[0x16c - 0x138];
    Reg reg;                                         // 0x16c
    String Part(int *key);                           // 0x40f3c0
    int Find(int *key);
};
int Holder7::Find(int *key)
{
    String part = Part(key);
    String name = c.Get(*key)->GetProperties()->NameOf(part);
    if (reg.Has(name))
        return reg.Get(name)->value;
    return 0;
}

// 0x40f770: same lookup on the sibling registry, returning the entry's second value.
struct Entry8 { int a; int b; int value; };
struct Reg8 { virtual int v0(); virtual int Has(String &k); Entry8 *Get(String &k); };
struct Holder8 {
    char pad[0x134];
    ListC c;                                         // 0x134
    char pad2[0x16c - 0x138];
    Reg8 reg;                                        // 0x16c
    String Part(int *key);                           // 0x40f3c0
    int Find(int *key);
};
int Holder8::Find(int *key)
{
    String part = Part(key);
    String name = c.Get(*key)->GetProperties()->NameOf(part);
    if (reg.Has(name))
        return reg.Get(name)->value;
    return 0;
}

// 0x40f8d0: same lookup, third value.
struct Entry9 { int a; int b; int c; int value; };
struct Reg9 { virtual int v0(); virtual int Has(String &k); Entry9 *Get(String &k); };
struct Holder9 {
    char pad[0x134];
    ListC c;
    char pad2[0x16c - 0x138];
    Reg9 reg;
    String Part(int *key);
    int Find(int *key);
};
int Holder9::Find(int *key)
{
    String part = Part(key);
    String name = c.Get(*key)->GetProperties()->NameOf(part);
    if (reg.Has(name))
        return reg.Get(name)->value;
    return 0;
}

// 0x40fb20: same lookup, float value (default from a global constant).
extern float gDefaultF;                                          // see targets
struct Entry10 { int a; int b; int c; int d; float value; };
struct Reg10 { virtual int v0(); virtual int Has(String &k); Entry10 *Get(String &k); };
struct Holder10 {
    char pad[0x134];
    ListC c;
    char pad2[0x16c - 0x138];
    Reg10 reg;
    String Part(int *key);
    float Find(int *key);
};
float Holder10::Find(int *key)
{
    String part = Part(key);
    String name = c.Get(*key)->GetProperties()->NameOf(part);
    if (reg.Has(name))
        return reg.Get(name)->value;
    return gDefaultF;
}

// 0x40f9b0: same lookup returning the entry's string (empty string when missing).
struct EntryS { String Str(); };                         // 0x40fab0
struct RegS { virtual int v0(); virtual int Has(String &k); EntryS *Get(String &k); };
struct HolderS {
    char pad[0x134];
    ListC c;
    char pad2[0x16c - 0x138];
    RegS reg;
    String Part(int *key);
    String Find(int *key);
};
String HolderS::Find(int *key)
{
    String part = Part(key);
    String name = c.Get(*key)->GetProperties()->NameOf(part);
    if (reg.Has(name)) {
        EntryS *e = reg.Get(name);
        return e->Str();
    }
    return String("");
}
