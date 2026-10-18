// RunFrame (SQZ.EXE 0x4206e0): one game step; the clock's sim stopwatch runs around it. The game time of a step is the clock's fixed step in seconds.
struct ClockR {
    char pad[0xc];
    unsigned simRateMs;             // 0x0c (read by 0x41ec00)
    unsigned SimRate();             // 0x41ec00
    void BeginSim();                // 0x41eca0
    void EndSim();                  // 0x41ecb0
};
ClockR *TheClock();                 // 0x4206d0
extern ClockR *g_clock;             // 0x4badc0
struct SimR {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5();
    virtual void s6(); virtual void s7(); virtual void s8();
};
extern SimR *g_sim;                 // 0x4badbc
void AudioUpdate();                 // 0x44aae0
void ScriptUpdateTimers(float seconds);   // 0x422f70
void UpdateProps(float seconds);          // 0x42d210

extern const float kMilli;         // 0x49da38 (0.001f)

void RunFrame()
{
    float seconds = (float)((long double)TheClock()->SimRate() * kMilli);
    g_clock->BeginSim();
    AudioUpdate();
    ScriptUpdateTimers(seconds);
    UpdateProps(seconds);
    if (g_sim) {
        g_sim->s8();
        g_sim->s3();
    }
    g_clock->EndSim();
}
