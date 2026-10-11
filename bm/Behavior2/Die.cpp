// DieActivity::Execute (0x401ef0): drop what the creature holds, clear its inventory, report state 0x16.
#include "Behavior2/Activity.h"

struct DieActivity : Activity {
    virtual int Execute(int dt);
    char pad[0x2ac - 0x0c];
    int done;                                            // 0x2ac
};

int DieActivity::Execute(int dt)
{
    if (!done) {
        ActSoul *s = soul;
        s->place->GetLevel()->DropHeld(s->Owner()->Inventory());
        soul->Owner()->Inventory()->Clear();
        done = 1;
    }
    return 0x16;
}
