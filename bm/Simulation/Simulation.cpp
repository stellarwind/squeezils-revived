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

int Simulation::FindIdBySoul(void *soul)
{
    for (int i = 0; i < creatures->keys.count; i++) {
        if (creatures->data[i]->soul == soul)
            return creatures->keys.Get(i);
    }
    return -1;
}

int Simulation::FindIdByActor(void *actor)
{
    for (int i = 0; i < creatures->keys.count; i++) {
        if (creatures->data[i]->actor == actor)
            return creatures->keys.Get(i);
    }
    return -1;
}
