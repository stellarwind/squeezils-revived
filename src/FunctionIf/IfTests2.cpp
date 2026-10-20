// 19 spare declarations (symbol count, see padscan.sh)
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
void PadDecl19();
// Test slots of IfHasAllKeys / IfCompletedPlace (SQZ.EXE 0x443850, 0x4438b0).
#include "FunctionIf/IfClasses.h"

struct KCall { void *Actor(); };                         // 0x420c20
struct Sim { int dummy; };
Sim *GetSim();                                           // 0x420690 (cdecl)
void *DynamicCast(void *p, int zero, const void *from, const void *to, int ref);   // 0x48f060 (cdecl)
extern const char kSimType[];                            // 0x4ba1d0
extern const char kPlaceType[];                          // 0x4ba370
struct Place { int Region(); int Piece(); };             // 0x418150 / 0x418160
struct Inv { int KeysFound(int region); };               // 0x416580
struct InvOwner { Inv *Inventory(); };                   // 0x448990
struct KActor {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16(); virtual void s17();
    virtual InvOwner *Soul();                            // slot 18 (0x48)
};
struct Progress { int Completed(int region, int piece); };   // 0x41e9b0
Progress *GetProgress();                                 // 0x420680 (cdecl)

int IfHasAllKeysFunction::Test(ScriptCall *args, int extra)
{
    Place *pl = (Place *)DynamicCast(GetSim(), 0, kSimType, kPlaceType, 0);
    if (!pl)
        return 0;
    return ((KActor *)((KCall *)args)->Actor())->Soul()->Inventory()->KeysFound(pl->Piece());
}

