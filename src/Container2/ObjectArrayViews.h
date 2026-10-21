// Pointer-array views shared by Character/Creature table owners.
#pragma once
#include "Container/ContainerSet.h"
class Character;
class Creature;
// SIZE 0x14: Array at +0x00; BufferObject at +0x10.
struct ObjArrayC : Array<Character *>, BufferObject {
    virtual const char *TypeName(); // BufferObject vtable+0x00
    virtual BufferObject *Create(); // BufferObject vtable+0x04
    virtual void Write(void *stream); // BufferObject vtable+0x08
    virtual void Read(void *stream); // BufferObject vtable+0x0c
};
// SIZE 0x14: Array at +0x00; BufferObject at +0x10.
struct ObjArrayD : Array<Creature *>, BufferObject {
    virtual const char *TypeName(); // BufferObject vtable+0x00
    virtual BufferObject *Create(); // BufferObject vtable+0x04
    virtual void Write(void *stream); // BufferObject vtable+0x08
    virtual void Read(void *stream); // BufferObject vtable+0x0c
};
