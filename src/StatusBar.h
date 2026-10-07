// StatusBar: the strip of the in-game panel with the two nut counters, the goodie list, the mood face and the dead / won
// messages. A sub object of the panel (at Panel + 8, 0x38c bytes). Reconstructed from SQZ.EXE 0x42ec10 (Init) and 0x42f2c0
// (LoadAll); its constructor (0x42ea70), destructor (0x42eb40) and per-frame drawing (0x42ed70 ...) are still original code.
// Positions are screen pixels inside the panel picture.
#pragma once
#include "NutCounter.h"
#include "MoodFace.h"

struct StatusBar {
    NutCounter amo;            // 0x000 [Interface] AmoNut: one number (the nuts the player can spit)
    int        amoA;           // 0x02c (0)
    int        amoB;           // 0x030 (-1)
    NutCounter food;           // 0x034 [Interface] FoodNut: two numbers
    int        foodA;          // 0x060 (0)
    int        foodB;          // 0x064
    int        foodC;          // 0x068 (-1)
    char       goodies[0x30];  // 0x06c the goodie list widget (original code)
    MoodFace   face;           // 0x09c
    char       pad364[0x18];   // 0x364
    Bitmap    *deadMessage;    // 0x37c [Interface] DeadMessage picture, shown when the player dies
    Bitmap    *wonMessage;     // 0x380 [Interface] WonMessage
    int        messageX;       // 0x384 where the messages go (405)
    int        messageY;       // 0x388 (12)

    StatusBar();               // @42ea70 builds the counters, goodie list and face
    ~StatusBar();              // @42eb40 frees the dead and won pictures, then the widgets

    BOOL Init();               // @42ec10 place and set up the widgets, load all their pictures
    BOOL LoadAll();            // @42f2c0 load the counters', goodie list's, dead and won pictures
};
static_assert(sizeof(StatusBar) == 0x38c, "StatusBar layout");
