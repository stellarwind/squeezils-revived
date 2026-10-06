#include <windows.h>
#include <ddraw.h>
#include <stddef.h>
#include <string.h>

// Types and storage for the real ddraw.dll functions we call

typedef HRESULT (WINAPI *DirectDrawCreate_t)(GUID *, LPDIRECTDRAW *, IUnknown *);
typedef HRESULT (WINAPI *DirectDrawCreateClipper_t)(DWORD, LPDIRECTDRAWCLIPPER *, IUnknown *);

static HMODULE                   g_realDDraw;
static DirectDrawCreate_t        real_DirectDrawCreate;
static DirectDrawCreateClipper_t real_DirectDrawCreateClipper;

// Thanks Claude for this part. 
// Other system DLLs import from "ddraw.dll" and Windows gives them this module,
// so we must export every name the real one has. Each unused one is a stub: jmp *pointer.
// A jmp leaves the stack and arguments untouched, so any signature works.

#define FORWARDED(X) \
    X(AcquireDDThreadLock) X(CompleteCreateSysmemSurface) X(D3DParseUnknownCommand) \
    X(DDGetAttachedSurfaceLcl) X(DDInternalLock) X(DDInternalUnlock) X(DSoundHelp) \
    X(DirectDrawCreateEx) X(DirectDrawEnumerateA) X(DirectDrawEnumerateExA) \
    X(DirectDrawEnumerateExW) X(DirectDrawEnumerateW) X(DllCanUnloadNow) X(DllGetClassObject) \
    X(GetDDSurfaceLocal) X(GetOLEThunkData) X(GetSurfaceFromDC) X(RegisterSpecialCase) \
    X(ReleaseDDThreadLock) X(SetAppCompatData)

