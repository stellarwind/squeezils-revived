/* One SDL-created native window; the game's thread remains the message owner. */
#define SDL_MAIN_HANDLED
#include "runtime.h"
#include "geometry.h"
#include "save_compat.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <string.h>
#include <stdio.h>

#define SDL_FUNCTIONS(X)                                                                           \
    X(SDL_Init)                                                                                    \
    X(SDL_SetMainReady)                                                                            \
    X(SDL_SetHint) X(SDL_CreateWindow) X(SDL_GetWindowProperties) X(SDL_GetPointerProperty)        \
        X(SDL_DestroyWindow) X(SDL_PumpEvents) X(SDL_FlushEvents) X(SDL_GetError)
#define DECLARE(name) static __typeof__(&name) p_##name;
SDL_FUNCTIONS(DECLARE)
static SDL_Window *sdl_window;
static HWND game_hwnd;
void sqz_platform_gpu_title(void)
{
    if (game_hwnd)
        SetWindowTextA(game_hwnd, "Squeezils [SDL3 GPU]");
}
static WNDPROC sdl_proc, game_proc;
static int initial_width, initial_height;
static IDirectDrawSurface *virtual_primary, *native_primary;
static BOOL enabled, creating, pumping, presenting;
static HWND(WINAPI *native_create)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU,
                                   HINSTANCE, LPVOID);
static BOOL(WINAPI *native_destroy)(HWND);
static LRESULT(WINAPI *native_default)(HWND, UINT, WPARAM, LPARAM);
static BOOL(WINAPI *native_peek)(MSG *, HWND, UINT, UINT, UINT);
static LONG(WINAPI *native_style)(HWND, int, LONG);
static BOOL(WINAPI *native_position)(HWND, HWND, int, int, int, int, UINT);
static BOOL(WINAPI *native_show)(HWND, int);
static BOOL(WINAPI *native_client_rect)(HWND, RECT *);
static BOOL(WINAPI *native_to_screen)(HWND, POINT *);
static BOOL(WINAPI *native_to_client)(HWND, POINT *);
static BOOL(WINAPI *native_cursor)(int, int);
static HCURSOR(WINAPI *native_set_cursor)(HCURSOR);
static HDC(WINAPI *native_get_dc)(HWND);
static int(WINAPI *native_release_dc)(HWND, HDC);
static int(WINAPI *native_device_caps)(HDC, int);
static HFONT(WINAPI *native_font_indirect)(const LOGFONTA *);
static HDC canvas_dc;
static IDirectDrawSurface *dc_surface;
static unsigned dc_depth;
static int dc_states[32];
static BOOL repaint_queued, repaint_pending, refresh_clipping;
static int WINAPI canvas_device_caps(HDC dc, int index)
{
    int native_value = native_device_caps(dc, index);
    int value = native_value;
    /* HUD point sizes are converted to pixels by the original game. Its canvas
     * is always 640x480; presentation scales the complete frame afterward. Use
     * 96 DPI for canvas DCs and the game window's DC before the primary exists.
     * Screen, printer and unrelated memory DCs retain their native metrics. */
    BOOL game_dc = dc && (dc == canvas_dc || (game_hwnd && WindowFromDC(dc) == game_hwnd));
    if (game_dc && (index == LOGPIXELSX || index == LOGPIXELSY))
        value = 96;
    if (index == LOGPIXELSX || index == LOGPIXELSY || index == BITSPIXEL)
        sqz_log("DIAG GAME_CAPS dc=%p canvas=%d index=%d value=%d native=%d", dc, dc == canvas_dc,
                index, value, native_value);
    return value;
}
static HFONT WINAPI diagnostic_font_indirect(const LOGFONTA *font)
{
    HFONT result = native_font_indirect(font);
    if (font)
        sqz_log("DIAG FONT font=%p face=%.32s height=%ld width=%ld weight=%ld charset=%u", result,
                font->lfFaceName, font->lfHeight, font->lfWidth, font->lfWeight, font->lfCharSet);
    return result;
}
#define SQZ_REPAINT (WM_APP + 0x351)
static HRESULT present_canvas(void);
static void request_repaint(void)
{
    if (!game_hwnd || !virtual_primary)
        return;
    repaint_pending = TRUE;
    if (!repaint_queued && PostMessageA(game_hwnd, SQZ_REPAINT, 0, 0))
        repaint_queued = TRUE;
}

