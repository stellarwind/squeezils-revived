#include "runtime.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "geometry.h"
static LRESULT CALLBACK procedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    /* Reproduce a legacy callback swallowing the SDL border hit-test. */
    if (msg == WM_NCHITTEST)
        return HTCLIENT;
    return DefWindowProcA(hwnd,msg,wp,lp);
}
static DWORD pixel(IDirectDrawSurface *surface, int x, int y)
{
    DDSURFACEDESC locked = {0};
    DWORD value = 0;
    locked.dwSize = sizeof(locked);
    assert(SUCCEEDED(IDirectDrawSurface_Lock(surface,NULL,&locked,DDLOCK_WAIT,NULL)));
    unsigned bytes = locked.ddpfPixelFormat.dwRGBBitCount / 8;
    assert(bytes == 2 || bytes == 4);
    memcpy(&value,(BYTE *)locked.lpSurface+y*locked.lPitch+x*bytes,bytes);
    assert(SUCCEEDED(IDirectDrawSurface_Unlock(surface,locked.lpSurface)));
    return value;
}
static unsigned timer_ink(IDirectDrawSurface *surface)
{
    DDSURFACEDESC locked = {0};
    unsigned ink = 0;
    locked.dwSize = sizeof(locked);
    assert(SUCCEEDED(IDirectDrawSurface_Lock(surface,NULL,&locked,DDLOCK_WAIT,NULL)));
    unsigned bytes = locked.ddpfPixelFormat.dwRGBBitCount / 8;
    for (unsigned y=366; y<390; ++y) for (unsigned x=300; x<370; ++x) {
        DWORD value = 0;
        memcpy(&value,(BYTE *)locked.lpSurface+y*locked.lPitch+x*bytes,bytes);
        if (value & 0x00ffffff) ++ink;
    }
    assert(SUCCEEDED(IDirectDrawSurface_Unlock(surface,locked.lpSurface)));
    return ink;
}
static void messages(void)
{
    MSG message;
    for (unsigned i=0; i<128 && PeekMessageA(&message,NULL,0,0,PM_REMOVE); ++i) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}
