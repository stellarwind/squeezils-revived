// LaunchFromFaceNormalFunction::Execute (0x4307b0): launch a creature standing on the ground off the face it stands on.
// arg 1 == 1: stand it on the face normal (up = normal, facing kept as far as possible) and send it along the normal at
// speed arg 0. Otherwise pitch its facing up by the climb its motion still lacks to reach height arg 0 and send it along
// the old motion plus arg 0 times its up. Then the creature's state becomes 0x11 (arg 2 == 0) or 0xe (jump).
// Only when the creature's tuning record has its launch flag (+0x288) set.
#define V3_COPYCTOR
#include "FunctionTweak/BodyParams.h"

struct LaunchMatrix {                                    // 64 bytes
    float m[16];
    LaunchMatrix() {}
    LaunchMatrix(const LaunchMatrix &o);                 // 0x4099e0
    ~LaunchMatrix();                                     // 0x402310
    LaunchMatrix &SetRows(const Vector3f &right, const Vector3f &up,
                          const Vector3f &forward, const Vector3f &position);   // 0x409f00
};
struct LaunchContact {
    int face;                                            // 0x00
    int second;                                          // 0x04 the face inside the surface
    LaunchMatrix matrix;                                 // 0x08
    ~LaunchContact();                                    // 0x4031e0
};
struct LaunchParams {                                    // 0x2a0 bytes (BodyParams)
    char pad[0x288];
    int launch;                                          // 0x288
    char pad2[0x2a0 - 0x28c];
    LaunchParams(const LaunchParams &o);                 // 0x405040
    ~LaunchParams();                                     // 0x4052b0
};
struct LaunchBody {
    virtual void s0(); virtual void s1(); virtual void s2();
    virtual LaunchMatrix *Matrix();                      // slot 3 (0x0c)
    virtual Vector3f *Direction();                       // slot 4 (0x10)
    virtual float Speed();                               // slot 5 (0x14)
    virtual void s6();
    virtual LaunchContact *Contact();                    // slot 7 (0x1c)
    virtual void s8();
    virtual void SetMatrix(LaunchMatrix *m);             // slot 9 (0x24)
    virtual void SetMotion(Vector3f *dir, float speed);  // slot 10 (0x28)
    virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual int OnGround();                              // slot 15 (0x3c)
};
struct LaunchActor {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual Vector3f *Up();                              // slot 10 (0x28)
    virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual LaunchBody *Body();                          // slot 17 (0x44)
    virtual void s18(); virtual void s19(); virtual void s20();
    virtual void Refresh();                              // slot 21 (0x54)
    virtual void s22(); virtual void s23();
    virtual void SetState(int state, int arg);           // slot 24 (0x60)
    char pad[0x48 - 4];
    LaunchParams *params;                                // 0x48
};
struct LaunchFace { char pad[0xc]; Vector3f normal; char pad2[0x28 - 0x18]; };   // 0x28 bytes
struct LaunchMesh { char pad[0xc]; LaunchFace *faces; };                          // 0x0c
struct LaunchWorld { LaunchMesh *SurfacesOf(int face); };                         // 0x4365a0
LaunchWorld *GetWorld();                                                          // 0x4206a0

#define LAUNCH_ACTOR() ((LaunchActor *)((CallActor *)a)->Actor())

int LaunchFromFaceNormalFunction::Execute(ScriptCall *a, int extra)
{
    if (!LAUNCH_ACTOR()->Body()->OnGround())
        return 1;
    LaunchParams params(*LAUNCH_ACTOR()->params);
    if (!params.launch)
        return 1;
    LaunchActor *actor = LAUNCH_ACTOR();
    LaunchBody *body = actor->Body();
    LaunchContact contact = *body->Contact();
    Vector3f normal = GetWorld()->SurfacesOf(contact.face)->faces[contact.second].normal;
    if (ARGF(1, extra) == 1.0f) {
        LaunchMatrix m;
        m.m[0] = 1.0f;  m.m[1] = 0.0f;  m.m[2] = 0.0f;  m.m[3] = 0.0f;
        m.m[4] = 0.0f;  m.m[5] = 1.0f;  m.m[6] = 0.0f;  m.m[7] = 0.0f;
        m.m[8] = 0.0f;  m.m[9] = 0.0f;  m.m[10] = 1.0f; m.m[11] = 0.0f;
        m.m[12] = 0.0f; m.m[13] = 0.0f; m.m[14] = 0.0f; m.m[15] = 1.0f;
        Vector3f n = normal;
        Vector3f up = *LAUNCH_ACTOR()->Up();
        Vector3f down = up * -1.0f;
        down.Normalize();
        Vector3f side = n.Cross(down);
        Vector3f forward = side.Cross(n);
        Vector3f pos = *(Vector3f *)&body->Matrix()->m[12];
        pos = pos + normal;
        m.SetRows(side, forward, normal, pos);
        body->SetMatrix(&m);
        body->SetMotion(&normal, ARGF(0, extra));
    } else {
        LaunchMatrix m(*body->Matrix());
        Vector3f forward = *(Vector3f *)&m.m[8];
        Vector3f up = *(Vector3f *)&m.m[4];
        Vector3f right = *(Vector3f *)&m.m[0];
        float height = ARGF(0, extra);
        Vector3f *dir = body->Direction();
        float along = body->Speed() * (dir->x * forward.x + dir->y * forward.y + dir->z * forward.z);
        float t = 1.0f;
        if (along > 0.01f)
            t = height / along;
        if (t > 1.0f)
            t = 1.0f;
        Vector3f newForward = forward + up * t;
        newForward.Normalize();
        Vector3f newUp = forward.Cross(right);
        Vector3f velocity = *body->Direction() * along + *(Vector3f *)&body->Matrix()->m[4] * height;
        Vector3f launch = velocity;
        launch.Normalize();
        m.SetRows(right, newUp, newForward, *(Vector3f *)&m.m[12] + launch);
        body->SetMatrix(&m);
        body->SetMotion(&launch, velocity.Length());
    }
    actor->SetState(0, 0);
    actor->SetState(ARGF(2, extra) == 0.0f ? 0x11 : 0xe, 0);
    LAUNCH_ACTOR()->Refresh();
    return 1;
}
