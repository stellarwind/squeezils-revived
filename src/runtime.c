#include "runtime.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Observation only: original object AND vtable addresses are preserved. Some
 * DirectDraw implementations identify interfaces by their native table address.
 * Patch table entries in this process, retaining immutable original snapshots.
 * Modules and instrumentation storage live until process teardown. */
typedef struct Record
{
    void *self;
    void **original;
    const SqzInterface *type;
    struct Record *next;
} Record;
#define BUCKETS 4096
#define SHADOW_SLOTS 256
static Record *records[BUCKETS];
static INIT_ONCE init_once = INIT_ONCE_STATIC_INIT;
static CRITICAL_SECTION lock;
static HMODULE module;
static char directory[MAX_PATH];
static INIT_ONCE real_once[2] = {INIT_ONCE_STATIC_INIT, INIT_ONCE_STATIC_INIT};
static HMODULE real_modules[2];

static BOOL CALLBACK initialize(PINIT_ONCE once, PVOID arg, PVOID *ctx)
{
    char *slash;
    (void)once;
    (void)arg;
    (void)ctx;
    InitializeCriticalSection(&lock);
    GetModuleFileNameA(module, directory, sizeof(directory));
    slash = strrchr(directory, '\\');
    if (slash)
        slash[1] = 0;
    return TRUE;
}

static unsigned bucket(void *self)
{
    return ((uintptr_t)self >> 3) % BUCKETS;
}
static Record *find(void *self)
{
    Record *r;
    for (r = records[bucket(self)]; r; r = r->next)
        if (r->self == self)
            return r;
    return NULL;
}

void *sqz_original(void *self)
{
    Record *r;
    void *result;
    DWORD saved = GetLastError();
    EnterCriticalSection(&lock);
    r = find(*(void **)self);
    result = r ? r->original : NULL;
    LeaveCriticalSection(&lock);
    SetLastError(saved);
    if (!result)
    {
        abort();
    }
    return result;
}

static void install(void *self, const SqzInterface *type)
{
    MEMORY_BASIC_INFORMATION memory;
    Record *r;
    void **table;
    size_t count;
    DWORD old, unused, saved = GetLastError();
    if (!self)
        return;
    InitOnceExecuteOnce(&init_once, initialize, NULL, NULL);
    EnterCriticalSection(&lock);
    table = *(void ***)self;
    r = find(table);
    if (r && r->type->slots >= type->slots)
        goto done;
    if (!VirtualQuery(table, &memory, sizeof(memory)))
        goto failed;
    count = ((uintptr_t)memory.BaseAddress + memory.RegionSize - (uintptr_t)table) / sizeof(void *);
    if (count > SHADOW_SLOTS)
        count = SHADOW_SLOTS;
    if (count < type->slots)
        goto failed;
    if (!r)
    {
        r = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*r));
        if (!r)
            goto failed;
        r->original = HeapAlloc(GetProcessHeap(), 0, SHADOW_SLOTS * sizeof(void *));
        if (!r->original)
        {
            HeapFree(GetProcessHeap(), 0, r);
            goto failed;
        }
        memcpy(r->original, table, count * sizeof(void *));
        r->self = table;
        r->next = records[bucket(table)];
        records[bucket(table)] = r;
    }
    r->type = type;
    if (!VirtualProtect(table, type->slots * sizeof(void *), PAGE_READWRITE, &old))
        goto failed;
    for (size_t i = 0; i < type->slots; ++i)
        InterlockedExchangePointer((void *volatile *)&table[i], ((void *const *)type->hooks)[i]);
    VirtualProtect(table, type->slots * sizeof(void *), old, &unused);
    goto done;
failed:
done:
    LeaveCriticalSection(&lock);
    SetLastError(saved);
}

__declspec(dllexport) void sqz_hook(void *self, const char *name)
{
    size_t i;
    for (i = 0; i < sqz_interface_count; ++i)
        if (!strcmp(name, sqz_interfaces[i].name))
        {
            install(self, &sqz_interfaces[i]);
            return;
        }
}

