// WaveSound: one sound effect read from a .wav file and turned into a static DirectSound buffer (size 0x68, no vtable).
// Reconstructed from SQZ.EXE 0x448d90-0x4493a0. The code is the stock "read a wave file with the mmio functions" routine of the
// DirectSound samples: Load opens the file, checks RIFF/WAVE, reads the 'fmt ' chunk, finds the 'data' chunk and reads it into
// memory; CreateBuffer copies that into a secondary buffer; Free releases the two memory blocks. The caller that makes the
// objects and files them into the sample table is 0x449aa0 (not reconstructed).
#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

// The 20 byte DSBUFFERDESC of DirectSound 3 (the DirectX 7 header has 36 bytes).
struct DSBufferDesc3 { DWORD dwSize, dwFlags, dwBufferBytes, dwReserved; WAVEFORMATEX *lpwfxFormat; };

class WaveSound {
public:
    WaveSound();                               // @448d90 (0x448db0 is an empty destructor)
    BOOL OpenWave(const char *path);           // @448dc0 open, check RIFF/WAVE, read the 'fmt ' chunk into `format`
    BOOL FindData();                           // @448f70 seek past the wave chunk and descend into 'data'
    int  ReadData(int size);                   // @448fe0 read up to `size` bytes into `data`; returns the count, 0 on error
    BOOL Load(const char *path);               // @4490f0 OpenWave, FindData, allocate, ReadData, close the file
    BOOL CreateBuffer();                       // @449200 make the secondary DirectSound buffer and fill it
    BOOL Free();                               // @4493a0 free the format structure and the data block

    int               useCount;                // 0x00 how many users share this sound (0x44a1a0 frees it when it is 1)
    int               id;                      // 0x04 -1 at the start
    int               unknown08;               // 0x08 never touched by the code seen so far
    int               unknown0c;               // 0x0c
    int               unknown10;               // 0x10 cleared by the constructor
    int               unknown14;               // 0x14 cleared by the constructor
    IDirectSoundBuffer *buffer;                // 0x18 secondary buffer made by CreateBuffer
    HMMIO             file;                    // 0x1c open only while loading
    MMCKINFO          riff;                    // 0x20 the RIFF chunk (cksize at 0x24 doubles as the read limit, see ReadData)
    MMCKINFO          dataChunk;               // 0x34 the 'data' chunk (cksize at 0x38 is the number of bytes to load)
    BYTE             *data;                    // 0x48 the samples, LocalAlloc'd
    int               dataBytes;               // 0x4c how many bytes ReadData returned
    WAVEFORMATEX     *format;                  // 0x50 18 bytes, LocalAlloc'd; only the first 16 are filled in
    DSBufferDesc3     desc;                    // 0x54 the buffer description
};
static_assert(sizeof(WaveSound) == 0x68, "WaveSound layout");

static IDirectSound **const kDirectSound = (IDirectSound **)0x4bd95c;   // the game's DirectSound object