/* Window text (including the timer) and DD sprites must share one logical
 * primary. Nested GetDC calls share a surface lease; release/present only when
 * the outermost window-DC scope ends, preserving each nested GDI state. */
static HDC WINAPI window_get_dc(HWND hwnd)
{
    HRESULT hr;
    int state;
    if (!sqz_platform_window(hwnd) || !virtual_primary)
        return native_get_dc(hwnd);
    if (dc_depth == sizeof(dc_states) / sizeof(dc_states[0]))
        return NULL;
    if (!dc_depth)
    {
        dc_surface = virtual_primary;
        IDirectDrawSurface_AddRef(dc_surface);
        hr = IDirectDrawSurface_GetDC(dc_surface, &canvas_dc);
        if (FAILED(hr))
        {
            sqz_log("GDI CANVAS GetDC failed=%08lx", hr);
            IDirectDrawSurface_Release(dc_surface);
            dc_surface = NULL;
            return NULL;
        }
    }
    state = SaveDC(canvas_dc);
    if (!state)
    {
        if (!dc_depth)
        {
            IDirectDrawSurface_ReleaseDC(dc_surface, canvas_dc);
            IDirectDrawSurface_Release(dc_surface);
            dc_surface = NULL;
            canvas_dc = NULL;
        }
        return NULL;
    }
    dc_states[dc_depth++] = state;
    sqz_log("GDI CANVAS acquire hwnd=%p dc=%p depth=%u", hwnd, canvas_dc, dc_depth);
    return canvas_dc;
}

static int WINAPI window_release_dc(HWND hwnd, HDC dc)
{
    HRESULT hr;
    IDirectDrawSurface *surface;
    if (!sqz_platform_window(hwnd) || !dc_depth || dc != canvas_dc)
        return native_release_dc(hwnd, dc);
    RestoreDC(dc, dc_states[--dc_depth]);
    sqz_log("GDI CANVAS release dc=%p depth=%u", dc, dc_depth);
    if (dc_depth)
        return 1;
    surface = dc_surface;
    canvas_dc = NULL;
    dc_surface = NULL;
    hr = IDirectDrawSurface_ReleaseDC(surface, dc);
    IDirectDrawSurface_Release(surface);
    if (repaint_pending)
        request_repaint();
    return SUCCEEDED(hr);
}
typedef struct Dispatch
{
    HWND hwnd;
    UINT msg;
    WPARAM wp;
    LPARAM lp;
    LRESULT value;
    struct Dispatch *previous;
} Dispatch;
static DWORD dispatch_tls = TLS_OUT_OF_INDEXES;

static RECT content_rect(void)
{
    RECT client = {0};
    GetClientRect(game_hwnd, &client);
    return sqz_fit(client.right, client.bottom, initial_width, initial_height);
}

/* The game's primary is a virtual screen at (0,0). Only the OS cursor crosses
 * into physical desktop coordinates. Runtime/SDL imports remain unmodified. */
