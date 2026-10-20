// SQZ.EXE 0x43ba40: clear latched commands whose mapped keys were released.
#include <windows.h>

struct ReleaseKeyMap {
    virtual void slot0();
    virtual int VirtualKey(int action);                 // slot 1
};
struct ReleaseKeyController {
    virtual void slot0(); virtual void slot1(); virtual void slot2();
    virtual void slot3(); virtual void slot4(); virtual void slot5();
    virtual void SetAction3(int value);                 // slot 6
    virtual void slot7();
    virtual void SetAction4(int value);                 // slot 8
};
struct ReleaseKeyInput {
    char pad00[4];
    ReleaseKeyMap *map;                                 // +0x04
    char pad08[0x18 - 8];
    ReleaseKeyController *controller;                   // +0x18
    char pad1c[0x64 - 0x1c];
    int action2Held, action8Held, action3Held, action4Held, action9Held;
    void ReleaseAction2();                              // 0x43bba0
    void ReleaseAction8();                              // 0x43bbc0
    void ReleaseAction4();                              // 0x43bbd0
    void ReleaseAction3();                              // 0x43bbf0
    void PollHeldKeys();                                // 0x43bb10
    void ReleaseKeys();
};

void ReleaseKeyInput::ReleaseKeys()
{
    if (action2Held && GetKeyState(map->VirtualKey(2)) >= 0)
        ReleaseAction2();
    if (action8Held && GetKeyState(map->VirtualKey(8)) >= 0)
        ReleaseAction8();
    if (action4Held && GetKeyState(map->VirtualKey(4)) >= 0) {
        ReleaseAction4();
        controller->SetAction4(0);
    }
    if (action9Held && GetKeyState(map->VirtualKey(9)) >= 0)
        action9Held = 0;
    if (action3Held && GetKeyState(map->VirtualKey(3)) >= 0) {
        ReleaseAction3();
        controller->SetAction3(0);
    }
    PollHeldKeys();
}
