// QuestItems: the collectable key pieces and the stone they form (inferred from the pictures and ini names). Each place has three key pieces (left, middle and
// right: the stone fragments lkey/mkey/rkey.bmp, drawn on a magenta colour key) and a combined picture (ckey1..7.bmp, the pieces
// joined into one frame with the place's emblem) that stands for the finished set. A sub object of the panel (at Panel + 0x394,
// 0x68 bytes). Reconstructed from SQZ.EXE 0x42fc30-0x430440; the picture names come from the place's region\region.ini section
// [UIResources] (CombinedKey with the place number added, LeftKey, MiddleKey, RightKey); the pictures are in the game's ui folder.
// The game's own sound for picking one up is keyEffect in SQZ.INI. Verified: ckey1, lkey, mkey and rkey load and render.
#pragma once
#include "Bitmap.h"

class QuestItems {
public:
    QuestItems();                                  // @42fc30
    ~QuestItems();                                 // @42fc40
    BOOL Init(int a, int b, int index);          // @430150 name and load the combined picture for place `index`, then the three key piece pictures; a and b are kept
    BOOL LoadCombined(int index);                // @430360 load the combined picture (the original always answers FALSE)
    BOOL LoadKeys();                             // @42fca0 load the left, middle and right key piece pictures, once per run

    Bitmap *combined;          // 0x00
    char    combinedFile[20];  // 0x04
    Bitmap *left;              // 0x18
    char    leftFile[20];      // 0x1c
    Bitmap *middle;            // 0x30
    char    middleFile[20];    // 0x34
    Bitmap *right;             // 0x48
    char    rightFile[20];     // 0x4c
    char    pad60[8];          // 0x60 the two values Init was given (kept by original code)
};
static_assert(sizeof(QuestItems) == 0x68, "QuestItems layout");
