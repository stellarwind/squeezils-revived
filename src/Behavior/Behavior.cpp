// Behavior base ctor (0x4022f0).
#include "Behavior/Behavior.h"

Behavior::Behavior(void *body, void *soul)
{
    this->body = body;
    this->soul = soul;
}

// Complete destructor at 0x403b50; the compiler installs Behavior's own vtable.
Behavior::~Behavior() {}
