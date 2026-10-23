/* Exercise the actual import-hook implementation without changing OS settings. */
#include "../src/platform.c"
#include <assert.h>
#include <stdarg.h>

void *sqz_original(void *self)
{
    return *(void **)self;
}
void sqz_hook(void *self, const char *name)
{
    (void)self;
    (void)name;
}
HRESULT WINAPI sqz_create_rm(IDirect3DRM **out)
{
    (void)out;
    return E_NOTIMPL;
}

static int host_dpi;
static int show_command;
static BOOL WINAPI host_show(HWND hwnd, int command)
{
    (void)hwnd;
    show_command = command;
    return FALSE;
}
static int WINAPI host_caps(HDC dc, int index)
{
    (void)dc;
    if (index == LOGPIXELSX || index == LOGPIXELSY)
        return host_dpi;
    return 32;
}

int main(void)
{
    native_device_caps = host_caps;
    canvas_dc = CreateCompatibleDC(NULL);
    HDC other = CreateCompatibleDC(NULL);
    assert(canvas_dc && other);
    game_hwnd =
        CreateWindowExA(0, "STATIC", "DPI test", WS_POPUP, 0, 0, 64, 64, NULL, NULL, NULL, NULL);
    assert(game_hwnd);
    native_show = host_show;
    LONG original_style = GetWindowLongA(game_hwnd, GWL_STYLE);
    SetWindowLongW(game_hwnd, GWL_STYLE, original_style | WS_MAXIMIZE);
    window_show(game_hwnd, SW_SHOWNORMAL);
    assert(show_command == SW_SHOW);
    window_show(NULL, SW_SHOWNORMAL);
    assert(show_command == SW_SHOWNORMAL);
    window_show(game_hwnd, SW_HIDE);
    assert(show_command == SW_HIDE);
    SetWindowLongW(game_hwnd, GWL_STYLE, original_style);
    window_show(game_hwnd, SW_SHOWNORMAL);
    assert(show_command == SW_SHOWNORMAL);
    HDC early_window_dc = GetDC(game_hwnd);
    assert(early_window_dc);
    SIZE baseline = {0};
    const int dpis[] = {96, 144, 192, 288};
    for (unsigned i = 0; i < sizeof(dpis) / sizeof(dpis[0]); ++i)
    {
        host_dpi = dpis[i];
        assert(canvas_device_caps(canvas_dc, LOGPIXELSY) == 96);
        assert(canvas_device_caps(canvas_dc, LOGPIXELSX) == 96);
        assert(canvas_device_caps(early_window_dc, LOGPIXELSY) == 96);
        assert(canvas_device_caps(other, LOGPIXELSY) == host_dpi);
        assert(canvas_device_caps(canvas_dc, BITSPIXEL) == 32);
        LOGFONTA spec = {.lfHeight = -MulDiv(12, canvas_device_caps(canvas_dc, LOGPIXELSY), 72),
                         .lfWeight = FW_BOLD,
                         .lfCharSet = DEFAULT_CHARSET};
        strcpy(spec.lfFaceName, "Arial");
        assert(spec.lfHeight == -16);
        HFONT font = CreateFontIndirectA(&spec);
        assert(font);
        HGDIOBJ old = SelectObject(canvas_dc, font);
        SIZE size;
        assert(GetTextExtentPoint32A(canvas_dc, "5:59 Hey! That", 14, &size));
        if (!i)
            baseline = size;
        assert(size.cx == baseline.cx && size.cy == baseline.cy);
        SelectObject(canvas_dc, old);
        DeleteObject(font);
    }
    ReleaseDC(game_hwnd, early_window_dc);
    DestroyWindow(game_hwnd);
    game_hwnd = NULL;
    DeleteDC(other);
    DeleteDC(canvas_dc);
    canvas_dc = NULL;
    puts("PASS: HUD font metrics stay fixed at 100/150/200/300% DPI; early window DC covered; "
         "unrelated DC and pixel depth preserved");
    return 0;
}
