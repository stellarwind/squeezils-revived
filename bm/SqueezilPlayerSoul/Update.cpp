// 0x41f5a0: update the player, fall asleep when idle, and play the landing sound.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
#include "String/String.h"
#include <math.h>
#pragma intrinsic(fabs)
struct PlayerUpdateMatrix { Vector3f *Position(); }; // 0x41faf0
struct PlayerUpdateBody {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual PlayerUpdateMatrix *GetMatrix();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void GetAngles(float *pitch, float *yaw, float *roll);
    virtual void v13(); virtual void v14(); virtual int IsOnGround();
};
struct PlayerUpdateClock { unsigned Elapsed(); }; // 0x41ec00
PlayerUpdateClock *GetPlayerUpdateClock(); // 0x4206d0
void PlayerSleepSound(String name); // 0x44a750
void PlayerResumeSounds(); // 0x44add0
void PlayerLandingSound(String name, Vector3f position); // 0x44a250
extern float g_playerSleepTime; // 0x4c7058
struct PlayerUpdateBase {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual PlayerUpdateBody *GetBody();
    virtual void v18(); virtual void v19(); virtual void v20(); virtual void v21();
    virtual void v22(); virtual void v23();
    virtual void SetState(int state, int arg);
    virtual int GetState();
    int pad04, finished;
    char pad0c[0x44 - 0x0c];
    int frozen;
    char pad48[0x9c - 0x48];
    float sleepTimer;
    int wasOnGround;
    void Update(); // 0x450e90
};
struct PlayerUpdate : PlayerUpdateBase { void Update(); };
void PlayerUpdate::Update()
{
    float pitch, yaw;
    GetBody()->GetAngles(&pitch, &yaw, &pitch);
    if (!finished) {
        int before = GetState();
        PlayerUpdateBase::Update();
        int after = GetState();
        if (before == 0 && after == 0 && fabs(yaw) <= (double)0.1f && frozen == 0) {
            sleepTimer += GetPlayerUpdateClock()->Elapsed() * 0.001f;
            if (!(sleepTimer <= g_playerSleepTime)) {
                SetState(1, 0);
                PlayerSleepSound(String("sleepEffect"));
                PlayerResumeSounds();
            }
        } else {
            sleepTimer = 0;
        }
        if (GetBody()->IsOnGround() != wasOnGround) {
            if (GetBody()->IsOnGround() == 1)
                PlayerLandingSound(String("landEffect"), *GetBody()->GetMatrix()->Position());
            wasOnGround = GetBody()->IsOnGround();
        }
    }
}
