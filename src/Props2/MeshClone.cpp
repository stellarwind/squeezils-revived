// MeshProp::Clone (virtual slot 1, SQZ.EXE 0x426470): a new instance of this prop with the same pose and name.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
#include "String/String.h"

struct MeshOwner2;
struct MeshProp {
    virtual ~MeshProp();
    virtual MeshProp *Clone(MeshOwner2 *owner);          // slot 1  0x426470
    Vector3f position;                                   // 0x04
    Vector3f scale;                                      // 0x10
    Vector3f direction;                                  // 0x1c
    Vector3f up;                                         // 0x28
    void *frame;                                         // 0x34
    MeshOwner2 *owner;                                   // 0x38
    int isInstance;                                      // 0x3c
    String name;                                         // 0x40
    MeshProp(const char *name, Vector3f position, Vector3f scale, Vector3f direction, Vector3f up);   // 0x4262e0
};

MeshProp *MeshProp::Clone(MeshOwner2 *newOwner)
{
    MeshProp *m = new MeshProp(name.c_str(), position, scale, direction, up);
    m->owner = newOwner;
    m->isInstance = 1;
    return m;
}
