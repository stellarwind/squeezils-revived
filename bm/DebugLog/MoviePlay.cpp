// Movie::Play, SQZ.EXE 0x418c90: start playback and pump messages until it ends or is interrupted.
#include <windows.h>
#include <mmsystem.h>

int DebugLogAll(const char *fmt, ...);        // 0x419290
extern HWND g_mainWnd;                         // 0x4c73cc

struct Movie {
    int state;            // 0x00
    char pad[0x30 - 4];
    DWORD playCb;         // 0x30
    char pad2[0x3c - 0x34];
    UINT device;          // 0x3c
    int x, y;             // 0x40, 0x44
    int Close();          // 0x418e40
    int Realize();        // 0x418e80
    int Play();
};
extern Movie *g_movie;                         // 0x4c75e4

int Movie::Play()
{
    DWORD flags = (y == 0 ? 0xfd000000 : 0) + 0x4000000;
    if (x)
        flags |= 1;
    else if (y == 0)
        flags |= 1;
    else
        flags |= 2;
    playCb = (WORD)g_mainWnd;
    state = 1;
    if (mciSendCommand(device, MCI_PLAY, flags, (DWORD)&playCb)) {
        DebugLogAll("ERROR: Could not play movie\n");
        state = 0;
        return 0;
    }
    MSG msg;
    if (x) {
        while (state) {
            PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
            switch (msg.message) {
            case WM_PAINT:
                g_movie->Realize();
                break;
            case WM_KEYDOWN:
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case 0x3b5:
            case 0x3b9:
                DebugLogAll("received interrupt message\n");
                g_movie->Close();
                break;
            case WM_SYSCOMMAND:
                if (msg.wParam == 0xf060) {
                    DebugLogAll("received close window message\n");
                    g_movie->Close();
                }
                break;
            }
        }
    }
    if (x == 0 && y == 0 && state) {
        do {
            PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
            switch (msg.message) {
            case WM_PAINT:
                g_movie->Realize();
                break;
            case WM_SYSCOMMAND:
                if (msg.wParam != 0xf060)
                    break;
                DebugLogAll("received close window message\n");
            case 0x3b9:
                g_movie->Close();
                break;
            }
        } while (state);
    }
    return 1;
}
