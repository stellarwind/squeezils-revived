// Prop::IsShown (0x426160): whether the frame is built (the prop is visible).
#include "Prop/Prop.h"

int Prop::IsShown()
{
    return frame != 0;
}

void *Prop::GetOwner()   { return owner; }
void Prop::UpdateNoop(float seconds) {}
void Prop::Slot8Noop()   {}
int  Prop::Slot11Return0(int a) { return 0; }
int  Prop::Slot12Return0()      { return 0; }
