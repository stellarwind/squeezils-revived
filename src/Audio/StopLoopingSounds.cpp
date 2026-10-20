// StopLoopingSounds, SQZ.EXE 0x44a8f0: remove the final active stream.
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

extern int g_audioOn; // 0x4bd95c
extern int g_streamOn; // 0x4bd960
extern int g_streamCount; // 0x4c77d4
extern LPDIRECTSOUNDBUFFER *g_streams; // 0x4c77dc
struct StreamBufferArray { void Remove(int index); }; // 0x468120
struct StreamNameArray { void Remove(int index); };   // 0x45e300
extern StreamBufferArray gStreamBuffers; // 0x4c77d0
extern StreamNameArray gStreamNames;     // 0x4c77b0

void StopLoopingSounds()
{
    if (g_audioOn) {
        g_streamOn = 0;
        int index = g_streamCount - 1;
        g_streams[index]->Stop();
        gStreamBuffers.Remove(index);
        gStreamNames.Remove(index);
    }
}
