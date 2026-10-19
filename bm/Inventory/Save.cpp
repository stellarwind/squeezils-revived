// Inventory::Save (SQZ.EXE 0x416600, ret 4): write the held item, the key and relic flags, then every goodie list as text.
#include <stdio.h>
#include "Inventory/Inventory.h"

struct TableKeys { String KeyAt(int i); };   // 0x4167b0

int Inventory::Save(void *file)
{
    FILE *f = (FILE *)file;
    const char *text = held.c_str();
    if (fprintf(f, "%s\n%d\n", text, heldValue) < 0)
        return 0;
    int region = 0;
    int *flags = &keyFlags[2];
    int *relic = &relics[0];
    for (; region < 6; region++) {
        if (fprintf(f, "%d %d %d %d\n", flags[-2], flags[-1], flags[0], *relic) < 0)
            return 0;
        flags += 3;
        relic++;
    }
    if (fprintf(f, "%d inventory items!\n", table->keys.count) < 0)
        return 0;
    for (int i = 0; i < table->keys.count; i++) {
        const char *name = ((TableKeys *)table)->KeyAt(i).c_str();
        IntList *list = table->values.data[i];
        if (fprintf(f, "%s %d\n", name, list->count) < 0)
            return 0;
        for (int j = 0; j < list->count; j++)
            if (fprintf(f, "%d\n", list->Get(j)) < 0)
                return 0;
    }
    return 1;
}
