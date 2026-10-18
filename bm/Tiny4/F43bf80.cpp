// 0x43bf80: sibling of Timed43bf10::Progress (live clock when `running`, frozen stamp otherwise).
#include <windows.h>
#include <mmsystem.h>
struct Timed43bf80 {
    char pad[0x44];
    int start;          // 0x44
    int frozen;         // 0x48
    int ramp;           // 0x4c
    int running;        // 0x50
    char pad3[0x58 - 0x54];
    float duration;     // 0x58
    float Progress();
};
float Timed43bf80::Progress()
{
    int now;
    if (running)
        now = (int)timeGetTime();
    else
        now = frozen;
    float r;
    float t = (float)(now - start) * 0.001f;
    if (t >= duration)
        r = (float)ramp;
    else
        r = (float)ramp * (t / duration);
    return r;
}
