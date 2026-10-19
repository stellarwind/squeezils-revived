// SQZ.EXE 0x448640: replace a stimulus function; the name "0" clears it.
#include <string.h>
#include "String/String.h"

extern const char kNoFunction[];            // 0x4bca44: "0"

struct CreatureTemplate {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void Replace(int index, const String &name); // array slot 3, +0xc
    void SetFunction(int stimulus, String name);
};

void CreatureTemplate::SetFunction(int stimulus, String name)
{
    if (strcmp(name.c_str(), kNoFunction) == 0)
        *(char *)name.c_str() = 0;
    Replace(stimulus, name);
}
