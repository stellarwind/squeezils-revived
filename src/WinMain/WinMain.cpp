// 0x43f310: WinMain - single-instance check, working directory and ini path, window class and main
// window, intro movie, renderer, splash, scene, game start, then the message/step loop and teardown.
#include <windows.h>
#include <string.h>
#include "IniReader/IniReader.h"

struct Movie {
    char pad[0x48];
    Movie();                                                            // 0x418b90
    int Open(const char *file, int a, int fullScreen, int fatal);        // 0x418ba0
    void Play();                                                        // 0x418c90
};

// SIZE 0x88
struct WindowCallbacks {
    void *slots[0x20];                                                  // 0x00
    HACCEL accelerators;                                                // 0x80
    int quitStatus;                                                     // 0x84 WM_QUIT wParam
    WindowCallbacks();                                                  // 0x477e60
    ~WindowCallbacks();                                                 // 0x477ea0
    int SetAccelerators(HACCEL accel);                                  // 0x477ed0
    int Add(void *callback, void *context);                             // 0x477ef0
    int PumpPending();                                                  // 0x477fd0: 0 = quit
};

// SIZE 0x9cc
struct Renderer {
    char pad0[0x18];
    HWND hwnd;                                                          // 0x18
    char pad1c[0x9cc - 0x1c];
    Renderer();                                                         // 0x474400
    ~Renderer();                                                        // 0x474520
    int Initialize(void *windowObj, HWND hwnd, int x, int y, int w, int h, int reqW, int reqH,
                   int bpp, int fullscreen, int stretch, const unsigned char *rgba);   // 0x474540
    void SetShades(int shades);                                         // 0x475040
    void SetTextureColors(int colors);                                  // 0x475060
    void SetTextureShades(int shades);                                  // 0x475080
    void UploadFrontPalette();                                          // 0x474e10
};

// SIZE 0x64
struct Splash {
    char pad[0x64];
    Splash();                                                           // 0x44e900
    ~Splash();                                                          // 0x44e9e0
    void Load(int which);                                               // 0x44ea30
    void Stop();                                                        // 0x44ee40
};

// SIZE 0x1c
struct Scene {
    char pad[0x1c];
    Scene();                                                            // 0x476720
    ~Scene();                                                           // 0x476740
    int Init(Renderer *renderer);                                       // 0x4768f0
};

struct Application {
    int Initialize(WindowCallbacks *callbacks);                         // 0x43a740
    void Cleanup();                                                     // 0x43a900
    void ResetClock();                                                  // 0x43c970
};

// SIZE 0x14: an empty growable list (never used outside WinMain)
struct GrowList {
    void *data;                                                         // 0x00
    int count;                                                          // 0x04
    int grow;                                                           // 0x08
    int capacity;                                                       // 0x0c
    int extra;                                                          // 0x10
    GrowList() { extra = 0; capacity = 0; count = 0; data = 0; grow = 20; }
};

int  CheckDesktopDepth(int *width, int *height);                        // 0x4743b0
void SetPlaceIni(const char *file);                                     // 0x438820
void OpenLog();                                                         // 0x419250
void EnableConsole();                                                   // 0x419280
void CreateDebugWindow(HWND parent, HINSTANCE instance);                // 0x419160
int DebugLogAll(const char *text, ...);                                     // 0x419290
void FillBackground(HDC dc, RECT *rect, int color);                     // 0x475380
int  LoadPaletteFile(const char *name, unsigned char *rgba);            // 0x474ca0
LRESULT CALLBACK ChainProc(HWND, UINT, WPARAM, LPARAM);                 // 0x478070
int  MainWindowProc(void *context, HWND, UINT, WPARAM, LPARAM);         // 0x43eea0
int  StartGame();                                                       // 0x420170
void Shutdown();                                                        // 0x4205a0
Application *GetApplication();                                          // 0x4206b0
void RunFrame();                                                        // 0x4206e0
void RunFrame2();                                                       // 0x420760
void SetPaused(int paused);                                             // 0x43ed80
void StepGame();                                                        // 0x43ee10

