// 0x40a5e0 / 0x40b2f0 (two event handlers, same shape) and 0x40c860 (cached 64 byte compare).
#include <string.h>
struct Sx {
    virtual void s0();
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual void s8();
    virtual void s9();
    virtual void s10();
    virtual void s11();
    virtual void s12();
    virtual void s13();
    virtual void s14();
    virtual void s15();
    virtual void s16();
    virtual void s17();
    virtual void s18();
    virtual void s19();
    virtual void s20();
    virtual void s21();
    virtual void s22();
    virtual void s23();
    virtual void s24();
    virtual int GetState();   // 25, +0x64
};
extern int g_a64fc;   // 0x4c64fc
extern int g_b649c;   // 0x4c649c

struct HandlerA {
    int pad0;
    int pad1;
    int pad2;
    Sx *soul;                 // +0xc
    char pad3[0x20 - 0x10];
    int armed;                // +0x20
    char pad4[0x28 - 0x24];
    int target;               // +0x28
    char pad5[0x144 - 0x2c];
    int flag;                 // +0x144
    void Handle(int kind, int arg);
};

void HandlerA::Handle(int kind, int arg)
{
    if (kind == 0xe && armed) {
        if (soul->GetState() != 0x13) {
            flag = 1;
            target = g_a64fc;
        }
    }
}

struct HandlerB {
    int pad0;
    int pad1;
    int pad2;
    Sx *soul;                 // +0xc
    int enabled;              // +0x10
    char pad3[0x20 - 0x14];
    int armed;                // +0x20
    char pad4[0x28 - 0x24];
    int target;               // +0x28
    char pad5[0x144 - 0x2c];
    int flag;                 // +0x144
    void Handle(int kind, int arg);
};

void HandlerB::Handle(int kind, int arg)
{
    if (enabled) {
        if (kind == 0xe && armed) {
            if (soul->GetState() != 0x13) {
                flag = 1;
                target = g_b649c;
            }
        }
    }
}

struct Cmp64 {
    int pad;
    char a[0x44];             // +4
    char b[0x44];             // +0x48
    int dirty;
    int result;
    int Compare();
};

int Cmp64::Compare()
{
    if (dirty) {
        result = memcmp(b, a, 64);
        dirty = 0;
    }
    return result;
}

struct MatHolder {
    int pad;
    int m[16];
    void Set(const int *src);
};

void MatHolder::Set(const int *src)
{
    m[0] = src[0];
    m[1] = src[1];
    m[2] = src[2];
    m[3] = src[3];
    m[4] = src[4];
    m[5] = src[5];
    m[6] = src[6];
    m[7] = src[7];
    m[8] = src[8];
    m[9] = src[9];
    m[10] = src[10];
    m[11] = src[11];
    m[12] = src[12];
    m[13] = src[13];
    m[14] = src[14];
    m[15] = src[15];
}
