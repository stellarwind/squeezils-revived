// Mouse cursor and capture helpers (SQZ.EXE 0x4592f0 and 0x4592a0).
#include <windows.h>
#include "compat41.h"

struct AppWnd { char pad[0x18]; HWND hwnd; };
extern AppWnd *g_app;                // 0x4c75dc

struct Mouse {
    char pad[0x20];
    int captured;                    // 0x20
    void Center(int how);            // 0x4591b0
    void Capture(int on);            // 0x4592a0
    void ShowCursor(int show);       // 0x4592f0
};

void Mouse::ShowCursor(int show)
{
    if (show)
        SetCursor(LoadCursor(0, IDC_ARROW));
    else
        SetCursor(0);
}

void Mouse::Capture(int on)
{
    if (on) {
        if (!captured) {
            SetCapture(g_app->hwnd);
            captured = 1;
            Center(2);
            return;
        }
    }
    if (!on) {
        ReleaseCapture();
        captured = 0;
    }
}
