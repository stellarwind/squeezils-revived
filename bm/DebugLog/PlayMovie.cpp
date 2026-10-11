// PlayMovie, SQZ.EXE 0x418aa0: fade out, play .\movies\<name> in the main window, return to the game.
#include <windows.h>
#include <stdio.h>

struct Renderer {
    int pad0[3];
    int fullscreen;        // 0x0c
    int pad10[2];
    HWND hwnd;             // 0x18
    void FadePalette(float seconds);        // 0x4750a0
};
void FillBackground(HDC dc, RECT *r, int mode);   // 0x475380 (cdecl)
struct Movie {
    int Open(const char *file, int a, int b, int fatal);   // 0x418ba0
    int Play();                                             // 0x418c90
};
extern Renderer *g_app;        // 0x4c75dc
extern Movie *g_movie;         // 0x4c75e4
extern HWND g_mainWnd;         // 0x4c73cc
extern int g_flag;             // 0x4bf458
void Stop1();   // 0x4499e0
int  Poll();    // 0x44aae0
void Stop2();   // 0x45aa70
void Stop3();   // 0x449a60
void Stop4();   // 0x449a80

int __stdcall PlayMovie(const char *name, float fade)
{
    char path[128];
    sprintf(path, ".\movies\%s", name);
    RECT rect;
    int ok = g_movie->Open(path, 0, g_app->fullscreen, 1);
    if (ok) {
        g_app->FadePalette(fade);
        GetClientRect(g_mainWnd, &rect);
        HDC dc = GetDC(g_mainWnd);
        FillBackground(dc, &rect, 0);
        ReleaseDC(g_mainWnd, dc);
        while (Poll() > 0)
            ;
        Stop1();
        if (g_flag)
            Stop2();
        Stop3();
        Stop4();
        g_movie->Play();
        SetFocus(g_app->hwnd);
    }
    return ok;
}
