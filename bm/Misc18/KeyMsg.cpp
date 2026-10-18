// SQZ.EXE 0x43af40: keyboard message handler: translates the key through the key table; unmapped keys are swallowed unless Alt is held.
struct KeyTab { int Translate(int msg, int wp, int lp, int a, int b); int Code(int slot); };   // 0x43d7a0, 0x43dc20
struct Pad2 { int Translate(int msg, int wp, int lp); };    // 0x447890
struct Input13 {
    int pad0;
    KeyTab *tab;                // +4
    char pad8[0x10 - 8];
    Pad2 *pad;                  // +0x10
    char pad14[0x74 - 0x14];
    int a74;
    char pad78[0xb4 - 0x78];
    int ab4;
    int Handle(int code, int down);             // 0x43b0d0
    int OnKey(int msg, int wp, int lp);
    int OnPad(int msg, int wp, int lp);          // 0x43b060
};
int Input13::OnKey(int msg, int wp, int lp)
{
    int down;
    if (msg == 0x100 || msg == 0x104)
        down = 1;
    else
        down = 0;
    int code = tab->Translate(msg, wp, lp, ab4, a74);
    if (code != -1 && code != 0x33)
        return Handle(code, down);
    return msg == 0x104;
}

int Input13::OnPad(int msg, int wp, int lp)
{
    int down = msg == 0x3b5;
    int code = pad->Translate(msg, wp, lp);
    if (code == -1)
        return 0;
    for (int i = 0; i < 0x33; i++)
        if (tab->Code(i) == code)
            Handle(i, down);
    return 1;
}
