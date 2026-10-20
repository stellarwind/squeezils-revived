// SpriteProp::BuildFrame (virtual slot 3, SQZ.EXE 0x426b00): a decal of the sprite's texture, sized by the scale.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"

struct SpriteFrame;
struct SpriteTexture;
struct SpriteName { const char *c_str(); };              // 0x438530
struct DecalSettings {                                   // 0x2c bytes, out-of-line copy ctor 0x425970 / dtor 0x425a30
    int d[11];
    DecalSettings(const DecalSettings &o);
    ~DecalSettings();
};
struct SpriteWorld {
    SpriteFrame *CreateDecal(SpriteTexture *tex, Vector3f position, DecalSettings settings, SpriteFrame *parent);   // 0x435bb0
};
SpriteTexture *FindTexture(const char *name);            // 0x425220
int DebugLogAll(const char *fmt, ...);                      // 0x419290
SpriteWorld *GetWorld();                                 // 0x4206a0

struct SpriteProp {
    virtual ~SpriteProp();
    virtual void v1();
    virtual void v2();
    virtual SpriteFrame *BuildFrame(SpriteFrame *parent);   // slot 3  0x426b00
    virtual void v4();
    virtual void v5();
    virtual void ApplyScale(SpriteFrame *frame, Vector3f scale);   // slot 6
    Vector3f position;                                   // 0x04
    Vector3f scale;                                      // 0x10
    char pad1c[0x40 - 0x1c];
    SpriteName name;                                     // 0x40
    DecalSettings settings;                              // 0x44
};

SpriteFrame *SpriteProp::BuildFrame(SpriteFrame *parent)
{
    const char *textureName = name.c_str();
    SpriteTexture *texture = FindTexture(name.c_str());
    if (texture == 0) {
        DebugLogAll("ERROR: sprite %s not loaded from textureResources\n", textureName);
        return 0;
    }
    SpriteFrame *decal = GetWorld()->CreateDecal(texture, position, settings, parent);
    ApplyScale(decal, scale);
    return decal;
}
