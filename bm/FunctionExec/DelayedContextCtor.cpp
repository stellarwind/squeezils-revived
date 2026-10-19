// SQZ.EXE 0x422370: capture the script name, timing mode, and a copied argument record.
#include "String/String.h"

struct DelayedArguments {
    int fields[6];
    DelayedArguments(void *source);         // 0x420bf0: copy the six-word argument record
};

struct DelayedExecutionContext {
    String name;
    int frameBased;
    float timer;
    void *payload;
    int extra;
    DelayedExecutionContext(const char *text, int frames, float delay, void *args, int value);
};

DelayedExecutionContext::DelayedExecutionContext(const char *text, int frames, float delay,
                                               void *args, int value)
{
    name = String(text);
    frameBased = frames;
    timer = delay;
    payload = new DelayedArguments(args);
    extra = value;
}
