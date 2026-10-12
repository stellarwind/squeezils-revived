// Frees ten stream objects held as five pairs plus two singles (SQZ.EXE 0x4143d0; 0x4143c0 jumps here).
#include "compat41.h"

struct Stream { ~Stream(); };                // 0x473890
struct StreamSet {
    Stream *a[2];                            // 0x00
    Stream *b[2];                            // 0x08
    Stream *c[2];                            // 0x10
    Stream *d[2];                            // 0x18
    Stream *e[2];                            // 0x20
    Stream *f, *g;                           // 0x28 0x2c
    void FreeAll();
};

void StreamSet::FreeAll()
{
    for (int i = 0; i < 2; i++) {
        delete a[i];
        delete b[i];
        delete e[i];
        delete d[i];
        delete c[i];
    }
    delete f;
    delete g;
}
