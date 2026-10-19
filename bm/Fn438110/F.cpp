// SQZ.EXE 0x438110 (ret 4): show or hide the cursor line between two region places.
#define V3_CTOR_OUTOFLINE
#define V3_COPYCTOR
#include "Vector3f/Vector3f.h"

struct CursorFrame { virtual void q0(); virtual void q1(); virtual void __stdcall Release(); };
struct SceneReg {
    void *Find(CursorFrame *frame);                      // 0x477290
    void Attach(void *handle);                           // 0x4773c0
    void Release(void *handle);                          // 0x4773e0
};
struct CursorPlace { Vector3f pos; };
struct CursorRegion { CursorPlace *GetPlace(int id); };  // 0x418140
extern SceneReg *g_scene2;                               // 0x4c75e0
extern CursorFrame *g_cursor;                            // 0x4c7340

struct CursorSoul {
    char pad[0x88];
    CursorRegion *region;                                // 0x88
    int pad8c;
    int current;                                         // 0x90
    int visible;                                         // 0x94
    CursorFrame *Make(Vector3f from, Vector3f to, Vector3f scale);   // 0x436fc0
    void ShowCursor(int target);
};

void CursorSoul::ShowCursor(int target)
{
    if (target) {
        if (visible)
            g_scene2->Release(g_scene2->Find(g_cursor));
        Vector3f from = region->GetPlace(current)->pos;
        Vector3f to = region->GetPlace(target)->pos;
        g_cursor = Make(from, to, Vector3f(1.0f, 1.0f, 1.0f));
        g_scene2->Attach(g_scene2->Find(g_cursor));
        g_cursor->Release();
    } else if (visible)
        g_scene2->Release(g_scene2->Find(g_cursor));
}
