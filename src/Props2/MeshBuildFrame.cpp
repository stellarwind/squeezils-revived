// 0x4265f0: attach the geometry to a new frame and apply the prop's pose and scale.
#include "Renderer2/Renderer2.h"
#include "Vector3f/Vector3f.h"
#include "String/String.h"
struct MeshBuildMatrix {
    float m[16];
    MeshBuildMatrix() {}
    ~MeshBuildMatrix(); // 0x402310
    void Set(const Vector3f &right, const Vector3f &up, const Vector3f &direction, const Vector3f &position) {
        m[0] = right.x; m[1] = right.y; m[2] = right.z; m[3] = 0;
        m[4] = up.x; m[5] = up.y; m[6] = up.z; m[7] = 0;
        m[8] = direction.x; m[9] = direction.y; m[10] = direction.z; m[11] = 0;
        m[12] = position.x; m[13] = position.y; m[14] = position.z; m[15] = 1;
    }
};
struct MeshBuildScene {
    LPDIRECT3DRMFRAME CreateFrameUnder(LPDIRECT3DRMFRAME parent); // 0x476f90
    void AddVisual(LPDIRECT3DRMFRAME frame, LPDIRECT3DRMFRAME geometry); // 0x4773a0
    void ApplyTransform(LPDIRECT3DRMFRAME frame, LPDIRECT3DRMFRAME parent, const MeshBuildMatrix &matrix); // 0x4770d0
};
extern MeshBuildScene *g_meshBuildScene;
LPDIRECT3DRMFRAME FindMeshGeometry(const char *name); // 0x4246f0
inline int operator!=(const Vector3f &a, const Vector3f &b)
{
    return !(b.x == a.x && a.y == b.y && a.z == b.z);
}
struct MeshBuildProp {
    void *vtable;
    Vector3f position, scale, direction, up;
    void *frame, *owner;
    int isInstance;
    String name;
    LPDIRECT3DRMFRAME BuildFrame(LPDIRECT3DRMFRAME parent);
};
LPDIRECT3DRMFRAME MeshBuildProp::BuildFrame(LPDIRECT3DRMFRAME parent)
{
    name.c_str();
    LPDIRECT3DRMFRAME geometry = FindMeshGeometry(name.c_str());
    LPDIRECT3DRMFRAME result = g_meshBuildScene->CreateFrameUnder(parent);
    g_meshBuildScene->AddVisual(result, geometry);
    MeshBuildMatrix matrix;
    Vector3f right = up.Cross(direction);
    matrix.Set(right, up, direction, position);
    g_meshBuildScene->ApplyTransform(result, parent, matrix);
    if (scale != Vector3f(1, 1, 1))
        result->AddScale(D3DRMCOMBINE_BEFORE, scale.x, scale.y, scale.z);
    return result;
}
