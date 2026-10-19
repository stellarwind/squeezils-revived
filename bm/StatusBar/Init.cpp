// StatusBar::Init (SQZ.EXE 0x42ec10): set up both counters, the two sub-objects and the two face bitmaps; 0 on failure.
extern char kAmoLabel[], kFoodLabel[];      // 0x4bbfec, 0x4bbfe4
struct CounterI { void Setup(const char *label, int x, int y, int flag); };   // 0x42f7d0
struct SubA { int Init(int a, int b); };    // 0x415750
struct SubB { int Init(int a, int b); };    // 0x405520
struct AppI;
extern AppI *g_app;                         // 0x4c75dc
struct FaceI { FaceI(AppI *app); char body[0x80]; };    // 0x473760

struct StatusI {
    CounterI amo;                           // 0x00
    char pad0[0x2c - 1];
    int amoCount, amoShown;                 // 0x2c
    CounterI food;                          // 0x34
    char pad1[0x60 - 0x35];
    int foodCount;                          // 0x60
    int pad64;
    int foodShown;                          // 0x68
    SubA subA;                              // 0x6c
    char pad2[0x9c - 0x6d];
    SubB subB;                              // 0x9c
    char pad3[0x37c - 0x9d];
    FaceI *faceA;                           // 0x37c
    FaceI *faceB;                           // 0x380
    int x, y;                               // 0x384
    int Layout();                           // 0x42f2c0
    int Init();
};

int StatusI::Init()
{
    amo.Setup(kAmoLabel, 0x19, 0x14, 0);
    amoCount = 0;
    amoShown = -1;
    food.Setup(kFoodLabel, 0x1fe, 10, 1);
    foodCount = 0;
    foodShown = -1;
    if (!subA.Init(0x19a, 0x14))
        return 0;
    if (!subB.Init(0x96, 14))
        return 0;
    faceA = new FaceI(g_app);
    x = 0x195;
    y = 12;
    faceB = new FaceI(g_app);
    return Layout() != 0;
}
