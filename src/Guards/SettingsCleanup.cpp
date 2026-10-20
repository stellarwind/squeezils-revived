// 0x44f110 / 0x459540: clean up the vector pair at the start of each settings block.
struct Float3 { float x, y, z; ~Float3(); };
struct Pair {
    Float3 a, b;
    ~Pair();                                // 0x4052b0
};
struct SettingsCleanup {
    Pair vectors;
    ~SettingsCleanup() {}
};
extern SettingsCleanup gSqueezilSettingsCleanup; // 0x4c7828
extern SettingsCleanup gGoodieSettingsCleanup;   // 0x4c7bc0

void G44f110()
{
    gSqueezilSettingsCleanup.~SettingsCleanup();
}

void G459540()
{
    gGoodieSettingsCleanup.~SettingsCleanup();
}
