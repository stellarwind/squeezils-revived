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
#include "Splash.h"
#include "Menu.h"
#include "Dialogs.h"
#include "World.h"
#include "TextureDefaults.h"
#include "Resources.h"
#include "Prop.h"
#include "FrameProp.h"
#include "LODProp.h"
#include "Script.h"
#include "MoodFace.h"
#include "NutCounter.h"
#include "Panel.h"
#include "StatusBar.h"
#include "GoodieList.h"
#include "QuestItems.h"
#include "QuestPanel.h"
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
void  GameLog(const char *format, ...)     // printf style like SqgError: format here and hand it over as one string
{
    char buf[1024];
    va_list ap; va_start(ap, format);
    _vsnprintf(buf, sizeof buf - 1, format, ap);
    va_end(ap);
    buf[sizeof buf - 1] = 0;
    ((void (__cdecl *)(const char *, ...))0x419290)("%s", buf);
}
IniFileTable *GetIniFiles() { return (IniFileTable *)0x4c7738; }
void *GetCurrentSimulation() { return *(void **)0x4badbc; }
const char *GetPlaceName(int index) { return ((const char *(__attribute__((thiscall)) *)(void *, int))0x41e8c0)(*(void **)0x4badb8, index); }
void *DynamicCast(void *obj, const void *from, const void *to) { return ((void *(__cdecl *)(void *, int, const void *, const void *, int))0x48f060)(obj, 0, from, to, 0); }
ScreenInfo *GetScreenInfo() { return (ScreenInfo *)*(void **)0x4badac; }
int GetKeyCode(const char *name) { return ((int (__attribute__((thiscall)) *)(void *, const char *))0x43ace0)(*(void **)0x4badac, name); }
int GameLoadGame(const char *path)
{
    void *sim = *(void **)0x4badbc;
    void **vtbl = *(void ***)sim;
    return ((int (__attribute__((thiscall)) *)(void *, const char *))vtbl[10])(sim, path);
}
HINSTANCE GetInstance() { return *(HINSTANCE *)0x4c73c8; }
HWND GetMainWindow() { return *(HWND *)0x4c73cc; }
// ---- the input object and the dialogs' other boundary calls ----
#define TC __attribute__((thiscall))
InputSystem *GetInputSystem() { return *(InputSystem **)0x4badac; }
int  InputJoystickMode(InputSystem *in)                  { return ((int (TC *)(void *))0x43aee0)(in); }
int  InputSetJoystickMode(InputSystem *in, int mode)     { return ((int (TC *)(void *, int))0x43aef0)(in, mode); }
int  InputJoystickPresent(InputSystem *in)               { return ((int (TC *)(void *))0x43c7a0)(in); }
int  InputActionName(InputSystem *in, int index, char *line) { return ((int (TC *)(void *, int, char *))0x43c7b0)(in, index, line); }
void InputRevertBindings(InputSystem *in, int which)     { ((void (TC *)(void *, int))0x43ad30)(in, which); }
void InputBindingLabel(InputSystem *in, const char *line, char *keyName) { ((void (TC *)(void *, const char *, char *))0x43ad00)(in, line, keyName); }
int  InputAssignKey(InputSystem *in, char *line, const char *keyName)
{
    return ((int (TC *)(void *, char *, const char *))(*(void ***)in)[0])(in, line, keyName);
}
int  InputSetKeySlot(InputSystem *in, char *line, int slot)
{
    return ((int (TC *)(void *, char *, int))(*(void ***)in)[1])(in, line, slot);
}
int  InputSetButtonSlot(InputSystem *in, char *line, int slot) { return ((int (TC *)(void *, char *, int))0x43ae70)(in, line, slot); }
int  InputSaveBindings(InputSystem *in)                  { return ((int (TC *)(void *))0x43c4d0)(in); }
void *InputCalibration(InputSystem *in)                  { return *(void **)((char *)in + 0x10); }
void CalibrationStart(void *cal)                         { ((void (TC *)(void *))0x447880)(cal); }
int  CalibrationCheck(void *cal)                         { return ((int (TC *)(void *))0x4479c0)(cal); }
int  CalibrationRun(void *cal, HWND dlg)                 { return ((int (TC *)(void *, HWND))0x447f00)(cal, dlg); }
// ---- the world ----
World *GetWorld()                                        { return *(World **)0x4bada8; }
void *NavMeshConstruct(void *memory)                     { return ((void *(TC *)(void *))0x47aec0)(memory); }
int   NavMeshLoad(void *nav, const char *path)           { return ((int (TC *)(void *, const char *))0x47c8e0)(nav, path); }
void  NavMeshDestroy(void *nav)                          { ((void (TC *)(void *))0x47af00)(nav); }
int   LoadPaletteFile(const char *path, unsigned char *rgba) { return ((int (__cdecl *)(const char *, unsigned char *))0x474ca0)(path, rgba); }
void  ClearWorldObjects()                                { ((void (__cdecl *)())0x42d010)(); }
// ---- the resource loaders' model calls ----
SqgModel *SqgModelConstruct(void *memory)                { return ((SqgModel *(TC *)(void *))0x4719d0)(memory); }
void  SqgModelDestruct(SqgModel *model)                  { ((void (TC *)(void *))0x471a00)(model); }
int   SqgModelLoadWrl(SqgModel *model, const char *path, int flag) { return ((int (TC *)(void *, const char *, int))0x472480)(model, path, flag); }
void  SqgModelSave(SqgModel *model, const char *path)    { ((void (TC *)(void *, const char *))0x472b90)(model, path); }
void  SqgSetErrorCallback(void (__cdecl *fn)(const char *))                    { ((void (__cdecl *)(void *))0x4736f0)((void *)fn); }
void  SqgSetTextureLookup(IDirect3DRMTexture *(__cdecl *fn)(const char *))     { ((void (__cdecl *)(void *))0x473700)((void *)fn); }
IDirect3DRMFrame *LoadWrlFrame(IDirect3DRM *d3drm, const char *path, IDirect3DRMTexture *(__cdecl *lookup)(const char *))
{
    return ((IDirect3DRMFrame *(__cdecl *)(IDirect3DRM *, const char *, void *))0x47a580)(d3drm, path, (void *)lookup);
}
void  WrlSetLogCallback(void (__cdecl *fn)(const char *))   { ((void (__cdecl *)(void *))0x47a790)((void *)fn); }
void  WrlSetErrorCallback(void (__cdecl *fn)(const char *)) { ((void (__cdecl *)(void *))0x47a7a0)((void *)fn); }
void  AddUpdatingProp(Prop *prop)                        { ((void (__cdecl *)(Prop *))0x42d180)(prop); }
void  RemoveUpdatingProp(Prop *prop)                     { ((void (__cdecl *)(Prop *))0x42d1a0)(prop); }
Prop *FindPropTemplate(const char *name, int unknown1, int unknown2)
{
    String s(name);                       // passed by value (the rep pointer); the callee releases it
    StringRep *rep = s.rep;
    s.rep = NULL;
    return ((Prop *(__cdecl *)(StringRep *, int, int))0x42c8b0)(rep, unknown1, unknown2);
}
void *SimulationActor(int index)                         { return ((void *(TC *)(void *, int))0x414050)(*(void **)0x4badbc, index); }
void *SimulationSoul(int index)                          { return ((void *(TC *)(void *, int))0x414080)(*(void **)0x4badbc, index); }
float *ActorMatrix(void *actor)                          { return ((float *(TC *)(void *))(*(void ***)actor)[3])(actor); }
float *CameraMatrix()
{
    char *simulation = *(char **)0x4badbc;
    char *table = *(char **)(simulation + 4);                   // @414150
    char **values = *(char ***)(table + 0x24);
    void *actor = *(void **)(values[1] + 4);
    return ((float *(TC *)(void *))(*(void ***)actor)[3])(actor);
}
ScriptCaller *ScriptCallerInit(ScriptCaller *c, int actorIndex, Prop *prop)
{
    return ((ScriptCaller *(TC *)(ScriptCaller *, int, Prop *))0x4207a0)(c, actorIndex, prop);
}
ScriptCaller *ScriptCallerInitSoul(ScriptCaller *c, void *soul, int actorIndex, Prop *prop)
{
    return ((ScriptCaller *(TC *)(ScriptCaller *, void *, int, Prop *))0x420900)(c, soul, actorIndex, prop);
}
int  SimulationCanSave(void *sim)                        { return ((int (TC *)(void *))(*(void ***)sim)[0x24 / 4])(sim); }
int  SimulationSave(void *sim, const char *path)         { return ((int (TC *)(void *, const char *))(*(void ***)sim)[0x2c / 4])(sim, path); }
void SetSensitivity1(float value)                        { ((void (__cdecl *)(float))0x44aa10)(value); }
void SetSensitivity2(float value)                        { ((void (__cdecl *)(float))0x44aab0)(value); }
void SetSensitivity3(float value)                        { ((void (__cdecl *)(float))0x44a940)(value); }
void *GetPlayerSoul() { return ((void *(__attribute__((thiscall)) *)(void *, int))0x414080)(*(void **)0x4badbc, 0); }
int SoulMood(void *soul)
{
    void *s = DynamicCast(soul, kSoulType, kSqueezilPlayerSoulType);
    return ((int (__attribute__((thiscall)) *)(void *))(*(void ***)s)[0x64 / 4])(s);
}
void *GetPanel() { return *(void **)0x4badb0; }
BOOL GoodieCount(const char *name, int *count)
{
    void *soul = GetPlayerSoul();
    if (!soul)
        return FALSE;
    void *inv = ((void *(__attribute__((thiscall)) *)(void *))(*(void ***)soul)[0x48 / 4])(soul);
    if (!inv)
        return FALSE;
    void *table = *(void **)((char *)inv + 4);
    *count = ((int (__attribute__((thiscall)) *)(void *, const char *))0x416300)(table, name);
    return TRUE;
}
void StoreSetting(const char *name, void *data, int size)
{
    String s(name);                       // the original passes a pointer to the String object and keeps ownership
    void *store = (void *)0x4c7060;
    void *fn = (*(void ***)store)[0];
    ((void (__attribute__((thiscall)) *)(void *, String *, void *, int))fn)(store, &s, data, size);
}
BOOL CurrentGoodieName(String *name)
{
    void *soul = GetPlayerSoul();
    if (!soul)
        return FALSE;
    void *inv = ((void *(__attribute__((thiscall)) *)(void *))(*(void ***)soul)[0x48 / 4])(soul);
    if (!inv)
        return FALSE;
    void *table = *(void **)((char *)inv + 4);
    *name = *(String *)((char *)table + 8);
    return TRUE;
}
void *PlayerInventory()
{
    void *soul = GetPlayerSoul();
    if (!soul)
        return NULL;
    void *inv = ((void *(__attribute__((thiscall)) *)(void *))(*(void ***)soul)[0x48 / 4])(soul);
    if (!inv)
        return NULL;
    return *(void **)((char *)inv + 4);
}
float *PlayerMatrix()
{
    void *obj = ((void *(__attribute__((thiscall)) *)(void *, int))0x414050)(*(void **)0x4badbc, 0);
    if (!obj)
        return NULL;
    return ((float *(__attribute__((thiscall)) *)(void *))(*(void ***)obj)[3])(obj);
}
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
static const size_t kTrampPool = 16384;  // 32 bytes per hook: 512 hooks

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
    if (!g_tramp || g_trampUsed + 32 > kTrampPool || h.stolen > 27) { Log("SKIP %s: no room for its trampoline", h.name); return false; }
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
static void (__attribute__((thiscall)) *o_bpapply)(BodyPart *, IDirect3DRMFrame *, const PartPose *);
static void __attribute__((thiscall)) h_bpapply(BodyPart *b, IDirect3DRMFrame *ref, const PartPose *p) { HEAVY_LOG(H_bpapply); b->Apply(ref, p); }
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
static void BitmapTrace(const char *path, int ok) { Log("bmp load: %s -> %s", path, ok ? "ok" : "FAILED"); }
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

// ---- containers ("ctn") and the parsed ini files ("inip") ---------------------------------------------------------------
// Each original instantiation has its own copy of every method; all copies of one shape share a wrapper template, tied to
// its Hook entry for the call count. Methods are called qualified (A::Insert), so the wrappers never dispatch through the
// vtable, which would lead back into the hook. Pure lookups also run the original and compare; everything else replaces.
// Functions that return a String by value take the MSVC result pointer as the first stack argument.
typedef Array<IniFileSection *> SectionArray;
typedef Array<IniFileCache *> FileArray;
typedef Table<String, String, StringList, 23> ValueTable;
typedef Table<String, IniFileSection *, SectionArray, 50> SectionTable;

template<class A, Hook &H> static A *THISCALL W_ctor(A *a) { H.calls++; return ::new (a) A(); }
template<class A, Hook &H> static A *THISCALL W_copy(A *a, const A *o) { H.calls++; return ::new (a) A(*o); }
template<class A, Hook &H> static void THISCALL W_dtor(A *a) { H.calls++; a->~A(); }
template<class A, Hook &H> static String *THISCALL W_strAt(A *a, String *result, int i) { H.calls++; return new (result) String(a->At(i)); }
template<class T, Hook &H> static T THISCALL W_get(Array<T> *a, int i) { H.calls++; return a->Array<T>::Get(i); }
template<class A, class T, Hook &H> static int THISCALL W_insert(A *a, const T *v, int at) { H.calls++; return a->A::Insert(*v, at); }
template<class A, Hook &H> static void THISCALL W_removeAt(A *a, int i) { H.calls++; a->A::RemoveAt(i); }
template<class A, class T, Hook &H> static void THISCALL W_replace(A *a, int i, const T *v) { H.calls++; a->A::Replace(i, *v); }
template<class T, Hook &H> static T *THISCALL W_data(Array<T> *a) { H.calls++; return a->Array<T>::Data(); }
template<class A, Hook &H> static int THISCALL W_grow(A *a, int n) { H.calls++; return a->A::Grow(n); }
template<class A, class K, Hook &H> static void THISCALL W_removeKey(A *a, const K *k) { H.calls++; a->A::RemoveKey(*k); }
template<class A, class K, class V, Hook &H> static int THISCALL W_tinsert(A *a, const K *k, const V *v, int hint) { H.calls++; return a->A::Insert(*k, *v, hint); }
template<class A, Hook &H> static String *THISCALL W_tgetStr(A *a, String *result, const String *k) { H.calls++; return new (result) String(a->Get(*k)); }
template<class A, Hook &H> static String *THISCALL W_tgetAtStr(A *a, String *result, int i) { H.calls++; return new (result) String(a->GetAt(i)); }

// run the original (trampoline) and ours, count a mismatch, return ours
#define CTN_COMPARE(h, orig, ours) \
    { (h).calls++; auto o = (orig); auto m = (ours); \
      if (o != m) { (h).mismatches++; Log("MISMATCH %s orig=%08x ours=%08x", (h).name, (unsigned)o, (unsigned)m); } return m; }
template<class A, class K, Hook &H> static int THISCALL W_contains(A *a, const K *k)
    CTN_COMPARE(H, ((int (THISCALL *)(A *, const K *))*H.orig)(a, k), a->A::Contains(*k))
template<class A, class K, Hook &H> static int THISCALL W_indexOf(A *a, const K *k)
    CTN_COMPARE(H, ((int (THISCALL *)(A *, const K *))*H.orig)(a, k), a->A::IndexOf(*k))
template<class A, class K, class V, Hook &H> static V THISCALL W_tget(A *a, const K *k)
    CTN_COMPARE(H, ((V (THISCALL *)(A *, const K *))*H.orig)(a, k), a->Get(*k))

static Hook H_as_ctor, H_as_copy, H_as_dtor, H_as_get, H_as_at, H_as_insert, H_as_remove, H_as_replace, H_as_data, H_as_grow,
            H_ai_ctor, H_ai_dtor, H_ai_get, H_ai_insert, H_ai_remove, H_ai_replace, H_ai_data, H_ai_grow,
            H_asec_get, H_asec_insert, H_asec_remove, H_asec_replace, H_asec_data, H_asec_grow, H_asec_dtor,
            H_afile_get, H_afile_insert, H_afile_remove, H_afile_replace, H_afile_data, H_afile_grow, H_afile_dtor,
            H_pa3_dtor, H_pa23_dtor, H_pa50_dtor, H_pa51_dtor,
            H_s23_insert, H_s23_replace, H_s23_contains, H_s23_indexof, H_s23_removekey, H_s23_dtor,
            H_s50_insert, H_s50_replace, H_s50_contains, H_s50_indexof, H_s50_removekey, H_s50_dtor,
            H_s51_insert, H_s51_replace, H_s51_contains, H_s51_indexof, H_s51_removekey, H_s51_dtor,
            H_t23_ctor, H_t23_dtor, H_t23_insert, H_t23_contains, H_t23_indexof, H_t23_removeat, H_t23_removekey, H_t23_get, H_t23_getat,
            H_bot23_dtor,
            H_t50_ctor, H_t50_dtor, H_t50_insert, H_t50_contains, H_t50_indexof, H_t50_removeat, H_t50_removekey, H_t50_get,
            H_t51_dtor, H_t51_insert, H_t51_contains, H_t51_indexof, H_t51_removeat, H_t51_removekey, H_t51_get;

static Hook H_ip_skip, H_ip_back, H_ip_blank, H_ip_add, H_ip_secfind, H_ip_seckeys, H_ip_filedtor, H_ip_load, H_ip_filekeys,
            H_ip_find, H_ip_open, H_ip_closeall, H_ip_intlist, H_ip_keys, H_ip_parseints;
