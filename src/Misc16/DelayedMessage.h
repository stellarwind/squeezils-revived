// Delayed script-message layout and entry points.
#pragma once
#include "compat41.h"
#include "String/String.h"
struct DelayedMessage {
    String name;
    int fired;
    float timer;
    char *payload;
    int arg;
    ~DelayedMessage();  // 0x422450
    int Tick(float dt); // 0x4224b0
    int TickFrame();    // 0x422510
};
