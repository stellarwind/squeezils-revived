// SqueezilSoul::IsAttacking (0x4509d0).
#include "IsAttacking/IsAttacking.h"

int IsAttackingSoul::IsAttacking()
{
    if (state == 7 || (state == 17 && GetBody()->GetMode() == 22))
        return 1;
    return 0;
}