static char *__cdecl h_ip_skip(char *s) CTN_COMPARE(H_ip_skip, ((char *(__cdecl *)(char *))*H_ip_skip.orig)(s), IniSkipBlanks(s))
static char *__cdecl h_ip_back(char *a, char *p) CTN_COMPARE(H_ip_back, ((char *(__cdecl *)(char *, char *))*H_ip_back.orig)(a, p), IniBackOverBlanks(a, p))
static char *__cdecl h_ip_blank(char *s) CTN_COMPARE(H_ip_blank, ((char *(__cdecl *)(char *))*H_ip_blank.orig)(s), IniFindBlank(s))
static void THISCALL h_ip_add(IniFileSection *s, char *line, const char *sect) { H_ip_add.calls++; s->Add(line, sect); }
static const char *THISCALL h_ip_secfind(IniFileSection *s, const char *key)
    CTN_COMPARE(H_ip_secfind, ((const char *(THISCALL *)(IniFileSection *, const char *))*H_ip_secfind.orig)(s, key), s->Find(key))
static StringList *THISCALL h_ip_seckeys(IniFileSection *s) { H_ip_seckeys.calls++; return s->Keys(); }
static void THISCALL h_ip_filedtor(IniFileCache *f) { H_ip_filedtor.calls++; f->~IniFileCache(); }
static int THISCALL h_ip_load(IniFileCache *f, const char *path) { H_ip_load.calls++; return f->Load(path); }
static StringList *THISCALL h_ip_filekeys(IniFileCache *f, const char *sect)
    CTN_COMPARE(H_ip_filekeys, ((StringList *(THISCALL *)(IniFileCache *, const char *))*H_ip_filekeys.orig)(f, sect), f->Keys(sect))
static const char *THISCALL h_ip_find(IniFileCache *f, const char *sect, const char *key)
    CTN_COMPARE(H_ip_find, ((const char *(THISCALL *)(IniFileCache *, const char *, const char *))*H_ip_find.orig)(f, sect, key), f->Find(sect, key))
static IniFileCache *__cdecl h_ip_open(const char *path) { H_ip_open.calls++; return IniOpen(path); }
static void __cdecl h_ip_closeall() { H_ip_closeall.calls++; IniCloseAll(); }
static IntList *THISCALL h_ip_intlist(const IniReader *r, const char *key) { H_ip_intlist.calls++; return r->GetIntList(key); }
static StringList *THISCALL h_ip_keys(const IniReader *r) { H_ip_keys.calls++; return r->GetKeys(); }
static IntList *THISCALL h_ip_parseints(const IniReader *r, const char *s) { H_ip_parseints.calls++; return r->ParseIntList(s); }

static Hook *g_ctnHooks[] = {
    &H_as_ctor, &H_as_copy, &H_as_dtor, &H_as_get, &H_as_at, &H_as_insert, &H_as_remove, &H_as_replace, &H_as_data, &H_as_grow,
    &H_ai_ctor, &H_ai_dtor, &H_ai_get, &H_ai_insert, &H_ai_remove, &H_ai_replace, &H_ai_data, &H_ai_grow,
    &H_asec_get, &H_asec_insert, &H_asec_remove, &H_asec_replace, &H_asec_data, &H_asec_grow, &H_asec_dtor,
    &H_afile_get, &H_afile_insert, &H_afile_remove, &H_afile_replace, &H_afile_data, &H_afile_grow, &H_afile_dtor,
    &H_pa3_dtor, &H_pa23_dtor, &H_pa50_dtor, &H_pa51_dtor,
    &H_s23_insert, &H_s23_replace, &H_s23_contains, &H_s23_indexof, &H_s23_removekey, &H_s23_dtor,
    &H_s50_insert, &H_s50_replace, &H_s50_contains, &H_s50_indexof, &H_s50_removekey, &H_s50_dtor,
    &H_s51_insert, &H_s51_replace, &H_s51_contains, &H_s51_indexof, &H_s51_removekey, &H_s51_dtor,
    &H_t23_ctor, &H_t23_dtor, &H_t23_insert, &H_t23_contains, &H_t23_indexof, &H_t23_removeat, &H_t23_removekey, &H_t23_get, &H_t23_getat,
    &H_bot23_dtor,
    &H_t50_ctor, &H_t50_dtor, &H_t50_insert, &H_t50_contains, &H_t50_indexof, &H_t50_removeat, &H_t50_removekey, &H_t50_get,
    &H_t51_dtor, &H_t51_insert, &H_t51_contains, &H_t51_indexof, &H_t51_removeat, &H_t51_removekey, &H_t51_get,
    &H_ip_skip, &H_ip_back, &H_ip_blank, &H_ip_add, &H_ip_secfind, &H_ip_seckeys, &H_ip_filedtor, &H_ip_load, &H_ip_filekeys,
    &H_ip_find, &H_ip_open, &H_ip_closeall, &H_ip_intlist, &H_ip_keys, &H_ip_parseints };

static void SetupContainers()
{
    static void *orig[sizeof g_ctnHooks / sizeof g_ctnHooks[0]];
    static int used;
    auto C = [](Hook &h, const char *name, DWORD addr, int stolen, const char *hex, void *fn, const char *id) {
        h = { name, addr, stolen, hex, fn, &orig[used++], false, 0, 0, id, 0 };
    };
    // Array<String> (shared by every PrimitiveArray<String,N> and Set<String,N>)
    C(H_as_ctor,    "Array<String>::Array",      0x45e1b0, 6, "64a100000000", (void *)W_ctor<Array<String>, H_as_ctor>, "ctn");
    C(H_as_copy,    "Array<String>::Array(copy)", 0x45e0e0, 6, "64a100000000", (void *)W_copy<Array<String>, H_as_copy>, "ctn");
    C(H_as_dtor,    "Array<String>::~Array",     0x45e250, 10, "568b410cc7010c004a00", (void *)W_dtor<Array<String>, H_as_dtor>, "ctn");
    C(H_as_get,     "Array<String>::Get",        0x40ec10, 6, "64a100000000", (void *)W_strAt<Array<String>, H_as_get>, "ctn");
    C(H_as_at,      "Array<String>::At",         0x40f310, 6, "64a100000000", (void *)W_strAt<Array<String>, H_as_at>, "ctn");
    C(H_as_insert,  "Array<String>::Insert",     0x45e280, 5, "53568b0157", (void *)W_insert<Array<String>, String, H_as_insert>, "ctn");
    C(H_as_remove,  "Array<String>::RemoveAt",   0x45e300, 5, "53568b4104", (void *)W_removeAt<Array<String>, H_as_remove>, "ctn");
    C(H_as_replace, "Array<String>::Replace",    0x45e340, 7, "8b4424088b490c", (void *)W_replace<Array<String>, String, H_as_replace>, "ctn");
    C(H_as_data,    "Array<String>::Data",       0x45e360, 5, "8b410cc3cc", (void *)W_data<String, H_as_data>, "ctn");
    C(H_as_grow,    "Array<String>::Grow",       0x45e370, 6, "64a100000000", (void *)W_grow<Array<String>, H_as_grow>, "ctn");
    // Array<int> (IntList)
    C(H_ai_ctor,    "Array<int>::Array",         0x45d8d0, 9, "566a08c701ccff4900", (void *)W_ctor<Array<int>, H_ai_ctor>, "ctn");
    C(H_ai_dtor,    "Array<int>::~Array",        0x45d900, 9, "8b410cc701ccff4900", (void *)W_dtor<Array<int>, H_ai_dtor>, "ctn");
    C(H_ai_get,     "Array<int>::Get",           0x40ec80, 7, "8b490c8b442404", (void *)W_get<int, H_ai_get>, "ctn");
    C(H_ai_insert,  "Array<int>::Insert",        0x45d920, 5, "53568b0157", (void *)W_insert<Array<int>, int, H_ai_insert>, "ctn");
    C(H_ai_remove,  "Array<int>::RemoveAt",      0x45d990, 8, "568b41048b742408", (void *)W_removeAt<Array<int>, H_ai_remove>, "ctn");
    C(H_ai_replace, "Array<int>::Replace",       0x45d9d0, 7, "8b4424088b490c", (void *)W_replace<Array<int>, int, H_ai_replace>, "ctn");
    C(H_ai_data,    "Array<int>::Data",          0x45d9f0, 5, "8b410cc3cc", (void *)W_data<int, H_ai_data>, "ctn");
    C(H_ai_grow,    "Array<int>::Grow",          0x45da00, 5, "53568b5108", (void *)W_grow<Array<int>, H_ai_grow>, "ctn");
    // Array<IniFileSection*>, Array<IniFileCache*> (constructors inlined in the Table constructors)
    C(H_asec_get,     "Array<IniFileSection*>::Get",      0x46d400, 7, "8b490c8b442404", (void *)W_get<IniFileSection *, H_asec_get>, "ctn");
    C(H_asec_insert,  "Array<IniFileSection*>::Insert",   0x46dc60, 5, "53568b0157", (void *)W_insert<SectionArray, IniFileSection *, H_asec_insert>, "ctn");
    C(H_asec_remove,  "Array<IniFileSection*>::RemoveAt", 0x46dcd0, 8, "568b41048b742408", (void *)W_removeAt<SectionArray, H_asec_remove>, "ctn");
    C(H_asec_replace, "Array<IniFileSection*>::Replace",  0x46dd10, 7, "8b4424088b490c", (void *)W_replace<SectionArray, IniFileSection *, H_asec_replace>, "ctn");
    C(H_asec_data,    "Array<IniFileSection*>::Data",     0x46dd30, 5, "8b410cc3cc", (void *)W_data<IniFileSection *, H_asec_data>, "ctn");
    C(H_asec_grow,    "Array<IniFileSection*>::Grow",     0x46dd40, 5, "53568b5108", (void *)W_grow<SectionArray, H_asec_grow>, "ctn");
    C(H_asec_dtor,    "Array<IniFileSection*>::~Array",   0x46f350, 9, "8b410cc7016c0d4a00", (void *)W_dtor<SectionArray, H_asec_dtor>, "ctn");
    C(H_afile_get,     "Array<IniFileCache*>::Get",       0x46e090, 7, "8b490c8b442404", (void *)W_get<IniFileCache *, H_afile_get>, "ctn");
    C(H_afile_insert,  "Array<IniFileCache*>::Insert",    0x46e900, 5, "53568b0157", (void *)W_insert<FileArray, IniFileCache *, H_afile_insert>, "ctn");
    C(H_afile_remove,  "Array<IniFileCache*>::RemoveAt",  0x46e970, 8, "568b41048b742408", (void *)W_removeAt<FileArray, H_afile_remove>, "ctn");
    C(H_afile_replace, "Array<IniFileCache*>::Replace",   0x46e9b0, 7, "8b4424088b490c", (void *)W_replace<FileArray, IniFileCache *, H_afile_replace>, "ctn");
    C(H_afile_data,    "Array<IniFileCache*>::Data",      0x46e9d0, 5, "8b410cc3cc", (void *)W_data<IniFileCache *, H_afile_data>, "ctn");
    C(H_afile_grow,    "Array<IniFileCache*>::Grow",      0x46e9e0, 5, "53568b5108", (void *)W_grow<FileArray, H_afile_grow>, "ctn");
    C(H_afile_dtor,    "Array<IniFileCache*>::~Array",    0x46f3d0, 9, "8b410cc7011c0e4a00", (void *)W_dtor<FileArray, H_afile_dtor>, "ctn");
    // PrimitiveArray<String,N> destructors
    C(H_pa3_dtor,  "PrimitiveArray<String,3>::~",  0x40be40, 6, "64a100000000", (void *)W_dtor<StringList, H_pa3_dtor>, "ctn");
    C(H_pa23_dtor, "PrimitiveArray<String,23>::~", 0x46f4e0, 6, "64a100000000", (void *)W_dtor<PrimitiveArray<String, 23>, H_pa23_dtor>, "ctn");
    C(H_pa50_dtor, "PrimitiveArray<String,50>::~", 0x46f760, 6, "64a100000000", (void *)W_dtor<PrimitiveArray<String, 50>, H_pa50_dtor>, "ctn");
    C(H_pa51_dtor, "PrimitiveArray<String,51>::~", 0x46f7b0, 6, "64a100000000", (void *)W_dtor<PrimitiveArray<String, 51>, H_pa51_dtor>, "ctn");
    // Set<String,23> (section values), 50 (sections), 51 (files)
    C(H_s23_insert,    "Set<String,23>::Insert",    0x462380, 6, "64a100000000", (void *)W_insert<Set<String, 23>, String, H_s23_insert>, "ctn");
    C(H_s23_replace,   "Set<String,23>::Replace",   0x4627e0, 6, "64a100000000", (void *)W_replace<Set<String, 23>, String, H_s23_replace>, "ctn");
    C(H_s23_contains,  "Set<String,23>::Contains",  0x4626f0, 5, "8b44240453", (void *)W_contains<Set<String, 23>, String, H_s23_contains>, "ctn");
    C(H_s23_indexof,   "Set<String,23>::IndexOf",   0x462710, 5, "53568b7104", (void *)W_indexOf<Set<String, 23>, String, H_s23_indexof>, "ctn");
    C(H_s23_removekey, "Set<String,23>::RemoveKey", 0x4627c0, 5, "8b44240456", (void *)W_removeKey<Set<String, 23>, String, H_s23_removekey>, "ctn");
    C(H_s23_dtor,      "Set<String,23>::~Set",      0x46eeb0, 6, "64a100000000", (void *)W_dtor<Set<String, 23>, H_s23_dtor>, "ctn");
    C(H_s50_insert,    "Set<String,50>::Insert",    0x46d640, 6, "64a100000000", (void *)W_insert<Set<String, 50>, String, H_s50_insert>, "ctn");
    C(H_s50_replace,   "Set<String,50>::Replace",   0x46daa0, 6, "64a100000000", (void *)W_replace<Set<String, 50>, String, H_s50_replace>, "ctn");
    C(H_s50_contains,  "Set<String,50>::Contains",  0x46d9b0, 5, "8b44240453", (void *)W_contains<Set<String, 50>, String, H_s50_contains>, "ctn");
    C(H_s50_indexof,   "Set<String,50>::IndexOf",   0x46d9d0, 5, "53568b7104", (void *)W_indexOf<Set<String, 50>, String, H_s50_indexof>, "ctn");
    C(H_s50_removekey, "Set<String,50>::RemoveKey", 0x46da80, 5, "8b44240456", (void *)W_removeKey<Set<String, 50>, String, H_s50_removekey>, "ctn");
    C(H_s50_dtor,      "Set<String,50>::~Set",      0x46f2f0, 6, "64a100000000", (void *)W_dtor<Set<String, 50>, H_s50_dtor>, "ctn");
    C(H_s51_insert,    "Set<String,51>::Insert",    0x46e2d0, 6, "64a100000000", (void *)W_insert<Set<String, 51>, String, H_s51_insert>, "ctn");
    C(H_s51_replace,   "Set<String,51>::Replace",   0x46e740, 6, "64a100000000", (void *)W_replace<Set<String, 51>, String, H_s51_replace>, "ctn");
    C(H_s51_contains,  "Set<String,51>::Contains",  0x46e650, 5, "8b44240453", (void *)W_contains<Set<String, 51>, String, H_s51_contains>, "ctn");
    C(H_s51_indexof,   "Set<String,51>::IndexOf",   0x46e670, 5, "53568b5904", (void *)W_indexOf<Set<String, 51>, String, H_s51_indexof>, "ctn");
    C(H_s51_removekey, "Set<String,51>::RemoveKey", 0x46e720, 5, "8b44240456", (void *)W_removeKey<Set<String, 51>, String, H_s51_removekey>, "ctn");
    C(H_s51_dtor,      "Set<String,51>::~Set",      0x46f370, 6, "64a100000000", (void *)W_dtor<Set<String, 51>, H_s51_dtor>, "ctn");
    // Table<String,String,..,23> (section values) and its BufferObjectTable
    C(H_t23_ctor,      "Table<..23>::Table",        0x4629a0, 6, "64a100000000", (void *)W_ctor<ValueTable, H_t23_ctor>, "ctn");
    C(H_t23_dtor,      "Table<..23>::~Table",       0x462b50, 6, "64a100000000", (void *)W_dtor<ValueTable, H_t23_dtor>, "ctn");
    C(H_t23_insert,    "Table<..23>::Insert",       0x462bc0, 6, "53568b74240c", (void *)W_tinsert<ValueTable, String, String, H_t23_insert>, "ctn");
    C(H_t23_contains,  "Table<..23>::Contains",     0x462c20, 8, "5383c1048b442408", (void *)W_contains<ValueTable, String, H_t23_contains>, "ctn");
    C(H_t23_indexof,   "Table<..23>::IndexOf",      0x462c40, 8, "5383c1048b442408", (void *)W_indexOf<ValueTable, String, H_t23_indexof>, "ctn");
    C(H_t23_removeat,  "Table<..23>::RemoveAt",     0x462c60, 8, "56578bf98b74240c", (void *)W_removeAt<ValueTable, H_t23_removeat>, "ctn");
    C(H_t23_removekey, "Table<..23>::RemoveKey",    0x462c90, 5, "8b44240453", (void *)W_removeKey<ValueTable, String, H_t23_removekey>, "ctn");
    C(H_t23_get,       "Table<..23>::Get",          0x462cc0, 6, "64a100000000", (void *)W_tgetStr<ValueTable, H_t23_get>, "ctn");
    C(H_t23_getat,     "Table<..23>::GetAt",        0x40f2a0, 6, "64a100000000", (void *)W_tgetAtStr<ValueTable, H_t23_getat>, "ctn");
    C(H_bot23_dtor,    "BufferObjectTable<..23>::~", 0x40c3d0, 6, "64a100000000", (void *)W_dtor<IniValueTable, H_bot23_dtor>, "ctn");
    // Table<String,IniFileSection*,..,50> (a file's sections)
    C(H_t50_ctor,      "Table<..50>::Table",        0x46d2c0, 6, "64a100000000", (void *)W_ctor<SectionTable, H_t50_ctor>, "ctn");
    C(H_t50_dtor,      "Table<..50>::~Table",       0x46ddc0, 6, "64a100000000", (void *)W_dtor<SectionTable, H_t50_dtor>, "ctn");
    C(H_t50_insert,    "Table<..50>::Insert",       0x46de30, 7, "535657558b7104", (void *)W_tinsert<SectionTable, String, IniFileSection *, H_t50_insert>, "ctn");
    C(H_t50_contains,  "Table<..50>::Contains",     0x46de90, 8, "5383c1048b442408", (void *)W_contains<SectionTable, String, H_t50_contains>, "ctn");
    C(H_t50_indexof,   "Table<..50>::IndexOf",      0x46deb0, 8, "5383c1048b442408", (void *)W_indexOf<SectionTable, String, H_t50_indexof>, "ctn");
    C(H_t50_removeat,  "Table<..50>::RemoveAt",     0x46ded0, 8, "56578bf98b74240c", (void *)W_removeAt<SectionTable, H_t50_removeat>, "ctn");
    C(H_t50_removekey, "Table<..50>::RemoveKey",    0x46df00, 5, "8b44240453", (void *)W_removeKey<SectionTable, String, H_t50_removekey>, "ctn");
    C(H_t50_get,       "Table<..50>::Get",          0x46df30, 5, "8b44240453", (void *)W_tget<SectionTable, String, IniFileSection *, H_t50_get>, "ctn");
    // Table<String,IniFileCache*,..,51> (the global cache; its constructor runs before the hooks are in)
    C(H_t51_dtor,      "Table<..51>::~Table",       0x46ea60, 6, "64a100000000", (void *)W_dtor<IniFileTable, H_t51_dtor>, "ctn");
    C(H_t51_insert,    "Table<..51>::Insert",       0x46ead0, 6, "53568b74240c", (void *)W_tinsert<IniFileTable, String, IniFileCache *, H_t51_insert>, "ctn");
    C(H_t51_contains,  "Table<..51>::Contains",     0x46eb30, 8, "5383c1048b442408", (void *)W_contains<IniFileTable, String, H_t51_contains>, "ctn");
    C(H_t51_indexof,   "Table<..51>::IndexOf",      0x46eb50, 8, "5383c1048b442408", (void *)W_indexOf<IniFileTable, String, H_t51_indexof>, "ctn");
    C(H_t51_removeat,  "Table<..51>::RemoveAt",     0x46eb70, 8, "56578bf98b74240c", (void *)W_removeAt<IniFileTable, H_t51_removeat>, "ctn");
    C(H_t51_removekey, "Table<..51>::RemoveKey",    0x46eba0, 5, "8b44240453", (void *)W_removeKey<IniFileTable, String, H_t51_removekey>, "ctn");
    C(H_t51_get,       "Table<..51>::Get",          0x46ebd0, 5, "8b44240453", (void *)W_tget<IniFileTable, String, IniFileCache *, H_t51_get>, "ctn");
    // the parsed ini files
    C(H_ip_skip,     "IniSkipBlanks",              0x4426d0, 7, "8b442404803800", (void *)h_ip_skip, "inip");
    C(H_ip_back,     "IniBackOverBlanks",          0x4426f0, 8, "8b4424088b4c2404", (void *)h_ip_back, "inip");
    C(H_ip_blank,    "IniFindBlank",               0x442710, 7, "8b442404803800", (void *)h_ip_blank, "inip");
    C(H_ip_add,      "IniFileSection::Add",        0x442730, 6, "64a100000000", (void *)h_ip_add, "inip");
    C(H_ip_secfind,  "IniFileSection::Find",       0x4428a0, 6, "64a100000000", (void *)h_ip_secfind, "inip");
    C(H_ip_seckeys,  "IniFileSection::Keys",       0x442980, 5, "8d4130c3cc", (void *)h_ip_seckeys, "inip");
    C(H_ip_filedtor, "IniFileCache::~IniFileCache", 0x4429e0, 6, "64a100000000", (void *)h_ip_filedtor, "inip");
    C(H_ip_load,     "IniFileCache::Load",         0x442aa0, 6, "64a100000000", (void *)h_ip_load, "inip");
    C(H_ip_filekeys, "IniFileCache::Keys",         0x442e20, 6, "64a100000000", (void *)h_ip_filekeys, "inip");
    C(H_ip_find,     "IniFileCache::Find",         0x442ee0, 6, "64a100000000", (void *)h_ip_find, "inip");
    C(H_ip_open,     "IniOpen",                    0x442fa0, 6, "64a100000000", (void *)h_ip_open, "inip");
    C(H_ip_closeall, "IniCloseAll",                0x4430e0, 8, "56578b3d40774c00", (void *)h_ip_closeall, "inip");
    C(H_ip_intlist,  "IniReader::GetIntList",      0x443350, 5, "8b44240456", (void *)h_ip_intlist, "inip");
    C(H_ip_keys,     "IniReader::GetKeys",         0x443470, 6, "64a100000000", (void *)h_ip_keys, "inip");
    C(H_ip_parseints, "IniReader::ParseIntList",   0x4435f0, 6, "64a100000000", (void *)h_ip_parseints, "inip");
}

