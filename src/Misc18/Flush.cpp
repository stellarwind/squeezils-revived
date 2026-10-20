// SQZ.EXE 0x43c970: drain pending keyboard and mouse messages, then reset the two timers and their helpers.
#include <windows.h>
struct Clock {
    char pad0[0x3c];
    int running;                // +0x3c
    char pad40[0x10];
    int running2;               // +0x50
    void M0(); void M1(); void M2(); void M3();     // 0x43bbf0, 0x43bba0, 0x43bbd0, 0x43bbc0
    void Reset();
};
void Clock::Reset()
{
    MSG msg;
    while (PeekMessage(&msg, 0, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
        ;
    while (PeekMessage(&msg, 0, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
        ;
    running2 = 0;
    running = 0;
    M0();
    M1();
    M2();
    M3();
}
