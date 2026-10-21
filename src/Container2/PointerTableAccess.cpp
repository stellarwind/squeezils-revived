// Addressed nonvirtual pointer-table accessors; Table layout is 0x28:
// String key Set at +4, pointer Array at +0x18, value data pointer at +0x24.
#include "String/String.h"
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#define SET_CREATE
#include "Container/ContainerTable.h"
#ifndef SQZ_INTEGRATION
#include "Container/Accessors.h"
#endif

struct RoboBrain;
class SurfaceTypeDescription;

template String Table<String, RoboBrain *, Array<RoboBrain *>, 0>::KeyAt(int);
template SurfaceTypeDescription *Table<String, SurfaceTypeDescription *,
    Array<SurfaceTypeDescription *>, 0>::Get(const String &);
