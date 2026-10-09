// RegionSimulation getters and place-table leaves (0x418110-0x418160, 0x4189a0).
#include "RegionSimulation/RegionSimulation.h"

int RegionSimulation::CanSave()     { return 1; }
int RegionSimulation::PlaceCount()  { return placeCount; }
int RegionSimulation::HasPlace(int id)   { return places.Contains(id); }
void *RegionSimulation::GetPlace(int id) { return places.Get(id); }
int RegionSimulation::GetLocation() { return location; }
int RegionSimulation::GetRegion()   { return region; }
