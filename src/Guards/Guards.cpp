// Guarded one-shot calls on a global: `if (!(flags & 1)) { flags |= 1; g.M(); }` (SQZ.EXE 0x42d350, 0x4429c0).
struct OA { void M(); };            // M = 0x468940
extern OA g_42d350;                 // 0x4c7208
extern unsigned char f_42d350;      // 0x4c72f8
void G42d350() { if (!(f_42d350 & 1)) { f_42d350 |= 1; g_42d350.M(); } }

struct OB { void M(); };            // M = 0x46ea60
extern OB g_4429c0;                 // 0x4c7738
extern unsigned char f_4429c0;      // 0x4c7760
void G4429c0() { if (!(f_4429c0 & 1)) { f_4429c0 |= 1; g_4429c0.M(); } }

extern unsigned char f_4c70d8;
struct O421ce0 { void M(); };   // M = 0x46c2c0
extern O421ce0 g_421ce0;   // 0x4c70a0
void G421ce0() { if (!(f_4c70d8 & 1)) { f_4c70d8 |= 1; g_421ce0.M(); } }

extern unsigned char f_4c70dc;
struct O4225a0 { void M(); };   // M = 0x46c480
extern O4225a0 g_4225a0;   // 0x4c7090
void G4225a0() { if (!(f_4c70dc & 1)) { f_4c70dc |= 1; g_4225a0.M(); } }

struct O4225f0 { void M(); };   // M = 0x46c630
extern O4225f0 g_4225f0;   // 0x4c70c8
void G4225f0() { if (!(f_4c70dc & 2)) { f_4c70dc |= 2; g_4225f0.M(); } }

extern unsigned char f_4c70e0;
struct O424e00 { void M(); };   // M = 0x466ab0
extern O424e00 g_424e00;   // 0x4c71c0
void G424e00() { if (!(f_4c70e0 & 1)) { f_4c70e0 |= 1; g_424e00.M(); } }

extern unsigned char f_4c70e4;
struct O425d20 { void M(); };   // M = 0x466c60
extern O425d20 g_425d20;   // 0x4c7180
void G425d20() { if (!(f_4c70e4 & 1)) { f_4c70e4 |= 1; g_425d20.M(); } }

extern unsigned char f_4c72f8;
struct O42d3a0 { void M(); };   // M = 0x468940
extern O42d3a0 g_42d3a0;   // 0x4c7230
void G42d3a0() { if (!(f_4c72f8 & 2)) { f_4c72f8 |= 2; g_42d3a0.M(); } }

struct O42d3f0 { void M(); };   // M = 0x468940
extern O42d3f0 g_42d3f0;   // 0x4c72c0
void G42d3f0() { if (!(f_4c72f8 & 4)) { f_4c72f8 |= 4; g_42d3f0.M(); } }

struct O42d440 { void M(); };   // M = 0x468f40
extern O42d440 g_42d440;   // 0x4c71f8
void G42d440() { if (!(f_4c72f8 & 8)) { f_4c72f8 |= 8; g_42d440.M(); } }

struct O42d490 { void M(); };   // M = 0x468f40
extern O42d490 g_42d490;   // 0x4c72e8
void G42d490() { if (!(f_4c72f8 & 16)) { f_4c72f8 |= 16; g_42d490.M(); } }

struct O42d4e0 { void M(); };   // M = 0x45efa0
extern O42d4e0 g_42d4e0;   // 0x4c72a8
void G42d4e0() { if (!(f_4c72f8 & 32)) { f_4c72f8 |= 32; g_42d4e0.M(); } }

struct O42d530 { void M(); };   // M = 0x468d80
extern O42d530 g_42d530;   // 0x4c7280
void G42d530() { if (!(f_4c72f8 & 64)) { f_4c72f8 |= 64; g_42d530.M(); } }

struct O42d580 { void M(); };   // M = 0x468d80
extern O42d580 g_42d580;   // 0x4c7258
void G42d580() { if (!(f_4c72f8 & 128)) { f_4c72f8 |= 128; g_42d580.M(); } }

extern unsigned char f_4c72fc;
struct O42d5d0 { void M(); };   // M = 0x468d80
extern O42d5d0 g_42d5d0;   // 0x4c71d0
void G42d5d0() { if (!(f_4c72fc & 1)) { f_4c72fc |= 1; g_42d5d0.M(); } }

extern unsigned char f_4c780c;
struct O449430 { void M(); };   // M = 0x467ee0
extern O449430 g_449430;   // 0x4c7810
void G449430() { if (!(f_4c780c & 1)) { f_4c780c |= 1; g_449430.M(); } }

struct O449480 { void M(); };   // M = 0x468090
extern O449480 g_449480;   // 0x4c77e0
void G449480() { if (!(f_4c780c & 2)) { f_4c780c |= 2; g_449480.M(); } }

struct O4494d0 { void M(); };   // M = 0x468090
extern O4494d0 g_4494d0;   // 0x4c77d0
void G4494d0() { if (!(f_4c780c & 4)) { f_4c780c |= 4; g_4494d0.M(); } }

extern unsigned char f_4c7b78;
struct O458030 { void M(); };   // M = 0x46b160
extern O458030 g_458030;   // 0x4c7b98
void G458030() { if (!(f_4c7b78 & 1)) { f_4c7b78 |= 1; g_458030.M(); } }
