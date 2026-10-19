// CreatureTemplate: a StringList (PrimitiveArray<String,3>) of 20 stimulus function names followed by the
// sense radii and body colour (SQZ.EXE 0x4483d0/0x4484a0). StringList vtables: 0x49d50c (main), 0x49d4f4 (BufferObject).
#pragma once
#include "String/String.h"
#include "Container/ContainerSet.h"
struct PendingSoul;

struct CreatureTemplate {
    PrimitiveArray<String, 3> names;   // 0x00 StringList of stimulus function names
    float senseRadius[3];   // 0x14 SenseRadiusP/R/G
    int   bodyColor;        // 0x20

    CreatureTemplate();     // 0x4483d0
    ~CreatureTemplate();    // 0x4484a0
    void RunPending(PendingSoul *soul); // 0x4487f0
    int Load(String &section, class IniReader *ini);   // 0x448510
};
