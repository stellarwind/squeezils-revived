// FrameProp: see FrameProp.h. Reconstructed from SQZ.EXE 0x426c70-0x427f60.
//
// The body is always an AnimatedBody this class made (the class has no subclasses), so the original's calls through the body's
// vtable are plain calls here. The children can be any kind of prop and are called through their own vtables.
#include <string.h>
#include "FrameProp.h"
#include "PropLoader.h"
#include "BodyPart.h"
#include "Animation.h"
#include "Script.h"
#include "Scene.h"
#include "Renderer.h"

static void *const kFramePropVtable = (void *)0x49dc24;
static const char *const kBodyPartName = "part";     // the one part of a frame prop's body (0x4b9a7c)

// A frame's 4x4 matrix from a placement: rows right (up x direction), up, direction, position. The up vector is used as it is,
// not made square to the direction.
static void PlacementMatrix(float m[16], const Float3 &direction, const Float3 &up, const Float3 &position)
{
    float right[3];
    right[0] = up.y * direction.z - up.z * direction.y;          // @0041a350
    right[1] = up.z * direction.x - up.x * direction.z;
    right[2] = up.x * direction.y - up.y * direction.x;
    m[0]  = right[0];    m[1]  = right[1];    m[2]  = right[2];    m[3]  = 0.0f;
    m[4]  = up.x;        m[5]  = up.y;        m[6]  = up.z;        m[7]  = 0.0f;
    m[8]  = direction.x; m[9]  = direction.y; m[10] = direction.z; m[11] = 0.0f;
    m[12] = position.x;  m[13] = position.y;  m[14] = position.z;  m[15] = 1.0f;
}

// result = a * b, row-major 4x4                                                                                          @0047dea0
static void MultiplyMatrix(float result[16], const float a[16], const float b[16])
{
    for (int row = 0; row < 4; row++)
        for (int column = 0; column < 4; column++)
            result[row * 4 + column] = a[row * 4 + 0] * b[0 * 4 + column] + a[row * 4 + 1] * b[1 * 4 + column] +
                                       a[row * 4 + 2] * b[2 * 4 + column] + a[row * 4 + 3] * b[3 * 4 + column];
}

// @00426c70 The child list starts with a NULL for every name; BuildFrame fills them in.
FrameProp *FrameProp::Init(Float3 pos, Float3 scl, Float3 dir, Float3 upVector, StringList *names, MorphCycle *clip,
                           CycleMessageTable *cycleMessages)
{
    Prop::Init(pos, scl, dir, upVector);
    ::new (&childNames) StringList();
    ::new (&children) Array<Prop *>();
    ::new (&messages) CycleMessageTable();
    vtbl = (void **)kFramePropVtable;
    frame = NULL;
    for (int i = 0; i < names->count; i++) {
        childNames.Insert(names->At(i));
        children.Insert((Prop *)NULL);
    }
    actorIndex = -1;
    cycle = clip;
    body = NULL;
    startTime = 0.0f;
    followMode = 0;
    hideAtEnd = 0;
    for (int i = 0; i < cycleMessages->keys.count; i++)
        messages.Insert(cycleMessages->keys.At(i), cycleMessages->GetAt(i));
    return this;
}

// @00427100
void FrameProp::Destruct()
{
    vtbl = (void **)kFramePropVtable;
    for (int i = 0; i < children.count; i++) {
        Prop *child = children.At(i);
        if (child)
            child->VDestroy(1);
    }
    if (body) {
        body->Destruct();
        Operator_Delete(body);
    }
    messages.~CycleMessageTable();
    children.~Array<Prop *>();
    childNames.~StringList();
    Prop::Destruct();
}

