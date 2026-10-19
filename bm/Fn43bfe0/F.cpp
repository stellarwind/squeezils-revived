// 0x43bfe0 / 0x43c0c0: a control axis normalised against its centre: joystick (stick) and mouse variants.
struct Pt { int x, y; };
struct Dead { int Limit(); };                                   // 0x4593c0
struct StickS { void Position(Pt *p); };                        // 0x448170
struct Ctl {
    char pad[8];
    Dead *dead;                                                 // 0x08
    char pad2[4];
    StickS *stick;                                              // 0x10
    int Center();                                               // 0x43aa30
    float Stickish();
};
float Ctl::Stickish()
{
    volatile float r;
    int d;
    float f;
    {
        Pt p;
        stick->Position(&p);
        d = p.x - Center();
    }
    int a = d < 0 ? -d : d;
    if (dead->Limit() > a)
        return 0.0f;
    f = (float)d;
    if (f > 0.0f) {
        if (Center() <= a) {
            *(int *)&r = 0x3f800000;
            return r;
        }
        r = f / Center();
        return r;
    }
    if (Center() <= a) {
        *(int *)&r = 0xbf800000;
        return r;
    }
    r = f / Center();
    return r;
}

// 0x43c0c0: the vertical twin (centre is 0x43aa40, measured from the centre upwards).
struct Ctl2 {
    char pad[8];
    Dead *dead;                                                 // 0x08
    char pad2[4];
    StickS *stick;                                              // 0x10
    int Center2();                                              // 0x43aa40
    float Vertical();
};
float Ctl2::Vertical()
{
    volatile float r;
    int d;
    float f;
    {
        Pt p;
        stick->Position(&p);
        d = Center2() - p.y;
    }
    int a = d < 0 ? -d : d;
    if (dead->Limit() > a)
        return 0.0f;
    f = (float)d;
    if (f > 0.0f) {
        if (Center2() <= a) {
            *(int *)&r = 0x3f800000;
            return r;
        }
        r = f / Center2();
        return r;
    }
    if (Center2() <= a) {
        *(int *)&r = 0xbf800000;
        return r;
    }
    r = f / Center2();
    return r;
}
