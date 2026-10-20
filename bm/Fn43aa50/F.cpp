void PadDecl1();
void PadDecl2();
void PadDecl3();
// 0x43aa50: window-message handler of the game shell (activation, size, Alt-Enter, system keys, menu/input dispatch).
#include <windows.h>
struct Input { int Process(unsigned msg, unsigned wp, unsigned lp, unsigned *out); };    // 0x406420
struct Display { void Set(int on); };                                                    // 0x4592f0
struct Rend {
    char pad[0xc];
    int fullscreen;         // 0xc
    char pad2[0x18 - 0x10];
    HWND hwnd;              // 0x18
    void ToWindow();        // 0x474b70
    void ToFullscreen();    // 0x474bb0
};
extern Rend *g_renderer;                                     // 0x4c75dc
void SetFocusFlag(int on);                                   // 0x43ed80
struct Shell {
    char pad[8];
    Display *display;       // 0x8
    Input *input;           // 0xc
    char pad2[0x78 - 0x10];
    int active;             // 0x78
    int ready;              // 0x7c
    char pad3[0x98 - 0x80];
    int state;              // 0x98
    int blocked;            // 0x9c
    void OnSize();          // 0x43c970
    void OnPaint();         // 0x43a9a0
    unsigned HandleA(unsigned msg, unsigned wp, unsigned lp);   // 0x43af40
    unsigned HandleB(unsigned msg, unsigned wp, unsigned lp);   // 0x43afb0
    unsigned HandleC(unsigned msg, unsigned wp, unsigned lp);   // 0x43b060
};
unsigned __cdecl ShellProc(int unused, unsigned msg, unsigned wp, unsigned lp, Shell *shell)
{
    unsigned result;
    if (!shell)
        return 0;
    switch (msg) {
    case 0x20:
        if (shell->ready) {
            if (shell->active == 0 && shell->state != 1) {
                shell->display->Set(0);
                return 1;
            }
            shell->display->Set(1);
            return 1;
        }
        break;
    case 5:
        shell->OnSize();
        if (wp == 0 || wp == 1) {
            if (shell->active)
                return 0;
            shell->active = 1;
            SetFocusFlag(1);
        }
        return 1;
    case 6:
        if (shell->ready == 0 && (short)wp != 0)
            shell->OnPaint();
        break;
    case 8:
        SetFocusFlag(1);
        return 1;
    case 0x104:
        if (wp == 0xd) {
            if (g_renderer->fullscreen == 1) {
                g_renderer->ToFullscreen();
                return 1;
            }
            g_renderer->ToWindow();
            return 1;
        }
        break;
    }
    if (shell->ready == 0)
        return 0;
    if ((shell->active && shell->blocked == 0 && GetActiveWindow() == g_renderer->hwnd) || msg == 0x112 || msg == 0x104) {
        unsigned out;
        int r = shell->input->Process(msg, wp, lp, &out);
        switch (r) {
        case 0:
            if ((msg != 0x112 || (wp & 0xfff0) != 0xf060) && (msg != 0x104 || wp != 0x73))
                return 0;
            return 1;
        case 1:
            DestroyWindow(g_renderer->hwnd);
            return 1;
        case 2:
            SetFocusFlag(0);
            return 0;
        }
    }
    result = shell->HandleA(msg, wp, lp);
    if (result != 1) {
        if (shell->state == 0)
            return 0;
        if ((shell->state != 2 || (result = shell->HandleC(msg, wp, lp)) != 1) && shell->state == 1)
            result = shell->HandleB(msg, wp, lp);
    }
    return result;
}
