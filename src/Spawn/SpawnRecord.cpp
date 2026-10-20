// SQZ.EXE 0x44d290: spawn a placed object and record its allocated world ID.
#include "String/String.h"
#include "Vector3f/Vector3f.h"

struct SpawnRecord {
    int kind;                   // +0x00
    char name[0x48];             // +0x04
    Vector3f position;           // +0x4c
    Vector3f up;                 // +0x58
    Vector3f forward;            // +0x64
    int worldId;                // +0x70
};
struct RecordTransform {
    float m[16];
    RecordTransform() {}
    ~RecordTransform(); // 0x402310
    void Orthonormalize(); // 0x47e350
};
struct Body { char pad[0xb8]; float scale; char pad2[0xc8 - 0xbc]; };
struct NamedBodyN : Body {
    NamedBodyN(const RecordTransform &matrix, const char *name); // 0x45a850
    void *operator new(unsigned size); // 0x48ea00
    void operator delete(void *p); // 0x48e9f0
};
struct MessageMap {
    char pad[0x34];
    MessageMap(Body *body, const char *name, int id); // 0x459590
    void *operator new(unsigned size); // 0x48ea00
    void operator delete(void *p); // 0x48e9f0
};
struct StringTable188 { String Lookup(const String &key); }; // 0x462cc0
struct IdMaker { virtual int Make(int id); char body[0x14 - 4]; };
struct ScaleTable { virtual float Scale(int id); char body[0x14 - 4]; };
struct World {
    int Current(); // 0x414250
    void Add(int current, Body *body, MessageMap *map); // 0x414160
};
World *GetWorld(); // 0x420690
struct Spawner {
    char pad[0x160];
    ScaleTable scales; // +0x160
    IdMaker maker; // +0x174
    StringTable188 names; // +0x188
    void SpawnRecordObject(SpawnRecord *record); // 0x44d290
};

void Spawner::SpawnRecordObject(SpawnRecord *record)
{
    RecordTransform matrix;
    Vector3f right = record->up.Cross(record->forward);
    right.Normalize();
    matrix.m[0] = right.x;
    matrix.m[1] = right.y;
    matrix.m[2] = right.z;
    matrix.m[3] = 0;
    matrix.m[4] = record->up.x;
    matrix.m[5] = record->up.y;
    matrix.m[6] = record->up.z;
    matrix.m[7] = 0;
    matrix.m[8] = record->forward.x;
    matrix.m[9] = record->forward.y;
    matrix.m[10] = record->forward.z;
    matrix.m[11] = 0;
    matrix.m[12] = record->position.x;
    matrix.m[13] = record->position.y;
    matrix.m[14] = record->position.z;
    matrix.m[15] = 1.0f;
    matrix.Orthonormalize();
    NamedBodyN *body = new NamedBodyN(matrix, names.Lookup(String(record->name)).c_str());
    MessageMap *map = new MessageMap(body, record->name, maker.Make(record->kind));
    body->scale = scales.Scale(record->kind);
    World *world = GetWorld();
    int id = world->Current();
    world->Add(id, body, map);
    record->worldId = id;
}
