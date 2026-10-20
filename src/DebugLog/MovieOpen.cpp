// Movie::Open, SQZ.EXE 0x418ba0: open an AVI through MCI and attach it to the main window.
#include <windows.h>
#include <mmsystem.h>

int DebugLogAll(const char *fmt, ...);        // 0x419290
int ErrorBox(void *movie, const char *fmt, ...);  // 0x418a60
struct App { char pad[0xc]; int unused; };
extern App *g_app;                             // 0x4c75dc
extern HWND g_mainWnd;                         // 0x4c73cc
extern const char kOpenFailed[];               // 0x4b9fec
int FatalErrorApp(App *app, const char *fmt, ...);   // 0x475510 (Renderer::FatalError, this = g_app)

struct Movie {
    int state;            // 0x00
    DWORD cb;             // 0x04
    MCIDEVICEID id;       // 0x08
    const char *type;     // 0x0c
    const char *name;     // 0x10
    DWORD alias;          // 0x14
    DWORD style;          // 0x18
    HWND parent;          // 0x1c
    DWORD wcb;            // 0x20
    HWND hwnd;            // 0x24
    int show;             // 0x28
    char pad[0x3c - 0x2c];
    UINT device;          // 0x3c
    int x, y;             // 0x40, 0x44
    int Open(const char *file, int a, int b, int fatal);
};

int Movie::Open(const char *file, int a, int b, int fatal)
{
    int err = -1;
    name = file;
    type = "avivideo";
    style = 0x40000000;
    parent = g_mainWnd;
    while (err) {
        err = mciSendCommand(0, MCI_OPEN, 0x2202, (DWORD)&cb);
        if (err) {
            int r;
            if (fatal)
                r = FatalErrorApp(g_app, kOpenFailed, file);
            else
                r = ErrorBox(this, kOpenFailed, file);
            if (r == 2) {
                DebugLogAll("movie %s.", file);
                return 0;
            }
        }
    }
    device = id;
    show = 1;
    hwnd = g_mainWnd;
    if (mciSendCommand(device, 0x841, 0x50002, (DWORD)&wcb)) {
        DebugLogAll("ERROR: Could not play movie\n");
        return 0;
    }
    x = a;
    y = b;
    return 1;
}
