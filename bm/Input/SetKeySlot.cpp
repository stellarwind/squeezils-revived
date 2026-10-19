// SQZ.EXE 0x43ad80: select a key slot for a named action and refresh its label.
// The real function is 112 bytes; fns_all merges it with the next AssignKey entry.
#include "compat41.h"

struct KeySlotBinding {
    int editable;
    char rest[0x40];
};

struct KeySlotBindings {
    virtual void slot0();
    virtual int CurrentKey(int index);      // +0x4
    virtual void slot2();
    virtual int Find(const char *line);     // +0xc
    char pad04[0x4c - 4];
    KeySlotBinding entries[51];             // stride 0x44; editable at +0x4c
    int SelectKey(int index, int current, int action); // 0x43d680
    int SetKey(int index, int key);         // 0x43d640
};

struct SqzInputManager {
    virtual int AssignKey(char *line, const char *keyName);
    virtual int SetKeySlot(char *line, int action);
    KeySlotBindings *bindings;              // +0x4
    int ActionName(int index, char *line);  // 0x43c7b0
};

int SqzInputManager::SetKeySlot(char *line, int action)
{
    int index = bindings->Find(line);
    int current = bindings->CurrentKey(index);
    int result = 0;
    if (index != -1) {
        if (!bindings->entries[index].editable)
            return 0;
        int key = bindings->SelectKey(index, current, action);
        if (key)
            result = bindings->SetKey(index, key);
    }
    if (result)
        ActionName(index, line);
    return result;
}
