// Keylist::Keylist (SQZ.EXE 0x43c9e0): builds the key-name table (name -> virtual-key code) on the
// stack, copies it to a heap array, names the bindings file "Commandlist" and loads the bindings.
#include <string.h>

// SIZE 0x14
struct KeyName {
    char name[10];   // 0x00
    int code;        // 0x0c
    int flag;        // 0x10
};

// SIZE 0xdac
class Keylist {
public:
    Keylist();
    virtual void V0();   // vtable+0x00
    void Init();         // 0x43dc40: clear the 0x33 bindings, then load them

    KeyName *entries;    // 0x04
    int count;           // 0x08
    char bindings[0xd8c];// 0x0c (0x33 entries of 0x44 bytes)
    int mode;            // 0xd98
    char file[16];       // 0xd9c
};

Keylist::Keylist()
{
    mode = 4;
    KeyName table[] = {
        { "ESC", 0x1b, 1 },
        { "F1", 0x70, 1 }, { "F2", 0x71, 1 }, { "F3", 0x72, 1 }, { "F4", 0x73, 1 },
        { "F5", 0x74, 1 }, { "F6", 0x75, 1 }, { "F7", 0x76, 1 }, { "F8", 0x77, 1 },
        { "F9", 0x78, 1 }, { "F11", 0x7a, 1 }, { "F12", 0x7b, 1 },
        { "1", 0x31, 1 }, { "2", 0x32, 1 }, { "3", 0x33, 1 }, { "4", 0x34, 1 }, { "5", 0x35, 1 },
        { "6", 0x36, 1 }, { "7", 0x37, 1 }, { "8", 0x38, 1 }, { "9", 0x39, 1 }, { "0", 0x30, 1 },
        { "A", 0x41, 1 }, { "B", 0x42, 1 }, { "C", 0x43, 1 }, { "D", 0x44, 1 }, { "E", 0x45, 1 },
        { "F", 0x46, 1 }, { "G", 0x47, 1 }, { "H", 0x48, 1 }, { "I", 0x49, 1 }, { "J", 0x4a, 1 },
        { "K", 0x4b, 1 }, { "L", 0x4c, 1 }, { "M", 0x4d, 1 }, { "N", 0x4e, 1 }, { "O", 0x4f, 1 },
        { "P", 0x50, 1 }, { "Q", 0x51, 1 }, { "R", 0x52, 1 }, { "S", 0x53, 1 }, { "T", 0x54, 1 },
        { "U", 0x55, 1 }, { "V", 0x56, 1 }, { "W", 0x57, 1 }, { "X", 0x58, 1 }, { "Y", 0x59, 1 },
        { "Z", 0x5a, 1 },
        { "LEFT", 0x25, 1 }, { "RIGHT", 0x27, 1 }, { "UP", 0x26, 1 }, { "DOWN", 0x28, 1 },
        { "ENTER", 0x0d, 1 }, { "SHIFT", 0x10, 1 }, { "CTRL", 0x11, 1 }, { "TAB", 0x09, 1 },
        { "SPACE", 0x20, 1 },
        { "JUMP+JUMP", 999, 0 },
        { "", 0, 0 }
    };

    for (count = 0; strlen(table[count].name) != 0; count++)
        ;
    entries = new KeyName[count];
    for (int i = 0; i < count; i++) {
        strcpy(entries[i].name, table[i].name);
        entries[i].code = table[i].code;
    }
    strcpy(file, "Commandlist");
    Init();
}
