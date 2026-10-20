// LODProp: see LODProp.h. Reconstructed from SQZ.EXE 0x428050-0x4288b0.
#include "LODProp.h"
#include "Scene.h"

static void *const kLODPropVtable = (void *)0x49dcb4;
// A setting that would switch the levels off beyond a distance: while it is not positive (as its raw bits read as an integer),
// as it is in the game, the check never fires.
static const float *const kFarDistance = (const float *)0x4bb908;

// @00428050
LODProp *LODProp::Init(Float3 pos, Float3 scl, Float3 dir, Float3 upVector, StringList *names, MorphCycle *clip,
                       CycleMessageTable *cycleMessages, WeightList *distanceList)
{
    FrameProp::Init(pos, scl, dir, upVector, names, clip, cycleMessages);
    ::new (&distances) WeightList();
    vtbl = (void **)kLODPropVtable;
    level = 0;
    for (int i = 0; i < distanceList->count; i++)
        distances.Insert(distanceList->At(i));

    // a child without a number gets the last one
    for (int i = distances.count; names->count > i; i++)
        distances.Insert(distances.At(i - 1));
    return this;
}

// @00428260
void LODProp::Destruct()
{
    vtbl = (void **)kLODPropVtable;
    distances.~WeightList();
    FrameProp::Destruct();
}

// @00428240
Prop *LODProp::Destroy(int flags)
{
    Destruct();
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @004282d0
Prop *LODProp::Clone(FrameProp *newOwner)
{
    LODProp *copy = (LODProp *)Operator_New(sizeof(LODProp));
    if (copy)
        copy->Init(position, scale, direction, up, &childNames, cycle, &messages, &distances);
    copy->owner = newOwner;
    copy->isInstance = 1;
    return copy;
}

// @004283f0 (The original also fetches the child's name here and does not use it.)
IDirect3DRMFrame *LODProp::BuildFrame(IDirect3DRMFrame *parent)
{
    BuildOwnFrame(parent);
    AddUpdatingProp(this);
    if (level == -1)
        level = 0;
    Prop *child = children.At(level);
    IDirect3DRMFrame *childFrame = child->VShow(GetPartFrame());
    children.At(level)->VApplyScale(childFrame, scale);
    return frame;
}

// @00428510 The camera is entry 1 of the simulation's actor table (CameraMatrix): its position and the direction it looks in
// are the matrix's last row and third row.
void LODProp::UpdateLevel()
{
    const float *camera = CameraMatrix();
    float m[16];
    for (int i = 0; i < 16; i++)
        m[i] = camera[i];
    Float3 position = { m[12], m[13], m[14] };
    Float3 forward  = { m[8], m[9], m[10] };

    int newLevel = SelectLevel(position, forward);
    if (newLevel == level)
        return;
    if (level != -1)
        children.At(level)->VHide();
    level = newLevel;
    if (newLevel != -1) {
        IDirect3DRMFrame *childFrame = children.At(newLevel)->VShow(GetPartFrame());
        children.At(newLevel)->VApplyScale(childFrame, scale);
    }
}

// @004286d0 The squared distance from the camera to this prop's frame picks the first level whose number it is under. Before
// that: when this prop (or the first prop up its owner chain that reports a size) is behind the camera by more than its size,
// nothing is shown (-1). Slot 11 of a prop gives its size squared, slot 9 its owner.
int LODProp::SelectLevel(Float3 cameraPosition, Float3 cameraForward)
{
    float frameMatrix[16];
    GetFrameMatrix(GetPartFrame(), GetScene()->GetSceneChild(), frameMatrix);
    float dx = frameMatrix[12] - cameraPosition.x;
    float dy = frameMatrix[13] - cameraPosition.y;
    float dz = frameMatrix[14] - cameraPosition.z;
    float distanceSquared = dz * dz + dy * dy + dx * dx;

    if (*(const int *)kFarDistance > 0 && *kFarDistance * *kFarDistance < distanceSquared)
        return -1;

    float ahead = dz * cameraForward.z + dy * cameraForward.y + dx * cameraForward.x;
    for (Prop *p = CallVirtual<Prop *>(this, 9); p; p = CallVirtual<Prop *>(p, 9)) {
        float sizeSquared;
        if (CallVirtual<int>(p, 11, &sizeSquared)) {
            // found one: behind the camera by more than its size is out of sight
            if (*(unsigned *)&ahead > 0x80000000u && !(ahead * ahead <= sizeSquared))
                return -1;
            break;
        }
    }

    int last = childNames.count - 1;
    int chosen = 0;
    if (distances.At(0) < distanceSquared) {
        while (last > chosen) {
            chosen++;
            if (!(distances.At(chosen) < distanceSquared))
                break;
        }
    }
    return chosen;
}
