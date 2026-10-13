// Mouse::Center (SQZ.EXE 0x4591b0, ret 4): move the pointer to the centre (or one axis of it) of the window.
#include "compat41.h"

struct IPoint { int x, y; };

struct Pointer {
    short x, y;                      // 0x00 0x02
    char pad[0x10 - 4];
    int width, height;               // 0x10 0x14
    int flag;                        // 0x18
    void Center(int how);
    void MoveTo(IPoint p);           // 0x459150
};

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
