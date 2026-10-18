// ClientBitmap::Free, SQZ.EXE 0x478170.
#include <windows.h>
#include "Misc/Misc.h"

void ClientBitmap::Free()
{
    if (f8) {
        DeleteObject((HGDIOBJ)f8);
        f8 = 0;
    }
    if (fc) {
        DeleteObject((HGDIOBJ)fc);
        fc = 0;
    }
    if (f0) {
        DeleteDC((HDC)f0);
        f0 = 0;
    }
    f4 = 0;
    f10 = 0;
    f14 = 0;
}
