// SpriteProp constructor, 0x4267b0.
#define V3_COPYCTOR_OUTOFLINE
#define V3_CTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
#include "String/String.h"
struct SpriteCtorBase {
    virtual ~SpriteCtorBase(); // 0x426080
    Vector3f position, scale, direction, up;
    void *frame, *owner;
    int isInstance;
    SpriteCtorBase(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up); // 0x425e50
};
struct SpriteCtorSettings {
    int id;
    Vector3f a, b;
    int mode;
    Vector3f c;
    ~SpriteCtorSettings(); // 0x425a30
};
struct SpriteCtor : SpriteCtorBase {
    String name;
    SpriteCtorSettings settings;
    SpriteCtor(const char *text, Vector3f position, Vector3f scale, SpriteCtorSettings *defaults);
};
SpriteCtor::SpriteCtor(const char *text, Vector3f position, Vector3f scale,
    SpriteCtorSettings *defaults)
    : SpriteCtorBase(position, scale, Vector3f(0, 0, 1), Vector3f(0, 1, 0))
{
    name = String(text);
    settings = *defaults;
}
