// PlaceSimulation trivial getters (0x439b50, 0x439b60, 0x439f70, 0x439f80).
#include "PlaceSimulation/PlaceSimulation.h"

void *PlaceSimulation::GetLevel()      { return level; }
void *PlaceSimulation::GetSurfaces()   { return surfaces; }
void *PlaceSimulation::StartPlacement(){ return startPlacement; }
int   PlaceSimulation::CanSave()       { return 0; }
