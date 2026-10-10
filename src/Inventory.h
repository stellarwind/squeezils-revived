// Inventory: what the player carries and has done (RTTI "Inventory", derives BufferObject; vtable 0x49d6c4, 0x70 bytes, code
// 0x415ef0-0x416bf0). One list of integers per goodie name ("FoodNutGoodie", "SpitNutGoodie", "PlacesCompleted", "PlacesBeenTo",
// "TotalFoodNuts", ...), the one goodie held (name and value), and the quest flags: three key piece flags and one relic flag per
// region. Saved to and loaded from the saved game as plain text. The whole inventory lives in Possessions (+4).
//
// Regions are numbered from 1 by the callers, but the flag arrays start at region 1: keyFlags[(region - 1) * 3 + piece].
// A fresh inventory has only region 1's flags set (every player starts having found region 1's pieces and relic).
//
// The class has no C++ virtuals: the original's vtable has the BufferObject slots plus the destroy slot, and the objects are
// made and freed by original code, so vtbl is set to the original table and the slot methods are plain methods that the hooks
// install over the original entries.
#pragma once
#include "Container.h"

typedef Table<String, IntList *, Array<IntList *>, 5> InventoryTable;   // goodie name -> its values; ctor @4655a0, dtor @4660a0

struct Inventory {
    const void *vtbl;         // 0x00 0x49d6c4
    InventoryTable *table;    // 0x04 one value list per goodie name (lists and table are freed with the inventory)
    String   held;            // 0x08 the goodie held ("" for none)
    int      heldValue;       // 0x0c -1 for none
    int      keyFlags[18];    // 0x10 three per region, regions 1 to 6
    int      relics[6];       // 0x58 one per region, regions 1 to 6

    Inventory();                                        // @00415ef0
    ~Inventory();                                       // @00416030 (the body of the scalar deleting destructor @00416010)

    void  Add(const char *goodie, int value);           // @00416110 appends a value to the goodie's list, making the list when it is the first
    void  Remove(const char *goodie, int index);        // @00416280 removes one value (the goodie must exist)
    int   Count(const char *goodie);                    // @00416300 how many values the goodie has; 0 for none
    IntList *Items(const char *goodie);                 // @004163d0 its list, NULL when it has none
    void  RemoveNuts();                                 // @004164a0 empties the food nut and spit nut lists and drops the held goodie
    int   KeysFound(int region);                        // @00416580 (ret 4) all three key pieces of the region found
    int   AllRelics();                                  // @004165b0 the relics of regions 1 to 5 found
    int   Save(void *file);                             // @00416600 (ret 4) text to the saved game; 0 on a write error
    int   Load(void *file);                             // @00416820 (ret 4) from the saved game; 0 on a read error

    // the BufferObject slots
    const char *TypeName();                             // @00416be0 slot 0: "Inventory"
    Inventory  *Create();                               // @00416bf0 slot 1: a new empty inventory (NULL when out of memory)
    void  Write(void *stream);                          // @004165d0 slot 2 (ret 4): does nothing (not hooked: 3 bytes)
    void  ReleaseTable(void *stream);                   // @004165e0 slot 3 (ret 4): destroys the table but leaves the pointer dangling (an original bug)
    static void *operator new(size_t n) noexcept { return Operator_New(n); }
    static void operator delete(void *p) { Operator_Delete(p); }
    static void *operator new(size_t, void *p) noexcept { return p; }     // for the hooks, which construct in place
    static void operator delete(void *, void *) noexcept {}
};
static_assert(sizeof(Inventory) == 0x70, "Inventory layout");
