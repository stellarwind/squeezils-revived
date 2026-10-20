// Input dispatch of the application object (SQZ.EXE 0x43b0d0, ret 8): while a modal handler is active only key 0x17
// is let through; otherwise try the mapped handler, then the default one, then the fallback.
#include "AppInput.h"

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
