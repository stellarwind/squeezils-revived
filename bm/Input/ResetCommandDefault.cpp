// SQZ.EXE 0x43ebc0: restore one command's key from the CommandDefaults INI section.
#include "IniReader/IniReader.h"

struct DefaultCommand {
    int key;
    int field04;
    int kind;                              // +0x8 (bindings +0x20)
    char name[0x28];                        // +0xc (bindings +0x24)
    int editable;
    char rest[0xc];                        // complete command stride: 0x44
};

struct DefaultCommandBindings {
    virtual int ParseKey(const char *text); // slot 0
    char pad04[0x18 - 4];
    DefaultCommand commands[51];            // +0x18
    int ResetCommandDefault(int index);
};

int DefaultCommandBindings::ResetCommandDefault(int index)
{
    IniReader reader;
    reader.SetSection("CommandDefaults");
    char *name = commands[index].name;
    reader.GetString(name);
    if (commands[index].kind != 2)
        commands[index].key = ParseKey(reader.GetString(name));
    return commands[index].key;
}
