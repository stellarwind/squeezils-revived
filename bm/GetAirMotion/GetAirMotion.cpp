// SqueezilSoul::GetAirMotion (0x456ca0).
#include "GetAirMotion/GetAirMotion.h"

void GGASoul::GetAirMotion(int forState, GGAControl *c)
{
    switch (forState) {
    case 11:
        c->flight = physics->tumble;
        break;
    case 12:
        c->jump = physics->jump;
        break;
    case 14:
        c->jump = physics->hop;
        break;
    case 15:
        c->jump = physics->skihop;
        break;
    case 17:
        c->flight = physics->fly;
        break;
    case 18:
        c->flight = physics->dive;
        break;
    case 19:
    case 20:
        break;
    }
}
