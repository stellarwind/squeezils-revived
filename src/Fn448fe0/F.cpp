// WaveFile::ReadData (0x448fe0, ret 4): copies up to 'size' bytes of the wave data into the buffer, returns the requested size or 0 on error.
#include <windows.h>
#include <mmsystem.h>
int DebugLogAll(const char *fmt, ...);     // 0x419290
struct WaveFile {
    char pad[0x1c];
    HMMIO hmmio;            // 0x1c
    MMCKINFO riff;          // 0x20
    MMCKINFO data;          // 0x34
    BYTE *buffer;           // 0x48
    int ReadData(DWORD size);
};
int WaveFile::ReadData(DWORD size)
{
    DWORD i;
    MMIOINFO info;
    if (mmioGetInfo(hmmio, &info, 0)) {
        DebugLogAll("Could not get information about .wav file.\n");
        return 0;
    }
    DWORD n = size;
    if (n > riff.cksize)
        n = riff.cksize;
    riff.cksize -= n;
    i = 0;
    while (n > i) {
        if (info.pchNext == info.pchEndRead) {
            if (mmioAdvance(hmmio, &info, MMIO_READ)) {
                DebugLogAll("Error reading .wav file.\n");
                return 0;
            }
            if (info.pchNext == info.pchEndRead) {
                DebugLogAll("Error reading .wav file.\n");
                return 0;
            }
        }
        buffer[i++] = *info.pchNext++;
    }
    if (mmioSetInfo(hmmio, &info, 0)) {
        DebugLogAll("Error reading .wav file.\n");
        return 0;
    }
    return size;
}
