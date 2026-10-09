// Prop::IsShown (0x426160): whether the frame is built (the prop is visible).
#include "Prop/Prop.h"

int Prop::IsShown()
{
    return frame != 0;
}
