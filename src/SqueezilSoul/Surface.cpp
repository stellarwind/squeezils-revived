// SqueezilSoul: the surface type (0x451500).
#include "Soul.h"

// @00451500 No surface type: tell the place's surfaces about the change.
void SqueezilSoul::ResetSurface()
{
    SurfaceInfo old = surface;
    surface.type = 0;
    surface.typeName = String(" none");
    surface.value = String("none");
    place->Surfaces()->TypeChanged(this, Body()->GetContact(), &old, &surface);
}