void sqz_hook_iid(void *self, REFIID iid)
{
    size_t i;
    if (!self || !iid)
        return;
    /* DX1 creates a device by querying a surface with a driver class GUID. */
    if (IsEqualGUID(iid, &IID_IDirect3DHALDevice) || IsEqualGUID(iid, &IID_IDirect3DRGBDevice) ||
        IsEqualGUID(iid, &IID_IDirect3DRampDevice) || IsEqualGUID(iid, &IID_IDirect3DMMXDevice))
    {
        sqz_hook(self, "IDirect3DDevice");
        return;
    }
    for (i = 0; i < sqz_interface_count; ++i)
        if (sqz_interfaces[i].iid && IsEqualGUID(iid, sqz_interfaces[i].iid))
        {
            install(self, &sqz_interfaces[i]);
            return;
        }
}

void sqz_forget(void *self)
{
    (void)self; /* Tables belong to pinned native modules, not released objects. */
}

static BOOL CALLBACK load_real(PINIT_ONCE once, PVOID arg, PVOID *ctx)
{
    char path[MAX_PATH + 64];
    unsigned library = (unsigned)(uintptr_t)arg;
    (void)once;
    (void)ctx;
    InitOnceExecuteOnce(&init_once, initialize, NULL, NULL);
    if (library == 0)
    {
        UINT length = GetSystemDirectoryA(path, sizeof(path));
        if (!length || length + 11 >= sizeof(path))
            return FALSE;
        strcat(path, "\\ddraw.dll");
    }
    else
    {
        /* Normal deployment leaves the game's original RM DLL untouched. The
         * legacy path is retained solely for the standalone export-shim tests. */
        snprintf(path, sizeof(path), "%slegacy\\d3drm.dll", directory);
        if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES)
            snprintf(path, sizeof(path), "%sd3drm.dll", directory);
    }
    real_modules[library] = LoadLibraryExA(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    return real_modules[library] != NULL;
}

static FARPROC original_export(unsigned library, const char *name)
{
    FARPROC function;
    if (!InitOnceExecuteOnce(&real_once[library], load_real, (void *)(uintptr_t)library, NULL))
    {
        abort();
    }
    function = GetProcAddress(real_modules[library], name);
    if (!function)
    {
        abort();
    }
    return function;
}

static HRESULT WINAPI create_dd(GUID *guid, IDirectDraw **out, IUnknown *outer)
{
    typedef HRESULT(WINAPI * Function)(GUID *, IDirectDraw **, IUnknown *);
    HRESULT hr = ((Function)original_export(0, "DirectDrawCreate"))(guid, out, outer);
    if (SUCCEEDED(hr) && out)
        sqz_hook(*out, "IDirectDraw");
    return hr;
}
static HRESULT WINAPI create_clipper(DWORD flags, IDirectDrawClipper **out, IUnknown *outer)
{
    typedef HRESULT(WINAPI * Function)(DWORD, IDirectDrawClipper **, IUnknown *);
    HRESULT hr = ((Function)original_export(0, "DirectDrawCreateClipper"))(flags, out, outer);
    if (SUCCEEDED(hr) && out)
        sqz_hook(*out, "IDirectDrawClipper");
    return hr;
}
HRESULT WINAPI sqz_create_rm(IDirect3DRM **out)
{
    typedef HRESULT(WINAPI * Function)(IDirect3DRM **);
    HRESULT hr = ((Function)original_export(1, "Direct3DRMCreate"))(out);
    if (SUCCEEDED(hr) && out)
        sqz_hook(*out, "IDirect3DRM");
    return hr;
}

__declspec(dllexport) FARPROC __cdecl sqz_resolve(unsigned library, const char *name)
{
    if (!library && !strcmp(name, "DirectDrawCreate"))
        return (FARPROC)create_dd;
    if (!library && !strcmp(name, "DirectDrawCreateClipper"))
        return (FARPROC)create_clipper;
    if (library && !strcmp(name, "Direct3DRMCreate"))
        return (FARPROC)sqz_create_rm;
    return original_export(library, name);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH)
    {
        module = instance;
        /* Memory-only verified code edits, before WinMain's desktop-depth gate. */
        sqz_color_install();
        sqz_platform_install();
    }
    return TRUE;
}
