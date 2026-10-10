// AppWindow.cpp: reconstructed from SQZ.EXE 0x43eea0-0x43f2f8. See AppWindow.h.
// The objects it talks to are not reconstructed; they are named by what this function does with them and the calls are boundaries
// in Original.h ("progress" = the object in the global at 0x4c75e8 that draws the loading screen, "viewer" = the object at 0x4c75e0
// that holds the model viewer's render options, "level manager" = 0x4badb0).
#include <string.h>
#include "AppWindow.h"
#include "GameLoop.h"
#include "Original.h"
#include "Renderer.h"
#include "World.h"

static void *&Progress()     { return *(void **)0x4c75e8; }
static void *&Viewer()       { return *(void **)0x4c75e0; }
static HINSTANCE &Instance() { return *(HINSTANCE *)0x4c73c8; }
static HWND &MainWindow()    { return *(HWND *)0x4c73cc; }

// the menu's quality entries (0x4bcca8): id and the quality the device must have for the entry to be checked
static const struct { UINT id; DWORD quality; } kQualityMenu[5] = {
    { 200, 0x40 }, { 201, 0x80 }, { 202, 0x88 }, { 203, 0x89 }, { 204, 0x8a },
};

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (*(int *)0x4c76f0 == 0) {
        void *progress = Progress();
        if (message != WM_PAINT || !progress || *(int *)((char *)progress + 0x1c) == 0)
            return 0;
    }

    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        result = 1;
        ReleaseCapture();
        break;

    case WM_ACTIVATE:
        result = 0;
        break;

    case WM_PAINT: {
        void *progress = Progress();
        if (progress && *(int *)((char *)progress + 0x1c)) {
            // the loading screen is up: it draws itself again
            ProgressBegin(progress);
            ValidateRect(GetRenderer()->hwnd, NULL);
            ProgressEnd(Progress());
            return 1;
        }
        if (GamePaused()) {
            void *app = GetApplication();
            if (ApplicationRunning(app)) {
                ApplicationRedraw(GetApplication());
                GetWorld()->RenderFrame();
            } else if (GetRenderer()->initializing == 0) {
                HWND active = NULL;
                if (GetRenderer()->fullscreen)
                    active = GetActiveWindow();
                GetWorld()->RenderFrame();
                ApplicationRedraw(GetApplication());
                if (GetLevelManager())
                    LevelManagerRefresh(GetLevelManager(), 1);
                if (active && GetRenderer()->hwnd != active)
                    RedrawWindowsAbove();
            }
        } else if (GetRenderer()->initializing == 0) {
            if (GetLevelManager())
                LevelManagerRefresh(GetLevelManager(), 1);
        }
        ValidateRect(GetRenderer()->hwnd, NULL);
        result = 1;
        break;
    }

    case WM_ACTIVATEAPP:
        *(int *)0x4c76fc = (int)wParam;
        result = 1;
        break;

    case WM_SETCURSOR:
        result = 0;
        break;

    case WM_COMMAND: {
        UINT id = (UINT)wParam & 0xffff;
        if (id == ID_EXIT) {
            DestroyWindow(hwnd);
            result = 2;
        } else if (id >= ID_QUALITY_FIRST && id <= ID_QUALITY_LAST) {
            ViewerSetQuality(Viewer(), id - ID_QUALITY_FIRST);
        } else if (id == ID_LOAD_MODEL) {
            char *path = FileOpenDialog(MainWindow());
            if (path) {
                path[strlen(path) - 6] = 0;                  // drops the last six characters (the original's choice)
                ViewerLoadModel(Viewer(), path);
            }
        } else if (id == ID_ABOUT) {
            SetGamePaused(1);
            DialogBoxParamA(Instance(), "AppAbout", MainWindow(), SimpleDialogProc, 0);
            SetGamePaused(0);
            result = 2;
        } else if (id == ID_SHOW_BITMAP) {
            ClientBitmap picture;
            ClientBitmapInit(&picture);
            ClientBitmapAttach(&picture, hwnd, 128, 128);
            RECT client;
            GetClientRect(hwnd, &client);
            ClientBitmapDraw(&picture, 0, client.bottom - 128);
            ClientBitmapFree(&picture);
        }
        break;
    }

    case WM_INITMENU: {
        // check the quality entry that matches the device's quality
        if (ApplicationRunning(GetApplication())) {
            void *device = GetRenderer()->device;
            DWORD quality = ((DWORD (__stdcall *)(void *))(*(void ***)device)[0x78 / 4])(device);
            HMENU menu = GetMenu(hwnd);
            for (int i = 0; i < 5; i++)
                CheckMenuItem(menu, kQualityMenu[i].id, kQualityMenu[i].quality == quality ? MF_CHECKED : MF_UNCHECKED);
        } else {
            result = 1;
        }
        break;
    }

    case WM_ENTERMENULOOP:
    case WM_EXITMENULOOP:
        if (message == WM_EXITMENULOOP || ApplicationRunning(GetApplication()))
            SetGamePaused(message == WM_ENTERMENULOOP);
        result = 1;
        break;
    }
    return result;
}
