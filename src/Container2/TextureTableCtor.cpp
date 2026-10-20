// 0x467a70: named texture table constructor.
#include "String/String.h"
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#define SET_CREATE
#include "Container/ContainerTable.h"

struct IDirect3DRMFrame;
struct IDirect3DRMTexture;
template class Table<String, IDirect3DRMTexture *, Array<IDirect3DRMTexture *>, 0>;
