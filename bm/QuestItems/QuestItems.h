// QuestItems: the panel's key-piece pictures. Only the ctor (0x42fc30) is ported.
#pragma once
#include "compat41.h"

struct QuestItems {
    void *combined;          // 0x00
    char  combinedFile[20];  // 0x04
    void *left;              // 0x18
    char  leftFile[20];      // 0x1c
    void *middle;            // 0x30
    char  middleFile[20];    // 0x34
    void *right;             // 0x48
    char  rightFile[20];     // 0x4c
    int   x;                 // 0x60
    int   y;                 // 0x64

    QuestItems();            // 0x42fc30
};
