// Joystick start-up probe (SQZ.EXE 0x447810) and position read (0x4481f0, stdcall, ret 8), both over joyGetPosEx.
#include <windows.h>
#include <mmsystem.h>
#include "compat41.h"

struct Joystick {
    char pad[0x18];
    int f18, f1c;                    // 0x18 0x1c
    char pad2[0x2c - 0x20];
    int f2c;                         // 0x2c
    int present;                     // 0x30
    char pad3[0x44 - 0x34];
    int f44;                         // 0x44
    Joystick *Init();
};
extern Joystick *g_joystick;         // 0x4bd6f8

Joystick *Joystick::Init()
{
    JOYINFOEX info;
    info.dwSize = sizeof(info);
    info.dwFlags = JOY_RETURNCENTERED;
    f2c = 0;
    f18 = 0;
    f1c = 0;
    if (joyGetPosEx(0, &info) == JOYERR_NOERROR)
        present = 1;
    else
        present = 0;
    g_joystick = this;
    f44 = 0;
    return this;
}

int __stdcall ReadJoystick(int *x, int *y)
{
    JOYINFOEX info;
    info.dwSize = sizeof(info);
    info.dwFlags = JOY_RETURNALL;
    if (joyGetPosEx(0, &info))
        return 0;
    *x = info.dwXpos;
    *y = info.dwYpos;
    return 1;
}
