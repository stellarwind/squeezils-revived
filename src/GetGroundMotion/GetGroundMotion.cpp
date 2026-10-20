// SqueezilSoul::GetGroundMotion (0x456b90).
#include "GetGroundMotion/GetGroundMotion.h"

void GGMSoul::GetGroundMotion(int forState, GGMControl *c)
{
    switch (forState) {
    case 0: case 1: case 2: case 3:
    walk_body:
        c->ground = physics->walk;
        break;
    case 4:
        c->ground = physics->run;
        break;
    case 5:
        c->ground = physics->skid;
        break;
    case 6:
    slip_body:
        c->lever = physics->slip;
        break;
    case 7:
        c->lever = physics->attack;
        break;
    case 8:
        goto slip_body;
    case 9: case 10:
        c->ground = physics->scamper;
        break;
    case 13:
        goto walk_body;
    case 16:
        goto walk_body;
    case 19:
        goto walk_body;
    case 21: case 22:
        goto walk_body;
    }
}
