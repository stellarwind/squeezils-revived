// A delayed script message (SQZ.EXE 0x422450 dtor, 0x4224b0 Tick): fires its text through 0x422ce0 once the timer runs out.
#include "Misc16/DelayedMessage.h"
void PostMessageText(const char *text, int a, int b, int c);      // 0x422ce0 (cdecl)
DelayedMessage::~DelayedMessage() { if (payload) delete payload; }
int DelayedMessage::Tick(float dt)
{
    if (fired)
        return 0;
    timer -= dt;
    if (timer > 0)
        return 0;
    PostMessageText(name.c_str(), (int)payload, arg, 0);
    delete payload;
    payload = 0;
    return 1;
}

int DelayedMessage::TickFrame()
{
    if (!fired)
        return 0;
    timer -= 1.0f;
    if (timer > 0)
        return 0;
    PostMessageText(name.c_str(), (int)payload, arg, 0);
    delete payload;
    payload = 0;
    return 1;
}
