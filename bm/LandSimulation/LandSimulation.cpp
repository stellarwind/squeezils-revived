// LandSimulation trivial getters.
#include "LandSimulation/LandSimulation.h"

int LandSimulation::CharacterCount() { return table->count; }
PossessionsTable *LandSimulation::GetTable() { return table; }
int LandSimulation::IsLentOut(int id) { return lentIds->Contains(id); }
void *LandSimulation::Lend(int id)
{
    lentIds->Insert(id, -1);
    return table->Get(id);
}

void LandSimulation::Return(int id, void *possessions)
{
    lentIds->RemoveKey(id);
    int i = table->IndexOf(id);
    table->Insert(id, possessions, i);
}
const char *LandSimulation::GetPlaceName(int index) { return names[index]; }
