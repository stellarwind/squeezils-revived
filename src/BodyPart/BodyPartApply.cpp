// BodyPart destructor (SQZ.EXE 0x41d8e0) and Apply (0x41d900, slot 0).
#include <d3drm.h>

struct PZMorphMesh { ~PZMorphMesh(); };       // 0x471990 (Destroy), freed by the caller
struct SqgModel {
    char pad[0x24];
    int deltaListCount;                       // 0x24
    void Morph(PZMorphMesh *inst, int count, const float *weights);   // 0x4734e0
};
struct WeightList {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual const float *Data();              // slot 4 (+0x10)
    int count;                                // 0x04
};
struct PartPose {
    float px, py, pz;                         // 0x00
    float dx, dy, dz;                         // 0x0c
    float ux, uy, uz;                         // 0x18
    WeightList weights;                       // 0x24
};
struct BodyPartA {
    virtual void Apply(IDirect3DRMFrame *ref, const PartPose *pose);   // slot 0
    SqgModel *model;                          // 0x04
    PZMorphMesh *morph;                       // 0x08
    IDirect3DRMFrame *frame;                  // 0x0c
    ~BodyPartA();
};

BodyPartA::~BodyPartA()
{
    delete morph;
}

void BodyPartA::Apply(IDirect3DRMFrame *ref, const PartPose *pose)
{
    frame->SetPosition(ref, pose->px, pose->py, pose->pz);
    frame->SetOrientation(ref, pose->dx, pose->dy, pose->dz, pose->ux, pose->uy, pose->uz);
    if (morph == 0)
        return;
    int count = pose->weights.count;
    if (count <= 0)
        return;
    int targets = model->deltaListCount;
    if (targets <= 0)
        return;
    if (targets > count)
        targets = count;
    model->Morph(morph, targets, ((WeightList *)&pose->weights)->Data());
}
