// FrameProp constructor (0x426c70): initialize child slots and copy clip messages.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
#include "Ini/IniFile.h"
typedef BufferObjectTable<float, String, StringList, 25> FrameCtorMessages;
struct FrameCtorSource {
    FrameCtorMessages table;
    String ValueAt(int index); // 0x427ae0
};
struct FrameCtorProp {
    virtual ~FrameCtorProp();
    Vector3f position, scale, direction, up;
    void *frame, *owner;
    int isInstance;
    FrameCtorProp(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up); // 0x425e50
};
struct FrameCtor : FrameCtorProp {
    StringList childNames;
    Array<FrameCtorProp *> children;
    FrameCtorMessages messages;
    void *body, *cycle;
    int actorIndex, followMode;
    float startTime;
    int hideAtEnd;
    FrameCtor(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
        StringList *names, void *clip, FrameCtorSource *source);
};
FrameCtor::FrameCtor(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
    StringList *names, void *clip, FrameCtorSource *source)
    : FrameCtorProp(position, scale, direction, up)
{
    frame = 0;
    for (int i = 0; i < names->count; ++i) {
        childNames.Insert(names->Get(i), -1);
        children.Insert((FrameCtorProp *)0, -1);
    }
    cycle = clip;
    actorIndex = -1;
    body = 0;
    startTime = 0;
    followMode = 0;
    hideAtEnd = 0;
    for (int i = 0; i < source->table.keys.count; ++i)
        messages.Insert(source->table.keys.Get(i), source->ValueAt(i), -1);
}
