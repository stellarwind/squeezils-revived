// SQZ.EXE 0x4246f0 / 0x425220: find a resource in the newest table containing it.
#include "String/String.h"

struct GeometryFrame;
struct ResourceTexture;
template <class T> struct ResourceLookupTable {
    virtual void slot0();
    virtual int Contains(const String &name);           // slot 1
    T *Get(const String &name);                         // 0x467a50 / 0x467e90
};
template <class T> struct ResourceLookupStack {
    void *vtable;
    int count, capacity;
    ResourceLookupTable<T> **data;
    ResourceLookupTable<T> *At(int index) { return data[index]; }
};
extern ResourceLookupStack<GeometryFrame> gGeometryStack; // 0x4c71c0
extern ResourceLookupStack<ResourceTexture> gTextureStack; // 0x4c7180

GeometryFrame *FindGeometry(const char *name)
{
    for (int index = gGeometryStack.count - 1; index >= 0; --index) {
        if (gGeometryStack.At(index)->Contains(String(name)))
            return gGeometryStack.At(index)->Get(String(name));
    }
    return 0;
}

ResourceTexture *FindTexture(const char *name)
{
    for (int index = gTextureStack.count - 1; index >= 0; --index) {
        if (gTextureStack.At(index)->Contains(String(name)))
            return gTextureStack.At(index)->Get(String(name));
    }
    return 0;
}
