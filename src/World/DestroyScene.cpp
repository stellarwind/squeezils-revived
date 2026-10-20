// World::DestroyScene, SQZ.EXE 0x435b30.
#include <windows.h>
#include <d3drm.h>
#include "World/World.h"
#include "World/WorldRuntime.h"

#include "World/WorldNavMesh.h"
void ClearWorldObjects();                       // 0x42d010

void World::DestroyScene()
{
    gWorldScene->DestroyFrame((IDirect3DRMFrame *)ambientLight);
    ((IDirect3DRMFrame *)ambientLight)->Release();
    ambientLight = 0;
    gWorldScene->DestroyFrame((IDirect3DRMFrame *)directionalLight);
    ((IDirect3DRMFrame *)directionalLight)->Release();
    directionalLight = 0;
    ClearWorldObjects();
    gWorldScene->DeleteSceneChild();
    delete navMesh;
    navMesh = 0;
}
