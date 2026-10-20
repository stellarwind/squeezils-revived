// SQZ.EXE 0x40eec0 (cdecl): read a ".nat" surface-type file ("face name" per line) and give every listed face of
// the mesh a type id; new type names are appended to the names set. Returns the number of entries, 0 on error.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma intrinsic(strlen)
#include "String/String.h"
#include "Container/ContainerTable.h"

typedef BufferObjectTable<String, int, PrimitiveArray<int, 1>, 21> TypeIds;
struct NatSpare { char d[0x14]; NatSpare(); ~NatSpare(); };   // ctor 0x45f610, dtor 0x45f700 (never used)
struct NatNames {
    virtual void v0();
    virtual int Insert(const String &name, int hint);   // slot 1
    int count;                                          // 0x04
};
struct NatFace { char pad[0x24]; int type; };           // 0x28 bytes
struct NatMesh {
    char pad[8];
    unsigned faceCount;                                 // 0x08
    NatFace *faces;                                     // 0x0c
};
struct NatApp;
extern NatApp *g_app;                                   // 0x4c75dc
int FatalErrorApp(NatApp *app, const char *fmt, ...);   // 0x475510
int DebugLogAll(const char *format, ...);                 // 0x419290
extern const char *kNatHeader;                          // 0x4b9b34 -> "Nat V1.0 ascii"
extern const char kReadMode[];                          // 0x4ba008
extern const char kCouldNotReadFile[];                  // 0x4b9fec
extern const char kCouldNotOpen[];                      // 0x4b9fc4
extern const char kNoFirstLine[];                       // 0x4b9fa0
extern const char kInvalidHeader[];                     // 0x4b9f54
extern const char kNoFaceCount[];                       // 0x4b9f78
extern const char kInvalidFaceCount[];                  // 0x4b9f30
extern const char kNone[];                              // 0x4b9ef4
extern const char kEntryFormat[];                       // 0x4b9efc "%d %s"
extern const char kNoEntry[];                           // 0x4b9f04

int __stdcall LoadSurfaceTypes(NatMesh *mesh, void *unused, const char *file, NatNames *names)
{
    if (!strlen(file))
        return 0;
    FILE *f = 0;
    while (!f) {
        f = fopen(file, kReadMode);
        if (!f && FatalErrorApp(g_app, kCouldNotReadFile, file) == 2) {
            DebugLogAll(kCouldNotOpen, file);
            return 0;
        }
    }
    char line[256];
    const char *error;
    if (!fgets(line, 0xff, f)) {
        error = kNoFirstLine;
    } else if (!strstr(line, kNatHeader)) {
        error = kInvalidHeader;
    } else if (!fgets(line, 0xff, f)) {
        error = kNoFaceCount;
    } else {
        int entries = atoi(line);
        if (entries > 0) {
            NatSpare spare;
            TypeIds ids;
            int none = 0;
            ids.Insert(String(kNone), none, -1);
            for (int i = 0; i < entries; i++) {
                unsigned face;
                char name[64];
                if (!fgets(line, 0xff, f)) {
                    DebugLogAll(kNoEntry, i);
                    fclose(f);
                    return 0;
                }
                if (sscanf(line, kEntryFormat, &face, name) != 2) {
                    DebugLogAll(kNoEntry, i);
                    fclose(f);
                    return 0;
                }
                if (face < mesh->faceCount) {
                    if (ids.Contains(String(name))) {
                        mesh->faces[face].type = ids.Get(String(name));
                    } else {
                        int id = names->count;
                        ids.Insert(String(name), id, -1);
                        mesh->faces[face].type = names->count;
                        names->Insert(String(name), -1);
                    }
                }
            }
            fclose(f);
            return entries;
        }
        error = kInvalidFaceCount;
    }
    DebugLogAll(error);
    fclose(f);
    return 0;
}
