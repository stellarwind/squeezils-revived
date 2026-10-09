// NavProp trivial getters (0x429690, 0x4296a0).
#include "NavProp/NavProp.h"

int NavProp::GetSurface()  { return surface; }
int NavProp::GetInstance() { return instance; }

int NavProp::GetSizeSquared(float *out)
{
    *out = sizeSquared;
    return 1;
}
