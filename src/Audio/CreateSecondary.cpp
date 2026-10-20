// SQZ.EXE 0x449200: create a secondary DirectSound buffer for this sound and fill it with the sample data; 0 on failure.
#include <windows.h>
#include <dsound.h>
#include <string.h>

extern LPDIRECTSOUND g_directSound;         // 0x4bd95c
int DebugLogAll(const char *format, ...);     // 0x419290
extern const char kNoSecondary[], kNoLock[], kNoUnlock[], kNoVolume[], kNoPan[];   // 0x4bdbf8 0x4bdbd4 0x4bdbb0 0x4bdb98 0x4bdb84

struct SoundObj {
    char pad0[0x18];
    LPDIRECTSOUNDBUFFER buffer;             // 0x18
    char pad1[0x48 - 0x1c];
    void *data;                             // 0x48
    DWORD size;                             // 0x4c
    LPWAVEFORMATEX format;                  // 0x50
    DSBUFFERDESC desc;                      // 0x54
    int CreateSecondary();
};

int SoundObj::CreateSecondary()
{
    void *p1 = 0;
    void *p2 = 0;
    DWORD n1, n2;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(DSBUFFERDESC);
    desc.dwBufferBytes = size;
    desc.lpwfxFormat = format;
    desc.dwFlags = 0xe2;
    if (g_directSound->CreateSoundBuffer(&desc, &buffer, 0) < 0) {
        DebugLogAll(kNoSecondary);
        return 0;
    }
    if (buffer->Lock(0, size, &p1, &n1, &p2, &n2, 0) < 0) {
        DebugLogAll(kNoLock);
        if (buffer) {
            buffer->Release();
            buffer = 0;
        }
        return 0;
    }
    memcpy(p1, data, size);
    if (buffer->Unlock(p1, size, 0, 0) < 0) {
        DebugLogAll(kNoUnlock);
        if (buffer) {
            buffer->Release();
            buffer = 0;
        }
        return 0;
    }
    p1 = 0;
    if (buffer->SetVolume(0) < 0) {
        DebugLogAll(kNoVolume);
        if (buffer) {
            buffer->Release();
            buffer = 0;
        }
        return 0;
    }
    if (buffer->SetPan(0) < 0) {
        DebugLogAll(kNoPan);
        if (buffer) {
            buffer->Release();
            buffer = 0;
        }
        return 0;
    }
    return 1;
}
