// NavProp::Hide (virtual slot 5, SQZ.EXE 0x429c20): hide as a frame prop, then drop the nav instance from the place's tables.
#define NAVPROP_HIDE
#include "NavProp/NavProp.h"

struct FrameBase { void Hide(); };                       // 0x427320
struct NavTables { void Remove(int instance); };         // 0x4102f0
struct NavPlace { NavTables *GetSurfaces(); };           // 0x439b60
struct SimRoot { int dummy; };
SimRoot *GetSim();                                       // 0x420690 (cdecl)
void *DynamicCast(void *p, int zero, const void *from, const void *to, int ref);   // 0x48f060 (cdecl)
extern const char kSimType[];                            // 0x4ba1d0
extern const char kPlaceSimType[];                       // 0x4bae38

void NavProp::Hide()
{
    ((FrameBase *)this)->Hide();
    ((NavPlace *)DynamicCast(GetSim(), 0, kSimType, kPlaceSimType, 0))->GetSurfaces()->Remove(instance);
}
