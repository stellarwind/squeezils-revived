// Logging proxy d3drm.dll. Loads the game's real DLL (renamed d3drm_real.dll), forwards all exports,
// and wraps Direct3DRMCreate. Every D3DRM object found is vtable-patched so each method call is logged
// to d3drm_log.txt, with a per-method call count in d3drm_summary.txt. Purpose: learn which D3DRM
// interfaces and methods the game really uses. Not meant to ship.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define MAX_VT      96
#define MAX_SLOTS   256
#define FULL_LOG_CALLS 1500   // log every call for the first N, then only first FIRST_N per (vt,slot)
#define FIRST_N     3

typedef struct {
    void **vt;                 // vtable in the real DLL (patched in place)
    int    n;                  // slot count (run of code pointers)
    void  *orig[MAX_SLOTS];    // original function pointers
    unsigned count[MAX_SLOTS];
    char   how[96];            // how this vtable was first seen
} VTInfo;

static VTInfo  g_vt[MAX_VT];
static int     g_nvt;
static HMODULE g_real;
static DWORD   g_modBase, g_modSize, g_dataLo, g_dataHi, g_textLo, g_textHi;
static HANDLE  g_log = INVALID_HANDLE_VALUE;
static CRITICAL_SECTION g_cs;
static unsigned g_calls;
static unsigned char *g_stubMem; static size_t g_stubUsed;

#define RING 24
static struct { unsigned id; unsigned args[8]; int scans; } g_ring[RING];
static unsigned g_ringPos;

static void dlog(const char *fmt, ...)
{
    char buf[512]; DWORD w;
    va_list ap; va_start(ap, fmt);
    int n = _vsnprintf(buf, sizeof buf - 2, fmt, ap);
    va_end(ap);
    if (n < 0 || n > (int)sizeof buf - 2) n = sizeof buf - 2;
    buf[n++] = '\n';
    if (g_log != INVALID_HANDLE_VALUE) WriteFile(g_log, buf, n, &w, NULL);
}

static BOOL readable(const void *p, size_t n) { return p && !IsBadReadPtr(p, n); }

// Who made the calls: how often each return address called each hooked method. Return addresses inside the game's
// EXE are call sites still running the original code; anything else (for example sqzhook.dll) is reconstructed code.
#define MAX_CALLERS 4096
static struct { unsigned ret, id, count; } g_callers[MAX_CALLERS];
static DWORD g_exeLo, g_exeHi;

static void note_caller(unsigned ret, unsigned id)
{
    unsigned i = (ret * 2654435761u ^ id * 40503u) % MAX_CALLERS;
    for (int probe = 0; probe < MAX_CALLERS; probe++, i = (i + 1) % MAX_CALLERS) {
        if (g_callers[i].count == 0) { g_callers[i].ret = ret; g_callers[i].id = id; g_callers[i].count = 1; return; }
        if (g_callers[i].ret == ret && g_callers[i].id == id) { g_callers[i].count++; return; }
    }
}