#define DECLARE_STUB(n) \
    static FARPROC p_##n __attribute__((used)); \
    __asm__(".text\n.globl _" #n "\n_" #n ":\n\tjmp *_p_" #n "\n");
FORWARDED(DECLARE_STUB)

// Load the real ddraw.dll by full path
// This limits us to Windwos 10+. Should go away after a full decomp. 

static BOOL load_real_ddraw(void)
{
    char path[MAX_PATH];
    UINT n = GetSystemDirectoryA(path, MAX_PATH);

    if (n == 0 || n > MAX_PATH - 16)
        return FALSE;
    strcat(path, "\\ddraw.dll");

    g_realDDraw = LoadLibraryA(path);
    if (!g_realDDraw)
        return FALSE;

    real_DirectDrawCreate        = (DirectDrawCreate_t)(void *)GetProcAddress(g_realDDraw, "DirectDrawCreate");
    real_DirectDrawCreateClipper = (DirectDrawCreateClipper_t)(void *)GetProcAddress(g_realDDraw, "DirectDrawCreateClipper");

#define LOAD_STUB(n) p_##n = GetProcAddress(g_realDDraw, #n);
    FORWARDED(LOAD_STUB)

    return real_DirectDrawCreate && real_DirectDrawCreateClipper;
}


// Repeat the og GDI call find current regio bbp
static BOOL desktop_deeper_than_16bpp(void)
{
    HDC dc = GetDC(NULL);
    int bpp;

    if (!dc)
        return FALSE;
    bpp = GetDeviceCaps(dc, BITSPIXEL);
    ReleaseDC(NULL, dc);
    return bpp > 16;
}

// Silently force 16bpp compat layer and relaunch the process. 
static BOOL relaunch_in_16bpp(void)
{
    char exe[MAX_PATH];
    char env[64];
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    if (GetEnvironmentVariableA("SQZ_PROXY_RESTARTED", env, sizeof env) > 0)
        return FALSE;
    if (!SetEnvironmentVariableA("SQZ_PROXY_RESTARTED", "1"))
        return FALSE;
    // ^ Disgusting part just to kill a looped trigger
    //////////////
    if (!desktop_deeper_than_16bpp())
        return FALSE;
    if (!GetModuleFileNameA(NULL, exe, MAX_PATH) || !SetEnvironmentVariableA("__COMPAT_LAYER", "16BITCOLOR"))
        return FALSE;

    memset(&si, 0, sizeof si);
    si.cb = sizeof si;
    if (!CreateProcessA(exe, GetCommandLineA(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        return FALSE;   // could not restart, carry on in the current process
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return TRUE;
}

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved)
{
    (void)reserved; // Skips warnings

    if (reason == DLL_PROCESS_ATTACH) {
        if (relaunch_in_16bpp())
            TerminateProcess(GetCurrentProcess(), 0);   // not ExitProcess: no DLL detach under the loader lock
        DisableThreadLibraryCalls(inst);
        if (!load_real_ddraw())
            return FALSE;
    } else if (reason == DLL_PROCESS_DETACH) {
        if (g_realDDraw)
            FreeLibrary(g_realDDraw);
    }
    return TRUE;
}

// The GetCaps fix:
// SQZ.EXE fails with "This display does not support a depth buffer" when
// DDCAPS.dwZBufferBitDepths is 0, which is what modern ddraw.dll reports for this legacy field.
// We hook IDirectDraw::GetCaps and report a 16-bit z-buffer, the game's own first choice.

// Used DX7 headers to gather offsets
// TODO remove superficial validation after we're done here
_Static_assert(offsetof(IDirectDrawVtbl, GetCaps) == 0x2c, "GetCaps vtable offset");
_Static_assert(offsetof(DDCAPS, dwZBufferBitDepths) == 0x38, "dwZBufferBitDepths offset");

typedef HRESULT (WINAPI *GetCaps_t)(LPDIRECTDRAW, LPDDCAPS, LPDDCAPS);
static GetCaps_t real_GetCaps;
static volatile LONG g_getCapsHooked;

static void fix_caps(LPDDCAPS caps)
{
    caps->dwZBufferBitDepths = DDBD_16;
}

static HRESULT WINAPI my_GetCaps(LPDIRECTDRAW self, LPDDCAPS driverCaps, LPDDCAPS helCaps)
{
    HRESULT hr = real_GetCaps(self, driverCaps, helCaps);

    if (SUCCEEDED(hr)) {
        fix_caps(driverCaps);
        fix_caps(helCaps);
    }
    return hr;
}

// All IDirectDraw objects from the real ddraw.dll share one vtable, so we patch it once
static void hook_getcaps(LPDIRECTDRAW renderer)
{
    void **slot;
    DWORD oldProtect;

    if (!renderer || InterlockedCompareExchange(&g_getCapsHooked, 1, 0) != 0)
        return;

    // Address of the GetCaps entry in the vtable (vtable + 0x2c)
    slot = (void **)&renderer->lpVtbl->GetCaps;

    // The vtable is read-only, make it writable for the swap
    if (!VirtualProtect(slot, sizeof(void *), PAGE_READWRITE, &oldProtect)) {
        g_getCapsHooked = 0;
        return;
    }
    real_GetCaps = (GetCaps_t)*slot;    // save the real function
    *slot = (void *)my_GetCaps;         // point the table at ours
    VirtualProtect(slot, sizeof(void *), oldProtect, &oldProtect);
}

// The exports

HRESULT WINAPI DirectDrawCreate(GUID *lpGUID, LPDIRECTDRAW *lplpDD, IUnknown *pUnkOuter)
{
    HRESULT hr = real_DirectDrawCreate(lpGUID, lplpDD, pUnkOuter);

    if (SUCCEEDED(hr) && lplpDD)
        hook_getcaps(*lplpDD);
    return hr;
}

HRESULT WINAPI DirectDrawCreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER *lplpDDClipper, IUnknown *pUnkOuter)
{
    return real_DirectDrawCreateClipper(dwFlags, lplpDDClipper, pUnkOuter);
}
