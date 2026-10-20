// WrlLog (SQZ.EXE 0x47a7b0): the VRML reader's printf-style log through the optional callback at 0x4c3c4c.
// The QvLib objects were compiled without -G5 and without -GR (see the #FLAGS line of the targets file).
#include "compat41.h"
#include <stdarg.h>
#include <stdio.h>

typedef void (*LogFn)(const char *);
extern LogFn g_wrlLog;      // 0x4c3c4c

void WrlLog(const char *fmt, ...)
{
    char buf[256];
    if (g_wrlLog) {
        va_list ap;
        va_start(ap, fmt);
        vsprintf(buf, fmt, ap);
        g_wrlLog(buf);
    }
}
