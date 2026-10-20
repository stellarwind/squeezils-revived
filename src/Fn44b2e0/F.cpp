// PlaceLevel constructor (SQZ.EXE 0x44af30) and destructor (0x44b2e0). The constructor only builds the members; the
// destructor frees the robo brains, the squeezil and goodie spots owned through the containers, then the members die.
#include "Ini/IniFile.h"
#include "IniReader/IniReader.h"
#include <stdio.h>
#include <string.h>
void DebugLog(const char *format, ...);                                     // 0x419290
struct RoboBrain { char body[0x24]; ~RoboBrain(); };                        // 0x4484a0
struct PickMatrix { float m[16]; PickMatrix() {} ~PickMatrix(); };          // 0x402310
struct Contact {                                                            // dtor 0x4031e0
    int face, second;
    PickMatrix matrix;
    Contact() : face(-1), second(0) {}
    ~Contact();
};
struct V3 { float x, y, z; V3() {} ~V3(); };                                // 0x401660
struct BodyPlacement {                                                      // 0xb0, dtor 0x402250
    int unknown00;
    float matrix[16];
    Contact contact;                                                        // 0x44
    V3 position, up, direction;                                             // 0x8c 0x98 0xa4
    BodyPlacement() {}
    ~BodyPlacement();
};
struct _SqueezilSpotInfo {                                                  // 0xb0
    int kind;
    char name[64];
    Contact contact;                                                        // 0x44
    V3 position, up, direction;                                             // 0x8c 0x98 0xa4
};
struct _GoodieSpotInfo {
    char head[0x4c];
    V3 position, up, direction;                                             // 0x4c 0x58 0x64
};
typedef BufferObjectTable<String, int, PrimitiveArray<int, 1>, 21> NumberTable;
typedef PrimitiveArray<float, 2> FloatList;
typedef PrimitiveArray<int, 1> IntList;

struct PlaceLevel {                                                         // 0x2f4
    char head[0x108];
    StringList goodieNames;                                                   // 0x108
    NumberTable goodieIndex;                                                // 0x11c
    FloatList rebudRates;                                                      // 0x14c
    FloatList collisionRadii;                                                      // 0x160
    IntList ungrabbable;                                                          // 0x174
    IniValueTable goodieProps;                                                  // 0x188
    IniValueTable bumpFunctions;                                                  // 0x1b8
    Array<_GoodieSpotInfo *> goodieSpots;                                   // 0x1e8
    BodyPlacement start;                                                    // 0x1f8
    Array<_SqueezilSpotInfo *> squeezilSpots;                               // 0x2a8
    IntList intsB;                                                          // 0x2b8
    Table<String, RoboBrain *, Array<RoboBrain *>, 0> brains;               // 0x2cc
    PlaceLevel();
    int ReadGoodieList();                                                   // 0x44b950
    ~PlaceLevel();
};

PlaceLevel::PlaceLevel()
{
}

PlaceLevel::~PlaceLevel()
{
    int i = brains.keys.count;
    while (--i >= 0) {
        RoboBrain *b = brains.values.data[i];
        if (b)
            delete b;
        brains.RemoveAt(i);
    }
    for (i = squeezilSpots.count - 1; i >= 0; i--) {
        _SqueezilSpotInfo *s = squeezilSpots.data[i];
        if (s)
            delete s;
        squeezilSpots.Remove(i);
    }
    for (i = goodieSpots.count - 1; i >= 0; i--) {
        _GoodieSpotInfo *g = goodieSpots.data[i];
        if (g)
            delete g;
        goodieSpots.Remove(i);
    }
}

int PlaceLevel::ReadGoodieList()
{
    IniReader ini;
    ini.SetSection("GoodieList");
    StringList *keys = ini.GetKeys();
    for (int i = 0; i < keys->count; i++) {
        int number;
        sscanf(keys->Get(i).c_str(), "%d", &number);
        ini.SetSection("GoodieList");
        String name(ini.GetString(keys->Get(i).c_str()));
        ini.SetSection(name.c_str());
        String prop(ini.GetString("Prop"));
        if (strlen(prop.c_str()) == 0) {
            DebugLog("ERROR: no Prop entry, ignoring goodie!\n");
        } else {
            goodieIndex.Insert(name, goodieNames.count, -1);
            goodieNames.Insert(name, -1);
            goodieProps.Insert(name, prop, -1);
            rebudRates.Insert(ini.GetFloat("RebudRate", 0.0f), -1);
            collisionRadii.Insert(ini.GetFloat("CollisionRadius", 0.5f), -1);
            ungrabbable.Insert(ini.GetInt("UngrabbableByRobos", 0), -1);
            String bump(ini.GetString("BumpFunction"));
            if (strlen(bump.c_str()) != 0)
                bumpFunctions.Insert(name, bump, -1);
        }
    }
    delete keys;
    return 1;
}
