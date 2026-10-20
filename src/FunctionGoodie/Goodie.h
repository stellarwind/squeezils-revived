// CreateGoodieAtPositionFunction (vtable 0x49e764): like CreateProp but makes a goodie (no prop name lookup).
#pragma once
#ifndef GOODIE_EXEC
#define V3_CTOR_OUTOFLINE
#endif
#define V3_COPYCTOR
#include "FunctionExec/Exec.h"
#include "Vector3f/Vector3f.h"

#ifdef GOODIE_EXEC
struct GoodieMatrix {                                    // 64 bytes: right, up, forward, position rows (0x440fa0)
    float m[16];
    GoodieMatrix() {}
    GoodieMatrix(const GoodieMatrix &o);                 // out of line (0x4099e0)
    ~GoodieMatrix();                                     // empty, out of line (0x402310)
    void Set(const Vector3f &r, const Vector3f &u, const Vector3f &f, const Vector3f &p)
    {
        m[0] = r.x; m[1] = r.y; m[2] = r.z; m[3] = 0;
        m[4] = u.x; m[5] = u.y; m[6] = u.z; m[7] = 0;
        m[8] = f.x; m[9] = f.y; m[10] = f.z; m[11] = 0;
        m[12] = p.x; m[13] = p.y; m[14] = p.z; m[15] = 1.0f;
    }
};
#endif
class Vector3fFunctionArgument : public FunctionArgument {   // vtable 0x49da94
public:
#ifdef GOODIE_EXEC
    Vector3f Get(int context);                           // 0x421800 (by value)
#endif
    Vector3fFunctionArgument();
    virtual void Print();                                // 0x4217c0
    Vector3f value;                                      // 0x0c
};
Vector3fFunctionArgument *ParseVectorArgument(ArgList *list, Vector3f fallback);   // 0x421560 (cdecl)

class CreateGoodieAtPositionFunction : public Function {
public:
    CreateGoodieAtPositionFunction() { name = 0; position = 0; direction = 0; }
    ~CreateGoodieAtPositionFunction() { delete name; delete position; delete direction; }
    virtual int Execute(ScriptCall *args, int extra);    // 0x440fa0
    virtual Function *Create(ArgList *list);             // 0x440ed0
    virtual void Print();                                // 0x440cc0
    virtual const char *Name();                          // 0x440ec0
    virtual void ParseExtra(ArgList *list);              // slot 5: 0x440c20
    virtual void DumpExtra();                            // slot 6: 0x440c10
    virtual CreateGoodieAtPositionFunction *Make();      // slot 7: 0x440c30
#ifdef GOODIE_EXEC
    virtual void Spawn(int extra, void *level, const char *name, GoodieMatrix *m);   // slot 8: 0x440e50
#endif
    StringFunctionArgument *name;                        // 0x04
    Vector3fFunctionArgument *position;                  // 0x08
    Vector3fFunctionArgument *direction;                 // 0x0c
};

class FloatFunctionArgument : public FunctionArgument {      // vtable 0x49da84
public:
    FloatFunctionArgument();
    virtual void Print();                                // 0x421410
    float value;                                         // 0x0c
    float Get(int context);                              // 0x421440
};
FloatFunctionArgument *ParseFloatArgument(ArgList *list);   // 0x421250 (cdecl)

class CreateGoodieAtPositionWithVelocityFunction : public CreateGoodieAtPositionFunction {
public:
    CreateGoodieAtPositionWithVelocityFunction() { velocity = 0; speed = 0; }
    ~CreateGoodieAtPositionWithVelocityFunction() { delete velocity; delete speed; }
    virtual const char *Name();                          // 0x4415f0
    virtual void ParseExtra(ArgList *list);              // 0x441220
    virtual void DumpExtra();                            // 0x441200
    virtual CreateGoodieAtPositionFunction *Make();      // 0x4412a0
#ifdef GOODIE_EXEC
    virtual void Spawn(int extra, void *level, const char *name, GoodieMatrix *m);   // slot 8: 0x441550
#endif
    Vector3fFunctionArgument *velocity;                  // 0x10
    FloatFunctionArgument *speed;                        // 0x14
};

