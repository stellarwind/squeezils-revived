// BodyPart and AnimatedBody: see BodyPart.h. Reconstructed from SQZ.EXE 0x41d870-0x41e30d.
#include "BodyPart.h"
#include "Scene.h"
#include "Original.h"

// The containers have no virtual destructor in the original either (Container.h), and the objects deleted here are exactly the
// classes the original deletes, so the warning does not apply.
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"

static void *const kBodyPartVtable     = (void *)0x49d7c4;
static void *const kAnimatedBodyVtable = (void *)0x49d7d4;

// ---- BodyPart ----

// @0041d870
BodyPart *BodyPart::Init(SqgModel *partModel)
{
    vtbl = kBodyPartVtable;
    model = partModel;
    morph = NULL;
    frame = NULL;
    return this;
}

// @0041d8a0
BodyPart *BodyPart::Init()
{
    return Init((SqgModel *)NULL);
}

// @0041d8c0
BodyPart *BodyPart::Init(const BodyPart *like)
{
    return Init(like->model);
}

// @0041d8e0 The mesh is the part's own; the model and the frame belong to others.
void BodyPart::Destruct()
{
    vtbl = kBodyPartVtable;
    if (morph) {
        morph->Destroy();
        Operator_Delete(morph);
    }
}

// @0041d900
// Places the part's frame (relative to `ref`) and, if it has a morphing mesh, applies the pose's weights to it.
// The number of weights used is capped by the number of morph targets the model has.
void BodyPart::Apply(IDirect3DRMFrame *ref, const PartPose *pose)
{
    frame->SetPosition(ref, pose->position.x, pose->position.y, pose->position.z);
    frame->SetOrientation(ref, pose->direction.x, pose->direction.y, pose->direction.z,
                               pose->up.x, pose->up.y, pose->up.z);

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
    model->Morph(morph, targets, pose->weights.data);
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

// ---- AnimatedBody ----

// @0041d9e0
AnimatedBody *AnimatedBody::Init()
{
    ::new (&partTable) BodyPartTable();
    vtbl = kAnimatedBodyVtable;
    rootFrame = NULL;
    cycle = NULL;
    time = 0.0f;
    hint = 0;
    direction = 1;
    loopMode = 1;
    previousPose = NULL;
    blendLeft = 0.0f;
    pose = NULL;
    return this;
}

// @0041dab0
AnimatedBody *AnimatedBody::InitCopy(const AnimatedBody *like)
{
    ::new (&partTable) BodyPartTable();
    rootFrame = NULL;
    cycle = NULL;
    time = 0.0f;
    hint = 0;
    direction = 1;
    loopMode = 1;
    vtbl = kAnimatedBodyVtable;
    previousPose = NULL;
    blendLeft = 0.0f;
    for (int i = 0; i < like->partTable.keys.count; i++) {
        BodyPart *copy = (BodyPart *)Operator_New(sizeof(BodyPart));
        if (copy)
            copy->Init(like->partTable.values.data[i]);
        partTable.Insert(like->partTable.keys.data[i], copy, -1);
    }
    pose = NULL;
    return this;
}

// @0041dc70
void AnimatedBody::Destruct()
{
    vtbl = kAnimatedBodyVtable;
    for (int i = 0; i < partTable.keys.count; i++) {
        BodyPart *part = partTable.values.data[i];
        if (part) {
            part->Destruct();
            Operator_Delete(part);
        }
    }
    delete pose;
    delete previousPose;
    partTable.~BodyPartTable();
}

// @0041dd30
int AnimatedBody::AddPart(const char *name, BodyPart *part)
{
    return partTable.Insert(String(name), part, -1);
}

// @0041dda0
void AnimatedBody::Build(IDirect3DRMFrame *parent)
{
    rootFrame = GetScene()->CreateFrameUnder(parent);
    if (rootFrame)
        rootFrame->AddRef();               // this body keeps its own reference to the root frame
    for (int i = 0; i < partTable.keys.count; i++)
        partTable.values.data[i]->Build(rootFrame);
}

// @0041ddf0
void AnimatedBody::DestroyFrames(IDirect3DRMFrame *parent)
{
    parent->DeleteChild(rootFrame);
    rootFrame->Release();
    rootFrame = NULL;
}

// @0041de20 The pose table is made once, from the first clip, and kept. With blend, the pose the body is in now becomes the
// previous pose and fades out over the new clip's period; without, there is none.
void AnimatedBody::SetCycle(MorphCycle *clip, int blend)
{
    time = 0.0f;
    hint = 0;
    cycle = clip;
    direction = 1;
    if (pose == NULL)
        pose = clip->NewPoseTable();
    if (blend) {
        delete previousPose;
        previousPose = new PoseTable(*pose);
        blendLeft = clip->Period();
    } else {
        delete previousPose;
        previousPose = NULL;
    }
}

// @0041df10 The parts and the poses are in the same order (both by part name).
void AnimatedBody::ApplyPose()
{
    for (int i = 0; i < partTable.keys.count; i++)
        partTable.values.data[i]->Apply(rootFrame, pose->At(i));
}

// @0041df50 skipApply: move the time on but leave the parts as they are.
void AnimatedBody::Advance(float seconds, int skipApply)
{
    float step = (float)direction * seconds;
    step = step * (1.0f / cycle->Speed());          // one second of play is 1/speed of the clip

    if (previousPose) {
        // Changing clip: the new clip does not move yet; the pose fades from the old one towards it.
        float amount = step < 0.0f ? -step : step;
        blendLeft -= amount;
        if (blendLeft < 0.0f) {
            delete previousPose;
            previousPose = NULL;
            blendLeft = 0.0f;
            if (!skipApply) {
                hint = cycle->SamplePose(pose, time, hint);
                ApplyPose();
            }
            return;
        }
        if (!skipApply) {
            pose->Assign(*previousPose);
            hint = cycle->BlendPose(pose, time, 1.0f - blendLeft / cycle->Period(), hint);
            ApplyPose();
        }
        return;
    }

    float duration = cycle->Duration();
    float moved = time + step;
    if (moved > duration) {
        // ran past the end
        if (loopMode == 0 || cycle->NoLoop()) {
            time = duration;
            direction = 0;
        } else if (loopMode == -1) {
            // back and forth: turn round; the step just taken is taken back
            direction = -direction;
            float back = time - step;
            if (back > 0.0f)
                time = back;
        } else {
            time = time + step;
            if (duration == 0.0f) {
                time = 0.0f;
            } else {
                while (duration < time)
                    time = time - duration;
            }
        }
    } else if (moved >= 0.0f) {
        time = moved;
    } else {
        // ran back past the start
        if (loopMode == -1) {
            direction = -direction;
            float back = time - step;
            if (back <= duration)
                time = back;
        } else if (loopMode == 0) {
            time = 0.0f;
            direction = 0;
        } else {
            time = moved;
            if (duration == 0.0f) {
                time = 0.0f;
            } else if (time < 0.0f) {
                do {
                    time = duration + time;
                } while (time < 0.0f);
            }
        }
    }

    if (!skipApply) {
        hint = cycle->SamplePose(pose, time, hint);
        ApplyPose();
    }
}

// @0041e2d0
void AnimatedBody::SetDirection(int d)
{
    direction = d < 0 ? -1 : (d > 0 ? 1 : 0);
}

// @0041e2f0
void AnimatedBody::SetLoopMode(int m)
{
    loopMode = m < 0 ? -1 : (m > 0 ? 1 : 0);
}

// @0041e230
// Places the root frame (relative to `ref`) from a 4x4 matrix: position m[12..14], direction m[8..10], up m[4..6].
void AnimatedBody::SetRootTransform(IDirect3DRMFrame *ref, const float *m)
{
    rootFrame->SetPosition(ref, m[12], m[13], m[14]);
    rootFrame->SetOrientation(ref, m[8], m[9], m[10], m[4], m[5], m[6]);
}

// @0041e2a0
float AnimatedBody::GetBlendFraction()
{
    if (previousPose == NULL)
        return -1.0f;
    return 1.0f - blendLeft / cycle->Period();
}
