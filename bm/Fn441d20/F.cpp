// spare declarations: the compiler's register choices follow the symbol count (tools/flagsearch/padscan.sh)
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
// 0x441d20: registers the script functions of this group (new X + RegisterFunction, in order).
#include "FunctionGoodie/Goodie.h"
#define FloatFunctionArgument FloatFunctionArgument_O
#define ParseFloatArgument ParseFloatArgument_O
#include "FunctionFlash/Flash.h"
#include "FunctionMessage/Message.h"
#include "FunctionMisc/Misc.h"
#include "FunctionMisc/MiscClasses.h"
#include "FunctionMisc/Noarg2.h"
#include "FunctionMovie/Movie.h"
#include "FunctionStr/Str.h"
#include "FunctionSurf/Surf.h"
#undef FloatFunctionArgument
#undef ParseFloatArgument
class KillSqueezilFunction : public Function {
public:
    KillSqueezilFunction() { field_4 = 0; }
    virtual int Execute(ScriptCall *args, int extra);
    virtual Function *Create(ArgList *list);
    virtual void Print();
    virtual const char *Name();
    int field_4;                                         // 0x04
};
void RegisterFunction(Function *f);                      // 0x421d00 (cdecl)

void RegisterFunctionsB()
{
    RegisterFunction(new MessageFunction);
    RegisterFunction(new TimerMessageFunction);
    RegisterFunction(new TimerTimeMessageFunction);
    RegisterFunction(new ClearMessageFunction);
    RegisterFunction(new ClearTimerMessageFunction);
    RegisterFunction(new SetSurfaceTypeFunction);
    RegisterFunction(new RemoveCurrentGoodieFunction);
    RegisterFunction(new CreateGoodieAtPositionFunction);
    RegisterFunction(new CreateGoodieAtPositionWithVelocityFunction);
    RegisterFunction(new KillSqueezilFunction);
    RegisterFunction(new GoToRegionFunction);
    RegisterFunction(new FlashGoodieFunction);
    RegisterFunction(new PlayMovieFunction);
    RegisterFunction(new FadeScreenDownFunction);
}
