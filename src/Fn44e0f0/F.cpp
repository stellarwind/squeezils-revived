// 0x44e0f0: advances the timers of all children and fires the ones that ran out.
// SIZE 0x4c (fields used here)
struct Item {
    char pad[0x44];
    float limit;        // 0x44
    float timer;        // 0x48
};
// SIZE 0x08 (fields used here)
struct ItemList {
    virtual Item *Get(int i);
    int count;          // 0x1ec - 0x1e8 + ... (list object at 0x1e8)
};
struct Clock { unsigned Ticks(); };            // 0x41ec00
Clock *GetClock();                             // 0x4206d0
// SIZE 0x1f0 (fields used here)
struct Owner {
    char pad[0x1e8];
    ItemList items;     // 0x1e8
    void Fire(Item *it);                       // 0x44d290
    void Tick();
};
void Owner::Tick()
{
    for (int i = 0; i < items.count; i++) {
        if (items.Get(i)->timer != -1.0f) {
            Item *it = items.Get(i);
            it->timer = it->timer + (float)GetClock()->Ticks() * 0.001f;
            float t = items.Get(i)->timer;
            Item *limitItem = items.Get(i);
            // VC4 reverses >= and changes the unordered (NaN) result.
            // Preserve the original x87 comparison and its C0 branch test.
            unsigned char compareFlags;
            __asm {
                mov eax, limitItem
                fld DWORD PTR t
                fcomp DWORD PTR [eax+0x44]
                fnstsw ax
                mov compareFlags, ah
            }
            if (!(compareFlags & 1)) {
                Fire(items.Get(i));
                items.Get(i)->timer = -1.0f;
            }
        }
    }
}
