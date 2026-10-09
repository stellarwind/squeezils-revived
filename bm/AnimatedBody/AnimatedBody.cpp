// AnimatedBody getters/setter (SQZ.EXE 0x41da70-0x41daa0, 0x41e290).
#include "AnimatedBody/AnimatedBody.h"

float AnimatedBody::GetTime()
{
    return time;
}

float AnimatedBody::GetDuration()
{
    return cycle->Duration();
}

int AnimatedBody::GetLoopMode()
{
    return loopMode;
}

void *AnimatedBody::GetRootFrame()
{
    return rootFrame;
}

void AnimatedBody::SetTime(float t)
{
    time = t;
}

void AnimatedBody::SetDirection(int d)
{
    if (d < 0)
        d = -1;
    else if (d > 0)
        d = 1;
    direction = d;
}

void AnimatedBody::SetLoopMode(int m)
{
    if (m < 0)
        m = -1;
    else if (m > 0)
        m = 1;
    loopMode = m;
}
