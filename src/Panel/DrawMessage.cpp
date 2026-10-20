// 0x42dd30: restore the message box and draw a new message, word-wrapped to the box width.
#include <windows.h>
#include <string.h>
#pragma intrinsic(strlen, strcpy)
#include "Bitmap/Bitmap.h"
struct MessageRenderer {
    char pad[0x18];
    HWND hwnd;
    char pad1c[0x994 - 0x1c];
    IDirectDrawSurface *primary;
    void PutText(HFONT font, int x, int y, const char *text); // 0x4753d0
};
extern MessageRenderer *g_msgRenderer;
struct MessagePanel {
    char pad[0x460];
    Bitmap *background;      // 0x460
    char pad464[0x618 - 0x464];
    HFONT font;              // 0x618
    int lineHeight;          // 0x61c
    int x, y;                // 0x620
    int width;               // 0x628
    int lines;               // 0x62c
    char message[100];       // 0x630
    void GetScreenOrigin(int *x, int *y); // 0x42da30
    void ClearMessage();     // 0x42dd00
    int DrawMessage(const char *text);
};
int MessagePanel::DrawMessage(const char *text)
{
    char *words[100];
    int row = 0;
    int ox, oy;
    GetScreenOrigin(&ox, &oy);
    POINT origin = { ox, oy };
    ScreenToClient(g_msgRenderer->hwnd, &origin);
    POINT at = { x, y };
    ClientToScreen(g_msgRenderer->hwnd, &at);
    int height = lines * lineHeight;
    RECT screen = { at.x, at.y, at.x + width, at.y + height };
    RECT picture;
    picture.left = x - origin.x;
    picture.top = y - origin.y;
    picture.right = picture.left + width;
    int bottom = height + picture.top;
    if (bottom >= background->GetHeight()) {
        int over = bottom - background->GetHeight();
        bottom -= over;
        screen.bottom -= over;
    }
    background->DrawToSurface(g_msgRenderer->primary, screen.left, screen.top,
        screen.right, screen.bottom, picture.left, picture.top, picture.right, bottom);
    if (strlen(text) == 0) {
        ClearMessage();
        return 1;
    }
    if (text != message)
        strcpy(message, text);
    words[0] = message;
    char *end = message + strlen(message);
    int count = 1;
    for (unsigned i = 1; i < strlen(message); i++)
        if (message[i] == ' ')
            words[count++] = &message[i + 1];
    words[count] = end;
    HDC dc = GetDC(g_msgRenderer->hwnd);
    HGDIOBJ oldFont;
    if (font)
        oldFont = SelectObject(dc, font);
    SIZE size;
    size.cx = 1000;
    char *line = new char[0x50];
    char *lineEnd = end;
    char *lineStart = words[0];
    while (lineStart < lineEnd && row < lines) {
        size.cx = 1000;
        char **w = &words[count];
        while (size.cx > width && lineEnd >= lineStart) {
            lineEnd = *w;
            int n = lineEnd - lineStart;
            strncpy(line, lineStart, n);
            line[n] = 0;
            w--;
            GetTextExtentPoint32(dc, line, strlen(line), &size);
        }
        g_msgRenderer->PutText(font, x, lineHeight * row + y, line);
        row++;
        lineStart = lineEnd;
        lineEnd = end;
    }
    delete line;
    if (font)
        SelectObject(dc, oldFont);
    ReleaseDC(g_msgRenderer->hwnd, dc);
    return 0;
}
