// CompassRuler/Panel leaf wrappers (SQZ.EXE 0x4153e0, 0x42e2a0).
#include "CompassRuler/CompassRuler.h"

int CompassRuler::UpdateSprites()
{
    UpdateSpritesReal();
    return 1;
}

int Panel::UpdateRegionSprites()
{
    return compass.UpdateSprites();
}
