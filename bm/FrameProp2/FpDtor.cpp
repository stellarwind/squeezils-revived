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
// FrameProp::~FrameProp (SQZ.EXE 0x427100): deletes the children, then the body, then the members and the Prop base.
struct Child { virtual ~Child(); };
struct ChildList {
    virtual Child *Get(int i);
    int count;
    int cap, data;
    ~ChildList();
};
struct Names { int rep; ~Names(); };
struct Holder { char d[0x30]; ~Holder(); };
struct BodyObj { ~BodyObj(); };
struct PropBase { virtual ~PropBase(); char d[0x3c]; };
struct FrameProp2 : PropBase {
    Names names;                 // 0x40
    char pad[0x54 - 0x44 - 0x10 + 0x10];
    ChildList children;          // 0x54
    Holder holder;               // 0x64
    BodyObj *body;               // 0x94
    virtual ~FrameProp2();
};
FrameProp2::~FrameProp2()
{
    int i;
    for (i = 0; i < children.count; i++) {
        delete children.Get(i);
    }
    if (body) delete body;
}
