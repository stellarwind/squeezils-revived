// 0x44ef30: draw progress-bar tiles after pumping window messages.
// 1 spare declaration (register allocation, see padscan.sh).
void PadDecl1();
#include <windows.h>
#include "Bitmap/Bitmap.h"
#include "Ui/Splash.h"

struct SplashProgressRenderer {
    char pad0[0x0c];
    int fullscreen;
    char pad10[8];
    HWND hwnd;
    char pad1c[0x994 - 0x1c];
    IDirectDrawSurface *primary;
};
extern SplashProgressRenderer *gSplashProgressRenderer; // 0x4c75dc

void Splash::DrawProgress()
{
    if (!bar || step >= maxSteps)
        return;
    int oldStep = step;
    if (progressMode == -1) {
        step = maxSteps;
        return;
    }
    if (!gSplashProgressRenderer->fullscreen)
        Pump();
    if ((float)step / (float)maxSteps < progress) {
        POINT origin = { 0, 0 };
        if (!gSplashProgressRenderer->fullscreen)
            ClientToScreen(gSplashProgressRenderer->hwnd, &origin);
        IDirectDrawSurface *surface = gSplashProgressRenderer->primary;
        while (step <= oldStep) {
            int width = bar->GetWidth();
            bar->DrawToSurfaceAt(surface, width * step + barX + origin.x,
                origin.y + barY, 0, 0, 1);
            step++;
        }
    }
}
