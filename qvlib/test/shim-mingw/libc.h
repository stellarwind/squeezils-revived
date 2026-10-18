// Compatibility declarations for the unmodified SGI sources under MinGW.
#ifndef QV_TEST_MINGW_LIBC_H
#define QV_TEST_MINGW_LIBC_H
#include <stdlib.h>
#include <string.h>
typedef unsigned long u_long;
typedef unsigned short u_short;
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_4
#define M_PI_4 0.78539816339744830962
#endif
#endif
