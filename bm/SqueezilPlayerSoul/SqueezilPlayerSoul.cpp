// SqueezilPlayerSoul::GetKind (0x41fb70): 6 once the soul is finished, else 0.
#include "SqueezilPlayerSoul/SqueezilPlayerSoul.h"

int SqueezilPlayerSoul::GetKind()
{
    return finished ? 6 : 0;
}
