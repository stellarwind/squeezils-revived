// Joystick-like axis readers (SQZ.EXE 0x459450 X, 0x4593d0 Y): position relative to the centre, dead zone removed, scaled to -1..1.
#include <stdlib.h>
struct Pos { int x, y; };
struct Pad {
    char pad0[0x10];
    int width;                  // +0x10
    int height;                 // +0x14
    char pad18[4];
    int dead;                   // +0x1c
    void GetPos(Pos *p);        // 0x459130
    float AxisX();              // 0x459450
    float AxisY();              // 0x4593d0
};
float Pad::AxisX()
{
    Pos p;
    GetPos(&p);
    int c = width / 2;
    int d = p.x - c;
    int a = abs(d);
    if (a < dead)
        return 0.0f;
    float r = (float)(a - dead) / (float)(c - dead);
    if (d < 0)
        return -r;
    return r;
}
float Pad::AxisY()
{
    Pos p;
    GetPos(&p);
    int c = height / 2;
    int d = c - p.y;
    int a = abs(d);
    if (a < dead)
        return 0.0f;
    float r = (float)(a - dead) / (float)(c - dead);
    if (d < 0)
        return -r;
    return r;
}
