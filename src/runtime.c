#include "runtime.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

/* Observation only: original object AND vtable addresses are preserved. Some
 * DirectDraw implementations identify interfaces by their native table address.
 * Patch table entries in this process, retaining immutable original snapshots.
 * Modules and instrumentation storage live until process teardown. */
typedef struct Record {
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
static HANDLE log_file = INVALID_HANDLE_VALUE;
static HMODULE module;
static LONG sequence;
static char directory[MAX_PATH];
static INIT_ONCE real_once[2] = {INIT_ONCE_STATIC_INIT, INIT_ONCE_STATIC_INIT};
static HMODULE real_modules[2];

static BOOL CALLBACK initialize(PINIT_ONCE once, PVOID arg, PVOID *ctx)
{
    char path[MAX_PATH + 64];
    char *slash;
    (void)once; (void)arg; (void)ctx;
    InitializeCriticalSection(&lock);
    GetModuleFileNameA(module, directory, sizeof(directory));
    slash = strrchr(directory, '\\');
    if (slash) slash[1] = 0;
    snprintf(path, sizeof(path), "%ssqz-forward-%lu.log", directory, GetCurrentProcessId());
    log_file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    return TRUE;
}

void sqz_log(const char *format, ...)
{
    DWORD saved = GetLastError(), written;
    char line[1400];
    int n, tail;
    va_list args;
    InitOnceExecuteOnce(&init_once, initialize, NULL, NULL);
    EnterCriticalSection(&lock);
    n = snprintf(line, sizeof(line), "%ld tid=%lu ", ++sequence, GetCurrentThreadId());
    va_start(args, format);
    tail = vsnprintf(line + n, sizeof(line) - n - 2, format, args);
    va_end(args);
    if (tail < 0) tail = 0;
    n += tail < (int)sizeof(line) - n - 2 ? tail : (int)sizeof(line) - n - 2;
    line[n++] = '\n';
    if (log_file != INVALID_HANDLE_VALUE) WriteFile(log_file, line, (DWORD)n, &written, NULL);
    LeaveCriticalSection(&lock);
    SetLastError(saved);
}

static unsigned bucket(void *self) { return ((uintptr_t)self >> 3) % BUCKETS; }
static Record *find(void *self)
{
    Record *r;
    for (r = records[bucket(self)]; r; r = r->next) if (r->self == self) return r;
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
    if (!result) { sqz_log("FATAL missing original self=%p", self); abort(); }
    return result;
}

static void install(void *self, const SqzInterface *type)
{
    MEMORY_BASIC_INFORMATION memory;
    Record *r;
    void **table;
    size_t count;
    DWORD old, unused, saved = GetLastError();
    if (!self) return;
    InitOnceExecuteOnce(&init_once, initialize, NULL, NULL);
    EnterCriticalSection(&lock);
    table = *(void ***)self;
    r = find(table);
    if (r && r->type->slots >= type->slots) goto done;
    if (!VirtualQuery(table, &memory, sizeof(memory))) goto failed;
    count = ((uintptr_t)memory.BaseAddress + memory.RegionSize - (uintptr_t)table) / sizeof(void *);
    if (count > SHADOW_SLOTS) count = SHADOW_SLOTS;
    if (count < type->slots) goto failed;
    if (!r) {
        r = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*r));
        if (!r) goto failed;
        r->original = HeapAlloc(GetProcessHeap(), 0, SHADOW_SLOTS * sizeof(void *));
        if (!r->original) { HeapFree(GetProcessHeap(), 0, r); goto failed; }
        memcpy(r->original, table, count * sizeof(void *));
        r->self = table;
        r->next = records[bucket(table)];
        records[bucket(table)] = r;
    }
    r->type = type;
    if (!VirtualProtect(table, type->slots * sizeof(void *), PAGE_READWRITE, &old)) goto failed;
    for (size_t i = 0; i < type->slots; ++i)
        InterlockedExchangePointer((void *volatile *)&table[i], ((void *const *)type->hooks)[i]);
    VirtualProtect(table, type->slots * sizeof(void *), old, &unused);
    sqz_log("HOOK %s self=%p slots=%u", type->name, self, (unsigned)type->slots);
    goto done;
failed:
    sqz_log("GAP cannot instrument %s self=%p error=%lu", type->name, self, GetLastError());
done:
    LeaveCriticalSection(&lock);
    SetLastError(saved);
}

void sqz_hook(void *self, const char *name)
{
    size_t i;
    for (i = 0; i < sqz_interface_count; ++i)
        if (!strcmp(name, sqz_interfaces[i].name)) { install(self, &sqz_interfaces[i]); return; }
    sqz_log("GAP interface=%s self=%p", name, self);
}

