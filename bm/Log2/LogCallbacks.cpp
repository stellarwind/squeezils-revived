// SqgLog (SQZ.EXE 0x473710): printf-style logging through the optional callback at 0x4c0f88 (256-byte stack buffer).
#include "compat41.h"
#include <stdarg.h>
#include <stdio.h>

typedef void (*LogFn)(const char *);
extern LogFn g_sqgLog;      // 0x4c0f88

void SqgLog(const char *fmt, ...)
{
    char buf[256];
    if (g_sqgLog) {
        va_list ap;
        va_start(ap, fmt);
        vsprintf(buf, fmt, ap);
        g_sqgLog(buf);
    }
}
