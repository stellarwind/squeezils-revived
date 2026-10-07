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
#include "Warp.h"
#include "Original.h"
#include "Sky.h"
#include "Image.h"
#include "Bitmap.h"
#include "GameString.h"
#include "IniReader.h"
#include "SqgModel.h"
#include "Texture.h"
#include "BodyPart.h"
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

// ---- functions the reconstructed code needs from the original binary (declared in Original.h) ----
const char *HResultToString(HRESULT hr) { return ((const char *(__cdecl *)(int))0x478430)((int)hr); }
int  PrepareForDialog()                         { return 0; }
void Lib_Free(void *p)                                { ((void (__cdecl *)(void *))0x490540)(p); }
void Operator_Delete(void *p)         { ((void (__cdecl *)(void *))0x48e9f0)(p); }
void *Operator_New(size_t n)                          { return ((void *(__cdecl *)(size_t))0x48ea00)(n); }
// Texture creation helpers: the game's allocator wrappers and its renderer global
void *GameAlloc(size_t n)  { return ((void *(__cdecl *)(size_t))0x448350)(n); }
void  GameFree(void *p)    { ((void (__cdecl *)(void *))0x448360)(p); }
Renderer *GetRenderer()    { return *(Renderer **)0x4c75dc; }
Scene *GetScene()          { return *(Scene **)0x4c75e0; }
void  SqgFree(void *p)     { (*(void (__cdecl **)(void *))0x4c0f80)(p); }
// SqgModel helpers: the game's allocator (function pointer at 0x4c0f78), its error reporter and texture lookup (pointer at 0x4c0f84)
void *SqgAlloc(size_t n)       { return (*(void *(__cdecl **)(size_t))0x4c0f78)(n); }
void  SqgError(const char *format, ...)    // the original is printf style; format here and hand it over as one string
{
    char buf[512];
    va_list ap; va_start(ap, format);
    _vsnprintf(buf, sizeof buf - 1, format, ap);
    va_end(ap);
    buf[sizeof buf - 1] = 0;
    ((void (__cdecl *)(const char *, ...))0x473710)("%s", buf);
}
SqgTextureLookupFn SqgGetTextureLookup() { return *(SqgTextureLookupFn *)0x4c0f84; }
const char *IniFind(void *file, const char *section, const char *key) { return ((const char *(__attribute__((thiscall)) *)(void *, const char *, const char *))0x442ee0)(file, section, key); }
void *IniOpen(const char *path) { return ((void *(__cdecl *)(const char *))0x442fa0)(path); }
void NumericError()                   { ((void (__cdecl *)())0x41eb80)(); }
void *Lib_Malloc(size_t n)            { return ((void *(__cdecl *)(size_t))0x4904c0)(n); }

