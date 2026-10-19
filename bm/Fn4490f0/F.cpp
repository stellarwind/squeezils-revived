// WaveFile::Load (0x4490f0, ret 4): opens a .wav file, finds its data chunk and reads the samples into a global buffer.
#include <windows.h>
#include <mmsystem.h>
void DebugLog(const char *fmt, ...);     // 0x419290
struct WaveFile {
    char pad[0x1c];
    HMMIO hmmio;            // 0x1c
    MMCKINFO riff;          // 0x20
    MMCKINFO data;          // 0x34
    void *buffer;           // 0x48
    int readOk;             // 0x4c
    int Open(const char *path);          // 0x448dc0
    int SeekToData();                    // 0x448f70
    int ReadData(DWORD size);            // 0x448fe0
    int Load(const char *path);
};
#define WAVE_FAIL(msg) \
    do { \
        DebugLog(msg); \
        if (hmmio) { \
            mmioClose(hmmio, 0); \
            hmmio = 0; \
        } \
        return 0; \
    } while (0)
int WaveFile::Load(const char *path)
{
    if (!Open(path))
        WAVE_FAIL("Error opening .wav file.\n");
    if (!SeekToData())
        WAVE_FAIL("Error starting data read.\n");
    buffer = GlobalAlloc(0, data.cksize);
    if (!buffer)
        WAVE_FAIL("Error allocating memory for wave buffer.\n");
    readOk = ReadData(data.cksize);
    if (!readOk)
        WAVE_FAIL("Error reading wave file into wave buffer.\n");
    if (hmmio) {
        mmioClose(hmmio, 0);
        hmmio = 0;
    }
    return 1;
}
