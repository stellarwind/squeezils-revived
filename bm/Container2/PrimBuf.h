// BufferObject slots of PrimitiveArray<T,ID> (TypeName, Create, Write, Read), VC++ 4.1 dialect.
// The original keeps a global name cache shared by every instantiation (0x4bf95c) and rebuilds the ID digits on each call.
// Include the element type's header (String/String.h, Vector3f/Vector3f.h) before this file.
#pragma once
#define BUFFEROBJECT_SIGNATURES
#define CONTAINER_DEFINITIONS
#include <stdio.h>
#include <string.h>
#include "Container/ContainerSet.h"

// Only the member declarations of the stream that the slots call (the class is in Persist/Stream.h).
struct Vec3 { float x, y, z; };
class ByteStream {
public:
    ByteStream &WriteInt(int *p);
    ByteStream &ReadInt(int *p);
    ByteStream &WriteFloat(float *p);
    ByteStream &ReadFloat(float *p);
    ByteStream &WriteVec3(Vec3 *v);
    ByteStream &ReadVec3(Vec3 *v);
};

extern char *g_typeNameBase;      // "Array  " at 0x4bf958
extern char *g_typeNameCache;     // 0x4bf95c

// Per element type: how one element goes into / out of the stream.
inline void WriteElem(ByteStream *s, int *p) { s->WriteInt(p); }
inline void ReadElem(ByteStream *s, int *p) { s->ReadInt(p); }
inline void WriteElem(ByteStream *s, float *p) { s->WriteFloat(p); }
inline void ReadElem(ByteStream *s, float *p) { s->ReadFloat(p); }
#ifdef PB_VECTOR3F
inline void WriteElem(ByteStream *s, Vector3f *p) { s->WriteVec3((Vec3 *)p); }
inline void ReadElem(ByteStream *s, Vector3f *p) { s->ReadVec3((Vec3 *)p); }
#endif
#ifdef PB_STRING
void WriteElem(ByteStream *s, String *p);        // 0x438770
void ReadElem(ByteStream *s, String *p);         // 0x4387a0
#endif

template <class T, int ID>
const char *PrimitiveArray<T, ID>::TypeName()
{
    if (g_typeNameCache == 0) {
        g_typeNameCache = new char[strlen(g_typeNameBase) + 3];
        strcpy(g_typeNameCache, g_typeNameBase);
    }
    sprintf(g_typeNameCache + strlen("PrimArray"), "%2ux", ID);
    return g_typeNameCache;
}

template <class T, int ID>
BufferObject *PrimitiveArray<T, ID>::Create()
{
    return new PrimitiveArray<T, ID>;
}

template <class T, int ID>
void PrimitiveArray<T, ID>::Write(void *stream)
{
    ByteStream *s = (ByteStream *)stream;
#ifdef PB_WRITE_TEMPS
    // Same code; the extra named temporaries only shift VC++ 4.1's register choice (PrimitiveArray<String, 3>).
    int *n = &count;
    s->WriteInt(n);
    for (int i = 0; i < count; i++) {
        T *e = &data[i];
        ByteStream *t = s;
        WriteElem(t, e);
    }
#else
    s->WriteInt(&count);
    for (int i = 0; i < count; i++)
        WriteElem(s, &data[i]);
#endif
#ifdef PB_WRITE_TAIL
    enum { WT1, WT2, WT3, WT4, WT5, WT6, WT7, WT8 };
#endif
}

template <class T, int ID>
void PrimitiveArray<T, ID>::Read(void *stream)
{
    ByteStream *s = (ByteStream *)stream;
    s->ReadInt(&count);
    if (capacity < count) {
        Array<T> *self = this;
        self->Grow(count - capacity);
    }
    for (int i = 0; i < count; i++)
        ReadElem(s, &data[i]);
}

#ifdef SET_CREATE
template <class T, int ID>
BufferObject *Set<T, ID>::Create()
{
    return new Set<T, ID>;
}
#endif
