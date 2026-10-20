// SQZ.EXE 0x447d80: dialog procedure (WM_INITDIALOG runs a global's method, sets a flag, closes; WM_COMMAND/IDCANCEL closes)
#include <windows.h>
struct Dlg { void Setup(); };               // 0x447de0
extern Dlg *g_dlg;                          // 0x4bd6f8
extern int g_dlgDone;                       // 0x4bd744
BOOL CALLBACK DlgProc447d80(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG:
        g_dlg->Setup();
        g_dlgDone = 1;
        EndDialog(hwnd, 1);
        return 1;
    case WM_COMMAND:
        if (LOWORD(wp) == IDCANCEL)
            EndDialog(hwnd, 1);
        return 0;
    }
    return 0;
}