int main(void)
{
    HINSTANCE instance = GetModuleHandleA(NULL);
    WNDCLASSA cls = {.lpfnWndProc=procedure,.hInstance=instance,.lpszClassName="Squeezils"};
    HWND hwnd;
    IDirectDraw *dd;
    IDirectDrawSurface *primary, *snapshot;
    DDSURFACEDESC desc = {0}, actual = {0};
    DDBLTFX fill = {0};
    RECT patch = {10,10,30,30}, rect, capture = {0,0,40,40};
    assert(RegisterClassA(&cls));
    hwnd = CreateWindowExA(0,"Squeezils","Hidden SDL forwarding test",WS_OVERLAPPEDWINDOW,
                           0,0,640,480,NULL,NULL,instance,NULL);
    assert(hwnd);
    BOOL (WINAPI *user_position)(HWND, HWND, int, int, int, int, UINT) =
        (void *)GetProcAddress(GetModuleHandleA("user32.dll"), "SetWindowPos");
    assert(user_position);
    SetCursor(NULL);
    assert(GetCursor() == LoadCursorA(NULL,IDC_ARROW));
    assert(GetWindowLongA(hwnd,GWL_STYLE) & WS_THICKFRAME);
    RECT border_rect;
    assert(GetWindowRect(hwnd,&border_rect));
    LPARAM edge_point=MAKELPARAM(border_rect.right-2,(border_rect.top+border_rect.bottom)/2);
    assert(SendMessageA(hwnd,WM_NCHITTEST,0,edge_point)==HTRIGHT);
    assert(SUCCEEDED(DirectDrawCreate(NULL,&dd,NULL)));
    assert(SUCCEEDED(IDirectDraw_SetCooperativeLevel(dd,hwnd,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN)));
    assert(SUCCEEDED(IDirectDraw_SetDisplayMode(dd,640,480,16)));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS;
    desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    assert(SUCCEEDED(IDirectDraw_CreateSurface(dd,&desc,&primary,NULL)));
    actual.dwSize = sizeof(actual);
    assert(SUCCEEDED(IDirectDrawSurface_GetSurfaceDesc(primary,&actual)));
    assert(actual.dwWidth == 640 && actual.dwHeight == 480);
    DDCAPS driver = {.dwSize=sizeof(driver)}, software = {.dwSize=sizeof(software)};
    assert(SUCCEEDED(IDirectDraw_GetCaps(dd,&driver,&software)));
    assert(driver.dwZBufferBitDepths && software.dwZBufferBitDepths);
    fill.dwSize = sizeof(fill);
    assert(SUCCEEDED(IDirectDrawSurface_Blt(primary,NULL,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,&fill)));
    fill.dwFillColor = 0xff;
    assert(SUCCEEDED(IDirectDrawSurface_Blt(primary,&patch,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,&fill)));
    assert(pixel(primary,20,20) == 0xff && pixel(primary,0,0) == 0);
    desc.dwFlags |= DDSD_WIDTH|DDSD_HEIGHT;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;
    desc.dwWidth = desc.dwHeight = 40;
    assert(SUCCEEDED(IDirectDraw_CreateSurface(dd,&desc,&snapshot,NULL)));
    assert(SUCCEEDED(IDirectDrawSurface_Blt(snapshot,NULL,primary,&capture,DDBLT_WAIT,NULL)));
    assert(pixel(snapshot,20,20) == 0xff && pixel(snapshot,0,0) == 0);
    /* Same nested GetDC and ExtTextOut pattern as timer 0x42e0e0/0x4753d0. */
    LOGFONTA font_spec = {.lfHeight=-14,.lfWeight=FW_NORMAL,.lfCharSet=DEFAULT_CHARSET,
                         .lfFaceName="Arial"};
    HFONT font = CreateFontIndirectA(&font_spec);
    assert(font);
    HDC outer_dc = GetDC(hwnd);
    assert(outer_dc);
    assert(GetDeviceCaps(outer_dc,LOGPIXELSY) == 96);
    assert(GetDeviceCaps(outer_dc,LOGPIXELSX) == 96);
    HGDIOBJ old_font = SelectObject(outer_dc,font);
    SetTextColor(outer_dc,RGB(255,0,0));
    HDC text_dc = GetDC(hwnd);
    assert(text_dc);
    SelectObject(text_dc,font);
    SetTextColor(text_dc,RGB(255,255,255));
    SetBkMode(text_dc,TRANSPARENT);
    RECT text_box = {300,366,370,390};
    assert(ExtTextOutA(text_dc,300,366,ETO_CLIPPED,&text_box,"1:23",4,NULL));
    assert(ReleaseDC(hwnd,text_dc));
    assert(GetTextColor(outer_dc) == RGB(255,0,0));
    SelectObject(outer_dc,old_font);
    assert(ReleaseDC(hwnd,outer_dc));
    DeleteObject(font);
    unsigned ink = timer_ink(primary);
    assert(ink > 0);
    messages();
    assert(user_position(hwnd,NULL,0,0,900,700,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE));
    messages();
    assert(GetClientRect(hwnd,&rect) && rect.right == 640 && rect.bottom == 480);
    assert(GetWindowRect(hwnd,&rect) && rect.right-rect.left == 900 && rect.bottom-rect.top == 700);
    RECT placement = rect;
    /* Original renderer 0x4764c0/0x4765d0 repeats this at level changes. */
    LONG style = GetWindowLongA(hwnd,GWL_STYLE);
    SetWindowLongA(hwnd,GWL_STYLE,style & ~WS_OVERLAPPEDWINDOW);
    assert(SetWindowPos(hwnd,NULL,0,0,640,480,SWP_NOZORDER|SWP_FRAMECHANGED));
    assert(GetWindowRect(hwnd,&rect) && EqualRect(&rect,&placement));
    /* Exercise maximize-state preservation without showing the fixture. */
    SetWindowLongW(hwnd,GWL_STYLE,style | WS_MAXIMIZE);
    assert(IsZoomed(hwnd));
    SetWindowLongA(hwnd,GWL_STYLE,style);
    assert(IsZoomed(hwnd));
    SetWindowLongW(hwnd,GWL_STYLE,style);
    assert(SUCCEEDED(IDirectDrawSurface_GetSurfaceDesc(primary,&actual)));
    assert(actual.dwWidth == 640 && actual.dwHeight == 480);
    assert(pixel(primary,20,20) == 0xff && pixel(primary,0,0) == 0);
    assert(timer_ink(primary) == ink);
    assert(user_position(hwnd,NULL,0,0,640,480,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE));
    messages();
    assert(timer_ink(primary) == ink);
    assert(InvalidateRect(hwnd,NULL,FALSE));
    UpdateWindow(hwnd);
    messages();
    IDirectDrawSurface_Release(snapshot);
    IDirectDrawSurface_Release(primary);
    IDirectDraw_Release(dd);
    assert(DestroyWindow(hwnd));
    puts("PASS: SDL HWND, canvas pixels/snapshots, nested GDI timer text, resize/restore without UI action, paired teardown");
    return 0;
}
