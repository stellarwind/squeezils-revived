// CycleProp constructor (0x4288b0): copy step events and initialize playback.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
#include "Ini/IniFile.h"
typedef BufferObjectTable<int, String, StringList, 24> CtorStepTable;
struct CycleCtorBlock;
struct CycleCtorFrame {
    virtual ~CycleCtorFrame();
    char data[0xa4 - 4];
    float startTime;
    int hideAtEnd;
    CycleCtorFrame(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
        CycleCtorBlock *names, void *cycle, CycleCtorBlock *messages); // 0x426c70
};
struct CycleCtorEvents {
    CtorStepTable table;
    String ValueAt(int index); // 0x428af0
};
struct CycleCtor : CycleCtorFrame {
    CtorStepTable events;
    int current, step, mode, carry;
    float timer, period;
    CycleCtor(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
        CycleCtorBlock *names, void *cycle, CycleCtorBlock *messages,
        CycleCtorEvents *source, float stepPeriod, int keepRemainder, float clipStart);
};
CycleCtor::CycleCtor(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
    CycleCtorBlock *names, void *cycle, CycleCtorBlock *messages,
    CycleCtorEvents *source, float stepPeriod, int keepRemainder, float clipStart)
    : CycleCtorFrame(position, scale, direction, up, names, cycle, messages)
{
    carry = keepRemainder;
    timer = stepPeriod;
    period = stepPeriod;
    current = 0;
    step = 1;
    mode = 1;
    startTime = clipStart;
    for (int i = 0; i < source->table.keys.count; ++i)
        events.Insert(source->table.keys.Get(i), source->ValueAt(i), -1);
}
