// sqzhook.dll: runtime validation of reconstructed functions inside the original SQZ.EXE (not modified on disk).
// Loaded by the ddraw.dll proxy. Each hooked function is detoured; depending on SQZ_HOOK_MODE:
//   compare (default): run the original, run our version on a snapshot, log differences
//   replace:           run only our version
//   off:               do nothing
// Log: sqzhook_log.txt in the working directory.
#define private public            // hooks poke at Renderer's private helpers
#include "Renderer.h"
#include "ListenerTable.h"
#include "Scene.h"
#undef private
#include <stdio.h>
#include <stdarg.h>
#include <new>

static HANDLE g_log = INVALID_HANDLE_VALUE;
enum Mode { MODE_OFF, MODE_COMPARE, MODE_REPLACE };
static Mode g_mode = MODE_COMPARE;

static void Log(const char *fmt, ...)
{
    char buf[512]; DWORD w;
    va_list ap; va_start(ap, fmt);
    int n = _vsnprintf(buf, sizeof buf - 2, fmt, ap);
    va_end(ap);
    if (n < 0 || n > (int)sizeof buf - 2) n = sizeof buf - 2;
    buf[n++] = '\n';
    if (g_log != INVALID_HANDLE_VALUE) WriteFile(g_log, buf, n, &w, NULL);
}

// ---- functions the reconstructed code needs from the original binary ----------------------
const char *HResultToString(HRESULT hr) { return ((const char *(__cdecl *)(int))0x478430)((int)hr); }
int  PrepareForDialog()                         { return 0; }
// Warp backdrop (not reconstructed) and CRT helpers used by Scene
void Warp_Destroy(void *w)            { ((void (__attribute__((thiscall)) *)(void *))0x478cb0)(w); }
void Warp_Resize(void *w, int a, int b) { ((void (__attribute__((thiscall)) *)(void *, int, int))0x478dc0)(w, a, b); }
void Warp_Render(void *w, void *bits) { ((void (__attribute__((thiscall)) *)(void *, void *))0x478da0)(w, bits); }
void Operator_Delete(void *p)         { ((void (__cdecl *)(void *))0x48e9f0)(p); }
void Unknown_48C710()                 { ((void (__cdecl *)())0x48c710)(); }

// The reconstructed code calls these through renamed imports (see build.cmd): resolve from the real DLLs.
extern "C" HRESULT WINAPI Hook_DirectDrawCreate(GUID *g, LPDIRECTDRAW *dd, IUnknown *o)
{
    typedef HRESULT (WINAPI *F)(GUID *, LPDIRECTDRAW *, IUnknown *);
    static F f = (F)GetProcAddress(LoadLibraryA("ddraw.dll"), "DirectDrawCreate");
    return f(g, dd, o);
}
extern "C" HRESULT WINAPI Hook_DirectDrawCreateClipper(DWORD fl, LPDIRECTDRAWCLIPPER *c, IUnknown *o)
{
    typedef HRESULT (WINAPI *F)(DWORD, LPDIRECTDRAWCLIPPER *, IUnknown *);
    static F f = (F)GetProcAddress(LoadLibraryA("ddraw.dll"), "DirectDrawCreateClipper");
    return f(fl, c, o);
}
extern "C" D3DCOLOR WINAPI Hook_D3DRMCreateColorRGB(D3DVALUE r, D3DVALUE g, D3DVALUE b)
{
    typedef D3DCOLOR (WINAPI *F)(D3DVALUE, D3DVALUE, D3DVALUE);
    static F f = (F)GetProcAddress(LoadLibraryA("d3drm.dll"), "D3DRMCreateColorRGB");
    return f(r, g, b);
}
extern "C" HRESULT WINAPI Hook_Direct3DRMCreate(LPDIRECT3DRM *p)
{
    typedef HRESULT (WINAPI *F)(LPDIRECT3DRM *);
    static F f = (F)GetProcAddress(LoadLibraryA("d3drm.dll"), "Direct3DRMCreate");
    return f(p);
}

// ---- detour engine -----------------------------------------------------------------------
struct Hook {
    const char *name;
    DWORD addr;
    int stolen;               // bytes of whole instructions (>=5) copied to the trampoline
    const char *expect;       // expected original bytes (hex), checked before patching
    void *hook;
    void **orig;              // receives the trampoline
    bool replaceOnly;         // only installed in replace mode
    unsigned calls, mismatches;
    const char *id;           // short name for SQZ_REPLACE (heavy hooks: installed only when listed)
    int relocCallPlus1;       // 1 + offset of an E8 call inside the stolen bytes whose rel32 must be fixed up; 0 = none
};

static BYTE *g_tramp; static size_t g_trampUsed;

static bool HexMatches(const BYTE *p, const char *hex)
{
    for (; hex[0] && hex[1]; hex += 2, p++) {
        unsigned v; sscanf(hex, "%2x", &v);
        if (*p != v) return false;
    }
    return true;
}

