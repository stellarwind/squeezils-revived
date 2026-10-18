// World::~World, SQZ.EXE 0x4358e0.
#include "World/World.h"
#include "World/WorldRuntime.h"
#include <new.h>

#include "World/WorldNavMesh.h"

World::~World()
{
    operator delete(gFrameTimer);
    delete navMesh;
}
