// spare declarations: the compiler's register choices follow the symbol count (tools/flagsearch/padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
// 0x478070: window procedure that offers every message to a table of 16 handler slots (last slot first) before DefWindowProc.
#include <windows.h>
typedef int (*HandlerFn)(HWND, UINT, WPARAM, LPARAM, int ctx);
struct Slot { HandlerFn fn; int ctx; };
extern char *g_slots;                                  // 0x4c1afc
LRESULT CALLBACK ChainProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (g_slots == 0)
        return DefWindowProc(hWnd, msg, wParam, lParam);
    int i = 0x78;
    int handled = 0;
    do {
        Slot *s = (Slot *)(g_slots + i);
        HandlerFn fn = s->fn;
        int ctx = s->ctx;
        if (fn) {
            int r = fn(hWnd, msg, wParam, lParam, ctx);
            switch (r) {
            case 1:
                handled = 1;
                break;
            case 2:
                return 0;
            }
        }
        i -= 8;
    } while (i >= 0);
    if (!handled)
        return DefWindowProc(hWnd, msg, wParam, lParam);
    return 0;
}