static BOOL WINAPI window_client_rect(HWND hwnd, RECT *rect)
{
    if (!sqz_platform_window(hwnd))
        return native_client_rect(hwnd, rect);
    if (!rect)
        return FALSE;
    SetRect(rect, 0, 0, initial_width, initial_height);
    return TRUE;
}
static BOOL WINAPI window_to_screen(HWND hwnd, POINT *point)
{
    if (!sqz_platform_window(hwnd))
        return native_to_screen(hwnd, point);
    return point != NULL;
}
static BOOL WINAPI window_to_client(HWND hwnd, POINT *point)
{
    if (!sqz_platform_window(hwnd))
        return native_to_client(hwnd, point);
    return point != NULL;
}
static BOOL WINAPI window_cursor(int x, int y)
{
    if (game_hwnd)
    {
        if (IsIconic(game_hwnd) || GetForegroundWindow() != game_hwnd)
            return TRUE;
        POINT point = sqz_to_native((POINT){x, y}, content_rect(), initial_width, initial_height);
        ClientToScreen(game_hwnd, &point);
        return native_cursor(point.x, point.y);
    }
    return native_cursor(x, y);
}
static HCURSOR WINAPI window_set_cursor(HCURSOR cursor)
{
    /* Original helper 0x4592f0 hides the hardware pointer with SetCursor(NULL).
     * Keep a visible native pointer in the SDL window as explicitly requested. */
    if (game_hwnd && !cursor)
        cursor = LoadCursorA(NULL, IDC_ARROW);
    return native_set_cursor(cursor);
}

BOOL sqz_platform_window(HWND hwnd)
{
    return game_hwnd && hwnd == game_hwnd;
}

static LRESULT WINAPI window_default(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    Dispatch *dispatch = TlsGetValue(dispatch_tls);
    if (dispatch && hwnd == dispatch->hwnd && msg == dispatch->msg && wp == dispatch->wp &&
        lp == dispatch->lp)
    {
        if (msg == WM_CLOSE)
            PostQuitMessage(0);
        return dispatch->value;
    }
    return native_default(hwnd, msg, wp, lp);
}

static LRESULT CALLBACK window_dispatch(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    Dispatch frame = {hwnd, msg, wp, lp, 0, TlsGetValue(dispatch_tls)};
    LRESULT result;
    if (msg == SQZ_REPAINT)
    {
        repaint_queued = FALSE;
        if (repaint_pending && !dc_depth && virtual_primary)
        {
            repaint_pending = FALSE;
            HRESULT hr = present_canvas();
            sqz_log("SDL REPAINT hr=%08lx", hr);
        }
        return 0;
    }
    /* SDL receives lifecycle messages even if a game callback suppresses default
     * handling. The original DefWindowProc import returns this cached result. */
    frame.value = CallWindowProcA(sdl_proc, hwnd, msg, wp, lp);
    /* The SDL window owns its native frame. The original fullscreen callback
     * can return zero/HTCLIENT for these messages, erasing SDL's resize grips
     * even though WS_THICKFRAME is present. Keep native border hit-testing,
     * track limits and non-client cursors outside the gameplay message path. */
    if (msg == WM_NCHITTEST || msg == WM_NCCALCSIZE || msg == WM_GETMINMAXINFO ||
        (msg == WM_SETCURSOR && LOWORD(lp) != HTCLIENT))
        return frame.value;
    if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST && msg != WM_MOUSEWHEEL &&
        msg != WM_MOUSEHWHEEL)
    {
        RECT content = content_rect();
        if (content.right > content.left && content.bottom > content.top)
        {
            POINT point = sqz_to_logical((POINT){(short)LOWORD(lp), (short)HIWORD(lp)}, content,
                                         initial_width, initial_height);
            lp = MAKELPARAM((short)point.x, (short)point.y);
            frame.lp = lp;
        }
    }
    TlsSetValue(dispatch_tls, &frame);
    result = CallWindowProcA(game_proc, hwnd, msg, wp, lp);
    TlsSetValue(dispatch_tls, frame.previous);
    if (msg == WM_SIZE || msg == WM_ACTIVATEAPP || msg == WM_DISPLAYCHANGE)
        sqz_log("WINDOW message=%04x wparam=%08lx lparam=%08lx", msg, (DWORD)wp, (DWORD)lp);
    if (msg == WM_SIZE || msg == WM_WINDOWPOSCHANGED || msg == WM_DISPLAYCHANGE ||
        msg == WM_PAINT || msg == WM_EXITSIZEMOVE || (msg == WM_ACTIVATEAPP && wp))
    {
        if (msg != WM_PAINT)
            refresh_clipping = TRUE;
        request_repaint();
    }
    return result;
}

