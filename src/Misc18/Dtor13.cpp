// SQZ.EXE 0x405f90: destructor of a polymorphic object holding up to ten 0x24-byte entries (starting at +0x18) with two owned objects each (count at +4).
struct Obj { ~Obj(); };                      // 0x473890
struct Entry { Obj *a, *b; char pad[0x1c]; };
struct C13 {
    virtual ~C13();
    int count;                  // +4
    char pad8[0x18 - 8];
    Entry e[10];                // +0x18
};
C13::~C13()
{
    for (int i = 0; i < count; i++) {
        Obj *p = e[i].a;
        if (p && i < 10) {
            delete p;
            delete e[i].b;
        }
    }
}
