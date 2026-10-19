// 0x4690c0: construct the region's place-descriptor table.
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#define SET_CREATE
#include "Container/ContainerTable.h"

class PlaceDescriptor;
template class Table<int, PlaceDescriptor *, Array<PlaceDescriptor *>, 9>;
