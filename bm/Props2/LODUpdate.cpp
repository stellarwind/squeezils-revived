// 0x428510: choose a level of detail from the camera and switch the visible child.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
#include "Container/ContainerArray.h"
struct LODViewMatrix {
    float a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
    LODViewMatrix(const LODViewMatrix &o) {
        a0=o.a0; a1=o.a1; a2=o.a2; a3=o.a3;
        a4=o.a4; a5=o.a5; a6=o.a6; a7=o.a7;
        a8=o.a8; a9=o.a9; a10=o.a10; a11=o.a11;
        a12=o.a12; a13=o.a13; a14=o.a14; a15=o.a15;
    }
    ~LODViewMatrix(); // 0x402310
    Vector3f *Position(); // 0x41faf0
    Vector3f *Forward(); // 0x4286c0
};
struct LODViewBody {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual LODViewMatrix *GetMatrix();
};
struct LODViewActor { int pad; LODViewBody *body; };
struct LODViewActors { char pad[0x24]; LODViewActor **data; };
struct LODViewSimulation { LODViewActors *GetActors(); }; // 0x414150
LODViewSimulation *GetLODViewSimulation(); // 0x420690
struct LODUpdatePart {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void *Show(void *parent);
    virtual void Hide();
    virtual void ApplyScale(void *frame, Vector3f scale);
};
struct LODUpdate {
    char pad[0x10];
    Vector3f scale;
    char pad1c[0x54 - 0x1c];
    Array<LODUpdatePart *> children;
    char pad64[0xac - 0x64];
    int current;
    void *Underlying(); // 0x4276b0
    int SelectLevel(Vector3f cameraPosition, Vector3f cameraForward); // 0x4286d0
    void UpdateLevel();
};
void LODUpdate::UpdateLevel()
{
    LODViewMatrix camera(*GetLODViewSimulation()->GetActors()->data[1]->body->GetMatrix());
    int level = SelectLevel(*camera.Position(), *camera.Forward());
    if (level != current) {
        if (current != -1)
            children.Get(current)->Hide();
        current = level;
        if (level != -1) {
            void *frame = children.Get(current)->Show(Underlying());
            children.Get(current)->ApplyScale(frame, scale);
        }
    }
}
