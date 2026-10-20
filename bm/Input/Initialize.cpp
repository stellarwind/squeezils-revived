// 0x43a740: attach input listeners and create the keyboard and menu controllers.
#include <windows.h>
#include "ListenerTable/ListenerTable.h"
struct InitKeys { char data[0xdac]; InitKeys(); }; // 0x43c9e0
struct InitControls { char data[0x24]; InitControls(); void Initialize(); }; // 0x459050, 0x459080
struct InitMenuData { char data[0x48]; InitMenuData(); }; // 0x447810
struct InitMenu {
    char data[400];
    InitMenu(); // 0x405e90
    int Initialize(InitControls *controls, InitMenuData *settings); // 0x405ff0
};
struct InitRenderer {
    char pad[0x18];
    HWND hwnd;
    void GetClientRectInWindow(RECT *rect); // 0x474f70
};
extern InitRenderer *g_inputRenderer; // 0x4c75dc
void InputControllerCallback(); // 0x43aa50, address registered with the window
struct InputInitializer {
    int pad0;
    InitKeys *keys;
    InitControls *controls;
    InitMenu *menu;
    InitMenuData *menuSettings;
    ListenerTable *window;
    char pad18[0x20 - 0x18];
    RECT rect;
    char pad30[0x64 - 0x30];
    int state64, state68;
    char pad6c[0x98 - 0x6c];
    int state98, state9c;
    int Initialize(ListenerTable *wnd);
    int Shutdown(); // 0x43a900
    void Activate(); // 0x43a9a0
};
int InputInitializer::Initialize(ListenerTable *wnd)
{
    if (!wnd->Add((void *)InputControllerCallback, this))
        return 0;
    window = wnd;
    g_inputRenderer->GetClientRectInWindow(&rect);
    keys = new InitKeys;
    controls = new InitControls;
    controls->Initialize();
    menuSettings = new InitMenuData;
    state9c = 0;
    menu = new InitMenu;
    if (!menu->Initialize(controls, menuSettings)) {
        Shutdown();
        return 0;
    }
    state98 = 0;
    state64 = 0;
    state68 = 0;
    if (GetActiveWindow() == g_inputRenderer->hwnd)
        Activate();
    return 1;
}
