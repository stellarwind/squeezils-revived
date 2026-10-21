// QvLib node model (VC++ 4.1 dialect), reconstructed from SQZ.EXE 0x47e660-0x488ba0. Follows the structure of the public
// QvLib 1.0 macros (QV_NODE_SOURCE / ADD_FIELD) but is written from the disassembly.
#pragma once
#include "compat41.h"
#include "Qv/Field.h"

struct QvPList {                      // 0xc bytes: ctor 0x482a60 (zeroes), dtor 0x482a80 (delete items)
    void *items; int count; int cap;
    QvPList();
    ~QvPList();
};

class QvNode;
class QvName;
// SIZE 0x18
struct QvFieldData {                  // 0x18 bytes
    QvPList fields; // 0x00
    QvPList enums;  // 0x0c
    void addField(QvNode *node, const char *name, QvField *f);   // 0x480720
    void addEnumValue(const char *type, const char *name, int value); // 0x480840
    void getEnumData(const char *type, int &count, const int *&values,
                     const QvName *&names); // 0x480b50
};

class QvNode {                        // 0x18 bytes, vtbl 0x4a11d8
public:
    QvNode();                          // 0x47e660
    virtual ~QvNode();                 // 0x47e710
    int unk4, unk8;
    int isBuiltIn;                     // +0xc
    void *name;                        // +0x10
    int refCount;                      // +0x14
};

class QvSFVec3f : public QvField {     // 0x18 bytes; ctor 0x489ba0, dtor 0x489c20
public:
    QvSFVec3f();
    ~QvSFVec3f();
    float x, y, z;
};
