// WaveSound::Free, SQZ.EXE 0x4493a0.
// Spare declarations preserve VC++ 4.1 register allocation.
void WaveFreePad1();
void WaveFreePad2();
void WaveFreePad3();
void WaveFreePad4();
void WaveFreePad5();
void WaveFreePad6();
void WaveFreePad7();
void WaveFreePad8();
void WaveFreePad9();
#include <windows.h>
#include "WaveSound/WaveSound.h"
void GameLog(const char *format, ...); // 0x419290

int WaveSound::Free()
{
    if (format && LocalFree(format)) {
        GameLog("Error deallocating wave format structure.\n");
        return 0;
    }
    if (data && LocalFree(data)) {
        GameLog("Error deallocating wave buffer.\n");
        return 0;
    }
    return 1;
}
