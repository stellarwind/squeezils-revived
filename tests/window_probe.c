#include "runtime.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static LRESULT CALLBACK procedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
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
    SetCursor(NULL);
    assert(GetCursor() == LoadCursorA(NULL,IDC_ARROW));
    assert(GetWindowLongA(hwnd,GWL_STYLE) & WS_THICKFRAME);
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
    assert(SetWindowPos(hwnd,NULL,0,0,900,700,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE));
    assert(GetClientRect(hwnd,&rect) && rect.right == 640 && rect.bottom == 480);
    assert(GetWindowRect(hwnd,&rect) && rect.right-rect.left == 900 && rect.bottom-rect.top == 700);
    assert(SUCCEEDED(IDirectDrawSurface_GetSurfaceDesc(primary,&actual)));
    assert(actual.dwWidth == 640 && actual.dwHeight == 480);
    assert(pixel(primary,20,20) == 0xff && pixel(primary,0,0) == 0);
    IDirectDrawSurface_Release(snapshot);
    IDirectDrawSurface_Release(primary);
    IDirectDraw_Release(dd);
    assert(DestroyWindow(hwnd));
    puts("PASS: SDL-created resizable HWND, virtual primary pixels/snapshots, logical size survives physical resize, paired teardown");
    return 0;
}
