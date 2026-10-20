// SqueezilBody::~SqueezilBody (0x457380): detaches its frame from the character and the registry, then deletes the character.
struct Mat64 { float m[16]; ~Mat64(); };                   // 0x402310
struct Chr { virtual void v0(); virtual void v1(); virtual void Detach(void *frame); ~Chr(); };   // dtor 0x41dc70
struct Registry { void *Find(void *frame); void Release(void *h); };   // 0x477290, 0x4773e0
extern Registry *g_reg;                                    // 0x4c75e0
class BodyB {
public:
    virtual ~BodyB();                                      // 0x40bae0
    virtual void s1();
    char pad[0xb8 - 4];
};
class SqBody : public BodyB {
public:
    float r0, r1;                                          // 0xb8, 0xbc
    Mat64 a, b, c, d, e;                                   // 0xc0 .. 0x1c0
    char pad2[0x220 - 0x200];
    void *frame;                                           // 0x220
    int pad224, pad228;
    Chr *chr;                                              // 0x22c
    int mode;                                              // 0x230
    virtual ~SqBody();
};
SqBody::~SqBody()
{
    if (mode != 0x31)
        chr->Detach(frame);
    delete chr;
    g_reg->Release(g_reg->Find(frame));
}
