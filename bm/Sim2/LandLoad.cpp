// LandSimulation slot 3 (SQZ.EXE 0x41e910): drop the two sub-objects and read new ones from a stream.
struct Streamable;
struct LandStream { LandStream &operator>>(Streamable **into); };   // 0x448ca0
#include "String/String.h"
struct LandFirst { String name; };                       // implicit destructor (0x4651d0 is the out-of-line instance)
struct LandSecond { void Release(); };                   // 0x45efa0
struct LandSimulation {
    int pad;
    LandFirst *first;                                    // 0x04
    LandSecond *second;                                  // 0x08
    LandStream &Load(LandStream &in);                    // slot 3
};

LandStream &LandSimulation::Load(LandStream &in)
{
    if (first)
        delete first;
    LandSecond *s = second;
    if (s) {
        s->Release();
        delete s;
    }
    return in >> (Streamable **)&first >> (Streamable **)&second;
}
