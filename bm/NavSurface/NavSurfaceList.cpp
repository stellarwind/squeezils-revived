// NavSurfaceList getters (SQZ.EXE 0x412c10 / 0x412bf0).
#include "NavSurface/NavSurfaceList.h"

LoadedSurface *NavSurfaceList::Loaded(int number)
{
    return loaded.Get(number);
}

InstancedSurface *NavSurfaceList::FaceOf(const Contact *contact)
{
    return instances.Get(contact->face);
}