// ---- Splash (the load screen; replace only, "splash") ---------------------------------------------------------------------
static Hook H_spctor, H_spdtor, H_spload, H_spfile, H_spshow, H_sphide, H_spshown, H_sppump, H_spflush, H_spstep, H_spprog, H_spreset;

static void *(THISCALL *o_spctor)(Splash *);
static void *THISCALL h_spctor(Splash *s) { HEAVY_LOG(H_spctor); new (s) Splash(); return s; }
static void (__fastcall *o_spdtor)(Splash *);
static void __fastcall h_spdtor(Splash *s) { HEAVY_LOG(H_spdtor); s->~Splash(); }
static int (THISCALL *o_spload)(Splash *, int);
static int THISCALL h_spload(Splash *s, int w) { HEAVY_LOG(H_spload); return s->Load(w); }
static void (THISCALL *o_spfile)(Splash *, int);
static void THISCALL h_spfile(Splash *s, int w) { HEAVY_LOG(H_spfile); s->SetFile(w); }
static void (__fastcall *o_spshow)(Splash *);
static void __fastcall h_spshow(Splash *s) { HEAVY_LOG(H_spshow); s->Show(); }
static void (__fastcall *o_sphide)(Splash *);
static void __fastcall h_sphide(Splash *s) { HEAVY_LOG(H_sphide); s->Hide(); }
static int (__fastcall *o_spshown)(Splash *);
static int __fastcall h_spshown(Splash *s) { HEAVY_LOG(H_spshown); return s->IsShown(); }
static void (__fastcall *o_sppump)(Splash *);
static void __fastcall h_sppump(Splash *s) { HEAVY_LOG(H_sppump); s->Pump(); }
static void (__fastcall *o_spflush)(Splash *);
static void __fastcall h_spflush(Splash *s) { HEAVY_LOG(H_spflush); s->Flush(); }
static void (__fastcall *o_spstep)(Splash *);
static void __fastcall h_spstep(Splash *s) { HEAVY_LOG(H_spstep); s->Step(); }
static void (THISCALL *o_spprog)(Splash *, float);
static void THISCALL h_spprog(Splash *s, float p) { HEAVY_LOG(H_spprog); s->SetProgress(p); }
static void (__fastcall *o_spreset)(Splash *);
static void __fastcall h_spreset(Splash *s) { HEAVY_LOG(H_spreset); s->Reset(); }

// ---- Menu (the main menu; replace only, "menu") -----------------------------------------------------------------------------
static Hook H_mnctor, H_mndtor, H_mnload, H_mnacts, H_mnbmp, H_mnanchor, H_mnlayout, H_mndraw, H_mnsel, H_mnmove, H_mnhit, H_mnmsg,
            H_mnstart, H_mnloadgame, H_mnoptions, H_mnhelp, H_mncredits, H_mnquit;

static void *(THISCALL *o_mnctor)(Menu *);
static void *THISCALL h_mnctor(Menu *m) { HEAVY_LOG(H_mnctor); new (m) Menu(); return m; }
static void (__fastcall *o_mndtor)(Menu *);
static void __fastcall h_mndtor(Menu *m) { HEAVY_LOG(H_mndtor); m->~Menu(); }
static int (THISCALL *o_mnload)(Menu *, int, int);
static int THISCALL h_mnload(Menu *m, int a, int b) { HEAVY_LOG(H_mnload); return m->Load(a, b); }
static void (__fastcall *o_mnacts)(Menu *);
static void __fastcall h_mnacts(Menu *m) { HEAVY_LOG(H_mnacts); m->SetActions(); }
static int (THISCALL *o_mnbmp)(Menu *, const char *, Bitmap *);
static int THISCALL h_mnbmp(Menu *m, const char *f, Bitmap *b) { HEAVY_LOG(H_mnbmp); return m->LoadItemBitmap(f, b); }
static void (__fastcall *o_mnanchor)(Menu *);
static void __fastcall h_mnanchor(Menu *m) { HEAVY_LOG(H_mnanchor); m->SaveAnchor(); }
static void (__fastcall *o_mnlayout)(Menu *);
static void __fastcall h_mnlayout(Menu *m) { HEAVY_LOG(H_mnlayout); m->Relayout(); }
static void (THISCALL *o_mndraw)(Menu *, int, int);
static void THISCALL h_mndraw(Menu *m, int i, int lit) { HEAVY_LOG(H_mndraw); m->DrawItem(i, lit); }
static int (THISCALL *o_mnsel)(Menu *, int);
static int THISCALL h_mnsel(Menu *m, int i) { HEAVY_LOG(H_mnsel); return m->SetSelected(i); }
static int (THISCALL *o_mnmove)(Menu *, int, int);
static int THISCALL h_mnmove(Menu *m, int x, int y) { HEAVY_LOG(H_mnmove); return m->OnMouseMove(x, y); }
static int (THISCALL *o_mnhit)(Menu *, int, int, int);
static int THISCALL h_mnhit(Menu *m, int i, int x, int y) { HEAVY_LOG(H_mnhit); return m->HitTest(i, x, y); }
static LRESULT (THISCALL *o_mnmsg)(Menu *, UINT, WPARAM, LPARAM, WPARAM *);
static LRESULT THISCALL h_mnmsg(Menu *m, UINT msg, WPARAM w, LPARAM l, WPARAM *k) { HEAVY_LOG(H_mnmsg); return m->OnMessage(msg, w, l, k); }
static int (__cdecl *o_mnstart)();
static int __cdecl h_mnstart() { HEAVY_LOG(H_mnstart); return Menu::ActionStart(); }
static int (__cdecl *o_mnloadgame)();
static int __cdecl h_mnloadgame() { HEAVY_LOG(H_mnloadgame); return Menu::ActionLoadGame(); }
static int (__cdecl *o_mnoptions)();
static int __cdecl h_mnoptions() { HEAVY_LOG(H_mnoptions); return Menu::ActionOptions(); }
static int (__cdecl *o_mnhelp)();
static int __cdecl h_mnhelp() { HEAVY_LOG(H_mnhelp); return Menu::ActionHelp(); }
static int (__cdecl *o_mncredits)();
static int __cdecl h_mncredits() { HEAVY_LOG(H_mncredits); return Menu::ActionCredits(); }
static int (__cdecl *o_mnquit)();
static int __cdecl h_mnquit() { HEAVY_LOG(H_mnquit); return Menu::ActionQuit(); }

// ---- Dialogs (the options dialog and the dialogs it opens, load / save game; replace only, "dialogs") -------------------------
static Hook H_dgquit, H_dgopt, H_dgjoy, H_dgkeys, H_dgsens, H_dgsave, H_dgname, H_dgkind, H_dgpress,
            H_dgshowjoy, H_dgshowkeys, H_dgshowsens, H_dgcreate, H_dgvalid;

