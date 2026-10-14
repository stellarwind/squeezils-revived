// Inventory (SQZ.EXE 0x415ef0-0x416bf0), functions in address order.
#include "Inventory/Inventory.h"

extern char kFoodNutGoodie[];            // "FoodNutGoodie" 0x4ba2e0
extern char kSpitNutGoodie[];            // "SpitNutGoodie" 0x4b9334

Inventory::Inventory()
{
    table = new InventoryTable;
    held = String(kEmptyString);
    heldValue = -1;
    for (int region = 0; region < 6; region++) {
        for (int piece = 0; piece < 3; piece++) {
            keyFlags[region * 3 + piece] = 1;
            if (region != 0)
                keyFlags[region * 3 + piece] = 0;
        }
        relics[region] = 1;
        if (region != 0)
            relics[region] = 0;
    }
}

Inventory::~Inventory()
{
    int i = 0;
    for (;;) {
        if (i >= table->keys.count)
            break;
        IntList *list = table->values.data[i];
        if (list)
            delete list;
        i++;
    }
    delete table;
}

void Inventory::Add(const char *goodie, int value)
{
    if (table->IndexOf(String(goodie)) == -1) {
        IntList *list = new IntList;
        list->Insert(value, -1);
        table->Insert(String(goodie), list, -1);
    } else {
        table->Get(String(goodie))->Insert(value, -1);
    }
}

void Inventory::Remove(const char *goodie, int index)
{
    table->Get(String(goodie))->Remove(index);
}

int Inventory::Count(const char *goodie)
{
    if (table->IndexOf(String(goodie)) == -1)
        return 0;
    return table->Get(String(goodie))->count;
}

IntList *Inventory::Items(const char *goodie)
{
    if (table->IndexOf(String(goodie)) == -1)
        return 0;
    return table->Get(String(goodie));
}

void Inventory::RemoveNuts()
{
    int n = Count(kFoodNutGoodie);
    IntList *list = Items(kFoodNutGoodie);
    int i;
    for (i = n - 1; i >= 0; i--)
        list->Remove(i);
    n = Count(kSpitNutGoodie);
    list = Items(kSpitNutGoodie);
    for (i = n - 1; i >= 0; i--)
        list->Remove(i);
    held = String(kEmptyString);
    heldValue = -1;
}

int Inventory::KeysFound(int region)
{
    for (int piece = 0; piece < 3; piece++)
        if (!keyFlags[(region - 1) * 3 + piece])
            return 0;
    return 1;
}

int Inventory::AllRelics()
{
    for (int region = 0; region < 5; region++)
        if (!relics[region])
            return 0;
    return 1;
}

void Inventory::Write(void *stream)
{
}

void Inventory::Read(void *stream)
{
    delete table;
}

const char *Inventory::TypeName()
{
    return "Inventory";
}
