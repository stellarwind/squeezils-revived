// TimerTimeMessageFunction::Execute (0x440520): show a time variable as minutes:seconds on the HUD.
#include <stdio.h>
#include "FunctionStr/Str.h"

struct TimeVars {
    virtual void Set(String *name, int *value, int flags);   // slot 0
    virtual int Contains(String *name);                      // slot 1
    int Get(String *name);                                   // 0x461230
};
extern TimeVars g_timeVars;                                  // 0x4c7060
struct TimeHud { void ShowTimerMessage(const char *text); }; // 0x42e0e0
TimeHud *GetTimeHud();                                       // 0x4206c0 (cdecl)

int TimerTimeMessageFunction::Execute(ScriptCall *args, int extra)
{
    TimeVars *vars = &g_timeVars;
    if (!vars->Contains(&text))
        return 0;
    int secs = g_timeVars.Get(&text);
    char buf[20];
    sprintf(buf, "%d:%02d", secs / 60, secs % 60);
    GetTimeHud()->ShowTimerMessage(buf);
    return 1;
}
