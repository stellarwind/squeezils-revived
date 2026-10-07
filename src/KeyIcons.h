// KeyIcons: the pictures of the control keys the panel shows: one combined picture (numbered, chosen with the place it is loaded
// for) and the left, middle and right key pictures. A sub object of the panel (at Panel + 0x394, 0x68 bytes). Reconstructed
// from SQZ.EXE 0x42fc30-0x430440. The picture names come from the place's \region\region.ini, section [UIResources]: CombinedKey
// (a number and ".bmp" are added), LeftKey, MiddleKey and RightKey; the pictures are under the game's ui folder.
#pragma once
#include "Bitmap.h"

class KeyIcons {
public:
    KeyIcons();                                  // @42fc30
    ~KeyIcons();                                 // @42fc40
    BOOL Init(int a, int b, int index);          // @430150 name and load the combined picture for place `index`, then the three key pictures; a and b are kept
    BOOL LoadCombined(int index);                // @430360 load the combined picture (the original always answers FALSE)
    BOOL LoadKeys();                             // @42fca0 load the left, middle and right pictures, once per run

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
static_assert(sizeof(KeyIcons) == 0x68, "KeyIcons layout");
