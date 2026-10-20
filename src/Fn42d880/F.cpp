// 0x42d5f0 / 0x42d880: constructor and destructor of the display/overlay object: deletes its bitmaps, releases a GDI handle, destroys the members.
#include <windows.h>
#include "Bitmap2/Bitmap2.h"
#include "IniReader/IniReader.h"
extern Renderer *g_renderer;                               // 0x4c75dc
extern int g_fontFound;                                    // 0x4bbea0
int CALLBACK FontFound(const LOGFONT *lf, const TEXTMETRIC *tm, DWORD type, LPARAM out);   // 0x42e710
void DebugLog(const char *format, ...);                    // 0x419290
struct Vec3 { float x, y, z; Vec3() {} ~Vec3(); };         // 0x401660
struct MemA { char d[0x38c]; MemA(); ~MemA(); };           // 0x42ea70 / 0x42eb40
struct MemB { char d[0x94]; MemB(); ~MemB(); };            // 0x42f580 / 0x42f5f0
struct MemC { char d[0x13c]; MemC(); ~MemC(); void Reset(); };   // 0x414380 / 0x4143c0, Reset 0x414490
struct Overlay {
    int originX, originY;                                  // 0x000
    MemA a;                                                // 0x008
    MemB b;                                                // 0x394
    Bitmap *tiles[10];                                     // 0x428
    int modeA, modeB;                                      // 0x450
    int selected;                                          // 0x458
    int unknown45c;                                        // 0x45c
    Bitmap *main;                                          // 0x460
    char pad464[0x478 - 0x464];
    int unknown478, unknown47c;                            // 0x478
    MemC c;                                                // 0x480
    float step;                                            // 0x5bc
    float verticalThreshold;                               // 0x5c0
    float verticalChange;                                  // 0x5c4
    Vec3 v0, v1, v2, v3, v4, v5;                           // 0x5c8
    float turn;                                            // 0x610
    int unknown614;                                        // 0x614
    HGDIOBJ font;                                          // 0x618
    int lineHeight;                                        // 0x61c
    int messageX, messageY;                                // 0x620
    int maxMessageWidth;                                   // 0x628
    int maxMessageLines;                                   // 0x62c
    char textA[0x694 - 0x630];                             // 0x630
    char textB[1];                                         // 0x694
    Overlay();
    ~Overlay();
    void GetScreenOrigin(int *x, int *y);                  // 0x42da30
    void Layout();                                         // 0x42e7a0
};
Overlay::~Overlay()
{
    delete main;
    for (int i = 0; i < 10; i++)
        delete tiles[i];
    if (font)
        DeleteObject(font);
}

Overlay::Overlay()
{
    main = new Bitmap(g_renderer);
    unknown478 = 0;
    unknown47c = 0;
    c.Reset();
    for (int i = 0; i < 10; i++) {
        tiles[i] = 0;
        tiles[i] = new Bitmap(g_renderer);
    }
    GetScreenOrigin(&originX, &originY);
    v5.x = v5.y = -5.0f;
    turn = 6.2831855f;
    step = 0.01f;
    v5.z = -5.0f;
    IniReader ini;
    ini.SetSection("Compass");
    verticalThreshold = ini.GetFloat("VerticalThreshold", 1.0f);
    verticalChange = ini.GetFloat("VerticalChange", 0.1f);
    modeA = 2;
    modeB = 2;
    unknown45c = 0;
    unknown614 = 0;
    selected = -1;
    font = 0;
    Layout();
    textA[0] = 0;
    textB[0] = 0;
}

void Overlay::Layout()
{
    IniReader ini;
    ini.SetSection("Interface");
    char fontName[28];
    strcpy(fontName, ini.GetString("FontName"));
    int size = ini.GetInt("FontSize", 12);
    g_fontFound = 0;
    HDC dc = GetDC(g_renderer->hwnd);
    LOGFONT lf;
    EnumFontFamilies(dc, fontName, (FONTENUMPROC)FontFound, (LPARAM)&lf);
    __int64 points = (__int64)size * GetDeviceCaps(dc, LOGPIXELSY);
    float pixels = points * (1.0f / 72.0f);
    if (g_fontFound) {
        int h = (int)pixels;
        lf.lfHeight = h < points ? -1 - h : -h;
        lf.lfWidth = 0;
        font = CreateFontIndirect(&lf);
        lineHeight = abs(lf.lfHeight) + 5;
    } else {
        DebugLog("ERROR: could not find font %s!\n", fontName);
        EnumFontFamilies(dc, "System", (FONTENUMPROC)FontFound, (LPARAM)&lf);
        if (g_fontFound) {
            int h = (int)pixels;
            lf.lfHeight = h < points ? -1 - h : -h;
            lf.lfWidth = 0;
            font = CreateFontIndirect(&lf);
            lineHeight = abs(lf.lfHeight) + 5;
        } else {
            lineHeight = 21;
        }
    }
    ReleaseDC(g_renderer->hwnd, dc);
    messageX = ini.GetInt("Messagex", 360);
    messageY = ini.GetInt("Messagey", 400);
    strcpy(textA, ini.GetString("Message"));
    maxMessageWidth = ini.GetInt("MaxMessageWidth", 120);
    maxMessageLines = ini.GetInt("MaxMessageHeight", 100) / lineHeight;
}
