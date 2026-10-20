// 0x42e2b0: follow the camera: copy its position/target/up, work out heading and a damped tilt, aim the rig at +0x480.
#include <math.h>
struct Cam { virtual void v0(); virtual void v1(); virtual void v2(); virtual float *Get(); };   // slot 3 (+0xc)
struct Holder { Cam *Find(); };                       // 0x414050 (thiscall)
Holder *__stdcall GetHolder(int a);                             // 0x420690 (stdcall, 1 arg)
struct Rig { void Aim(float heading, float tilt); };  // 0x414fb0 (thiscall, ret 8)
extern float gZeroF;                                   // 0x49de2c
extern float gLastHeading, gLastY, gTilt;             // 0x4bbe94 0x4bbe98 0x4bbe9c
struct Follower {
    char pad[0x480];
    Rig rig;                                          // 0x480
    char pad2[0x5bc - 0x480 - 1];
    float headingTol;                                 // 0x5bc
    float yTol;                                       // 0x5c0
    float tiltStep;                                   // 0x5c4
    float pos[3];                                     // 0x5c8
    float tgt[3];                                     // 0x5d4
    float up[3];                                      // 0x5e0
    float saved[9];                                   // 0x5ec
    float wrap;                                       // 0x610
    void Follow(int force);
};
void Follower::Follow(int force)
{
    Cam *cam = GetHolder(0)->Find();
    if (!cam)
        return;
    float *p = cam->Get();
    pos[0] = p[0];
    pos[1] = p[1];
    pos[2] = p[2];
    float *m = cam->Get();
    up[0] = m[8];
    up[1] = m[9];
    up[2] = m[10];
    m = cam->Get();
    double h = atan2(up[2], pos[2]);
    tgt[0] = m[12];
    tgt[1] = m[13];
    tgt[2] = m[14];
    float heading = (float)h;
    if (h < gZeroF)
        heading = wrap + heading;
    float dh = heading - gLastHeading;
    float dy;
    if (gLastY == -99.0f)
        dy = 0.0f;
    else
        dy = tgt[1] - gLastY;
    if (headingTol > fabs(dh) && force == 0 && yTol >= fabs(dy) && (*(unsigned *)&gTilt & 0x7fffffff) == 0)
        return;
    if (yTol < fabs(dy)) {
        if (*(unsigned *)&dy > 0x80000000)
            gTilt = tiltStep + gTilt;
        else
            gTilt = gTilt - tiltStep;
    } else {
        if (*(unsigned *)&gTilt > 0x80000000)
            gTilt = gTilt - tiltStep;
        else if (*(int *)&gTilt > 0)
            gTilt = tiltStep + gTilt;
    }
    if (*(int *)&gTilt >= 0x3f800000 || *(unsigned *)&gTilt >= 0xbf800000)
        gTilt = 0.0f;
    rig.Aim(heading, gTilt);
    saved[0] = pos[0]; saved[1] = pos[1]; saved[2] = pos[2];
    saved[3] = tgt[0]; saved[4] = tgt[1]; saved[5] = tgt[2];
    saved[6] = up[0]; saved[7] = up[1]; saved[8] = up[2];
    gLastHeading = heading;
    gLastY = tgt[1];
}
