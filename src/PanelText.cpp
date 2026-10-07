// PanelText: the panel's message line, the short note below it, the font they use. From SQZ.EXE 0x42dd30 (message),
// 0x42e0e0 (note), and 0x42e7a0 (font set-up, with the EnumFontFamilies callback 0x42e710).
#include <string.h>
#include <stdio.h>
#include "Panel.h"
#include "IniReader.h"
#include "Original.h"

// Wipe the area under the text with the panel picture: the area on the screen and the same area in the picture.
static void EraseArea(Panel *p, const RECT &screen, const RECT &local)
{
    Renderer *r = GetRenderer();
    p->background->DrawToSurface(r->primary, screen.left, screen.top, screen.right, screen.bottom,
                                 local.left, local.top, local.right, local.bottom);
}

BOOL Panel::DrawMessage(const char *text)
{
    int ox, oy;
    GetScreenOrigin(&ox, &oy);
    Renderer *r = GetRenderer();
    POINT origin = { ox, oy };
    ScreenToClient(r->hwnd, &origin);
    POINT at = { messageX, messageY };
    ClientToScreen(r->hwnd, &at);
    int height = messageLines * fontHeight;
    RECT onScreen = { at.x, at.y, at.x + messageWidth, at.y + height };
    RECT inPicture = { messageX - origin.x, messageY - origin.y, 0, 0 };
    inPicture.right = inPicture.left + messageWidth;
    inPicture.bottom = inPicture.top + height;
    if (background->GetHeight() <= inPicture.bottom) {           // keep the area inside the picture
        int over = inPicture.bottom - background->GetHeight();
        inPicture.bottom -= over;
        onScreen.bottom -= over;
    }
    EraseArea(this, onScreen, inPicture);

    if (strlen(text) == 0) {
        ClearMessage();
        return TRUE;
    }
    if (text != message)
        strcpy(message, text);

    // the start of every word (after a space), and the end of the text
    const char *start[102];
    int count = 1;
    start[0] = message;
    size_t length = strlen(message);
    for (size_t i = 1; i < length; i++) {
        if (message[i] == ' ')
            start[count++] = &message[i + 1];
    }
    const char *end = message + length;
    start[count] = end;

    HDC dc = GetDC(r->hwnd);
    HGDIOBJ oldFont = NULL;
    if (font)
        oldFont = SelectObject(dc, font);
    char *line = (char *)Operator_New(80);
    line[0] = 0;
    const char *from = message;
    int lineNumber = 0;
    while (from < end && lineNumber < messageLines) {
        SIZE size;
        size.cx = 1000;
        const char *to = end;
        if (messageWidth < 1000) {
            int k = count;
            while (to >= from) {                                 // the longest run of words that fits, shortening a word at a time
                to = start[k];
                int n = (int)(to - from);
                strncpy(line, from, n);
                line[n] = 0;
                GetTextExtentPoint32A(dc, line, (int)strlen(line), &size);
                k--;
                if (!(messageWidth < size.cx))
                    break;
            }
        }
        r->PutText(font, messageX, fontHeight * lineNumber + messageY, line);
        from = to;
        lineNumber++;
    }
    Operator_Delete(line);
    if (font)
        SelectObject(dc, oldFont);
    ReleaseDC(r->hwnd, dc);
    return FALSE;
}

BOOL Panel::DrawNote(const char *text)
{
    int ox, oy;
    GetScreenOrigin(&ox, &oy);
    Renderer *r = GetRenderer();
    POINT origin = { ox, oy };
    ScreenToClient(r->hwnd, &origin);
    POINT at = { 300, 366 };
    ClientToScreen(r->hwnd, &at);
    RECT onScreen = { at.x, at.y, at.x + 90, at.y + 30 };
    RECT inPicture = { 300 - origin.x, 366 - origin.y, 0, 0 };
    inPicture.right = inPicture.left + 90;
    inPicture.bottom = inPicture.top + 30;
    EraseArea(this, onScreen, inPicture);
    if (strlen(text) == 0) {
        ClearNote();
        return TRUE;
    }
    HDC dc = GetDC(r->hwnd);
    HGDIOBJ oldFont = NULL;
    if (font)
        oldFont = SelectObject(dc, font);
    r->PutText(font, 300, 366, text);
    if (font)
        SelectObject(dc, oldFont);
    ReleaseDC(r->hwnd, dc);
    if (text != note)
        strcpy(note, text);
    return TRUE;
}

// ---- the message font ----
static int g_fontFound;      // @4bbea0 set once the enumeration has found a font

// EnumFontFamilies callback @42e710: keep the first font found, but prefer one whose style is "bold".
static int CALLBACK FontFound(const LOGFONTA *font, const TEXTMETRICA *, DWORD, LPARAM param)
{
    const ENUMLOGFONTA *found = (const ENUMLOGFONTA *)font;
    ENUMLOGFONTA *keep = (ENUMLOGFONTA *)param;
    GameLog("MESSAGE DEBUG: EnumFamCallBack, fullName %s, style %s.\n", found->elfFullName, found->elfStyle);
    if (!g_fontFound) {
        memcpy(keep, found, sizeof(ENUMLOGFONTA));
        g_fontFound = 1;
    }
    if (_stricmp((const char *)found->elfStyle, "bold") == 0 && _stricmp((const char *)keep->elfStyle, "bold") != 0)
        memcpy(keep, found, sizeof(ENUMLOGFONTA));
    return 1;
}

// Make the font from the enumerated face at the point size: the height is minus the size in pixels, taken the way the original does
// (one pixel larger than the truncated value).
static void MakeFont(Panel *p, ENUMLOGFONTA *face, long long product)
{
    int pixels = (int)((double)product * (double)(1.0f / 72.0f));
    face->elfLogFont.lfHeight = (double)pixels < (double)product ? -1 - pixels : -pixels;
    face->elfLogFont.lfWidth = 0;
    p->font = CreateFontIndirectA(&face->elfLogFont);
    int h = face->elfLogFont.lfHeight;
    p->fontHeight = (h < 0 ? -h : h) + 5;
}

void PanelLoadFonts(Panel *p)
{
    IniReader ini;
    ini.SetSection("Interface");
    int size = ini.GetInt("FontSize", 12);
    char name[64];
    strcpy(name, ini.GetString("FontName"));
    g_fontFound = 0;
    HDC dc = GetDC(GetRenderer()->hwnd);
    ENUMLOGFONTA face;
    EnumFontFamiliesA(dc, name, (FONTENUMPROCA)FontFound, (LPARAM)&face);
    long long product = (long long)GetDeviceCaps(dc, LOGPIXELSY) * size;
    if (g_fontFound) {
        MakeFont(p, &face, product);
    } else {
        GameLog("ERROR: could not find font %s!\n", name);
        EnumFontFamiliesA(dc, "System", (FONTENUMPROCA)FontFound, (LPARAM)&face);
        if (g_fontFound)
            MakeFont(p, &face, product);
        else
            p->fontHeight = 21;
    }
    ReleaseDC(GetRenderer()->hwnd, dc);
    p->messageX = ini.GetInt("Messagex", 360);
    p->messageY = ini.GetInt("Messagey", 400);
    strcpy(p->message, ini.GetString("Message"));
    p->messageWidth = ini.GetInt("MaxMessageWidth", 120);
    p->messageLines = ini.GetInt("MaxMessageHeight", 100) / p->fontHeight;
}
