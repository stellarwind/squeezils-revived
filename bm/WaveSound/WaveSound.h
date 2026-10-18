// WaveSound: stock DirectSound sample wave reader. Only the ctor/dtor (0x448d90/0x448db0) are ported.
#pragma once
#include "compat41.h"

struct WaveSound {
    int    useCount;    // 0x00
    int    id;          // 0x04 (-1 at the start)
    int    unknown08;   // 0x08
    int    unknown0c;   // 0x0c
    int    unknown10;   // 0x10
    int    unknown14;   // 0x14
    void  *buffer;      // 0x18
    void  *file;        // 0x1c
    char   riff[0x14];  // 0x20
    char   dataChunk[0x14]; // 0x34
    void  *data;        // 0x48
    int    dataBytes;   // 0x4c
    void  *format;      // 0x50
    char   desc[0x14];  // 0x54

    WaveSound();   // 0x448d90
    ~WaveSound();  // 0x448db0 (empty)
    int Free();    // 0x4493a0
};
