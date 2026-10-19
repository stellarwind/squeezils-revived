// SqueezilSoul: spitting (0x4569c0).
// 18 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
void PadDecl13();
void PadDecl14();
void PadDecl15();
void PadDecl16();
void PadDecl17();
void PadDecl18();
#include "Soul.h"

// @004569c0 Standing still starts a spit (its strength comes later, SetSpitStrength); walking and attacking start a full one;
// on the move a nut flies at once. Returns the state to go to (24: none).
int SqueezilSoul::Spit()
{
    if (frozen) {
        spitRequested = 1;
        return 24;
    }
    int next;
    switch (GetState()) {
    case 0:
        spitStrength = -1.0f;
        next = 16;
        break;
    case 3: case 7:
        spitStrength = 1.0f;
        spitRequested = 1;
        next = 16;
        break;
    case 4: case 6: case 9: case 10: case 12: case 14: case 17:
        next = 24;
        {
        SoulBody *b = Body();
        PlayEffect("spitEffect", *b->GetMatrix()->Position());
        }
        SpitNut(physics->spitVelocity);
        break;
    default:
        spitRequested = 1;
        next = 24;
        break;
    }
    return next;
}
