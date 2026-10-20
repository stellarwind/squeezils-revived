// 0x43b7c0: sibling of 0x43b5a0 (key handler with press/release stamps); no value fields, adds cases 9 and 0xe.
#include <windows.h>
#include <mmsystem.h>
struct Sink { virtual void v0(); virtual void Send(int code, const char *text); };
extern const char gText[];                              // 0x4b9488
struct Axis { DWORD down, up; int dir; };
struct Presser {
    char pad[0x1c];
    Sink *sink;             // 0x1c
    char pad2[0x30 - 0x20];
    Axis axisA;             // 0x30
    int heldA;              // 0x3c
    char pad3[0x44 - 0x40];
    Axis axisB;             // 0x44
    int heldB;              // 0x50
    char pad4[0x74 - 0x54];
    int flag;               // 0x74
    int Press(int code, int pressed);
};
int Presser::Press(int code, int pressed)
{
    int ok = 1;
    switch (code) {
    case 0:
    case 0xc: {
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
    case 9:
        if (pressed) {
            flag = 1;
            break;
        }
        flag = 0;
        break;
    case 0xe:
        if (pressed)
            sink->Send(code, gText);
        break;
    }
    return ok;
}
