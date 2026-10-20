// SQZ.EXE 0x427740: follow an actor, advance the clip and dispatch crossed messages.
#include "Vector3f/Vector3f.h"
#include "FrameProp/FrameProp.h"

struct FrameTickMatrix {
    float m[16];
    FrameTickMatrix() {}
    ~FrameTickMatrix(); // 0x402310
    FrameTickMatrix operator*(const FrameTickMatrix &other); // 0x47dea0
    FrameTickMatrix &SetTranslation(const Vector3f &position); // 0x47e330
    void Set(const Vector3f &right, const Vector3f &up, const Vector3f &direction, const Vector3f &position) {
        m[0] = right.x; m[1] = right.y; m[2] = right.z; m[3] = 0;
        m[4] = up.x; m[5] = up.y; m[6] = up.z; m[7] = 0;
        m[8] = direction.x; m[9] = direction.y; m[10] = direction.z; m[11] = 0;
        m[12] = position.x; m[13] = position.y; m[14] = position.z; m[15] = 1;
    }
    void CopyInline(const FrameTickMatrix &other) {
        m[0] = other.m[0]; m[1] = other.m[1]; m[2] = other.m[2]; m[3] = other.m[3];
        m[4] = other.m[4]; m[5] = other.m[5]; m[6] = other.m[6]; m[7] = other.m[7];
        m[8] = other.m[8]; m[9] = other.m[9]; m[10] = other.m[10]; m[11] = other.m[11];
        m[12] = other.m[12]; m[13] = other.m[13]; m[14] = other.m[14]; m[15] = other.m[15];
    }
};
struct FrameTickActor {
    virtual void slot0(); virtual void slot1(); virtual void slot2();
    virtual FrameTickMatrix *GetMatrix(); // slot 3
};
struct FrameTickSimulation { FrameTickActor *GetActor(int id); }; // 0x414050
FrameTickSimulation *GetSimulation(); // 0x420690
struct FrameTickScene {
    void *Parent(void *frame); // 0x4772a0
    void Apply(void *frame, void *parent, const FrameTickMatrix &matrix); // 0x4770d0
};
extern FrameTickScene *g_scene; // 0x4c75e0
struct FrameTickBase {
    char pad[0x1c];
    Vector3f direction, up; // +0x1c, +0x28
    void *frame; // +0x34
};
struct FrameTickBody {
    virtual void slot0(); virtual void slot1(); virtual void slot2(); virtual void slot3();
    virtual void slot4(); virtual void Advance(float seconds, int skipApply); // slot 5
    virtual void slot6(); virtual void slot7(); virtual float Time(); // slot 8
    virtual void slot9(); virtual void slot10(); virtual float Duration(); // slot 11
    virtual int LoopMode(); // slot 12
};
struct FrameTickHide {
    virtual void slot0(); virtual void slot1(); virtual void slot2();
    virtual void slot3(); virtual void slot4(); virtual void Hide(); // slot 5
};

void FrameProp::Update(float seconds)
{
    FrameTickBase *base = (FrameTickBase *)this;
    if (!base->frame)
        return;
    if (actorIndex != -1 && followMode) {
        FrameTickMatrix matrix;
        Vector3f right = base->up.Cross(base->direction);
        matrix.Set(right, base->up, base->direction, Vector3f(0, 0, 0));
        if (followMode == 2)
            matrix.CopyInline(matrix * *GetSimulation()->GetActor(actorIndex)->GetMatrix());
        else {
            FrameTickMatrix *actor = GetSimulation()->GetActor(actorIndex)->GetMatrix();
            matrix.SetTranslation(*(Vector3f *)&actor->m[12]);
        }
        void *parent = g_scene->Parent(base->frame);
        g_scene->Apply(base->frame, parent, matrix);
    }
    if (!body)
        return;
    float before = ((FrameTickBody *)body)->Time();
    float duration = ((FrameTickBody *)body)->Duration();
    ((FrameTickBody *)body)->Advance(seconds, 0);
    float after = ((FrameTickBody *)body)->Time();
    float end = seconds + before;
    if (end <= duration && !(end < 0.0f)) {
        FireTimeMessages(before, after);
        return;
    }
    int mode = ((FrameTickBody *)body)->LoopMode();
    int pastStart = *(unsigned *)&end > 0x80000000U;
    switch (mode) {
    case 1:
        FireTimeMessages(before, pastStart ? 0.0f : duration);
        FireTimeMessages(pastStart ? duration : 0.0f, after);
        break;
    case -1:
        FireTimeMessages(before, pastStart ? 0.0f : duration);
        FireTimeMessages(pastStart ? 0.0f : duration, after);
        break;
    default:
        FireTimeMessages(before, pastStart ? 0.0f : duration);
        if (hideAtEnd)
            ((FrameTickHide *)this)->Hide();
        break;
    }
}
