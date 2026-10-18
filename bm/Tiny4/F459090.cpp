// 0x459090 (ret 0xc): mouse message handler of the pointer widget; returns -1 = handled/no button, 1 = left, 2 = right.
#include <windows.h>
struct Pointer459 {
    int pos;                    // 0x00: packed (x,y) shorts
    char pad[0x20 - 4];
    int captured;               // 0x20
    void Clamp(const void *pt); // 0x459220
    int OnMouse(unsigned msg, unsigned wparam, int lparam);
};
int Pointer459::OnMouse(unsigned msg, unsigned wparam, int lparam)
{
    switch (msg) {
    case WM_MOUSEMOVE:
        pos = lparam;
        Clamp(this);
        return -1;
    case WM_LBUTTONDOWN:
        return 1;
    case WM_LBUTTONUP:
        return 1;
    case WM_RBUTTONDOWN:
        return 2;
    case WM_RBUTTONUP:
        return 2;
    case WM_CAPTURECHANGED:
        captured = 0;
        return -1;
    }
    return -1;
}
