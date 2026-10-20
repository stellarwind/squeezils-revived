// 0x45b7a0: refills a 1 KB read buffer from the data file (seek to the saved position, read up to 0x400 bytes).
#include <windows.h>
extern HANDLE gDataFile;                                  // 0x4bf444
int DebugLogAll(const char *msg, ...);                              // 0x419290
struct Stream {
    int pad0[2];
    unsigned chunk;     // 0x08
    char *buf;          // 0x0c
    char *cur;          // 0x10
    int pad14[3];
    long pos;           // 0x20
    unsigned left;      // 0x24
};
int FillBuffer(Stream *s)
{
    DWORD got;
    if (s->left != 0) {
        s->cur = s->buf;
        DWORD at = SetFilePointer(gDataFile, s->pos, 0, 0);
        if (at == 0xffffffff) {
            DebugLogAll("Seek error");
            return 1;
        }
        unsigned *n = &s->chunk;
        if (s->left > 0x400)
            *n = 0x400;
        else
            *n = s->left;
        BOOL ok = ReadFile(gDataFile, s->buf, *n, &got, 0);
        s->left -= got;
        s->pos = got + at;
        *n = got;
        if (ok && got)
            return 0;
        DebugLogAll("Read error");
        *n = got;
        return 1;
    }
    return 1;
}
