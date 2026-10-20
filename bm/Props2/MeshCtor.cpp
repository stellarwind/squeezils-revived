// MeshProp constructor, 0x4262e0.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
#include "String/String.h"
struct MeshCtorBase {
    virtual ~MeshCtorBase(); // 0x425dc0
    Vector3f position, scale, direction, up;
    void *frame, *owner;
    int isInstance;
    MeshCtorBase(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up); // 0x425e50
};
struct MeshCtor : MeshCtorBase {
    String name;
    MeshCtor(const char *text, Vector3f position, Vector3f scale, Vector3f direction, Vector3f up);
};
MeshCtor::MeshCtor(const char *text, Vector3f position, Vector3f scale,
    Vector3f direction, Vector3f up)
    : MeshCtorBase(position, scale, direction, up)
{
    name = String(text);
}
