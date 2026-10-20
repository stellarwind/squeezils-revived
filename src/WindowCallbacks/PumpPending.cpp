// WindowCallbacks::PumpPending, SQZ.EXE 0x477fd0.
#include <windows.h>
#include "WindowCallbacks/WindowCallbacks.h"

int WindowCallbacks::PumpPending()
{
    MSG message;
    int processed = 0;
    while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT) {
            quitStatus = message.wParam;
            return 0;
        }
        if (!TranslateAcceleratorA(message.hwnd, (HACCEL)accelerators, &message)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        ++processed;
    }
    return processed ? 2 : 1;
}
