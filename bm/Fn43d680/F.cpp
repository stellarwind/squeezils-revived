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
void PadDecl19();
void PadDecl20();
void PadDecl21();
// 0x43d680 (ret 0xc): next/previous id in a ring of 20-byte records; modes 2 and 3 are forwarded to two helpers.
struct Rec { int a, b, c; int id; int e; };
struct Ring {
    int pad0;
    Rec *recs;                                       // 0x04
    int count;                                       // 0x08
    int Mode2(int x);                                // 0x43eb30
    int Mode3(int x);                                // 0x43ebc0
    int Step(int x, int id, int mode);
};
int Ring::Step(int x, int id, int mode)
{
    if (mode == 2)
        return Mode2(x);
    if (mode == 3)
        return Mode3(x);
    int step = mode == 1 ? -1 : 1;
    for (int i = 0; i < count; i++) {
        if (recs[i].id == id) {
            int n = i + step;
            if (n >= count)
                n = 0;
            if (n < 0)
                n = count - 1;
            if (recs[n].id != 999)
                return recs[n].id;
            n += step;
            if (n >= count)
                n = 0;
            if (n < 0)
                n = count - 1;
            return recs[n].id;
        }
    }
    return recs[0].id;
}
