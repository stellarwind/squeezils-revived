// StatusBar: the status strip. Only the ctor is ported here.
#pragma once
#include "compat41.h"

struct SB_NutCounter { char pad[0x2c]; SB_NutCounter(); ~SB_NutCounter(); };
struct SB_GoodieList { char pad[0x30]; SB_GoodieList(); ~SB_GoodieList(); };
struct SB_MoodFace   { char pad[0x2c8]; SB_MoodFace(); ~SB_MoodFace(); };

struct StatusBarCtor {
    SB_NutCounter amo;           // 0x000
    int  amoCount;               // 0x02c
    int  amoShown;               // 0x030
    SB_NutCounter food;          // 0x034
    int  foodCount;              // 0x060
    int  foodGoal;               // 0x064
    int  foodShown;              // 0x068
    SB_GoodieList goodies;       // 0x06c
    SB_MoodFace   face;          // 0x09c
    int  goodieStep;             // 0x364
    int  goodieSteps;            // 0x368
    int  goodieInterval;         // 0x36c
    unsigned goodieTick;         // 0x370
    int  goodieFlip;             // 0x374
    int  faceState;              // 0x378

    StatusBarCtor();             // 0x42ea70
};
