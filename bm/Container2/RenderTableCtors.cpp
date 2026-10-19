// 0x466de0: named frame table constructor.
#include "String/String.h"
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#define SET_CREATE
#include "Container/ContainerTable.h"

struct IDirect3DRMFrame;
struct IDirect3DRMTexture;
template class Table<String, IDirect3DRMFrame *, Array<IDirect3DRMFrame *>, 0>;
