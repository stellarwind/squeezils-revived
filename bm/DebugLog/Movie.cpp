// MCI movie player helpers, SQZ.EXE 0x418e40 / 0x418e80.
#include <windows.h>
#include <mmsystem.h>

int DebugLogAll(const char *fmt, ...);   // 0x419290

struct Movie {
    int state;                   // 0x00
    DWORD parms[14];             // 0x04
    UINT device;                 // 0x3c
    int Close();                 // 0x418e40
    int Realize();               // 0x418e80
};

int Movie::Close()
{
    if (mciSendCommand(device, MCI_CLOSE, 0, (DWORD)parms)) {
        DebugLogAll("ERROR: Could not close movie\n");
        return 0;
    }
    state = 0;
    return 1;
}

int Movie::Realize()
{
    if (mciSendCommand(device, MCI_REALIZE, 0, 0)) {
        DebugLogAll("ERROR: Could not realize movie player\n");
        return 0;
    }
    return 1;
}
