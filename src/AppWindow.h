// AppWindow: the game window's message handler (SQZ.EXE 0x43eea0, 952 bytes). The window class is registered by the program's
// start (0x43f310) with this as its procedure. Menu ids are those of the "APPMENU" resource.
#pragma once
#include <windows.h>

// @0043eea0 (stdcall, ret 0x10). Returns 0 for "not handled"; 1 or 2 are the results the original sets after handling a message.
// Until the game has set itself up (the flag at 0x4c76f0) only a WM_PAINT for the loading screen is handled, so that it can be redrawn.
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// menu command ids
enum {
    ID_EXIT              = 3,
    ID_QUALITY_FIRST     = 200,     // 200-204 (0xc8-0xcc): the five render quality entries of the menu; they go to the viewer's SetQuality
    ID_QUALITY_LAST      = 204,
    ID_LOAD_MODEL        = 0x9c4e,  // opens the file dialog and loads the model into the viewer
    ID_ABOUT             = 0x9c52,
    ID_SHOW_BITMAP       = 0x9c65,  // draws the 128x128 picture at the bottom left of the client area
};
