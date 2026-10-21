// BufferObjectArray<Character*,5>::Create (SQZ.EXE 0x464f50): new instance, returned as its BufferObject sub-object.
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#include "Container2/ObjectArrayViews.h"

BufferObject *ObjArrayC::Create()
{
    return new ObjArrayC;
}

BufferObject *ObjArrayD::Create()
{
    return new ObjArrayD;
}
