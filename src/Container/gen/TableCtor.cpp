// Character/Creature pointer-table constructors (0x4650b0, 0x466590).
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#include "Container/ContainerTable.h"
#include "Container2/ObjectArrayViews.h"
// RTTI: 0x4a051c/0x4a0574 => Character*, ID 5;
// 0x4a009c/0x4a05e4 => Creature*, ID 10. Explicit constructor owners only.
template Table<int, Character *, ObjArrayC, 5>::Table(); // 0x4650b0
template Table<int, Creature *, ObjArrayD, 10>::Table(); // 0x466590
