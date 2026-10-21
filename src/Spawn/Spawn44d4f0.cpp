// 0x44d4f0: like Spawn, but into a given slot; the message map takes the params by value.
#include "String/String.h"

struct SpawnParams { char data[0x40]; SpawnParams(const SpawnParams &o); ~SpawnParams(); };   // 0x4099e0 / 0x459c00
struct Body { char pad[0xb8]; float scale; char pad2[0xc8 - 0xbc]; };
struct MessageMap {
    void Start(SpawnParams params);                                 // 0x459c00
    char pad[0x34];
    MessageMap(Body *body, const char *name, int id);               // 0x459590
    void *operator new(unsigned size);                              // 0x48ea00
    void operator delete(void *p);                                  // 0x48e9f0
};
struct NamedBodyN : Body {
    NamedBodyN(const SpawnParams &p, const char *name);             // 0x45a850
    void *operator new(unsigned size);
    void operator delete(void *p);
};
struct StringTable188 { String Lookup(const String &key); };            // 0x462cc0
struct IdTable11c { int Id(const String &key); };                         // 0x461230
struct IdMaker { virtual int Make(int id); char body[0x14 - 4]; };
struct ScaleTable { virtual float Scale(int id); char body[0x14 - 4]; };
struct World {
    int Current();                                                  // 0x414250
    void Add(int current, Body *body, MessageMap *map);             // 0x414160
};
World *GetSpawnSimulation();                                                  // 0x420690

struct Spawner {
    char pad[0x11c];
    IdTable11c ids;                                                 // 0x11c
    char pad2[0x160 - 0x11c - 4];
    ScaleTable scales;                                              // 0x160
    IdMaker maker;                                                  // 0x174
    StringTable188 names;                                           // 0x188
    void SpawnAt(int slot, const char *name, SpawnParams params);   // 0x44d4f0
};

void Spawner::SpawnAt(int slot, const char *name, SpawnParams params)
{
    NamedBodyN *body = new NamedBodyN(params, names.Lookup(String(name)).c_str());
    MessageMap *map = new MessageMap(body, name, maker.Make(ids.Id(String(name))));
    float s = scales.Scale(ids.Id(String(name)));
    body->scale = s;
    map->Start(params);
    World *w = GetSpawnSimulation();
    w->Add(slot, body, map);
}