static bool Install(Hook &h)
{
    BYTE *t = (BYTE *)h.addr;
    if (!HexMatches(t, h.expect)) { Log("SKIP %s @%08x: original bytes differ (different SQZ.EXE?)", h.name, h.addr); return false; }
    BYTE *tr = g_tramp + g_trampUsed; g_trampUsed += 32;
    memcpy(tr, t, h.stolen);
    if (h.relocCallPlus1) *(int *)(tr + (h.relocCallPlus1 - 1) + 1) += (int)(t - tr);
    tr[h.stolen] = 0xE9;
    *(int *)(tr + h.stolen + 1) = (int)((t + h.stolen) - (tr + h.stolen + 5));
    DWORD old;
    VirtualProtect(t, h.stolen, PAGE_EXECUTE_READWRITE, &old);
    t[0] = 0xE9;
    *(int *)(t + 1) = (int)((BYTE *)h.hook - (t + 5));
    memset(t + 5, 0x90, h.stolen - 5);
    VirtualProtect(t, h.stolen, old, &old);
    FlushInstructionCache(GetCurrentProcess(), t, h.stolen);
    *h.orig = tr;
    Log("hooked %s @%08x (stolen %d)", h.name, h.addr, h.stolen);
    return true;
}

// ---- comparison helpers ------------------------------------------------------------------

static void CompareMem(Hook &h, const void *real, const void *ours, size_t n)
{
    h.calls++;
    const BYTE *a = (const BYTE *)real, *b = (const BYTE *)ours;
    if (!memcmp(a, b, n)) return;
    h.mismatches++;
    if (h.mismatches > 5) return;
    char line[400]; int len = sprintf(line, "MISMATCH %s:", h.name);
    int runs = 0;
    for (size_t i = 0; i < n && runs < 12; ) {
        if (a[i] == b[i]) { i++; continue; }
        size_t s = i; while (i < n && a[i] != b[i]) i++;
        len += sprintf(line + len, " +0x%x[%u] orig=", (unsigned)s, (unsigned)(i - s));
        for (size_t k = s; k < i && k < s + 4; k++) len += sprintf(line + len, "%02x", a[k]);
        len += sprintf(line + len, " ours=");
        for (size_t k = s; k < i && k < s + 4; k++) len += sprintf(line + len, "%02x", b[k]);
        runs++;
    }
    Log("%s", line);
}

// ---- hooked functions --------------------------------------------------------------------
static Hook H_ctor, H_updateRect, H_getClient, H_getScreen, H_pix1, H_pix2, H_desktop, H_check;

// @474400 Renderer ctor, __fastcall(this) returns this
static Renderer *(__fastcall *o_ctor)(Renderer *);
static Renderer *__fastcall h_ctor(Renderer *r)
{
    if (g_mode == MODE_REPLACE) { new (r) Renderer(); return r; }
    alignas(8) BYTE snap[sizeof(Renderer)];
    memcpy(snap, r, sizeof snap);
    Renderer *res = o_ctor(r);
    new ((Renderer *)snap) Renderer();
    CompareMem(H_ctor, r, snap, sizeof snap);
    return res;
}

// @475870 UpdateScreenRect, __fastcall(this)
static int (__fastcall *o_updateRect)(Renderer *);
static int __fastcall h_updateRect(Renderer *r)
{
    if (g_mode == MODE_REPLACE) { r->UpdateScreenRect(); return 0; }
    alignas(8) BYTE snap[sizeof(Renderer)];
    memcpy(snap, r, sizeof snap);
    int ret = o_updateRect(r);
    ((Renderer *)snap)->UpdateScreenRect();
    CompareMem(H_updateRect, r, snap, sizeof snap);
    return ret;
}

// @474f70 GetClientRectInWindow, __thiscall(this, LONG out[4])
typedef int (__attribute__((thiscall)) *PFN_ThisOut)(Renderer *, LONG *);
static PFN_ThisOut o_getClient;
static int __attribute__((thiscall)) h_getClient(Renderer *r, LONG *out)
{
    if (g_mode == MODE_REPLACE) { r->GetClientRectInWindow(out); return 0; }
    LONG mine[4] = { 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e };
    int ret = o_getClient(r, out);
    r->GetClientRectInWindow(mine);
    CompareMem(H_getClient, out, mine, sizeof mine);
    return ret;
}

// @474fe0 GetScreenRect
static PFN_ThisOut o_getScreen;
static int __attribute__((thiscall)) h_getScreen(Renderer *r, LONG *out)
{
    if (g_mode == MODE_REPLACE) { r->GetScreenRect(out); return 0; }
    LONG mine[4] = { 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e };
    int ret = o_getScreen(r, out);
    r->GetScreenRect(mine);
    CompareMem(H_getScreen, out, mine, sizeof mine);
    return ret;
}

// @475000 and @475020 GetPixelFormat (two identical copies), __thiscall(this, int out[6])
typedef int *(__attribute__((thiscall)) *PFN_ThisOutI)(Renderer *, int *);
static PFN_ThisOutI o_pix1, o_pix2;
static int *__attribute__((thiscall)) h_pix1(Renderer *r, int *out)
{
    if (g_mode == MODE_REPLACE) return r->GetPixelFormat(out);
    int mine[6] = { 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e };
    int *ret = o_pix1(r, out);
    r->GetPixelFormat(mine);
    CompareMem(H_pix1, out, mine, sizeof mine);
    return ret;
}
static int *__attribute__((thiscall)) h_pix2(Renderer *r, int *out)
{
    if (g_mode == MODE_REPLACE) return r->GetPixelFormat(out);
    int mine[6] = { 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e };
    int *ret = o_pix2(r, out);
    r->GetPixelFormat(mine);
    CompareMem(H_pix2, out, mine, sizeof mine);
    return ret;
}

