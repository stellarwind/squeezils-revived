// PlaceLevel constructor (SQZ.EXE 0x44af30) and destructor (0x44b2e0). The constructor only builds the members; the
// destructor frees the robo brains, the squeezil and goodie spots owned through the containers, then the members die.
// Read (0x44bd60) loads the place's .lvl file: goodie spots, the enter spot and the robo spots.
#include "Ini/IniFile.h"
#include "IniReader/IniReader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int DebugLogAll(const char *format, ...);                                     // 0x419290
struct App;
int FatalErrorApp(App *app, const char *fmt, ...);                          // 0x475510 (Renderer::FatalError; 2 = give up)
extern App *g_app;                                                          // 0x4c75dc
extern const char *g_lvlHeaderV10;                                          // 0x4be1e0 "#Lvl V1.0 ascii\n"
extern const char *g_lvlHeaderV11;                                          // 0x4be1e4 "#Lvl V1.1 ascii\n"
extern const char *g_lvlHeaderV12;                                          // 0x4be1e8 "#Lvl V1.2 ascii\n"
struct RoboBrain { char body[0x24]; ~RoboBrain(); };                        // 0x4484a0
struct PickMatrix { float m[16]; PickMatrix() {} ~PickMatrix(); };          // 0x402310
struct Contact {                                                            // dtor 0x4031e0
    int face, second;
    PickMatrix matrix;
    Contact() : face(-1), second(0) {}
    ~Contact();
};
struct V3 { float x, y, z; V3() {} ~V3(); float Normalize(); };             // dtor 0x401660, Normalize 0x47d630
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
struct _GoodieSpotInfo {                                                    // 0x74
    int kind;                                                               // 0x00 index into goodieNames
    char name[64];                                                          // 0x04
    float rebudRate;                                                        // 0x44
    float unknown48;                                                        // 0x48 (-1 when read)
    V3 position, up, direction;                                             // 0x4c 0x58 0x64
    int unknown70;                                                          // 0x70
};
typedef BufferObjectTable<String, int, PrimitiveArray<int, 1>, 21> NumberTable;
typedef PrimitiveArray<float, 2> FloatList;
typedef PrimitiveArray<int, 1> IntList;

