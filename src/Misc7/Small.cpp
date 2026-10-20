// Joystick dead-zone read (SQZ.EXE 0x448240) and the awareness record's "take the nearer sighting" (0x448370).
#include "compat41.h"

struct Stick {
    int x0, y0;                 // 0x00 0x04 centre
    char pad[0x20 - 8];
    int x, y;                   // 0x20 0x24 raw position
    int deadZone;               // 0x28
    int Read(int *dx, int *dy);
};

static inline int Abs(int v) { return v < 0 ? -v : v; }

int Stick::Read(int *dx, int *dy)
{
    *dx = x - x0;
    *dy = y - y0;
    if (Abs(*dx) < deadZone)
        *dx = 0;
    if (Abs(*dy) < deadZone)
        *dy = 0;
    return 1;
}

struct Sighting {
    int id, w4, w8, wc;
    float distance;
    int stale;
    void Offer(const Sighting *o);
    void Take(const Sighting *o)
    {
        id = o->id;
        distance = o->distance;
        w4 = o->w4;
        w8 = o->w8;
        int t = o->wc;
        stale = 0;
        wc = t;
    }
};

void Sighting::Offer(const Sighting *o)
{
    if (stale || distance > o->distance)
        Take(o);
}
