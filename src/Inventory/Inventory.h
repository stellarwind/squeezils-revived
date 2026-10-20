// Inventory (RTTI "Inventory", derives BufferObject; vtable 0x49d6c4, 0x70 bytes, code 0x415ef0-0x416bf0), VC++ 4.1 dialect.
// Behaviour notes: Repo/legacy_gcc/Inventory.h. This unit defines BUFFEROBJECT_SIGNATURES (see Container/ContainerSet.h).
#pragma once
#define BUFFEROBJECT_SIGNATURES
#include "Container/Container.h"
#include "String/String.h"

typedef PrimitiveArray<int, 1> IntList;
typedef Table<String, IntList *, Array<IntList *>, 5> InventoryTable;   // ctor 0x4655a0, dtor 0x4660a0

class Inventory : public BufferObject {
public:
    Inventory();                                        // 0x415ef0
    virtual ~Inventory();                               // 0x416030 (slot 4; 0x416010 is the scalar deleting destructor)

    void Add(const char *goodie, int value);            // 0x416110
    void Remove(const char *goodie, int index);         // 0x416280
    int Count(const char *goodie);                      // 0x416300
    IntList *Items(const char *goodie);                 // 0x4163d0
    void RemoveNuts();                                  // 0x4164a0
    int KeysFound(int region);                          // 0x416580 (ret 4)
    int AllRelics();                                    // 0x4165b0
    int Save(void *file);                               // 0x416600 (ret 4)
    int Load(void *file);                               // 0x416820 (ret 4)

    virtual const char *TypeName();                     // 0x416be0 slot 0
    virtual BufferObject *Create();                     // 0x416bf0 slot 1
    virtual void Write(void *stream);                   // 0x4165d0 slot 2
    virtual void Read(void *stream);                    // 0x4165e0 slot 3 (destroys the table, leaves the pointer dangling)

    InventoryTable *table;    // 0x04
    String held;              // 0x08
    int heldValue;            // 0x0c
    int keyFlags[18];         // 0x10 three per region
    int relics[6];            // 0x58
};
