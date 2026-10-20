// World::LoadNavMesh, SQZ.EXE 0x4363a0.
#include <windows.h>
#include <string.h>
#include "World/World.h"
#include "IniReader/IniReader.h"
#include "World/WorldNavMesh.h"
void GameLog(const char *format, ...); // 0x419290

void World::LoadNavMesh(IniReader *reader)
{
    reader->SetSection("Defaults");
    const char *name = reader->GetString("NavMesh");
    if (strlen(name) == 0) {
        GameLog("No nav surface defined.\n");
        return;
    }
    navMesh = new NavMesh;
    GameLog("Loading nav surface %s: ", name);
    char savedDirectory[128];
    strcpy(savedDirectory, kEmptyString);
    if (!strchr(name, '\\')) {
        GetCurrentDirectoryA(127, savedDirectory);
        SetCurrentDirectoryA(reader->GetDirectory().c_str());
    }
    if (!navMesh->Load(name)) {
        GameLog("failed!\n");
        delete navMesh;
        navMesh = 0;
        if (strlen(savedDirectory))
            SetCurrentDirectoryA(savedDirectory);
    } else {
        GameLog("OK.\n");
        if (strlen(savedDirectory))
            SetCurrentDirectoryA(savedDirectory);
    }
}
