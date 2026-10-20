// Release the delayed-message array, SQZ.EXE 0x422b80.
#include "Misc16/DelayedMessage.h"
struct DelayedMessageArray {
    void *vtable;
    int count, capacity;
    DelayedMessage **data;
    void Remove(int index); // 0x46c6c0, direct call
};
extern DelayedMessageArray gDelayedMessages; // 0x4c70c8

void ReleaseDelayedMessages()
{
    int index = gDelayedMessages.count;
    while (--index >= 0) {
        delete gDelayedMessages.data[index];
        gDelayedMessages.Remove(index);
    }
}
