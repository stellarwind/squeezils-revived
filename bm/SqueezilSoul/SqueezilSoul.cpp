// SqueezilSoul trivial getters (0x404fc0, 0x404fd0, 0x405010, 0x450300).
#include "SqueezilSoul/SqueezilSoul.h"

extern void *g_defaultPhysics;   // 0x4c7ad0

int  SqueezilSoul::GetState()       { return state; }
void *SqueezilSoul::GetPhysics()    { return physics; }
void *SqueezilSoul::GetBody()       { return body; }
void *SqueezilSoul::GetPossessions(){ return possessions; }
void *SqueezilSoul::GetDefaultPhysics() { return g_defaultPhysics; }
