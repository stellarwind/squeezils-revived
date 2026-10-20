// 0x447890 (ret 0xc): joystick window-message handler; returns the button number (1..6) on a press/release edge, -1 otherwise.
#include <windows.h>
#include <mmsystem.h>
extern int g_buttonPrev[6];     // 0x4bd700
extern int g_buttonNow[6];      // 0x4bd718
int __stdcall JoyMessage(unsigned msg, unsigned wparam, unsigned lparam)
{
    JOYINFOEX info;
    int i;
    info.dwSize = sizeof(info);
    info.dwFlags = JOY_RETURNBUTTONS;
    if (msg != MM_JOY1MOVE) {
        if (msg != MM_JOY1BUTTONDOWN && msg != MM_JOY1BUTTONUP)
            return -1;
        if (joyGetPosEx(0, &info))
            return -1;
        for (i = 0; i < 6; i++) {
            switch (i) {
            case 0: g_buttonNow[i] = info.dwButtons & JOY_BUTTON1; break;
            case 1: g_buttonNow[i] = info.dwButtons & JOY_BUTTON2; break;
            case 2: g_buttonNow[i] = info.dwButtons & JOY_BUTTON3; break;
            case 3: g_buttonNow[i] = info.dwButtons & JOY_BUTTON4; break;
            case 4: g_buttonNow[i] = info.dwButtons & JOY_BUTTON5; break;
            case 5: g_buttonNow[i] = info.dwButtons & JOY_BUTTON6; break;
            }
            if (!g_buttonNow[i] && g_buttonPrev[i]) {
                g_buttonPrev[i] = 0;
                return i + 1;
            }
            if (g_buttonNow[i] && !g_buttonPrev[i]) {
                g_buttonPrev[i] = 1;
                return i + 1;
            }
        }
    }
    return -1;
}
