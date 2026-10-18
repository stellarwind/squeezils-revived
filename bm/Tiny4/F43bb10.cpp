// 0x43bb10: poll the key bindings (device at +4 maps action -> virtual key) into two "held" flags.
#include <windows.h>
struct KeyMap43 {
    virtual void v0();
    virtual int VirtualKey(int action);      // slot 1 (+4)
};
struct Keys43bb10 {
    int f0;
    KeyMap43 *map;      // 0x04
    char pad[0x3c - 8];
    int heldA;          // 0x3c
    char pad2[0x50 - 0x40];
    int heldB;          // 0x50
    void Poll();
};
void Keys43bb10::Poll()
{
    if (GetKeyState(map->VirtualKey(0)) < 0 || GetKeyState(map->VirtualKey(1)) < 0)
        heldA = 1;
    else
        heldA = 0;
    if (GetKeyState(map->VirtualKey(5)) < 0 || GetKeyState(map->VirtualKey(6)) < 0)
        heldB = 1;
    else
        heldB = 0;
}
