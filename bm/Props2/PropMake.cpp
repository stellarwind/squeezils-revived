// Prop virtual slot 2 (SQZ.EXE 0x425fa0, shared by MeshProp/FrameProp/LODProp/NavProp/CycleProp): make a copy placed at the given pose.
#include "Vector3f/Vector3f.h"

struct Float3 { float x, y, z; };
struct PropMade {
    Float3 position;                                     // 0x04 (the vptr takes 0)
    Float3 scale;                                        // 0x10
    Float3 direction;                                    // 0x1c
    Float3 up;                                           // 0x28
    virtual void v0();
    virtual PropMade *Clone(int how);                    // slot 1 (+4)
    PropMade *Place(int how, Vector3f pos, Vector3f scale, Vector3f dir, Vector3f up);   // slot 2  0x425fa0
};

PropMade *PropMade::Place(int how, Vector3f pos, Vector3f sc, Vector3f dir, Vector3f u)
{
    PropMade *p = Clone(how);
    p->position.x = pos.x; p->position.y = pos.y; p->position.z = pos.z;
    p->scale.x = sc.x; p->scale.y = sc.y; p->scale.z = sc.z;
    p->direction.x = dir.x; p->direction.y = dir.y; p->direction.z = dir.z;
    p->up.x = u.x; p->up.y = u.y; p->up.z = u.z;
    return p;
}
