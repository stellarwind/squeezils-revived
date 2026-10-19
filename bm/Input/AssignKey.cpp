// SQZ.EXE 0x43adf0: assign the first character of a key name to a named action.
#include "compat41.h"

extern "C" __declspec(dllimport) short __stdcall VkKeyScanA(char ch);

struct KeyBinding {
    int editable;
    char rest[0x40];                        // complete binding stride: 0x44
};

struct KeyBindings {
    virtual void slot0();
    virtual void Select(int index);         // +0x4
    virtual void slot2();
    virtual int Find(const char *line);     // +0xc
    char pad04[0x4c - 4];
    KeyBinding entries[51];                 // editable flag at +0x4c + index * 0x44
    int SetKey(int index, int key);         // 0x43d640
};

struct SqzInputManager {
    virtual int AssignKey(char *line, const char *keyName);
    KeyBindings *bindings;                 // +0x4
    int ActionName(int index, char *line);  // 0x43c7b0
};

int SqzInputManager::AssignKey(char *line, const char *keyName)
{
    int index = bindings->Find(line);

    bindings->Select(index);
    char ch = *keyName;
    int result = 0;
    if (index != -1) {
        if (!bindings->entries[index].editable)
            return 0;
        unsigned char key = (unsigned char)VkKeyScanA(ch);
        result = bindings->SetKey(index, key);
    }
    if (result)
        ActionName(index, line);
    return result;
}
