// ~SqueezilSoul (SQZ.EXE 0x44f600, vtables 0x49f74c / 0x49f72c): the members and bases are modelled by out-of-line destructors.
#include "compat41.h"
struct Base0 { virtual ~Base0(); };                                // 0x450e40
struct Base4 { virtual ~Base4(); };                                // 0x450980
struct Part10 { char d[0x38]; ~Part10(); };                        // 0x456e60 (0x10..0x48)
struct Part60 { char d[0x0c]; ~Part60(); };                        // 0x44f590 (0x60..0x6c)
struct PhysCore { char d[0x2a0]; ~PhysCore(); };                  // 0x4052b0
struct PhysBlock : PhysCore { ~PhysBlock() {} };                   // 0x405260 (inline wrapper)
struct Carried { virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void Destroy(int freeMem); };   // slot 4
struct SqueezilSoulD : Base0, Base4 {
    char pad8[8];                                                  // 0x08
    Part10 part10;                                                 // 0x10
    PhysBlock *physics;                                            // 0x48
    char pad4c[0x60 - 0x4c];
    Part60 part60;                                                 // 0x60
    Carried *carried;                                              // 0x6c
    virtual ~SqueezilSoulD();                                      // 0x44f600
};
SqueezilSoulD::~SqueezilSoulD()
{
    if (carried) {
        carried->Destroy(1);
        carried = 0;
    }
    if (physics) {
        delete physics;
        physics = 0;
    }
}