void sqz_hook_iid(void *self, REFIID iid)
{
    size_t i;
    if (!self || !iid) return;
    /* DX1 creates a device by querying a surface with a driver class GUID. */
    if (IsEqualGUID(iid, &IID_IDirect3DHALDevice) || IsEqualGUID(iid, &IID_IDirect3DRGBDevice) ||
        IsEqualGUID(iid, &IID_IDirect3DRampDevice) || IsEqualGUID(iid, &IID_IDirect3DMMXDevice)) {
        sqz_hook(self, "IDirect3DDevice");
        return;
    }
    for (i = 0; i < sqz_interface_count; ++i)
        if (sqz_interfaces[i].iid && IsEqualGUID(iid, sqz_interfaces[i].iid)) { install(self, &sqz_interfaces[i]); return; }
    sqz_log("GAP IID=%08lx-%04x-%04x self=%p (native pointer preserved)",
            iid->Data1, iid->Data2, iid->Data3, self);
}

void sqz_forget(void *self)
{
    (void)self; /* Tables belong to pinned native modules, not released objects. */
}

static BOOL CALLBACK load_real(PINIT_ONCE once, PVOID arg, PVOID *ctx)
{
    unsigned library = (unsigned)(uintptr_t)arg;
    char path[MAX_PATH + 64];
    (void)once; (void)ctx;
    InitOnceExecuteOnce(&init_once, initialize, NULL, NULL);
    if (library == 0) {
        UINT length = GetSystemDirectoryA(path, sizeof(path));
        if (!length || length + 11 >= sizeof(path)) return FALSE;
        strcat(path, "\\ddraw.dll");
    } else {
        /* Normal deployment leaves the game's original RM DLL untouched. The
         * legacy path is retained solely for the standalone export-shim tests. */
        snprintf(path, sizeof(path), "%slegacy\\d3drm.dll", directory);
        if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES)
            snprintf(path, sizeof(path), "%sd3drm.dll", directory);
    }
    real_modules[library] = LoadLibraryExA(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    sqz_log("LOAD %s module=%p error=%lu", path, real_modules[library],
            real_modules[library] ? 0 : GetLastError());
    return real_modules[library] != NULL;
}

static FARPROC original_export(unsigned library, const char *name)
{
    FARPROC function;
    if (!InitOnceExecuteOnce(&real_once[library], load_real, (void *)(uintptr_t)library, NULL)) {
        sqz_log("FATAL load %u %s", library, name); abort();
    }
    function = GetProcAddress(real_modules[library], name);
    if (!function) { sqz_log("FATAL missing export %u %s", library, name); abort(); }
    return function;
}

static HRESULT WINAPI create_dd(GUID *guid, IDirectDraw **out, IUnknown *outer)
{
    typedef HRESULT (WINAPI *Function)(GUID *, IDirectDraw **, IUnknown *);
    HRESULT hr = ((Function)original_export(0, "DirectDrawCreate"))(guid, out, outer);
    sqz_log("EXPORT DirectDrawCreate hr=%08lx self=%p", hr, SUCCEEDED(hr) && out ? *out : NULL);
    if (SUCCEEDED(hr) && out) sqz_hook(*out, "IDirectDraw");
    return hr;
}
static HRESULT WINAPI create_clipper(DWORD flags, IDirectDrawClipper **out, IUnknown *outer)
{
    typedef HRESULT (WINAPI *Function)(DWORD, IDirectDrawClipper **, IUnknown *);
    HRESULT hr = ((Function)original_export(0, "DirectDrawCreateClipper"))(flags, out, outer);
    sqz_log("EXPORT DirectDrawCreateClipper hr=%08lx", hr);
    if (SUCCEEDED(hr) && out) sqz_hook(*out, "IDirectDrawClipper");
    return hr;
}
HRESULT WINAPI sqz_create_rm(IDirect3DRM **out)
{
    typedef HRESULT (WINAPI *Function)(IDirect3DRM **);
    HRESULT hr = ((Function)original_export(1, "Direct3DRMCreate"))(out);
    sqz_log("EXPORT Direct3DRMCreate hr=%08lx self=%p", hr, SUCCEEDED(hr) && out ? *out : NULL);
    if (SUCCEEDED(hr) && out) sqz_hook(*out, "IDirect3DRM");
    return hr;
}

__declspec(dllexport) FARPROC __cdecl sqz_resolve(unsigned library, const char *name)
{
    sqz_log("ROUTE %s!%s", library ? "d3drm" : "ddraw", name);
    if (!library && !strcmp(name, "DirectDrawCreate")) return (FARPROC)create_dd;
    if (!library && !strcmp(name, "DirectDrawCreateClipper")) return (FARPROC)create_clipper;
    if (library && !strcmp(name, "Direct3DRMCreate")) return (FARPROC)sqz_create_rm;
    return original_export(library, name);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        module = instance;
        sqz_platform_install();
    }
    return TRUE;
}
