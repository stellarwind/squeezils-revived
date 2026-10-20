// Timed ramp value, two variants (SQZ.EXE 0x43bf10, 0x43bf80): ramps 0..total over `duration` seconds measured with timeGetTime().
extern "C" __declspec(dllimport) unsigned long __stdcall timeGetTime(void);
// SIZE 0x5c
struct Ramp {
    char pad0[0x30];
    int start;           // +0x30 start time (ms)
    int pad34;
    int total;           // +0x38
    int running;         // +0x3c
    char pad40[4];
    int a44;             // +0x44
    int a48;             // +0x48
    int total2;          // +0x4c
    int running2;        // +0x50
    char pad54[4];
    float duration;      // +0x58
    float Value();       // 0x43bf10
    float Value2();      // 0x43bf80
};
float Ramp::Value()
{
    if (running) {
        int ms = timeGetTime() - start;
        float t = ms * 0.001f;
        if (t >= duration) {
            volatile float result = (float)total;
            return result;
        }
        { float q = t / duration; volatile float r = total * q; return r; }
    }
    return 0.0f;
}
float Ramp::Value2()
{
    int ms;
    if (running2)
        ms = timeGetTime();
    else
        ms = a48;
    ms -= a44;
    float t = ms * 0.001f;
    if (t >= duration) {
        volatile float result = (float)total2;
        return result;
    }
    { float q = t / duration; volatile float r = total2 * q; return r; }
}
