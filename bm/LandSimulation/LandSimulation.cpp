// LandSimulation trivial getters.
#include "LandSimulation/LandSimulation.h"

int LandSimulation::CharacterCount() { return table->count; }
PossessionsTable *LandSimulation::GetTable() { return table; }
const char *LandSimulation::GetPlaceName(int index) { return names[index]; }
