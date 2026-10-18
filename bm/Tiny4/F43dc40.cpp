// 0x43dc40: clear the 0x33 input bindings, then load them (falling back to the defaults).
#include <string.h>
struct Binding {
    int a;              // 0x00
    int b;              // 0x04
    char name[0x20];    // 0x08
    int c, d, enabled;  // 0x28 0x2c 0x30
    char pad[0x44 - 0x34];
};
struct Bindings43 {
    char pad[0x1c];
    Binding e[0x33];    // 0x1c
    int Load();         // 0x43ec70
    void Default(int i);    // 0x43df10
    void M43ebc0(int i);
    void M43dcd0(int i);
    void Init();
};
void Bindings43::Init()
{
    int i;
    for (i = 0; i < 0x33; i++) {
        e[i].b = 0;
        strcpy(e[i].name, "");
        e[i].c = 0;
        e[i].d = 0;
        e[i].a = 0;
        e[i].enabled = 1;
    }
    if (!Load()) {
        for (i = 0; i < 0x33; i++) {
            Default(i);
            M43ebc0(i);
            M43dcd0(i);
        }
    }
}
