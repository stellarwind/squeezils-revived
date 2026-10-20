// AnimatedBody copy constructor (0x41dab0): clone each named body part.
#include "String/String.h"
struct ClonePart {
    char data[0x10];
    ClonePart(const ClonePart *source); // 0x41d8c0
};
struct CloneTable {
    virtual void Add(const String &key, ClonePart *const &part, int index);
    char data[0x24];
    CloneTable(); // 0x46acf0
    ~CloneTable(); // 0x46afa0
    String KeyAt(int index); // 0x41dc00
};
extern void *kCloneBodyVtable[]; // 0x49d7d4
struct CloneBody {
    void *vtable;
    CloneTable parts; // 0x04
    void *cycle;
    float time;
    int hint, direction, loopMode;
    void *pose;
    void *previousPose;
    float blendLeft;
    void *rootFrame;
    CloneBody(CloneBody *source);
};
CloneBody::CloneBody(CloneBody *source)
{
    rootFrame = 0;
    cycle = 0;
    time = 0;
    hint = 0;
    direction = 1;
    loopMode = 1;
    vtable = kCloneBodyVtable;
    previousPose = 0;
    blendLeft = 0;
    for (int i = 0; i < *(const int *)((const char *)source + 0x0c); ++i) {
        parts.Add(source->parts.KeyAt(i), new ClonePart(((ClonePart *const *)
            *(void *const *)((const char *)source + 0x28))[i]), -1);
    }
    pose = 0;
}
