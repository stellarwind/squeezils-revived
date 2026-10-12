// Input dispatch of the application object (SQZ.EXE 0x43b0d0, ret 8): while a modal handler is active only key 0x17
// is let through; otherwise try the mapped handler, then the default one, then the fallback.
#include "compat41.h"

struct App {
    char pad[0x78];
    int modal;                       // 0x78
    char pad2[0xb4 - 0x7c];
    int mapped;                      // 0xb4
    int Mapped(int code, int arg);   // 0x43b120
    int Default(int code, int arg);  // 0x43b5a0
    int Fallback(int code, int arg); // 0x43b7c0
    int Dispatch(int code, int arg);
};

int App::Dispatch(int code, int arg)
{
    if (modal && code != 0x17)
        return 0;
    int r;
    if (mapped)
        r = Mapped(code, arg);
    else
        r = Default(code, arg);
    if (!r)
        r = Fallback(code, arg);
    return r;
}
