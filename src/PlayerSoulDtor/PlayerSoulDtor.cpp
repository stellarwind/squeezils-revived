// ~SqueezilPlayerSoul (SQZ.EXE 0x41ef80, deleting form 0x41ef60): clear the global player pointer, base dtor 0x44f600 in the unwind path.
#include "compat41.h"
struct B0 { virtual ~B0(); };
struct B4 { virtual ~B4(); };
struct SqueezilSoulB : B0, B4 { char d[0x78]; virtual ~SqueezilSoulB(); };   // 0x44f600, 0x80 bytes
struct InputPart { virtual void i0(); };                              // vptr at 0x80
struct SqueezilPlayerSoulD : SqueezilSoulB, InputPart {
    virtual ~SqueezilPlayerSoulD();
};
extern SqueezilPlayerSoulD *g_thePlayer;                              // 0x4bad08
SqueezilPlayerSoulD::~SqueezilPlayerSoulD()
{
    g_thePlayer = 0;
}
SqueezilPlayerSoulD *Force(SqueezilPlayerSoulD *p) { delete p; return p; }
