// QuestPanel: the panel's quest items + places counter. Only the ctor is ported here.
#pragma once
#include "compat41.h"

struct QPQuestItems {
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

    QPQuestItems();
    ~QPQuestItems();
};

struct QPNutCounter {
    void *icon;      // 0x00
    void *original;  // 0x04
    int   x;         // 0x08
    int   y;         // 0x0c
    char  file[20];  // 0x10
    int   lastValue; // 0x24
    int   mode;      // 0x28

    QPNutCounter();
    ~QPNutCounter();
};

struct QuestPanel {
    QPQuestItems items;    // 0x00
    QPNutCounter places;   // 0x68

    QuestPanel();          // 0x42f580
};
