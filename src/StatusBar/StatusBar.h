// StatusBar: the game-panel status strip. Only the trivial food-goal setter is ported here.
#pragma once
#include "compat41.h"

struct StatusNutCounter {
    char storage[0x2c];
    void Draw(int value, int second); // 0x42f9f0
};
struct StatusBar {
    StatusNutCounter amo;
    int amoCount, amoShown;
    StatusNutCounter food; // 0x34
    int foodCount; // 0x60
    int  foodGoal;   // 0x64
    int foodShown; // 0x68

    void SetFoodGoal(int value);   // 0x42ef40
    int Init();                   // 0x42ec10
    void SetRound(int steps, float seconds); // 0x42f200
    int UpdateAmo(int force); // 0x42eec0
    int UpdateFood(int force); // 0x42ef50
};