// @474360 QueryDesktopMode, __cdecl(int*,int*,int*)
static int (__cdecl *o_desktop)(int *, int *, int *);
static int __cdecl h_desktop(int *w, int *h, int *bpp)
{
    if (g_mode == MODE_REPLACE) { QueryDesktopMode(w, h, bpp); return 1; }
    int m[3] = { -1, -1, -1 };
    int ret = o_desktop(w, h, bpp);
    QueryDesktopMode(&m[0], &m[1], &m[2]);
    int real[3] = { *w, *h, *bpp };
    CompareMem(H_desktop, real, m, sizeof m);
    return ret;
}

// @4743b0 CheckDesktopDepth, __cdecl(int*,int*)  (replace mode only: shows a message box on failure)
static int (__cdecl *o_check)(int *, int *);
static int __cdecl h_check(int *w, int *h)
{
    H_check.calls++;
    return CheckDesktopDepth(w, h);
}

// ---- ListenerTable (window object listener registry): pure memory, compared on a snapshot of the 0x80-byte table ----
static Hook H_lfree, H_lcontains, H_ladd, H_lremove;

// @478130 FindFreeSlot, __fastcall(table) -> int
static int (__fastcall *o_lfree)(ListenerTable *);
static int __fastcall h_lfree(ListenerTable *t)
{
    if (g_mode == MODE_REPLACE) return t->FindFreeSlot();
    int ret = o_lfree(t);
    int mine = t->FindFreeSlot();
    H_lfree.calls++;
    if (ret != mine) { H_lfree.mismatches++; Log("MISMATCH FindFreeSlot orig=%d ours=%d", ret, mine); }
    return ret;
}

// @477f20 Contains, __thiscall(table, cb, ctx) -> BOOL
static int (__attribute__((thiscall)) *o_lcontains)(ListenerTable *, void *, void *);
static int __attribute__((thiscall)) h_lcontains(ListenerTable *t, void *cb, void *ctx)
{
    if (g_mode == MODE_REPLACE) return t->Contains(cb, ctx);
    int ret = o_lcontains(t, cb, ctx);
    int mine = t->Contains(cb, ctx);
    H_lcontains.calls++;
    if (ret != mine) { H_lcontains.mismatches++; Log("MISMATCH Contains orig=%d ours=%d", ret, mine); }
    return ret;
}

// @477ef0 Add, __thiscall(table, cb, ctx) -> BOOL
static int (__attribute__((thiscall)) *o_ladd)(ListenerTable *, void *, void *);
static int __attribute__((thiscall)) h_ladd(ListenerTable *t, void *cb, void *ctx)
{
    if (g_mode == MODE_REPLACE) return t->Add(cb, ctx);
    ListenerTable snap = *t;
    int ret = o_ladd(t, cb, ctx);
    int mine = snap.Add(cb, ctx);
    CompareMem(H_ladd, t, &snap, sizeof snap);
    if (ret != mine) { H_ladd.mismatches++; Log("MISMATCH Add return orig=%d ours=%d", ret, mine); }
    return ret;
}

// @477f60 Remove, __thiscall(table, cb, ctx), return value unused by callers
static int (__attribute__((thiscall)) *o_lremove)(ListenerTable *, void *, void *);
static int __attribute__((thiscall)) h_lremove(ListenerTable *t, void *cb, void *ctx)
{
    if (g_mode == MODE_REPLACE) { t->Remove(cb, ctx); return 0; }
    ListenerTable snap = *t;
    int ret = o_lremove(t, cb, ctx);
    snap.Remove(cb, ctx);
    CompareMem(H_lremove, t, &snap, sizeof snap);
    return ret;
}

// ---- heavy functions: COM side effects, cannot be compared; installed only via SQZ_REPLACE=id1,id2 ----
static Hook H_quality, H_planes, H_frames, H_viewport, H_enum, H_surfaces, H_shutdown;

#define HEAVY_LOG(h) do { (h).calls++; } while (0)

// @4756a0 ApplyQualitySettings, __fastcall(this), returns last HRESULT in EAX
static int (__fastcall *o_quality)(Renderer *);
static int __fastcall h_quality(Renderer *r) { HEAVY_LOG(H_quality); return (int)r->ApplyQualitySettings(); }

// @4757d0 UpdateViewportPlanes, __fastcall(this)
static int (__fastcall *o_planes)(Renderer *);
static int __fastcall h_planes(Renderer *r) { HEAVY_LOG(H_planes); r->UpdateViewportPlanes(); return 0; }

// @475b40 CreateFrames, __fastcall(this) -> BOOL
static int (__fastcall *o_frames)(Renderer *);
static int __fastcall h_frames(Renderer *r) { HEAVY_LOG(H_frames); return r->CreateFrames(); }

