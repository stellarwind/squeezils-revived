// SQZ.EXE 0x40e480 (thiscall, ret 4): NavSurfaceList::LoadGround - load the place's own ground. [Defaults] SurfaceFile names
// its .nat file; a name without a backslash is taken relative to the ini's folder, an empty one becomes <folder>\placexxx.nat
// and no file is read. Surface type 0 is "none"; the ground's instance has every type bound to "none" unless the ini's
// [SurfaceBindingList] says otherwise. The ground becomes loaded surface 0 (only if none is loaded yet) and a new instance.
#include <string.h>
#pragma intrinsic(strlen, strcpy, strcat)
#include "IniReader/IniReader.h"

struct NavMesh;
struct GroundWorld { char pad[0xc]; NavMesh *navMesh; };   // 0x0c
GroundWorld *GetWorld();                                     // 0x4206a0
void DebugLog(const char *format, ...);                      // 0x419290
extern int g_debugSurfaceTypeOutput;                         // 0x4b9b30

struct GroundLoaded {                                        // 0x58 bytes
    GroundLoaded(NavMesh *mesh, int faces, const char *folder, const char *file, StringList *types);   // 0x40bc60
    char pad[0x58];
};
struct GroundMatrix {                                        // 64 bytes, empty out-of-line destructor 0x402310
    float m[16];
    GroundMatrix() {}
    ~GroundMatrix();
};

struct NavSurfaceList {
    char path[0x124];                                        // 0x000 the ground's .nat path
    Array<GroundLoaded *> loaded;                            // 0x124

    int LoadGround(IniReader *ini);
    // The two readers below ignore this (ported as __stdcall free functions), but the caller passes it in ecx.
    void ReadSurfaceBindings(IniReader *ini, IniValueTable *out);                                      // 0x40e2e0
    int LoadSurfaceTypes(NavMesh *mesh, const char *folder, const char *file, StringList *names);      // 0x40eec0
    int ReadSurfaceList(IniReader *ini);                                                                // 0x40d870
    int AddSurface(void *owner, int loadedId, GroundMatrix *matrix, IniValueTable *properties);         // 0x4100b0
};

int NavSurfaceList::LoadGround(IniReader *ini)
{
    IniReader current;
    g_debugSurfaceTypeOutput = current.GetInt("DebugSurfaceTypeOutput", 0);
    String surfaceFile(ini->GetString("SurfaceFile"));
    int faces = 0;
    String name;
    StringList *types = new StringList;
    types->Insert(String("none"), -1);

    if (strlen(surfaceFile.c_str()) == 0) {
        strcpy(path, "placexxx.nat");
        String file(path);
        strcpy(path, ini->GetDirectory().c_str());
        strcat(path, "\\");
        strcat(path, file.c_str());
    } else {
        strcpy(path, surfaceFile.c_str());
        if (!strchr(path, '\\')) {
            name = String(path);
            strcpy(path, ini->GetDirectory().c_str());
            strcat(path, "\\");
            strcat(path, name.c_str());
            DebugLog("...surface file name relative, setting to %s.\n", path);
        }
        GroundWorld *world = GetWorld();
        faces = LoadSurfaceTypes(world->navMesh, ini->GetDirectory().c_str(), path, types);
    }

    IniValueTable *bindings = new IniValueTable;
    ReadSurfaceBindings(ini, bindings);

    if (loaded.count == 0) {
        GroundWorld *world = GetWorld();
        GroundLoaded *surface = new GroundLoaded(world->navMesh, faces, ini->GetDirectory().c_str(),
                                                 surfaceFile.c_str(), types);
        loaded.Insert(surface, -1);
    }

    GroundMatrix identity;
    identity.m[0] = 1.0f;  identity.m[1] = 0.0f;  identity.m[2] = 0.0f;  identity.m[3] = 0.0f;
    identity.m[4] = 0.0f;  identity.m[5] = 1.0f;  identity.m[6] = 0.0f;  identity.m[7] = 0.0f;
    identity.m[8] = 0.0f;  identity.m[9] = 0.0f;  identity.m[10] = 1.0f; identity.m[11] = 0.0f;
    identity.m[12] = 0.0f; identity.m[13] = 0.0f; identity.m[14] = 0.0f; identity.m[15] = 1.0f;

    IniValueTable *properties = new IniValueTable;
    int i;
    for (i = 0; i < types->count; i++)
        properties->Insert(types->Get(i), String("none"), -1);
    for (i = 0; i < bindings->keys.count; i++)
        properties->Insert(bindings->keys.At(i), bindings->At(i), -1);

    AddSurface(0, 0, &identity, properties);

    delete types;
    delete bindings;
    delete properties;

    if (!ReadSurfaceList(ini))
        DebugLog("Error reading surface list.\n");
    ini->SetSection("Defaults");
    return 1;
}
