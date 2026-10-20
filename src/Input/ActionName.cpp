// SQZ.EXE 0x43c7b0: format an action name, its key, and optional button number.
#include "compat41.h"
#include <string.h>
#include <stdlib.h>

extern char kEmptyString[]; // 0x4b9488
extern const char kBindingSpace[]; // 0x4b95e4, " "
struct ActionNameBindings {
    virtual void slot0();
    virtual int CurrentKey(int index); // slot 1
    int Type(int index); // 0x43dbe0
    int GetName(int index, char *out); // 0x43da00
    void NameOf(int key, char *out); // 0x43db60
    int Button(int index); // 0x43db40
};
struct SqzInputManager {
    virtual int AssignKey(char *line, const char *keyName);
    virtual int SetKeySlot(char *line, int action);
    ActionNameBindings *bindings; // +4
    int ActionName(int index, char *line); // 0x43c7b0
};

int SqzInputManager::ActionName(int index, char *line)
{
    char keyName[40];
    strcpy(line, kEmptyString);
    if (bindings->Type(index) == 2)
        return strlen(line);
    int length = bindings->GetName(index, line);
    if (length > 0) {
        for (int i = 0; i < 28 - length; ++i)
            strcat(line, kBindingSpace);
        int key = bindings->CurrentKey(index);
        bindings->NameOf(key, keyName);
        strcat(line, keyName);
        int button = bindings->Button(index);
        if (button > 0) {
            int padding = 38 - strlen(line);
            for (int i = 0; i < padding; ++i)
                strcat(line, kBindingSpace);
            _itoa(button, line + strlen(line), 10);
        }
        return strlen(line);
    }
    return length;
}