// @4761f0 CreateViewport, __fastcall(this) -> BOOL
static int (__fastcall *o_viewport)(Renderer *);
static int __fastcall h_viewport(Renderer *r) { HEAVY_LOG(H_viewport); return r->CreateViewport(); }

// @475ad0 EnumerateDevices (named EnumDeviceCallback in Ghidra), __fastcall(this) -> BOOL
static int (__fastcall *o_enum)(Renderer *);
static bool g_enumCompare;   // SQZ_REPLACE contains "enumcmp": run original, then ours on a snapshot, compare
static int __fastcall h_enum(Renderer *r)
{
    HEAVY_LOG(H_enum);
    if (g_enumCompare) {
        alignas(8) BYTE snap[sizeof(Renderer)];
        memcpy(snap, r, sizeof snap);
        int ret = o_enum(r);
        int mine = ((Renderer *)snap)->EnumerateDevices();      // EnumDevices has no side effects beyond the callback
        CompareMem(H_enum, r, snap, sizeof snap);
        if (ret != mine) { H_enum.mismatches++; Log("MISMATCH Renderer::EnumerateDevices return: orig=%d ours=%d", ret, mine); }
        return ret;
    }
    return r->EnumerateDevices();
}

// @475c10 CreateSurfaces, __fastcall(this) -> BOOL
static int (__fastcall *o_surfaces)(Renderer *);
static int __fastcall h_surfaces(Renderer *r) { HEAVY_LOG(H_surfaces); return r->CreateSurfaces(); }

// @474930 Shutdown, __thiscall(this, int force) -> BOOL
static int (__attribute__((thiscall)) *o_shutdown)(Renderer *, int);
static int __attribute__((thiscall)) h_shutdown(Renderer *r, int force) { HEAVY_LOG(H_shutdown); return r->Shutdown(force); }

// @475560 QueryDisplayMode, __fastcall(this); GetDisplayMode has no side effects, so "qdmcmp" can compare on a snapshot
typedef int (__attribute__((thiscall)) *PFN_Init)(Renderer *, void *, HWND, int, int, int, int, int, int, int, int, int, const unsigned char *);
static Hook H_qdm, H_enterdisplay, H_uploadpal, H_setpal, H_init, H_fade;
static Hook H_swarp, H_sinit, H_sbegin, H_srender, H_spresent, H_squality, H_sxform, H_schild, H_sdel;
static Hook H_cframe, H_cframeu, H_cdecal, H_dframe, H_gmatrix;
static bool g_qdmCompare;
static int (__fastcall *o_qdm)(Renderer *);
static int __fastcall h_qdm(Renderer *r)
{
    HEAVY_LOG(H_qdm);
    if (g_qdmCompare) {
        alignas(8) BYTE snap[sizeof(Renderer)];
        memcpy(snap, r, sizeof snap);
        int ret = o_qdm(r);
        ((Renderer *)snap)->QueryDisplayMode();
        CompareMem(H_qdm, r, snap, sizeof snap);
        return ret;
    }
    r->QueryDisplayMode();
    return 0;
}

// @476250 EnterDisplayMode, __thiscall(this, w, h): switches display mode / window layout (side effects, replace only)
static int (__attribute__((thiscall)) *o_enterdisplay)(Renderer *, int, int);
static int __attribute__((thiscall)) h_enterdisplay(Renderer *r, int w, int h) { HEAVY_LOG(H_enterdisplay); return r->EnterDisplayMode(w, h); }

// @4750a0 FadePalette, __thiscall(this, float seconds): blocking fade to black (replace only)
static void (__attribute__((thiscall)) *o_fade)(Renderer *, float);
static void __attribute__((thiscall)) h_fade(Renderer *r, float seconds) { HEAVY_LOG(H_fade); r->FadePalette(seconds); }

