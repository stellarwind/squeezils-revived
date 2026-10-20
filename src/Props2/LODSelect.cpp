// 0x4286d0: cull props behind the camera and choose a distance threshold.
#include "Vector3f/Vector3f.h"
#include "Container/ContainerArray.h"
struct LODSelectMatrix {
    float m[16];
    LODSelectMatrix() {}
    ~LODSelectMatrix(); // 0x402310
    const Vector3f &Position() const { return *(const Vector3f *)(m + 12); }
};
struct LODSelectScene {
    void *GetSceneChild(); // 0x477290
    void GetMatrix(void *frame, void *relative, LODSelectMatrix *out); // 0x477120
};
extern LODSelectScene *g_lodSelectScene;
extern float g_lodFarDistance; // 0x4bb908
struct LODSelect {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual LODSelect *GetOwner();
    virtual void v10(); virtual int GetSizeSquared(float *out);
    char pad04[0x40 - 4];
    void *namesVtable;
    int nameCount;
    char pad48[0xb0 - 0x48];
    Array<float> distances;
    void *distanceBufferObject;
    void *Underlying(); // 0x4276b0
    int SelectLevel(Vector3f cameraPosition, Vector3f cameraForward);
};
int LODSelect::SelectLevel(Vector3f cameraPosition, Vector3f cameraForward)
{
    LODSelectMatrix matrix;
    g_lodSelectScene->GetMatrix(Underlying(), g_lodSelectScene->GetSceneChild(), &matrix);
    Vector3f delta = matrix.Position() - cameraPosition;
    float distanceSquared = delta.x * delta.x + delta.z * delta.z + delta.y * delta.y;
    if (*(const int *)&g_lodFarDistance > 0 && g_lodFarDistance * g_lodFarDistance < distanceSquared)
        return -1;
    float ahead = delta.z * cameraForward.z + delta.y * cameraForward.y + delta.x * cameraForward.x;
    for (LODSelect *owner = GetOwner(); owner; owner = owner->GetOwner()) {
        float sizeSquared;
        if (owner->GetSizeSquared(&sizeSquared)) {
            if (*(const unsigned *)&ahead > 0x80000000u && !(ahead * ahead <= sizeSquared))
                return -1;
            break;
        }
    }
    int last = nameCount - 1;
    int chosen = 0;
    if (distances.Get(0) < distanceSquared) {
        while (chosen < last) {
            ++chosen;
            if (!(distances.Get(chosen) < distanceSquared))
                break;
        }
    }
    return chosen;
}
