// 0x40d530: constructor of an object whose members are seven container instances (constructed out of line, destroyed in reverse on unwinding).
struct MA { char d[0x14]; MA(); ~MA(); };    // ctor 0x45f610, dtor 0x45f700
struct MB { char d[0x10]; MB(); ~MB(); };    // ctor 0x46c960, dtor 0x46c990
struct MC { char d[0x10]; MC(); ~MC(); };    // ctor 0x46cb10, dtor 0x46cb40
struct MD { char d[0x10]; MD(); ~MD(); };    // ctor 0x46ccc0, dtor 0x46ccf0
struct ME { char d[0x14]; ME(); ~ME(); };    // ctor 0x45eef0, dtor 0x45efa0
struct MF { char d[0x14]; MF(); ~MF(); };    // ctor 0x46cff0, dtor 0x46d130
struct Holder7 {
    char pad[0x100];
    MA a;           // 0x100
    MB b;           // 0x114
    MC c;           // 0x124
    MD d;           // 0x134
    ME e1;          // 0x144
    ME e2;          // 0x158
    MF f;           // 0x16c
    Holder7();
};
Holder7::Holder7() {}