// ---- Scene (draw path): all COM side effects, replace only ---------------------------------
// @4767a0 EnsureWarpSurface, __fastcall(this) -> BOOL
static int (__fastcall *o_swarp)(Scene *);
static int __fastcall h_swarp(Scene *s) { HEAVY_LOG(H_swarp); return s->EnsureWarpSurface(); }
// @4768f0 Init, __thiscall(this, renderer) -> BOOL
static int (__attribute__((thiscall)) *o_sinit)(Scene *, Renderer *);
static int __attribute__((thiscall)) h_sinit(Scene *s, Renderer *r) { HEAVY_LOG(H_sinit); return s->Init(r); }
// @476af0 BeginFrame, __fastcall(this) -> BOOL
static int (__fastcall *o_sbegin)(Scene *);
static int __fastcall h_sbegin(Scene *s) { HEAVY_LOG(H_sbegin); return s->BeginFrame(); }
// @476cd0 Render, __fastcall(this) -> BOOL
static int (__fastcall *o_srender)(Scene *);
static int __fastcall h_srender(Scene *s) { HEAVY_LOG(H_srender); return s->Render(); }
// @476d20 Present, __fastcall(this) -> BOOL
static int (__fastcall *o_spresent)(Scene *);
static int __fastcall h_spresent(Scene *s) { HEAVY_LOG(H_spresent); return s->Present(); }
// @476ea0 SetQuality, __thiscall(this, level)
static void (__attribute__((thiscall)) *o_squality)(Scene *, int);
static void __attribute__((thiscall)) h_squality(Scene *s, int level) { HEAVY_LOG(H_squality); s->SetQuality(level); }
// @477040 SetFrameTransform, __thiscall(this, frame, matrix)
static void (__attribute__((thiscall)) *o_sxform)(Scene *, IDirect3DRMFrame *, const float *);
static void __attribute__((thiscall)) h_sxform(Scene *s, IDirect3DRMFrame *f, const float *m) { HEAVY_LOG(H_sxform); s->SetFrameTransform(f, m); }
// @4772d0 SetSceneChild, __thiscall(this, frame) -> BOOL
static int (__attribute__((thiscall)) *o_schild)(Scene *, IDirect3DRMFrame *);
static int __attribute__((thiscall)) h_schild(Scene *s, IDirect3DRMFrame *f) { HEAVY_LOG(H_schild); return s->SetSceneChild(f); }
// @477340 DeleteSceneChild, __fastcall(this) -> BOOL
static int (__fastcall *o_sdel)(Scene *);
static int __fastcall h_sdel(Scene *s) { HEAVY_LOG(H_sdel); return s->DeleteSceneChild(); }

// @476f40 CreateSceneFrame, __fastcall(this) -> frame
static IDirect3DRMFrame *(__fastcall *o_cframe)(Scene *);
static IDirect3DRMFrame *__fastcall h_cframe(Scene *s) { HEAVY_LOG(H_cframe); return s->CreateSceneFrame(); }
// @476f90 CreateFrameUnder, __thiscall(this, parent) -> frame
static IDirect3DRMFrame *(__attribute__((thiscall)) *o_cframeu)(Scene *, IDirect3DRMFrame *);
static IDirect3DRMFrame *__attribute__((thiscall)) h_cframeu(Scene *s, IDirect3DRMFrame *p) { HEAVY_LOG(H_cframeu); return s->CreateFrameUnder(p); }
// @477400 CreateDecalFrame, __thiscall(this, 16 stack dwords) -> frame
static IDirect3DRMFrame *(__attribute__((thiscall)) *o_cdecal)(Scene *, IDirect3DRMTexture *, float, float, float, int, float, float, int, float, float, int, BOOL, float, float, float, IDirect3DRMFrame *);
static IDirect3DRMFrame *__attribute__((thiscall)) h_cdecal(Scene *s, IDirect3DRMTexture *t, float px, float py, float pz, int a18, float sw, float sh, int a24,
                                                            float sx, float sy, int a30, BOOL tr, float r, float g, float b, IDirect3DRMFrame *p)
{ HEAVY_LOG(H_cdecal); return s->CreateDecalFrame(t, px, py, pz, a18, sw, sh, a24, sx, sy, a30, tr, r, g, b, p); }
// @476ff0 DestroyFrame, __stdcall(frame) -> BOOL
static BOOL (__stdcall *o_dframe)(IDirect3DRMFrame *);
static BOOL __stdcall h_dframe(IDirect3DRMFrame *f) { HEAVY_LOG(H_dframe); return DestroyFrame(f); }
// @477120 GetFrameMatrix, __stdcall(frame, ref, m)
static void (__stdcall *o_gmatrix)(IDirect3DRMFrame *, IDirect3DRMFrame *, float *);
static void __stdcall h_gmatrix(IDirect3DRMFrame *f, IDirect3DRMFrame *r, float *m) { HEAVY_LOG(H_gmatrix); GetFrameMatrix(f, r, m); }

// @474e10 UploadFrontPalette, __fastcall(this) -> BOOL
static int (__fastcall *o_uploadpal)(Renderer *);
static int __fastcall h_uploadpal(Renderer *r) { HEAVY_LOG(H_uploadpal); return r->UploadFrontPalette(); }

// @474c00 SetPaletteEntries, __thiscall(this, first, count, rgba*) -> BOOL
static int (__attribute__((thiscall)) *o_setpal)(Renderer *, int, int, const unsigned char *);
static int __attribute__((thiscall)) h_setpal(Renderer *r, int first, int count, const unsigned char *rgba)
{
    HEAVY_LOG(H_setpal);
    r->CopyPaletteEntries(first, count, rgba);
    if (r->device != NULL && r->initializing == 0) {
        // the original re-runs Initialize here; call it by ADDRESS so it goes through whichever Initialize is active (hooked or not)
        int w = r->doubleSize ? r->width * 2 : r->width;
        int h = r->doubleSize ? r->height * 2 : r->height;
        return ((PFN_Init)0x474540)(r, r->windowObj, r->hwnd, r->originX, r->originY, w, h, r->requestedWidth, r->requestedHeight, r->fmt.bpp, r->fullscreen, r->doubleSize, NULL);
    }
    return 1;
}

