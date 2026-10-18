// StatusBar::~StatusBar (SQZ.EXE 0x42eb40): free the dead and won pictures, then destroy the widgets.
struct BitmapS { ~BitmapS(); };                 // 0x473890
struct NutCounterS { char d[0x2c]; ~NutCounterS(); };     // 0x42f790
struct GoodieListS { char d[0x30]; ~GoodieListS(); };     // 0x4156d0
struct MoodFaceS { char d[0x2c8]; ~MoodFaceS(); };        // 0x405490

struct StatusBarD {
    NutCounterS amo;            // 0x000
    int amoCount, amoShown;     // 0x02c
    NutCounterS food;           // 0x034
    int foodCount, foodGoal, foodShown;   // 0x060
    GoodieListS goodies;        // 0x06c
    MoodFaceS face;             // 0x09c
    char pad[0x37c - 0x364];
    BitmapS *deadMessage;       // 0x37c
    BitmapS *wonMessage;        // 0x380
    int messageX, messageY;     // 0x384
    ~StatusBarD();
};

StatusBarD::~StatusBarD()
{
    delete deadMessage;
    delete wonMessage;
}
