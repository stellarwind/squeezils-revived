// SQZ.EXE 0x427380: create the frame, optional clip body and missing children.
#define FRAMEPROP_BUILD_OWN
#include "Renderer2/Renderer2.h"
#include "Vector3f/Vector3f.h"
#include "String/String.h"
#include "FrameProp/FrameProp.h"

struct FrameBuildMatrix {
    float m[16];
    FrameBuildMatrix() {}
    ~FrameBuildMatrix(); // 0x402310
    void Set(const Vector3f &right, const Vector3f &up, const Vector3f &direction, const Vector3f &position) {
        m[0] = right.x; m[1] = right.y; m[2] = right.z; m[3] = 0;
        m[4] = up.x; m[5] = up.y; m[6] = up.z; m[7] = 0;
        m[8] = direction.x; m[9] = direction.y; m[10] = direction.z; m[11] = 0;
        m[12] = position.x; m[13] = position.y; m[14] = position.z; m[15] = 1;
    }
};
struct FrameBuildScene {
    LPDIRECT3DRMFRAME CreateFrameUnder(void *parent); // 0x476f90
    void ApplyTransform(LPDIRECT3DRMFRAME frame, void *parent, const FrameBuildMatrix &matrix); // 0x4770d0
};
extern FrameBuildScene *g_scene; // 0x4c75e0
struct FrameBuildPart {
    char data[0x10];
    FrameBuildPart(); // 0x41d8a0
};
struct FrameBuildBody {
    FrameBuildBody(); // 0x41d9e0
    virtual void AddPart(const char *name, FrameBuildPart *part); // slot 0
    virtual void Build(LPDIRECT3DRMFRAME parent); // slot 1
    virtual void slot2(); virtual void SetCycle(void *cycle, int blend); // slot 3
    virtual void slot4(); virtual void slot5(); virtual void slot6(); virtual void slot7();
    virtual void slot8(); virtual void slot9(); virtual void slot10(); virtual void slot11();
    virtual void slot12(); virtual void slot13(); virtual void SetTime(float time); // slot 14
    char data[0x50 - 4];
};
struct FrameBuildProp {
    virtual void slot0();
    virtual FrameBuildProp *Clone(FrameProp *owner); // slot 1
};
struct FrameBuildNames {
    virtual String Get(int index); // slot 0
    int count;
};
struct FrameBuildChildren {
    virtual FrameBuildProp *Get(int index); // slot 0
    virtual void slot1(); virtual void slot2();
    virtual void Replace(int index, FrameBuildProp *const &value); // slot 3
};
struct FrameBuildState {
    void *vtable;
    Vector3f position, scale, direction, up; // +4, +0x10, +0x1c, +0x28
    LPDIRECT3DRMFRAME frame; // +0x34
    char rest38[0x94 - 0x38];
    FrameBuildBody *body; // +0x94
    void *cycle; // +0x98
    int actorIndex, followMode;
    float startTime; // +0xa4
};
FrameBuildProp *FindPropTemplate(String name, int context, int mode); // 0x42c8b0
extern const char kFrameBodyPartName[]; // 0x4b9a7c, "part"
inline int operator!=(const Vector3f &a, const Vector3f &b)
{
    return !(b.x == a.x && a.y == b.y && a.z == b.z);
}

void FrameProp::BuildOwnFrame(void *parent)
{
    FrameBuildState *state = (FrameBuildState *)this;
    FrameBuildMatrix matrix;
    Vector3f right = state->up.Cross(state->direction);
    matrix.Set(right, state->up, state->direction, state->position);
    state->frame = g_scene->CreateFrameUnder(parent);
    g_scene->ApplyTransform(state->frame, parent, matrix);
    if (state->scale != Vector3f(1, 1, 1))
        state->frame->AddScale(D3DRMCOMBINE_AFTER, state->scale.x, state->scale.y, state->scale.z);
    if (state->cycle) {
        FrameBuildPart *part = new FrameBuildPart;
        state->body = new FrameBuildBody;
        state->body->AddPart(kFrameBodyPartName, part);
        state->body->Build(state->frame);
        state->body->SetCycle(state->cycle, 0);
        state->body->SetTime(state->startTime);
    }
    FrameBuildNames *names = (FrameBuildNames *)((char *)this + 0x40);
    FrameBuildChildren *children = (FrameBuildChildren *)((char *)this + 0x54);
    for (int i = 0; i < names->count; ++i) {
        if (!children->Get(i)) {
            const char *name = names->Get(i).c_str();
            FrameBuildProp *child = FindPropTemplate(String(name), 0, 0)->Clone(this);
            children->Replace(i, child);
        }
    }
}
