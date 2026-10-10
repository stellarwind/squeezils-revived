// The game's universal "destroy and optionally free" slot-0 pattern (SQZ.EXE):
//   push esi; mov esi,ecx; call <Destruct>; mov eax,[esp+8]; test al,1; je skip;
//   push esi; call operator delete; add esp,4; skip: mov eax,esi; pop esi; ret 4
// One source matches every Prop/Soul/Body/Behavior class's Destroy/DestroyAndFree (the Destruct call and
// the operator-delete import are masked relocations, so the bytes are identical).
#pragma once
#include "compat41.h"

struct DestroyObj {
    void Destruct();                              // out-of-line (target masked)
    DestroyObj *DestroyAndFree(int flags);        // slot 0
};
