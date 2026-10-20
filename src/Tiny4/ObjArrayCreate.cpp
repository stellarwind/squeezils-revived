// BufferObjectArray<Character*,5>::Create (SQZ.EXE 0x464f50): new instance, returned as its BufferObject sub-object.
#define CONTAINER_DEFINITIONS
#include "Persist/Stream.h"
#include "Container/ContainerArray.h"
struct Character;
struct ObjArrayC : Array<Character *>, BufferObject {
    virtual const char *TypeName();
    virtual BufferObject *Create();
    virtual void Write(ByteStream *s);
    virtual void Read(ByteStream *s);
};
BufferObject *ObjArrayC::Create()
{
    return new ObjArrayC;
}

struct Creature;
struct ObjArrayD : Array<Creature *>, BufferObject {
    virtual const char *TypeName();
    virtual BufferObject *Create();
    virtual void Write(ByteStream *s);
    virtual void Read(ByteStream *s);
};
BufferObject *ObjArrayD::Create()
{
    return new ObjArrayD;
}
