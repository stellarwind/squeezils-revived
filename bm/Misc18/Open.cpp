// SQZ.EXE 0x45aa00: stream buffer setup from a file name; logs on failure.
#include <string.h>
extern char g_name[];                   // 0x4c86e0
extern int g_next;                      // 0x4c77a0
extern int g_loaded;                    // 0x4bf458
int TryOpen();                          // 0x45bf40
void LogError(const char *msg);         // 0x419290
void Continue(int v);                   // 0x45ac40
int F45aa00(const char *name)
{
    strcpy(g_name, name);
    if (TryOpen()) {
        LogError("Error in StreamBufferSetup.\n");
        return 0;
    }
    Continue(g_next);
    g_loaded = 1;
    return 1;
}
