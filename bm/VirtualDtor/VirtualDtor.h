// The scalar-deleting-destructor of a *trivial virtual-dtor* class (SQZ.EXE):
//   mov eax,[esp+4]; push esi; mov [ecx],<vtable>; mov esi,ecx; test al,1; je skip;
//   push esi; call operator delete; add esp,4; skip: mov eax,esi; pop esi; ret 4
// This is the MSVC `??_G` generated for `delete obj` when the complete-object destructor is just
// `mov [ecx],vtbl`. The vtable store must be INLINE (the dtor declared `virtual ~T() {}` in the header)
// so MSVC inlines the store instead of CALLing the complete-object dtor.
#pragma once
#include "compat41.h"

struct VirtualDtorObj {
    virtual ~VirtualDtorObj() {}
};
