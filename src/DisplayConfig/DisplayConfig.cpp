// DisplayConfig helpers (SQZ.EXE 0x459130 / 0x4593c0).
#include "DisplayConfig/DisplayConfig.h"

void DisplayConfig::ToInt(int *dst)
{
    dst[0] = w;
    dst[1] = h;
}

int DisplayConfig::GetField1c()
{
    return field1c;
}
