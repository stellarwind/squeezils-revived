// 0x43afb0 (ret 0xc): route a mouse message to the pointer widget, then find which of the 0x33 bound buttons it hit.
struct Pointer459 { int OnMouse(unsigned msg, unsigned wparam, int lparam); };   // 0x459090
struct Bindings { int Button(int i); };                                          // 0x43dc20
struct MouseRouter {
    int f0;
    Bindings *bindings;     // 0x04
    Pointer459 *pointer;    // 0x08
    int Press(int i, int down);     // 0x43b0d0
    int OnMessage(unsigned msg, unsigned wparam, int lparam);
};
int MouseRouter::OnMessage(unsigned msg, unsigned wparam, int lparam)
{
    int which, down, d;
    int i;
    if (pointer->OnMouse(msg, wparam, lparam) == -1)
        return 0;
    if (msg != 0x201 && msg != 0x202) {
        which = d;
        down = d;
    } else {
        which = 1;
        down = msg == 0x201;
    }
    if (msg == 0x204 || msg == 0x205) {
        which = 2;
        down = msg == 0x204;
    }
    for (i = 0; i < 0x33; i++) {
        if (bindings->Button(i) == which)
            return Press(i, down);
    }
    return 0;
}
