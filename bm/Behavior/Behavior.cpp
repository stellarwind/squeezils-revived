// Behavior base ctor (0x4022f0).
#include "Behavior/Behavior.h"

Behavior::Behavior(void *body, void *soul)
{
    this->body = body;
    this->soul = soul;
    vtbl = (void **)0x49d0bc;
}
