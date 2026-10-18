// SQZ.EXE 0x459050: constructor: base ctor (0x459320), then two fields.
struct Base11 { Base11(); char data[0x1c]; };
struct C11 : Base11 {
    int f1c, f20;
    C11();
};
C11::C11() { f1c = 0x28; f20 = 0; }
