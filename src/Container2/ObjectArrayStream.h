// Stream methods used by the pointer-array serialization owners.
#pragma once
class BufferObject;
class ByteStream {
public:
    ByteStream &WriteInt(int *p); // 0x448ac0
    ByteStream &ReadInt(int *p); // 0x448af0
    ByteStream &WriteObject(BufferObject *object); // 0x448c60
    ByteStream &ReadObject(BufferObject **object); // 0x448ca0
};
