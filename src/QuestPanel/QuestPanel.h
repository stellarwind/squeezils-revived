// QuestPanel: the panel's quest items + places counter.
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
    void Draw(int place); // 0x430440
    void FreeCombined(); // 0x430340
    int Init(int x, int y, int place); // 0x430150
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
    int Init(const char *key, int x, int y, int mode); // 0x42f7d0
    int Load();                                     // 0x42f900
    void Draw(int gold, int diamond);                // 0x42f9f0
};

struct QuestPanel {
    QPQuestItems items;    // 0x00
    QPNutCounter places;   // 0x68

    QuestPanel();          // 0x42f580
    ~QuestPanel();         // 0x42f5f0
    int Load();            // 0x42f660
    int Draw(int force, int place); // 0x42f690
    int LoadPlaces();      // 0x42f6c0
    int DrawPlaces(int force); // 0x42f6d0
    int DrawItems(int force, int place); // 0x42f760
};
