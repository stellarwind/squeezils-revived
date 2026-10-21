// More small functions: a call-then-return-0 thunk (SQZ.EXE 0x414fa0), the window-rect reader (0x42da30, ret 8)
// and the String output operator of the text stream (0x438770).
#include "compat41.h"
#include "String/String.h"
#include "Persist/Stream.h"

void RunAll();                               // 0x4143d0
int RunAllThunk()
{
    RunAll();
    return 0;
}

struct Window { void GetRect(int *rc); };    // 0x474fe0
struct App { char pad[1]; };
extern Window *g_window;                     // 0x4c75dc
void __stdcall WindowOrigin(int *x, int *y)
{
    int rc[4];
    g_window->GetRect(rc);
    *y = rc[3];
    *x = rc[0];
}

ByteStream *WriteElem(ByteStream *w, String *s)
{
    char *text = (char *)s->c_str();
    w->WriteString(&text);
    return w;
}
