// SQZ.EXE 0x404f10: constructor of a 24-byte record (id = -1, flag = 1) holding a 12-byte member with an out-of-line dtor (0x401660).
struct Part { int v[3]; Part() {} ~Part(); };
struct Rec {
    int id;          // +0
    Part part;       // +4
    int pad;         // +0x10
    int flag;        // +0x14
    Rec();
};
Rec::Rec() { id = -1; flag = 1; }
