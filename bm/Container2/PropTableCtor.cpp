// 0x468800: named prop table constructor.
#include "String/String.h"
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#define SET_CREATE
#include "Container/ContainerTable.h"

class Prop;
class MorphCycle;
template class Table<String, Prop *, Array<Prop *>, 0>;