// @00426f70
Prop *FrameProp::Destroy(int flags)
{
    Destruct();
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @00426fe0 The copy has the template's names, clip and messages but none of its children (BuildFrame makes its own). Like the
// other props, the original reads the new object without checking the allocation.
Prop *FrameProp::Clone(FrameProp *newOwner)
{
    FrameProp *copy = (FrameProp *)Operator_New(sizeof(FrameProp));
    if (copy)
        copy->Init(position, scale, direction, up, &childNames, cycle, &messages);
    copy->owner = newOwner;
    copy->isInstance = 1;
    return copy;
}

// @004271f0 Each child is shown under the body part's frame (or this prop's frame) and gets this prop's scale too.
// (The original also fetches each child's name here and does not use it.)
IDirect3DRMFrame *FrameProp::BuildFrame(IDirect3DRMFrame *parent)
{
    BuildOwnFrame(parent);
    AddUpdatingProp(this);
    for (int i = 0; i < childNames.count; i++) {
        Prop *child = children.At(i);
        IDirect3DRMFrame *childFrame = child->VShow(GetPartFrame());
        children.At(i)->VApplyScale(childFrame, scale);
    }
    return frame;
}

// @00427380 The frame is placed like a mesh prop's, but the scale goes after the placement (a mesh prop's goes before).
void FrameProp::BuildOwnFrame(IDirect3DRMFrame *parent)
{
    float m[16];
    PlacementMatrix(m, direction, up, position);
    frame = GetScene()->CreateFrameUnder(parent);
    ApplyFrameTransform(frame, parent, m);
    if (!(scale.x == 1.0f && scale.y == 1.0f && scale.z == 1.0f))
        frame->AddScale(D3DRMCOMBINE_AFTER, scale.x, scale.y, scale.z);

    if (cycle) {
        BodyPart *part = (BodyPart *)Operator_New(sizeof(BodyPart));
        if (part)
            part = part->Init();
        AnimatedBody *newBody = (AnimatedBody *)Operator_New(sizeof(AnimatedBody));
        if (newBody)
            newBody = newBody->Init();
        body = newBody;
        body->AddPart(kBodyPartName, part);
        body->Build(frame);
        body->SetCycle(cycle, 0);
        body->SetTime(startTime);
    }

    for (int i = 0; i < childNames.count; i++) {
        if (children.At(i) == NULL) {
            String name = childNames.At(i);
            Prop *child = FindPropTemplate(name.c_str(), 0, 0)->VClone(this);
            children.Replace(i, child);
        }
    }
}

// @004276b0 Where children and owned props attach: the body part's frame, or this prop's frame when there is no body.
IDirect3DRMFrame *FrameProp::GetPartFrame()
{
    if (body == NULL)
        return frame;
    BodyPart *part = body->partTable.Get(String(kBodyPartName));
    return part->GetFrame();
}

// @00427320
void FrameProp::Hide()
{
    for (int i = 0; i < children.count; i++) {
        if (children.At(i))
            children.At(i)->VHide();
    }
    if (body)
        body->DestroyFrames(frame);
    Prop::Hide();
    RemoveUpdatingProp(this);
}

// @00427740 Called every frame while the prop is shown (it is in the updating list from BuildFrame to Hide).
void FrameProp::Update(float seconds)
{
    if (frame == NULL)
        return;

    if (actorIndex != -1 && followMode != 0) {
        Float3 origin = { 0.0f, 0.0f, 0.0f };
        float m[16];
        PlacementMatrix(m, direction, up, origin);
        if (followMode == 2) {
            float placed[16];
            MultiplyMatrix(placed, m, ActorMatrix(SimulationActor(actorIndex)));
            memcpy(m, placed, sizeof m);
        } else {
            const float *actor = ActorMatrix(SimulationActor(actorIndex));
            m[12] = actor[12];                                            // @0047e330 set the translation row
            m[13] = actor[13];
            m[14] = actor[14];
        }
        ApplyFrameTransform(frame, GetFrameParent(frame), m);
    }

    if (body == NULL)
        return;
    float before = body->GetTime();
    float duration = body->GetDuration();
    body->Advance(seconds, 0);
    float after = body->GetTime();

    // Where the clip would be without looping. Inside the clip, the messages between the two times run.
    float end = seconds + before;
    if (end <= duration && !(end < 0.0f)) {
        FireMessages(before, after);
        return;
    }

    // Past an end of the clip: past the start when playing backwards, else past the end. The body has already wrapped or
    // stopped; the messages run in two pieces, the way the time went.
    bool pastStart = end < 0.0f;
    float reached = pastStart ? 0.0f : duration;
    switch (body->GetLoopMode()) {
    case 1:         // loops: on from the other end
        FireMessages(before, reached);
        FireMessages(pastStart ? duration : 0.0f, after);
        break;
    case -1:        // back and forth: back from the same end
        FireMessages(before, reached);
        FireMessages(reached, after);
        break;
    default:        // stops at the end
        FireMessages(before, reached);
        if (hideAtEnd)
            VHide();
        break;
    }
}

// @00427b50 Forwards (from <= to) the messages with from < time <= to run, in time order; backwards the ones with
// to <= time < from, latest first.
void FrameProp::FireMessages(float from, float to)
{
    if (!(to < from)) {
        for (int i = 0; i < messages.keys.count; i++) {
            float time = messages.keys.At(i);
            if (time > from && time <= to)
                RunMessage(i);
        }
    } else {
        for (int i = messages.keys.count - 1; i >= 0; i--) {
            float time = messages.keys.At(i);
            if (time < from && time >= to)
                RunMessage(i);
        }
    }
}

// Runs message number index: its script function, with this prop (and its actor, when it follows one) as the caller.
void FrameProp::RunMessage(int index)
{
    String name = messages.GetAt(index);
    RunScript(name.c_str());
}

// Runs a script function with this prop (and its actor, when it follows one) as the caller.
void FrameProp::RunScript(const char *functionName)
{
    ScriptCaller caller;
    if (actorIndex != -1) {
        void *soul = DynamicCast(SimulationSoul(actorIndex), kSoulType, kSqueezilSoulType);
        ScriptCallerInitSoul(&caller, soul, actorIndex, this);
    } else {
        ScriptCallerInit(&caller, -1, this);
    }
    PostFunction(functionName, &caller, 0, 0);
}

// @00427f60
Prop *FrameProp::FindChild(const String &name)
{
    for (int i = 0; i < childNames.count; i++) {
        String childName = childNames.At(i);
        if (StringEqual(&childName, &name))
            return children.At(i);
    }
    return NULL;
}

// @00427e30
void FrameProp::SetCycleTime(float time)
{
    if (body)
        body->SetTime(time);
}

// @00427e50
void FrameProp::SetCycleDirection(int d)
{
    if (body)
        body->SetDirection(d);
}

// @00427e70
void FrameProp::SetCycleLoopMode(int mode)
{
    if (body)
        body->SetLoopMode(mode);
}

// @00427e90 Returns a 4x4 matrix by value in the original (result is the caller's space). There must be a body.
float *FrameProp::GetPartMatrix(float *result)
{
    body->pose->Find(kBodyPartName)->ToMatrix(result);
    return result;
}

// @00427f20
void FrameProp::SetFollowMode(int mode)
{
    followMode = mode;
}

// @00427f10
void FrameProp::SetActor(int index)
{
    actorIndex = index;
}

// @00427f30
void FrameProp::SetHideAtEnd(int hide)
{
    hideAtEnd = hide;
}

// @00427f40
int FrameProp::HasBody()
{
    return body != NULL;
}
