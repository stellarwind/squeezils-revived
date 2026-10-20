// SQZ.EXE 0x422f70 / 0x423050: advance delayed messages and remove completed ones.
#include "DelayedMessage.h"

struct DelayedMessageArray {
    void *vtable;
    int count, capacity;
    DelayedMessage **data;
    int Count() const { return count; }
    DelayedMessage *At(int index) { return data[index]; }
    void Remove(int index);                             // 0x46c6c0
};
extern DelayedMessageArray gDelayedMessages;             // 0x4c70c8

struct TimerRemovalIndices {
    void *vtable;
    int count, capacity;
    int *data;
    void *bufferVtable;
    int Count() const { return count; }
    TimerRemovalIndices();                             // 0x45eef0
    ~TimerRemovalIndices();                            // 0x45efa0
    int Insert(const int &index, int hint);              // 0x45f000
};

void ScriptUpdateTimers(float seconds)
{
    int index;
    TimerRemovalIndices removed;
    for (index = 0; ; ++index) {
        if (index >= gDelayedMessages.Count())
            break;
        if (gDelayedMessages.At(index)->Tick(seconds)) {
            delete gDelayedMessages.At(index);
            removed.Insert(index, -1);
        }
    }
    for (index = removed.Count() - 1; ; --index) {
        if (index < 0)
            break;
        gDelayedMessages.Remove(removed.data[index]);
    }
}

void ScriptResetTimers()
{
    int index;
    TimerRemovalIndices removed;
    for (index = 0; ; ++index) {
        if (index >= gDelayedMessages.Count())
            break;
        if (gDelayedMessages.At(index)->TickFrame()) {
            delete gDelayedMessages.At(index);
            removed.Insert(index, -1);
        }
    }
    for (index = removed.Count() - 1; ; --index) {
        if (index < 0)
            break;
        gDelayedMessages.Remove(removed.data[index]);
    }
}
