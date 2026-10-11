// Menu::ActionLoadGame (0x406760): pick a saved game and load it.
#include <windows.h>
#include <string.h>

extern char g_saveName[];      // 0x4c63d0
extern char g_loadName[];      // 0x4c63e8
extern char g_saveDir[];       // 0x4c6400
extern HINSTANCE g_hInstance;  // 0x4c73c8
extern HWND g_mainWnd;         // 0x4c73cc
struct App { char pad[0x18]; HWND hwnd; };
extern App *g_app;             // 0x4c75dc
int CALLBACK LoadGameDlgProc(HWND, UINT, WPARAM, LPARAM);   // 0x406eb0

struct Game {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual int Load(const char *path);   // slot 10
};
Game *GetGame();   // 0x420690

int ActionLoadGame()
{
    char path[200];
    strcpy(g_loadName, "");
    strcpy(g_saveName, "");
    if (DialogBoxParam(g_hInstance, MAKEINTRESOURCE(0x71), g_mainWnd, (DLGPROC)LoadGameDlgProc, 0)) {
        if (strlen(g_loadName)) {
            strcpy(path, g_saveDir);
            strcat(path, g_loadName);
            if (!GetGame()->Load(path)) {
                strcpy(path, "Could not load game: ");
                strcat(path, g_loadName);
                MessageBox(g_app->hwnd, path, "Load Game", 0);
            }
        }
    }
    return 0;
}
