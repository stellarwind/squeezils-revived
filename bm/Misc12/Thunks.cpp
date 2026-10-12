// Forwarding thunks: 0x412be0 (deref then call), 0x412c30 and 0x4143c0 (plain tail calls), 0x4153f0 (cdecl pass-through).
#include "compat41.h"

struct Holder { void Inner(int v); void DerefCall(int *p); };   // Inner: 0x412c10
void Holder::DerefCall(int *p) { Inner(*p); }   // 0x412be0

void Target1();                              // 0x412c40
void Thunk1() { Target1(); }                 // 0x412c30
void Target2();                              // 0x4143d0
void Thunk2() { Target2(); }                 // 0x4143c0

void Sub(int v);                             // 0x448350 (cdecl)
void PassThrough(int v) { Sub(v); }          // 0x4153f0