static BOOL load_sdl(void)
{
    char path[MAX_PATH];
    HMODULE runtime, library;
    if (p_SDL_Init)
        return TRUE;
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            (LPCSTR)load_sdl, &runtime))
        return FALSE;
    if (!GetModuleFileNameA(runtime, path, sizeof(path)))
        return FALSE;
    char *slash = strrchr(path, '\\');
    if (!slash || slash - path > MAX_PATH - 10)
        return FALSE;
    strcpy(slash + 1, "SDL3.dll");
    library = LoadLibraryExA(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!library)
    {
        sqz_log("SDL load failed error=%lu", GetLastError());
        return FALSE;
    }
#define LOAD(name)                                                                                 \
    p_##name = (void *)GetProcAddress(library, #name);                                             \
    if (!p_##name)                                                                                 \
        return FALSE;
    SDL_FUNCTIONS(LOAD)
    p_SDL_SetMainReady();
    p_SDL_SetHint(SDL_HINT_WINDOWS_ENABLE_MESSAGELOOP, "0");
    if (!p_SDL_Init(SDL_INIT_VIDEO))
    {
        sqz_log("SDL init failed: %s", p_SDL_GetError());
        return FALSE;
    }
    return TRUE;
}

static HWND WINAPI window_create(DWORD exstyle, LPCSTR cls, LPCSTR title, DWORD style, int x, int y,
                                 int width, int height, HWND parent, HMENU menu, HINSTANCE instance,
                                 LPVOID param)
{
    WNDCLASSA original;
    if (creating || !enabled || parent || (uintptr_t)cls < 65536 || strcmp(cls, "Squeezils"))
        return native_create(exstyle, cls, title, style, x, y, width, height, parent, menu,
                             instance, param);
    if (!GetClassInfoA(instance, cls, &original) || !load_sdl())
        return NULL;
    creating = TRUE;
    /* The game's adjusted outer dimensions may include its old border. Preserve
     * its configured logical client size when replacing that native window. */
    RECT border = {0, 0, 0, 0};
    AdjustWindowRectEx(&border, style, FALSE, exstyle);
    initial_width = width - (border.right - border.left);
    initial_height = height - (border.bottom - border.top);
    if (initial_width < 64)
        initial_width = 640;
    if (initial_height < 64)
        initial_height = 480;
    sdl_window = p_SDL_CreateWindow(title, initial_width, initial_height,
                                    SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
    creating = FALSE;
    if (!sdl_window)
    {
        sqz_log("SDL create failed: %s", p_SDL_GetError());
        return NULL;
    }
    game_hwnd = p_SDL_GetPointerProperty(p_SDL_GetWindowProperties(sdl_window),
                                         SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    if (!game_hwnd)
    {
        p_SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
        return NULL;
    }
    game_proc = original.lpfnWndProc;
    sdl_proc = (WNDPROC)SetWindowLongPtrA(game_hwnd, GWLP_WNDPROC, (LONG_PTR)window_dispatch);
    sqz_log("SDL WINDOW created sdl=%p hwnd=%p client=%dx%d gameproc=%p sdlproc=%p", sdl_window,
            game_hwnd, initial_width, initial_height, game_proc, sdl_proc);
    return game_hwnd;
}

static BOOL WINAPI window_destroy(HWND hwnd)
{
    if (sqz_platform_window(hwnd) && sdl_window)
    {
        SDL_Window *window = sdl_window;
        sdl_window = NULL;
        sqz_log("SDL WINDOW destroy hwnd=%p", hwnd);
        p_SDL_DestroyWindow(window);
        game_hwnd = NULL;
        PostQuitMessage(0);
        return TRUE;
    }
    return native_destroy(hwnd);
}

static LONG WINAPI window_style(HWND hwnd, int index, LONG value)
{
    if (sqz_platform_window(hwnd) && index == GWL_STYLE)
    {
        value = (value & ~WS_POPUP) | WS_OVERLAPPEDWINDOW;
        /* Renderer setup rewrites its old fullscreen style on every level.
         * Minimize/maximize belong to the user's native window placement. */
        const LONG placement = WS_MAXIMIZE | WS_MINIMIZE;
        value = (value & ~placement) | (GetWindowLongA(hwnd, GWL_STYLE) & placement);
    }
    return native_style(hwnd, index, value);
}

static BOOL WINAPI window_position(HWND hwnd, HWND after, int x, int y, int width, int height,
                                   UINT flags)
{
    /* Only the EXE import is routed here. SDL and Windows still position and
     * resize the window directly in response to border/title-bar actions. */
    if (sqz_platform_window(hwnd))
        flags |= SWP_NOMOVE | SWP_NOSIZE;
    return native_position(hwnd, after, x, y, width, height, flags);
}

static BOOL WINAPI window_show(HWND hwnd, int command)
{
    if (sqz_platform_window(hwnd) && command == SW_SHOWNORMAL && IsZoomed(hwnd))
        command = SW_SHOW;
    return native_show(hwnd, command);
}

static BOOL WINAPI window_peek(MSG *message, HWND hwnd, UINT low, UINT high, UINT flags)
{
    if (sdl_window && !pumping)
    {
        pumping = TRUE;
        p_SDL_PumpEvents();
        /* Win32 remains the gameplay input source in this checkpoint. */
        p_SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST);
        pumping = FALSE;
    }
    return native_peek(message, hwnd, low, high, flags);
}

void sqz_platform_install(void)
{
    /* Loader-lock work is only import-table memory routing. No SDL, file IO,
     * LoadLibrary, logging, window creation or native GUI calls happen here. */
    char value[8] = "";
    BYTE *base = (BYTE *)GetModuleHandleA(NULL);
    IMAGE_DOS_HEADER *dos = (void *)base;
    IMAGE_NT_HEADERS32 *nt;
    IMAGE_IMPORT_DESCRIPTOR *imports;
    GetEnvironmentVariableA("SQZ_SDL_WINDOW", value, sizeof(value));
    enabled = strcmp(value, "0") != 0;
    if (!base || dos->e_magic != IMAGE_DOS_SIGNATURE)
        return;
    if (enabled)
    {
        dispatch_tls = TlsAlloc();
        if (dispatch_tls == TLS_OUT_OF_INDEXES)
            enabled = FALSE;
    }
    nt = (void *)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE || nt->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
        return;
    imports =
        (void *)(base +
                 nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    for (; imports->Name; ++imports)
    {
        if (!imports->OriginalFirstThunk)
            continue;
        IMAGE_THUNK_DATA32 *names = (void *)(base + imports->OriginalFirstThunk);
        IMAGE_THUNK_DATA32 *slots = (void *)(base + imports->FirstThunk);
        for (; names->u1.AddressOfData; ++names, ++slots)
        {
            void *replacement = NULL;
            void **original = NULL;
            DWORD old, unused;
            if (IMAGE_SNAP_BY_ORDINAL32(names->u1.Ordinal))
                continue;
            const char *name =
                (char *)((IMAGE_IMPORT_BY_NAME *)(base + names->u1.AddressOfData))->Name;
#define ROUTE(api, hook, save)                                                                     \
    if (!strcmp(name, api))                                                                        \
    {                                                                                              \
        replacement = hook;                                                                        \
        original = (void **)&save;                                                                 \
    }
            static void *native_rm_factory;
            static void *native_file_create;
            ROUTE("Direct3DRMCreate", sqz_create_rm, native_rm_factory)
            ROUTE("CreateFileA", sqz_save_create_file, native_file_create)
            if (enabled)
            {
                ROUTE("CreateWindowExA", window_create, native_create)
                ROUTE("DestroyWindow", window_destroy, native_destroy)
                ROUTE("DefWindowProcA", window_default, native_default)
                ROUTE("PeekMessageA", window_peek, native_peek)
                ROUTE("SetWindowLongA", window_style, native_style)
                ROUTE("SetWindowPos", window_position, native_position)
                ROUTE("ShowWindow", window_show, native_show)
                ROUTE("GetClientRect", window_client_rect, native_client_rect)
                ROUTE("ClientToScreen", window_to_screen, native_to_screen)
                ROUTE("ScreenToClient", window_to_client, native_to_client)
                ROUTE("SetCursorPos", window_cursor, native_cursor)
                ROUTE("SetCursor", window_set_cursor, native_set_cursor)
                ROUTE("GetDC", window_get_dc, native_get_dc)
                ROUTE("ReleaseDC", window_release_dc, native_release_dc)
                ROUTE("GetDeviceCaps", canvas_device_caps, native_device_caps)
                ROUTE("CreateFontIndirectA", diagnostic_font_indirect, native_font_indirect)
            }
            if (!replacement)
                continue;
            if (!VirtualProtect(&slots->u1.Function, sizeof(DWORD), PAGE_READWRITE, &old))
                continue;
            *original = (void *)(uintptr_t)slots->u1.Function;
            InterlockedExchange((LONG *)&slots->u1.Function, (LONG)(uintptr_t)replacement);
            VirtualProtect(&slots->u1.Function, sizeof(DWORD), old, &unused);
        }
    }
}

HRESULT sqz_cooperative(const IDirectDrawVtbl *original, IDirectDraw *self, HWND hwnd, DWORD flags)
{
    if (sqz_platform_window(hwnd))
    {
        sqz_log("SDL COOPERATIVE requested=%08lx effective=DDSCL_NORMAL hwnd=%p", flags, hwnd);
        flags = DDSCL_NORMAL;
    }
    return original->SetCooperativeLevel(self, hwnd, flags);
}

HRESULT sqz_display_mode(const IDirectDrawVtbl *original, IDirectDraw *self, DWORD width,
                         DWORD height, DWORD bpp)
{
    if (game_hwnd)
    {
        initial_width = (int)width;
        initial_height = (int)height;
        sqz_log("SDL DISPLAY virtual=%lux%lu bpp=%lu (desktop mode preserved)", width, height, bpp);
        sqz_log("DIAG DISPLAY requested=%lux%lu bpp=%lu physical_change=suppressed", width, height,
                bpp);
        return DD_OK;
    }
    return original->SetDisplayMode(self, width, height, bpp);
}

void sqz_surface_created(IDirectDraw *ddraw, IDirectDrawSurface *surface)
{
    DDSCAPS caps = {0};
    IDirectDrawClipper *clipper;
    HRESULT hr;
    if (!game_hwnd || !surface || FAILED(IDirectDrawSurface_GetCaps(surface, &caps)) ||
        !(caps.dwCaps & DDSCAPS_PRIMARYSURFACE))
        return;
    hr = IDirectDraw_CreateClipper(ddraw, 0, &clipper, NULL);
    if (SUCCEEDED(hr))
    {
        hr = IDirectDrawClipper_SetHWnd(clipper, 0, game_hwnd);
        if (SUCCEEDED(hr))
            hr = IDirectDrawSurface_SetClipper(surface, clipper);
        IDirectDrawClipper_Release(clipper);
    }
    sqz_log("SDL CLIPPER primary=%p hwnd=%p hr=%08lx", surface, game_hwnd, hr);
}

HRESULT sqz_create_surface(const IDirectDrawVtbl *original, IDirectDraw *ddraw, DDSURFACEDESC *desc,
                           IDirectDrawSurface **out, IUnknown *outer)
{
    DDSURFACEDESC canvas;
    HRESULT hr;
    if (!game_hwnd || !desc || !(desc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE))
        return original->CreateSurface(ddraw, desc, out, outer);
    if (native_primary || !out || outer)
        return DDERR_INVALIDPARAMS;
    hr = original->CreateSurface(ddraw, desc, &native_primary, NULL);
    if (FAILED(hr))
        return hr;
    sqz_hook(native_primary, "IDirectDrawSurface");
    sqz_surface_created(ddraw, native_primary);
    memset(&canvas, 0, sizeof(canvas));
    canvas.dwSize = sizeof(canvas);
    canvas.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    canvas.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    canvas.dwWidth = initial_width;
    canvas.dwHeight = initial_height;
    hr = original->CreateSurface(ddraw, &canvas, &virtual_primary, NULL);
    if (FAILED(hr))
    {
        IDirectDrawSurface_Release(native_primary);
        native_primary = NULL;
        return hr;
    }
    *out = virtual_primary;
    sqz_log("SDL CANVAS virtual=%p native=%p size=%dx%d", virtual_primary, native_primary,
            initial_width, initial_height);
    return hr;
}

HRESULT sqz_get_caps(const IDirectDrawVtbl *original, IDirectDraw *self, DDCAPS *driver,
                     DDCAPS *software)
{
    HRESULT hr = original->GetCaps(self, driver, software);
    /* The game's depth selection cannot handle an empty legacy bit-depth mask.
     * This replaces the supplied compatibility wrapper's 16-bit Z-buffer fix. */
    DDCAPS *caps[] = {driver, software};
    if (SUCCEEDED(hr) && enabled)
        for (unsigned i = 0; i < 2; ++i)
            if (caps[i] &&
                caps[i]->dwSize >= offsetof(DDCAPS, dwZBufferBitDepths) + sizeof(DWORD) &&
                !caps[i]->dwZBufferBitDepths)
            {
                caps[i]->dwZBufferBitDepths = DDBD_16;
                sqz_log("COMPAT GetCaps reports 16-bit Z buffer");
            }
    return hr;
}

static HRESULT present_canvas(void)
{
    static BOOL reported_gdi;
    RECT destination, client, source = {0, 0, initial_width, initial_height};
    POINT origin = {0, 0};
    DDBLTFX fill = {0};
    HRESULT hr;
    const IDirectDrawSurfaceVtbl *original;
    if (presenting || dc_depth || !game_hwnd || !native_primary || !virtual_primary ||
        IsIconic(game_hwnd))
        return DD_OK;
    GetClientRect(game_hwnd, &client);
    if (!client.right || !client.bottom)
        return DD_OK;
    destination = content_rect();
    ClientToScreen(game_hwnd, &origin);
    OffsetRect(&destination, origin.x, origin.y);
    OffsetRect(&client, origin.x, origin.y);
    original = sqz_original(native_primary);
    /* DirectDraw's 16BITCOLOR compatibility stretch becomes very expensive at
     * desktop resolutions. Keep rendering into the same native canvas, but let
     * GDI scale its DC directly into the SDL HWND. These are runtime imports,
     * deliberately bypassing the game's redirected GetDC/ReleaseDC pair. */
    const IDirectDrawSurfaceVtbl *canvas = sqz_original(virtual_primary);
    HDC source_dc = NULL, target_dc = NULL;
    presenting = TRUE;
    hr = canvas->GetDC(virtual_primary, &source_dc);
    if (SUCCEEDED(hr))
        target_dc = GetDC(game_hwnd);
    if (target_dc)
    {
        RECT fitted = content_rect(), local = {0};
        GetClientRect(game_hwnd, &local);
        SetStretchBltMode(target_dc, COLORONCOLOR);
        BOOL copied = StretchBlt(target_dc, fitted.left, fitted.top, fitted.right - fitted.left,
                                 fitted.bottom - fitted.top, source_dc, 0, 0, initial_width,
                                 initial_height, SRCCOPY);
        if (copied)
        {
            RECT bars[] = {{0, 0, local.right, fitted.top},
                           {0, fitted.bottom, local.right, local.bottom},
                           {0, fitted.top, fitted.left, fitted.bottom},
                           {fitted.right, fitted.top, local.right, fitted.bottom}};
            for (unsigned i = 0; i < sizeof(bars) / sizeof(bars[0]); ++i)
                if (bars[i].right > bars[i].left && bars[i].bottom > bars[i].top)
                    FillRect(target_dc, &bars[i], GetStockObject(BLACK_BRUSH));
        }
        ReleaseDC(game_hwnd, target_dc);
        hr = canvas->ReleaseDC(virtual_primary, source_dc);
        presenting = FALSE;
        if (copied && SUCCEEDED(hr))
        {
            if (!reported_gdi)
            {
                sqz_log("SDL PRESENT backend=GDI (native canvas; DirectDraw fallback)");
                reported_gdi = TRUE;
            }
            return DD_OK;
        }
    }
    else
    {
        if (source_dc)
            canvas->ReleaseDC(virtual_primary, source_dc);
        presenting = FALSE;
    }
    sqz_log("SDL PRESENT GDI unavailable; falling back to DirectDraw");
    if (refresh_clipping)
    {
        IDirectDrawClipper *clipper;
        if (SUCCEEDED(original->GetClipper(native_primary, &clipper)))
        {
            HRESULT clip_hr = IDirectDrawClipper_SetHWnd(clipper, 0, game_hwnd);
            sqz_log("SDL CLIPPER refresh=%08lx", clip_hr);
            IDirectDrawClipper_Release(clipper);
            if (SUCCEEDED(clip_hr))
                refresh_clipping = FALSE;
        }
    }
    if (original->IsLost(native_primary) == DDERR_SURFACELOST)
    {
        hr = original->Restore(native_primary);
        sqz_log("SDL PRESENT restore=%08lx", hr);
        if (FAILED(hr))
            return hr;
    }
    presenting = TRUE;
    fill.dwSize = sizeof(fill);
    /* Never expose a black intermediate frame. Only letterbox bars need fill. */
    RECT bars[] = {{client.left, client.top, client.right, destination.top},
                   {client.left, destination.bottom, client.right, client.bottom},
                   {client.left, destination.top, destination.left, destination.bottom},
                   {destination.right, destination.top, client.right, destination.bottom}};
    for (unsigned i = 0; i < sizeof(bars) / sizeof(bars[0]); ++i)
        if (bars[i].right > bars[i].left && bars[i].bottom > bars[i].top)
            original->Blt(native_primary, &bars[i], NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT,
                          &fill);
    hr = original->Blt(native_primary, &destination, virtual_primary, &source, DDBLT_WAIT, NULL);
    presenting = FALSE;
    if (FAILED(hr))
        sqz_log("SDL PRESENT failed=%08lx", hr);
    return hr;
}

void sqz_surface_released(void *self, ULONG refs)
{
    if (!refs && self == virtual_primary)
    {
        virtual_primary = NULL;
        if (native_primary)
        {
            IDirectDrawSurface *surface = native_primary;
            native_primary = NULL;
            IDirectDrawSurface_Release(surface);
        }
    }
}

HRESULT sqz_release_dc(const IDirectDrawSurfaceVtbl *original, IDirectDrawSurface *self, HDC dc)
{
    HRESULT hr = original->ReleaseDC(self, dc);
    if (SUCCEEDED(hr) && self == virtual_primary)
        request_repaint();
    return hr;
}

HRESULT sqz_surface_blt(const IDirectDrawSurfaceVtbl *original, IDirectDrawSurface *self,
                        RECT *destination, IDirectDrawSurface *source, RECT *source_rect,
                        DWORD flags, DDBLTFX *effects)
{
    HRESULT hr = original->Blt(self, destination, source, source_rect, flags, effects);
    if (hr == DDERR_SURFACELOST)
    {
        HRESULT restore = original->Restore(self), source_restore = DD_OK;
        if (source && IDirectDrawSurface_IsLost(source) == DDERR_SURFACELOST)
            source_restore = IDirectDrawSurface_Restore(source);
        sqz_log("SURFACE LOST blt dst=%p src=%p restore=%08lx source_restore=%08lx flags=%08lx",
                self, source, restore, source_restore, flags);
        if (SUCCEEDED(restore) && SUCCEEDED(source_restore))
            hr = original->Blt(self, destination, source, source_rect, flags, effects);
        sqz_log("SURFACE RETRY blt hr=%08lx", hr);
    }
    if (SUCCEEDED(hr) && self == virtual_primary)
        request_repaint();
    return hr;
}
