// FrameProp leaf functions (SQZ.EXE 0x427e30, 0x427e50, 0x427e70, 0x427f10, 0x427f20, 0x427f30, 0x427f40, 0x427f50).
#include "FrameProp/FrameProp.h"

void FrameProp::SetCycleTime(float time)
{
    if (body)
        body->SetTime(time);
}

void FrameProp::SetCycleDirection(int d)
{
    if (body)
        body->SetDirection(d);
}

void FrameProp::SetCycleLoopMode(int mode)
{
    if (body)
        body->SetLoopMode(mode);
}

void FrameProp::SetFollowMode(int mode)
{
    followMode = mode;
}

void FrameProp::SetActor(int index)
{
    actorIndex = index;
}

void FrameProp::SetHideAtEnd(int hide)
{
    hideAtEnd = hide;
}

int FrameProp::HasBody()
{
    return body != 0;
}

void *FrameProp::ChildNames() { return childNames; }
