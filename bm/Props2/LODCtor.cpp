// LODProp constructor (0x428050): copy thresholds and extend them for remaining parts.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
#include "Container/ContainerSet.h"
struct LODCtorNames { void *vtable; int count; };
struct LODCtorMessages;
struct LODCtorFrame {
    virtual ~LODCtorFrame(); // 0x426eb0
    char data[0xac - 4];
    LODCtorFrame(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
        LODCtorNames *names, void *cycle, LODCtorMessages *messages); // 0x426c70
};
struct LODCtor : LODCtorFrame {
    int current;
    PrimitiveArray<float, 2> thresholds;
    LODCtor(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
        LODCtorNames *names, void *cycle, LODCtorMessages *messages, Array<float> *distances);
};
LODCtor::LODCtor(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
    LODCtorNames *names, void *cycle, LODCtorMessages *messages, Array<float> *distances)
    : LODCtorFrame(position, scale, direction, up, names, cycle, messages)
{
    current = 0;
    for (int i = 0; i < distances->count; ++i)
        thresholds.Insert(distances->Get(i), -1);
    if (names->count > thresholds.count) {
        for (int i = thresholds.count; i < names->count; ++i)
            thresholds.Insert(thresholds.Get(i - 1), -1);
    }
}
