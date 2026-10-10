// GameLoop.cpp: reconstructed from SQZ.EXE 0x43ed80-0x43ee9e. See GameLoop.h.
#include "GameLoop.h"
#include "GameClock.h"
#include "Original.h"
#include "Renderer.h"

void SetGamePaused(int paused)
{
    if (**(int **)0x4c75e4 != 0)
        return;
    if (paused) {
        ApplicationSetInputMode(GetApplication(), 1);
        PauseAudio();
        GamePaused() = 1;
    } else {
        ApplicationSetInputMode(GetApplication(), 0);
        StartAudio();
        GamePaused() = 0;
    }
}

INT_PTR CALLBACK SimpleDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM)
{
    if (message == WM_INITDIALOG)
        return 1;
    if (message == WM_COMMAND && LOWORD(wParam) == IDOK)
        EndDialog(hwnd, 1);
    return 0;
}

int GameIdle()
{
    if (GamePaused())
        return 0;
    if (OneSimPerRender() || !TheClock()->Ahead())
        RunFrame();
    if (OneSimPerRender() || TheClock()->Ahead())
        ResetClock();
    return 0;
}

int RedrawWindowsAbove()
{
    HWND window = GetRenderer()->hwnd;
    if (window) {
        while ((window = GetWindow(window, GW_HWNDPREV)) != NULL)
            RedrawWindow(window, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_FRAME);
    }
    return 1;
}
