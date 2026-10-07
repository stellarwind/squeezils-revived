// BodyPart: see BodyPart.h. Reconstructed from SQZ.EXE 0x41d900-0x41d9ce.
#include "BodyPart.h"
#include "Scene.h"

extern Scene    *GetScene();       // the global at 0x4c75e0
extern Renderer *GetRenderer();    // the global at 0x4c75dc

typedef float *(__attribute__((thiscall)) *WeightDataFn)(WeightArray *);

float *WeightArray::Data() { return ((WeightDataFn)vtbl[4])(this); }

// @0041d900
// Places the part's frame (relative to `ref`) and, if it has a morphing mesh, applies the pose's weights to it.
// The number of weights used is capped by the number of morph targets the model has.
void BodyPart::Apply(IDirect3DRMFrame *ref, const BodyPose *pose)
{
    frame->SetPosition(ref, pose->position[0], pose->position[1], pose->position[2]);
    frame->SetOrientation(ref, pose->direction[0], pose->direction[1], pose->direction[2],
                               pose->up[0], pose->up[1], pose->up[2]);

    if (morph == NULL)
        return;
    int count = pose->weights.count;
    if (count <= 0)
        return;
    int targets = model->deltaListCount;
    if (targets <= 0)
        return;
    if (targets > count)
        targets = count;
    model->Morph(morph, targets, ((WeightArray *)&pose->weights)->Data());
}

// @0041d990
// Makes the part's frame under `parent` and attaches the morphing mesh built from its model.
void BodyPart::Build(IDirect3DRMFrame *parent)
{
    frame = GetScene()->CreateFrameUnder(parent);
    if (model) {
        morph = model->CreateMesh(GetRenderer()->d3drm);
        frame->AddVisual(morph->mesh);
    }
}

// @0041e230
// Places the root frame (relative to `ref`) from a 4x4 matrix: position m[12..14], direction m[8..10], up m[4..6].
void AnimatedBody::SetRootTransform(IDirect3DRMFrame *ref, const float *m)
{
    rootFrame->SetPosition(ref, m[12], m[13], m[14]);
    rootFrame->SetOrientation(ref, m[8], m[9], m[10], m[4], m[5], m[6]);
}

// @0041dda0
void AnimatedBody::Build(IDirect3DRMFrame *parent)
{
    rootFrame = GetScene()->CreateFrameUnder(parent);
    if (rootFrame)
        rootFrame->AddRef();               // this body keeps its own reference to the root frame
    for (int i = 0; i < partCount; i++)
        parts[i]->Build(rootFrame);        // slot 1 of the BodyPart vtable
}

// @0041ddf0
void AnimatedBody::DestroyFrames(IDirect3DRMFrame *parent)
{
    parent->DeleteChild(rootFrame);
    rootFrame->Release();
    rootFrame = NULL;
}
