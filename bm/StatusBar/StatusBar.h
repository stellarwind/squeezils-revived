// StatusBar: the game-panel status strip. Only the trivial food-goal setter is ported here.
#pragma once
#include "compat41.h"

struct StatusBar {
    char pad[0x64];
    int  foodGoal;   // 0x64

    void SetFoodGoal(int value);   // 0x42ef40
};
