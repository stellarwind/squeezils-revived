// 0x43a580: registers the PlayAudioEffect and StartAmbientLoop script functions.
#include "FunctionStr/Str.h"
#include "FunctionAmbient/Ambient.h"
void RegisterFunction(Function *f);                      // 0x421d00 (cdecl)

void RegisterAudioFunctions()
{
    RegisterFunction(new PlayAudioEffectFunction);
    RegisterFunction(new StartAmbientLoopFunction);
}
