// spare declarations: the compiler's register choices follow the symbol count (tools/flagsearch/padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
// 0x476630: window-message handler of the app window (hwnd, msg, wParam, app): returns 1 when handled.
#include <windows.h>
#include <ddraw.h>
struct App {
    char pad[0x18];
    HWND hwnd;                 // 0x18
    char pad2[8];
    LPDIRECTDRAW dd;           // 0x24
    char pad3[0x78 - 0x28];
    int minimized;             // 0x78
};
void Moved();                  // 0x475870
void Activate();               // 0x474e10
void MenuLoop();               // 0x4753c0
int HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, App *app)
{
    if (!app)
        return 0;
    if (hWnd != app->hwnd)
        return 0;
    switch (msg) {
    case WM_SHOWWINDOW:
        return 1;
    case WM_MOVE:
        Moved();
        return 1;
    case WM_SIZE:
        if (wParam == 1) {
            app->minimized = 1;
            app->dd->RestoreDisplayMode();
            return 1;
        }
        app->minimized = 0;
        return 1;
    case WM_ACTIVATE:
        if (LOWORD(wParam))
            Activate();
        return 1;
    case WM_ENTERMENULOOP:
        MenuLoop();
        return 1;
    case WM_EXITMENULOOP:
        return 0;
    case WM_PALETTECHANGED:
        if (app->hwnd == (HWND)wParam)
            return 0;
    case WM_QUERYNEWPALETTE:
        Activate();
        return 1;
    }
    return 0;
}
