// 0x4589a0: is the player's actor close enough and (roughly) facing this soul?
#include "Vector3f/Vector3f.h"
struct Body3 {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual struct Pose *GetPose();                    // slot 3
};
struct Pose {
    char pad[0x20];
    float dx, dy, dz;                                  // 0x20 facing
    float pad2c;
    Vector3f pos;                                      // 0x30
};
struct Sim3 { Body3 *GetActor(); };                    // 0x414050
Sim3 * __stdcall GetSim(int which);                               // 0x420690
extern float g_range;                                  // 0x4c7bac
struct Soul9 {
    char pad[0x34];
    Vector3f pos;                                      // 0x34
    int Near();
};
int Soul9::Near()
{
    Body3 *b = GetSim(1)->GetActor();
    Pose *p = b->GetPose();
    Vector3f d = pos - p->pos;
    if (g_range * g_range < d.x * d.x + d.y * d.y + d.z * d.z)
        return 0;
    float dot = p->dx * d.x + p->dy * d.y + p->dz * d.z;
    if (dot < 0.0f && dot * dot > 4.0f)
        return 0;
    return 1;
}
