// 0x468ad0: named animation-cycle table constructor.
#include "String/String.h"
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#define SET_CREATE
#include "Container/ContainerTable.h"

class Prop;
class MorphCycle;
template class Table<String, MorphCycle *, Array<MorphCycle *>, 0>;
