// Splash::Show, SQZ.EXE 0x44edd0.
#include <windows.h>
#include "Bitmap/Bitmap.h"
#include "Ui/Splash.h"

struct SplashShowRenderer {
    char pad0[0x0c];
    int fullscreen;
    char pad10[8];
    HWND hwnd;
    char pad1c[0x994 - 0x1c];
    IDirectDrawSurface *primary;
};
extern SplashShowRenderer *gSplashShowRenderer; // 0x4c75dc

void Splash::Show()
{
    if (!picture)
        return;
    shown = 1;
    POINT origin = { 0, 0 };
    if (!gSplashShowRenderer->fullscreen)
        ClientToScreen(gSplashShowRenderer->hwnd, &origin);
    picture->DrawToSurfaceAt(gSplashShowRenderer->primary,
        origin.x, origin.y, 0, 0, 0);
}
