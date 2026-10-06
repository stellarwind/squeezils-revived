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

// Other system DLLs (e.g. D3DIM.DLL) import from "ddraw.dll" and Windows gives them this module,
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

// Load the real ddraw.dll by full path (a bare name would find this file again)

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

// One-time setup only, this runs under the loader lock so keep it light

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved)
{
    (void)reserved;

    if (reason == DLL_PROCESS_ATTACH) {
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
// SQZ.EXE (FUN_00475c10) fails with "This display does not support a depth buffer" when
// DDCAPS.dwZBufferBitDepths is 0, which is what modern ddraw.dll reports for this legacy field.
// We hook IDirectDraw::GetCaps and report a 16-bit z-buffer, the game's own first choice.

// Verify vtable and struct offsets at compile time
_Static_assert(offsetof(IDirectDrawVtbl, GetCaps) == 0x2c, "GetCaps vtable offset");
_Static_assert(offsetof(DDCAPS, dwZBufferBitDepths) == 0x38, "dwZBufferBitDepths offset");

typedef HRESULT (WINAPI *GetCaps_t)(LPDIRECTDRAW, LPDDCAPS, LPDDCAPS);
static GetCaps_t real_GetCaps;
static volatile LONG g_getCapsHooked;

static void fix_caps(LPDDCAPS caps)
{
    // Only touch the field if the caller's struct is big enough to contain it
    if (caps && caps->dwSize >= offsetof(DDCAPS, dwZBufferBitDepths) + sizeof(DWORD)
             && caps->dwZBufferBitDepths == 0)
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
static void hook_getcaps(LPDIRECTDRAW dd)
{
    void **slot;
    DWORD oldProtect;

    if (!dd || InterlockedCompareExchange(&g_getCapsHooked, 1, 0) != 0)
        return;

    // Address of the GetCaps entry in the vtable (vtable + 0x2c)
    slot = (void **)&dd->lpVtbl->GetCaps;

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
