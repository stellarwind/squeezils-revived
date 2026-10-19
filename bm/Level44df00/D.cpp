// PlaceLevelFinish (SQZ.EXE 0x44df00): feeds each entry of the level's list to the 0x44d290 handler, with a scratch object on the stack.
struct Float3 { float x, y, z; Float3() {} ~Float3(); };   // dtor 0x401660
struct Scratch { char pad[0x4c]; Float3 a; Float3 b; Float3 c; int tail; Scratch() {} ~Scratch(); };   // dtor 0x44dfa0
struct List { virtual void *Get(int i); int count; };
struct Level {
    char pad[0x1e8];
    List items;                                        // 0x1e8
    void Handle(void *item);                           // 0x44d290
    void Finish();
};
void Level::Finish()
{
    Scratch scratch;
    for (int i = 0; i < items.count; i++)
        Handle(items.Get(i));
}
