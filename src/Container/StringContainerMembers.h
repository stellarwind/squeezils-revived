// Typed complete-destructor members used by loaded-surface and instance cleanup.
#pragma once
#include "String/String.h"
#include "Container/ContainerTable.h"

// SIZE 0x14: original Array<String> base plus BufferObject secondary vptr.
struct C40be40 {
    Array<String> array; // 0x00: vptr/count/capacity/data at +0/+4/+8/+c
    void *bufferVirtualTable; // 0x10
    ~C40be40(); // 0x40be40: complete owner Misc17/Dt2.cpp; Array base 0x45e250
};

// SIZE 0x30: original String-to-String table and trailing BufferObject vptr.
struct C40c3d0 {
    Table<String, String, PrimitiveArray<String, 3>, 23> table; // 0x00; keys +4, values +0x18
    void *bufferVirtualTable; // 0x2c
    ~C40c3d0(); // 0x40c3d0: complete owner Misc17/Dt2.cpp; Table base 0x462b50
};
typedef char StringArrayMemberSize[(sizeof(C40be40) == 0x14) ? 1 : -1];
typedef char StringTableMemberSize[(sizeof(C40c3d0) == 0x30) ? 1 : -1];