#define DIALOG_HOOK(tag, fn) \
    static INT_PTR (CALLBACK *o_##tag)(HWND, UINT, WPARAM, LPARAM); \
    static INT_PTR CALLBACK h_##tag(HWND d, UINT m, WPARAM w, LPARAM l) { HEAVY_LOG(H_##tag); return Dialogs::fn(d, m, w, l); }
DIALOG_HOOK(dgquit, QuitQuestion)
DIALOG_HOOK(dgopt, Options)
DIALOG_HOOK(dgjoy, Joystick)
DIALOG_HOOK(dgkeys, Keys)
DIALOG_HOOK(dgsens, Sensitivity)
DIALOG_HOOK(dgsave, SaveLoad)
DIALOG_HOOK(dgname, SaveName)
DIALOG_HOOK(dgkind, KeyKind)
DIALOG_HOOK(dgpress, PressKey)

static int (__cdecl *o_dgshowjoy)(HWND);
static int __cdecl h_dgshowjoy(HWND p) { HEAVY_LOG(H_dgshowjoy); return Dialogs::ShowJoystick(p); }
static int (__cdecl *o_dgshowkeys)(HWND);
static int __cdecl h_dgshowkeys(HWND p) { HEAVY_LOG(H_dgshowkeys); return Dialogs::ShowKeys(p); }
static int (__cdecl *o_dgshowsens)(HWND);
static int __cdecl h_dgshowsens(HWND p) { HEAVY_LOG(H_dgshowsens); return Dialogs::ShowSensitivity(p); }
static HANDLE (__cdecl *o_dgcreate)(HWND, const char *);
static HANDLE __cdecl h_dgcreate(HWND o, const char *p) { HEAVY_LOG(H_dgcreate); return Dialogs::CreateSaveFile(o, p); }
static int (__cdecl *o_dgvalid)(const char *);
static int __cdecl h_dgvalid(const char *n) { HEAVY_LOG(H_dgvalid); return Dialogs::IsValidSaveName(n); }

// ---- MoodFace (the animated face in the panel; replace only, "face") --------------------------------------------------------
static Hook H_fcctor, H_fcdtor, H_fcload, H_fcdraw, H_fcfind, H_fcorigin;

static void *(__fastcall *o_fcctor)(MoodFace *);
static void *__fastcall h_fcctor(MoodFace *f) { HEAVY_LOG(H_fcctor); new (f) MoodFace(); return f; }
static void (__fastcall *o_fcdtor)(MoodFace *);
static void __fastcall h_fcdtor(MoodFace *f) { HEAVY_LOG(H_fcdtor); f->~MoodFace(); }
static int (THISCALL *o_fcload)(MoodFace *, int, int);
static int THISCALL h_fcload(MoodFace *f, int x, int y) { HEAVY_LOG(H_fcload); return f->Load(x, y); }
static int (THISCALL *o_fcdraw)(MoodFace *, int, Bitmap *);
static int THISCALL h_fcdraw(MoodFace *f, int force, Bitmap *p) { HEAVY_LOG(H_fcdraw); return f->Draw(force, p); }
static int (THISCALL *o_fcfind)(MoodFace *, int);
static int THISCALL h_fcfind(MoodFace *f, int mood) { HEAVY_LOG(H_fcfind); return f->FindAnimation(mood); }
static void (__stdcall *o_fcorigin)(int *, int *);
static void __stdcall h_fcorigin(int *x, int *y) { HEAVY_LOG(H_fcorigin); GetScreenOrigin(x, y); }

// ---- NutCounter (a picture with numbers stamped on it, in the panel; replace only, "nutc") ----------------------------------
static Hook H_ncctor, H_ncdtor, H_ncinit, H_ncload, H_ncdraw, H_ncnum;

static void *(__fastcall *o_ncctor)(NutCounter *);
static void *__fastcall h_ncctor(NutCounter *n) { HEAVY_LOG(H_ncctor); new (n) NutCounter(); return n; }
static void (__fastcall *o_ncdtor)(NutCounter *);
static void __fastcall h_ncdtor(NutCounter *n) { HEAVY_LOG(H_ncdtor); n->~NutCounter(); }
static int (THISCALL *o_ncinit)(NutCounter *, const char *, int, int, int);
static int THISCALL h_ncinit(NutCounter *n, const char *k, int x, int y, int m) { HEAVY_LOG(H_ncinit); return n->Init(k, x, y, m); }
static int (__fastcall *o_ncload)(NutCounter *);
static int __fastcall h_ncload(NutCounter *n) { HEAVY_LOG(H_ncload); return n->Load(); }
static void (THISCALL *o_ncdraw)(NutCounter *, int, int);
static void THISCALL h_ncdraw(NutCounter *n, int a, int b) { HEAVY_LOG(H_ncdraw); n->Draw(a, b); }
static void (THISCALL *o_ncnum)(NutCounter *, int, int, int);
static void THISCALL h_ncnum(NutCounter *n, int v, int x, int y) { HEAVY_LOG(H_ncnum); n->DrawNumber(v, x, y); }

// ---- Panel (the in-game panel; the parts reconstructed so far; replace only, "panel") -------------------------------------------
static Hook H_pnload, H_pnctor, H_pndtor, H_sbctor, H_sbdtor, H_qpctor, H_qpdtor;
static void *(__fastcall *o_pnctor)(Panel *);
static void *__fastcall h_pnctor(Panel *p) { HEAVY_LOG(H_pnctor); new (p) Panel(); return p; }
static void (__fastcall *o_pndtor)(Panel *);
static void __fastcall h_pndtor(Panel *p) { HEAVY_LOG(H_pndtor); p->~Panel(); }
static void *(__fastcall *o_sbctor)(StatusBar *);
static void *__fastcall h_sbctor(StatusBar *s) { HEAVY_LOG(H_sbctor); new (s) StatusBar(); return s; }
static void (__fastcall *o_sbdtor)(StatusBar *);
static void __fastcall h_sbdtor(StatusBar *s) { HEAVY_LOG(H_sbdtor); s->~StatusBar(); }
static void *(__fastcall *o_qpctor)(QuestPanel *);
static void *__fastcall h_qpctor(QuestPanel *q) { HEAVY_LOG(H_qpctor); new (q) QuestPanel(); return q; }
static void (__fastcall *o_qpdtor)(QuestPanel *);
static void __fastcall h_qpdtor(QuestPanel *q) { HEAVY_LOG(H_qpdtor); q->~QuestPanel(); }
static int (__fastcall *o_pnload)(Panel *);
static int __fastcall h_pnload(Panel *p) { HEAVY_LOG(H_pnload); return p->LoadBitmaps(); }

// ---- StatusBar (nut counters, goodies, mood face and messages in the panel; replace only, "status") ---------------------------
static Hook H_sbinit, H_sbload;
static int (__fastcall *o_sbinit)(StatusBar *);
static int __fastcall h_sbinit(StatusBar *s) { HEAVY_LOG(H_sbinit); return s->Init(); }
static int (__fastcall *o_sbload)(StatusBar *);
static int __fastcall h_sbload(StatusBar *s) { HEAVY_LOG(H_sbload); return s->LoadAll(); }
static Hook H_sbupd, H_sbface, H_sbamo, H_sbfoodb, H_sbfood;
static int (THISCALL *o_sbupd)(StatusBar *, int, int *, Bitmap *);
static int THISCALL h_sbupd(StatusBar *s, int f, int *e, Bitmap *p) { HEAVY_LOG(H_sbupd); return s->Update(f, e, p); }
static int (THISCALL *o_sbface)(StatusBar *, int, Bitmap *);
static int THISCALL h_sbface(StatusBar *s, int f, Bitmap *p) { HEAVY_LOG(H_sbface); return s->UpdateFace(f, p); }
static int (THISCALL *o_sbamo)(StatusBar *, int);
static int THISCALL h_sbamo(StatusBar *s, int f) { HEAVY_LOG(H_sbamo); return s->UpdateAmo(f); }
static void (THISCALL *o_sbfoodb)(StatusBar *, int);
static void THISCALL h_sbfoodgoal(StatusBar *s, int v) { HEAVY_LOG(H_sbfoodb); s->SetFoodGoal(v); }
static int (THISCALL *o_sbfood)(StatusBar *, int);
static int THISCALL h_sbfood(StatusBar *s, int f) { HEAVY_LOG(H_sbfood); return s->UpdateFood(f); }
static Hook H_sbgood, H_sbround, H_pngoodie;
static int (THISCALL *o_sbgood)(StatusBar *, int, int *, Bitmap *);
static int THISCALL h_sbgood(StatusBar *s, int f, int *e, Bitmap *p) { HEAVY_LOG(H_sbgood); return s->UpdateGoodies(f, e, p); }
static void (THISCALL *o_sbround)(StatusBar *, int, float);
static void THISCALL h_sbround(StatusBar *s, int n, float sec) { HEAVY_LOG(H_sbround); s->SetRound(n, sec); }
static void (THISCALL *o_pngoodie)(Panel *, int, float);
static void THISCALL h_pngoodie(Panel *p, int n, float sec) { HEAVY_LOG(H_pngoodie); p->StartGoodieAnimation(n, sec); }
// ---- World (World.h): the level's scene set-up ----
static Hook H_wlctor, H_wldtor, H_wlcreate, H_wldestroy, H_wlnav, H_wllights, H_wlframe;
static void *(__fastcall *o_wlctor)(World *);
static void *__fastcall h_wlctor(World *w) { HEAVY_LOG(H_wlctor); new (w) World(); return w; }
static void (__fastcall *o_wldtor)(World *);
static void __fastcall h_wldtor(World *w) { HEAVY_LOG(H_wldtor); w->~World(); }
static void (THISCALL *o_wlcreate)(World *, IniReader *, IniReader *);
static void THISCALL h_wlcreate(World *w, IniReader *a, IniReader *b) { HEAVY_LOG(H_wlcreate); w->CreateScene(a, b); }
static void (__fastcall *o_wldestroy)(World *);
static void __fastcall h_wldestroy(World *w) { HEAVY_LOG(H_wldestroy); w->DestroyScene(); }
static void (THISCALL *o_wlnav)(World *, IniReader *);
static void THISCALL h_wlnav(World *w, IniReader *r) { HEAVY_LOG(H_wlnav); w->LoadNavMesh(r); }
static void (THISCALL *o_wllights)(World *, IniReader *);
static void THISCALL h_wllights(World *w, IniReader *r) { HEAVY_LOG(H_wllights); w->SetupLights(r); }
static void (__fastcall *o_wlframe)(World *);
static void __fastcall h_wlframe(World *w) { HEAVY_LOG(H_wlframe); w->RenderFrame(); }
static Hook H_rsloadtex, H_rsfindtex, H_rstexres, H_rstexlist, H_rsdefsprite, H_rspoptex, H_rstexlook, H_rsloadgeom, H_rsfindgeom, H_rsgeomres, H_rsgeomlist, H_rspopgeom, H_rsinitgeom, H_rserrlog;
static IDirect3DRMTexture *(__cdecl *o_rsloadtex)(const char *, const char *);
static IDirect3DRMTexture *__cdecl h_rsloadtex(const char *n, const char *p) { HEAVY_LOG(H_rsloadtex); return LoadTexture(n, p); }
static IDirect3DRMTexture *(__cdecl *o_rsfindtex)(const char *);
static IDirect3DRMTexture *__cdecl h_rsfindtex(const char *n) { HEAVY_LOG(H_rsfindtex); return FindTexture(n); }
static void (__cdecl *o_rstexres)(IniReader *);
static void __cdecl h_rstexres(IniReader *i) { HEAVY_LOG(H_rstexres); LoadTextureResources(i); }
static void (__cdecl *o_rstexlist)(IniReader *);
static void __cdecl h_rstexlist(IniReader *i) { HEAVY_LOG(H_rstexlist); ReadTextureList(i); }
static void (__cdecl *o_rsdefsprite)(IniReader *);
static void __cdecl h_rsdefsprite(IniReader *i) { HEAVY_LOG(H_rsdefsprite); ReadDefaultPropSprite(i); }
static void (__cdecl *o_rspoptex)();
static void __cdecl h_rspoptex() { HEAVY_LOG(H_rspoptex); PopTextureTable(); }
static IDirect3DRMTexture *(__cdecl *o_rstexlook)(const char *);
static IDirect3DRMTexture *__cdecl h_rstexlook(const char *n) { HEAVY_LOG(H_rstexlook); return TextureLookup(n); }
static IDirect3DRMFrame *(__cdecl *o_rsloadgeom)(const char *, const char *);
static IDirect3DRMFrame *__cdecl h_rsloadgeom(const char *n, const char *s) { HEAVY_LOG(H_rsloadgeom); return LoadGeometry(n, s); }
static IDirect3DRMFrame *(__cdecl *o_rsfindgeom)(const char *);
static IDirect3DRMFrame *__cdecl h_rsfindgeom(const char *n) { HEAVY_LOG(H_rsfindgeom); return FindGeometry(n); }
static void (__cdecl *o_rsgeomres)(IniReader *);
static void __cdecl h_rsgeomres(IniReader *i) { HEAVY_LOG(H_rsgeomres); LoadGeometryResources(i); }
static void (__cdecl *o_rsgeomlist)(IniReader *);
static void __cdecl h_rsgeomlist(IniReader *i) { HEAVY_LOG(H_rsgeomlist); ReadGeometryList(i); }
static void (__cdecl *o_rspopgeom)();
static void __cdecl h_rspopgeom() { HEAVY_LOG(H_rspopgeom); PopGeometryTable(); }
static void (__cdecl *o_rsinitgeom)();
static void __cdecl h_rsinitgeom() { HEAVY_LOG(H_rsinitgeom); InitGeometryLoading(); }
static void (__cdecl *o_rserrlog)(const char *);
static void __cdecl h_rserrlog(const char *m) { HEAVY_LOG(H_rserrlog); SqgErrorLog(m); }
// ---- Props (Prop.h): the frame-carrying things placed in a level ----
static Hook H_prinit, H_prdestruct, H_prdestroy, H_prinst, H_prshow, H_prisshown, H_prhide, H_prattach;
static Hook H_mpinit, H_mpdestruct, H_mpdestroy, H_mpclone, H_mpbuild;
static Hook H_spinit, H_spdestruct, H_spdestroy, H_spclone, H_spbuild, H_spscale;
static Prop *(THISCALL *o_prinit)(Prop *, Float3, Float3, Float3, Float3);
static Prop *THISCALL h_prinit(Prop *p, Float3 a, Float3 b, Float3 c, Float3 d) { HEAVY_LOG(H_prinit); return p->Init(a, b, c, d); }
static void (__fastcall *o_prdestruct)(Prop *);
static void __fastcall h_prdestruct(Prop *p) { HEAVY_LOG(H_prdestruct); p->Destruct(); }
static Prop *(THISCALL *o_prdestroy)(Prop *, int);
static Prop *THISCALL h_prdestroy(Prop *p, int flags) { HEAVY_LOG(H_prdestroy); return p->Destroy(flags); }
static Prop *(THISCALL *o_prinst)(Prop *, FrameProp *, Float3, Float3, Float3, Float3);
static Prop *THISCALL h_prinst(Prop *p, FrameProp *o, Float3 a, Float3 b, Float3 c, Float3 d) { HEAVY_LOG(H_prinst); return p->Instantiate(o, a, b, c, d); }
static IDirect3DRMFrame *(THISCALL *o_prshow)(Prop *, IDirect3DRMFrame *);
static IDirect3DRMFrame *THISCALL h_prshow(Prop *p, IDirect3DRMFrame *parent) { HEAVY_LOG(H_prshow); return p->Show(parent); }
static int (__fastcall *o_prisshown)(Prop *);
static int __fastcall h_prisshown(Prop *p) { HEAVY_LOG(H_prisshown); return p->IsShown(); }
static void (__fastcall *o_prhide)(Prop *);
static void __fastcall h_prhide(Prop *p) { HEAVY_LOG(H_prhide); p->Hide(); }
static IDirect3DRMFrame *(__fastcall *o_prattach)(Prop *);
static IDirect3DRMFrame *__fastcall h_prattach(Prop *p) { HEAVY_LOG(H_prattach); return p->GetAttachFrame(); }
static MeshProp *(THISCALL *o_mpinit)(MeshProp *, const char *, Float3, Float3, Float3, Float3);
static MeshProp *THISCALL h_mpinit(MeshProp *p, const char *n, Float3 a, Float3 b, Float3 c, Float3 d) { HEAVY_LOG(H_mpinit); return p->Init(n, a, b, c, d); }
static void (__fastcall *o_mpdestruct)(MeshProp *);
static void __fastcall h_mpdestruct(MeshProp *p) { HEAVY_LOG(H_mpdestruct); p->Destruct(); }
static Prop *(THISCALL *o_mpdestroy)(MeshProp *, int);
static Prop *THISCALL h_mpdestroy(MeshProp *p, int flags) { HEAVY_LOG(H_mpdestroy); return p->Destroy(flags); }
static Prop *(THISCALL *o_mpclone)(MeshProp *, FrameProp *);
static Prop *THISCALL h_mpclone(MeshProp *p, FrameProp *o) { HEAVY_LOG(H_mpclone); return p->Clone(o); }
static IDirect3DRMFrame *(THISCALL *o_mpbuild)(MeshProp *, IDirect3DRMFrame *);
static IDirect3DRMFrame *THISCALL h_mpbuild(MeshProp *p, IDirect3DRMFrame *parent) { HEAVY_LOG(H_mpbuild); return p->BuildFrame(parent); }
static SpriteProp *(THISCALL *o_spinit)(SpriteProp *, const char *, Float3, Float3, const DecalSettings *);
static SpriteProp *THISCALL h_spinit(SpriteProp *p, const char *n, Float3 a, Float3 b, const DecalSettings *d) { HEAVY_LOG(H_spinit); return p->Init(n, a, b, d); }
static void (__fastcall *o_spdestruct)(SpriteProp *);
static void __fastcall h_spdestruct(SpriteProp *p) { HEAVY_LOG(H_spdestruct); p->Destruct(); }
static Prop *(THISCALL *o_spdestroy)(SpriteProp *, int);
static Prop *THISCALL h_spdestroy(SpriteProp *p, int flags) { HEAVY_LOG(H_spdestroy); return p->Destroy(flags); }
static Prop *(THISCALL *o_spclone)(SpriteProp *, FrameProp *);
static Prop *THISCALL h_spclone(SpriteProp *p, FrameProp *o) { HEAVY_LOG(H_spclone); return p->Clone(o); }
static IDirect3DRMFrame *(THISCALL *o_spbuild)(SpriteProp *, IDirect3DRMFrame *);
static IDirect3DRMFrame *THISCALL h_spbuild(SpriteProp *p, IDirect3DRMFrame *parent) { HEAVY_LOG(H_spbuild); return p->BuildFrame(parent); }
static void (THISCALL *o_spscale)(SpriteProp *, IDirect3DRMFrame *, Float3);
static void THISCALL h_spscale(SpriteProp *p, IDirect3DRMFrame *f, Float3 s) { HEAVY_LOG(H_spscale); p->ApplyScale(f, s); }
// ---- FrameProp (FrameProp.h): groups of child props, with an optional animated body ----
static Hook H_fpinit, H_fpdestruct, H_fpdestroy, H_fpclone, H_fpbuild, H_fphide, H_fpown, H_fppart, H_fpupdate, H_fpfire;
static Hook H_lpinit, H_lpdestruct, H_lpdestroy, H_lpclone, H_lpbuild, H_lplevel, H_lpselect;
static LODProp *(THISCALL *o_lpinit)(LODProp *, Float3, Float3, Float3, Float3, StringList *, MorphCycle *, CycleMessageTable *, WeightList *);
static LODProp *THISCALL h_lpinit(LODProp *p, Float3 a, Float3 b, Float3 c, Float3 d, StringList *n, MorphCycle *m, CycleMessageTable *t, WeightList *w)
{
    HEAVY_LOG(H_lpinit);
    return p->Init(a, b, c, d, n, m, t, w);
}
static void (__fastcall *o_lpdestruct)(LODProp *);
static void __fastcall h_lpdestruct(LODProp *p) { HEAVY_LOG(H_lpdestruct); p->Destruct(); }
static Prop *(THISCALL *o_lpdestroy)(LODProp *, int);
static Prop *THISCALL h_lpdestroy(LODProp *p, int flags) { HEAVY_LOG(H_lpdestroy); return p->Destroy(flags); }
static Prop *(THISCALL *o_lpclone)(LODProp *, FrameProp *);
static Prop *THISCALL h_lpclone(LODProp *p, FrameProp *o) { HEAVY_LOG(H_lpclone); return p->Clone(o); }
static IDirect3DRMFrame *(THISCALL *o_lpbuild)(LODProp *, IDirect3DRMFrame *);
static IDirect3DRMFrame *THISCALL h_lpbuild(LODProp *p, IDirect3DRMFrame *parent) { HEAVY_LOG(H_lpbuild); return p->BuildFrame(parent); }
static void (__fastcall *o_lplevel)(LODProp *);
static void __fastcall h_lplevel(LODProp *p) { HEAVY_LOG(H_lplevel); p->UpdateLevel(); }
static int (THISCALL *o_lpselect)(LODProp *, Float3, Float3);
static int THISCALL h_lpselect(LODProp *p, Float3 pos, Float3 fwd) { HEAVY_LOG(H_lpselect); return p->SelectLevel(pos, fwd); }
static Hook H_fptime, H_fpdir, H_fploop, H_fpmatrix, H_fpactor, H_fpfollow, H_fphideend, H_fpfind;
static FrameProp *(THISCALL *o_fpinit)(FrameProp *, Float3, Float3, Float3, Float3, StringList *, MorphCycle *, CycleMessageTable *);
static FrameProp *THISCALL h_fpinit(FrameProp *p, Float3 a, Float3 b, Float3 c, Float3 d, StringList *n, MorphCycle *m, CycleMessageTable *t)
{
    HEAVY_LOG(H_fpinit);
    return p->Init(a, b, c, d, n, m, t);
}
static void (__fastcall *o_fpdestruct)(FrameProp *);
static void __fastcall h_fpdestruct(FrameProp *p) { HEAVY_LOG(H_fpdestruct); p->Destruct(); }
static Prop *(THISCALL *o_fpdestroy)(FrameProp *, int);
static Prop *THISCALL h_fpdestroy(FrameProp *p, int flags) { HEAVY_LOG(H_fpdestroy); return p->Destroy(flags); }
static Prop *(THISCALL *o_fpclone)(FrameProp *, FrameProp *);
static Prop *THISCALL h_fpclone(FrameProp *p, FrameProp *o) { HEAVY_LOG(H_fpclone); return p->Clone(o); }
static IDirect3DRMFrame *(THISCALL *o_fpbuild)(FrameProp *, IDirect3DRMFrame *);
static IDirect3DRMFrame *THISCALL h_fpbuild(FrameProp *p, IDirect3DRMFrame *parent) { HEAVY_LOG(H_fpbuild); return p->BuildFrame(parent); }
static void (__fastcall *o_fphide)(FrameProp *);
static void __fastcall h_fphide(FrameProp *p) { HEAVY_LOG(H_fphide); p->Hide(); }
static void (THISCALL *o_fpown)(FrameProp *, IDirect3DRMFrame *);
static void THISCALL h_fpown(FrameProp *p, IDirect3DRMFrame *parent) { HEAVY_LOG(H_fpown); p->BuildOwnFrame(parent); }
static IDirect3DRMFrame *(__fastcall *o_fppart)(FrameProp *);
static IDirect3DRMFrame *__fastcall h_fppart(FrameProp *p) { HEAVY_LOG(H_fppart); return p->GetPartFrame(); }
static void (THISCALL *o_fpupdate)(FrameProp *, float);
static void THISCALL h_fpupdate(FrameProp *p, float seconds) { HEAVY_LOG(H_fpupdate); p->Update(seconds); }
static void (THISCALL *o_fpfire)(FrameProp *, float, float);
static void THISCALL h_fpfire(FrameProp *p, float from, float to) { HEAVY_LOG(H_fpfire); p->FireMessages(from, to); }
static void (THISCALL *o_fptime)(FrameProp *, float);
static void THISCALL h_fptime(FrameProp *p, float t) { HEAVY_LOG(H_fptime); p->SetCycleTime(t); }
static void (THISCALL *o_fpdir)(FrameProp *, int);
static void THISCALL h_fpdir(FrameProp *p, int d) { HEAVY_LOG(H_fpdir); p->SetCycleDirection(d); }
static void (THISCALL *o_fploop)(FrameProp *, int);
static void THISCALL h_fploop(FrameProp *p, int m) { HEAVY_LOG(H_fploop); p->SetCycleLoopMode(m); }
// slot 18 returns a matrix by value: MSVC passes the caller's result space as the first stack argument and returns it
static float *(THISCALL *o_fpmatrix)(FrameProp *, float *);
static float *THISCALL h_fpmatrix(FrameProp *p, float *result) { HEAVY_LOG(H_fpmatrix); return p->GetPartMatrix(result); }
static void (THISCALL *o_fpactor)(FrameProp *, int);
static void THISCALL h_fpactor(FrameProp *p, int i) { HEAVY_LOG(H_fpactor); p->SetActor(i); }
static void (THISCALL *o_fpfollow)(FrameProp *, int);
static void THISCALL h_fpfollow(FrameProp *p, int m) { HEAVY_LOG(H_fpfollow); p->SetFollowMode(m); }
static void (THISCALL *o_fphideend)(FrameProp *, int);
static void THISCALL h_fphideend(FrameProp *p, int h) { HEAVY_LOG(H_fphideend); p->SetHideAtEnd(h); }
// slot 13 takes a String by value (its rep pointer) and releases it
static Prop *(THISCALL *o_fpfind)(FrameProp *, StringRep *);
static Prop *THISCALL h_fpfind(FrameProp *p, StringRep *rep)
{
    HEAVY_LOG(H_fpfind);
    String name;
    name.rep = rep;
    return p->FindChild(name);
}
// ---- Animation (Animation.h): poses, tracks, clips ----
// animcmp (with the "anim" hooks): run the original and compare our version on a copy of the same input. The poses the game
// keeps are the original's; ours only checks.
static bool g_animCompare;
static void CompareFloats(Hook &h, const float *real, const float *ours, int n, const char *what)
{
    h.calls++;
    for (int i = 0; i < n; i++) {
        float a = real[i], b = ours[i];
        float scale = fabsf(a) > fabsf(b) ? fabsf(a) : fabsf(b);
        if (fabsf(a - b) <= 1e-5f * (scale > 1.0f ? scale : 1.0f)) continue;
        h.mismatches++;
        if (h.mismatches <= 8) Log("MISMATCH %s %s[%d]: orig=%g ours=%g", h.name, what, i, a, b);
        return;
    }
}
static void ComparePoses(Hook &h, const PartPose *real, const PartPose *ours)
{
    CompareFloats(h, &real->position.x, &ours->position.x, 9, "vectors");
    if (real->weights.count != ours->weights.count) { h.mismatches++; Log("MISMATCH %s weight count orig=%d ours=%d", h.name, real->weights.count, ours->weights.count); return; }
    if (real->weights.count > 0) CompareFloats(h, real->weights.data, ours->weights.data, real->weights.count, "weights");
}
static Hook H_pzctor, H_pzassign, H_pzmatrix, H_pcctor, H_pcdtor, H_pcfind, H_pcsample, H_pcnew, H_pcblend;
static Hook H_ptctor, H_ptcopy, H_ptdtor, H_ptfind, H_ptat, H_ptassign;
static Hook H_mcctor, H_mcrange, H_mcdtor, H_mcnew, H_mcsample, H_mcblend;
static PartPose *(THISCALL *o_pzctor)(PartPose *, const Vector3f *, const Vector3f *, const Vector3f *, const WeightList *);
static PartPose *THISCALL h_pzctor(PartPose *p, const Vector3f *a, const Vector3f *b, const Vector3f *c, const WeightList *w) { HEAVY_LOG(H_pzctor); return ::new (p) PartPose(*a, *b, *c, w); }
static void (THISCALL *o_pzassign)(PartPose *, const PartPose *);
static void THISCALL h_pzassign(PartPose *p, const PartPose *o) { HEAVY_LOG(H_pzassign); p->Assign(*o); }
static float *(THISCALL *o_pzmatrix)(PartPose *, float *);
static float *THISCALL h_pzmatrix(PartPose *p, float *m)
{
    if (g_mode == MODE_REPLACE || !g_animCompare) { HEAVY_LOG(H_pzmatrix); p->ToMatrix(m); return m; }
    float mine[16];
    o_pzmatrix(p, m);
    p->ToMatrix(mine);
    CompareFloats(H_pzmatrix, m, mine, 16, "matrix");
    return m;
}
static PartCycle *(THISCALL *o_pcctor)(PartCycle *, int, const Vector3f *, int, const PartOrientation *, int, WeightList **, MorphCycle *);
static PartCycle *THISCALL h_pcctor(PartCycle *p, int pn, const Vector3f *pd, int on, const PartOrientation *od, int mn, WeightList **md, MorphCycle *o)
{ HEAVY_LOG(H_pcctor); return ::new (p) PartCycle(pn, pd, on, od, mn, md, o); }
static void (__fastcall *o_pcdtor)(PartCycle *);
static void __fastcall h_pcdtor(PartCycle *p) { HEAVY_LOG(H_pcdtor); p->~PartCycle(); }
static int (THISCALL *o_pcfind)(PartCycle *, float, int *);
static int THISCALL h_pcfind(PartCycle *p, float t, int *hint)
{
    if (g_mode == MODE_REPLACE || !g_animCompare) { HEAVY_LOG(H_pcfind); return p->FindSegment(t, hint); }
    int mineHint = *hint;
    int real = o_pcfind(p, t, hint);
    int mine = p->FindSegment(t, &mineHint);
    H_pcfind.calls++;
    if (real != mine || *hint != mineHint) {
        H_pcfind.mismatches++;
        if (H_pcfind.mismatches <= 8) Log("MISMATCH %s orig=%d (hint %d) ours=%d (hint %d)", H_pcfind.name, real, *hint, mine, mineHint);
    }
    return real;
}
static int (THISCALL *o_pcsample)(PartCycle *, PartPose *, float, int);
static int THISCALL h_pcsample(PartCycle *p, PartPose *out, float t, int hint)
{
    if (g_mode == MODE_REPLACE || !g_animCompare) { HEAVY_LOG(H_pcsample); return p->Sample(out, t, hint); }
    PartPose scratch(*out);
    int real = o_pcsample(p, out, t, hint);
    int mine = p->Sample(&scratch, t, hint);
    ComparePoses(H_pcsample, out, &scratch);
    if (real != mine) { H_pcsample.mismatches++; if (H_pcsample.mismatches <= 8) Log("MISMATCH %s hint orig=%d ours=%d", H_pcsample.name, real, mine); }
    return real;
}
static PartPose *(THISCALL *o_pcnew)(PartCycle *, float, int);
static PartPose *THISCALL h_pcnew(PartCycle *p, float t, int hint) { HEAVY_LOG(H_pcnew); return p->NewPose(t, hint); }
static int (THISCALL *o_pcblend)(PartCycle *, PartPose *, float, float, int);
static int THISCALL h_pcblend(PartCycle *p, PartPose *pose, float t, float w, int hint)
{
    if (g_mode == MODE_REPLACE || !g_animCompare) { HEAVY_LOG(H_pcblend); return p->Blend(pose, t, w, hint); }
    PartPose scratch(*pose);
    int real = o_pcblend(p, pose, t, w, hint);
    int mine = p->Blend(&scratch, t, w, hint);
    ComparePoses(H_pcblend, pose, &scratch);
    if (real != mine) { H_pcblend.mismatches++; if (H_pcblend.mismatches <= 8) Log("MISMATCH %s hint orig=%d ours=%d", H_pcblend.name, real, mine); }
    return real;
}
static PoseTable *(__fastcall *o_ptctor)(PoseTable *);
static PoseTable *__fastcall h_ptctor(PoseTable *t) { HEAVY_LOG(H_ptctor); return ::new (t) PoseTable(); }
static PoseTable *(THISCALL *o_ptcopy)(PoseTable *, const PoseTable *);
static PoseTable *THISCALL h_ptcopy(PoseTable *t, const PoseTable *o) { HEAVY_LOG(H_ptcopy); return ::new (t) PoseTable(*o); }
static void (__fastcall *o_ptdtor)(PoseTable *);
static void __fastcall h_ptdtor(PoseTable *t) { HEAVY_LOG(H_ptdtor); t->~PoseTable(); }
static PartPose *(THISCALL *o_ptfind)(PoseTable *, const char *);
static PartPose *THISCALL h_ptfind(PoseTable *t, const char *n) { HEAVY_LOG(H_ptfind); return t->Find(n); }
static PartPose *(THISCALL *o_ptat)(PoseTable *, int);
static PartPose *THISCALL h_ptat(PoseTable *t, int i) { HEAVY_LOG(H_ptat); return t->At(i); }
static void (THISCALL *o_ptassign)(PoseTable *, const PoseTable *);
static void THISCALL h_ptassign(PoseTable *t, const PoseTable *o) { HEAVY_LOG(H_ptassign); t->Assign(*o); }
static MorphCycle *(THISCALL *o_mcctor)(MorphCycle *, int, float, float);
static MorphCycle *THISCALL h_mcctor(MorphCycle *c, int noLoop, float speed, float period) { HEAVY_LOG(H_mcctor); return ::new (c) MorphCycle(noLoop, speed, period); }
static MorphCycle *(THISCALL *o_mcrange)(MorphCycle *, const MorphCycle *, int, int, int, float, float);
static MorphCycle *THISCALL h_mcrange(MorphCycle *c, const MorphCycle *whole, int first, int last, int noLoop, float speed, float period)
{ HEAVY_LOG(H_mcrange); return ::new (c) MorphCycle(*whole, first, last, noLoop, speed, period); }
static void (__fastcall *o_mcdtor)(MorphCycle *);
static void __fastcall h_mcdtor(MorphCycle *c) { HEAVY_LOG(H_mcdtor); c->~MorphCycle(); }
static PoseTable *(__fastcall *o_mcnew)(MorphCycle *);
static PoseTable *__fastcall h_mcnew(MorphCycle *c) { HEAVY_LOG(H_mcnew); return c->NewPoseTable(); }
static int (THISCALL *o_mcsample)(MorphCycle *, PoseTable *, float, int);
static int THISCALL h_mcsample(MorphCycle *c, PoseTable *pose, float t, int hint) { HEAVY_LOG(H_mcsample); return c->SamplePose(pose, t, hint); }
static int (THISCALL *o_mcblend)(MorphCycle *, PoseTable *, float, float, int);
static int THISCALL h_mcblend(MorphCycle *c, PoseTable *pose, float t, float w, int hint) { HEAVY_LOG(H_mcblend); return c->BlendPose(pose, t, w, hint); }
// ---- BodyPart and AnimatedBody (BodyPart.h) ----
static Hook H_bpinitm, H_bpinit0, H_bpinitc, H_bpdtor;
static Hook H_abinit, H_abcopy, H_abdtor, H_abadd, H_abcycle, H_abapply, H_abadv, H_abdir, H_abloop, H_abtime, H_abblend;
static BodyPart *(THISCALL *o_bpinitm)(BodyPart *, SqgModel *);
static BodyPart *THISCALL h_bpinitm(BodyPart *b, SqgModel *m) { HEAVY_LOG(H_bpinitm); return b->Init(m); }
static BodyPart *(__fastcall *o_bpinit0)(BodyPart *);
static BodyPart *__fastcall h_bpinit0(BodyPart *b) { HEAVY_LOG(H_bpinit0); return b->Init(); }
static BodyPart *(THISCALL *o_bpinitc)(BodyPart *, const BodyPart *);
static BodyPart *THISCALL h_bpinitc(BodyPart *b, const BodyPart *o) { HEAVY_LOG(H_bpinitc); return b->Init(o); }
static void (__fastcall *o_bpdtor)(BodyPart *);
static void __fastcall h_bpdtor(BodyPart *b) { HEAVY_LOG(H_bpdtor); b->Destruct(); }
static AnimatedBody *(__fastcall *o_abinit)(AnimatedBody *);
static AnimatedBody *__fastcall h_abinit(AnimatedBody *a) { HEAVY_LOG(H_abinit); return a->Init(); }
static AnimatedBody *(THISCALL *o_abcopy)(AnimatedBody *, const AnimatedBody *);
static AnimatedBody *THISCALL h_abcopy(AnimatedBody *a, const AnimatedBody *o) { HEAVY_LOG(H_abcopy); return a->InitCopy(o); }
static void (__fastcall *o_abdtor)(AnimatedBody *);
static void __fastcall h_abdtor(AnimatedBody *a) { HEAVY_LOG(H_abdtor); a->Destruct(); }
static int (THISCALL *o_abadd)(AnimatedBody *, const char *, BodyPart *);
static int THISCALL h_abadd(AnimatedBody *a, const char *n, BodyPart *p) { HEAVY_LOG(H_abadd); return a->AddPart(n, p); }
static void (THISCALL *o_abcycle)(AnimatedBody *, MorphCycle *, int);
static void THISCALL h_abcycle(AnimatedBody *a, MorphCycle *c, int blend) { HEAVY_LOG(H_abcycle); a->SetCycle(c, blend); }
static void (__fastcall *o_abapply)(AnimatedBody *);
static void __fastcall h_abapply(AnimatedBody *a) { HEAVY_LOG(H_abapply); a->ApplyPose(); }
static void (THISCALL *o_abadv)(AnimatedBody *, float, int);
static void THISCALL h_abadv(AnimatedBody *a, float dt, int skip) { HEAVY_LOG(H_abadv); a->Advance(dt, skip); }
static void (THISCALL *o_abdir)(AnimatedBody *, int);
static void THISCALL h_abdir(AnimatedBody *a, int d) { HEAVY_LOG(H_abdir); a->SetDirection(d); }
static void (THISCALL *o_abloop)(AnimatedBody *, int);
static void THISCALL h_abloop(AnimatedBody *a, int m) { HEAVY_LOG(H_abloop); a->SetLoopMode(m); }
static void (THISCALL *o_abtime)(AnimatedBody *, float);
static void THISCALL h_abtime(AnimatedBody *a, float t) { HEAVY_LOG(H_abtime); a->SetTime(t); }
static float (__fastcall *o_abblend)(AnimatedBody *);
static float __fastcall h_abblend(AnimatedBody *a) { HEAVY_LOG(H_abblend); return a->GetBlendFraction(); }
static Hook H_scpost;
static int (__cdecl *o_scpost)(const char *, void *, int, int);
static int __cdecl h_scpost(const char *n, void *a, int x, int l) { HEAVY_LOG(H_scpost); return PostFunction(n, a, x, l); }
static Hook H_texdef;
static void (__cdecl *o_texdef)();
static void __cdecl h_texdef() { HEAVY_LOG(H_texdef); InitTextureDefaults(); }
static Hook H_wldecset, H_wldecal, H_wldecsize;
static DecalSettings *(THISCALL *o_wldecset)(World *, DecalSettings *, IniReader *, const char *, const char *, DecalSettings);
static DecalSettings *THISCALL h_wldecset(World *w, DecalSettings *out, IniReader *r, const char *section, const char *prefix, DecalSettings defaults)
{ HEAVY_LOG(H_wldecset); return w->ReadDecalSettings(out, r, section, prefix, defaults); }
static IDirect3DRMFrame *(THISCALL *o_wldecal)(World *, IDirect3DRMTexture *, Float3, DecalSettings, IDirect3DRMFrame *);
static IDirect3DRMFrame *THISCALL h_wldecal(World *w, IDirect3DRMTexture *t, Float3 p, DecalSettings s, IDirect3DRMFrame *parent)
{ HEAVY_LOG(H_wldecal); return w->CreateDecal(t, p, s, parent); }
static IDirect3DRMFrame *(THISCALL *o_wldecsize)(World *, IDirect3DRMFrame *, float, float);
static IDirect3DRMFrame *THISCALL h_wldecsize(World *w, IDirect3DRMFrame *f, float width, float height)
{ HEAVY_LOG(H_wldecsize); return w->SetDecalSize(f, width, height); }
static Hook H_pnld, H_pnsq, H_pnss, H_pninv, H_pnupd, H_pnbg, H_pnmsg, H_pnnote, H_pncomp, H_rtext, H_qifree;
static int (__fastcall *o_pnld)(Panel *);
static int __fastcall h_pnld(Panel *p) { HEAVY_LOG(H_pnld); return p->Load(); }
static void (THISCALL *o_pnsq)(Panel *, int);
static void THISCALL h_pnsq(Panel *p, int k) { HEAVY_LOG(H_pnsq); p->ShowQuest(k); }
static void (THISCALL *o_pnss)(Panel *, int);
static void THISCALL h_pnss(Panel *p, int v) { HEAVY_LOG(H_pnss); p->ShowStatus(v); }
static void (__fastcall *o_pninv)(Panel *);
static void __fastcall h_pninv(Panel *p) { HEAVY_LOG(H_pninv); p->Invalidate(); }
static int (THISCALL *o_pnupd)(Panel *, int);
static int THISCALL h_pnupd(Panel *p, int f) { HEAVY_LOG(H_pnupd); return p->Update(f); }
static int (__fastcall *o_pnbg)(Panel *);
static int __fastcall h_pnbg(Panel *p) { HEAVY_LOG(H_pnbg); return p->DrawBackground(); }
static int (THISCALL *o_pnmsg)(Panel *, const char *);
static int THISCALL h_pnmsg(Panel *p, const char *t) { HEAVY_LOG(H_pnmsg); return p->DrawMessage(t); }
static int (THISCALL *o_pnnote)(Panel *, const char *);
static int THISCALL h_pnnote(Panel *p, const char *t) { HEAVY_LOG(H_pnnote); return p->DrawNote(t); }
static void (THISCALL *o_pncomp)(Panel *, int);
static void THISCALL h_pncomp(Panel *p, int f) { HEAVY_LOG(H_pncomp); p->UpdateCompass(f); }
static void (THISCALL *o_rtext)(Renderer *, HFONT, int, int, const char *);
static void THISCALL h_rtext(Renderer *r, HFONT f, int x, int y, const char *t) { HEAVY_LOG(H_rtext); r->PutText(f, x, y, t); }
static void (__fastcall *o_qifree)(QuestItems *);
static void __fastcall h_qifree(QuestItems *k) { HEAVY_LOG(H_qifree); k->FreeCombined(); }
static Hook H_pnspr;
static int (__fastcall *o_pnspr)(Panel *);
static int __fastcall h_pnspr(Panel *p) { HEAVY_LOG(H_pnspr); return p->UpdateRegionSprites(); }
static Hook H_qpload, H_qpdraw, H_qidraw;
static int (__fastcall *o_qpload)(QuestPanel *);
static int __fastcall h_qpload(QuestPanel *q) { HEAVY_LOG(H_qpload); return q->Load(); }
static int (THISCALL *o_qpdraw)(QuestPanel *, int, int);
static int THISCALL h_qpdraw(QuestPanel *q, int f, int k) { HEAVY_LOG(H_qpdraw); return q->Draw(f, k); }
static void (THISCALL *o_qidraw)(QuestItems *, int);
static void THISCALL h_qidraw(QuestItems *k, int place) { HEAVY_LOG(H_qidraw); k->Draw(place); }

// ---- GoodieList (the goodie pictures in the status bar; replace only, "goodies") ----------------------------------------------
static Hook H_glctor, H_gldtor, H_glinit, H_glload, H_gldraw;
static void *(__fastcall *o_glctor)(GoodieList *);
static void *__fastcall h_glctor(GoodieList *g) { HEAVY_LOG(H_glctor); new (g) GoodieList(); return g; }
static void (__fastcall *o_gldtor)(GoodieList *);
static void __fastcall h_gldtor(GoodieList *g) { HEAVY_LOG(H_gldtor); g->~GoodieList(); }
static int (THISCALL *o_glinit)(GoodieList *, int, int);
static int THISCALL h_glinit(GoodieList *g, int x, int y) { HEAVY_LOG(H_glinit); return g->Init(x, y); }
static int (__fastcall *o_glload)(GoodieList *);
static int __fastcall h_glload(GoodieList *g) { HEAVY_LOG(H_glload); return g->Load(); }
// the name is a String passed by value (4 bytes); the callee releases it
static void (THISCALL *o_gldraw)(GoodieList *, int, StringRep *, Bitmap *);
static void THISCALL h_gldraw(GoodieList *g, int frame, StringRep *rep, Bitmap *panel)
{
    HEAVY_LOG(H_gldraw);
    String name;
    name.rep = rep;
    g->Draw(frame, name, panel);
}

// ---- QuestItems (the collectable key pieces and the stone they form; replace only, "quest") --------------------------------------------------
static Hook H_qictor, H_qidtor, H_qiinit, H_qicomb, H_qikeys;
static void *(__fastcall *o_qictor)(QuestItems *);
static void *__fastcall h_qictor(QuestItems *k) { HEAVY_LOG(H_qictor); new (k) QuestItems(); return k; }
static void (__fastcall *o_qidtor)(QuestItems *);
static void __fastcall h_qidtor(QuestItems *k) { HEAVY_LOG(H_qidtor); k->~QuestItems(); }
static int (THISCALL *o_qiinit)(QuestItems *, int, int, int);
static int THISCALL h_qiinit(QuestItems *k, int a, int b, int i) { HEAVY_LOG(H_qiinit); return k->Init(a, b, i); }
static int (THISCALL *o_qicomb)(QuestItems *, int);
static int THISCALL h_qicomb(QuestItems *k, int i) { HEAVY_LOG(H_qicomb); return k->LoadCombined(i); }
static int (__fastcall *o_qikeys)(QuestItems *);
static int __fastcall h_qikeys(QuestItems *k) { HEAVY_LOG(H_qikeys); return k->LoadKeys(); }

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
                           &H_inictor, &H_inictor2, &H_inidtor, &H_iniget, &H_iniint, &H_inifloats, &H_inifloat, &H_iniset, &H_inidir, &H_inipath, &H_iniparse,
                           &H_spctor, &H_spdtor, &H_spload, &H_spfile, &H_spshow, &H_sphide, &H_spshown, &H_sppump, &H_spflush, &H_spstep, &H_spprog, &H_spreset,
                           &H_mnctor, &H_mndtor, &H_mnload, &H_mnacts, &H_mnbmp, &H_mnanchor, &H_mnlayout, &H_mndraw, &H_mnsel, &H_mnmove, &H_mnhit, &H_mnmsg,
                           &H_mnstart, &H_mnloadgame, &H_mnoptions, &H_mnhelp, &H_mncredits, &H_mnquit,
                           &H_dgquit, &H_dgopt, &H_dgjoy, &H_dgkeys, &H_dgsens, &H_dgsave, &H_dgname, &H_dgkind, &H_dgpress,
                           &H_dgshowjoy, &H_dgshowkeys, &H_dgshowsens, &H_dgcreate, &H_dgvalid,
                           &H_fcctor, &H_fcdtor, &H_fcload, &H_fcdraw, &H_fcfind, &H_fcorigin,
                           &H_ncctor, &H_ncdtor, &H_ncinit, &H_ncload, &H_ncdraw, &H_ncnum,
                           &H_pnload, &H_pnctor, &H_pndtor, &H_sbctor, &H_sbdtor, &H_qpctor, &H_qpdtor,
                           &H_sbinit, &H_sbload, &H_sbupd, &H_sbface, &H_sbamo, &H_sbfoodb, &H_sbfood, &H_sbgood, &H_sbround, &H_pngoodie,
                           &H_glctor, &H_gldtor, &H_glinit, &H_glload, &H_gldraw,
                           &H_pnld, &H_pnsq, &H_pnss, &H_pninv, &H_pnupd, &H_pnbg, &H_pnmsg, &H_pnnote, &H_pncomp, &H_rtext, &H_qifree, &H_pnspr, &H_qpload, &H_qpdraw, &H_qidraw,
                           &H_qictor, &H_qidtor, &H_qiinit, &H_qicomb, &H_qikeys,
                           &H_wlctor, &H_wldtor, &H_wlcreate, &H_wldestroy, &H_wlnav, &H_wllights, &H_wlframe,
                           &H_wldecset, &H_wldecal, &H_wldecsize, &H_texdef,
                           &H_rsloadtex, &H_rsfindtex, &H_rstexres, &H_rstexlist, &H_rsdefsprite, &H_rspoptex, &H_rstexlook,
                           &H_rsloadgeom, &H_rsfindgeom, &H_rsgeomres, &H_rsgeomlist, &H_rspopgeom, &H_rsinitgeom, &H_rserrlog,
                           &H_prinit, &H_prdestruct, &H_prdestroy, &H_prinst, &H_prshow, &H_prisshown, &H_prhide, &H_prattach,
                           &H_mpinit, &H_mpdestruct, &H_mpdestroy, &H_mpclone, &H_mpbuild,
                           &H_spinit, &H_spdestruct, &H_spdestroy, &H_spclone, &H_spbuild, &H_spscale,
                           &H_fpinit, &H_fpdestruct, &H_fpdestroy, &H_fpclone, &H_fpbuild, &H_fphide, &H_fpown, &H_fppart, &H_fpupdate, &H_fpfire,
                           &H_lpinit, &H_lpdestruct, &H_lpdestroy, &H_lpclone, &H_lpbuild, &H_lplevel, &H_lpselect,
                           &H_fptime, &H_fpdir, &H_fploop, &H_fpmatrix, &H_fpactor, &H_fpfollow, &H_fphideend, &H_fpfind,
                           &H_pzctor, &H_pzassign, &H_pzmatrix, &H_pcctor, &H_pcdtor, &H_pcfind, &H_pcsample, &H_pcnew, &H_pcblend,
                           &H_ptctor, &H_ptcopy, &H_ptdtor, &H_ptfind, &H_ptat, &H_ptassign,
                           &H_mcctor, &H_mcrange, &H_mcdtor, &H_mcnew, &H_mcsample, &H_mcblend,
                           &H_bpinitm, &H_bpinit0, &H_bpinitc, &H_bpdtor,
                           &H_abinit, &H_abcopy, &H_abdtor, &H_abadd, &H_abcycle, &H_abapply, &H_abadv, &H_abdir, &H_abloop, &H_abtime, &H_abblend , &H_scpost };
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
    H_spctor = { "Splash::Splash", 0x44e900, 7, "64a10000000055", (void *)h_spctor, (void **)&o_spctor, false, 0, 0, "splash", 0 };
    H_spdtor = { "Splash::~Splash", 0x44e9e0, 5, "56578b7104", (void *)h_spdtor, (void **)&o_spdtor, false, 0, 0, "splash", 0 };
    H_spload = { "Splash::Load", 0x44ea30, 7, "64a10000000055", (void *)h_spload, (void **)&o_spload, false, 0, 0, "splash", 0 };
    H_spfile = { "Splash::SetFile", 0x44eb20, 7, "64a10000000055", (void *)h_spfile, (void **)&o_spfile, false, 0, 0, "splash", 0 };
    H_spshow = { "Splash::Show", 0x44edd0, 5, "83ec0833c0", (void *)h_spshow, (void **)&o_spshow, false, 0, 0, "splash", 0 };
    H_sphide = { "Splash::Hide", 0x44ee40, 5, "56578b4108", (void *)h_sphide, (void **)&o_sphide, false, 0, 0, "splash", 0 };
    H_spshown = { "Splash::IsShown", 0x44eea0, 5, "8b411cc3cc", (void *)h_spshown, (void **)&o_spshown, false, 0, 0, "splash", 0 };
    H_sppump = { "Splash::Pump", 0x44eeb0, 8, "83ec1ca1dc754c00", (void *)h_sppump, (void **)&o_sppump, false, 0, 0, "splash", 0 };
    H_spflush = { "Splash::Flush", 0x44ef00, 6, "5356578b590c", (void *)h_spflush, (void **)&o_spflush, false, 0, 0, "splash", 0 };
    H_spstep = { "Splash::Step", 0x44ef30, 5, "83ec0c5356", (void *)h_spstep, (void **)&o_spstep, false, 0, 0, "splash", 0 };
    H_spprog = { "Splash::SetProgress", 0x44f010, 5, "83ec045657", (void *)h_spprog, (void **)&o_spprog, false, 0, 0, "splash", 0 };
    H_spreset = { "Splash::Reset", 0x44f080, 7, "c7410c00000000", (void *)h_spreset, (void **)&o_spreset, false, 0, 0, "splash", 0 };
    H_mnctor = { "Menu::Menu", 0x405e90, 7, "64a10000000055", (void *)h_mnctor, (void **)&o_mnctor, false, 0, 0, "menu", 0 };
    H_mndtor = { "Menu::~Menu", 0x405f90, 5, "53568b4104", (void *)h_mndtor, (void **)&o_mndtor, false, 0, 0, "menu", 0 };
    H_mnload = { "Menu::Load", 0x405ff0, 7, "64a10000000055", (void *)h_mnload, (void **)&o_mnload, false, 0, 0, "menu", 0 };
    H_mnacts = { "Menu::SetActions", 0x406600, 6, "5333c0394104", (void *)h_mnacts, (void **)&o_mnacts, false, 0, 0, "menu", 0 };
    H_mnbmp = { "Menu::LoadItemBitmap", 0x406aa0, 6, "81ecc8000000", (void *)h_mnbmp, (void **)&o_mnbmp, false, 0, 0, "menu", 0 };
    H_mnanchor = { "Menu::SaveAnchor", 0x406b80, 6, "8b41088b510c", (void *)h_mnanchor, (void **)&o_mnanchor, false, 0, 0, "menu", 0 };
    H_mnlayout = { "Menu::Relayout", 0x406ba0, 5, "83ec045356", (void *)h_mnlayout, (void **)&o_mnlayout, false, 0, 0, "menu", 0 };
    H_mndraw = { "Menu::DrawItem", 0x406c20, 8, "83ec08a1dc754c00", (void *)h_mndraw, (void **)&o_mndraw, false, 0, 0, "menu", 0 };
    H_mnsel = { "Menu::SetSelected", 0x406a10, 7, "535657558b4104", (void *)h_mnsel, (void **)&o_mnsel, false, 0, 0, "menu", 0 };
    H_mnmove = { "Menu::OnMouseMove", 0x406960, 6, "83ec04535657", (void *)h_mnmove, (void **)&o_mnmove, false, 0, 0, "menu", 0 };
    H_mnhit = { "Menu::HitTest", 0x406a60, 8, "8b44240453c1e002", (void *)h_mnhit, (void **)&o_mnhit, false, 0, 0, "menu", 0 };
    H_mnmsg = { "Menu::OnMessage", 0x406420, 7, "83ec0c8b442410", (void *)h_mnmsg, (void **)&o_mnmsg, false, 0, 0, "menu", 0 };
    H_mnstart = { "Menu::ActionStart", 0x406950, 5, "b802000000", (void *)h_mnstart, (void **)&o_mnstart, false, 0, 0, "menu", 0 };
    H_mnloadgame = { "Menu::ActionLoadGame", 0x406760, 6, "81ecc8000000", (void *)h_mnloadgame, (void **)&o_mnloadgame, false, 0, 0, "menu", 0 };
    H_mnoptions = { "Menu::ActionOptions", 0x406730, 6, "53a1cc734c00", (void *)h_mnoptions, (void **)&o_mnoptions, false, 0, 0, "menu", 0 };
    H_mnhelp = { "Menu::ActionHelp", 0x406670, 6, "53a1dc754c00", (void *)h_mnhelp, (void **)&o_mnhelp, false, 0, 0, "menu", 0 };
    H_mncredits = { "Menu::ActionCredits", 0x406690, 6, "53a1dc754c00", (void *)h_mncredits, (void **)&o_mncredits, false, 0, 0, "menu", 0 };
    H_mnquit = { "Menu::ActionQuit", 0x406920, 6, "53a1cc734c00", (void *)h_mnquit, (void **)&o_mnquit, false, 0, 0, "menu", 0 };
    H_dgquit = { "Dialogs::QuitQuestion", 0x406d60, 5, "8b44240853", (void *)h_dgquit, (void **)&o_dgquit, false, 0, 0, "dialogs", 0 };
    H_dgopt = { "Dialogs::Options", 0x406cb0, 5, "8b44240853", (void *)h_dgopt, (void **)&o_dgopt, false, 0, 0, "dialogs", 0 };
    H_dgjoy = { "Dialogs::Joystick", 0x4078e0, 5, "8b44240853", (void *)h_dgjoy, (void **)&o_dgjoy, false, 0, 0, "dialogs", 0 };
    H_dgkeys = { "Dialogs::Keys", 0x407d40, 6, "81ecd4000000", (void *)h_dgkeys, (void **)&o_dgkeys, false, 0, 0, "dialogs", 0 };
    H_dgsens = { "Dialogs::Sensitivity", 0x408410, 5, "83ec0c5356", (void *)h_dgsens, (void **)&o_dgsens, false, 0, 0, "dialogs", 0 };
    H_dgsave = { "Dialogs::SaveLoad", 0x406eb0, 6, "81ecac030000", (void *)h_dgsave, (void **)&o_dgsave, false, 0, 0, "dialogs", 0 };
    H_dgname = { "Dialogs::SaveName", 0x4077c0, 5, "8b44240853", (void *)h_dgname, (void **)&o_dgname, false, 0, 0, "dialogs", 0 };
    H_dgkind = { "Dialogs::KeyKind", 0x407c90, 5, "8b44240856", (void *)h_dgkind, (void **)&o_dgkind, false, 0, 0, "dialogs", 0 };
    H_dgpress = { "Dialogs::PressKey", 0x407b90, 5, "8b44240853", (void *)h_dgpress, (void **)&o_dgpress, false, 0, 0, "dialogs", 0 };
    H_dgshowjoy = { "Dialogs::ShowJoystick", 0x4066b0, 6, "53568b74240c", (void *)h_dgshowjoy, (void **)&o_dgshowjoy, false, 0, 0, "dialogs", 0 };
    H_dgshowkeys = { "Dialogs::ShowKeys", 0x4066f0, 6, "53568b74240c", (void *)h_dgshowkeys, (void **)&o_dgshowkeys, false, 0, 0, "dialogs", 0 };
    H_dgshowsens = { "Dialogs::ShowSensitivity", 0x4068e0, 6, "53568b74240c", (void *)h_dgshowsens, (void **)&o_dgshowsens, false, 0, 0, "dialogs", 0 };
    H_dgcreate = { "Dialogs::CreateSaveFile", 0x406dc0, 11, "83ec0cc74424000c000000", (void *)h_dgcreate, (void **)&o_dgcreate, false, 0, 0, "dialogs", 0 };
    H_dgvalid = { "Dialogs::IsValidSaveName", 0x407730, 6, "53b9ffffffff", (void *)h_dgvalid, (void **)&o_dgvalid, false, 0, 0, "dialogs", 0 };
    H_fcctor = { "MoodFace::MoodFace", 0x405430, 6, "53ba0a000000", (void *)h_fcctor, (void **)&o_fcctor, false, 0, 0, "face", 0 };
    H_fcdtor = { "MoodFace::~MoodFace", 0x405490, 7, "83ec08894c2400", (void *)h_fcdtor, (void **)&o_fcdtor, false, 0, 0, "face", 0 };
    H_fcload = { "MoodFace::Load", 0x405520, 7, "64a10000000055", (void *)h_fcload, (void **)&o_fcload, false, 0, 0, "face", 0 };
    H_fcdraw = { "MoodFace::Draw", 0x4059e0, 7, "64a10000000055", (void *)h_fcdraw, (void **)&o_fcdraw, false, 0, 0, "face", 0 };
    H_fcfind = { "MoodFace::FindAnimation", 0x405d10, 7, "8b44240483ec14", (void *)h_fcfind, (void **)&o_fcfind, false, 0, 0, "face", 0 };
    H_fcorigin = { "GetScreenOrigin", 0x42da30, 9, "83ec108b0ddc754c00", (void *)h_fcorigin, (void **)&o_fcorigin, false, 0, 0, "face", 0 };
    H_ncctor = { "NutCounter::NutCounter", 0x42f780, 8, "8bc1c70100000000", (void *)h_ncctor, (void **)&o_ncctor, false, 0, 0, "nutc", 0 };
    H_ncdtor = { "NutCounter::~NutCounter", 0x42f790, 6, "56578b318bf9", (void *)h_ncdtor, (void **)&o_ncdtor, false, 0, 0, "nutc", 0 };
    H_ncinit = { "NutCounter::Init", 0x42f7d0, 7, "64a10000000055", (void *)h_ncinit, (void **)&o_ncinit, false, 0, 0, "nutc", 0 };
    H_ncload = { "NutCounter::Load", 0x42f900, 6, "81ecc8000000", (void *)h_ncload, (void **)&o_ncload, false, 0, 0, "nutc", 0 };
    H_ncdraw = { "NutCounter::Draw", 0x42f9f0, 5, "83ec085356", (void *)h_ncdraw, (void **)&o_ncdraw, false, 0, 0, "nutc", 0 };
    H_ncnum = { "NutCounter::DrawNumber", 0x42fb10, 7, "83ec04894c2400", (void *)h_ncnum, (void **)&o_ncnum, false, 0, 0, "nutc", 0 };
    H_pnload = { "Panel::LoadBitmaps", 0x42e500, 7, "64a10000000055", (void *)h_pnload, (void **)&o_pnload, false, 0, 0, "panel", 0 };
    H_sbinit = { "StatusBar::Init", 0x42ec10, 7, "64a10000000055", (void *)h_sbinit, (void **)&o_sbinit, false, 0, 0, "status", 0 };
    H_sbload = { "StatusBar::LoadAll", 0x42f2c0, 7, "64a10000000055", (void *)h_sbload, (void **)&o_sbload, false, 0, 0, "status", 0 };
    H_glctor = { "GoodieList::GoodieList", 0x4156c0, 5, "33c0894120", (void *)h_glctor, (void **)&o_glctor, false, 0, 0, "goodies", 0 };
    H_gldtor = { "GoodieList::~GoodieList", 0x4156d0, 5, "53568b4120", (void *)h_gldtor, (void **)&o_gldtor, false, 0, 0, "goodies", 0 };
    H_glinit = { "GoodieList::Init", 0x415750, 7, "64a10000000055", (void *)h_glinit, (void **)&o_glinit, false, 0, 0, "goodies", 0 };
    H_glload = { "GoodieList::Load", 0x415b40, 6, "81ec30010000", (void *)h_glload, (void **)&o_glload, false, 0, 0, "goodies", 0 };
    H_gldraw = { "GoodieList::Draw", 0x415d50, 7, "64a10000000055", (void *)h_gldraw, (void **)&o_gldraw, false, 0, 0, "goodies", 0 };
    H_sbupd = { "StatusBar::Update", 0x42ed70, 6, "83ec08535657", (void *)h_sbupd, (void **)&o_sbupd, false, 0, 0, "status", 0 };
    H_sbface = { "StatusBar::UpdateFace", 0x42ee90, 5, "8b44240856", (void *)h_sbface, (void **)&o_sbface, false, 0, 0, "status", 0 };
    H_sbamo = { "StatusBar::UpdateAmo", 0x42eec0, 5, "566a008bf1", (void *)h_sbamo, (void **)&o_sbamo, false, 0, 0, "status", 0 };
    H_sbfoodb = { "StatusBar::SetFoodGoal", 0x42ef40, 7, "8b442404894164", (void *)h_sbfoodgoal, (void **)&o_sbfoodb, false, 0, 0, "status", 0 };
    H_sbfood = { "StatusBar::UpdateFood", 0x42ef50, 7, "64a10000000055", (void *)h_sbfood, (void **)&o_sbfood, false, 0, 0, "status", 0 };
    H_sbgood = { "StatusBar::UpdateGoodies", 0x42f050, 7, "64a10000000055", (void *)h_sbgood, (void **)&o_sbgood, false, 0, 0, "status", 0 };
    H_sbround = { "StatusBar::SetRound", 0x42f200, 10, "d9442408d80d3cde4900", (void *)h_sbround, (void **)&o_sbround, false, 0, 0, "status", 0 };
    H_pngoodie = { "Panel::StartGoodieAnimation", 0x42e6f0, 8, "8b4424088b542404", (void *)h_pngoodie, (void **)&o_pngoodie, false, 0, 0, "panel", 0 };
    H_pnld = { "Panel::Load", 0x42d9f0, 8, "568bf1e8080b0000", (void *)h_pnld, (void **)&o_pnld, false, 0, 0, "panel", 4 };
    H_pnsq = { "Panel::ShowQuest", 0x42da60, 6, "53568b74240c", (void *)h_pnsq, (void **)&o_pnsq, false, 0, 0, "panel", 0 };
    H_pnss = { "Panel::ShowStatus", 0x42dac0, 5, "8b44240456", (void *)h_pnss, (void **)&o_pnss, false, 0, 0, "panel", 0 };
    H_pninv = { "Panel::Invalidate", 0x42db00, 10, "c7815c04000001000000", (void *)h_pninv, (void **)&o_pninv, false, 0, 0, "panel", 0 };
    H_pnupd = { "Panel::Update", 0x42db10, 6, "53a1dc754c00", (void *)h_pnupd, (void **)&o_pnupd, false, 0, 0, "panel", 0 };
    H_pnbg = { "Panel::DrawBackground", 0x42dc50, 8, "83ec10a1dc754c00", (void *)h_pnbg, (void **)&o_pnbg, false, 0, 0, "panel", 0 };
    H_pnmsg = { "Panel::DrawMessage", 0x42dd30, 6, "81ecd8010000", (void *)h_pnmsg, (void **)&o_pnmsg, false, 0, 0, "panel", 0 };
    H_pnnote = { "Panel::DrawNote", 0x42e0e0, 7, "83ec388d442434", (void *)h_pnnote, (void **)&o_pnnote, false, 0, 0, "panel", 0 };
    H_pncomp = { "Panel::UpdateCompass", 0x42e2b0, 6, "83ec0c535657", (void *)h_pncomp, (void **)&o_pncomp, false, 0, 0, "panel", 0 };
    H_rtext = { "Renderer::PutText", 0x4753d0, 5, "83ec1c5356", (void *)h_rtext, (void **)&o_rtext, false, 0, 0, "panel", 0 };
    H_qifree = { "QuestItems::FreeCombined", 0x430340, 5, "568b3185f6", (void *)h_qifree, (void **)&o_qifree, false, 0, 0, "quest", 0 };
    H_pnspr = { "Panel::UpdateRegionSprites", 0x42e2a0, 6, "81c180040000", (void *)h_pnspr, (void **)&o_pnspr, false, 0, 0, "panel", 0 };
    H_qpload = { "QuestPanel::Load", 0x42f660, 5, "566a018bf1", (void *)h_qpload, (void **)&o_qpload, false, 0, 0, "quest", 0 };
    H_qpdraw = { "QuestPanel::Draw", 0x42f690, 8, "56578bf98b74240c", (void *)h_qpdraw, (void **)&o_qpdraw, false, 0, 0, "quest", 0 };
    H_qidraw = { "QuestItems::Draw", 0x430440, 7, "83ec108d442404", (void *)h_qidraw, (void **)&o_qidraw, false, 0, 0, "quest", 0 };
    H_qictor = { "QuestItems::QuestItems", 0x42fc30, 7, "33c08901894118", (void *)h_qictor, (void **)&o_qictor, false, 0, 0, "quest", 0 };
    H_qidtor = { "QuestItems::~QuestItems", 0x42fc40, 9, "56578bf9e8f7060000", (void *)h_qidtor, (void **)&o_qidtor, false, 0, 0, "quest", 5 };
    H_qiinit = { "QuestItems::Init", 0x430150, 7, "64a10000000055", (void *)h_qiinit, (void **)&o_qiinit, false, 0, 0, "quest", 0 };
    H_qicomb = { "QuestItems::LoadCombined", 0x430360, 6, "81ecc8000000", (void *)h_qicomb, (void **)&o_qicomb, false, 0, 0, "quest", 0 };
    H_qikeys = { "QuestItems::LoadKeys", 0x42fca0, 7, "64a10000000055", (void *)h_qikeys, (void **)&o_qikeys, false, 0, 0, "quest", 0 };
    H_pnctor = { "Panel::Panel", 0x42d5f0, 7, "64a10000000055", (void *)h_pnctor, (void **)&o_pnctor, false, 0, 0, "panel", 0 };
    H_pndtor = { "Panel::~Panel", 0x42d880, 7, "64a10000000055", (void *)h_pndtor, (void **)&o_pndtor, false, 0, 0, "panel", 0 };
    H_sbctor = { "StatusBar::StatusBar", 0x42ea70, 7, "64a10000000055", (void *)h_sbctor, (void **)&o_sbctor, false, 0, 0, "status", 0 };
    H_sbdtor = { "StatusBar::~StatusBar", 0x42eb40, 7, "64a10000000055", (void *)h_sbdtor, (void **)&o_sbdtor, false, 0, 0, "status", 0 };
    H_qpctor = { "QuestPanel::QuestPanel", 0x42f580, 7, "64a10000000055", (void *)h_qpctor, (void **)&o_qpctor, false, 0, 0, "quest", 0 };
    H_qpdtor = { "QuestPanel::~QuestPanel", 0x42f5f0, 7, "64a10000000055", (void *)h_qpdtor, (void **)&o_qpdtor, false, 0, 0, "quest", 0 };
    H_wlctor = { "World::World", 0x4357b0, 7, "64a10000000055", (void *)h_wlctor, (void **)&o_wlctor, false, 0, 0, "world", 0 };
    H_wldtor = { "World::~World", 0x4358e0, 6, "56a108734c00", (void *)h_wldtor, (void **)&o_wldtor, false, 0, 0, "world", 0 };
    H_wlcreate = { "World::CreateScene", 0x435910, 7, "64a10000000055", (void *)h_wlcreate, (void **)&o_wlcreate, false, 0, 0, "world", 0 };
    H_wldestroy = { "World::DestroyScene", 0x435b30, 5, "53568b4108", (void *)h_wldestroy, (void **)&o_wldestroy, false, 0, 0, "world", 0 };
    H_wlnav = { "World::LoadNavMesh", 0x4363a0, 7, "64a10000000055", (void *)h_wlnav, (void **)&o_wlnav, false, 0, 0, "world", 0 };
    H_wllights = { "World::SetupLights", 0x436040, 7, "64a10000000055", (void *)h_wllights, (void **)&o_wllights, false, 0, 0, "world", 0 };
    H_wlframe = { "World::RenderFrame", 0x4362a0, 6, "56a124734c00", (void *)h_wlframe, (void **)&o_wlframe, false, 0, 0, "world", 0 };
    H_wldecset = { "World::ReadDecalSettings", 0x435c90, 7, "64a10000000055", (void *)h_wldecset, (void **)&o_wldecset, false, 0, 0, "world", 0 };
    H_wldecal = { "World::CreateDecal", 0x435bb0, 7, "64a10000000055", (void *)h_wldecal, (void **)&o_wldecal, false, 0, 0, "world", 0 };
    H_wldecsize = { "World::SetDecalSize", 0x435c70, 8, "8b44240c8b4c2408", (void *)h_wldecsize, (void **)&o_wldecsize, false, 0, 0, "world", 0 };
    H_texdef = { "InitTextureDefaults", 0x424e20, 7, "64a10000000055", (void *)h_texdef, (void **)&o_texdef, false, 0, 0, "world", 0 };
    H_rsloadtex = { "LoadTexture", 0x425070, 7, "64a10000000055", (void *)h_rsloadtex, (void **)&o_rsloadtex, false, 0, 0, "resources", 0 };
    H_rsfindtex = { "FindTexture", 0x425220, 7, "64a10000000055", (void *)h_rsfindtex, (void **)&o_rsfindtex, false, 0, 0, "resources", 0 };
    H_rstexres = { "LoadTextureResources", 0x425310, 7, "64a10000000055", (void *)h_rstexres, (void **)&o_rstexres, false, 0, 0, "resources", 0 };
    H_rstexlist = { "ReadTextureList", 0x425500, 7, "64a10000000055", (void *)h_rstexlist, (void **)&o_rstexlist, false, 0, 0, "resources", 0 };
    H_rsdefsprite = { "ReadDefaultPropSprite", 0x425870, 7, "64a10000000055", (void *)h_rsdefsprite, (void **)&o_rsdefsprite, false, 0, 0, "resources", 0 };
    H_rspoptex = { "PopTextureTable", 0x425ab0, 7, "64a10000000055", (void *)h_rspoptex, (void **)&o_rspoptex, false, 0, 0, "resources", 0 };
    H_rstexlook = { "TextureLookup", 0x424010, 7, "64a10000000055", (void *)h_rstexlook, (void **)&o_rstexlook, false, 0, 0, "resources", 0 };
    H_rsloadgeom = { "LoadGeometry", 0x4242b0, 7, "64a10000000055", (void *)h_rsloadgeom, (void **)&o_rsloadgeom, false, 0, 0, "resources", 0 };
    H_rsfindgeom = { "FindGeometry", 0x4246f0, 7, "64a10000000055", (void *)h_rsfindgeom, (void **)&o_rsfindgeom, false, 0, 0, "resources", 0 };
    H_rsgeomres = { "LoadGeometryResources", 0x4247e0, 7, "64a10000000055", (void *)h_rsgeomres, (void **)&o_rsgeomres, false, 0, 0, "resources", 0 };
    H_rsgeomlist = { "ReadGeometryList", 0x4249a0, 7, "64a10000000055", (void *)h_rsgeomlist, (void **)&o_rsgeomlist, false, 0, 0, "resources", 0 };
    H_rspopgeom = { "PopGeometryTable", 0x424c80, 7, "64a10000000055", (void *)h_rspopgeom, (void **)&o_rspopgeom, false, 0, 0, "resources", 0 };
    H_rsinitgeom = { "InitGeometryLoading", 0x4241d0, 7, "64a10000000055", (void *)h_rsinitgeom, (void **)&o_rsinitgeom, false, 0, 0, "resources", 0 };
    H_rserrlog = { "SqgErrorLog", 0x4241b0, 5, "8b44240450", (void *)h_rserrlog, (void **)&o_rserrlog, false, 0, 0, "resources", 0 };
    H_prinit = { "Prop::Init", 0x425e50, 7, "64a10000000055", (void *)h_prinit, (void **)&o_prinit, false, 0, 0, "props", 0 };
    H_prdestruct = { "Prop::Destruct", 0x426080, 7, "64a10000000055", (void *)h_prdestruct, (void **)&o_prdestruct, false, 0, 0, "props", 0 };
    H_prdestroy = { "Prop::Destroy", 0x425f80, 8, "568bf1e8f8000000", (void *)h_prdestroy, (void **)&o_prdestroy, false, 0, 0, "props", 4 };
    H_prinst = { "Prop::Instantiate", 0x425fa0, 7, "64a10000000055", (void *)h_prinst, (void **)&o_prinst, false, 0, 0, "props", 0 };
    H_prshow = { "Prop::Show", 0x426120, 5, "53568b4134", (void *)h_prshow, (void **)&o_prshow, false, 0, 0, "props", 0 };
    H_prisshown = { "Prop::IsShown", 0x426160, 8, "837934011bc040c3", (void *)h_prisshown, (void **)&o_prisshown, false, 0, 0, "props", 0 };
    H_prhide = { "Prop::Hide", 0x426170, 6, "568b41348bf1", (void *)h_prhide, (void **)&o_prhide, false, 0, 0, "props", 0 };
    H_prattach = { "Prop::GetAttachFrame", 0x426230, 5, "56578b4134", (void *)h_prattach, (void **)&o_prattach, false, 0, 0, "props", 0 };
    H_mpinit = { "MeshProp::Init", 0x4262e0, 7, "64a10000000055", (void *)h_mpinit, (void **)&o_mpinit, false, 0, 0, "props", 0 };
    H_mpdestruct = { "MeshProp::Destruct", 0x426580, 7, "64a10000000055", (void *)h_mpdestruct, (void **)&o_mpdestruct, false, 0, 0, "props", 0 };
    H_mpdestroy = { "MeshProp::Destroy", 0x426450, 8, "568bf1e828010000", (void *)h_mpdestroy, (void **)&o_mpdestroy, false, 0, 0, "props", 4 };
    H_mpclone = { "MeshProp::Clone", 0x426470, 7, "64a10000000055", (void *)h_mpclone, (void **)&o_mpclone, false, 0, 0, "props", 0 };
    H_mpbuild = { "MeshProp::BuildFrame", 0x4265f0, 7, "64a10000000055", (void *)h_mpbuild, (void **)&o_mpbuild, false, 0, 0, "props", 0 };
    H_spinit = { "SpriteProp::Init", 0x4267b0, 7, "64a10000000055", (void *)h_spinit, (void **)&o_spinit, false, 0, 0, "props", 0 };
    H_spdestruct = { "SpriteProp::Destruct", 0x426a70, 7, "64a10000000055", (void *)h_spdestruct, (void **)&o_spdestruct, false, 0, 0, "props", 0 };
    H_spdestroy = { "SpriteProp::Destroy", 0x426980, 8, "568bf1e8e8000000", (void *)h_spdestroy, (void **)&o_spdestroy, false, 0, 0, "props", 4 };
    H_spclone = { "SpriteProp::Clone", 0x4269a0, 7, "64a10000000055", (void *)h_spclone, (void **)&o_spclone, false, 0, 0, "props", 0 };
    H_spbuild = { "SpriteProp::BuildFrame", 0x426b00, 7, "64a10000000055", (void *)h_spbuild, (void **)&o_spbuild, false, 0, 0, "props", 0 };
    H_spscale = { "SpriteProp::ApplyScale", 0x426c00, 7, "64a10000000055", (void *)h_spscale, (void **)&o_spscale, false, 0, 0, "props", 0 };
    H_fpinit = { "FrameProp::Init", 0x426c70, 7, "64a10000000055", (void *)h_fpinit, (void **)&o_fpinit, false, 0, 0, "frameprop", 0 };
    H_fpdestruct = { "FrameProp::Destruct", 0x427100, 7, "64a10000000055", (void *)h_fpdestruct, (void **)&o_fpdestruct, false, 0, 0, "frameprop", 0 };
    H_fpdestroy = { "FrameProp::Destroy", 0x426f70, 8, "568bf1e888010000", (void *)h_fpdestroy, (void **)&o_fpdestroy, false, 0, 0, "frameprop", 4 };
    H_fpclone = { "FrameProp::Clone", 0x426fe0, 7, "64a10000000055", (void *)h_fpclone, (void **)&o_fpclone, false, 0, 0, "frameprop", 0 };
    H_fpbuild = { "FrameProp::BuildFrame", 0x4271f0, 7, "64a10000000055", (void *)h_fpbuild, (void **)&o_fpbuild, false, 0, 0, "frameprop", 0 };
    H_fphide = { "FrameProp::Hide", 0x427320, 5, "53568b4158", (void *)h_fphide, (void **)&o_fphide, false, 0, 0, "frameprop", 0 };
    H_fpown = { "FrameProp::BuildOwnFrame", 0x427380, 7, "64a10000000055", (void *)h_fpown, (void **)&o_fpown, false, 0, 0, "frameprop", 0 };
    H_fppart = { "FrameProp::GetPartFrame", 0x4276b0, 7, "64a10000000055", (void *)h_fppart, (void **)&o_fppart, false, 0, 0, "frameprop", 0 };
    H_fpupdate = { "FrameProp::Update", 0x427740, 7, "64a10000000055", (void *)h_fpupdate, (void **)&o_fpupdate, false, 0, 0, "frameprop", 0 };
    H_fpfire = { "FrameProp::FireMessages", 0x427b50, 7, "64a10000000055", (void *)h_fpfire, (void **)&o_fpfire, false, 0, 0, "frameprop", 0 };
    H_fptime = { "FrameProp::SetCycleTime", 0x427e30, 7, "538b8994000000", (void *)h_fptime, (void **)&o_fptime, false, 0, 0, "frameprop", 0 };
    H_fpdir = { "FrameProp::SetCycleDirection", 0x427e50, 7, "538b8994000000", (void *)h_fpdir, (void **)&o_fpdir, false, 0, 0, "frameprop", 0 };
    H_fploop = { "FrameProp::SetCycleLoopMode", 0x427e70, 7, "538b8994000000", (void *)h_fploop, (void **)&o_fploop, false, 0, 0, "frameprop", 0 };
    H_fpmatrix = { "FrameProp::GetPartMatrix", 0x427e90, 7, "64a10000000055", (void *)h_fpmatrix, (void **)&o_fpmatrix, false, 0, 0, "frameprop", 0 };
    H_fpactor = { "FrameProp::SetActor", 0x427f10, 10, "8b44240489819c000000", (void *)h_fpactor, (void **)&o_fpactor, false, 0, 0, "frameprop", 0 };
    H_fpfollow = { "FrameProp::SetFollowMode", 0x427f20, 10, "8b4424048981a0000000", (void *)h_fpfollow, (void **)&o_fpfollow, false, 0, 0, "frameprop", 0 };
    H_fphideend = { "FrameProp::SetHideAtEnd", 0x427f30, 10, "8b4424048981a8000000", (void *)h_fphideend, (void **)&o_fphideend, false, 0, 0, "frameprop", 0 };
    H_lpinit = { "LODProp::Init", 0x428050, 7, "64a10000000055", (void *)h_lpinit, (void **)&o_lpinit, false, 0, 0, "lodprop", 0 };
    H_lpdestruct = { "LODProp::Destruct", 0x428260, 7, "64a10000000055", (void *)h_lpdestruct, (void **)&o_lpdestruct, false, 0, 0, "lodprop", 0 };
    H_lpdestroy = { "LODProp::Destroy", 0x428240, 8, "568bf1e818000000", (void *)h_lpdestroy, (void **)&o_lpdestroy, false, 0, 0, "lodprop", 4 };
    H_lpclone = { "LODProp::Clone", 0x4282d0, 7, "64a10000000055", (void *)h_lpclone, (void **)&o_lpclone, false, 0, 0, "lodprop", 0 };
    H_lpbuild = { "LODProp::BuildFrame", 0x4283f0, 7, "64a10000000055", (void *)h_lpbuild, (void **)&o_lpbuild, false, 0, 0, "lodprop", 0 };
    H_lplevel = { "LODProp::UpdateLevel", 0x428510, 7, "64a10000000055", (void *)h_lplevel, (void **)&o_lplevel, false, 0, 0, "lodprop", 0 };
    H_lpselect = { "LODProp::SelectLevel", 0x4286d0, 7, "64a10000000055", (void *)h_lpselect, (void **)&o_lpselect, false, 0, 0, "lodprop", 0 };
    H_fpfind = { "FrameProp::FindChild", 0x427f60, 7, "64a10000000055", (void *)h_fpfind, (void **)&o_fpfind, false, 0, 0, "frameprop", 0 };
    H_pzctor = { "PartPose::PartPose", 0x4193c0, 6, "64a100000000", (void *)h_pzctor, (void **)&o_pzctor, false, 0, 0, "anim", 0 };
    H_pzassign = { "PartPose::Assign", 0x419530, 5, "83ec085356", (void *)h_pzassign, (void **)&o_pzassign, false, 0, 0, "anim", 0 };
    H_pzmatrix = { "PartPose::ToMatrix", 0x4195b0, 6, "64a100000000", (void *)h_pzmatrix, (void **)&o_pzmatrix, false, 0, 0, "anim", 0 };
    H_pcctor = { "PartCycle::PartCycle", 0x419750, 6, "64a100000000", (void *)h_pcctor, (void **)&o_pcctor, false, 0, 0, "anim", 0 };
    H_pcdtor = { "PartCycle::~PartCycle", 0x419880, 6, "64a100000000", (void *)h_pcdtor, (void **)&o_pcdtor, false, 0, 0, "anim", 0 };
    H_pcfind = { "PartCycle::FindSegment", 0x41b2a0, 5, "5356576a00", (void *)h_pcfind, (void **)&o_pcfind, false, 0, 0, "anim", 0 };
    H_pcsample = { "PartCycle::Sample", 0x419960, 6, "64a100000000", (void *)h_pcsample, (void **)&o_pcsample, false, 0, 0, "anim", 0 };
    H_pcnew = { "PartCycle::NewPose", 0x41a3f0, 6, "64a100000000", (void *)h_pcnew, (void **)&o_pcnew, false, 0, 0, "anim", 0 };
    H_pcblend = { "PartCycle::Blend", 0x41a5d0, 6, "64a100000000", (void *)h_pcblend, (void **)&o_pcblend, false, 0, 0, "anim", 0 };
    H_ptctor = { "PoseTable::PoseTable", 0x41b3f0, 6, "64a100000000", (void *)h_ptctor, (void **)&o_ptctor, false, 0, 0, "anim", 0 };
    H_ptcopy = { "PoseTable::PoseTable(copy)", 0x41b440, 6, "64a100000000", (void *)h_ptcopy, (void **)&o_ptcopy, false, 0, 0, "anim", 0 };
    H_ptdtor = { "PoseTable::~PoseTable", 0x41b5f0, 6, "64a100000000", (void *)h_ptdtor, (void **)&o_ptdtor, false, 0, 0, "anim", 0 };
    H_ptfind = { "PoseTable::Find", 0x41b6e0, 6, "64a100000000", (void *)h_ptfind, (void **)&o_ptfind, false, 0, 0, "anim", 0 };
    H_ptat = { "PoseTable::At", 0x41b750, 7, "8b49248b442404", (void *)h_ptat, (void **)&o_ptat, false, 0, 0, "anim", 0 };
    H_ptassign = { "PoseTable::Assign", 0x41b760, 5, "53565733db", (void *)h_ptassign, (void **)&o_ptassign, false, 0, 0, "anim", 0 };
    H_mcctor = { "MorphCycle::MorphCycle", 0x41b7b0, 6, "64a100000000", (void *)h_mcctor, (void **)&o_mcctor, false, 0, 0, "anim", 0 };
    H_mcrange = { "MorphCycle::MorphCycle(range)", 0x41b8b0, 7, "8b44241083ec04", (void *)h_mcrange, (void **)&o_mcrange, false, 0, 0, "anim", 0 };
    H_mcdtor = { "MorphCycle::~MorphCycle", 0x41b940, 6, "64a100000000", (void *)h_mcdtor, (void **)&o_mcdtor, false, 0, 0, "anim", 0 };
    H_mcnew = { "MorphCycle::NewPoseTable", 0x41ba70, 6, "64a100000000", (void *)h_mcnew, (void **)&o_mcnew, false, 0, 0, "anim", 0 };
    H_mcsample = { "MorphCycle::SamplePose", 0x41bbf0, 6, "64a100000000", (void *)h_mcsample, (void **)&o_mcsample, false, 0, 0, "anim", 0 };
    H_mcblend = { "MorphCycle::BlendPose", 0x41bcf0, 5, "83ec045356", (void *)h_mcblend, (void **)&o_mcblend, false, 0, 0, "anim", 0 };
    H_bpinitm = { "BodyPart::BodyPart(model)", 0x41d870, 10, "8b442404c701c4d74900", (void *)h_bpinitm, (void **)&o_bpinitm, false, 0, 0, "body", 0 };
    H_bpinit0 = { "BodyPart::BodyPart()", 0x41d8a0, 8, "33c0c701c4d74900", (void *)h_bpinit0, (void **)&o_bpinit0, false, 0, 0, "body", 0 };
    H_bpinitc = { "BodyPart::BodyPart(copy)", 0x41d8c0, 10, "8b442404c701c4d74900", (void *)h_bpinitc, (void **)&o_bpinitc, false, 0, 0, "body", 0 };
    H_bpdtor = { "BodyPart::~BodyPart", 0x41d8e0, 7, "56c701c4d74900", (void *)h_bpdtor, (void **)&o_bpdtor, false, 0, 0, "body", 0 };
    H_abinit = { "AnimatedBody::AnimatedBody", 0x41d9e0, 6, "64a100000000", (void *)h_abinit, (void **)&o_abinit, false, 0, 0, "body", 0 };
    H_abcopy = { "AnimatedBody::AnimatedBody(copy)", 0x41dab0, 6, "64a100000000", (void *)h_abcopy, (void **)&o_abcopy, false, 0, 0, "body", 0 };
    H_abdtor = { "AnimatedBody::~AnimatedBody", 0x41dc70, 6, "64a100000000", (void *)h_abdtor, (void **)&o_abdtor, false, 0, 0, "body", 0 };
    H_abadd = { "AnimatedBody::AddPart", 0x41dd30, 6, "64a100000000", (void *)h_abadd, (void **)&o_abadd, false, 0, 0, "body", 0 };
    H_abcycle = { "AnimatedBody::SetCycle", 0x41de20, 6, "64a100000000", (void *)h_abcycle, (void **)&o_abcycle, false, 0, 0, "body", 0 };
    H_abapply = { "AnimatedBody::ApplyPose", 0x41df10, 5, "53565733db", (void *)h_abapply, (void **)&o_abapply, false, 0, 0, "body", 0 };
    H_abadv = { "AnimatedBody::Advance", 0x41df50, 5, "83ec045356", (void *)h_abadv, (void **)&o_abadv, false, 0, 0, "body", 0 };
    H_abdir = { "AnimatedBody::SetDirection", 0x41e2d0, 6, "8b44240485c0", (void *)h_abdir, (void **)&o_abdir, false, 0, 0, "body", 0 };
    H_abloop = { "AnimatedBody::SetLoopMode", 0x41e2f0, 6, "8b44240485c0", (void *)h_abloop, (void **)&o_abloop, false, 0, 0, "body", 0 };
    H_abtime = { "AnimatedBody::SetTime", 0x41e290, 7, "8b442404894130", (void *)h_abtime, (void **)&o_abtime, false, 0, 0, "body", 0 };
    H_abblend = { "AnimatedBody::GetBlendFraction", 0x41e2a0, 6, "568b41448bf1", (void *)h_abblend, (void **)&o_abblend, false, 0, 0, "body", 0 };
    H_scpost = { "PostFunction", 0x422ce0, 7, "64a10000000055", (void *)h_scpost, (void **)&o_scpost, false, 0, 0, "script", 0 };
    H_imgrow    = { "ImageConvertRow",     0x477a90, 7,  "8b4c240883ec24",             (void *)h_imgrow,    (void **)&o_imgrow,    false, 0, 0, "img", 0 };
}

