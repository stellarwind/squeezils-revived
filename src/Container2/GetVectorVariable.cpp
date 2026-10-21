// SQZ.EXE 0x46b700: named-vector lookup used by Vector3fFunctionArgument::Get.
// Only the interface and layout needed by this accessor are declared here.
#include "String/String.h"
#include "Container/ContainerArray.h"

// SIZE 0x0c
struct Vector3f {
    float x, y, z;                          // 0x00, 0x04, 0x08
    Vector3f() {}
    ~Vector3f();                            // 0x401660
};

// SIZE 0x14
struct VectorVariableKeys {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual int IndexOf(const String &name); // +0x1c
    char rest[0x10];                        // full key-set size: 0x14
};

// SIZE 0x28
struct VectorVariableTable {
    virtual void slot0();
    VectorVariableKeys keys;                // +0x4
    Array<Vector3f> values;                 // +0x18, Array::data at +0x0c
    Vector3f At(int index);                 // 0x45d7e0: values array at +0x18
    Vector3f Get(const String &name);
};

Vector3f VectorVariableTable::At(int index)
{
    return values.At(index);
}

Vector3f VectorVariableTable::Get(const String &name)
{
    return At(keys.IndexOf(name));
}
