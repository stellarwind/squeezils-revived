// World::CreateDecal, SQZ.EXE 0x435bb0.
#include "World/World.h"
#include "World/WorldDecal.h"
#include "World/WorldRuntime.h"

IDirect3DRMFrame *World::CreateDecal(IDirect3DRMTexture *texture, WorldVector position,
    WorldDecalSettings settings, IDirect3DRMFrame *parent)
{
    return gWorldScene->CreateDecalFrame(texture, position, settings, parent);
}
