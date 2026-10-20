// BufferObjectTable<int, Character*/Creature*, BufferObjectArray<..>, ID> Create slots (SQZ.EXE 0x4653e0, 0x4668c0).
#define CONTAINER_DEFINITIONS
#include "Container2/PrimBuf.h"
#include "Container/ContainerTable.h"

class Character;
class Creature;
struct ObjArrayC : Array<Character *>, BufferObject {
    virtual const char *TypeName();
    virtual BufferObject *Create();
    virtual void Write(void *s);
    virtual void Read(void *s);
};
struct ObjArrayD : Array<Creature *>, BufferObject {
    virtual const char *TypeName();
    virtual BufferObject *Create();
    virtual void Write(void *s);
    virtual void Read(void *s);
};

template <class K, class V, class VArray, int ID>
BufferObject *BufferObjectTable<K, V, VArray, ID>::Create()
{
    return new BufferObjectTable<K, V, VArray, ID>;
}

template <class K, class V, class VArray, int ID>
const char *BufferObjectTable<K, V, VArray, ID>::TypeName() { return 0; }
template <class K, class V, class VArray, int ID>
void BufferObjectTable<K, V, VArray, ID>::Write(void *) {}
template <class K, class V, class VArray, int ID>
void BufferObjectTable<K, V, VArray, ID>::Read(void *) {}

template class BufferObjectTable<int, Character *, ObjArrayC, 5>;
template class BufferObjectTable<int, Creature *, ObjArrayD, 10>;
