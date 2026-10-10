// Inventory.cpp: reconstructed from SQZ.EXE 0x415ef0-0x416bf0. See Inventory.h.
#include "Inventory.h"

static const char kFoodNut[] = "FoodNutGoodie";   // 0x4ba2e0
static const char kSpitNut[] = "SpitNutGoodie";   // 0x4b9334

static const void *const kInventoryVtable = (const void *)0x49d6c4;

Inventory::Inventory()
{
    vtbl = kInventoryVtable;
    table = new InventoryTable();
    held = String("");
    heldValue = -1;
    // region 1 starts with everything found, the other regions with nothing
    for (int region = 0; region < 6; region++) {
        for (int piece = 0; piece < 3; piece++)
            keyFlags[region * 3 + piece] = region == 0 ? 1 : 0;
        relics[region] = region == 0 ? 1 : 0;
    }
}

Inventory::~Inventory()
{
    for (int i = 0; i < table->keys.count; i++) {
        IntList *list = table->values.data[i];
        if (list)
            delete list;
    }
    delete table;
}

void Inventory::Add(const char *goodie, int value)
{
    String key(goodie);
    if (table->IndexOf(key) == -1) {
        IntList *list = new IntList();
        list->Insert(value, -1);
        table->Insert(key, list, -1);
    } else {
        table->Get(key)->Insert(value, -1);
    }
}

void Inventory::Remove(const char *goodie, int index)
{
    table->Get(String(goodie))->RemoveAt(index);
}

int Inventory::Count(const char *goodie)
{
    String key(goodie);
    if (table->IndexOf(key) == -1)
        return 0;
    return table->Get(key)->count;
}

IntList *Inventory::Items(const char *goodie)
{
    String key(goodie);
    if (table->IndexOf(key) == -1)
        return NULL;
    return table->Get(key);
}

void Inventory::RemoveNuts()
{
    static const char *const nuts[2] = { kFoodNut, kSpitNut };
    for (int n = 0; n < 2; n++) {
        int count = Count(nuts[n]);
        IntList *list = Items(nuts[n]);
        for (int i = count - 1; i >= 0; i--)
            list->RemoveAt(i);
    }
    held = String("");
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

// ---- the saved game text ----
//   <held goodie name>\n<held value>\n
//   six lines "key key key relic" (regions 1 to 6)
//   "<n> inventory items!"
//   n times: "<goodie name> <count>" and one line per value

int Inventory::Save(void *file)
{
    if (GameFprintf(file, "%s\n%d\n", held.c_str(), heldValue) < 0)
        return 0;
    for (int region = 0; region < 6; region++)
        if (GameFprintf(file, "%d %d %d %d\n", keyFlags[region * 3], keyFlags[region * 3 + 1], keyFlags[region * 3 + 2], relics[region]) < 0)
            return 0;
    if (GameFprintf(file, "%d inventory items!\n", table->keys.count) < 0)
        return 0;
    for (int i = 0; i < table->keys.count; i++) {
        String name = table->keys.At(i);
        IntList *list = table->values.data[i];
        if (GameFprintf(file, "%s %d\n", name.c_str(), list->count) < 0)
            return 0;
        for (int j = 0; j < list->count; j++)
            if (GameFprintf(file, "%d\n", list->At(j)) < 0)
                return 0;
    }
    return 1;
}

int Inventory::Load(void *file)
{
    char line[256];
    char word[256];

    if (!GameFgets(line, 255, file))
        return 0;
    if (strlen(line) == 1 && line[0] == '\n') {
        held = String("");
    } else {
        word[0] = 0;
        GameSscanf(line, "%s", word);
        held = String(word);
    }

    if (!GameFgets(line, 255, file))
        return 0;
    GameSscanf(line, "%d", &heldValue);

    for (int region = 0; region < 6; region++) {
        if (!GameFgets(line, 255, file))
            return 0;
        GameSscanf(line, "%d %d %d %d", &keyFlags[region * 3], &keyFlags[region * 3 + 1], &keyFlags[region * 3 + 2], &relics[region]);
    }

    // forget what the inventory had
    for (int i = table->keys.count - 1; i >= 0; i--) {
        IntList *list = table->values.data[i];
        if (list)
            delete list;
        table->RemoveAt(i);
    }

    if (!GameFgets(line, 255, file))
        return 0;
    int entries;
    GameSscanf(line, "%d", &entries);
    for (int e = 0; e < entries; e++) {
        if (!GameFgets(line, 255, file))
            return 0;
        int count;
        GameSscanf(line, "%s %d", word, &count);
        IntList *list = new IntList();
        for (int j = 0; j < count; j++) {
            if (!GameFgets(line, 255, file))
                return 0;                    // (the list made so far is lost, as in the original)
            int value;
            GameSscanf(line, "%d", &value);
            list->Insert(value, -1);
        }
        table->Insert(String(word), list, -1);
    }
    RemoveNuts();                            // nuts carried into a saved game do not come back
    return 1;
}

// ---- the BufferObject slots ----

const char *Inventory::TypeName() { return "Inventory"; }

Inventory *Inventory::Create() { return new Inventory(); }

void Inventory::Write(void *) {}

void Inventory::ReleaseTable(void *)
{
    delete table;                            // (table is not cleared: a second call or the destructor would free it again)
}

// ---- the free functions the rest of the code uses (Original.h) ----

void  InventoryConstruct(Inventory *inv)                               { new (inv) Inventory(); }
void  InventoryAdd(Inventory *inv, const char *goodie, int value)      { inv->Add(goodie, value); }
void  InventoryRemove(Inventory *inv, const char *goodie, int index)   { inv->Remove(goodie, index); }
int   InventoryCount(Inventory *inv, const char *goodie)               { return inv->Count(goodie); }
Array<int> *InventoryItems(Inventory *inv, const char *goodie)         { return inv->Items(goodie); }
void  InventoryRemoveNuts(Inventory *inv)                              { inv->RemoveNuts(); }
String *InventoryHeldName(Inventory *inv)                              { return &inv->held; }
int  *InventoryHeldValue(Inventory *inv)                               { return &inv->heldValue; }
int   InventoryLoad(Inventory *inv, void *file)                        { return inv->Load(file); }
int   InventorySave(Inventory *inv, void *file)                        { return inv->Save(file); }
