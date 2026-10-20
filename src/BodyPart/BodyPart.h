// BodyPart: one part of a character's body (SQZ.EXE 0x41d870-0x41d900), VC++ 4.1 dialect.
// Layout from Repo/legacy_gcc/BodyPart.h.
#pragma once
#include "compat41.h"

struct BodyPart {
    void *vtbl;              // 0x00 vtable 0x49d7c4
    void *model;             // 0x04 SqgModel*
    void *morph;             // 0x08 PZMorphMesh*
    void *frame;             // 0x0c IDirect3DRMFrame*

    BodyPart *Init(void *model);              // 0x41d870
    BodyPart *Init();                         // 0x41d8a0
    BodyPart *Init(const BodyPart *like);     // 0x41d8c0
    void *GetFrame();                         // 0x41d890
};
