// 0x448f70: position a RIFF/WAVE reader on its "data" chunk.
#include <windows.h>
#include <mmsystem.h>
void DebugLog(const char *fmt, ...);     // 0x419290
struct WaveFile {
    char pad[0x1c];
    HMMIO hmmio;            // 0x1c
    MMCKINFO riff;          // 0x20
    MMCKINFO data;          // 0x34
    int SeekToData();
};
int WaveFile::SeekToData()
{
    if (mmioSeek(hmmio, riff.dwDataOffset + 4, SEEK_SET) == -1) {
        DebugLog("WaveFile: cannot seek to chunk start\n");
        return 0;
    }
    data.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(hmmio, &data, &riff, MMIO_FINDCHUNK)) {
        DebugLog("WaveFile: cannot find data chunk\n");
        return 0;
    }
    return 1;
}
