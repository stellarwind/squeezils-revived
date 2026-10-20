// CycleProp: see CycleProp.h. Reconstructed from SQZ.EXE 0x4288b0-0x429380.
#include "CycleProp.h"

static void *const kCyclePropVtable = (void *)0x49dd44;

// @004288b0
CycleProp *CycleProp::Init(Float3 pos, Float3 scl, Float3 dir, Float3 upVector, StringList *names, MorphCycle *clip,
                           CycleMessageTable *cycleMessages, StepEventTable *stepEvents, float stepPeriod, int keepRemainder,
                           float clipStart)
{
    FrameProp::Init(pos, scl, dir, upVector, names, clip, cycleMessages);
    ::new (&events) StepEventTable();
    vtbl = (void **)kCyclePropVtable;
    carry = keepRemainder;
    timer = stepPeriod;
    period = stepPeriod;
    current = 0;
    startTime = clipStart;
    step = 1;
    mode = 1;
    for (int i = 0; i < stepEvents->keys.count; i++)
        events.Insert(stepEvents->keys.At(i), stepEvents->GetAt(i));
    return this;
}

// @00428d10
void CycleProp::Destruct()
{
    vtbl = (void **)kCyclePropVtable;
    events.~StepEventTable();
    FrameProp::Destruct();
}

// @00428b60
Prop *CycleProp::Destroy(int flags)
{
    Destruct();
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @00428bd0 (The copy starts on the same child as the template.)
Prop *CycleProp::Clone(FrameProp *newOwner)
{
    CycleProp *copy = (CycleProp *)Operator_New(sizeof(CycleProp));
    if (copy)
        copy->Init(position, scale, direction, up, &childNames, cycle, &messages, &events, period, carry, startTime);
    copy->current = current;
    copy->owner = newOwner;
    copy->isInstance = 1;
    return copy;
}

// @00428d80 (The original also fetches the child's name here and does not use it.)
IDirect3DRMFrame *CycleProp::BuildFrame(IDirect3DRMFrame *parent)
{
    BuildOwnFrame(parent);
    AddUpdatingProp(this);
    IDirect3DRMFrame *childFrame = children.At(current)->VShow(GetPartFrame());
    children.At(current)->VApplyScale(childFrame, scale);
    return frame;
}

// @00428e90
void CycleProp::Update(float seconds)
{
    FrameProp::Update(seconds);
    if (step == 0)
        return;

    timer -= seconds;
    int from = current;
    int next = from;
    if (*(unsigned *)&timer > 0x80000000u) {          // below zero: time for a step (one, however much time went by)
        next = from + step;
        do {
            if (carry)
                timer += period;
            else
                timer = period;
        } while (*(unsigned *)&timer > 0x80000000u);
    }

    int count = childNames.count;
    if (next < count && next >= 0) {
        FireStepMessages(from, next);
    } else if (mode == 1) {                           // wrap: up to the end, then on from the other one
        bool below = next < 0;
        FireStepMessages(from, below ? 0 : count - 1);
        next = below ? next + count : next - count;
        FireStepMessages(below ? count - 1 : 0, next);
    } else if (mode == -1) {                          // turn round at the end
        step = -step;
        next = next < 0 ? 0 : count - 1;
        FireStepMessages(from, next);
    } else {                                          // stop at the end
        FireStepMessages(from, next < 0 ? 0 : count - 1);
        if (hideAtEnd)
            VHide();
    }
    SetCurrent(next);
}

// @00428fd0 Without a frame (not shown) only the number changes. The stop-at-the-end case of Update can hand this a number
// outside the children when "hide at end" is off, which the original does not guard against either.
void CycleProp::SetCurrent(int child)
{
    if (frame == NULL) {
        current = child;
        return;
    }
    if (current == child)
        return;
    children.At(current)->VHide();
    current = child;
    IDirect3DRMFrame *childFrame = children.At(child)->VShow(GetPartFrame());
    children.At(current)->VApplyScale(childFrame, scale);
}

// @00429350
void CycleProp::SetStep(int newStep)
{
    step = newStep;
}

// @00429360
void CycleProp::SetMode(int newMode)
{
    mode = newMode;
}

// @004290b0 Going down (to below from), the events for the numbers from, from-1 ... to+1 run: the number it arrives at does not.
// Going up the original's code has a branch that can never be taken (its direction test is -1 or 1 and the branch needs more
// than 1), so nothing runs.
void CycleProp::FireStepMessages(int from, int to)
{
    if (!(to < from))
        return;
    for (int number = from; number > to; number--) {
        if (events.Contains(number)) {
            String name = events.Get(number);
            RunScript(name.c_str());
        }
    }
}
