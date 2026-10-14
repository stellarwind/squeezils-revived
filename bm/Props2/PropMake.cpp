// Prop virtual slot 2 (SQZ.EXE 0x425fa0, shared by MeshProp/FrameProp/LODProp/NavProp/CycleProp): make a copy placed at the given pose.
#include "Vector3f/Vector3f.h"

struct PropMade {
    Vector3f position;                                     // 0x04 (the vptr takes 0)
    Vector3f scale;                                        // 0x10
    Vector3f direction;                                    // 0x1c
    Vector3f up;                                           // 0x28
    virtual void v0();
    virtual PropMade *Clone(int how);                    // slot 1 (+4)
    PropMade *Place(int how, Vector3f pos, Vector3f scale, Vector3f dir, Vector3f up);   // slot 2  0x425fa0
};

PropMade *PropMade::Place(int how, Vector3f pos, Vector3f sc, Vector3f dir, Vector3f u)
{
    PropMade *p = Clone(how);
    p->position = pos;
    p->scale = sc;
    p->direction = dir;
    p->up = u;
    return p;
}
