// RegionSimulation trivial getters (0x418110, 0x418150, 0x418160, 0x4189a0).
#include "RegionSimulation/RegionSimulation.h"

int RegionSimulation::CanSave()     { return 1; }
int RegionSimulation::PlaceCount()  { return placeCount; }
int RegionSimulation::GetLocation() { return location; }
int RegionSimulation::GetRegion()   { return region; }