extern HINSTANCE g_hInstance;                // 0x4c73c8
extern HWND g_mainWnd;                       // 0x4c73cc
extern HDC g_mainDC;                         // 0x4c73d4
extern char kGameDirectory[];                // 0x4c73d8
extern char g_iniPath[];                     // 0x4c74d8
extern WindowCallbacks *g_callbacks;         // 0x4c75d8
extern Renderer *g_renderer;                 // 0x4c75dc
extern Scene *gWorldScene;                   // 0x4c75e0
extern Movie *g_moviePlayer;                 // 0x4c75e4
extern Splash *g_splash;                     // 0x4c75e8
extern char g_regKey[];                      // 0x4c75ec
extern int gWindowReady;                     // 0x4c76f0 (MainWindowProc context)
extern int gWindowHook;                      // 0x4c76f4 (ini OneSimPerRender)
extern int gWindowActive;                    // 0x4c76fc
extern int gPaused;                          // 0x4c7700
extern GrowList *g_list0;                    // 0x4c7704
extern GrowList *g_list1;                    // 0x4c7708
extern GrowList *g_list2;                    // 0x4c770c
extern int g_4c7718, g_4c7720, g_4c7724, g_4c7728, g_4c772c, g_4c7730, g_4c7734;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int nCmdShow)
{
    int screenW, screenH;
    char buf[124];

    if (CreateFileMapping((HANDLE)0xffffffff, 0, PAGE_READONLY, 0, 0x20, "MySqueezilsMap") == 0) {
        MessageBox(0, "Error creating Application", "Mapping", 0);
        ExitProcess(1);
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(0, "Nuts! Application is already running.", "Error", 0);
        ExitProcess(1);
    }
    if (!CheckDesktopDepth(&screenW, &screenH))
        ExitProcess(1);
    g_hInstance = hInstance;

    // A command line naming an .ini selects the place file; its directory becomes the working one.
    if (strlen(cmdLine) != 0) {
        SetPlaceIni("");
        strcpy(buf, cmdLine);
        char *ext = strstr(buf, ".ini");
        if (ext == 0)
            ext = strstr(buf, ".INI");
        if (strlen(buf) > 9 && ext != 0) {
            char *end = ext + 4;
            char saved = *end;
            *end = 0;
            SetPlaceIni(buf);
            *end = saved;
            char *dir;
            if (saved != 0 && strchr(end, '\\') != 0) {
                dir = ext + 5;
            } else {
                GetModuleFileName(0, buf, 0x7f);
                char *slash = strrchr(buf, '\\');
                if (slash != 0)
                    *slash = 0;
                dir = buf;
            }
            SetCurrentDirectory(dir);
        }
    }
    SetCurrentDirectory("sqz");
    GetCurrentDirectory(0x100, kGameDirectory);
    strcpy(g_iniPath, kGameDirectory);
    strcat(g_iniPath, "\\SQZ.INI");

    UINT debugOutput = GetPrivateProfileInt("Defaults", "DebugIniFileOutput", 0, g_iniPath);
    if (debugOutput == 1) {
        OpenLog();
        EnableConsole();
        AllocConsole();
    } else if (debugOutput == 2) {
        OpenLog();
        CreateDebugWindow(g_mainWnd, g_hInstance);
    }

    IniReader ini;
    ini.SetSection("Render");
    int windowW = ini.GetInt("WindowWidth", 640);
    int windowH = ini.GetInt("WindowHeight", 480);
    int renderW = ini.GetInt("RenderWidth", 320);
    int renderH = ini.GetInt("RenderHeight", 240);
    int center = ini.GetInt("CenterRenderWindow", 0);
    int stretch = ini.GetInt("StretchMode", 1);
    int fullScreen = ini.GetInt("FullScreen", 0);
    int shades = ini.GetInt("Shades", 8);
    int textureColors = ini.GetInt("TextureColors", 16);
    int textureShades = ini.GetInt("TextureShades", 8);
    char palette[20];
    strcpy(palette, ini.GetString("Palette"));
    ini.SetSection("Defaults");
    strcpy(g_regKey, ini.GetString("RegistryKey"));

    gWindowReady = 0;
    gWindowHook = ini.GetInt("OneSimPerRender", 0);
    gWindowActive = 1;
    gPaused = 0;
    g_list0 = new GrowList;
    g_list1 = new GrowList;
    g_list2 = new GrowList;
    g_4c7720 = 0;
    g_4c7724 = 0;
    g_4c7728 = 0;
    g_4c772c = 0;
    g_4c7718 = 0;
    g_4c7730 = 0;
    g_4c7734 = 0;
    HACCEL accel = LoadAccelerators(g_hInstance, "AppAccel");

    WNDCLASS wc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    wc.lpfnWndProc = ChainProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, "APPICON");
    wc.hCursor = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = "Squeezils";
    if (!RegisterClass(&wc))
        return 0;

    RECT rect;
    DWORD style, exStyle;
    if (!fullScreen) {
        rect.left = screenW / 2 - windowW / 2;
        rect.top = screenH / 2 - windowH / 2;
        if (rect.top <= 0)
            rect.top += 20;
        if (rect.left <= 0)
            rect.left += 2;
        rect.right = rect.left + windowW;
        style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        exStyle = WS_EX_DLGMODALFRAME;
        rect.bottom = rect.top + windowH;
    } else {
        style = WS_POPUP;
        exStyle = WS_EX_APPWINDOW;
        rect.left = 0;
        rect.top = 0;
        rect.right = windowW - 1;
        rect.bottom = windowH - 1;
    }
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);
    g_mainWnd = CreateWindowEx(exStyle, "Squeezils", "Squeezils", style, rect.left, rect.top,
                               rect.right - rect.left, rect.bottom - rect.top, 0, 0, hInstance, 0);
    if (g_mainWnd == 0)
        return 0;
    g_mainDC = GetDC(g_mainWnd);
    SetTextColor(g_mainDC, 0xffffff);
    SetBkColor(g_mainDC, 0);

    g_moviePlayer = new Movie;
    if (g_moviePlayer->Open(".\\movies\\intro.avi", 1, fullScreen, 0))
        g_moviePlayer->Play();

    g_callbacks = new WindowCallbacks;
    if (!g_callbacks->SetAccelerators(accel))
        return 0;

    RECT client;
    GetClientRect(g_mainWnd, &client);
    HDC dc = GetDC(g_mainWnd);
    const unsigned char *rgba = 0;
    FillBackground(dc, &client, 0);
    ReleaseDC(g_mainWnd, dc);
    unsigned char paletteData[1024];
    if (LoadPaletteFile(palette, paletteData))
        rgba = paletteData;

    int x, y;
    if (center) {
        x = (windowW - renderW) / 2;
        y = (windowH - renderH) / 2;
    } else {
        x = 0;
        y = 0;
    }
    g_renderer = new Renderer;
    if (!g_renderer->Initialize(g_callbacks, g_mainWnd, x, y, renderW, renderH, windowW, windowH, 8,
                                fullScreen, stretch, rgba))
        return 0;
    g_renderer->SetShades(shades);
    g_renderer->SetTextureColors(textureColors);
    g_renderer->SetTextureShades(textureShades);
    g_renderer->UploadFrontPalette();

    g_splash = new Splash;
    g_splash->Load(0);
    gWorldScene = new Scene;
    if (!gWorldScene->Init(g_renderer))
        return 0;
    if (!g_callbacks->Add((void *)MainWindowProc, &gWindowReady))
        return 0;
    if (!StartGame())
        return 0;
    if (!GetApplication()->Initialize(g_callbacks)) {
        GetApplication()->Cleanup();
        return 0;
    }

    DebugLogAll("Initialization complete.\n");
    gWindowReady = 1;
    ShowWindow(g_mainWnd, nCmdShow);
    UpdateWindow(g_mainWnd);
    g_splash->Stop();
    RunFrame();
    RunFrame2();
    SetPaused(1);
    GetApplication()->ResetClock();
    for (;;) {
        if (!g_callbacks->PumpPending())
            break;
        StepGame();
    }

    int result = g_callbacks->quitStatus;
    WinHelp(g_renderer->hwnd, ".\\help\\sqz.hlp", HELP_QUIT, 0);
    Shutdown();
    delete g_splash;
    delete gWorldScene;
    delete g_renderer;
    delete g_callbacks;
    delete g_moviePlayer;
    delete g_list0;
    delete g_list1;
    delete g_list2;
    return result;
}
