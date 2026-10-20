// SQZ.EXE 0x40fc10: reuse or load a navigation surface by directory and NAT file.
#include <windows.h>
#include <string.h>
#include "NavSurfaceList.h"
#include "IniReader/IniReader.h"
#pragma intrinsic(strcpy, strcmp, strlen)

struct NavMesh {
    char data[0x2030];
    NavMesh();                                  // 0x47aec0
    ~NavMesh();                                 // 0x47af00
    int Load(const char *file);                  // 0x47c8e0
};
struct LoadedSurface {
    NavMesh *mesh;
    float sizeSquared;
    int faceCount;
    String directory, file;                     // +0x0c, +0x10
    char rest[0x58 - 0x14];
    LoadedSurface(NavMesh *mesh, int faces, const char *folder,
        const char *natFile, StringList *names); // 0x40bc60
};
int __stdcall LoadSurfaceTypes(NavMesh *mesh, const char *folder,
    const char *file, StringList *names);        // 0x40eec0
void DebugLog(const char *format, ...);          // 0x419290
extern const char kLoadingSurface[], kSurfaceAlreadyLoaded[], kSurfaceLoadFailed[], kSurfaceLoadOk[], kNone[];
    // 0x4ba0ac, 0x4ba098, 0x4ba08c, 0x4b9dbc, 0x4b9ef4

int NavSurfaceList::Load(IniReader *ini, const char *navFile, const char *natFile)
{
    String folder = ini->GetDirectory();
    DebugLog(kLoadingSurface, navFile, natFile, folder.c_str());
    for (int i = 0; i < loaded.count; ++i) {
        String current = ini->GetDirectory();
        String knownDirectory = loaded.Get(i)->directory;
        String knownFile = loaded.Get(i)->file;
        const char *currentText = current.c_str();
        const char *knownText = knownDirectory.c_str();
        if (!strcmp(currentText, knownText) && !strcmp(natFile, knownFile.c_str())) {
            DebugLog(kSurfaceAlreadyLoaded);
            return i;
        }
    }
    char saved[128];
    strcpy(saved, kEmptyString);
    if (!strchr(navFile, '\\')) {
        GetCurrentDirectoryA(0x7f, saved);
        SetCurrentDirectoryA(ini->GetDirectory().c_str());
    }
    NavMesh *mesh = new NavMesh;
    if (!mesh->Load(navFile)) {
        DebugLog(kSurfaceLoadFailed);
        delete mesh;
        if (strlen(saved)) SetCurrentDirectoryA(saved);
        return 0;
    }
    DebugLog(kSurfaceLoadOk);
    StringList types;
    types.Insert(String(kNone), -1);
    int faces = LoadSurfaceTypes(mesh, ini->GetDirectory().c_str(), natFile, &types);
    if (strlen(saved)) SetCurrentDirectoryA(saved);
    LoadedSurface *surface = new LoadedSurface(mesh, faces, ini->GetDirectory().c_str(), natFile, &types);
    loaded.Insert(surface, -1);
    return loaded.count - 1;
}