// @474540 Initialize, __thiscall(this + 12 stack args) -> BOOL
static PFN_Init o_init;
static int __attribute__((thiscall)) h_init(Renderer *r, void *wnd, HWND hwnd, int x, int y, int w, int h, int rw, int rh, int bpp, int full, int dbl, const unsigned char *pal)
{ HEAVY_LOG(H_init); return r->Initialize(wnd, hwnd, x, y, w, h, rw, rh, bpp, full, dbl, pal); }

// ---- table and entry point ---------------------------------------------------------------
static Hook *g_hooks[] = { &H_ctor, &H_updateRect, &H_getClient, &H_getScreen, &H_pix1, &H_pix2, &H_desktop, &H_check,
                           &H_quality, &H_planes, &H_frames, &H_viewport, &H_enum, &H_surfaces, &H_shutdown,
                           &H_lfree, &H_lcontains, &H_ladd, &H_lremove,
                           &H_qdm, &H_enterdisplay, &H_uploadpal, &H_setpal, &H_init, &H_fade,
                           &H_swarp, &H_sinit, &H_sbegin, &H_srender, &H_spresent, &H_squality, &H_sxform, &H_schild, &H_sdel,
                           &H_cframe, &H_cframeu, &H_cdecal, &H_dframe, &H_gmatrix };
static char g_replaceList[2048];
static bool Listed(const char *id) { return id && (strstr(g_replaceList, id) || strstr(g_replaceList, "all")); }

