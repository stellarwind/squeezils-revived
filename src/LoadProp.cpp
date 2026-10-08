// LoadProp: reading one prop's definition from its ini section. See PropLoader.h. Reconstructed from SQZ.EXE 0x42a520-0x42c700
// (LoadProp 0x42a520, the placement keys 0x42bfd0, and the helpers 0x421db0 and 0x42cc10).
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PropLoader.h"
#include "Prop.h"
#include "FrameProp.h"
#include "LODProp.h"
#include "CycleProp.h"
#include "NavProp.h"
#include "Animation.h"
#include "Resources.h"
#include "World.h"
#include "Vector3f.h"

#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"

static PropTable *const kSecondProps = (PropTable *)0x4c7230;
static PathTable *const kLevelPaths  = (PathTable *)0x4c7280;
static PathTable *const kSecondPaths = (PathTable *)0x4c7258;
static PathTable *const kThirdPaths  = (PathTable *)0x4c71d0;
static DecalSettings *const kDefaultDecal = (DecalSettings *)0x4c7190;

// ---- placement ----

// The 4x4 rotation matrices the Rotation key turns into a direction and an up vector (row vectors, 16 floats, rows 0..3)
// @0047dcd0 / 0047dd20 / 0047dd70
static void RotationX(float m[16], float angle)
{
    memset(m, 0, 16 * sizeof(float));
    m[0] = 1.0f; m[15] = 1.0f;
    m[5] = cosf(angle); m[6] = sinf(angle);
    m[9] = -sinf(angle); m[10] = cosf(angle);
}
static void RotationY(float m[16], float angle)
{
    memset(m, 0, 16 * sizeof(float));
    m[5] = 1.0f; m[15] = 1.0f;
    m[0] = cosf(angle); m[2] = -sinf(angle);
    m[8] = sinf(angle); m[10] = cosf(angle);
}
static void RotationZ(float m[16], float angle)
{
    memset(m, 0, 16 * sizeof(float));
    m[10] = 1.0f; m[15] = 1.0f;
    m[0] = cosf(angle); m[1] = sinf(angle);
    m[4] = -sinf(angle); m[5] = cosf(angle);
}
// result = a * b                                                                                                       @0047dea0
static void Multiply(float result[16], const float a[16], const float b[16])
{
    for (int row = 0; row < 4; row++)
        for (int column = 0; column < 4; column++)
            result[row * 4 + column] = a[row * 4 + 0] * b[0 * 4 + column] + a[row * 4 + 1] * b[1 * 4 + column] +
                                       a[row * 4 + 2] * b[2 * 4 + column] + a[row * 4 + 3] * b[3 * 4 + column];
}

// The key name: the prefix, then the key
static void PlacementKey(char key[184], const char *prefix, const char *name)
{
    strcpy(key, prefix ? prefix : "");
    strcat(key, name);
}

static void MakeUnit(Float3 *v)
{
    Vector3f unit(v->x, v->y, v->z);
    Normalize(unit);
    v->x = unit.x; v->y = unit.y; v->z = unit.z;
}

