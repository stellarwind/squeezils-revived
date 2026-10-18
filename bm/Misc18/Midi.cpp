// SQZ.EXE 0x45c250: release the MIDI stream buffers: unprepare every header if prepared, then free each slot's memory.
#include <windows.h>
#include <mmsystem.h>
struct MidiSlot { MIDIHDR hdr; char pad[0x14]; };      // 0x54 bytes
extern MidiSlot g_midiSlots[32];                       // 0x4c7c08
extern int g_midiPrepared;                             // 0x4bf450
extern HMIDIOUT g_hMidiOut;                            // 0x4c86b8
void F45c250()
{
    MidiSlot *p;
    if (g_midiPrepared) {
        for (p = g_midiSlots; p < &g_midiSlots[32]; p++)
            midiOutUnprepareHeader(g_hMidiOut, &p->hdr, sizeof(MIDIHDR));
        g_midiPrepared = 0;
    }
    for (p = g_midiSlots; p < &g_midiSlots[32]; p++) {
        if (p->hdr.lpData) {
            GlobalUnlock(GlobalHandle(p->hdr.lpData));
            GlobalFree(GlobalHandle(p->hdr.lpData));
            p->hdr.lpData = 0;
        }
    }
}
