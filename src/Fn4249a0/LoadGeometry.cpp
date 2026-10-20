// 0x4242b0 (cdecl): LoadGeometry - load the geometry file of one [GeometryResources] entry (text after ';' dropped)
// and register its frame under the key in the top geometry table, replacing an older one. With SqgifyAllFiles
// the .sqg version is loaded, or made from the .wrl and saved first; UseMeshForGeomResources wraps one mesh in a
// new frame instead of building a frame per group. Returns the frame, 0 on failure.
#include <windows.h>
#include <d3drm.h>
#include <string.h>
#pragma intrinsic(strlen, strcpy)
#include "IniReader/IniReader.h"

struct GeomRenderer { char pad[0x2c]; LPDIRECT3DRM d3drm; };
extern GeomRenderer *g_renderer;                               // 0x4c75dc
struct GeomScene { LPDIRECT3DRMFRAME CreateFrameUnder(LPDIRECT3DRMFRAME parent); };   // 0x476f90
extern GeomScene *g_scene;                                     // 0x4c75e0
struct Splash { virtual void v0(); virtual void Step(); };
extern Splash *g_splash;                                       // 0x4c75e8
struct SqgMesh { char pad[8]; LPDIRECT3DRMVISUAL visual; ~SqgMesh(); };   // dtor 0x471990
struct SqgModel {
    SqgModel();                                                // 0x4719d0
    ~SqgModel();                                               // 0x471a00
    int Load(const char *file, int flags);                     // 0x472560
    int Import(const char *file, int flags);                   // 0x472480
    int Save(const char *file);                                // 0x472b90
    SqgMesh *CreateMesh(LPDIRECT3DRM d3drm);                   // 0x472f60
    LPDIRECT3DRMFRAME BuildFrame(LPDIRECT3DRM d3drm);          // 0x4731f0
    char d[0x2c];
};
struct GeomTable {
    virtual int Insert(const String &k, LPDIRECT3DRMFRAME const &v, int at);   // slot 0
    virtual int Contains(const String &k);                                     // slot 1
    LPDIRECT3DRMFRAME Get(const String &k);                                    // 0x467a50
};
GeomTable *TopGeometryTable();                                 // 0x424dc0
void SqgSetErrorCallback(void *fn);                            // 0x4736f0
void SqgSetTextureLookup(void *fn);                            // 0x473700
void SqgErrorLog(const char *message);                         // 0x4241b0
struct Resource;
Resource *OpenResource(const char *name);                      // 0x424010
LPDIRECT3DRMFRAME OpenFile(LPDIRECT3DRM d3drm, const char *path, Resource *(*lookup)(const char *));   // 0x47a580
void DebugLog(const char *format, ...);                        // 0x419290
extern const char kLoadingGeom[], kSqgifyAllFiles[], kUseMeshForGeom[], kSqgExt[], kLoadedSqg[],
    kSavedToSqg[], kCouldNotSqgify[], kFailed[], kOk[];
    // 0x4bb6c8 0x4bb6b8 0x4bb6a0 0x4bb698 0x4bb688 0x4bb678 0x4bb658 0x4ba08c 0x4b9dbc

LPDIRECT3DRMFRAME LoadGeometry(const char *key, const char *value)
{
    int i = 0;
    String copy(value);
    char *path = (char *)copy.c_str();
    for (; i < (int)strlen(path); i++)
        if (path[i] == ';')
            break;
    if (i < (int)strlen(path))
        path[i] = 0;
    DebugLog(kLoadingGeom, key, path);
    g_splash->Step();
    IniReader ini;
    int sqgify = ini.GetInt(kSqgifyAllFiles, 0);
    int useMesh = ini.GetInt(kUseMeshForGeom, 0);
    LPDIRECT3DRMFRAME frame;
    if (sqgify) {
        SqgSetErrorCallback(SqgErrorLog);
        SqgSetTextureLookup(OpenResource);
        SqgModel *model = new SqgModel;
        String sqgName(path);
        char *name = (char *)sqgName.c_str();
        strcpy(name + strlen(name) - 4, kSqgExt);
        if (model->Load(name, 0)) {
            DebugLog(kLoadedSqg);
            if (useMesh) {
                frame = g_scene->CreateFrameUnder(0);
                SqgMesh *mesh = model->CreateMesh(g_renderer->d3drm);
                frame->AddVisual(mesh->visual);
                if (mesh)
                    delete mesh;
            } else {
                frame = model->BuildFrame(g_renderer->d3drm);
            }
        } else if (model->Import(path, 0)) {
            DebugLog(kSavedToSqg);
            model->Save(name);
            if (useMesh) {
                frame = g_scene->CreateFrameUnder(0);
                SqgMesh *mesh = model->CreateMesh(g_renderer->d3drm);
                frame->AddVisual(mesh->visual);
                if (mesh)
                    delete mesh;
            } else {
                frame = model->BuildFrame(g_renderer->d3drm);
            }
        } else {
            DebugLog(kCouldNotSqgify);
            frame = OpenFile(g_renderer->d3drm, path, OpenResource);
        }
        if (model)
            delete model;
    } else {
        frame = OpenFile(g_renderer->d3drm, path, OpenResource);
    }
    if (!frame) {
        DebugLog(kFailed);
        return 0;
    }
    DebugLog(kOk);
    if (TopGeometryTable()->Contains(String(key)))
        TopGeometryTable()->Get(String(key))->Release();
    TopGeometryTable()->Insert(String(key), frame, -1);
    return frame;
}
