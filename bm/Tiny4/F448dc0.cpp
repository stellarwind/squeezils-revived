// 0x448dc0 (ret 4): open a .wav file and read its RIFF header and "fmt " chunk; 0 on any failure.
#include <windows.h>
#include <mmsystem.h>
#include <string.h>
void DebugLog(const char *fmt, ...);     // 0x419290
struct WaveOpen {
    char pad[0x1c];
    HMMIO hmmio;            // 0x1c
    MMCKINFO riff;          // 0x20
    MMCKINFO data;          // 0x34
    char pad2[0x50 - 0x48];
    WAVEFORMATEX *format;   // 0x50
    int Open(char *name);
};
int WaveOpen::Open(char *name)
{
    MMCKINFO fmt;
    char raw[16];
    HMMIO h = mmioOpen(name, 0, MMIO_ALLOCBUF | MMIO_READ);
    if (!h) {
        DebugLog("Can not open file %s for reading\n", name);
        return 0;
    }
    if (mmioDescend(h, &riff, 0, 0)) {
        DebugLog("Error reading file %s\n", name);
        return 0;
    }
    if (riff.ckid != FOURCC_RIFF || riff.fccType != mmioFOURCC('W', 'A', 'V', 'E')) {
        DebugLog("File %s is not a .wav file\n", name);
        return 0;
    }
    fmt.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (mmioDescend(h, &fmt, &riff, MMIO_FINDCHUNK)) {
        DebugLog("Could not read 'fmt ' chunk from file\n");
        return 0;
    }
    if (fmt.cksize < 16) {
        DebugLog("Error reading 'fmt ' chunk\n");
        return 0;
    }
    if (mmioRead(h, (HPSTR)raw, 16) != 16) {
        DebugLog("Error reading 'fmt ' chunk\n");
        return 0;
    }
    format = (WAVEFORMATEX *)GlobalAlloc(0, 18);
    if (!format) {
        DebugLog("Could not allocate wave format structure\n");
        return 0;
    }
    memcpy(format, raw, 16);
    if (mmioAscend(h, &fmt, 0)) {
        DebugLog("Error ascending out of 'fmt ' chunk\n");
        return 0;
    }
    hmmio = h;
    return 1;
}
