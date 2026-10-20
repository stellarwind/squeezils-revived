// 0x4777b0 (ret 0x54): Scene::CreateQuadFrame - build a one-face, four-vertex mesh of a single colour and put it in a new
// root frame; returns the frame, or 0 when any D3DRM call fails (everything made so far is released).
#include "Scene/Scene.h"
#include "World/WorldDecal.h"

LPDIRECT3DRMFRAME Scene::CreateQuadFrame(WorldVector a, WorldVector b, WorldVector c, WorldVector d,
    WorldVector normal, double red, double green, double blue)
{
    LPDIRECT3DRMMESHBUILDER builder;
    if (renderer->d3drm->CreateMeshBuilder(&builder) < 0)
        return 0;
    if (builder->ReserveSpace(4, 1, 1) < 0) {
        builder->Release();
        return 0;
    }
    builder->AddVertex(a.x, a.y, a.z);
    builder->AddVertex(b.x, b.y, b.z);
    builder->AddVertex(c.x, c.y, c.z);
    builder->AddVertex(d.x, d.y, d.z);
    builder->AddNormal(normal.x, normal.y, normal.z);
    LPDIRECT3DRMFACE face;
    if (builder->CreateFace(&face) < 0) {
        builder->Release();
        return 0;
    }
    face->AddVertexAndNormalIndexed(0, 0);
    face->AddVertexAndNormalIndexed(1, 0);
    face->AddVertexAndNormalIndexed(2, 0);
    face->AddVertexAndNormalIndexed(3, 0);
    face->SetColor(D3DRGB(red, green, blue));
    LPDIRECT3DRMMESH mesh;
    if (builder->CreateMesh(&mesh) < 0) {
        face->Release();
        builder->Release();
        return 0;
    }
    LPDIRECT3DRMFRAME frame;
    if (renderer->d3drm->CreateFrame(0, &frame) < 0) {
        mesh->Release();
        face->Release();
        builder->Release();
        return 0;
    }
    if (frame->AddVisual(mesh) < 0) {
        frame->Release();
        mesh->Release();
        face->Release();
        builder->Release();
        return 0;
    }
    mesh->Release();
    face->Release();
    builder->Release();
    return frame;
}