struct PlaceLevel {                                                         // 0x2f4
    int version;                                                            // 0x00 .lvl format: 0 = V1.0, 1 = V1.1, 2 = V1.2
    char path[260];                                                         // 0x04
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
    int Read(IniReader *ini);                                               // 0x44bd60
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
            DebugLogAll("ERROR: no Prop entry, ignoring goodie!\n");
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

// Every read error logs, closes the file and returns 0; a half-read spot is freed first.
// func (2026-10-20): calls and constants agree with the original. Shape differences left for the bm track: the original keeps
// every DebugLog+fclose error tail separate (ours cross-jumps them), places the default-name branch, the invalid-header block
// and the enter-spot version cases after the main path, and calls ~BodyPlacement (0x402250) for one delete robo.
int PlaceLevel::Read(IniReader *ini)
{
    FILE *file = NULL;
    ini->SetSection("Defaults");
    const char *levelFile = ini->GetString("LevelFile");
    if (strlen(levelFile) == 0) {
        strcpy(path, "placexxx.lvl");
        String name(path);
        strcpy(path, ini->GetDirectory().c_str());
        strcat(path, "\\");
        strcat(path, name.c_str());
    } else {
        strcpy(path, levelFile);
        if (strchr(path, '\\') == NULL) {
            String name(path);
            strcpy(path, ini->GetDirectory().c_str());
            strcat(path, "\\");
            strcat(path, name.c_str());
        }
    }
    while (file == NULL) {
        file = fopen(path, "r");
        // the original passes the (NULL) file, not the path, to both messages
        if (file == NULL && FatalErrorApp(g_app, "Could not read file %s.\n", file) == 2) {
            DebugLogAll("Could not open level file %d.\n", file);
            return 0;
        }
    }

    char line[256];
    if (fgets(line, 255, file) == NULL) {
        DebugLogAll("ERROR: Could not read first line.\n");
        fclose(file);
        return 0;
    }
    if (strstr(line, g_lvlHeaderV12) != NULL) {
        version = 2;
        if (fgets(line, 255, file) == NULL) {
            DebugLogAll("ERROR: Could not read number of goodies.\n");
            fclose(file);
            return 0;
        }
    } else if (strstr(line, g_lvlHeaderV11) != NULL) {
        version = 1;
        if (fgets(line, 255, file) == NULL) {
            DebugLogAll("ERROR: Could not read number of goodies.\n");
            fclose(file);
            return 0;
        }
    } else if (strstr(line, g_lvlHeaderV10) != NULL) {
        version = 0;
        if (fgets(line, 255, file) == NULL) {
            DebugLogAll("ERROR: Could not read number of goodies.\n");
            fclose(file);
            return 0;
        }
    } else {
        DebugLogAll("ERROR: File has invalid header.\n");
        fclose(file);
        return 0;
    }

    // goodie spots: V1.0 "kind x y z ux uy uz" (the up vector is then ignored), V1.1 "kind pos up dir", V1.2 "name pos up dir"
    int count = atoi(line);
    int i;
    for (i = 0; i < count; i++) {
        _GoodieSpotInfo *goodie = new _GoodieSpotInfo;
        if (fgets(line, 255, file) == NULL) {
            DebugLogAll("Could not read goodie instance %d.\n", i);
            fclose(file);
            return 0;
        }
        if (version == 0) {
            if (sscanf(line, "%d %f %f %f %f %f %f", &goodie->kind, &goodie->position.x, &goodie->position.y,
                       &goodie->position.z, &goodie->up.x, &goodie->up.y, &goodie->up.z) != 7) {
                DebugLogAll("Could not read goodie spot %d information.\n", i);
                fclose(file);
                delete goodie;
                return 0;
            }
            if ((unsigned)goodie->kind >= (unsigned)goodieNames.count) {
                DebugLogAll("WARNING:  Goodie value %d does not exist, using 0\n", goodie->kind);
                goodie->kind = 0;
            }
            strcpy(goodie->name, goodieNames.Get(goodie->kind).c_str());
            goodie->up.x = 0.0f; goodie->up.y = 1.0f; goodie->up.z = 0.0f;
            goodie->direction.x = 0.0f; goodie->direction.y = 0.0f; goodie->direction.z = 1.0f;
        } else if (version == 1 || version == 2) {
            if (version == 1) {
                if (sscanf(line, "%d %f %f %f %f %f %f %f %f %f", &goodie->kind,
                           &goodie->position.x, &goodie->position.y, &goodie->position.z,
                           &goodie->up.x, &goodie->up.y, &goodie->up.z,
                           &goodie->direction.x, &goodie->direction.y, &goodie->direction.z) != 10) {
                    DebugLogAll("Could not read goodie spot %d information.\n", i);
                    fclose(file);
                    delete goodie;
                    return 0;
                }
                if ((unsigned)goodie->kind >= (unsigned)goodieNames.count) {
                    DebugLogAll("WARNING:  Goodie value %d does not exist, using 0\n", goodie->kind);
                    goodie->kind = 0;
                }
                strcpy(goodie->name, goodieNames.Get(goodie->kind).c_str());
            } else {
                if (sscanf(line, "%s %f %f %f %f %f %f %f %f %f", goodie->name,
                           &goodie->position.x, &goodie->position.y, &goodie->position.z,
                           &goodie->up.x, &goodie->up.y, &goodie->up.z,
                           &goodie->direction.x, &goodie->direction.y, &goodie->direction.z) != 10) {
                    DebugLogAll("Could not read goodie spot %d information.\n", i);
                    fclose(file);
                    delete goodie;
                    return 0;
                }
                if (!goodieProps.Contains(String(goodie->name))) {
                    DebugLogAll("WARNING:  Goodie type %s does not exist, using %s\n", goodie->name,
                             goodieProps.KeyAt(0).c_str());
                    strcpy(goodie->name, goodieProps.KeyAt(0).c_str());
                    goodie->kind = 0;
                } else {
                    goodie->kind = goodieIndex.Get(String(goodie->name));
                }
            }
            goodie->up.Normalize();
            goodie->direction.Normalize();
        }
        goodie->rebudRate = rebudRates.Get(goodie->kind);
        goodie->unknown48 = -1.0f;
        goodieSpots.Insert(goodie, -1);
    }

    // the enter spot (V1.0 files carry a spot count first, always 1)
    if (version == 0 && fgets(line, 255, file) == NULL) {
        DebugLogAll("Could not read enter spot count from level file.\n");
        fclose(file);
        return 0;
    }
    if (fgets(line, 255, file) == NULL) {
        DebugLogAll("Could not read enter spot %d.\n", i);
        fclose(file);
        return 0;
    }
    if (version == 0) {
        int unused;
        if (sscanf(line, "%d %d", &unused, &start.contact.second) != 2) {
            DebugLogAll("Could not read enter spot information.\n");
            fclose(file);
            return 0;
        }
        start.up.x = 0.0f; start.up.y = 1.0f; start.up.z = 0.0f;
        start.direction.x = 0.0f; start.direction.y = 0.0f; start.direction.z = 1.0f;
        start.contact.face = 0;
    } else if (version == 1) {
        if (sscanf(line, "%d %f %f %f %f %f %f %f %f %f", &start.contact.second,
                   &start.position.x, &start.position.y, &start.position.z,
                   &start.up.x, &start.up.y, &start.up.z,
                   &start.direction.x, &start.direction.y, &start.direction.z) != 10) {
            DebugLogAll("Could not read enter spot information.\n");
            fclose(file);
            return 0;
        }
        start.contact.face = 0;
    } else if (version == 2) {
        if (sscanf(line, "%d %d %f %f %f %f %f %f %f %f %f", &start.contact.face, &start.contact.second,
                   &start.position.x, &start.position.y, &start.position.z,
                   &start.up.x, &start.up.y, &start.up.z,
                   &start.direction.x, &start.direction.y, &start.direction.z) != 11) {
            DebugLogAll("Could not read enter spot information.\n");
            fclose(file);
            return 0;
        }
    }
    start.up.Normalize();
    start.direction.Normalize();

    // V1.0 files also list exit, rebirth and landing spots; they are skipped
    int n, j;
    if (version == 0) {
        if (fgets(line, 255, file) == NULL) {
            DebugLogAll("Could not read exit spot count from level file.\n");
            fclose(file);
            return 0;
        }
        n = atoi(line);
        for (j = 0; j < n; j++) {
            if (fgets(line, 255, file) == NULL) {
                DebugLogAll("Could not read exit spot %d.\n", j);
                fclose(file);
                return 0;
            }
        }
    }
    if (version == 0) {
        if (fgets(line, 255, file) == NULL) {
            DebugLogAll("Could not read rebirth spot count from level file.\n");
            fclose(file);
            return 0;
        }
        n = atoi(line);
        for (j = 0; j < n; j++) {
            if (fgets(line, 255, file) == NULL) {
                DebugLogAll("Could not read rebirth spot %d.\n", j);
                fclose(file);
                return 0;
            }
        }
    }
    if (version == 0) {
        if (fgets(line, 255, file) == NULL) {
            DebugLogAll("Could not read landing spot count from level file.\n");
            fclose(file);
            return 0;
        }
        n = atoi(line);
        for (j = 0; j < n; j++) {
            if (fgets(line, 255, file) == NULL) {
                DebugLogAll("Could not read landing spot %d.\n", j);
                fclose(file);
                return 0;
            }
        }
    }

    // robo spots: V1.0 "x face", V1.1 "kind face pos up dir", V1.2 "name surface face pos up dir"
    if (fgets(line, 255, file) == NULL) {
        DebugLogAll("Could not read robo spots count from level file.\n");
        fclose(file);
        return 0;
    }
    count = atoi(line);
    for (i = 0; i < count; i++) {
        _SqueezilSpotInfo *robo = new _SqueezilSpotInfo;
        if (fgets(line, 255, file) == NULL) {
            DebugLogAll("Could not read robo instance %d.\n", i);
            fclose(file);
            delete robo;
            return 0;
        }
        if (version == 0) {
            int unused;
            if (sscanf(line, "%d %d", &unused, &robo->contact.second) != 2) {
                DebugLogAll("Could not read robo spot %d information.\n", i);
                fclose(file);
                delete robo;
                return 0;
            }
            robo->kind = 0;
            robo->up.x = 0.0f; robo->up.y = 1.0f; robo->up.z = 0.0f;
            robo->direction.x = 0.0f; robo->direction.y = 0.0f; robo->direction.z = 1.0f;
            strcpy(robo->name, brains.KeyAt(robo->kind).c_str());
            robo->contact.face = 0;
        } else if (version == 1) {
            if (sscanf(line, "%d %d %f %f %f %f %f %f %f %f %f", &robo->kind, &robo->contact.second,
                       &robo->position.x, &robo->position.y, &robo->position.z,
                       &robo->up.x, &robo->up.y, &robo->up.z,
                       &robo->direction.x, &robo->direction.y, &robo->direction.z) != 11) {
                DebugLogAll("Could not read robo spot %d information.\n", i);
                fclose(file);
                delete robo;
                return 0;
            }
            strcpy(robo->name, brains.KeyAt(robo->kind).c_str());
            robo->contact.face = 0;
        } else if (version == 2) {
            if (sscanf(line, "%s %d %d %f %f %f %f %f %f %f %f %f", robo->name, &robo->contact.face,
                       &robo->contact.second, &robo->position.x, &robo->position.y, &robo->position.z,
                       &robo->up.x, &robo->up.y, &robo->up.z,
                       &robo->direction.x, &robo->direction.y, &robo->direction.z) != 12) {
                DebugLogAll("Could not read robo spot %d information.\n", i);
                fclose(file);
                delete robo;
                return 0;
            }
        }
        robo->up.Normalize();
        robo->direction.Normalize();
        squeezilSpots.Insert(robo, -1);
    }
    fclose(file);
    return 1;
}