static void write_summary(void)
{
    HANDLE h = CreateFileA("d3drm_summary.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    char line[200]; DWORD w;
    for (int v = 0; v < g_nvt; v++) {
        int n = _snprintf(line, sizeof line, "VT%d %08x slots=%d %s\r\n", v, (unsigned)((DWORD)g_vt[v].vt - g_modBase), g_vt[v].n, g_vt[v].how);
        WriteFile(h, line, n, &w, NULL);
        for (int s = 0; s < g_vt[v].n; s++) if (g_vt[v].count[s]) {
            n = _snprintf(line, sizeof line, "  VT%d slot %d (0x%x) calls=%u\r\n", v, s, s * 4, g_vt[v].count[s]);
            WriteFile(h, line, n, &w, NULL);
        }
    }
    {
        unsigned long long fromExe = 0, fromOther = 0;
        for (int i = 0; i < MAX_CALLERS; i++) if (g_callers[i].count) {
            if (g_callers[i].ret >= g_exeLo && g_callers[i].ret < g_exeHi) fromExe += g_callers[i].count; else fromOther += g_callers[i].count;
        }
        int n = _snprintf(line, sizeof line, "CALLERS total=%llu from_exe=%llu from_other=%llu\r\n", fromExe + fromOther, fromExe, fromOther);
        WriteFile(h, line, n, &w, NULL);
        for (int i = 0; i < MAX_CALLERS; i++) if (g_callers[i].count) {
            n = _snprintf(line, sizeof line, "CALLER %08x %s VT%u.%u calls=%u\r\n", g_callers[i].ret,
                          (g_callers[i].ret >= g_exeLo && g_callers[i].ret < g_exeHi) ? "EXE" : "OTHER", g_callers[i].id >> 16, g_callers[i].id & 0xffff, g_callers[i].count);
            WriteFile(h, line, n, &w, NULL);
        }
    }
    CloseHandle(h);
}

static BOOL in_range(DWORD v, DWORD lo, DWORD hi) { return v >= lo && v < hi; }

static void *make_stub(unsigned id, void **origSlot);

// Patch the vtable of object `obj` if not yet known. Returns vt index or -1.
static int adopt(void *obj, const char *how)
{
    if (!readable(obj, 4)) return -1;
    void **vt = *(void ***)obj;
    if (!in_range((DWORD)vt, g_dataLo, g_dataHi) || !readable(vt, 12)) return -1;
    for (int i = 0; i < g_nvt; i++) if (g_vt[i].vt == vt) return i;
    if (g_nvt >= MAX_VT) return -1;
    int n = 0;
    while (n < MAX_SLOTS && readable(&vt[n], 4) && in_range((DWORD)vt[n], g_textLo, g_textHi)) n++;
    if (n < 3) return -1;
    VTInfo *vi = &g_vt[g_nvt];
    vi->vt = vt; vi->n = n;
    _snprintf(vi->how, sizeof vi->how, "%s", how);
    DWORD old;
    VirtualProtect(vt, n * 4, PAGE_READWRITE, &old);
    for (int s = 0; s < n; s++) {
        vi->orig[s] = vt[s];
        vt[s] = make_stub((g_nvt << 16) | s, &vi->orig[s]);
    }
    VirtualProtect(vt, n * 4, old, &old);
    dlog("NEW VT%d %08x slots=%d  (%s)", g_nvt, (unsigned)((DWORD)vt - g_modBase), n, how);
    return g_nvt++;
}

static void fmt_arg(char *out, size_t cap, unsigned a)
{
    int n = _snprintf(out, cap, "%x", a);
    float f; memcpy(&f, &a, 4);
    if (f > 1e-6f && f < 1e7f) n += _snprintf(out + n, cap - n, "(f%.3g)", f);
    else if (f < -1e-6f && f > -1e7f) n += _snprintf(out + n, cap - n, "(f%.3g)", f);
    const char *s = (const char *)a;
    if (a > 0x10000 && readable(s, 4)) {
        int k = 0; while (k < 60 && readable(s + k, 1) && s[k] >= 32 && s[k] < 127) k++;
        if (k >= 3 && (s[k] == 0)) { n += _snprintf(out + n, cap - n, "\"%.*s\"", k, s); }
    }
}

void __cdecl log_hook(unsigned id, unsigned *args)
{
    EnterCriticalSection(&g_cs);
    // deferred check: recent non-IUnknown calls may have created objects through out-params.
    // Rescan the last few hooked calls (a call is still running while d3drm calls AddRef internally).
    for (int r = 0; r < RING; r++) {
        if (!g_ring[r].id || g_ring[r].scans >= 12) continue;
        g_ring[r].scans++;
        if ((g_ring[r].id & 0xffff) < 3) { g_ring[r].scans = 99; continue; }
        for (int i = 1; i < 8; i++) {
            unsigned a = g_ring[r].args[i];
            if (a < 0x10000 || !readable((void *)a, 4)) continue;
            unsigned first = *(unsigned *)a;
            if (in_range(first, g_dataLo, g_dataHi)) continue;
            if (first < 0x10000 || !readable((void *)first, 4)) continue;
            char how[96];
            _snprintf(how, sizeof how, "created by VT%u slot %u arg%d", g_ring[r].id >> 16, g_ring[r].id & 0xffff, i);
            adopt((void *)first, how);
        }
    }
    unsigned v = id >> 16, s = id & 0xffff;
    note_caller(args[-1], id);
    unsigned c = ++g_vt[v].count[s];
    g_calls++;
    if (g_calls <= FULL_LOG_CALLS || c <= FIRST_N) {
        char a[6][80]; for (int i = 0; i < 6; i++) fmt_arg(a[i], sizeof a[i], args[i]);
        dlog("#%u VT%u.%u(%s,%s,%s,%s,%s,%s) n=%u", g_calls, v, s, a[0], a[1], a[2], a[3], a[4], a[5], c);
    }
    { unsigned p = g_ringPos++ % RING; g_ring[p].id = id; memcpy(g_ring[p].args, args, sizeof g_ring[p].args); g_ring[p].scans = 0; }
    if ((g_calls % 4000) == 0) write_summary();
    LeaveCriticalSection(&g_cs);
}

// Per-slot stub: log then jump to the original, arguments and return address untouched.
static void *make_stub(unsigned id, void **origSlot)
{
    unsigned char *p = g_stubMem + g_stubUsed;
    unsigned char *s = p;
    *p++ = 0x9C;                                   // pushfd
    *p++ = 0x60;                                   // pushad
    *p++ = 0x8D; *p++ = 0x44; *p++ = 0x24; *p++ = 0x28; // lea eax,[esp+0x28]  (-> this)
    *p++ = 0x50;                                   // push eax
    *p++ = 0x68; *(unsigned *)p = id; p += 4;      // push id
    *p++ = 0xE8; *(int *)p = (int)((unsigned char *)log_hook - (p + 4)); p += 4; // call log_hook
    *p++ = 0x83; *p++ = 0xC4; *p++ = 0x08;         // add esp,8
    *p++ = 0x61;                                   // popad
    *p++ = 0x9D;                                   // popfd
    *p++ = 0xFF; *p++ = 0x25; *(void ***)p = origSlot; p += 4; // jmp [origSlot]
    g_stubUsed += 48;
    return s;
}

typedef HRESULT (WINAPI *Create_t)(void **);

HRESULT WINAPI Direct3DRMCreate(void **out)
{
    Create_t real = (Create_t)(void *)GetProcAddress(g_real, "Direct3DRMCreate");
    HRESULT hr = real(out);
    EnterCriticalSection(&g_cs);
    dlog("Direct3DRMCreate -> hr=%08x obj=%p", (unsigned)hr, out ? *out : NULL);
    if (hr == 0 && out && *out) adopt(*out, "root IDirect3DRM");
    LeaveCriticalSection(&g_cs);
    return hr;
}

BOOL WINAPI DllMain(HINSTANCE h, DWORD reason, LPVOID r)
{
    (void)r;
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(h);
        InitializeCriticalSection(&g_cs);
        char path[MAX_PATH]; GetModuleFileNameA(h, path, MAX_PATH);
        char *slash = strrchr(path, '\\'); if (slash) *slash = 0;
        char real[MAX_PATH]; _snprintf(real, sizeof real, "%s\\d3drm_real.dll", path);
        g_real = LoadLibraryA(real);
        if (!g_real) { MessageBoxA(NULL, "d3drm_real.dll not found next to the proxy.", "d3drm proxy", MB_OK); return FALSE; }
        g_modBase = (DWORD)g_real;
        // locate .text and .data of the real DLL
        IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)g_real;
        IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)((BYTE *)g_real + dos->e_lfanew);
        g_modSize = nt->OptionalHeader.SizeOfImage;
        IMAGE_SECTION_HEADER *sec = IMAGE_FIRST_SECTION(nt);
        for (int i = 0; i < nt->FileHeader.NumberOfSections; i++) {
            DWORD lo = g_modBase + sec[i].VirtualAddress, hi = lo + sec[i].Misc.VirtualSize;
            if (!memcmp(sec[i].Name, ".text", 5)) { g_textLo = lo; g_textHi = hi; }
            if (!memcmp(sec[i].Name, ".data", 5)) { g_dataLo = lo; g_dataHi = hi; }
        }
        { HMODULE exe = GetModuleHandleA(NULL); IMAGE_NT_HEADERS *xnt = (IMAGE_NT_HEADERS *)((BYTE *)exe + ((IMAGE_DOS_HEADER *)exe)->e_lfanew);
          g_exeLo = (DWORD)exe; g_exeHi = g_exeLo + xnt->OptionalHeader.SizeOfImage; }
        g_stubMem = (unsigned char *)VirtualAlloc(NULL, MAX_VT * MAX_SLOTS * 48, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        g_log = CreateFileA("d3drm_log.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
        dlog("d3drm logging proxy; real base %08lx text %08lx-%08lx data %08lx-%08lx", g_modBase, g_textLo, g_textHi, g_dataLo, g_dataHi);
    } else if (reason == DLL_PROCESS_DETACH) {
        write_summary();
        dlog("detach, %u calls", g_calls);
        if (g_log != INVALID_HANDLE_VALUE) CloseHandle(g_log);
    }
    return TRUE;
}
