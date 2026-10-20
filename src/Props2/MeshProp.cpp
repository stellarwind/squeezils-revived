// MeshProp small slots (SQZ.EXE 0x426170, 0x4261b0, 0x426230, 0x426280).
#include "Vector3f/Vector3f.h"
#include "String/String.h"

struct SceneTable {                                      // the scene's frame table (global at 0x4c75e0)
    void *Find(void *handle);                            // 0x4772a0
    void Remove(void *object, void *handle);             // 0x4773e0
};
extern SceneTable *g_scene;                              // 0x4c75e0
struct MeshOwner { void *Object(); };                    // 0x4276b0

struct MeshProp {
    int pad[13];
    void *frame;                                         // 0x34 (the vptr takes 0)
    MeshOwner *owner;                                    // 0x38
    void Hide();                                         // slot 5  0x426170
    void Place(int how, Vector3f where);                 // slot 6  0x4261b0
    void *Underlying();                                  // slot 10 0x426230
    int FindChild(String name);                          // slot 13 0x426280
};

void MeshProp::Hide()
{
    if (frame) {
        void *f = g_scene->Find(frame);
        if (f)
            g_scene->Remove(f, frame);
        frame = 0;
    }
}

void MeshProp::Place(int how, Vector3f where) {}

void *MeshProp::Underlying()
{
    void *found = 0;
    if (frame)
        found = g_scene->Find(frame);
    void *obj = 0;
    if (owner)
        obj = owner->Object();
    if (!obj)
        obj = found;
    return obj;
}

int MeshProp::FindChild(String name)
{
    return 0;
}
