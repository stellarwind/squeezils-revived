// Progress/movie player stop (SQZ.EXE 0x44ee40): run it to its end, release the stream, reset.
#include "compat41.h"

struct Stream { void Close(); };             // 0x473890
struct Player {
    virtual void v0();
    virtual void Step();                     // slot 1
    Stream *stream;                          // 0x04
    int active;                              // 0x08
    int cur, end;                            // 0x0c 0x10
    char pad[0x1c - 0x14];
    int open;                                // 0x1c
    char pad2[0x5c - 0x20];
    int mark;                                // 0x5c
    float rate;                              // 0x60
    void Reset();                            // 0x44f080
    void Stop();
};

void Player::Stop()
{
    rate = 1.0f;
    while (active && cur < end)
        Step();
    Stream *s = stream;
    if (s) {
        s->Close();
        delete s;
        stream = 0;
    }
    open = 0;
    Reset();
    mark = -1;
}
