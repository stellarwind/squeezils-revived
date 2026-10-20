// AnimateCameraPathFunction (vtable 0x49ecfc): fly the camera along a named path.
#pragma once
#include "FunctionExec/Exec.h"
#include "FunctionMisc/MiscClasses.h"

struct CamArgList {
    virtual String ReadString(int flag);                 // slot 0
};
struct PathHandle { int id; };
void *FindPath(const char *name);                        // 0x42cc10 (cdecl)
struct Simulation2 { void *GetSoul(int id); };           // 0x414080
Simulation2 *GetSimulation();                            // 0x420690 (cdecl)
void *DynamicCast(void *p, int zero, const void *from, const void *to, int ref);   // 0x48f060 (cdecl)
extern const char kSoulType[];                           // 0x4b93e8
extern const char kCameraSoulType[];                     // 0x4b98b0
struct CameraSoul { void FollowPath(void *path, int a, int b, float c); };   // 0x409a50

class AnimateCameraPathFunction : public Function {
public:
    AnimateCameraPathFunction() { a = 0; b = 0; c = 0; }
    ~AnimateCameraPathFunction() { delete a; delete b; delete c; }   // inline: 0x44e3f0 is the scalar deleting form
    virtual int Execute(ScriptCall *args, int extra);    // 0x44e560
    virtual Function *Create(ArgList *list);             // 0x44e2c0
    virtual void Print();                                // 0x44e500
    virtual const char *Name();                          // 0x44e3e0
    IntFunctionArgument *a;                              // 0x04
    IntFunctionArgument *b;                              // 0x08
    FloatFunctionArgument *c;                            // 0x0c
    String path;                                         // 0x10
};
