// Window procedures / Win32 glue (names are guesses).
#include <windows.h>

// 0x43edd0: a dialog that closes on OK
BOOL CALLBACK OkDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
            EndDialog(hDlg, 1);
        break;
    }
    return FALSE;
}

// 0x43c1b0 / 0x43c210: show or hide the window's menu bar (only when windowed; otherwise the full-screen helper runs)
struct App { void Refresh(); };                                    // 0x4753c0
struct FullScreen { void Toggle(); };                              // 0x406ba0
extern App *gApp;                                                  // 0x4c75dc
extern HINSTANCE gInstance;                                        // 0x4c73c8
extern HWND gMainWnd;                                              // 0x4c73cc
extern HMENU gMenu;                                                // 0x4bc7bc
struct MenuHost {
    char pad[0xc];
    FullScreen *full;                                              // 0x0c
    char pad2[0x9c - 0x10];
    int windowed;                                                  // 0x9c
    void ShowMenu();                                               // 0x43c1b0
    void HideMenu();                                               // 0x43c210
};
void MenuHost::ShowMenu()
{
    if (windowed) {
        gApp->Refresh();
        if (!gMenu)
            gMenu = LoadMenu(gInstance, "APPMENU");
        SetMenu(gMainWnd, gMenu);
    } else {
        full->Toggle();
    }
}
void MenuHost::HideMenu()
{
    if (windowed) {
        gApp->Refresh();
        SetMenu(gMainWnd, NULL);
        RedrawWindow(gMainWnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
    }
}

// 0x45bef0: read exactly n bytes from the open data file; nonzero on failure
int DebugLogAll(const char *fmt, ...);                               // 0x419290
extern HANDLE gDataFile;                                           // 0x4bf444
int GetInFileData(void *buf, DWORD n);
int GetInFileData(void *buf, DWORD n)
{
    DWORD got;
    if (ReadFile(gDataFile, buf, n, &got, NULL) && got == n)
        return 0;
    DebugLogAll("GetInFileData failed\n");
    return 1;
}

// 0x4592a0: grab or release the mouse
struct AppWnd2 { char pad[0x18]; HWND hwnd; };
extern AppWnd2 *gAppWnd2;                                          // 0x4c75dc
struct MouseGrab {
    char pad[0x20];
    int captured;                                                  // 0x20
    void Center(int mode);                                         // 0x4591b0
    void Capture(int on);                                          // 0x4592a0
};
void MouseGrab::Capture(int on)
{
    if (on) {
        if (!captured) {
            SetCapture(gAppWnd2->hwnd);
            captured = 1;
            Center(2);
            return;
        }
    }
    if (!on) {
        ReleaseCapture();
        captured = 0;
    }
}
