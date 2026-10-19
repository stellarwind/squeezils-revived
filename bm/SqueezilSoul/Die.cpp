// SqueezilSoul: dying (0x450a00).
// 3 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
#include "Soul.h"

// @00450a00 how: 0 at once (no clip), 1 faint, 2 drown, 3 shudder, 9 keep the current clip; 4-8 only start dying.
void SqueezilSoul::Die(int how)
{
    int current, dying, mode;
    current = GetState();
    if (current == 22)
        return;
    if (how != 0 && (current == 21 || current == 20))
        return;
    if (kLoopingSoundPlaying)
        StopLoopingSounds();
    GetPossessions()->GetInventory()->RemoveNuts();
    stateAtDeath = GetState();
    dying = Body()->IsOnGround() ? 21 : 20;
    switch (how) {
    case 0:
        finished = 1;
        if (GetState() != dying && GetState() != 22)
            SetState(dying, 0);
        SetState(22, 0);
        Body()->SetMode(49, 0, 0);
        break;
    case 1:
        SetState(dying, 0);
        Body()->SetMode(46, 0, 0);
        break;
    case 2:
        SetState(dying, 0);
        Body()->SetMode(24, 0, 0);
        break;
    case 3:
        SetState(dying, 0);
        Body()->SetMode(23, 0, 0);
        break;
    case 9: {
        mode = Body()->GetMode();
        SetState(dying, 0);
        Body()->SetMode(mode, 0, 0);
        break;
    }
    }
}
