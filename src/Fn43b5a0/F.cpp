// 0x43b5a0: input-key handler of a two-axis controller; cases press/release the four directions, 0x15 fires a callback, 0x17 toggles.
#include <windows.h>
#include <mmsystem.h>
struct Callback { virtual void Fire(); };
void SetFlag(int on);                                   // 0x43ed80
struct Axis { DWORD down, up; int dir; };
struct Controller {
    char pad[0x18];
    Callback *callback;     // 0x18
    char pad2[0x30 - 0x1c];
    Axis axisA;             // 0x30
    int heldA;              // 0x3c
    float valueA;           // 0x40
    Axis axisB;             // 0x44
    int heldB;              // 0x50
    float valueB;           // 0x54
    char pad3[0x78 - 0x58];
    int toggle;             // 0x78
    int Key(int code, int pressed);
};
int Controller::Key(int code, int pressed)
{
    int ok = 1;
    switch (code) {
    case 0:
    case 0xc: {
        if (pressed)
            valueA = 1.0f;
        if (heldA && pressed)
            return 0;
        Axis *a = &axisA;
        heldA = pressed;
        a->dir = 1;
        if (pressed) {
            a->down = timeGetTime();
            break;
        }
        a->up = timeGetTime();
        break;
    }
    case 1:
    case 0xd: {
        if (pressed)
            valueA = -1.0f;
        if (heldA && pressed)
            return 0;
        Axis *a = &axisA;
        heldA = pressed;
        a->dir = -1;
        if (pressed) {
            a->down = timeGetTime();
            break;
        }
        a->up = timeGetTime();
        break;
    }
    default:
        ok = 0;
        break;
    case 5:
    case 10: {
        if (pressed)
            valueB = -1.0f;
        if (heldB && pressed)
            return 0;
        Axis *a = &axisB;
        heldB = pressed;
        a->dir = -1;
        if (pressed) {
            a->down = timeGetTime();
            break;
        }
        a->up = timeGetTime();
        break;
    }
    case 6:
    case 0xb: {
        if (pressed)
            valueB = 1.0f;
        if (heldB && pressed)
            return 0;
        Axis *a = &axisB;
        heldB = pressed;
        a->dir = 1;
        if (pressed) {
            a->down = timeGetTime();
            break;
        }
        a->up = timeGetTime();
        break;
    }
    case 0x15:
        if (!callback)
            return 0;
        if (pressed) {
            callback->Fire();
            break;
        }
        break;
    case 0x17:
        if (!pressed)
            return 0;
        toggle = toggle == 0;
        if (toggle) {
            SetFlag(1);
            break;
        }
        SetFlag(0);
    }
    return ok;
}
