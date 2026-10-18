// Registry construction and destruction, SQZ.EXE 0x477e60 / 0x477ea0.
#include "WindowCallbacks/WindowCallbacks.h"

WindowCallbacks::WindowCallbacks()
{
    if (!gWindowCallbacks) {
        for (int i = 0; i < 16; ++i) {
            slots[i].handler = 0;
            slots[i].context = 0;
        }
        accelerators = 0;
        quitStatus = 0;
        gWindowCallbacks = this;
    }
}

WindowCallbacks::~WindowCallbacks()
{
    for (int i = 0; i < 16; ++i) {
        slots[i].handler = 0;
        slots[i].context = 0;
    }
    accelerators = 0;
    quitStatus = 0;
    gWindowCallbacks = 0;
}
