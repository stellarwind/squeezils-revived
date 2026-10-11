// FadeScreenDownFunction::Execute (0x441cf0): fade the screen down over the given time.
#include "FunctionMisc/MiscClasses.h"

struct ScreenFader { void FadeDown(float seconds); };    // 0x4750a0
extern ScreenFader *g_fader;                             // 0x4c75dc

int FadeScreenDownFunction::Execute(ScriptCall *args, int extra)
{
    g_fader->FadeDown(arg->Get(extra));
    return 1;
}
