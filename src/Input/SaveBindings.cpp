// 0x43c4d0: reject conflicting keys/buttons before writing the input bindings.
// 8 spare declarations (register allocation, see padscan.sh).
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
#include <windows.h>
#include <stdio.h>
#include "compat41.h"

struct BindingKinds {
    virtual void slot0();
    virtual int Key(int command);
    int Button(int command);                    // 0x43db40
    void Name(int command, char *text);          // 0x43da00
    int Save();                                 // 0x43ecf0
};
struct InputCompatibility {
    void *pad0;
    BindingKinds *bindings;
    bool Compatible(int first, int second);      // 0x43c650
    int SaveBindings(HWND window);
};
extern const char kDuplicateKeys[], kDuplicateButtons[]; // 0x4bc960 / 0x4bc938
extern const char kBindingErrorTitle[];                  // 0x4b95a4
extern const char kBindingWriteWarning[], kBindingWriteTitle[]; // 0x4bc8c0 / 0x4bc918

int InputCompatibility::SaveBindings(HWND window)
{
    char secondName[40], firstName[40], message[100];
    int i, j;
    for (i = 0; i < 51; i++) {
        int key = bindings->Key(i);
        for (j = 0; j < 51; j++)
            if (j != i && key == bindings->Key(j) && Compatible(i, j))
                goto duplicateKeys;
    }
    for (i = 0; i < 51; i++) {
        int button = bindings->Button(i);
        for (j = 0; j < 51; j++) {
            if (j != i) {
                int other = bindings->Button(j);
                if (button && other == button && Compatible(i, j))
                    goto duplicateButtons;
            }
        }
    }
    if (!bindings->Save())
        MessageBoxA(window, kBindingWriteWarning, kBindingWriteTitle, MB_ICONINFORMATION);
    return 1;
duplicateKeys:
    bindings->Name(i, firstName);
    bindings->Name(j, secondName);
    sprintf(message, kDuplicateKeys, firstName, secondName);
    MessageBoxA(window, message, kBindingErrorTitle, 0);
    return 0;
duplicateButtons:
    bindings->Name(i, firstName);
    bindings->Name(j, secondName);
    sprintf(message, kDuplicateButtons, firstName, secondName);
    MessageBoxA(GetFocus(), message, kBindingErrorTitle, 0);
    return 0;
}