// @0042bfd0
void ReadPlacement(IniReader *ini, const char *prefix, Float3 *position, Float3 *scale, Float3 *direction, Float3 *up)
{
    char key[184];

    PlacementKey(key, prefix, "Position");
    if (strlen(ini->GetString(key)) == 0) {
        GameLog("WARNING: no position for prop.\n");
        position->x = 0.0f; position->y = 0.0f; position->z = 0.0f;
    } else {
        ini->GetFloats(key, (float *)position);
    }

    PlacementKey(key, prefix, "Scale");
    if (strlen(ini->GetString(key)) == 0) {
        scale->x = 1.0f; scale->y = 1.0f; scale->z = 1.0f;
    } else {
        ini->GetFloats(key, (float *)scale);
    }

    PlacementKey(key, prefix, "Rotation");
    if (strlen(ini->GetString(key)) != 0) {
        // three angles in degrees: turned about x, then y, then z
        float angles[3];
        ini->GetFloats(key, angles);
        const float degrees = 0.017453292f;
        float m[16] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };
        float turn[16], product[16];
        RotationX(turn, angles[0] * degrees);
        Multiply(product, m, turn);
        memcpy(m, product, sizeof m);
        RotationY(turn, angles[1] * degrees);
        Multiply(product, m, turn);
        memcpy(m, product, sizeof m);
        RotationZ(turn, angles[2] * degrees);
        Multiply(product, m, turn);
        memcpy(m, product, sizeof m);
        direction->x = m[8]; direction->y = m[9]; direction->z = m[10];
        up->x = m[4]; up->y = m[5]; up->z = m[6];
        return;
    }

    PlacementKey(key, prefix, "RotationZdir");
    if (strlen(ini->GetString(key)) == 0) {
        direction->x = 0.0f; direction->y = 0.0f; direction->z = 1.0f;
    } else {
        ini->GetFloats(key, (float *)direction);
        MakeUnit(direction);
    }

    PlacementKey(key, prefix, "RotationYup");
    if (strlen(ini->GetString(key)) == 0) {
        up->x = 0.0f; up->y = 1.0f; up->z = 0.0f;
    } else {
        ini->GetFloats(key, (float *)up);
        MakeUnit(up);
    }
}

// ---- small helpers ----

// @00421db0
StringList *SplitWords(const char *text)
{
    StringList *words = new StringList();
    String copy(text);
    char *p = (char *)copy.c_str();                 // the copy's own text, cut in place
    while (*p != 0) {
        char *space = strchr(p, ' ');
        if (space == NULL) {
            words->Insert(String(p));
            p += strlen(p);
        } else {
            *space = 0;
            words->Insert(String(p));
            p = space + 1;
            while (*p == ' ')
                p++;
        }
    }
    return words;
}

// @0042cc10
MorphCycle *FindPath(const char *name)
{
    PathTable *tables[3] = { kThirdPaths, kSecondPaths, kLevelPaths };
    for (int i = 0; i < 3; i++) {
        if (tables[i]->Contains(String(name)))
            return tables[i]->Get(String(name));
    }
    return NULL;
}

// A key that begins with the given word (the entries PathFunction1, PathFunction2 ... share their start)
static bool StartsWith(const String &key, const char *word)
{
    return strstr(key.c_str(), word) == key.c_str();
}

// ---- LoadProp ----

// Files the new template in the table under its name; one already there is destroyed first.
static void FileProp(PropTable *table, const char *name, Prop *prop)
{
    if (table->Contains(String(name))) {
        Prop *old = table->Get(String(name));
        table->RemoveKey(String(name));
        if (old)
            old->VDestroy(1);
    }
    table->Insert(String(name), prop);
}

// A child prop of a group: its template must exist, so load it from its own section when it does not
static void NeedChild(IniReader *ini, PropTable *table, const String &child)
{
    if (!FindPropTemplate(child.c_str(), 0, 0)) {
        if (!LoadProp(ini, table, child.c_str(), NULL))
            GameLog("ERROR: couldn't find child prop %s.\n", child.c_str());
    }
}

static const Float3 kOrigin = { 0.0f, 0.0f, 0.0f };
static const Float3 kUnitScale = { 1.0f, 1.0f, 1.0f };
static const Float3 kForward = { 0.0f, 0.0f, 1.0f };
static const Float3 kUpward = { 0.0f, 1.0f, 0.0f };

