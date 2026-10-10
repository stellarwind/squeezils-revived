#include "WaveSound.h"
#include "Original.h"
#include <string.h>

WaveSound::WaveSound() {                       // @448d90 (only these eight words are cleared; the rest is left as it was)
    id = -1;
    format = 0;
    data = 0;
    file = 0;
    buffer = 0;
    useCount = 0;
    unknown10 = 0;
    unknown14 = 0;
}

BOOL WaveSound::OpenWave(const char *path) {   // @448dc0
    HMMIO h = mmioOpen((LPSTR)path, 0, MMIO_ALLOCBUF | MMIO_READ);
    if (!h) { GameLog("Can not open file %s for reading.\n", path); return 0; }
    // from here on a failure leaves the file open and `file` unset, as the original does
    if (mmioDescend(h, &riff, 0, 0)) { GameLog("Error reading file %s.\n", path); return 0; }
    if (riff.ckid != FOURCC_RIFF || riff.fccType != mmioFOURCC('W', 'A', 'V', 'E')) {
        GameLog("File %s is not a .wav file.\n", path);
        return 0;
    }
    MMCKINFO fmt;
    fmt.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (mmioDescend(h, &fmt, &riff, MMIO_FINDCHUNK)) { GameLog("Could not read 'fmt ' chunk from .wav file.\n"); return 0; }
    if (fmt.cksize < 16) { GameLog("Error reading 'fmt ' chunk.\n"); return 0; }
    DWORD pcm[4];
    if (mmioRead(h, (HPSTR)pcm, 16) != 16) { GameLog("Error reading 'fmt ' chunk.\n"); return 0; }
    format = (WAVEFORMATEX *)LocalAlloc(0, 18);
    if (!format) { GameLog("Could not allocate wave format structure.\n"); return 0; }
    ((DWORD *)format)[0] = pcm[0];
    ((DWORD *)format)[1] = pcm[1];
    ((DWORD *)format)[2] = pcm[2];
    ((DWORD *)format)[3] = pcm[3];             // cbSize (the last two bytes) is left as LocalAlloc gave it
    if (mmioAscend(h, &fmt, 0)) { GameLog("Error ascending out of 'fmt ' chunk.\n"); return 0; }
    file = h;
    return 1;
}

BOOL WaveSound::FindData() {                   // @448f70
    if (mmioSeek(file, riff.dwDataOffset + 4, SEEK_SET) == -1) { GameLog("Error seeking past wave chunk.\n"); return 0; }
    dataChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(file, &dataChunk, &riff, MMIO_FINDCHUNK)) { GameLog("Error descending into data chunk.\n"); return 0; }
    return 1;
}

int WaveSound::ReadData(int size) {            // @448fe0
    MMIOINFO info;
    if (mmioGetInfo(file, &info, 0)) { GameLog("Could not get information about .wav file.\n"); return 0; }
    // The limit is the RIFF chunk's size and it is the one that is counted down (the data chunk's own size is not used here).
    unsigned int n = size;
    if (n > riff.cksize) n = riff.cksize;
    riff.cksize -= n;
    for (unsigned int i = 0; i < n; i++) {
        if (info.pchNext == info.pchEndRead) {
            if (mmioAdvance(file, &info, MMIO_READ)) { GameLog("Error reading .wav file.\n"); return 0; }
            if (info.pchNext == info.pchEndRead) { GameLog("Error reading .wav file.\n"); return 0; }
        }
        data[i] = *info.pchNext++;
    }
    if (mmioSetInfo(file, &info, 0)) { GameLog("Error reading .wav file.\n"); return 0; }
    return n;
}

BOOL WaveSound::Load(const char *path) {       // @4490f0
    if (!OpenWave(path)) {
        GameLog("Error opening .wav file.\n");
    } else if (!FindData()) {
        GameLog("Error starting data read.\n");
    } else {
        data = (BYTE *)LocalAlloc(0, dataChunk.cksize);
        if (!data) {
            GameLog("Error allocating memory for wave buffer.\n");
        } else {
            dataBytes = ReadData(dataChunk.cksize);
            if (!dataBytes) {
                GameLog("Error reading wave file into wave buffer.\n");
            } else {
                if (file) { mmioClose(file, 0); file = 0; }
                return 1;
            }
        }
    }
    if (file) { mmioClose(file, 0); file = 0; }
    return 0;
}

BOOL WaveSound::CreateBuffer() {               // @449200
    desc.dwSize = sizeof(DSBufferDesc3);       // 20
    desc.dwFlags = 0xe2;                       // static, volume, pan, frequency
    desc.dwBufferBytes = dataBytes;
    desc.dwReserved = 0;
    desc.lpwfxFormat = format;
    if ((*kDirectSound)->CreateSoundBuffer((LPCDSBUFFERDESC)&desc, &buffer, 0) < 0) {
        GameLog("Could not allocate secondary buffer.\n");
        return 0;
    }
    void *p1, *p2;
    DWORD l1, l2;
    if (buffer->Lock(0, dataBytes, &p1, &l1, &p2, &l2, 0) < 0) {
        GameLog("Could not lock secondary buffer.\n");
        if (buffer) { buffer->Release(); buffer = 0; }
        return 0;
    }
    memcpy(p1, data, dataBytes);
    if (buffer->Unlock(p1, dataBytes, 0, 0) < 0) {
        GameLog("Could not unlock secondary buffer.\n");
        if (buffer) { buffer->Release(); buffer = 0; }
        return 0;
    }
    if (buffer->SetVolume(0) < 0) {
        GameLog("Could not set volume.\n");
        if (buffer) { buffer->Release(); buffer = 0; }
        return 0;
    }
    if (buffer->SetPan(0) < 0) {
        GameLog("Could not set pan.\n");
        if (buffer) { buffer->Release(); buffer = 0; }
        return 0;
    }
    return 1;
}

BOOL WaveSound::Free() {                       // @4493a0
    if (format && LocalFree(format)) { GameLog("Error deallocating wave format structure.\n"); return 0; }
    if (data && LocalFree(data)) { GameLog("Error deallocating wave buffer.\n"); return 0; }
    return 1;
}
