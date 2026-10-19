// SqueezilSoul (vtable 0x49f74c): flat layout for the larger members (0x4504f0-0x456d90).
// Offsets and names from Repo/src/SqueezilSoul.h; only what the ported functions touch is declared.
#pragma once
#include "compat41.h"

class String {
public:
    String(const char *s);                  // 0x438380
    String(const String &o);                // 0x4383b0
    ~String();                              // 0x4383d0
    String &operator=(const String &o);     // 0x438670
    char *rep;
};

struct Contact;

struct SurfaceInfo {                        // the soul's current surface type (+0x60)
    int    type;
    String typeName;
    String value;
    ~SurfaceInfo();                         // 0x44f590
};

struct NavSurfaces {
    void TypeChanged(void *soul, Contact *c, SurfaceInfo *old, SurfaceInfo *now);   // 0x412920
};

struct Place {
    NavSurfaces *Surfaces();              // 0x439b60
};

struct Vector3f {
    float x, y, z;
    Vector3f(const Vector3f &o);            // 0x4183c0
    ~Vector3f();                            // 0x401660
};

struct Matrix {
    Vector3f *Position();                   // 0x41faf0
};

void PlayEffect(String name, Vector3f at); // 0x44a250

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2();
    virtual Matrix *GetMatrix();            // slot 3, +0xc
    virtual void b4(); virtual void b5(); virtual void b6();
    virtual Contact *GetContact();          // slot 7, +0x1c
    virtual void b8(); virtual void b9(); virtual void b10(); virtual void b11();
    virtual void b12(); virtual void b13(); virtual void b14();
    virtual int  IsOnGround();              // slot 15, +0x3c
    virtual void b16(); virtual void b17(); virtual void b18(); virtual void b19();
    virtual int  GetMode();                 // slot 20, +0x50
    virtual void SetMode(int clip, int blend, int flag);   // slot 21, +0x54
};

struct Inventory {
    void RemoveNuts();                      // 0x4164a0
};

struct Possessions {
    Inventory *GetInventory();              // 0x448990
};

extern int kLoopingSoundPlaying;            // 0x4bd960
void StopLoopingSounds();                   // 0x44a8f0

struct SqueezilPhysics {
    char  pad[0x278];
    float spitVelocity;                     // 0x278
};

struct SqueezilSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual SoulBody *Body();               // slot 17, +0x44
    virtual Possessions *GetPossessions();  // slot 18, +0x48
    virtual void s19(); virtual void s20(); virtual void s21();
    virtual void s22(); virtual void s23();
    virtual void SetState(int state, int blend);   // slot 24, +0x60
    virtual int  GetState();                // slot 25, +0x64
    virtual void s26(); virtual void s27(); virtual void s28(); virtual void s29();
    virtual void s30(); virtual void s31(); virtual void s32(); virtual void s33();
    virtual void s34(); virtual void s35();
    virtual void SpitNut(float speed);      // slot 36, +0x90
    char pad04[0x8 - 4];
    int  finished;                          // 0x8
    char pad0c[0x38 - 0xc];
    int  stateAtDeath;                      // 0x38
    char pad3c[0x44 - 0x3c];
    int  frozen;                            // 0x44
    SqueezilPhysics *physics;               // 0x48
    char pad4c[0x54 - 0x4c];
    float spitStrength;                     // 0x54
    int   spitRequested;                    // 0x58
    char pad5c[0x60 - 0x5c];
    SurfaceInfo surface;                    // 0x60
    void *possessions;                      // 0x6c
    Place *place;                           // 0x70

    void Die(int how);                      // 0x450a00 slot 13
    void ResetSurface();                    // 0x451500 slot 21
    int  Spit();                            // 0x4569c0 slot 29
};
