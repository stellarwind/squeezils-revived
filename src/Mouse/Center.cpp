// Mouse::Center (SQZ.EXE 0x4591b0, ret 4): move the pointer to the centre (or one axis of it) of the window.
#include "compat41.h"

#include "Mouse/Pointer.h"

void Pointer::Center(int how)
{
    IPoint p;
    p.x = this->x;
    p.y = this->y;
    if (how != 0) {
        if (how != 1) { p.x = width / 2; p.y = height / 2; }
        else p.y = height / 2;
    } else {
        p.x = width / 2;
        if (flag == 0) p.y = height / 2;
    }
    MoveTo(p);
}
