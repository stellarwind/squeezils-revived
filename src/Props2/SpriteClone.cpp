// SpriteProp::Clone (virtual slot 1, SQZ.EXE 0x4269a0): a new sprite prop with the same name, position, scale and settings.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"

struct SpriteOwner;
struct SpriteName { const char *c_str(); };              // 0x438530
struct SpriteSettings { int a, b; };
struct SpriteProp {
    virtual ~SpriteProp();
    virtual SpriteProp *Clone(SpriteOwner *owner);       // slot 1  0x4269a0
    Vector3f position;                                   // 0x04
    Vector3f scale;                                      // 0x10
    char pad1c[0x38 - 0x1c];
    SpriteOwner *owner;                                  // 0x38
    int isInstance;                                      // 0x3c
    SpriteName name;                                     // 0x40
    SpriteSettings settings;                             // 0x44
    char pad4c[0x70 - 0x4c];
    SpriteProp(const char *name, Vector3f position, Vector3f scale, SpriteSettings *settings);   // 0x4267b0
};

SpriteProp *SpriteProp::Clone(SpriteOwner *newOwner)
{
    SpriteProp *n = new SpriteProp(name.c_str(), position, scale, &settings);
    n->owner = newOwner;
    n->isInstance = 1;
    return n;
}
