// Panel trivial setters (SQZ.EXE 0x42db00, 0x42ea60, 0x42dd00, 0x42dcd0).
#pragma once
#include "compat41.h"
#include "StatusBar/StatusBar.h"
#include "QuestPanel/QuestPanel.h"
class Bitmap;

struct Panel {
    int originX, originY;
    StatusBar status; // 0x008
    char pad074[0x394 - 0x74];
    QuestPanel quest; // 0x394
    void *digits[10]; // 0x428
    int mode, lastMode, questKey; // 0x450
    int  dirty;              // 0x45c
    Bitmap *background;      // 0x460
    char backgroundFile[20]; // 0x464
    int backgroundX, backgroundY; // 0x478
    char pad480[0x614 - 0x480];
    int  event;              // 0x614
    char pad618[0x630 - 0x618];
    char message[100];       // 0x630
    char note[20];           // 0x694

    void Invalidate();       // 0x42db00
    void ClearEvent();       // 0x42ea60
    void ClearMessage();     // 0x42dd00
    void ClearNote();        // 0x42dcd0
    int LoadBitmaps();       // 0x42e500
    int Load();              // 0x42d9f0
    void ShowStatus(int foodGoal); // 0x42dac0
    void ShowQuest(int key);       // 0x42da60
    int DrawBackground();         // 0x42dc50
    void StartGoodieAnimation(int steps, float seconds); // 0x42e6f0
};
