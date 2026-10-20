#pragma once
#include "String/String.h"

class SurfaceTypeDescription {
public:
    int walkable, bounce, slippery, flyThrough;
    float rebudGoodies;
    String activate, deactivate; // +0x14, +0x18
    SurfaceTypeDescription(int walk, int rebound, int slip, int fly, float rebud,
                           const char *activateName, const char *deactivateName) {
        walkable=walk; bounce=rebound; slippery=slip; flyThrough=fly;
        rebudGoodies=rebud;
        activate = String(activateName);
        deactivate = String(deactivateName);
    }
    String Deactivate(); // 0x412b50
    String Activate(); // 0x40fab0
};
