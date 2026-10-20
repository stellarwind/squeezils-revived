// 0x448170 (ret 4): read the joystick, map the two axes to screen-space coordinates.
struct Stick {
    char pad[0x18];
    float scaleX, scaleY;       // 0x18 0x1c
    int rawX, rawY;             // 0x20 0x24
    int Read(int *x, int *y);           // 0x4481f0
    void Centered(int *a, int *b);      // 0x448240
    int OffsetX();                      // 0x4482a0
    int OffsetY();                      // 0x4482c0
    int Position(int *out);
};
int Stick::Position(int *out)
{
    int a, b;
    Read(&rawX, &rawY);
    b = 0;
    Centered(&a, &b);
    out[0] = (int)(a / scaleX) + OffsetX();
    out[1] = (int)(b / scaleY) + OffsetY();
    return 0;
}
