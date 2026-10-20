// SQZ.EXE 0x425e50: initialize the common placement and hidden-frame state.
#define PROP_CONSTRUCTOR
#include "Prop/Prop.h"

extern void *kPropVtable[]; // 0x49db4c

Prop::Prop(Vector3f position_, Vector3f scale_, Vector3f direction_, Vector3f up_)
    : position(position_), scale(scale_), direction(direction_), up(up_)
{
    vtbl = kPropVtable;
    frame = 0;
    owner = 0;
    isInstance = 0;
}
