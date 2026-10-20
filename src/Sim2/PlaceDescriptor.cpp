// SQZ.EXE 0x416c60 / 0x416d40: construct and release a region-map place record.
#include "String/String.h"

struct PlaceVector {
    float x, y, z;
    PlaceVector() {}
    PlaceVector(float a, float b, float c) : x(a), y(b), z(c) {}
    ~PlaceVector();                                     // 0x401660
    PlaceVector &operator=(const PlaceVector &other) {
        x = other.x; y = other.y; z = other.z;
        return *this;
    }
};
struct PlaceIndexArray {
    void *vtable;
    int count, capacity;
    int *data;
    ~PlaceIndexArray();                                // 0x45d900
};
// PrimitiveArray's implicit destructor performs the Array base cleanup inline.
struct PlaceIndexList : PlaceIndexArray {
    void *bufferVtable;
};
struct PlaceDisplayProp {
    virtual ~PlaceDisplayProp();
    virtual void slot1(); virtual void slot2(); virtual void slot3(); virtual void slot4();
    virtual void Hide();                               // slot 5
};
struct PlaceDescriptor {
    PlaceVector position;                              // 0x00
    PlaceIndexList *destinations, *links;               // 0x0c, 0x10
    String function;                                   // 0x14
    PlaceVector propPosition;                          // 0x18
    void *frame;                                       // 0x24
    PlaceVector propScale;                             // 0x28
    PlaceDisplayProp *prop;                            // 0x34
    int state, kind, id;
    PlaceDescriptor();
    ~PlaceDescriptor();
};

PlaceDescriptor::PlaceDescriptor()
{
    position = PlaceVector(0.0f, 0.0f, 0.0f);
    links = 0;
    destinations = 0;
    prop = 0;
}

PlaceDescriptor::~PlaceDescriptor()
{
    if (destinations)
        delete destinations;
    if (links)
        delete links;
    if (prop) {
        prop->Hide();
        delete prop;
    }
}