// @0042a520
Prop *LoadProp(IniReader *ini, PropTable *table, const char *name, const char *kind)
{
    GameLog("Loading prop %s: ", name);
    String nameCopy(name);
    name = nameCopy.c_str();
    if (table == NULL)
        table = kSecondProps;
    Prop *prop = NULL;
    ini->SetSection(name);

    String type;
    if (kind != NULL && strlen(kind) != 0)
        type = String("Prop");
    else
        type = String(ini->GetString("Type"));

    // No Type: the section's name is a geometry, or else a texture, already loaded
    if (strlen(type.c_str()) == 0) {
        if (FindGeometry(name)) {
            MeshProp *mesh = (MeshProp *)Operator_New(sizeof(MeshProp));
            if (mesh)
                mesh = mesh->Init(name, kOrigin, kUnitScale, kForward, kUpward);
            prop = mesh;
        } else if (FindTexture(name)) {
            DecalSettings decal = *kDefaultDecal;
            SpriteProp *sprite = (SpriteProp *)Operator_New(sizeof(SpriteProp));
            if (sprite)
                sprite = sprite->Init(name, kOrigin, kUnitScale, &decal);
            prop = sprite;
        }
        if (prop)
            FileProp(table, name, prop);
        return prop;
    }

    Float3 position, scale, direction, up;

    if (stricmp(type.c_str(), "Mesh") == 0) {
        String meshName(ini->GetString("Mesh"));
        if (strlen(meshName.c_str()) == 0) {
            GameLog("error, no Mesh entry!\n");
            return NULL;
        }
        ReadPlacement(ini, "", &position, &scale, &direction, &up);
        MeshProp *mesh = (MeshProp *)Operator_New(sizeof(MeshProp));
        if (mesh)
            mesh = mesh->Init(meshName.c_str(), position, scale, direction, up);
        prop = mesh;
    } else if (stricmp(type.c_str(), "Sprite") == 0) {
        String spriteName(ini->GetString("Sprite"));
        if (strlen(spriteName.c_str()) == 0) {
            GameLog("error, no Sprite entry!\n");
            return NULL;
        }
        DecalSettings defaults = *kDefaultDecal;
        DecalSettings decal;
        GetWorld()->ReadDecalSettings(&decal, ini, nameCopy.c_str(), "Sprite", defaults);
        ReadPlacement(ini, "", &position, &scale, &direction, &up);
        SpriteProp *sprite = (SpriteProp *)Operator_New(sizeof(SpriteProp));
        if (sprite)
            sprite = sprite->Init(spriteName.c_str(), position, scale, &decal);
        prop = sprite;
    } else if (stricmp(type.c_str(), "Prop") == 0 || stricmp(type.c_str(), "Frame") == 0 ||
               stricmp(type.c_str(), "LODProp") == 0 || stricmp(type.c_str(), "Cycle") == 0 ||
               stricmp(type.c_str(), "Nav") == 0) {
        // the group types: the children first, "Prop" and then Child1 ... ChildCount
        StringList childNames;
        String first(ini->GetString("Prop"));
        if (strlen(first.c_str()) == 0) {
            GameLog("WARNING: no Prop entry.\n");
        } else {
            childNames.Insert(first);
            NeedChild(ini, table, first);
        }
        ini->SetSection(nameCopy.c_str());
        int childCount = ini->GetInt("ChildCount", 0);
        for (int number = 1; number < childCount + 1; number++) {
            char key[200];
            sprintf(key, "Child%d", number);
            ini->SetSection(nameCopy.c_str());
            String child(ini->GetString(key));
            if (strlen(child.c_str()) == 0) {
                GameLog("WARNING: no entry for child number %d, skipping...\n", number);
            } else {
                childNames.Insert(child);
                NeedChild(ini, table, child);
            }
        }
        ini->SetSection(nameCopy.c_str());
        ReadPlacement(ini, kind, &position, &scale, &direction, &up);

        // an animation path, with script functions at times in it
        MorphCycle *path = NULL;
        String pathName(ini->GetString("Path"));
        if (strlen(pathName.c_str()) != 0)
            path = FindPath(pathName.c_str());
        CycleMessageTable messages;
        if (path) {
            StringList *keys = ini->GetKeys();
            for (int i = 0; i < keys->count; i++) {
                if (!StartsWith(keys->At(i), "PathFunction"))
                    continue;
                String line(ini->GetString(keys->At(i).c_str()));
                StringList *words = SplitWords(line.c_str());
                if (words->count != 2) {
                    GameLog("WARNING: path function entry %s doesn't have proper form.\n", keys->At(i).c_str());
                } else {
                    float time = (float)atof(words->At(0).c_str());
                    messages.Insert(time, words->At(1));
                }
                delete words;
            }
            delete keys;
        }

        if (stricmp(type.c_str(), "Prop") == 0 || stricmp(type.c_str(), "Frame") == 0) {
            FrameProp *frame = (FrameProp *)Operator_New(sizeof(FrameProp));
            if (frame)
                frame = frame->Init(position, scale, direction, up, &childNames, path, &messages);
            prop = frame;
        } else if (stricmp(type.c_str(), "LODProp") == 0) {
            // squared distances: Falloff1 ... for each child, at least a step of Falloff further than the one before
            float step = ini->GetFloat("Falloff", 20.0f);
            WeightList distances;
            float distance = 0.0f;
            for (int number = 1; number <= childNames.count; number++) {
                char key[200];
                sprintf(key, "Falloff%d", number);
                ini->SetSection(nameCopy.c_str());
                String text(ini->GetString(key));
                if (strlen(text.c_str()) == 0) {
                    distance += step;
                } else {
                    float asked = (float)atof(text.c_str());
                    if (asked > distance)
                        distance = asked;
                    else
                        distance += step;
                }
                distances.Insert(distance * distance);
            }
            LODProp *lod = (LODProp *)Operator_New(sizeof(LODProp));
            if (lod)
                lod = lod->Init(position, scale, direction, up, &childNames, path, &messages, &distances);
            prop = lod;
        } else {
            // Cycle and Nav: FrameFunction1 ... say what to run when a frame number is passed
            int useRealTime = ini->GetInt("UseRealTime", 0);
            float frameDuration = ini->GetFloat("FrameDuration", 0.1f);
            StepEventTable events;
            StringList *keys = ini->GetKeys();
            for (int i = 0; i < keys->count; i++) {
                if (!StartsWith(keys->At(i), "FrameFunction"))
                    continue;
                String line(ini->GetString(keys->At(i).c_str()));
                StringList *words = SplitWords(line.c_str());
                if (words->count != 2) {
                    GameLog("WARNING: path function entry %s doesn't have proper form.\n", keys->At(i).c_str());
                } else {
                    int frameNumber = atoi(words->At(0).c_str()) - 1;
                    events.Insert(frameNumber, words->At(1));
                }
                delete words;
            }
            delete keys;

            if (stricmp(type.c_str(), "Cycle") == 0) {
                CycleProp *cycleProp = (CycleProp *)Operator_New(sizeof(CycleProp));
                if (cycleProp)
                    cycleProp = cycleProp->Init(position, scale, direction, up, &childNames, path, &messages, &events,
                                                frameDuration, useRealTime, 0.0f);
                prop = cycleProp;
            } else {
                // Nav: the surface file, and SurfaceBinding1 ... (face set name, surface type)
                ini->SetSection(nameCopy.c_str());
                String navFile(ini->GetString("NavFile"));
                String surfaceFile(ini->GetString("SurfaceFile"));
                IniValueTable *bindings = new IniValueTable();
                keys = ini->GetKeys();
                for (int i = 0; i < keys->count; i++) {
                    if (!StartsWith(keys->At(i), "SurfaceBinding"))
                        continue;
                    String line(ini->GetString(keys->At(i).c_str()));
                    StringList *words = SplitWords(line.c_str());
                    if (words->count != 2)
                        GameLog("WARNING: surface binding entry %s doesn't have form 'face-set-name surface-type'.\n",
                                keys->At(i).c_str());
                    else
                        bindings->Insert(words->At(0), words->At(1));
                    delete words;
                }
                delete keys;
                NavProp *nav = (NavProp *)Operator_New(sizeof(NavProp));
                if (nav)
                    nav = nav->Init(position, scale, direction, up, &childNames, path, &messages, &events, frameDuration,
                                    useRealTime, 0.0f, ini, navFile.c_str(), surfaceFile.c_str(), bindings);
                prop = nav;
                delete bindings;
            }
        }
    } else {
        GameLog("error: unknown type %s!\n", type.c_str());
        return NULL;
    }

    if (prop)
        FileProp(table, name, prop);
    return prop;
}
