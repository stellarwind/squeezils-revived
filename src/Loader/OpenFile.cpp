// 0x47a580: build a loader for `path` (or "<stdin>" when null) on a fresh frame and hand back its result.
#include <windows.h>
#include <string.h>
#include <d3drm.h>

struct Loader {
    int pad0;
    void *result;
    char pad8[0x368];
    Loader(const char *path, const char *dir, const char *file, IDirect3DRM *rm, void *callback, IDirect3DRMFrame *frame);  // 0x479b10
    ~Loader();                                                          // 0x479b70
    void *operator new(unsigned size);                                  // 0x48ea00
    void operator delete(void *p);                                      // 0x48e9f0
};
extern Loader *g_current;                                               // 0x4c3c44
extern char g_stdin[];                                                  // 0x4c3c38
extern char g_defaultDir[];                                             // 0x4bd220
void DefaultCallback();                                                 // 0x479af0
void SetCurrent(Loader *l);                                             // 0x47a770
void ClearCurrent();                                                    // 0x47a780
void Restore(const char *path);                                         // 0x47a990
IUnknown *Result(Loader *l);                                            // 0x47a570

IUnknown *OpenFile(IDirect3DRM *rm, const char *path, void *callback)
{
    char dir[128], file[128];
    IDirect3DRMFrame *frame;
    Loader *l;

    if (callback == 0)
        callback = DefaultCallback;
    rm->CreateFrame(0, &frame);
    if (path == 0) {
        l = new Loader(g_stdin, g_defaultDir, g_stdin, rm, callback, frame);
    } else {
        strcpy(dir, path);
        char *slash = strrchr(dir, '/');
        if (slash == 0)
            slash = strrchr(dir, '\\');
        char *name = dir;
        if (slash != 0) {
            name = slash + 1;
            *slash = 0;
        }
        strcpy(file, name);
        l = new Loader(path, dir, file, rm, callback, frame);
    }
    SetCurrent(l);
    Restore(path);
    IUnknown *r = Result(l);
    if (l->result == 0) {
        Result(l)->Release();
        r = 0;
    }
    if (l) {
        l->~Loader();
        delete l;
    }
    ClearCurrent();
    return r;
}
