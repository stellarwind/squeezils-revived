// 0x44d750: create a body and its message map for `name` and register both with the world.
#include "String/String.h"
struct Vec3 { float x, y, z; Vec3(const Vec3 &o); ~Vec3(); };   // copy ctor out of line, 0x4183c0

struct SpawnParams { char data[0x40]; SpawnParams(const SpawnParams &o); ~SpawnParams(); };   // 0x4099e0 / 0x459c00
struct Body { char pad[0xb8]; float scale; char pad2[0xc8 - 0xbc]; };
struct MessageMap {
    char pad[0x34];
    void Start(SpawnParams params, Vec3 vel, int extra);                                                   // 0x459ed0
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
    void Add(int cur, Body *body, MessageMap *map);             // 0x414160
};
World *GetWorld();                                                  // 0x420690

struct Spawner {
    char pad[0x11c];
    IdTable11c ids;                                                 // 0x11c
    char pad2[0x160 - 0x11c - 4];
    ScaleTable scales;                                              // 0x160
    IdMaker maker;                                                  // 0x174
    StringTable188 names;                                           // 0x188
    void Spawn(const char *name, SpawnParams params);
    void SpawnVel(const char *name, SpawnParams params, Vec3 vel, int extra);   // 0x44d990
};

void Spawner::SpawnVel(const char *name, SpawnParams params, Vec3 vel, int extra)
{
    NamedBodyN *body = new NamedBodyN(params, names.Lookup(String(name)).c_str());
    MessageMap *map = new MessageMap(body, name, maker.Make(ids.Id(String(name))));
    map->Start(params, vel, extra);
    int id = ids.Id(String(name));
    body->scale = scales.Scale(id);
    World *w = GetWorld();
    int cur = w->Current();
    w->Add(cur, body, map);
}
