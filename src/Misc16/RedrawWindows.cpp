// RedrawWindowsAbove, SQZ.EXE 0x43ee60.
#include <windows.h>

struct RedrawRenderer {
    char pad[0x18];
    HWND hwnd;
};
extern RedrawRenderer *gRedrawRenderer; // 0x4c75dc

int RedrawWindowsAbove()
{
    HWND window = gRedrawRenderer->hwnd;
    while (window && (window = GetWindow(window, GW_HWNDPREV)) != 0)
        RedrawWindow(window, 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_FRAME);
    return 1;
}
