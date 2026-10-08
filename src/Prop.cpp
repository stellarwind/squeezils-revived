// Prop: see Prop.h. Reconstructed from SQZ.EXE 0x425e50-0x426c70.
#include <string.h>
#include "Prop.h"
#include "Resources.h"
#include "Scene.h"
#include "Renderer.h"

static void *const kPropVtable       = (void *)0x49db4c;
static void *const kMeshPropVtable   = (void *)0x49db8c;
static void *const kSpritePropVtable = (void *)0x49dbe4;

// ---- Prop ----

// @00425e50
Prop *Prop::Init(Float3 pos, Float3 scl, Float3 dir, Float3 upVector)
{
    vtbl = (void **)kPropVtable;
    position = pos;
    scale = scl;
    direction = dir;
    up = upVector;
    frame = NULL;
    owner = NULL;
    isInstance = 0;
    return this;
}

// @00426080
void Prop::Destruct()
{
    vtbl = (void **)kPropVtable;
    Hide();
}

// @00425f80
Prop *Prop::Destroy(int flags)
{
    Destruct();
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @00425fa0
Prop *Prop::Instantiate(void *newOwner, Float3 pos, Float3 scl, Float3 dir, Float3 upVector)
{
    Prop *copy = VClone(newOwner);
    copy->position = pos;
    copy->scale = scl;
    copy->direction = dir;
    copy->up = upVector;
    return copy;
}

// @00426120 Showing a prop twice only logs; the frame stays as it was.
IDirect3DRMFrame *Prop::Show(IDirect3DRMFrame *parent)
{
    if (frame == NULL) {
        frame = VBuildFrame(parent);
        return frame;
    }
    GameLog("WARNING: tried to show already-visible prop.\n");
    return frame;
}

// @00426160
int Prop::IsShown()
{
    return frame != NULL;
}

// @00426170 Takes the frame out of its parent (which lets go of it); the prop keeps no reference.
void Prop::Hide()
{
    if (frame) {
        IDirect3DRMFrame *parent = GetFrameParent(frame);
        if (parent)
            DeleteChildFromFrame(parent, frame);
        frame = NULL;
    }
}

// @00426230
IDirect3DRMFrame *Prop::GetAttachFrame()
{
    IDirect3DRMFrame *parent = NULL;
    if (frame)
        parent = GetFrameParent(frame);
    IDirect3DRMFrame *ownerFrame = NULL;
    if (owner)
        ownerFrame = OwnerAttachFrame(owner);
    return ownerFrame ? ownerFrame : parent;
}

// Makes a copy of this prop of the same class (the object comes back with its owner set and isInstance 1)
Prop *Prop::VClone(void *newOwner)
{
    return CallVirtual<Prop *>(this, PROP_CLONE, newOwner);
}

// Builds this prop's frame under parent: a mesh puts its geometry there, a sprite a decal
IDirect3DRMFrame *Prop::VBuildFrame(IDirect3DRMFrame *parent)
{
    return CallVirtual<IDirect3DRMFrame *>(this, PROP_BUILD_FRAME, parent);
}

// Applies the scale to a frame this prop built (does nothing for a plain Prop; a sprite sets its decal size)
void Prop::VApplyScale(IDirect3DRMFrame *target, Float3 scl)
{
    CallVirtual<void>(this, PROP_APPLY_SCALE, target, scl);
}

// ---- MeshProp ----

// @004262e0
MeshProp *MeshProp::Init(const char *geometryName, Float3 pos, Float3 scl, Float3 dir, Float3 upVector)
{
    Prop::Init(pos, scl, dir, upVector);
    new (&name) String();
    vtbl = (void **)kMeshPropVtable;
    name = String(geometryName);
    return this;
}

// @00426580
void MeshProp::Destruct()
{
    vtbl = (void **)kMeshPropVtable;
    name.~String();
    Prop::Destruct();
}

// @00426450
Prop *MeshProp::Destroy(int flags)
{
    Destruct();
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @00426470 (the original reads the new object without checking the allocation)
Prop *MeshProp::Clone(void *newOwner)
{
    MeshProp *copy = (MeshProp *)Operator_New(sizeof(MeshProp));
    if (copy)
        copy->Init(name.c_str(), position, scale, direction, up);
    copy->owner = newOwner;
    copy->isInstance = 1;
    return copy;
}

// @004265f0 The geometry is a frame of its own in the geometry tables; it goes under a new frame of this prop's, which gets the
// prop's placement (right = up x direction, then up, direction, position) and, unless it is 1, its scale.
IDirect3DRMFrame *MeshProp::BuildFrame(IDirect3DRMFrame *parent)
{
    IDirect3DRMFrame *geometry = FindGeometry(name.c_str());
    IDirect3DRMFrame *propFrame = GetScene()->CreateFrameUnder(parent);
    AddVisualToFrame(propFrame, geometry);

    float right[3];
    right[0] = up.y * direction.z - up.z * direction.y;
    right[1] = up.z * direction.x - up.x * direction.z;
    right[2] = up.x * direction.y - up.y * direction.x;
    float m[16] = {
        right[0],     right[1],     right[2],     0.0f,
        up.x,         up.y,         up.z,         0.0f,
        direction.x,  direction.y,  direction.z,  0.0f,
        position.x,   position.y,   position.z,   1.0f,
    };
    ApplyFrameTransform(propFrame, parent, m);

    if (!(scale.x == 1.0f && scale.y == 1.0f && scale.z == 1.0f))
        propFrame->AddScale(D3DRMCOMBINE_BEFORE, scale.x, scale.y, scale.z);
    return propFrame;
}

// ---- SpriteProp ----

// @004267b0 The direction and up vector are fixed: a sprite faces +z with y up.
SpriteProp *SpriteProp::Init(const char *textureName, Float3 pos, Float3 scl, const DecalSettings *decal)
{
    Float3 facing = { 0.0f, 0.0f, 1.0f };
    Float3 upward = { 0.0f, 1.0f, 0.0f };
    Prop::Init(pos, scl, facing, upward);
    new (&name) String();
    vtbl = (void **)kSpritePropVtable;
    name = String(textureName);
    settings = *decal;
    return this;
}

// @00426a70
void SpriteProp::Destruct()
{
    vtbl = (void **)kSpritePropVtable;
    name.~String();
    Prop::Destruct();
}

// @00426980
Prop *SpriteProp::Destroy(int flags)
{
    Destruct();
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @004269a0 Like the original, the copy keeps the position, scale and settings but gets the default facing again.
Prop *SpriteProp::Clone(void *newOwner)
{
    SpriteProp *copy = (SpriteProp *)Operator_New(sizeof(SpriteProp));
    if (copy)
        copy->Init(name.c_str(), position, scale, &settings);
    copy->owner = newOwner;
    copy->isInstance = 1;
    return copy;
}

// @00426b00
IDirect3DRMFrame *SpriteProp::BuildFrame(IDirect3DRMFrame *parent)
{
    const char *textureName = name.c_str();
    IDirect3DRMTexture *texture = FindTexture(textureName);
    if (texture == NULL) {
        GameLog("ERROR: sprite %s not loaded from textureResources\n", textureName);
        return NULL;
    }
    IDirect3DRMFrame *decalFrame = GetWorld()->CreateDecal(texture, position, settings, parent);
    VApplyScale(decalFrame, scale);
    return decalFrame;
}

// @00426c00
void SpriteProp::ApplyScale(IDirect3DRMFrame *target, Float3 scl)
{
    GetWorld()->SetDecalSize(target, scl.x, scl.y);
}
