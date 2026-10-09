// GoodieSoul: a goodie's soul. The trivial getters and GrantMessageMap GetInterface are ported.
#pragma once
#include "compat41.h"

struct GoodieSoul {
    void **vtbl;        // 0x00
    void **grantVtbl;   // 0x04
    void *body;         // 0x08
    void *name;         // 0x0c (String)
    int  kind;          // 0x10
    int  thrown;        // 0x14

    void *GetInterface(int which);   // 0x459960 slot 1
    int  IsLoose();                  // 0x459980 slot 3 (kind == 0)
    int  GetKind();                  // 0x459bf0 slot 8 (kind==0 ? 2 : 4)
    int  IsAttacking();              // 0x45a830 slot 11 (thrown != 0)
    void *GetSettings();             // 0x45a840 slot 10 (global 0x4c7bb4)
};
