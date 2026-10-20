// 0x4100b0 (ret 0x10): NavSurfaceList::AddSurface - make a surface from a loaded surface, give each face set the walk flag
// of its surface type (1 when the type is unknown), and store it in the first free slot; returns the slot index.
#include "String/String.h"
#include "Container/ContainerArray.h"
struct Props { String Get(const String &key); };            // 0x462cc0
struct Walk { int walkable; };
struct Reg { virtual int v0(); virtual int Has(const String &k); Walk *Get(const String &k); };   // 0x46d2a0
struct Loaded { char pad[0x14]; Array<String> typeNames; };  // 0x14
struct LoadedTable { virtual Loaded *Get(int id); };        // slot 0
struct Surface {
    Surface(void *owner, Loaded *loaded, void *a, void *b); // 0x40bf90, 0xf4 bytes
    Props *GetProperties();                                 // 0x40d520
    void SetFlag(int i, int v);                             // 0x40c6e0
    char pad[0xf4];
};
struct NavSurfaceList {
    char pad[0x124];
    LoadedTable loaded;                                     // 0x124
    char pad2[0x134 - 0x128];
    Array<Surface *> surfaces;                              // 0x134
    Array<int> freedA;                                      // 0x144
    char pad3[0x158 - 0x154];
    Array<int> freedB;                                      // 0x158
    char pad4[0x16c - 0x168];
    Reg types;                                              // 0x16c
    int AddSurface(void *owner, int loadedId, void *a, void *b);
};

int NavSurfaceList::AddSurface(void *owner, int loadedId, void *a, void *b)
{
    Loaded *l = loaded.Get(loadedId);
    Surface *s = new Surface(owner, l, a, b);
    int i;
    for (i = 0; i < l->typeNames.count; i++) {
        String name = l->typeNames.Get(i);
        String type = s->GetProperties()->Get(name);
        if (types.Has(type))
            s->SetFlag(i, types.Get(type)->walkable);
        else
            s->SetFlag(i, 1);
    }
    int slot;
    for (slot = 0; slot < surfaces.count; slot++) {
        if (surfaces.Get(slot) == 0) {
            surfaces.Replace(slot, s);
            int j;
            for (j = freedA.count - 1; j >= 0; j--)
                if (freedA.Get(j) == slot)
                    freedA.Remove(j);
            for (j = freedB.count - 1; j >= 0; j--)
                if (freedB.Get(j) == slot)
                    freedB.Remove(j);
            break;
        }
    }
    if (surfaces.count == slot)
        surfaces.Insert(s, -1);
    return slot;
}
