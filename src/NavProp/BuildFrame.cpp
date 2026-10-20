// NavProp::BuildFrame (0x429ab0): attach a navigation surface to the prop's frame.
#include "String/String.h"
struct BuildMatrix {
    float m[16];
    BuildMatrix() {}
    ~BuildMatrix(); // 0x402310
};
struct BuildPlacement {
    int instance, sub;
    BuildMatrix matrix;
    BuildPlacement() { instance = -1; sub = 0; }
    ~BuildPlacement(); // 0x4031e0
};
struct BuildProperties;
struct BuildTables {
    int Add(void *prop, int surface, BuildMatrix *matrix, BuildProperties *properties); // 0x4100b0
};
struct BuildPlace { BuildTables *GetSurfaces(); }; // 0x439b60
struct BuildScene {
    void *GetSceneChild(); // 0x477290
    void Transform(void *frame, void *relative, BuildMatrix *out); // 0x477120
};
extern BuildScene *g_buildScene;
void *GetBuildSimulation();
void *BuildDynamicCast(void *, int, const void *, const void *, int);
extern const char kBuildSimType[], kBuildPlaceType[];
struct BuildPart {
    virtual void v0(); virtual void v1(); virtual void *GetFrame();
};
struct BuildPartTable { BuildPart *Find(const String &key); }; // 0x46b110
struct BuildBody { int pad; BuildPartTable parts; };
struct BuildCycle { void *BuildFrame(void *parent); }; // 0x428d80
struct NavBuild : BuildCycle {
    char pad[0x34];
    void *frame;
    char pad2[0x94 - 0x38];
    BuildBody *body;
    char pad3[0xf4 - 0x98];
    int surface, instance;
    char properties[1];
    void *BuildFrame(void *parent);
};
void *NavBuild::BuildFrame(void *parent)
{
    BuildCycle::BuildFrame(parent);
    void *partFrame = frame;
    if (body) {
        BuildPart *part = body->parts.Find(String("part"));
        partFrame = part->GetFrame();
    }
    BuildMatrix matrix;
    g_buildScene->Transform(partFrame, g_buildScene->GetSceneChild(), &matrix);
    int id = ((BuildPlace *)BuildDynamicCast(GetBuildSimulation(), 0,
        kBuildSimType, kBuildPlaceType, 0))->GetSurfaces()->Add(this, surface, &matrix,
            (BuildProperties *)properties);
    BuildPlacement placement;
    void *result = frame;
    instance = id;
    placement.instance = instance;
    return result;
}
