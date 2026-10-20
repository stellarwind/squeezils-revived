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
void PadDecl14();
void PadDecl15();
void PadDecl16();
void PadDecl17();
void PadDecl18();
// CycleProp::SetCurrent (0x428fd0, ret 4): switch the shown child: hide the old one, show the new one and move it to this prop's position.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
struct Part {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual int Attach(int parent);                  // slot 4 (+0x10)
    virtual void Hide();                             // slot 5 (+0x14)
    virtual void Place(int arg, Vector3f pos);       // slot 6 (+0x18)
};
struct Kids { virtual Part *Get(int i); };           // embedded at 0x54
struct CycleS {
    virtual void s0();
    char pad[0x10 - 4];
    Vector3f pos;                                    // 0x10
    char pad2[0x34 - 0x1c];
    int loaded;                                      // 0x34
    char pad3[0x54 - 0x38];
    Kids kids;                                       // 0x54
    char pad4[0xdc - 0x58];
    int current;                                     // 0xdc
    int Parent();                                    // 0x4276b0
    void SetCurrent(int child);
};
void CycleS::SetCurrent(int child)
{
    if (!loaded) {
        current = child;
        return;
    }
    if (current == child)
        return;
    kids.Get(current)->Hide();
    current = child;
    int a = kids.Get(child)->Attach(Parent());
    kids.Get(current)->Place(a, pos);
}
