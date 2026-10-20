// RegionSimulation::ReadLocation (SQZ.EXE 0x4178e0, ret 0xc): build the place record of one [LocationList] entry from its
// own ini section: type (place, portal, navspot, key or relic), positions, destinations, links and function. A place gets
// a frame of its own at its prop position and is registered; anything else shows its prop. Returns the record, also when
// the type is unknown (then nothing else is read).
#include <string.h>
#pragma intrinsic(strcmp, strlen)
#include "IniReader/IniReader.h"
#include "Container/ContainerSet.h"
void DebugLog(const char *format, ...);                  // 0x419290

typedef PrimitiveArray<int, 1> IntList;
struct LocVector {
    float x, y, z;
    LocVector() {}
    LocVector(float a, float b, float c) : x(a), y(b), z(c) {}
    ~LocVector();                                        // 0x401660
    LocVector &operator=(const LocVector &other) {
        x = other.x; y = other.y; z = other.z;
        return *this;
    }
};
struct LocMatrix {                                       // 64 bytes
    float m[16];
    LocMatrix() {}
    ~LocMatrix();                                        // 0x402310
    LocMatrix &SetTranslation(const float *t);           // 0x47e330
};
struct PlaceDescriptor {                                 // SIZE 0x44
    LocVector position;                                  // 0x00
    IntList *destinations;                               // 0x0c
    IntList *links;                                      // 0x10
    String function;                                     // 0x14
    LocVector propPosition;                              // 0x18
    int frame;                                           // 0x24
    LocVector propScale;                                 // 0x28
    void *prop;                                          // 0x34
    int state;                                           // 0x38
    int kind;                                            // 0x3c 0 place, 1 navspot, 2 portal, 3 key or relic
    int id;                                              // 0x40
    PlaceDescriptor();                                   // 0x416c60
};
struct LocProp {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual void Show(void *where);                      // slot 3 (0x0c)
};
struct LocScene {
    int NewHandle();                                     // 0x476f40
    void SetTransform(int handle, LocMatrix *m);         // 0x477040
    void *GetChild();                                    // 0x477290
};
extern LocScene *g_scene;                                // 0x4c75e0
LocProp *LoadProp(IniReader *ini, void *props, const char *name, const char *section);   // 0x42a520 (cdecl)

struct RegionSimulation {
    char pad[0x2c];
    int locationCount;                                   // 0x2c

    PlaceDescriptor *ReadLocation(IniReader *ini, const char *name, int id);
    void Link(PlaceDescriptor *place, int id);           // 0x4180e0
};

PlaceDescriptor *RegionSimulation::ReadLocation(IniReader *ini, const char *name, int id)
{
    PlaceDescriptor *place = new PlaceDescriptor;
    place->id = id;
    ini->SetSection(name);
    String placeName(name);
    String type(ini->GetString("Type"));
    if (strcmp(type.c_str(), "place") && strcmp(type.c_str(), "portal") && strcmp(type.c_str(), "navspot") &&
        strcmp(type.c_str(), "key") && strcmp(type.c_str(), "relic")) {
        DebugLog("error: type %s not recognized!\n", type.rep);
        return place;
    }

    locationCount++;
    int incomplete = 0;
    if (strlen(ini->GetString("Location")) == 0) {
        place->position = LocVector(0.0f, 0.0f, 0.0f);
        incomplete = 1;
    }
    ini->GetFloats("Location", &place->position.x);
    if (strlen(ini->GetString("PropPosition")) == 0) {
        place->propPosition = LocVector(0.0f, 0.0f, 0.0f);
        incomplete = 1;
    }
    float *propPosition = &place->propPosition.x;
    ini->GetFloats("PropPosition", propPosition);
    if (strlen(ini->GetString("PropScale")) == 0) {
        place->propScale = LocVector(1.0f, 1.0f, 1.0f);
        incomplete = 1;
    }
    ini->GetFloats("PropScale", &place->propScale.x);

    if (!strcmp(type.c_str(), "place") || !strcmp(type.c_str(), "portal")) {
        if (strlen(ini->GetString("Destination")) == 0) {
            DebugLog("error: no Destination entry!\n");
            place->destinations = new IntList;
            incomplete = 1;
        } else {
            place->destinations = ini->GetIntList("Destination");
        }
    } else {
        place->destinations = new IntList;
    }
    if (strlen(ini->GetString("Links")) == 0) {
        DebugLog("error: no Links entry!\n");
        place->links = new IntList;                       // leaked: replaced right below, as in the original
        incomplete = 1;
    }
    place->links = ini->GetIntList("Links");
    const char *function = ini->GetString("Function");
    if (strlen(function))
        place->function = String(function);
    if (!incomplete)
        DebugLog("data OK,\n  ");

    if (!strcmp(type.c_str(), "place")) {
        LocMatrix m;
        m.m[0] = 1.0f;  m.m[1] = 0.0f;  m.m[2] = 0.0f;  m.m[3] = 0.0f;
        m.m[4] = 0.0f;  m.m[5] = 1.0f;  m.m[6] = 0.0f;  m.m[7] = 0.0f;
        m.m[8] = 0.0f;  m.m[9] = 0.0f;  m.m[10] = 1.0f; m.m[11] = 0.0f;
        m.m[12] = 0.0f; m.m[13] = 0.0f; m.m[14] = 0.0f; m.m[15] = 1.0f;
        m.SetTranslation(propPosition);
        place->frame = g_scene->NewHandle();
        g_scene->SetTransform(place->frame, &m);
        Link(place, id);
        place->kind = 0;
        return place;
    }
    if (!strcmp(type.c_str(), "navspot"))
        place->kind = 1;
    else if (!strcmp(type.c_str(), "portal"))
        place->kind = 2;
    else
        place->kind = 3;

    String prop(ini->GetString("Prop"));
    if (strlen(prop.c_str()) == 0) {
        DebugLog("warning: no prop.\n");
        return place;
    }
    LocProp *shown = LoadProp(ini, 0, placeName.c_str(), "Prop");
    if (shown)
        shown->Show(g_scene->GetChild());
    else
        DebugLog("ERROR: couldn't locate prop %s at location!\n", prop.c_str());
    return place;
}
