// 0x43bf10: progress of a timed effect, scaled by the ramp value at +0x38.
#include <windows.h>
#include <mmsystem.h>
struct Timed43bf10 {
    char pad[0x30];
    int start;          // 0x30
    char pad2[4];
    int ramp;           // 0x38
    int active;         // 0x3c
    char pad3[0x58 - 0x40];
    float duration;     // 0x58
    float Progress();
};
float Timed43bf10::Progress()
{
    if (active) {
        float r;
        float t = (float)((int)timeGetTime() - start) * 0.001f;
        if (t >= duration)
            r = (float)ramp;
        else
            r = (float)ramp * (t / duration);
        return r;
    }
    return 0.0f;
}
