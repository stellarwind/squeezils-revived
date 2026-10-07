// StatusBar: the strip of the in-game panel with the two nut counters, the goodie list, the mood face and the dead / won
// messages. A sub object of the panel (at Panel + 8, 0x38c bytes). Reconstructed from SQZ.EXE 0x42ec10 (Init) and 0x42f2c0
// (LoadAll); its constructor (0x42ea70), destructor (0x42eb40) and per-frame drawing (0x42ed70 ...) are still original code.
// Positions are screen pixels inside the panel picture.
#pragma once
#include "NutCounter.h"
#include "MoodFace.h"
#include "GoodieList.h"

struct StatusBar {
    NutCounter amo;            // 0x000 [Interface] AmoNut: one number (the nuts the player can spit)
    int        amoA;           // 0x02c (0)
    int        amoB;           // 0x030 (-1)
    NutCounter food;           // 0x034 [Interface] FoodNut: two numbers
    int        foodA;          // 0x060 (0)
    int        foodB;          // 0x064
    int        foodC;          // 0x068 (-1)
    GoodieList goodies;        // 0x06c the row of goodie pictures
    MoodFace   face;           // 0x09c
    int        goodieStep;     // 0x364 step of the goodie list's animation (0 after a reset)
    int        goodieSteps;    // 0x368 steps in one round
    int        goodieInterval; // 0x36c milliseconds between steps
    unsigned   goodieTick;     // 0x370 timeGetTime of the last step
    int        goodieFlip;     // 0x374 flips between 0 and 1 on each step
    int        faceState;      // 0x378 what the face reported last frame: 0 awake, 1 dead (shows the dead message), else won
    Bitmap    *deadMessage;    // 0x37c [Interface] DeadMessage picture, shown when the player dies
    Bitmap    *wonMessage;     // 0x380 [Interface] WonMessage
    int        messageX;       // 0x384 where the messages go (405)
    int        messageY;       // 0x388 (12)

    StatusBar();               // @42ea70 builds the counters, goodie list and face
    ~StatusBar();              // @42eb40 frees the dead and won pictures, then the widgets

    BOOL Init();               // @42ec10 place and set up the widgets, load all their pictures
    BOOL LoadAll();            // @42f2c0 load the counters', goodie list's, dead and won pictures

    BOOL Update(int force, int *event, Bitmap *panel);   // @42ed70 once a frame: face, goodies, nut counts, then the dead / won message
    int  UpdateFace(int force, Bitmap *panel);           // @42ee90 draw the face, remember its state
    BOOL UpdateAmo(int force);                           // @42eec0 redraw the spit nut count when it changed (or force); 1 when it changed
    void SetFoodB(int value);                            // @42ef40 the second number of the food counter
    BOOL UpdateFood(int force);                          // @42ef50 same for the food nuts; also stores PlayerFoodNuts in the settings
};
static_assert(sizeof(StatusBar) == 0x38c, "StatusBar layout");
