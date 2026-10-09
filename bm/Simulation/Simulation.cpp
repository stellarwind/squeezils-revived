// Simulation/LocationSimulation creature lookups.
#include "Simulation/Simulation.h"

void *Simulation::GetCreatures() { return creatures; }

void *Simulation::GetActor(int id)
{
    int i = creatures->IndexOf(id);
    if (i == -1)
        return 0;
    return creatures->data[i]->actor;
}

void *Simulation::GetSoul(int id)
{
    int i = creatures->IndexOf(id);
    if (i == -1)
        return 0;
    return creatures->data[i]->soul;
}
