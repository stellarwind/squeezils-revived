// SqueezilSoul trivial getters (0x404fc0, 0x404fd0, 0x405010, 0x450300).
#include "SqueezilSoul/SqueezilSoul.h"

extern void *g_defaultPhysics;   // 0x4c7ad0
extern int   g_stateTable[];     // 0x49edf8 (stride 23, indexed by state)
extern int   g_stateMayFollow[]; // 0x49eda0 (stride 23, [state][newState])

void *SqueezilSoul::GetInterface(int which)
{
    if (which == 1)
        return this ? (char *)this + 4 : 0;
    return 0;
}

int  SqueezilSoul::GroundStateForSpeed(float speed)
{
    SqueezilPhysics *p = physics;
    if (p->speed298 < speed) return 8;
    if (p->speed294 < speed) return 4;
    if (0.01f < speed) return 3;
    return 0;
}

void SqueezilSoul::SetSpitStrength(float strength)
{
    if (spitRequested != 0)
        spitRequested = 0;
    else
        spitStrength = strength;
}

int  SqueezilSoul::GetState()       { return state; }
int  SqueezilSoul::IsDying()        { return g_stateTable[state * 23]; }
int  SqueezilSoul::CanEnter(int newState) { return g_stateMayFollow[state * 23 + newState]; }
int  SqueezilSoul::IsActive()       { return state != 22 && frozen == 0; }
void SqueezilSoul::Freeze()         { frozen++; }
void SqueezilSoul::Unfreeze()       { frozen--; if (frozen < 0) frozen = 0; }
void *SqueezilSoul::GetPhysics()    { return physics; }
void *SqueezilSoul::GetBody()       { return body; }
void *SqueezilSoul::GetPossessions(){ return possessions; }
void *SqueezilSoul::TakePossessions(){ void *p = possessions; possessions = 0; return p; }
void *SqueezilSoul::GetDefaultPhysics() { return g_defaultPhysics; }
