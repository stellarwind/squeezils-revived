// 0x448060: joystick detection and centre-stability probe (200 samples of the X/Y axes).
#include <windows.h>
#include <mmsystem.h>
struct Stick2 {
    char pad[0x30];
    int present;                // 0x30
    char pad2[0x44 - 0x34];
    int state;                  // 0x44: 2 = axes never moved, 1 = axes live
    int Probe();
};
int Stick2::Probe()
{
    JOYINFOEX info;
    UINT minX, maxX, minY, maxY;
    int i;
    memset(&info, 0, sizeof(info));
    info.dwSize = sizeof(info);
    info.dwFlags = JOY_RETURNX | JOY_RETURNY;
    if (joyGetPosEx(0, &info) == 0)
        present = 1;
    else
        present = 0;
    if (!present)
        return 0;
    minX = 0xfde8;
    maxX = 0;
    minY = 0xfde8;
    maxY = 0;
    for (i = 0; i < 200; i++) {
        if (joyGetPosEx(0, &info)) {
            present = 0;
            return 0;
        }
        if (minX > info.dwXpos)
            minX = info.dwXpos;
        if (maxX < info.dwXpos)
            maxX = info.dwXpos;
        if (minY > info.dwYpos)
            minY = info.dwYpos;
        if (maxY < info.dwYpos)
            maxY = info.dwYpos;
    }
    if (maxX == minX && maxY == minY)
        state = 2;
    else
        state = 1;
    return present;
}
