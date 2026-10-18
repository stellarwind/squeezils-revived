// Array<Vector3f>::At (SQZ.EXE 0x45c5b0): the element returned by value (Vector3f has a destructor, hence the return-slot flag).
#define V3_COPYCTOR
#include "Vector3f/Vector3f.h"
#include "Container/ContainerArray.h"
#include "Container/Accessors.h"

template Vector3f Array<Vector3f>::At(int);
