// 0x42e0e0: restore the note rectangle and draw a new note on the panel.
#include <windows.h>
#include <string.h>
#pragma intrinsic(strlen, strcpy)
#include "Bitmap/Bitmap.h"
struct NoteRenderer {
    char pad[0x18];
    HWND hwnd;
    char pad1c[0x994 - 0x1c];
    IDirectDrawSurface *primary;
    void PutText(HFONT font, int x, int y, const char *text); // 0x4753d0
};
extern NoteRenderer *g_noteRenderer;
struct NotePanel {
    char pad[0x460];
    Bitmap *background;
    char pad464[0x618 - 0x464];
    HFONT font;
    char pad61c[0x694 - 0x61c];
    char note[20];
    void GetScreenOrigin(int *x, int *y); // 0x42da30
    void ClearNote(); // 0x42dcd0
    int DrawNote(const char *text);
};
inline void EraseNoteArea(NotePanel *panel, const RECT &screen, const RECT &picture)
{
    panel->background->DrawToSurface(g_noteRenderer->primary, screen.left, screen.top,
        screen.right, screen.bottom, picture.left, picture.top, picture.right, picture.bottom);
}
int NotePanel::DrawNote(const char *text)
{
    int ox, oy;
    GetScreenOrigin(&ox, &oy);
    POINT origin = { ox, oy };
    ScreenToClient(g_noteRenderer->hwnd, &origin);
    POINT at = { 300, 366 };
    ClientToScreen(g_noteRenderer->hwnd, &at);
    RECT screen = { at.x, at.y, at.x + 90, at.y + 30 };
    RECT picture = { 300 - origin.x, 366 - origin.y, 0, 0 };
    picture.right = picture.left + 90;
    picture.bottom = picture.top + 30;
    EraseNoteArea(this, screen, picture);
    if (strlen(text) == 0) {
        ClearNote();
        return 1;
    }
    HDC dc = GetDC(g_noteRenderer->hwnd);
    HGDIOBJ oldFont = 0;
    if (font)
        oldFont = SelectObject(dc, font);
    g_noteRenderer->PutText(font, 300, 366, text);
    if (font)
        SelectObject(dc, oldFont);
    ReleaseDC(g_noteRenderer->hwnd, dc);
    if (text != note)
        strcpy(note, text);
    return 1;
}
