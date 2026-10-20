// 0x43eea0: cdecl message handler called by the window-procedure dispatcher.
#include <windows.h>
#include <d3drm.h>
#include <string.h>
#include "Globals/Globals.h"
#include "World/World.h"

struct WindowProgress {
    int IsShown();                           // 0x44eea0
    void Show();                             // 0x44edd0
    void Drain();                            // 0x44ef00
};
struct WindowApplication {
    int ApplicationRunning();                // 0x43c1a0
    void ShowMenu();                          // 0x43c1b0
};
struct WindowPanel { int Update(int force); }; // 0x42db10
struct WindowViewer {
    void SetQuality(int quality);             // 0x476ea0
    int LoadWarp(const char *path);            // 0x4769a0
};
struct WindowRenderer {
    char pad0[0xc];
    int fullscreen;
    char pad10[8];
    HWND hwnd;
    int initializing;
    char pad20[0x9a8 - 0x20];
    IDirect3DRMDevice *device;
};
struct ClientBitmap {
    int fields[6];
    ClientBitmap();                           // 0x478150
    ~ClientBitmap();                          // 0x478170
    int Attach(HWND window, int width, int height); // 0x4781d0
    void Draw(int x, int y);                   // 0x4783e0
};
extern WindowRenderer *gWindowRenderer;       // 0x4c75dc
extern WindowViewer *gWindowViewer;           // 0x4c75e0
extern WindowProgress *gWindowProgress;       // 0x4c75e8
extern int gWindowReady, gWindowActive;        // 0x4c76f0, 0x4c76fc
extern int gPaused;                           // 0x4c7700
extern HWND gMainWnd;                         // 0x4c73cc
extern HINSTANCE gInstance;                   // 0x4c73c8
void SetPaused(int on);                       // 0x43ed80
int RedrawWindowsAbove();                     // 0x43ee60
char *FileOpenDialog(HWND owner);             // 0x41ed80
BOOL CALLBACK OkDialogProc(HWND, UINT, WPARAM, LPARAM); // 0x43edd0
struct QualityMenuItem { UINT id; DWORD quality; };
static const QualityMenuItem qualityMenu[5] = {
    {200,0x40}, {201,0x80}, {202,0x88}, {203,0x89}, {204,0x8a}
};

LRESULT MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result=0;
    if (!gWindowReady) {
        if (message!=WM_PAINT || !gWindowProgress || !gWindowProgress->IsShown())
            return 0;
    }
    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        result=1;
        ReleaseCapture();
        break;
    case WM_ACTIVATE: result=0; break;
    case WM_PAINT:
        if (gWindowProgress && gWindowProgress->IsShown()) {
            gWindowProgress->Show();
            ValidateRect(gWindowRenderer->hwnd,0);
            gWindowProgress->Drain();
            return 1;
        }
        if (gPaused) {
            if (((WindowApplication *)GetApplication())->ApplicationRunning()) {
                ((WindowApplication *)GetApplication())->ShowMenu();
                ((World *)GetWorld())->RenderFrame();
            } else if (!gWindowRenderer->initializing) {
                HWND active=0;
                if (gWindowRenderer->fullscreen) active=GetActiveWindow();
                ((World *)GetWorld())->RenderFrame();
                ((WindowApplication *)GetApplication())->ShowMenu();
                if (GetPanel()) ((WindowPanel *)GetPanel())->Update(1);
                if (active && gWindowRenderer->hwnd!=active) RedrawWindowsAbove();
            }
        } else if (!gWindowRenderer->initializing) {
            if (GetPanel()) ((WindowPanel *)GetPanel())->Update(1);
        }
        ValidateRect(gWindowRenderer->hwnd,0);
        result=1;
        break;
    case WM_ACTIVATEAPP: gWindowActive=wParam; result=1; break;
    case WM_SETCURSOR: result=0; break;
    case WM_COMMAND:
        switch ((int)(wParam & 0xffff)) {
        case 3: DestroyWindow(hwnd); result=2; break;
        case 200: gWindowViewer->SetQuality(0); break;
        case 201: gWindowViewer->SetQuality(1); break;
        case 202: gWindowViewer->SetQuality(2); break;
        case 203: gWindowViewer->SetQuality(3); break;
        case 204: gWindowViewer->SetQuality(4); break;
        case 0x9c4e: {
            char *path=FileOpenDialog(gMainWnd);
            if (path) {
                path[strlen(path)-6]=0;
                gWindowViewer->LoadWarp(path);
            }
            break;
        }
        case 0x9c52:
            SetPaused(1);
            DialogBoxParam(gInstance,"AppAbout",gMainWnd,(DLGPROC)OkDialogProc,0);
            SetPaused(0);
            result=2;
            break;
        case 0x9c65: {
            ClientBitmap picture;
            picture.Attach(hwnd,128,128);
            RECT client;
            GetClientRect(hwnd,&client);
            picture.Draw(0,client.bottom-128);
            break;
        }
        }
        break;
    case WM_INITMENU:
        if (((WindowApplication *)GetApplication())->ApplicationRunning()) {
            DWORD quality=gWindowRenderer->device->GetQuality();
            HMENU menu=GetMenu(hwnd);
            for (int i=0; i<5; ++i)
                CheckMenuItem(menu,qualityMenu[i].id,qualityMenu[i].quality==quality ? MF_CHECKED : MF_UNCHECKED);
        } else result=1;
        break;
    case WM_ENTERMENULOOP:
        if (((WindowApplication *)GetApplication())->ApplicationRunning()) SetPaused(1);
        result=1;
        break;
    case WM_EXITMENULOOP:
        SetPaused(0);
        result=1;
        break;
    }
    return result;
}
