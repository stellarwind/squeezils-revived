// Splash::Pump, SQZ.EXE 0x44eeb0.
#include <windows.h>
#include "Ui/Ui.h"

struct SplashPumpRenderer {
    char pad[0x18];
    HWND hwnd;
};
extern SplashPumpRenderer *gSplashPumpRenderer; // 0x4c75dc

void Splash::Pump()
{
    MSG message;
    if (PeekMessageA(&message, gSplashPumpRenderer->hwnd, 0, 0, PM_REMOVE)) {
        ValidateRect(gSplashPumpRenderer->hwnd, 0);
        Show();
        step = 0;
    }
}
