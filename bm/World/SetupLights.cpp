// World::SetupLights, SQZ.EXE 0x436040.
#include <windows.h>
#include <d3drm.h>
#include "World/World.h"
#include "World/WorldDecal.h"
#include "World/WorldRuntime.h"
#include "IniReader/IniReader.h"

void World::SetupLights(IniReader *reader)
{
    reader->SetSection("Render");
    WorldVector position, direction, up, color;
    color.z = 1.0f; color.y = 1.0f; color.x = 1.0f;
    if (reader->GetFloats("InitialLightPosition", &position.x) == 3 &&
        reader->GetFloats("InitialLightOrientation", &direction.x) == 3) {
        if (directionalLight) {
            gWorldScene->DestroyFrame((IDirect3DRMFrame *)directionalLight);
            ((IDirect3DRMFrame *)directionalLight)->Release();
        }
        up.x = 0.0f; up.y = 1.0f; up.z = 0.0f;
        WorldVector side = up.Cross(direction);
        up = side.Cross(direction);
        directionalLight = gWorldScene->CreateDirectionalLight(color, position, direction, up);
    }
    if (reader->GetFloats("AmbientLightIntensity", &color.x) == 3) {
        if (ambientLight) {
            gWorldScene->DestroyFrame((IDirect3DRMFrame *)ambientLight);
            ((IDirect3DRMFrame *)ambientLight)->Release();
        }
        ambientLight = gWorldScene->CreateAmbientLight(color);
    }
}
