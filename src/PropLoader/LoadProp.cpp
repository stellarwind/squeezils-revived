// SQZ.EXE 0x42a520 (cdecl): LoadProp - build the prop of one INI section from its Type key (Mesh, Sprite, Prop/Frame,
// LODProp, Cycle, Nav; no Type = guess Mesh or Sprite from the resource name), load its child props, and store it in
// the prop table under the section name (replacing and deleting an older prop of that name). Returns the prop or 0.
// The local tables are the real container templates; their constructors are inlined around the base constructor.
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define V3_COPYCTOR_OUTOFLINE
#define V3_CTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
#include "IniReader/IniReader.h"
#include "World/WorldDecal.h"

struct Prop {
    virtual ~Prop();                                     // vtable+0x00
};
struct PropTable {                                       // Table<String, Prop *>
    virtual int Insert(const String &name, Prop *const &prop, int hint);   // vtable+0x00
    virtual int Contains(const String &name);           // vtable+0x04
    Prop *Get(const String &name);                       // 0x468ab0
};
extern PropTable gProps;                                 // 0x4c7230

typedef BufferObjectTable<float, String, PrimitiveArray<String, 3>, 25> PropFloatTable;   // ctor 0x464220
typedef BufferObjectTable<int, String, PrimitiveArray<String, 3>, 24> PropIntTable;       // ctor 0x4635e0
typedef PrimitiveArray<float, 2> PropFloatList;                                          // ctor 0x45dd40

// SIZE 0x44
struct MeshProp : Prop {
    char pad[0x44 - 4];
    MeshProp(const char *name, Vector3f position, Vector3f scale, Vector3f direction, Vector3f up);   // 0x4262e0
};
// SIZE 0x70
struct SpriteProp : Prop {
    char pad[0x70 - 4];
    SpriteProp(const char *name, Vector3f position, Vector3f scale, WorldDecalSettings *settings);   // 0x4267b0
};
// SIZE 0xac
struct FrameProp : Prop {
    char pad[0xac - 4];
    FrameProp(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
        StringList *names, void *cycle, PropFloatTable *messages);   // 0x426c70
};
// SIZE 0xc4
struct LODProp : Prop {
    char pad[0xc4 - 4];
    LODProp(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
        StringList *names, void *cycle, PropFloatTable *messages, PropFloatList *lod);   // 0x428050
};
// SIZE 0xf4
struct CycleProp : Prop {
    char pad[0xf4 - 4];
    CycleProp(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
        StringList *names, void *cycle, PropFloatTable *messages, PropIntTable *start,
        float stepPeriod, int keepRemainder, float clipStart);   // 0x4288b0
};
// SIZE 0x130
struct NavProp : Prop {
    char pad[0x130 - 4];
    NavProp(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
        StringList *names, void *cycle, PropFloatTable *messages, PropIntTable *start,
        float stepPeriod, int keepRemainder, float clipStart,
        IniReader *ini, const char *navFile, const char *surfaceFile, IniValueTable *bindings);   // 0x429380
};

struct PropWorld {
    WorldDecalSettings ReadDecalSettings(IniReader *ini, const char *section,
        const char *key, WorldDecalSettings defaults);   // 0x435c90
};
PropWorld *GetWorld();                                   // 0x4206a0
WorldDecalSettings GetDefaultDecalSettings();            // 0x425c00
void *FindGeometry(const char *name);                    // 0x4246f0
void *FindTexture(const char *name);                     // 0x425220
void *FindPath(const char *name);                        // 0x42cc10
void *FindPropTemplate(String name, int level, int how); // 0x42c8b0
StringList *SplitString(const char *text);               // 0x421db0
int ReadPlacement(IniReader *ini, const char *prefix, Vector3f *position,
                  Vector3f *scale, Vector3f *direction, Vector3f *up);   // 0x42bfd0
void GameLog(const char *format, ...);                   // 0x419290