// The reconstructed code calls these through renamed imports (see build.cmd): resolve from the real DLLs.
// ("ddraw.dll" here is the proxy already loaded in the process, so its fixes apply to our calls too)
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
    if (!g_tramp || g_trampUsed + 32 > 4096 || h.stolen > 27) { Log("SKIP %s: no room for its trampoline", h.name); return false; }
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
    char line[1024]; int len = sprintf(line, "MISMATCH %s:", h.name);   // (Log itself cuts a line at 512 characters)
    int runs = 0;
    for (size_t i = 0; i < n && runs < 12 && len < (int)sizeof line - 100; ) {
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
static Hook H_sqgload, H_sqgmesh, H_sqgmorph, H_sqgframe, H_texgif, H_bpapply, H_bpbuild, H_abroot, H_abbuild, H_abdestroy, H_pzdestroy, H_sdtor, H_quad;
static Hook H_lightamb, H_lightdir, H_fxform, H_fdecal;
static Hook H_sload, H_scam, H_sview, H_fparent, H_fvisual, H_fchild, H_fdel;
static Hook H_wctor, H_wdtor, H_winit, H_woutput, H_wresize, H_wupdate, H_wload;
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
static unsigned g_warpFrames;   // frames drawn with the warp (sky) backdrop active
static int __fastcall h_sbegin(Scene *s) { HEAVY_LOG(H_sbegin); if (s->warp) g_warpFrames++; return s->BeginFrame(); }
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

// @477700 CreateAmbientLight, __thiscall(this, r, g, b) -> frame
static IDirect3DRMFrame *(__attribute__((thiscall)) *o_lightamb)(Scene *, float, float, float);
static IDirect3DRMFrame *__attribute__((thiscall)) h_lightamb(Scene *s, float r, float g, float b) { HEAVY_LOG(H_lightamb); return s->CreateAmbientLight(r, g, b); }
// @4775c0 CreateDirectionalLight, __thiscall(this, 12 floats) -> frame (callee pops 0x30)
static IDirect3DRMFrame *(__attribute__((thiscall)) *o_lightdir)(Scene *, float, float, float, float, float, float, float, float, float, float, float, float);
static IDirect3DRMFrame *__attribute__((thiscall)) h_lightdir(Scene *s, float r, float g, float b, float px, float py, float pz, float dx, float dy, float dz, float ux, float uy, float uz)
{ HEAVY_LOG(H_lightdir); return s->CreateDirectionalLight(r, g, b, px, py, pz, dx, dy, dz, ux, uy, uz); }
// @4770d0 ApplyFrameTransform, __stdcall(frame, ref, matrix)
static void (__stdcall *o_fxform)(IDirect3DRMFrame *, IDirect3DRMFrame *, const float *);
static void __stdcall h_fxform(IDirect3DRMFrame *f, IDirect3DRMFrame *r, const float *m) { HEAVY_LOG(H_fxform); ApplyFrameTransform(f, r, m); }
// @477540 SetFrameDecalSize, __stdcall(frame, width, height) -> frame
static IDirect3DRMFrame *(__stdcall *o_fdecal)(IDirect3DRMFrame *, float, float);
static IDirect3DRMFrame *__stdcall h_fdecal(IDirect3DRMFrame *f, float w, float h) { HEAVY_LOG(H_fdecal); return SetFrameDecalSize(f, w, h); }

// @4769a0 LoadWarp, __thiscall(this, name) -> BOOL
static int (__attribute__((thiscall)) *o_sload)(Scene *, const char *);
static int __attribute__((thiscall)) h_sload(Scene *s, const char *name) { HEAVY_LOG(H_sload); return s->LoadWarp(name); }
// @476e10 SetCameraTransform, __thiscall(this, matrix)
static void (__attribute__((thiscall)) *o_scam)(Scene *, const float *);
static void __attribute__((thiscall)) h_scam(Scene *s, const float *m) { HEAVY_LOG(H_scam); s->SetCameraTransform(m); }
// @476e40 SetViewParams, __thiscall(this, near, far, fov)
static void (__attribute__((thiscall)) *o_sview)(Scene *, float, float, float);
static void __attribute__((thiscall)) h_sview(Scene *s, float n, float f, float fov) { HEAVY_LOG(H_sview); s->SetViewParams(n, f, fov); }
// @4772a0 GetFrameParent, __stdcall(frame) -> parent
static IDirect3DRMFrame *(__stdcall *o_fparent)(IDirect3DRMFrame *);
static IDirect3DRMFrame *__stdcall h_fparent(IDirect3DRMFrame *f) { HEAVY_LOG(H_fparent); return GetFrameParent(f); }
// @4773a0 / @4773c0 / @4773e0 frame wrappers, __stdcall(frame, x) -> HRESULT
static HRESULT (__stdcall *o_fvisual)(IDirect3DRMFrame *, IDirect3DRMVisual *);
static HRESULT __stdcall h_fvisual(IDirect3DRMFrame *f, IDirect3DRMVisual *v) { HEAVY_LOG(H_fvisual); return AddVisualToFrame(f, v); }
static HRESULT (__stdcall *o_fchild)(IDirect3DRMFrame *, IDirect3DRMFrame *);
static HRESULT __stdcall h_fchild(IDirect3DRMFrame *f, IDirect3DRMFrame *c) { HEAVY_LOG(H_fchild); return AddChildToFrame(f, c); }
static HRESULT (__stdcall *o_fdel)(IDirect3DRMFrame *, IDirect3DRMFrame *);
static HRESULT __stdcall h_fdel(IDirect3DRMFrame *f, IDirect3DRMFrame *c) { HEAVY_LOG(H_fdel); return DeleteChildFromFrame(f, c); }

// ---- SqgModel (character / prop meshes and morph animation): replace only ------------------------
// @472560 Load, __thiscall(this, path, mirror) -> BOOL
static int (__attribute__((thiscall)) *o_sqgload)(SqgModel *, const char *, int);
static int __attribute__((thiscall)) h_sqgload(SqgModel *m, const char *path, int mirror) { HEAVY_LOG(H_sqgload); return m->Load(path, mirror); }
// @472f60 CreateMesh, __thiscall(this, d3drm) -> PZMorphMesh *
static PZMorphMesh *(__attribute__((thiscall)) *o_sqgmesh)(SqgModel *, IDirect3DRM *);
static PZMorphMesh *__attribute__((thiscall)) h_sqgmesh(SqgModel *m, IDirect3DRM *d) { HEAVY_LOG(H_sqgmesh); return m->CreateMesh(d); }
// @4734e0 Morph, __thiscall(this, instance, count, weights)
static void (__attribute__((thiscall)) *o_sqgmorph)(SqgModel *, PZMorphMesh *, int, const float *);
static void __attribute__((thiscall)) h_sqgmorph(SqgModel *m, PZMorphMesh *i, int n, const float *w) { HEAVY_LOG(H_sqgmorph); m->Morph(i, n, w); }
// @4731f0 BuildFrame, __thiscall(this, d3drm, perspective) -> frame (callee pops 8)
static IDirect3DRMFrame *(__attribute__((thiscall)) *o_sqgframe)(SqgModel *, IDirect3DRM *, BOOL);
static IDirect3DRMFrame *__attribute__((thiscall)) h_sqgframe(SqgModel *m, IDirect3DRM *d, BOOL p) { HEAVY_LOG(H_sqgframe); return m->BuildFrame(d, p); }

// ---- Texture creation: replace only ---------------------------------------------------------
// @415440 CreateTextureFromGif, __cdecl(path, transparent, pixelFormat, byteCounter) -> texture
static IDirect3DRMTexture *(__cdecl *o_texgif)(const char *, const float *, const int *, int *);
static IDirect3DRMTexture *__cdecl h_texgif(const char *p, const float *t, const int *f, int *c) { HEAVY_LOG(H_texgif); return CreateTextureFromGif(p, t, f, c); }

// ---- BodyPart (character pieces): replace only ------------------------------------------------
// @41d900 Apply, __thiscall(this, ref, pose) (callee pops 8)
static void (__attribute__((thiscall)) *o_bpapply)(BodyPart *, IDirect3DRMFrame *, const BodyPose *);
static void __attribute__((thiscall)) h_bpapply(BodyPart *b, IDirect3DRMFrame *ref, const BodyPose *p) { HEAVY_LOG(H_bpapply); b->Apply(ref, p); }
// @41d990 Build, __thiscall(this, parent)
static void (__attribute__((thiscall)) *o_bpbuild)(BodyPart *, IDirect3DRMFrame *);
static void __attribute__((thiscall)) h_bpbuild(BodyPart *b, IDirect3DRMFrame *parent) { HEAVY_LOG(H_bpbuild); b->Build(parent); }

// @41e230 AnimatedBody::SetRootTransform, __thiscall(this, ref, matrix) (callee pops 8)
static void (__attribute__((thiscall)) *o_abroot)(AnimatedBody *, IDirect3DRMFrame *, const float *);
static void __attribute__((thiscall)) h_abroot(AnimatedBody *a, IDirect3DRMFrame *ref, const float *m) { HEAVY_LOG(H_abroot); a->SetRootTransform(ref, m); }

// @41dda0 AnimatedBody::Build, __thiscall(this, parent)
static void (__attribute__((thiscall)) *o_abbuild)(AnimatedBody *, IDirect3DRMFrame *);
static void __attribute__((thiscall)) h_abbuild(AnimatedBody *a, IDirect3DRMFrame *p) { HEAVY_LOG(H_abbuild); a->Build(p); }
// @41ddf0 AnimatedBody::DestroyFrames, __thiscall(this, parent)
static void (__attribute__((thiscall)) *o_abdestroy)(AnimatedBody *, IDirect3DRMFrame *);
static void __attribute__((thiscall)) h_abdestroy(AnimatedBody *a, IDirect3DRMFrame *p) { HEAVY_LOG(H_abdestroy); a->DestroyFrames(p); }
// @471990 PZMorphMesh::Destroy, __fastcall(this)
static void (__fastcall *o_pzdestroy)(PZMorphMesh *);
static void __fastcall h_pzdestroy(PZMorphMesh *m) { HEAVY_LOG(H_pzdestroy); m->Destroy(); }
// @476740 Scene::~Scene, __fastcall(this)
static void (__fastcall *o_sdtor)(Scene *);
static void __fastcall h_sdtor(Scene *s) { HEAVY_LOG(H_sdtor); s->~Scene(); }

// @4777b0 CreateColoredQuad, __thiscall(this, 15 floats, 3 doubles) -> frame (callee pops 0x54)
static IDirect3DRMFrame *(__attribute__((thiscall)) *o_quad)(Scene *, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, double, double, double);
static IDirect3DRMFrame *__attribute__((thiscall)) h_quad(Scene *s, float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float nx, float ny, float nz, double r, double g, double b)
{ HEAVY_LOG(H_quad); return s->CreateColoredQuad(x0, y0, z0, x1, y1, z1, x2, y2, z2, x3, y3, z3, nx, ny, nz, r, g, b); }

// ---- Warp (sky): replace only ---------------------------------------------------------------
// @478c80 constructor, __fastcall(this) -> this
static Warp *(__fastcall *o_wctor)(Warp *);
static Warp *__fastcall h_wctor(Warp *w) { HEAVY_LOG(H_wctor); new (w) Warp(); return w; }
// @478cb0 destructor, __fastcall(this)
static void (__fastcall *o_wdtor)(Warp *);
static void __fastcall h_wdtor(Warp *w) { HEAVY_LOG(H_wdtor); w->~Warp(); }
// @478ce0 Init, __thiscall(this, height, width, fov, flag) -> BOOL
static int (__attribute__((thiscall)) *o_winit)(Warp *, int, int, float, int);
static int __attribute__((thiscall)) h_winit(Warp *w, int h, int wd, float fov, int flag) { HEAVY_LOG(H_winit); return w->Init(h, wd, fov, (char)flag); }
// @478da0 SetOutput, __thiscall(this, bits)
static void (__attribute__((thiscall)) *o_woutput)(Warp *, void *);
static void __attribute__((thiscall)) h_woutput(Warp *w, void *bits) { HEAVY_LOG(H_woutput); w->SetOutput(bits); }
// @478dc0 Resize, __thiscall(this, width, height)
static void (__attribute__((thiscall)) *o_wresize)(Warp *, int, int);
static void __attribute__((thiscall)) h_wresize(Warp *w, int wd, int h) { HEAVY_LOG(H_wresize); w->Resize(wd, h); }
// @478e10 Update, __thiscall(this, matrix)
static void (__attribute__((thiscall)) *o_wupdate)(Warp *, const float *);
static void __attribute__((thiscall)) h_wupdate(Warp *w, const float *m) { HEAVY_LOG(H_wupdate); w->Update(m); }
// @479090 Load, __thiscall(this, prefix, palette, pixelFormat) -> BOOL
static int (__attribute__((thiscall)) *o_wload)(Warp *, const char *, unsigned char *, const int *);
static int __attribute__((thiscall)) h_wload(Warp *w, const char *p, unsigned char *pal, const int *fmt) { HEAVY_LOG(H_wload); return w->Load(p, pal, fmt); }

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
{
    HEAVY_LOG(H_init);
    int ok = r->Initialize(wnd, hwnd, x, y, w, h, rw, rh, bpp, full, dbl, pal);
    Log("renderer: ok=%d device=%s hardware=%d monoColorModel=%d doubleSize=%d display=%dbpp size=%dx%d fullscreen=%d",
        ok, r->chosenDevice >= 0 ? r->deviceNames[r->chosenDevice] : "(none)", r->hardware, r->monoColorModel,
        r->doubleSize, r->fmt.bpp, r->width, r->height, r->fullscreen);
    return ok;
}

// ---- sky software renderer (library at 0x48bb30-0x48e2ff) ---------------------------------
// "sky" replaces the library's entry points with SkyRenderer. "skycmp" runs the original first and then ours, and compares
// the return values, the tables the library builds and (for Draw) the pixels written. While the original runs, calls it makes
// into other hooked entry points just pass through (g_skyInside), otherwise ours would run with half-updated state.
static Hook H_skyinit, H_skyshut, H_skyface, H_skyout, H_skyview, H_skyorigin, H_skyfov, H_skydraw, H_skyresize, H_skymode, H_skyrot, H_skymul;
static bool g_skyCompare, g_skyInside;
static unsigned g_skyStateMismatches;

static void SkyReturn(Hook &h, unsigned char orig, unsigned char ours)
{
    h.calls++;
    if (orig == ours) return;
    if (++h.mismatches <= 5) Log("MISMATCH %s: returned orig=%u ours=%u", h.name, orig, ours);
}

// the tables the original keeps in its globals, against ours
static void SkyState(Hook &h)
{
    const SkyRenderer &s = g_sky;
    int rows = s.rows;
    if (rows < 1 || rows > SkyRenderer::MAX_ROWS) return;
    const char *bad = NULL;
    if (memcmp(s.rays, (void *)0x4d2070, rows * 36)) bad = "rays";
    for (int r = 0; r < rows && !bad; r++) {
        if (memcmp(s.faceX[r], (float *)0x4c89f8 + r * 4, 12)) bad = "faceX";
        else if (memcmp(s.faceY[r], (float *)0x4cd500 + r * 4, 12)) bad = "faceY";
    }
    if (!bad && memcmp(s.view, (void *)0x4c89a8, 36)) bad = "view";
    if (!bad && memcmp(s.faceMatrix, (void *)0x4ed870, 36)) bad = "faceMatrix";
    if (!bad && memcmp(s.faceAxisNeg, (void *)0x4c8920, 12)) bad = "faceAxisNeg";
    if (!bad) return;
    g_skyStateMismatches++;
    h.mismatches++;
    if (g_skyStateMismatches <= 8) Log("MISMATCH sky state (%s) after %s, rows=%d", bad, h.name, rows);
}

// runs the original (compare mode only), then ours; returns the original's result
#define SKY_COMPARE(H, ORIG_CALL, OURS_CALL)                         \
    if (g_mode == MODE_REPLACE || !g_skyCompare) { HEAVY_LOG(H); return OURS_CALL; } \
    if (g_skyInside) return ORIG_CALL;                               \
    g_skyInside = true; unsigned char a = ORIG_CALL; g_skyInside = false; \
    unsigned char b = OURS_CALL;                                     \
    SkyReturn(H, a, b); SkyState(H);                                 \
    return a;

static unsigned char (__cdecl *o_skyinit)(int, int, unsigned char);
static unsigned char __cdecl h_skyinit(int r, int c, unsigned char f) { SKY_COMPARE(H_skyinit, o_skyinit(r, c, f), g_sky.Init(r, c, f)) }
static unsigned char (__cdecl *o_skyshut)();
static unsigned char __cdecl h_skyshut() { SKY_COMPARE(H_skyshut, o_skyshut(), g_sky.Shutdown()) }
static unsigned char (__cdecl *o_skyface)(const void *, const WarpFace *);
static unsigned char __cdecl h_skyface(const void *p, const WarpFace *f) { SKY_COMPARE(H_skyface, o_skyface(p, f), g_sky.SetFace(p, f)) }
static unsigned char (__cdecl *o_skyout)(void *, int);
static unsigned char __cdecl h_skyout(void *bits, int d) { SKY_COMPARE(H_skyout, o_skyout(bits, d), g_sky.SetOutput(bits, d)) }
static unsigned char (__cdecl *o_skyview)(const float *);
static unsigned char __cdecl h_skyview(const float *m) { SKY_COMPARE(H_skyview, o_skyview(m), g_sky.SetView(m)) }
static unsigned char (__cdecl *o_skyorigin)(float, float, float);
static unsigned char __cdecl h_skyorigin(float x, float y, float z) { SKY_COMPARE(H_skyorigin, o_skyorigin(x, y, z), g_sky.SetOrigin(x, y, z)) }
static unsigned char (__cdecl *o_skyfov)(float, float);
static unsigned char __cdecl h_skyfov(float x, float y) { SKY_COMPARE(H_skyfov, o_skyfov(x, y), g_sky.SetFov(x, y)) }
static unsigned char (__cdecl *o_skyresize)(int, int);
static unsigned char __cdecl h_skyresize(int r, int c) { SKY_COMPARE(H_skyresize, o_skyresize(r, c), g_sky.Resize(r, c)) }
static unsigned char (__cdecl *o_skymode)(char);
static unsigned char __cdecl h_skymode(char m) { SKY_COMPARE(H_skymode, o_skymode(m), g_sky.SetMode(m)) }

// Draw: ours renders into a copy of the output buffer so both results can be compared
static unsigned char (__cdecl *o_skydraw)();
static unsigned char __cdecl h_skydraw()
{
    if (g_mode == MODE_REPLACE || !g_skyCompare) { HEAVY_LOG(H_skydraw); return g_sky.Draw(); }
    if (g_skyInside) return o_skydraw();
    g_skyInside = true; unsigned char a = o_skydraw(); g_skyInside = false;
    void *real = g_sky.output;
    size_t bytes = (size_t)g_sky.rows * g_sky.cols * (g_sky.depth / 8);
    if (!g_sky.initialized || !real || !bytes) { H_skydraw.calls++; return a; }
    static BYTE *shadow; static size_t shadowSize;
    if (shadowSize < bytes) { shadow = (BYTE *)realloc(shadow, bytes); shadowSize = bytes; }
    g_sky.output = shadow;
    unsigned char b = g_sky.Draw();
    g_sky.output = real;
    SkyReturn(H_skydraw, a, b);
    if (a == 0) CompareMem(H_skydraw, real, shadow, bytes);
    return a;
}

static void (__cdecl *o_skyrot)(float *, int, float);
static void __cdecl h_skyrot(float *out, int axis, float angle)
{
    if (g_mode == MODE_REPLACE || !g_skyCompare) { HEAVY_LOG(H_skyrot); SkyRenderer::RotationMatrix(out, axis, angle); return; }
    if (g_skyInside) { o_skyrot(out, axis, angle); return; }
    g_skyInside = true; o_skyrot(out, axis, angle); g_skyInside = false;
    float mine[9]; SkyRenderer::RotationMatrix(mine, axis, angle);
    CompareMem(H_skyrot, out, mine, sizeof mine);
}
static void (__cdecl *o_skymul)(float *, const float *, const float *);
static void __cdecl h_skymul(float *out, const float *a, const float *b)
{
    if (g_mode == MODE_REPLACE || !g_skyCompare) { HEAVY_LOG(H_skymul); SkyRenderer::MatrixMultiply(out, a, b); return; }
    if (g_skyInside) { o_skymul(out, a, b); return; }
    g_skyInside = true; o_skymul(out, a, b); g_skyInside = false;
    float mine[9]; SkyRenderer::MatrixMultiply(mine, a, b);
    CompareMem(H_skymul, out, mine, sizeof mine);
}

// ---- image loading (GIF reader and pixel format conversion, 0x477a90 and 0x477cc0) -------
// "img" replaces both with ImageLoad / ImageConvertRow. "imgcmp" runs the original, then ours, and compares the pixels,
// the size and the palette entries the file defines.
static Hook H_imgload, H_imgrow;
static bool g_imgCompare;

static int PaletteEntriesInFile(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    unsigned char h[13] = {};
    size_t n = fread(h, 1, sizeof h, f);
    fclose(f);
    if (n < sizeof h || !(h[10] & 0x80)) return 0;
    int count = 1 << ((h[10] & 0xf) + 1);
    return count > 256 ? 256 : count;
}

static int (__cdecl *o_imgload)(const char *, const int *, void *, int *, int *, void **, unsigned char *);
static int __cdecl h_imgload(const char *path, const int *fmt, void *alloc, int *outW, int *outH, void **pixels, unsigned char *pal)
{
    if (g_mode == MODE_REPLACE || !g_imgCompare) {
        HEAVY_LOG(H_imgload);
        return ImageLoad(path, (const DisplayFormat *)fmt, (ImageAllocFn)alloc, outW, outH, pixels, pal);
    }
    int r = o_imgload(path, fmt, alloc, outW, outH, pixels, pal);
    int w2 = 0, h2 = 0; void *p2 = NULL; unsigned char pal2[768] = {};
    int r2 = ImageLoad(path, (const DisplayFormat *)fmt, (ImageAllocFn)alloc, &w2, &h2, &p2, pal2);
    H_imgload.calls++;
    bool bad = (r != r2);
    if (!bad && r) {
        int bpp = fmt[0];
        size_t stride = (((size_t)(bpp / 8) * (size_t)*outW) + 3) & ~(size_t)3;
        bad = (*outW != w2 || *outH != h2) || memcmp(*pixels, p2, stride * (size_t)*outH) != 0;
        int n = PaletteEntriesInFile(path);
        for (int i = 0; i < n && !bad; i++)
            bad = pal[i] != pal2[i] || pal[0x100 + i] != pal2[0x100 + i] || pal[0x200 + i] != pal2[0x200 + i];
    }
    if (bad && ++H_imgload.mismatches <= 5)
        Log("MISMATCH %s: %s ret orig=%d ours=%d size orig=%dx%d ours=%dx%d bpp=%d", H_imgload.name, path, r, r2, r ? *outW : 0, r ? *outH : 0, w2, h2, fmt[0]);
    // free with the pair of the allocator used: the only caller that passes one is the texture loader (GameAlloc at 0x4153f0)
    if (p2) { if (alloc) GameFree(p2); else Lib_Free(p2); }
    return r;
}

static void (__cdecl *o_imgrow)(void *, const int *, const unsigned char *, const unsigned char *, unsigned);
static void __cdecl h_imgrow(void *dst, const int *fmt, const unsigned char *src, const unsigned char *pal, unsigned width)
{
    if (g_mode == MODE_REPLACE || !g_imgCompare) {
        HEAVY_LOG(H_imgrow);
        ImageConvertRow(dst, (const DisplayFormat *)fmt, src, pal, width);
        return;
    }
    o_imgrow(dst, fmt, src, pal, width);
    size_t bytes = fmt[1] ? width : (size_t)width * (fmt[0] / 8);
    if (bytes > 8192) { H_imgrow.calls++; return; }       // (rows wider than that are not compared)
    BYTE mine[8192] = {}, orig[8192];
    memcpy(orig, dst, bytes);
    ImageConvertRow(mine, (const DisplayFormat *)fmt, src, pal, width);
    CompareMem(H_imgrow, orig, mine, bytes);
}

// ---- Bitmap, the DirectDraw surface wrapper for the 2D artwork (0x473760-0x474230) ---------
// Replace only ("bmp"): the methods draw and lock surfaces, which cannot be compared on a snapshot. Virtual methods are
// reached through the original vtable, so detouring the functions the vtable points to covers the original callers too.
#define THISCALL __attribute__((thiscall))
static Hook H_bmpctor, H_bmpdtor, H_bmpcopy, H_bmprect, H_bmpsized, H_bmpcreate, H_bmpcapture, H_bmpdrawsurf, H_bmpdrawat, H_bmpdrawbmp, H_bmpload;

static void *(THISCALL *o_bmpctor)(Bitmap *, Renderer *);
static void *THISCALL h_bmpctor(Bitmap *b, Renderer *r) { HEAVY_LOG(H_bmpctor); new (b) Bitmap(r); return b; }
static void (__fastcall *o_bmpdtor)(Bitmap *);
static void __fastcall h_bmpdtor(Bitmap *b) { HEAVY_LOG(H_bmpdtor); b->~Bitmap(); }
static BOOL (THISCALL *o_bmpcopy)(Bitmap *, Bitmap *);
static BOOL THISCALL h_bmpcopy(Bitmap *b, Bitmap *src) { HEAVY_LOG(H_bmpcopy); return b->CopyFrom(src); }
static BOOL (THISCALL *o_bmprect)(Bitmap *, Bitmap *, int, int, int, int, int, int, int, int);
static BOOL THISCALL h_bmprect(Bitmap *b, Bitmap *src, int a, int c, int d, int e, int f, int g, int h, int i)
{ HEAVY_LOG(H_bmprect); return b->CopyRectFrom(src, a, c, d, e, f, g, h, i); }
static LPDIRECTDRAWSURFACE (THISCALL *o_bmpsized)(Bitmap *, int, int);
static LPDIRECTDRAWSURFACE THISCALL h_bmpsized(Bitmap *b, int w, int h) { HEAVY_LOG(H_bmpsized); return b->CreateSurfaceSized(w, h); }
static LPDIRECTDRAWSURFACE (__fastcall *o_bmpcreate)(Bitmap *);
static LPDIRECTDRAWSURFACE __fastcall h_bmpcreate(Bitmap *b) { HEAVY_LOG(H_bmpcreate); return b->CreateSurface(); }
static BOOL (THISCALL *o_bmpcapture)(Bitmap *, LPDIRECTDRAWSURFACE, int, int);
static BOOL THISCALL h_bmpcapture(Bitmap *b, LPDIRECTDRAWSURFACE s, int x, int y) { HEAVY_LOG(H_bmpcapture); return b->CaptureFrom(s, x, y); }
static BOOL (THISCALL *o_bmpdrawsurf)(Bitmap *, LPDIRECTDRAWSURFACE, int, int, int, int, int, int, int, int);
static BOOL THISCALL h_bmpdrawsurf(Bitmap *b, LPDIRECTDRAWSURFACE d, int a, int c, int e, int f, int g, int h, int i, int j)
{ HEAVY_LOG(H_bmpdrawsurf); return b->DrawToSurface(d, a, c, e, f, g, h, i, j); }
static BOOL (THISCALL *o_bmpdrawat)(Bitmap *, LPDIRECTDRAWSURFACE, int, int, int, int, int);
static BOOL THISCALL h_bmpdrawat(Bitmap *b, LPDIRECTDRAWSURFACE d, int x, int y, int sx, int sy, int key)
{ HEAVY_LOG(H_bmpdrawat); return b->DrawToSurfaceAt(d, x, y, sx, sy, key); }
static BOOL (THISCALL *o_bmpdrawbmp)(Bitmap *, Bitmap *, int, int, int, int);
static BOOL THISCALL h_bmpdrawbmp(Bitmap *b, Bitmap *d, int x, int y, int sx, int sy)
{ HEAVY_LOG(H_bmpdrawbmp); return b->DrawToBitmap(d, x, y, sx, sy); }
static LPDIRECTDRAWSURFACE (THISCALL *o_bmpload)(Bitmap *, const char *);
static LPDIRECTDRAWSURFACE THISCALL h_bmpload(Bitmap *b, const char *path) { HEAVY_LOG(H_bmpload); return b->Load(path); }

// ---- String (reference counted game string; replace only, "str") -----------------------------------------------------------
// ToFloat also runs the original on the same text and compares, since atof is the one call here that is not plain pointer work.
static Hook H_strctor, H_strcstr, H_strcopy, H_strrelease, H_strunique, H_strc_str, H_strreserve, H_strrepctor, H_strassign, H_strset,
            H_strfloat, H_strless, H_streq, H_strcmp;

static void *(THISCALL *o_strctor)(String *);
static void *THISCALL h_strctor(String *s) { HEAVY_LOG(H_strctor); s->rep = NULL; return s; }
static void *(THISCALL *o_strcstr)(String *, const char *);
static void *THISCALL h_strcstr(String *s, const char *c) { HEAVY_LOG(H_strcstr); new (s) String(c); return s; }
static void *(THISCALL *o_strcopy)(String *, const String *);
static void *THISCALL h_strcopy(String *s, const String *o) { HEAVY_LOG(H_strcopy); new (s) String(*o); return s; }
static void (__fastcall *o_strrelease)(String *);
static void __fastcall h_strrelease(String *s) { HEAVY_LOG(H_strrelease); s->Release(); }
static void (__fastcall *o_strunique)(String *);
static void __fastcall h_strunique(String *s) { HEAVY_LOG(H_strunique); s->MakeUnique(); }
static const char *(__fastcall *o_strc_str)(const String *);
static const char *__fastcall h_strc_str(const String *s) { HEAVY_LOG(H_strc_str); return s->c_str(); }
static int (THISCALL *o_strreserve)(StringRep *, unsigned);
static int THISCALL h_strreserve(StringRep *r, unsigned n) { HEAVY_LOG(H_strreserve); return r->Reserve(n); }
static void *(__fastcall *o_strrepctor)(StringRep *);
static void *__fastcall h_strrepctor(StringRep *r) { HEAVY_LOG(H_strrepctor); new (r) StringRep(); return r; }
static char *(THISCALL *o_strassign)(String *, const char *, int);
static char *THISCALL h_strassign(String *s, const char *c, int n) { HEAVY_LOG(H_strassign); return s->Assign(c, n); }
static void *(THISCALL *o_strset)(String *, const String *);
static void *THISCALL h_strset(String *s, const String *o) { HEAVY_LOG(H_strset); *s = *o; return s; }
static int (THISCALL *o_strfloat)(const String *, float *);
static int THISCALL h_strfloat(const String *s, float *out)
{
    HEAVY_LOG(H_strfloat);
    float on = 0, mine = 0;
    int ro = o_strfloat(s, &on);
    int rm = s->ToFloat(&mine);
    if (ro != rm || memcmp(&on, &mine, 4) != 0) { H_strfloat.mismatches++; Log("MISMATCH ToFloat \"%s\" orig=%d,%g ours=%d,%g", s->c_str(), ro, on, rm, mine); }
    *out = mine;
    return rm;
}
static int (*o_strless)(const String *, const String *);
static int h_strless(const String *a, const String *b) { HEAVY_LOG(H_strless); return StringLess(a, b); }
static int (*o_streq)(const String *, const String *);
static int h_streq(const String *a, const String *b) { HEAVY_LOG(H_streq); return StringEqual(a, b); }
static int (*o_strcmp)(const String *, const String *);
static int h_strcmp(const String *a, const String *b) { HEAVY_LOG(H_strcmp); return StringCompare(a, b); }

// ---- IniReader (settings cursor; "ini") ----------------------------------------------------------------------------------
// The lookups are pure reads of the cached parsed file, so each also runs the original and compares.
static Hook H_inictor, H_inictor2, H_inidtor, H_iniget, H_iniint, H_inifloats, H_inifloat, H_iniset, H_inidir, H_inipath, H_iniparse;

static void *(THISCALL *o_inictor)(IniReader *);
static void *THISCALL h_inictor(IniReader *r) { HEAVY_LOG(H_inictor); new (r) IniReader(); return r; }
static void *(THISCALL *o_inictor2)(IniReader *, const char *);
static void *THISCALL h_inictor2(IniReader *r, const char *f) { HEAVY_LOG(H_inictor2); new (r) IniReader(f); return r; }
static void (__fastcall *o_inidtor)(IniReader *);
static void __fastcall h_inidtor(IniReader *r) { HEAVY_LOG(H_inidtor); r->~IniReader(); }
static const char *(THISCALL *o_iniget)(const IniReader *, const char *);
static const char *THISCALL h_iniget(const IniReader *r, const char *key)
{
    HEAVY_LOG(H_iniget);
    const char *a = o_iniget(r, key), *b = r->GetString(key);
    if (a != b) { H_iniget.mismatches++; Log("MISMATCH GetString %s orig=%p ours=%p", key, a, b); }
    return b;
}
static int (THISCALL *o_iniint)(const IniReader *, const char *, int);
static int THISCALL h_iniint(const IniReader *r, const char *key, int def)
{
    HEAVY_LOG(H_iniint);
    int a = o_iniint(r, key, def), b = r->GetInt(key, def);
    if (a != b) { H_iniint.mismatches++; Log("MISMATCH GetInt %s orig=%d ours=%d", key, a, b); }
    return b;
}
static int (THISCALL *o_inifloats)(const IniReader *, const char *, float *);
static int THISCALL h_inifloats(const IniReader *r, const char *key, float *out)
{
    HEAVY_LOG(H_inifloats);
    float tmp[3] = { 0, 0, 0 };
    int a = o_inifloats(r, key, tmp), b = r->GetFloats(key, out);
    if (a != b || (b && memcmp(tmp, out, sizeof tmp) != 0)) { H_inifloats.mismatches++; Log("MISMATCH GetFloats %s orig=%d ours=%d", key, a, b); }
    return b;
}
static float (THISCALL *o_inifloat)(const IniReader *, const char *, float);
static float THISCALL h_inifloat(const IniReader *r, const char *key, float def)
{
    HEAVY_LOG(H_inifloat);
    float a = o_inifloat(r, key, def), b = r->GetFloat(key, def);
    if (memcmp(&a, &b, 4) != 0) { H_inifloat.mismatches++; Log("MISMATCH GetFloat %s orig=%g ours=%g", key, a, b); }
    return b;
}
static void (THISCALL *o_iniset)(IniReader *, const char *);
static void THISCALL h_iniset(IniReader *r, const char *name) { HEAVY_LOG(H_iniset); r->SetSection(name); }
static String *(THISCALL *o_inidir)(const IniReader *, String *);
static String *THISCALL h_inidir(const IniReader *r, String *result) { HEAVY_LOG(H_inidir); return r->GetDirectory(result); }
static const char *(__fastcall *o_inipath)(const IniReader *);
static const char *__fastcall h_inipath(const IniReader *r) { HEAVY_LOG(H_inipath); return r->GetPath(); }
static int (__stdcall *o_iniparse)(const char *, float *, const char **);
static int __stdcall h_iniparse(const char *s, float *out, const char **end)
{
    HEAVY_LOG(H_iniparse);
    float tmp[3]; const char *e1 = NULL, *e2 = NULL;
    int a = o_iniparse(s, tmp, &e1), b = IniParseFloats(s, out, &e2);
    if (a != b || memcmp(tmp, out, sizeof tmp) != 0 || e1 != e2) { H_iniparse.mismatches++; Log("MISMATCH IniParseFloats \"%s\"", s); }
    if (end) *end = e2;
    return b;
}

// ---- table and entry point ---------------------------------------------------------------
static Hook *g_hooks[] = { &H_ctor, &H_updateRect, &H_getClient, &H_getScreen, &H_pix1, &H_pix2, &H_desktop, &H_check,
                           &H_quality, &H_planes, &H_frames, &H_viewport, &H_enum, &H_surfaces, &H_shutdown,
                           &H_lfree, &H_lcontains, &H_ladd, &H_lremove,
                           &H_qdm, &H_enterdisplay, &H_uploadpal, &H_setpal, &H_init, &H_fade,
                           &H_swarp, &H_sinit, &H_sbegin, &H_srender, &H_spresent, &H_squality, &H_sxform, &H_schild, &H_sdel,
                           &H_cframe, &H_cframeu, &H_cdecal, &H_dframe, &H_gmatrix,
                           &H_wctor, &H_wdtor, &H_winit, &H_woutput, &H_wresize, &H_wupdate, &H_wload,
                           &H_sload, &H_scam, &H_sview, &H_fparent, &H_fvisual, &H_fchild, &H_fdel, &H_lightamb, &H_lightdir, &H_fxform, &H_fdecal,
                           &H_sqgload, &H_sqgmesh, &H_sqgmorph, &H_sqgframe, &H_texgif, &H_bpapply, &H_bpbuild, &H_abroot, &H_abbuild, &H_abdestroy, &H_pzdestroy, &H_sdtor, &H_quad,
                           &H_skyinit, &H_skyshut, &H_skyface, &H_skyout, &H_skyview, &H_skyorigin, &H_skyfov, &H_skydraw, &H_skyresize, &H_skymode, &H_skyrot, &H_skymul,
                           &H_imgload, &H_imgrow,
                           &H_bmpctor, &H_bmpdtor, &H_bmpcopy, &H_bmprect, &H_bmpsized, &H_bmpcreate, &H_bmpcapture, &H_bmpdrawsurf, &H_bmpdrawat, &H_bmpdrawbmp, &H_bmpload,
                           &H_strctor, &H_strcstr, &H_strcopy, &H_strrelease, &H_strunique, &H_strc_str, &H_strreserve, &H_strrepctor, &H_strassign, &H_strset,
                           &H_strfloat, &H_strless, &H_streq, &H_strcmp,
                           &H_inictor, &H_inictor2, &H_inidtor, &H_iniget, &H_iniint, &H_inifloats, &H_inifloat, &H_iniset, &H_inidir, &H_inipath, &H_iniparse };
static char g_replaceList[2048];
// whole-name match in the comma separated list ("cframe" must not match "cframeu")
static bool InList(const char *list, const char *name)
{
    size_t n = strlen(name);
    for (const char *p = list; *p; ) {
        const char *end = strchr(p, 0x2c);
        size_t len = end ? (size_t)(end - p) : strlen(p);
        if (len == n && !strncmp(p, name, n)) return true;
        if (!end) break;
        p = end + 1;
    }
    return false;
}
static bool Listed(const char *id) { return id && (InList(g_replaceList, id) || InList(g_replaceList, "all")); }


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
    H_wctor   = { "Warp::Warp",               0x478c80, 5, "578bd133c0",        (void *)h_wctor,   (void **)&o_wctor,   false, 0, 0, "wctor",   0 };
    H_wdtor   = { "Warp::~Warp",              0x478cb0, 9, "56578bf9be06000000", (void *)h_wdtor,   (void **)&o_wdtor,   false, 0, 0, "wdtor",   0 };
    H_winit   = { "Warp::Init",               0x478ce0, 5, "83ec045356",        (void *)h_winit,   (void **)&o_winit,   false, 0, 0, "winit",   0 };
    H_woutput = { "Warp::SetOutput",          0x478da0, 7, "8b41188b4c2404",    (void *)h_woutput, (void **)&o_woutput, false, 0, 0, "woutput", 0 };
    H_wresize = { "Warp::Resize",             0x478dc0, 5, "83ec045356",        (void *)h_wresize, (void **)&o_wresize, false, 0, 0, "wresize", 0 };
    H_wupdate = { "Warp::Update",             0x478e10, 6, "8b411483f801",     (void *)h_wupdate, (void **)&o_wupdate, false, 0, 0, "wupdate", 0 };
    H_wload   = { "Warp::Load",               0x479090, 6, "81ec14010000",      (void *)h_wload,   (void **)&o_wload,   false, 0, 0, "wload",   0 };
    H_sload   = { "Scene::LoadWarp",          0x4769a0, 7, "64a10000000055",   (void *)h_sload,   (void **)&o_sload,   false, 0, 0, "sload",   0 };
    H_scam    = { "Scene::SetCameraTransform",0x476e10, 8, "56578bf98b74240c", (void *)h_scam,    (void **)&o_scam,    false, 0, 0, "scam",    0 };
    H_sview   = { "Scene::SetViewParams",     0x476e40, 5, "83ec045356",       (void *)h_sview,   (void **)&o_sview,   false, 0, 0, "sview",   0 };
    H_fparent = { "GetFrameParent",           0x4772a0, 7, "83ec048d442400",   (void *)h_fparent, (void **)&o_fparent, false, 0, 0, "fparent", 0 };
    H_fvisual = { "AddVisualToFrame",         0x4773a0, 5, "8b44240853",       (void *)h_fvisual, (void **)&o_fvisual, false, 0, 0, "fvisual", 0 };
    H_fchild  = { "AddChildToFrame",          0x4773c0, 5, "8b44240853",       (void *)h_fchild,  (void **)&o_fchild,  false, 0, 0, "fchild",  0 };
    H_fdel    = { "DeleteChildFromFrame",     0x4773e0, 5, "8b44240853",       (void *)h_fdel,    (void **)&o_fdel,    false, 0, 0, "fdel",    0 };
    H_lightamb = { "Scene::CreateAmbientLight",     0x477700, 7, "64a10000000055", (void *)h_lightamb, (void **)&o_lightamb, false, 0, 0, "lightamb", 0 };
    H_lightdir = { "Scene::CreateDirectionalLight", 0x4775c0, 7, "64a10000000055", (void *)h_lightdir, (void **)&o_lightdir, false, 0, 0, "lightdir", 0 };
    H_fxform   = { "ApplyFrameTransform",           0x4770d0, 6, "53568b74240c", (void *)h_fxform,   (void **)&o_fxform,   false, 0, 0, "fxform",   0 };
    H_fdecal   = { "SetFrameDecalSize",             0x477540, 7, "83ec088d442400", (void *)h_fdecal,   (void **)&o_fdecal,   false, 0, 0, "fdecal",   0 };
    H_sqgload  = { "SqgModel::Load",       0x472560, 6, "83ec24535657",     (void *)h_sqgload,  (void **)&o_sqgload,  false, 0, 0, "sqgload",  0 };
    H_sqgmesh  = { "SqgModel::CreateMesh", 0x472f60, 7, "64a10000000055",   (void *)h_sqgmesh,  (void **)&o_sqgmesh,  false, 0, 0, "sqgmesh",  0 };
    H_sqgmorph = { "SqgModel::Morph",      0x4734e0, 7, "64a10000000055",   (void *)h_sqgmorph, (void **)&o_sqgmorph, false, 0, 0, "sqgmorph", 0 };
    H_sqgframe = { "SqgModel::BuildFrame",  0x4731f0, 7, "8b44240483ec1c",   (void *)h_sqgframe, (void **)&o_sqgframe, false, 0, 0, "sqgframe", 0 };
    H_texgif  = { "CreateTextureFromGif",    0x415440, 6, "81ec1c030000",     (void *)h_texgif,  (void **)&o_texgif,  false, 0, 0, "texgif",  0 };
    H_bpapply  = { "BodyPart::Apply",       0x41d900, 6, "53568b742410",     (void *)h_bpapply,  (void **)&o_bpapply,  false, 0, 0, "bpapply",  0 };
    H_bpbuild  = { "BodyPart::Build",       0x41d990, 5, "8b44240453",       (void *)h_bpbuild,  (void **)&o_bpbuild,  false, 0, 0, "bpbuild",  0 };
    H_abroot   = { "AnimatedBody::SetRootTransform", 0x41e230, 6, "53568b742410", (void *)h_abroot, (void **)&o_abroot, false, 0, 0, "abroot", 0 };
    H_abbuild   = { "AnimatedBody::Build",         0x41dda0, 5, "8b44240453",          (void *)h_abbuild,   (void **)&o_abbuild,   false, 0, 0, "abbuild",   0 };
    H_abdestroy = { "AnimatedBody::DestroyFrames", 0x41ddf0, 5, "53568b414c",          (void *)h_abdestroy, (void **)&o_abdestroy, false, 0, 0, "abdestroy", 0 };
    H_pzdestroy = { "PZMorphMesh::Destroy",        0x471990, 5, "568b018bf1",          (void *)h_pzdestroy, (void **)&o_pzdestroy, false, 0, 0, "pzdestroy", 0 };
    H_sdtor     = { "Scene::~Scene",               0x476740, 9, "56578bf1e8f70b0000",  (void *)h_sdtor,     (void **)&o_sdtor,     false, 0, 0, "sdtor",     5 };
    H_quad      = { "Scene::CreateColoredQuad",    0x4777b0, 7, "64a10000000055",      (void *)h_quad,      (void **)&o_quad,      false, 0, 0, "quad",      0 };
    H_skyinit   = { "Sky::Init",           0x48bb30, 8,  "8b4c24048b542408",           (void *)h_skyinit,   (void **)&o_skyinit,   false, 0, 0, "sky", 0 };
    H_skyshut   = { "Sky::Shutdown",       0x48bba0, 7,  "803d3c4e4c0000",             (void *)h_skyshut,   (void **)&o_skyshut,   false, 0, 0, "sky", 0 };
    H_skyface   = { "Sky::SetFace",        0x48bbc0, 10, "83ec30803d3c4e4c0000",       (void *)h_skyface,   (void **)&o_skyface,   false, 0, 0, "sky", 0 };
    H_skyout    = { "Sky::SetOutput",      0x48bd30, 7,  "803d3c4e4c0000",             (void *)h_skyout,    (void **)&o_skyout,    false, 0, 0, "sky", 0 };
    H_skyview   = { "Sky::SetView",        0x48bd60, 13, "81ec90000000803d3c4e4c0000", (void *)h_skyview,   (void **)&o_skyview,   false, 0, 0, "sky", 0 };
    H_skyorigin = { "Sky::SetOrigin",      0x48c620, 10, "83ec30803d3c4e4c0000",       (void *)h_skyorigin, (void **)&o_skyorigin, false, 0, 0, "sky", 0 };
    H_skyfov    = { "Sky::SetFov",         0x48c680, 10, "83ec08803d3c4e4c0000",       (void *)h_skyfov,    (void **)&o_skyfov,    false, 0, 0, "sky", 0 };
    H_skydraw   = { "Sky::Draw",           0x48c710, 7,  "803d3c4e4c0000",             (void *)h_skydraw,   (void **)&o_skydraw,   false, 0, 0, "sky", 0 };
    H_skyresize = { "Sky::Resize",         0x48c830, 7,  "803d3c4e4c0000",             (void *)h_skyresize, (void **)&o_skyresize, false, 0, 0, "sky", 0 };
    H_skymode   = { "Sky::SetMode",        0x48c870, 6,  "8a44240484c0",               (void *)h_skymode,   (void **)&o_skymode,   false, 0, 0, "sky", 0 };
    H_skyrot    = { "Sky::RotationMatrix", 0x48d7a0, 6,  "d944240cd9ff",               (void *)h_skyrot,    (void **)&o_skyrot,    false, 0, 0, "sky", 0 };
    H_skymul    = { "Sky::MatrixMultiply", 0x48d860, 5,  "8b4c240453",                 (void *)h_skymul,    (void **)&o_skymul,    false, 0, 0, "sky", 0 };
    H_imgload   = { "ImageLoad",           0x477cc0, 6,  "81ec10030000",               (void *)h_imgload,   (void **)&o_imgload,   false, 0, 0, "img", 0 };
    H_bmpctor = { "Bitmap::Bitmap", 0x473760, 6, "33d28b442404", (void *)h_bmpctor, (void **)&o_bmpctor, false, 0, 0, "bmp", 0 };
    H_bmpdtor = { "Bitmap::~Bitmap", 0x473890, 9, "8b4174c70160104a00", (void *)h_bmpdtor, (void **)&o_bmpdtor, false, 0, 0, "bmp", 0 };
    H_bmpcopy = { "Bitmap::CopyFrom", 0x473780, 5, "5356578bf1", (void *)h_bmpcopy, (void **)&o_bmpcopy, false, 0, 0, "bmp", 0 };
    H_bmprect = { "Bitmap::CopyRectFrom", 0x473810, 6, "5356578b4174", (void *)h_bmprect, (void **)&o_bmprect, false, 0, 0, "bmp", 0 };
    H_bmpsized = { "Bitmap::CreateSurfaceSized", 0x4738c0, 11, "83ec04c744240000000000", (void *)h_bmpsized, (void **)&o_bmpsized, false, 0, 0, "bmp", 0 };
    H_bmpcreate = { "Bitmap::CreateSurface", 0x473940, 11, "83ec04c744240000000000", (void *)h_bmpcreate, (void **)&o_bmpcreate, false, 0, 0, "bmp", 0 };
    H_bmpcapture = { "Bitmap::CaptureFrom", 0x473e80, 6, "81ec84000000", (void *)h_bmpcapture, (void **)&o_bmpcapture, false, 0, 0, "bmp", 0 };
    H_bmpdrawsurf = { "Bitmap::DrawToSurface", 0x473f70, 5, "83ec645356", (void *)h_bmpdrawsurf, (void **)&o_bmpdrawsurf, false, 0, 0, "bmp", 0 };
    H_bmpdrawat = { "Bitmap::DrawToSurfaceAt", 0x474020, 6, "81ec88000000", (void *)h_bmpdrawat, (void **)&o_bmpdrawat, false, 0, 0, "bmp", 0 };
    H_bmpdrawbmp = { "Bitmap::DrawToBitmap", 0x474130, 6, "81ec84000000", (void *)h_bmpdrawbmp, (void **)&o_bmpdrawbmp, false, 0, 0, "bmp", 0 };
    H_bmpload = { "Bitmap::Load", 0x4739d0, 6, "81ece4030000", (void *)h_bmpload, (void **)&o_bmpload, false, 0, 0, "bmp", 0 };
    H_strctor = { "String::String()", 0x438370, 8, "8bc1c70100000000", (void *)h_strctor, (void **)&o_strctor, false, 0, 0, "str", 0 };
    H_strcstr = { "String::String(const char*)", 0x438380, 5, "8b54240456", (void *)h_strcstr, (void **)&o_strcstr, false, 0, 0, "str", 0 };
    H_strcopy = { "String::String(const String&)", 0x4383b0, 6, "8b4424048b00", (void *)h_strcopy, (void **)&o_strcopy, false, 0, 0, "str", 0 };
    H_strrelease = { "String::Release", 0x4384f0, 6, "56578b018bf9", (void *)h_strrelease, (void **)&o_strrelease, false, 0, 0, "str", 0 };
    H_strunique = { "String::MakeUnique", 0x4383e0, 7, "64a10000000055", (void *)h_strunique, (void **)&o_strunique, false, 0, 0, "str", 0 };
    H_strc_str = { "String::c_str", 0x438530, 6, "8b0185c07506", (void *)h_strc_str, (void **)&o_strc_str, false, 0, 0, "str", 0 };
    H_strreserve = { "StringRep::Reserve", 0x438540, 5, "8b44240453", (void *)h_strreserve, (void **)&o_strreserve, false, 0, 0, "str", 0 };
    H_strrepctor = { "StringRep::StringRep", 0x4385c0, 8, "5356c70100000000", (void *)h_strrepctor, (void **)&o_strrepctor, false, 0, 0, "str", 0 };
    H_strassign = { "String::Assign", 0x438620, 5, "5356578bf1", (void *)h_strassign, (void **)&o_strassign, false, 0, 0, "str", 0 };
    H_strset = { "String::operator=", 0x438670, 8, "56578bf98b74240c", (void *)h_strset, (void **)&o_strset, false, 0, 0, "str", 0 };
    H_strfloat = { "String::ToFloat", 0x4386a0, 7, "568b018bf18b08", (void *)h_strfloat, (void **)&o_strfloat, false, 0, 0, "str", 0 };
    H_strless = { "StringLess", 0x4386f0, 9, "8b4c2408e837feffff", (void *)h_strless, (void **)&o_strless, false, 0, 0, "str", 5 };
    H_streq = { "StringEqual", 0x438720, 9, "8b4c2408e807feffff", (void *)h_streq, (void **)&o_streq, false, 0, 0, "str", 5 };
    H_strcmp = { "StringCompare", 0x438750, 9, "8b4c2408e8d7fdffff", (void *)h_strcmp, (void **)&o_strcmp, false, 0, 0, "str", 5 };
    H_inictor = { "IniReader::IniReader()", 0x443120, 5, "5356578bd9", (void *)h_inictor, (void **)&o_inictor, false, 0, 0, "ini", 0 };
    H_inictor2 = { "IniReader::IniReader(file)", 0x4431b0, 5, "5356578bd9", (void *)h_inictor2, (void **)&o_inictor2, false, 0, 0, "ini", 0 };
    H_inidtor = { "IniReader::~IniReader", 0x443240, 5, "568b018bf1", (void *)h_inidtor, (void **)&o_inidtor, false, 0, 0, "ini", 0 };
    H_iniget = { "IniReader::GetString", 0x443290, 7, "8b4424048b5104", (void *)h_iniget, (void **)&o_iniget, false, 0, 0, "ini", 0 };
    H_iniint = { "IniReader::GetInt", 0x4432b0, 7, "8b4424048b5104", (void *)h_iniint, (void **)&o_iniint, false, 0, 0, "ini", 0 };
    H_inifloats = { "IniReader::GetFloats", 0x4432e0, 5, "8b44240456", (void *)h_inifloats, (void **)&o_inifloats, false, 0, 0, "ini", 0 };
    H_inifloat = { "IniReader::GetFloat", 0x443310, 7, "8b44240483ec04", (void *)h_inifloat, (void **)&o_inifloat, false, 0, 0, "ini", 0 };
    H_iniset = { "IniReader::SetSection", 0x443370, 9, "568bd157b9ffffffff", (void *)h_iniset, (void **)&o_iniset, false, 0, 0, "ini", 0 };
    H_inidir = { "IniReader::GetDirectory", 0x4433a0, 7, "64a10000000055", (void *)h_inidir, (void **)&o_inidir, false, 0, 0, "ini", 0 };
    H_inipath = { "IniReader::GetPath", 0x443460, 5, "8b01c3cccc", (void *)h_inipath, (void **)&o_inipath, false, 0, 0, "ini", 0 };
    H_iniparse = { "IniParseFloats", 0x443590, 5, "53565733db", (void *)h_iniparse, (void **)&o_iniparse, false, 0, 0, "ini", 0 };
    H_imgrow    = { "ImageConvertRow",     0x477a90, 7,  "8b4c240883ec24",             (void *)h_imgrow,    (void **)&o_imgrow,    false, 0, 0, "img", 0 };
}

static void Summary()
{
    Log("Scene frames with warp backdrop=%u", g_warpFrames);
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
        g_skyCompare = strstr(g_replaceList, "skycmp") != NULL;
        g_imgCompare = strstr(g_replaceList, "imgcmp") != NULL;
        for (Hook *h : g_hooks) {
            if (h->id) { if (Listed(h->id) || (g_skyCompare && !strcmp(h->id, "sky")) || (g_imgCompare && !strcmp(h->id, "img"))) Install(*h); }          // heavy: explicit opt-in
            else if (!h->replaceOnly || g_mode == MODE_REPLACE) Install(*h);
        }
    } else if (reason == DLL_PROCESS_DETACH) {
        Summary();
        if (g_log != INVALID_HANDLE_VALUE) CloseHandle(g_log);
    }
    return TRUE;
}
