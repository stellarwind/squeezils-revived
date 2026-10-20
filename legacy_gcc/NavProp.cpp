// NavProp: see NavProp.h. Reconstructed from SQZ.EXE 0x429380-0x429dc0.
#include "NavProp.h"
#include "Scene.h"

static void *const kNavPropVtable = (void *)0x49ddac;
static const char *const kNoValue = "none";          // a surface property's value until the prop gives one (0x4b9cf4)

// The current place's surface list. The original casts the current simulation to a PlaceSimulation at every use.
static void *CurrentSurfaces()
{
    return GetPlaceSurfaces(DynamicCast(GetCurrentSimulation(), kLocationSimulationType, kPlaceSimulationType));
}

// @00429380 (+0xf8 instance is left unset until BuildFrame)
NavProp *NavProp::Init(Float3 pos, Float3 scl, Float3 dir, Float3 upVector, StringList *names, MorphCycle *clip,
                       CycleMessageTable *cycleMessages, StepEventTable *stepEvents, float stepPeriod, int keepRemainder,
                       float clipStart, IniReader *ini, const char *surfaceName, const char *natFile, IniValueTable *values)
{
    CycleProp::Init(pos, scl, dir, upVector, names, clip, cycleMessages, stepEvents, stepPeriod, keepRemainder, clipStart);
    ::new (&properties) IniValueTable();
    vtbl = (void **)kNavPropVtable;
    surface = LoadSurface(CurrentSurfaces(), ini, surfaceName, natFile);

    LoadedSurface *loaded = LoadedSurfaceAt(CurrentSurfaces(), surface);
    for (int i = 0; i < loaded->typeNames.count; i++)
        properties.Insert(loaded->typeNames.At(i), String(kNoValue));
    sizeSquared = loaded->sizeSquared;
    for (int i = 0; i < values->keys.count; i++)
        properties.Insert(values->keys.At(i), values->GetAt(i));
    return this;
}

// @004296d0 (+0xf8 instance and +0x12c sizeSquared are left unset; Clone sets the size)
NavProp *NavProp::InitCopy(Float3 pos, Float3 scl, Float3 dir, Float3 upVector, StringList *names, MorphCycle *clip,
                           CycleMessageTable *cycleMessages, StepEventTable *stepEvents, float stepPeriod, int keepRemainder,
                           float clipStart, int loadedSurface, IniValueTable *values)
{
    CycleProp::Init(pos, scl, dir, upVector, names, clip, cycleMessages, stepEvents, stepPeriod, keepRemainder, clipStart);
    ::new (&properties) IniValueTable();
    vtbl = (void **)kNavPropVtable;
    surface = loadedSurface;
    for (int i = 0; i < values->keys.count; i++)
        properties.Insert(values->keys.At(i), values->GetAt(i));
    return this;
}

// @00429a40
void NavProp::Destruct()
{
    vtbl = (void **)kNavPropVtable;
    properties.~IniValueTable();
    CycleProp::Destruct();
}

// @004296b0
Prop *NavProp::Destroy(int flags)
{
    Destruct();
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @004298f0 The copy shares the template's surface (it is loaded once) and gets its own property table. Unlike CycleProp's
// Clone, the copy starts on child 0.
Prop *NavProp::Clone(FrameProp *newOwner)
{
    NavProp *copy = (NavProp *)Operator_New(sizeof(NavProp));
    if (copy)
        copy->InitCopy(position, scale, direction, up, &childNames, cycle, &messages, &events, period, carry, startTime,
                       surface, &properties);
    copy->owner = newOwner;
    copy->sizeSquared = sizeSquared;
    copy->isInstance = 1;
    return copy;
}

// @00429ab0 The copy of the surface goes where the body part's frame (or the prop's frame) is in the scene.
// (The original then fills a placement record {instance, 0, matrix} on the stack and drops it unused.)
IDirect3DRMFrame *NavProp::BuildFrame(IDirect3DRMFrame *parent)
{
    CycleProp::BuildFrame(parent);
    float m[16];
    GetFrameMatrix(GetPartFrame(), GetScene()->GetSceneChild(), m);     // GetPartFrame is inlined here in the original
    instance = AddSurfaceInstance(CurrentSurfaces(), this, surface, m, &properties);
    return frame;
}

// @00429c20
void NavProp::Hide()
{
    FrameProp::Hide();
    RemoveSurfaceInstance(CurrentSurfaces(), instance);
}

// @00429c60 While shown, the copy of the surface is moved to where the prop is now. A prop without a body of its own is only
// moved when some prop up its owner chain has one (otherwise nothing animates it). (The original also fills an unused
// placement record here, like BuildFrame.)
void NavProp::Update(float seconds)
{
    FrameProp::Update(seconds);
    if (!CallVirtual<int>(this, PROP_IS_SHOWN))
        return;
    if (body == NULL) {
        FrameProp *p = CallVirtual<FrameProp *>(this, PROP_GET_OWNER);
        while (p && !p->HasBody())
            p = CallVirtual<FrameProp *>(p, PROP_GET_OWNER);
        if (p == NULL)
            return;
    }
    float m[16];
    GetFrameMatrix(GetPartFrame(), GetScene()->GetSceneChild(), m);
    MoveSurfaceInstance(CurrentSurfaces(), instance, m);
}

// @00429da0
int NavProp::GetSizeSquared(float *out)
{
    *out = sizeSquared;
    return 1;
}
