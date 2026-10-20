// Inventory::Load (SQZ.EXE 0x416820, ret 4): read back what Save wrote: the held item, the key and relic flags,
// then replace every goodie list. 0 when the file ends early.
#include <stdio.h>
#include <string.h>
#include "Inventory/Inventory.h"

extern char kEmptyString[];     // 0x4b9488

int Inventory::Load(void *file)
{
    FILE *f = (FILE *)file;
    char line[256];
    if (!fgets(line, 0xff, f))
        return 0;
    if (strlen(line) == 1 && line[0] == '\n') {
        held = String(kEmptyString);
    } else {
        char name[256];
        name[0] = 0;
        sscanf(line, "%s", name);
        held = String(name);
    }
    if (!fgets(line, 0xff, f))
        return 0;
    int value;
    sscanf(line, "%d", &value);
    heldValue = value;
    int region = 0;
    int *flags = &keyFlags[2];
    int *relic = &relics[0];
    for (; region < 6; region++) {
        if (!fgets(line, 0xff, f))
            return 0;
        sscanf(line, "%d %d %d %d", &flags[-2], &flags[-1], &flags[0], relic);
        flags += 3;
        relic++;
    }
    int i;
    for (i = table->keys.count - 1; i >= 0; i--) {
        IntList *old = table->values.data[i];
        if (old)
            delete old;
        table->RemoveAt(i);
    }
    if (!fgets(line, 0xff, f))
        return 0;
    int lists;
    sscanf(line, "%d", &lists);
    for (i = 0; i < lists; i++) {
        if (!fgets(line, 0xff, f))
            return 0;
        char goodie[256];
        int count;
        sscanf(line, "%s %d", goodie, &count);
        IntList *list = new IntList;
        for (int j = 0; j < count; ) {
            if (!fgets(line, 0xff, f))
                return 0;
            j++;
            int item;
            sscanf(line, "%d", &item);
            list->Insert(item, -1);
        }
        table->Insert(String(goodie), list, -1);
    }
    RemoveNuts();
    return 1;
}
