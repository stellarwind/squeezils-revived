// Simulation::AddCreature (0x414160): creatures->Put(id, actor, soul).
#include "SimulationAddCreature/SimulationAddCreature.h"

void Simulation::AddCreature(int id, void *actor, void *soul)
{
    creatures->Put(id, actor, soul);
}
