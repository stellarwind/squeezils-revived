// Declaration padding preserves VC4 code after sharing script dispatch.
void DispatchPad000();
void DispatchPad001();
void DispatchPad002();
void DispatchPad003();
void DispatchPad004();
void DispatchPad005();
void DispatchPad006();
void DispatchPad007();
void DispatchPad008();
void DispatchPad009();
void DispatchPad010();
void DispatchPad011();
void DispatchPad012();
void DispatchPad013();
void DispatchPad014();
void DispatchPad015();
void DispatchPad016();
void DispatchPad017();
void DispatchPad018();
void DispatchPad019();
void DispatchPad020();
void DispatchPad021();
void DispatchPad022();
void DispatchPad023();
void DispatchPad024();
void DispatchPad025();
#include "Runtime/DynamicCast.h"
// SQZ.EXE 0x427b50: dispatch clip messages crossed in either time direction.
#include "FrameProp/FrameProp.h"
#include "String/String.h"

struct FrameMessageTimes {
    virtual float Get(int index); // slot 0
    int count;
};
struct FrameMessageTable {
    int reserved;
    FrameMessageTimes keys; // +4; count at +8
    String ValueAt(int index); // 0x427ae0
};
struct FrameMessageSimulation { void *FindSoul(int actor); }; // 0x414080, ret 4
FrameMessageSimulation *GetSimulation(); // 0x420690
extern const char kFrameMessageSoulType[], kFrameMessageSqueezilSoulType[]; // 0x4b93e8, 0x4b9368
struct FrameMessageCaller {
    int storage[6];
    FrameMessageCaller(int actor, FrameProp *owner); // 0x4207a0
    FrameMessageCaller(void *soul, int actor, FrameProp *owner); // 0x420900
};
#include "Script/CallFunction.h"

void FrameProp::FireTimeMessages(float from, float to)
{
    FrameMessageTable *messages = (FrameMessageTable *)((char *)this + 0x64);
    if (!(to < from)) {
        for (int i = 0; i < messages->keys.count; ++i) {
            if (messages->keys.Get(i) > from && messages->keys.Get(i) <= to) {
                if (actorIndex != -1) {
                    void *soul = SqzRTDynamicCast(GetSimulation()->FindSoul(actorIndex), 0,
                        kFrameMessageSoulType, kFrameMessageSqueezilSoulType, 0);
                    FrameMessageCaller caller(soul, actorIndex, this);
                    CallFunction(messages->ValueAt(i).c_str(), &caller, 0, 0);
                } else {
                    FrameMessageCaller caller(-1, this);
                    CallFunction(messages->ValueAt(i).c_str(), &caller, 0, 0);
                }
            }
        }
    } else {
        for (int i = messages->keys.count - 1; i >= 0; --i) {
            if (messages->keys.Get(i) < from && messages->keys.Get(i) >= to) {
                if (actorIndex != -1) {
                    void *soul = SqzRTDynamicCast(GetSimulation()->FindSoul(actorIndex), 0,
                        kFrameMessageSoulType, kFrameMessageSqueezilSoulType, 0);
                    FrameMessageCaller caller(soul, actorIndex, this);
                    CallFunction(messages->ValueAt(i).c_str(), &caller, 0, 0);
                } else {
                    FrameMessageCaller caller(-1, this);
                    CallFunction(messages->ValueAt(i).c_str(), &caller, 0, 0);
                }
            }
        }
    }
}