static void Setup()
{
    H_ctor       = { "Renderer::Renderer",              0x474400, 5,  "5356578bf1",           (void *)h_ctor,       (void **)&o_ctor, false, 0, 0 };
    H_updateRect = { "Renderer::UpdateScreenRect",      0x475870, 5,  "83ec085356",          (void *)h_updateRect, (void **)&o_updateRect, false, 0, 0 };
    H_getClient  = { "Renderer::GetClientRectInWindow", 0x474f70, 5,  "83ec105356",          (void *)h_getClient,  (void **)&o_getClient, false, 0, 0 };
    H_getScreen  = { "Renderer::GetScreenRect",         0x474fe0, 8,  "538b41588b542408",     (void *)h_getScreen,  (void **)&o_getScreen, false, 0, 0 };
    H_pix1       = { "Renderer::GetPixelFormat(1)",     0x475000, 5,  "8b44240456",           (void *)h_pix1,       (void **)&o_pix1, false, 0, 0 };
    H_pix2       = { "Renderer::GetPixelFormat(2)",     0x475020, 5,  "8b44240456",           (void *)h_pix2,       (void **)&o_pix2, false, 0, 0 };
    H_desktop    = { "QueryDesktopMode",                0x474360, 10, "56576a00ff1544f64e00", (void *)h_desktop,   (void **)&o_desktop, false, 0, 0 };
    H_check      = { "CheckDesktopDepth",               0x4743b0, 8,  "8b4c24088b542404",     (void *)h_check,      (void **)&o_check, true, 0, 0 };
    H_quality    = { "Renderer::ApplyQualitySettings",  0x4756a0, 5,  "53568b412c",           (void *)h_quality,    (void **)&o_quality,  false, 0, 0, "quality" };
    H_planes     = { "Renderer::UpdateViewportPlanes",  0x4757d0, 5,  "83ec045356",           (void *)h_planes,     (void **)&o_planes,   false, 0, 0, "planes" };
    H_frames     = { "Renderer::CreateFrames",          0x475b40, 5,  "5356578bf1",           (void *)h_frames,     (void **)&o_frames,   false, 0, 0, "frames" };
    H_viewport   = { "Renderer::CreateViewport",        0x4761f0, 8,  "53568d81b8090000",     (void *)h_viewport,   (void **)&o_viewport, false, 0, 0, "viewport" };
    H_enum       = { "Renderer::EnumerateDevices",      0x475ad0, 5,  "53568b4128",           (void *)h_enum,       (void **)&o_enum,     false, 0, 0, "enum" };
    H_surfaces   = { "Renderer::CreateSurfaces",        0x475c10, 6,  "81ece4060000",         (void *)h_surfaces,   (void **)&o_surfaces, false, 0, 0, "surfaces" };
    H_shutdown   = { "Renderer::Shutdown",              0x474930, 8,  "53568b81b8090000",    (void *)h_shutdown,   (void **)&o_shutdown, false, 0, 0, "shutdown" };
    H_lfree      = { "ListenerTable::FindFreeSlot",    0x478130, 23, "33c08b1185d2740e83c1084083f8107cf1b8ffffffffc3",         (void *)h_lfree,      (void **)&o_lfree,    false, 0, 0, NULL, 0 };
    H_lcontains  = { "ListenerTable::Contains",        0x477f20, 5,  "8b44240453",           (void *)h_lcontains,  (void **)&o_lcontains, false, 0, 0, NULL, 0 };
    H_ladd       = { "ListenerTable::Add",             0x477ef0, 8,  "568bf1e838020000",     (void *)h_ladd,       (void **)&o_ladd,     false, 0, 0, NULL, 4 };
    H_lremove    = { "ListenerTable::Remove",          0x477f60, 6,  "5333c0568bd1",         (void *)h_lremove,    (void **)&o_lremove,  false, 0, 0, NULL, 0 };
    H_qdm          = { "Renderer::QueryDisplayMode",    0x475560, 5,  "83ec6c33c0",           (void *)h_qdm,          (void **)&o_qdm,          false, 0, 0, "qdm", 0 };
    H_enterdisplay = { "Renderer::EnterDisplayMode",    0x476250, 5,  "83ec105356",           (void *)h_enterdisplay, (void **)&o_enterdisplay, false, 0, 0, "enterdisplay", 0 };
    H_uploadpal    = { "Renderer::UploadFrontPalette",  0x474e10, 6,  "81ec00040000",         (void *)h_uploadpal,    (void **)&o_uploadpal,    false, 0, 0, "uploadpal", 0 };
    H_setpal       = { "Renderer::SetPaletteEntries",   0x474c00, 5,  "8b44240853",           (void *)h_setpal,       (void **)&o_setpal,       false, 0, 0, "setpal", 0 };
    H_init         = { "Renderer::Initialize",          0x474540, 5,  "53568b410c",           (void *)h_init,         (void **)&o_init,         false, 0, 0, "init", 0 };
    H_fade         = { "Renderer::FadePalette",         0x4750a0, 6,  "81ec1c040000",         (void *)h_fade,         (void **)&o_fade,         false, 0, 0, "fade", 0 };
    H_swarp   = { "Scene::EnsureWarpSurface", 0x4767a0, 5, "83ec6c5356",     (void *)h_swarp,   (void **)&o_swarp,   false, 0, 0, "swarp",   0 };
    H_sinit   = { "Scene::Init",              0x4768f0, 5, "8b44240453",     (void *)h_sinit,   (void **)&o_sinit,   false, 0, 0, "sinit",   0 };
    H_sbegin  = { "Scene::BeginFrame",        0x476af0, 6, "81ecf0000000",   (void *)h_sbegin,  (void **)&o_sbegin,  false, 0, 0, "sbegin",  0 };
    H_srender = { "Scene::Render",            0x476cd0, 6, "56578b018bf1",   (void *)h_srender, (void **)&o_srender, false, 0, 0, "srender", 0 };
    H_spresent= { "Scene::Present",           0x476d20, 5, "83ec205356",     (void *)h_spresent,(void **)&o_spresent,false, 0, 0, "spresent",0 };
    H_squality= { "Scene::SetQuality",        0x476ea0, 5, "538b0185c0",     (void *)h_squality,(void **)&o_squality,false, 0, 0, "squality",0 };
    H_sxform  = { "Scene::SetFrameTransform", 0x477040, 6, "53568b74240c",   (void *)h_sxform,  (void **)&o_sxform,  false, 0, 0, "sxform",  0 };
    H_schild  = { "Scene::SetSceneChild",     0x4772d0, 5, "5356578bf1",     (void *)h_schild,  (void **)&o_schild,  false, 0, 0, "schild",  0 };
    H_sdel    = { "Scene::DeleteSceneChild",  0x477340, 7, "53568bf18b490c", (void *)h_sdel,    (void **)&o_sdel,    false, 0, 0, "sdel",    0 };
    H_cframe  = { "Scene::CreateSceneFrame",  0x476f40, 7, "83ec048d542400",   (void *)h_cframe,  (void **)&o_cframe,  false, 0, 0, "cframe",  0 };
    H_cframeu = { "Scene::CreateFrameUnder",  0x476f90, 7, "83ec048d542400",   (void *)h_cframeu, (void **)&o_cframeu, false, 0, 0, "cframeu", 0 };
    H_cdecal  = { "Scene::CreateDecalFrame",  0x477400, 7, "64a10000000055",   (void *)h_cdecal,  (void **)&o_cdecal,  false, 0, 0, "cdecal",  0 };
    H_dframe  = { "DestroyFrame",             0x476ff0, 7, "83ec048d442400",   (void *)h_dframe,  (void **)&o_dframe,  false, 0, 0, "dframe",  0 };
    H_gmatrix = { "GetFrameMatrix",           0x477120, 7, "64a10000000055",   (void *)h_gmatrix, (void **)&o_gmatrix, false, 0, 0, "gmatrix", 0 };
}

static void Summary()
{
    for (Hook *h : g_hooks)
        Log("summary %-34s calls=%u mismatches=%u", h->name, h->calls, h->mismatches);
}

