// FrameProp::FindChild (virtual slot 13, SQZ.EXE 0x427f60): the child whose name matches.
#include "String/String.h"

struct NameList {                                        // the name list object embedded at +0x40
    virtual String Get(int i);                           // slot 0
    int count;                                           // 0x44
};
struct ChildArray {                                      // the child list object embedded at +0x54
    virtual void *Get(int i);                            // slot 0
    int count;
};
int NamesEqual(const String &a, const String &b);        // 0x438720 (cdecl)

struct FindFrame {
    char pad0[0x40];
    NameList names;                                      // 0x40
    char pad1[0x54 - 0x48];
    ChildArray children;                                 // 0x54
    void *FindChild(String name);                        // slot 13
};

void *FindFrame::FindChild(String name)
{
    for (int i = 0; i < names.count; i++)
        if (NamesEqual(names.Get(i), name))
            return children.Get(i);
    return 0;
}