Prop *LoadProp(IniReader *ini, PropTable *props, const char *name, const char *prefix)
{
    GameLog("Loading prop %s: ", name);
    String section(name);
    const char *propName = section.c_str();
    if (props == 0)
        props = &gProps;
    Prop *prop = 0;
    ini->SetSection(section.c_str());
    String type;
    if (prefix != 0 && strlen(prefix) != 0)
        type = String("Prop");
    else {
        type = String(ini->GetString("Type"));
        if (strlen(type.c_str()) == 0) {
            if (FindGeometry(propName) != 0)
                prop = new MeshProp(propName, Vector3f(0.0f, 0.0f, 0.0f), Vector3f(1.0f, 1.0f, 1.0f),
                                    Vector3f(0.0f, 0.0f, 1.0f), Vector3f(0.0f, 1.0f, 0.0f));
            else if (FindTexture(propName) != 0) {
                WorldDecalSettings settings = GetDefaultDecalSettings();
                prop = new SpriteProp(propName, Vector3f(0.0f, 0.0f, 0.0f), Vector3f(1.0f, 1.0f, 1.0f), &settings);
            }
            if (prop != 0) {
                if (props->Contains(String(propName))) {
                    Prop *old = props->Get(String(propName));
                    if (old)
                        delete old;
                }
                props->Insert(String(propName), prop, -1);
            }
            return prop;
        }
    }

    Vector3f position, scale, direction, up;
    if (stricmp(type.c_str(), "Mesh") == 0) {
        String mesh(ini->GetString("Mesh"));
        if (strlen(mesh.c_str()) == 0) {
            GameLog("error, no Mesh entry!\n");
            return 0;
        }
        ReadPlacement(ini, "", &position, &scale, &direction, &up);
        prop = new MeshProp(mesh.c_str(), position, scale, direction, up);
    } else if (stricmp(type.c_str(), "Sprite") == 0) {
        String sprite(ini->GetString("Sprite"));
        if (strlen(sprite.c_str()) == 0) {
            GameLog("error, no Sprite entry!\n");
            return 0;
        }
        WorldDecalSettings defaults = GetDefaultDecalSettings();
        WorldDecalSettings settings = GetWorld()->ReadDecalSettings(ini, section.c_str(), "Sprite", defaults);
        ReadPlacement(ini, "", &position, &scale, &direction, &up);
        prop = new SpriteProp(sprite.c_str(), position, scale, &settings);
    } else if (stricmp(type.c_str(), "Prop") != 0 && stricmp(type.c_str(), "Frame") != 0
               && stricmp(type.c_str(), "LODProp") != 0 && stricmp(type.c_str(), "Cycle") != 0
               && stricmp(type.c_str(), "Nav") != 0) {
        GameLog("error: unknown type %s!\n", type.c_str());
    } else {
        StringList names;
        String child(ini->GetString("Prop"));
        child.c_str();
        if (strlen(child.c_str()) == 0)
            GameLog("WARNING: no Prop entry.\n");
        else {
            names.Insert(child, -1);
            if (FindPropTemplate(child, 0, 0) == 0
                && LoadProp(ini, props, child.c_str(), 0) == 0)
                GameLog("ERROR: couldn't find child prop %s.\n", child.c_str());
        }
        ini->SetSection(section.c_str());
        int childCount = ini->GetInt("ChildCount", 0);
        char key[56];
        int i;
        for (i = 1; i < childCount + 1; i++) {
            sprintf(key, "Child%d", i);
            ini->SetSection(section.c_str());
            String entry(ini->GetString(key));
            if (strlen(entry.c_str()) == 0)
                GameLog("WARNING: no entry for child number %d, skipping...\n", i);
            else {
                names.Insert(entry, -1);
                if (FindPropTemplate(entry, 0, 0) == 0
                    && LoadProp(ini, props, entry.c_str(), 0) == 0)
                    GameLog("ERROR: couldn't find child prop %s.\n", entry.c_str());
            }
        }

        ini->SetSection(section.c_str());
        ReadPlacement(ini, prefix, &position, &scale, &direction, &up);
        void *cycle = 0;
        String path(ini->GetString("Path"));
        if (strlen(path.c_str()) != 0)
            cycle = FindPath(path.c_str());

        PropFloatTable messages;
        StringList *keys = ini->GetKeys();
        for (i = 0; i < keys->count; i++) {
            int isFunction = strstr(keys->Get(i).c_str(), "PathFunction") == keys->Get(i).c_str();
            if (isFunction) {
                String value(ini->GetString(keys->Get(i).c_str()));
                StringList *parts = SplitString(value.c_str());
                if (parts->count != 2) {
                    const char *bad = keys->Get(i).c_str();   // the original logs through a dead temporary
                    GameLog("WARNING: path function entry %s doesn't have proper form.\n", bad);
                    if (parts)
                        delete parts;
                } else {
                    float at = (float)atof(parts->Get(0).c_str());
                    messages.Insert(at, parts->Get(1), -1);
                    if (parts)
                        delete parts;
                }
            }
        }
        if (keys)
            delete keys;

        if (stricmp(type.c_str(), "Prop") == 0 || stricmp(type.c_str(), "Frame") == 0)
            prop = new FrameProp(position, scale, direction, up, &names, cycle, &messages);
        else if (stricmp(type.c_str(), "LODProp") == 0) {
            float falloff = ini->GetFloat("Falloff", 20.0f);
            PropFloatList lod;
            float last = 0.0f;
            for (i = 0; i < names.count; ) {
                int next = i + 1;
                char falloffKey[76];
                sprintf(falloffKey, "Falloff%d", next);
                ini->SetSection(section.c_str());
                String entry(ini->GetString(falloffKey));
                float distance;
                if (strlen(entry.c_str()) == 0)
                    distance = last + falloff;
                else {
                    distance = (float)atof(entry.c_str());
                    if (distance <= last)
                        distance = last + falloff;
                }
                last = distance;
                float squared = last * last;
                lod.Insert(squared, -1);
                i = next;
            }
            prop = new LODProp(position, scale, direction, up, &names, cycle, &messages, &lod);
        } else {
            int useRealTime = ini->GetInt("UseRealTime", 0);
            float frameDuration = ini->GetFloat("FrameDuration", 0.1f);
            PropIntTable start;
            keys = ini->GetKeys();
            for (i = 0; i < keys->count; i++) {
                int isFunction = strstr(keys->Get(i).c_str(), "FrameFunction") == keys->Get(i).c_str();
                if (isFunction) {
                    String value(ini->GetString(keys->Get(i).c_str()));
                    StringList *parts = SplitString(value.c_str());
                    if (parts->count != 2) {
                        const char *bad = keys->Get(i).c_str();
                        GameLog("WARNING: path function entry %s doesn't have proper form.\n", bad);
                        if (parts)
                            delete parts;
                    } else {
                        int frame = atoi(parts->Get(0).c_str());
                        frame--;
                        start.Insert(frame, parts->Get(1), -1);
                        if (parts)
                            delete parts;
                    }
                }
            }
            if (keys)
                delete keys;

            if (stricmp(type.c_str(), "Cycle") == 0)
                prop = new CycleProp(position, scale, direction, up, &names, cycle, &messages, &start,
                                     frameDuration, useRealTime, 0);
            else {
                ini->SetSection(section.c_str());
                String navFile(ini->GetString("NavFile"));
                String surfaceFile(ini->GetString("SurfaceFile"));
                IniValueTable *bindings = new IniValueTable;
                keys = ini->GetKeys();
                for (i = 0; i < keys->count; i++) {
                    int isBinding = strstr(keys->Get(i).c_str(), "SurfaceBinding") == keys->Get(i).c_str();
                    if (isBinding) {
                        String value(ini->GetString(keys->Get(i).c_str()));
                        StringList *parts = SplitString(value.c_str());
                        if (parts->count != 2) {
                            const char *bad = keys->Get(i).c_str();
                            GameLog("WARNING: surface binding entry %s doesn't have form 'face-set-name surface-type'.\n", bad);
                            if (parts)
                                delete parts;
                        } else {
                            bindings->Insert(parts->Get(0), parts->Get(1), -1);
                            if (parts)
                                delete parts;
                        }
                    }
                }
                if (keys)
                    delete keys;
                prop = new NavProp(position, scale, direction, up, &names, cycle, &messages, &start,
                                   frameDuration, useRealTime, 0, ini, navFile.c_str(), surfaceFile.c_str(), bindings);
                if (bindings)
                    delete bindings;
            }
        }
    }

    if (prop != 0) {
        if (props->Contains(String(name))) {
            Prop *old = props->Get(String(name));
            if (old)
                delete old;
        }
        props->Insert(String(name), prop, -1);
    }
    return prop;
}