// ---- crash reporter: logs registers and stack for the first access violation / illegal instruction ----
static LONG CALLBACK CrashHandler(EXCEPTION_POINTERS *ep)
{
    DWORD code = ep->ExceptionRecord->ExceptionCode;
    if (code != EXCEPTION_ACCESS_VIOLATION && code != EXCEPTION_ILLEGAL_INSTRUCTION && code != EXCEPTION_PRIV_INSTRUCTION)
        return EXCEPTION_CONTINUE_SEARCH;
    static LONG once = 0;
    if (InterlockedExchange(&once, 1)) return EXCEPTION_CONTINUE_SEARCH;
    CONTEXT *c = ep->ContextRecord;
    Log("CRASH code=%08lx at eip=%08lx (access addr %08lx, %s)", code, c->Eip,
        (unsigned long)ep->ExceptionRecord->ExceptionInformation[1],
        ep->ExceptionRecord->ExceptionInformation[0] == 0 ? "read" : ep->ExceptionRecord->ExceptionInformation[0] == 1 ? "write" : "exec");
    Log("  eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx ebp=%08lx esp=%08lx", c->Eax, c->Ebx, c->Ecx, c->Edx, c->Esi, c->Edi, c->Ebp, c->Esp);
    DWORD *sp = (DWORD *)c->Esp;
    for (int row = 0; row < 6; row++) {
        char line[200]; int n = 0;
        for (int i = 0; i < 4; i++) {
            DWORD v = 0;
            if (!IsBadReadPtr(sp + row * 4 + i, 4)) v = sp[row * 4 + i];
            n += sprintf(line + n, " %08lx", v);
        }
        Log("  [esp+%02x]%s", row * 16, line);
    }
    // symbolise return-address-looking values: inside SQZ.EXE .text or our trampolines/DLL
    for (int i = 0; i < 24; i++) {
        if (IsBadReadPtr(sp + i, 4)) break;
        DWORD v = sp[i];
        if (v >= 0x401000 && v < 0x49a000) Log("  stack[%d] looks like SQZ.EXE code %08lx", i, v);
        else if (v >= (DWORD)g_tramp && v < (DWORD)g_tramp + 4096) Log("  stack[%d] points into trampoline pool +%lx", i, v - (DWORD)g_tramp);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

// Settings can also come from sqzhook.cfg next to the DLL ("mode=..." and "replace=..." lines), used when the
// environment variables did not reach the game process (seen with the compatibility layer applied to SQZ.EXE).
static void ReadConfig(HINSTANCE inst, char *mode, size_t modeSize, char *replace, size_t replaceSize)
{
    char path[MAX_PATH];
    if (!GetModuleFileNameA(inst, path, MAX_PATH)) return;
    char *s = strrchr(path, 0x5c);
    if (!s || (size_t)(s - path) + 16 >= MAX_PATH) return;
    strcpy(s + 1, "sqzhook.cfg");
    HANDLE f = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) return;
    char buf[4096]; DWORD n = 0;
    ReadFile(f, buf, sizeof buf - 1, &n, NULL);
    CloseHandle(f);
    buf[n] = 0;
    for (char *line = strtok(buf, "\r\n"); line; line = strtok(NULL, "\r\n")) {
        if (!strncmp(line, "mode=", 5)) { strncpy(mode, line + 5, modeSize - 1); mode[modeSize - 1] = 0; }
        else if (!strncmp(line, "replace=", 8)) { strncpy(replace, line + 8, replaceSize - 1); replace[replaceSize - 1] = 0; }
    }
}

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(inst);
        char m[32] = "";
        GetEnvironmentVariableA("SQZ_HOOK_MODE", m, sizeof m);
        char cfgMode[32] = "", cfgReplace[2048] = "";
        ReadConfig(inst, cfgMode, sizeof cfgMode, cfgReplace, sizeof cfgReplace);
        if (!m[0]) strcpy(m, cfgMode);
        g_mode = !strcmp(m, "off") ? MODE_OFF : !strcmp(m, "replace") ? MODE_REPLACE : MODE_COMPARE;
        // log next to the DLL (not the current directory); fall back to a per-process file if another instance holds it
        char logPath[MAX_PATH] = "sqzhook_log.txt";
        if (GetModuleFileNameA(inst, logPath, MAX_PATH)) { char *s = strrchr(logPath, '\\'); if (s) strcpy(s + 1, "sqzhook_log.txt"); }
        g_log = CreateFileA(logPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
        if (g_log == INVALID_HANDLE_VALUE) {
            char alt[MAX_PATH + 16]; sprintf(alt, "%s.%lu", logPath, GetCurrentProcessId());
            g_log = CreateFileA(alt, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
        }
        { char exe[MAX_PATH] = ""; GetModuleFileNameA(NULL, exe, MAX_PATH);
          Log("sqzhook mode=%s pid=%lu exe=%s", g_mode == MODE_OFF ? "off" : g_mode == MODE_REPLACE ? "replace" : "compare", GetCurrentProcessId(), exe); }
        if (g_mode == MODE_OFF) return TRUE;
        g_tramp = (BYTE *)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        AddVectoredExceptionHandler(1, CrashHandler);
        Setup();
        if (!GetEnvironmentVariableA("SQZ_REPLACE", g_replaceList, sizeof g_replaceList))
            strcpy(g_replaceList, cfgReplace);
        Log("SQZ_REPLACE=%s", g_replaceList);
        g_enumCompare = strstr(g_replaceList, "enumcmp") != NULL;
        g_qdmCompare = strstr(g_replaceList, "qdmcmp") != NULL;
        for (Hook *h : g_hooks) {
            if (h->id) { if (Listed(h->id)) Install(*h); }          // heavy: explicit opt-in
            else if (!h->replaceOnly || g_mode == MODE_REPLACE) Install(*h);
        }
    } else if (reason == DLL_PROCESS_DETACH) {
        Summary();
        if (g_log != INVALID_HANDLE_VALUE) CloseHandle(g_log);
    }
    return TRUE;
}
