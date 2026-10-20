// Memberwise assignment of a 0x2a0-byte record (SQZ.EXE 0x405040, ret 4): scalar fields, then arrays of 6, 23, 10 and 11 ints, then scalars.
struct I6 { int v[6]; };
struct I23 { int v[23]; };
struct I10 { int v[10]; };
struct I11 { int v[11]; };
struct Record {
    int a0, a1, a2, a3, a4, a5, a6, a7, a8;   // 0x00
    I6 g1, g2, g3, g4;                        // 0x24, 0x3c, 0x54, 0x6c
    I23 h1, h2, h3;                           // 0x84, 0xe0, 0x13c
    I10 k1, k2, k3;                           // 0x198, 0x1c0, 0x1e8
    I11 m1, m2;                               // 0x210, 0x23c
    int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13;   // 0x268
    Record &operator=(const Record &o);
};

Record &Record::operator=(const Record &o)
{
    a0 = o.a0; a1 = o.a1; a2 = o.a2; a3 = o.a3; a4 = o.a4; a5 = o.a5; a6 = o.a6; a7 = o.a7; a8 = o.a8;
    g1 = o.g1; g2 = o.g2; g3 = o.g3; g4 = o.g4;
    h1 = o.h1; h2 = o.h2; h3 = o.h3;
    k1 = o.k1; k2 = o.k2; k3 = o.k3;
    m1 = o.m1; m2 = o.m2;
    t0 = o.t0; t1 = o.t1; t2 = o.t2; t3 = o.t3; t4 = o.t4; t5 = o.t5; t6 = o.t6;
    t7 = o.t7; t8 = o.t8; t9 = o.t9; t10 = o.t10; t11 = o.t11; t12 = o.t12; t13 = o.t13;
    return *this;
}
