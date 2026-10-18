// 0x43a6a0: constructor of the sound/media holder (vtable 0x49e304): probe the system, then clear every field.
void FatalError(const char *a, const char *b);   // 0x41eae0
struct Holder43a6a0 {
    virtual void v0();
    int f4, f8, fc, f10, f14, f18, f1c;
    char pad1[0x3c - 0x20];
    int f3c, f40;
    char pad2[0x50 - 0x44];
    int f50, f54;
    char pad3[0x64 - 0x58];
    int f64, f68, f6c, f70, f74, f78, f7c;
    char pad4[0x88 - 0x80];
    int f88, f8c, f90, f94;
    char pad5[0xa0 - 0x98];
    int fa0;
    char pad6[0xb4 - 0xa4];
    int fb4;
    int Probe();                // 0x43c250
    Holder43a6a0();
};
Holder43a6a0::Holder43a6a0()
{
    if (!Probe())
        FatalError("Holder: required component missing", "Holder");
    f64 = 0; f68 = 0; f74 = 0; f18 = 0; f1c = 0; f14 = 0; fa0 = 0; f78 = 0; fc = 0; f7c = 0;
    f10 = 0; f4 = 0; f8 = 0; f3c = 0; f40 = 0; f50 = 0; f54 = 0; f70 = 0; f6c = 0;
    f88 = 0; f8c = 0; f90 = 0;
    fb4 = 1;
    f94 = 0;
}
