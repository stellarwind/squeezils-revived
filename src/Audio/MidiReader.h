#pragma once
#include <windows.h>

struct MidiReader {
    unsigned flags;            // 0x00
    unsigned length;           // 0x04 chunk length
    unsigned buffered;         // 0x08 bytes in the buffer
    unsigned char *buffer;     // 0x0c
    unsigned char *cursor;     // 0x10
    unsigned delta;            // 0x14
    unsigned char runningStatus; // 0x18
    DWORD start;               // 0x1c chunk data offset
    DWORD position;            // 0x20 file offset after the buffer
    unsigned left;             // 0x24 bytes not yet read
};
struct MidiFileInfo {
    DWORD size;                // 0x4c8690
    unsigned timeDivision;     // 0x4c8694
    unsigned format;           // 0x4c8698
    unsigned trackCount;       // 0x4c869c
    MidiReader *tracks;        // 0x4c86a0
};
extern MidiFileInfo gMidiInfo;               // 0x4c8690
extern unsigned gMidiNow;                    // 0x4c86dc
extern HANDLE gDataFile;                     // 0x4bf444
