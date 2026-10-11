// ByteStream: the growable memory buffer the save/load code writes into and reads from (SQZ.EXE 0x448a70-0x448ca0), plus
// the BufferObject registry (0x448ce0 / 0x448d20). VC++ 4.1 dialect.
//   +0x04 writePos, +0x08 capacity, +0x0c readPos, +0x10 data (realloc'ed, grown by 1.5x)
// Every Write/Read returns the stream (chaining). Strings are stored with their terminating NUL.
#pragma once
#include "compat41.h"

class ByteStream;

struct Vec3 { float x, y, z; };      // plain 12-byte value copied member-wise

// The save/load interface of every container the game serialises (vtable of the BufferObject base at +0x10 of a PrimitiveArray).
class BufferObject {
public:
    virtual const char *TypeName() = 0;            // slot 0
    virtual BufferObject *Create() = 0;            // slot 1: a fresh empty instance
    virtual void Write(ByteStream *s) = 0;         // slot 2
    virtual void Read(ByteStream *s) = 0;          // slot 3
};

class ByteStream {
public:
    int unknown0;
    int writePos;
    int capacity;
    int readPos;
    char *data;

    void Reserve(int n);                           // 0x448a70
    ByteStream &WriteInt(int *p);                  // 0x448ac0
    ByteStream &ReadInt(int *p);                   // 0x448af0
    ByteStream &WriteFloat(float *p);              // 0x448b10
    ByteStream &ReadFloat(float *p);               // 0x448b40
    ByteStream &WriteVec3(Vec3 *v);               // 0x448b60
    ByteStream &ReadVec3(Vec3 *v);                // 0x448ba0
    ByteStream &WriteString(char **s);             // 0x448bd0
    ByteStream &ReadString(char **s);              // 0x448c20
    ByteStream &WriteObject(BufferObject *o);      // 0x448c60
    ByteStream &ReadObject(BufferObject **o);      // 0x448ca0
};

void RegisterBufferObject(BufferObject *proto);    // 0x448ce0
BufferObject *FindBufferObject(const char *name);  // 0x448d20
