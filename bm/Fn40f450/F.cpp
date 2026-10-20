// 0x40f450: NavSurfaceList::SetFaceSet - bind a face set of the contact's surface copy to a surface type.
#include "String/String.h"
#include "Container/ContainerArray.h"
void GameLog(const char *format, ...);                      // 0x419290
int StringEquals(const String &a, const String &b);         // 0x438720 (cdecl)
struct Contact { int face; };
struct Props {                                              // the copy's property table
    virtual int Insert(String &k, String &v, int at);
    virtual int Has(String &k);
};
struct Walk { int walkable; };
struct Reg { virtual int v0(); virtual int Has(String &k); Walk *Get(String &k); };   // 0x46d2a0
struct Rec { char pad[0x14]; Array<String> names; };
struct Item {
    Props *GetProperties();                                 // 0x40d520
    Rec *Ptr();                                             // 0x40c6b0
    void SetFlag(int i, int v);                             // 0x40c6e0
};
struct ListC { virtual Item *Get(int id); };
struct Holder {
    char pad[0x134];
    ListC c;                                                // 0x134
    char pad2[0x16c - 0x138];
    Reg types;                                              // 0x16c
    void SetFaceSet(String faceSet, String type, const Contact *contact);
};
void Holder::SetFaceSet(String faceSet, String type, const Contact *contact)
{
    const char *faceSetText = faceSet.c_str();
    const char *typeText = type.c_str();
    Props *p = c.Get(contact->face)->GetProperties();
    if (!p->Has(faceSet)) {
        GameLog("ERROR: can't set nonexistent face set %s to surface type %s.\n", faceSetText, typeText);
        return;
    }
    if (!types.Has(type)) {
        GameLog("ERROR: can't set face set %s to nonexistent surface type %s.\n", faceSetText, typeText);
        return;
    }
    p->Insert(faceSet, type, -1);
    Rec *l = c.Get(contact->face)->Ptr();
    for (int i = 0; i < l->names.count; i++) {
        if (StringEquals(l->names.Get(i), faceSet)) {
            c.Get(contact->face)->SetFlag(i, types.Get(type)->walkable);
            break;
        }
    }
}