static void Summary()
{
    Log("Scene frames with warp backdrop=%u", g_warpFrames);
    for (Hook *h : g_hooks)
        Log("summary %-34s calls=%u mismatches=%u", h->name, h->calls, h->mismatches);
    for (Hook *h : g_ctnHooks)
        if (h->id && Listed(h->id))
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
        else if (v >= (DWORD)g_tramp && v < (DWORD)g_tramp + kTrampPool) Log("  stack[%d] points into trampoline pool +%lx", i, v - (DWORD)g_tramp);
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
        g_tramp = (BYTE *)VirtualAlloc(NULL, kTrampPool, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        AddVectoredExceptionHandler(1, CrashHandler);
        Setup();
        SetupContainers();
        if (!GetEnvironmentVariableA("SQZ_REPLACE", g_replaceList, sizeof g_replaceList))
            strcpy(g_replaceList, cfgReplace);
        Log("SQZ_REPLACE=%s", g_replaceList);
        BitmapLoadTrace = BitmapTrace;
        g_enumCompare = strstr(g_replaceList, "enumcmp") != NULL;
        g_qdmCompare = strstr(g_replaceList, "qdmcmp") != NULL;
        g_skyCompare = strstr(g_replaceList, "skycmp") != NULL;
        g_imgCompare = strstr(g_replaceList, "imgcmp") != NULL;
        g_animCompare = strstr(g_replaceList, "animcmp") != NULL;
        for (Hook *h : g_hooks) {
            if (h->id) { if (Listed(h->id) || (g_skyCompare && !strcmp(h->id, "sky")) || (g_imgCompare && !strcmp(h->id, "img")) || (g_animCompare && (h == &H_pcfind || h == &H_pcsample || h == &H_pcblend || h == &H_pzmatrix))) Install(*h); }          // heavy: explicit opt-in
            else if (!h->replaceOnly || g_mode == MODE_REPLACE) Install(*h);
        }
        for (Hook *h : g_ctnHooks)
            if (Listed(h->id)) Install(*h);
    } else if (reason == DLL_PROCESS_DETACH) {
        Summary();
        if (g_log != INVALID_HANDLE_VALUE) CloseHandle(g_log);
    }
    return TRUE;
}
