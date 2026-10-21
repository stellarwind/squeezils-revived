#include "Runtime/DynamicCast.h"
// SQZ.EXE 0x44a250: duplicate and play a sample relative to the listener.
#include "compat41.h"
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <math.h>
#include "String/String.h"
#include "Vector3f/Vector3f.h"
#include "WaveSound/WaveSound.h"

struct EffectTransform { Vector3f right; char pad[0x24]; Vector3f position; };
struct EffectListener {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual EffectTransform *Transform(); // slot 3
};
struct EffectSimulation { EffectListener *Listener(int id); }; // 0x414050, ret 4
EffectSimulation *GetEffectSimulation(); // 0x420690, cdecl, no arguments
struct EffectMatrix { float m[16]; EffectMatrix() {} ~EffectMatrix(); }; // 0x402310
struct EffectContact {
    int face, second;
    EffectMatrix matrix;
    EffectContact(int id, int sub) : face(id), second(sub) {}
    ~EffectContact(); // 0x4031e0
};
struct EffectSurface { float unused, radiusSquared; };
struct EffectSurfaces { EffectSurface *Surface(EffectContact *where); }; // 0x412be0
struct EffectPlace { EffectSurfaces *GetSurfaces(); }; // 0x439b60
extern const char kWorldSimulationType[], kWorldPlaceType[]; // 0x4ba1d0, 0x4bae38
struct StreamBuffers { void Insert(LPDIRECTSOUNDBUFFER *item, int at); }; // 0x4680b0
extern StreamBuffers gEffectBuffers; // 0x4c77e0
extern LPDIRECTSOUND g_directSound; // 0x4bd95c
extern float g_musicVolume; // 0x4c77cc
extern const float kEffectDistanceThreshold; // 0x49eca0 (0.0f)
extern const char kEffectMissing[], kEffectDuplicateFailed[]; // 0x4be034, 0x4be004
extern const char kAmbBadVolume[], kEffectBadPan[], kAmbBadPlay[]; // 0x4bdfe4, 0x4bdfc8, 0x4bdfb0
WaveSound *FindSound(const char *name); // 0x449c30
int DebugLogAll(const char *format, ...); // 0x419290

void PlayEffectAt(String name, Vector3f position)
{
    if (!g_directSound)
        return;
    WaveSound *sound = FindSound(name.c_str());
    if (!sound) {
        DebugLogAll(kEffectMissing, name.c_str());
        return;
    }
    LPDIRECTSOUNDBUFFER buffer;
    if (g_directSound->DuplicateSoundBuffer((LPDIRECTSOUNDBUFFER)sound->buffer, &buffer) < 0) {
        DebugLogAll(kEffectDuplicateFailed);
        return;
    }
    EffectListener *listener = GetEffectSimulation()->Listener(0);
    Vector3f listenerPosition = listener->Transform()->position;
    Vector3f direction = position - listenerPosition;
    EffectContact contact(0, 0);
    float distance;
    // The original tests the unsigned representation of Y, including its sentinel.
    if (*(unsigned *)&position.y < 0xc7c35000U) {
        EffectPlace *place = (EffectPlace *)SqzRTDynamicCast(GetEffectSimulation(), 0,
            kWorldSimulationType, kWorldPlaceType, 0);
        float radius = (float)sqrt(place->GetSurfaces()->Surface(&contact)->radiusSquared);
        distance = direction.Length() / radius;
    } else
        distance = 0.0f;
    // Both arms in the original produce the same volume; retain its comparison.
    double volume = distance < kEffectDistanceThreshold ? g_musicVolume * 10000.0f - 10000.0f
                                  : g_musicVolume * 10000.0f - 10000.0f;
    if (buffer->SetVolume((int)volume) < 0) {
        DebugLogAll(kAmbBadVolume, volume);
        return;
    }
    Vector3f right = listener->Transform()->right;
    direction.Normalize();
    double pan = (right.x * direction.x + right.z * direction.z + right.y * direction.y) * 10000.0f;
    if (buffer->SetPan((int)pan) < 0) {
        DebugLogAll(kEffectBadPan, pan);
        return;
    }
    gEffectBuffers.Insert(&buffer, -1);
    if (buffer->Play(0, 0, 0) < 0)
        DebugLogAll(kAmbBadPlay);
}
