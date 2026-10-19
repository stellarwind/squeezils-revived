// SQZ.EXE 0x422e00: queue a script invocation after a time delay.
#include "String/String.h"

struct DelayedExecutionContext {
    String name;
    int frameBased;
    float timer;
    void *payload;
    int extra;
    DelayedExecutionContext(const char *name, int frameBased, float delay,
                            void *args, int extra); // 0x422370
};

struct DelayedContextQueue {
    void *vtable;
    int count, capacity;
    DelayedExecutionContext **data;
    int Insert(DelayedExecutionContext *const &context, int at); // 0x46c650
};
extern DelayedContextQueue gDelayedMessages; // 0x4c70c8

void ScheduleDelayed(const char *name, float delay, void *args, int extra)
{
    DelayedExecutionContext *context = new DelayedExecutionContext(name, 0, delay, args, extra);
    gDelayedMessages.Insert(context, -1);
}
