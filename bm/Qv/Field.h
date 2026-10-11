// QvLib field classes (SQZ.EXE 0x488b90-0x48b880), VC++ 4.1 dialect. RTTI-less (the library was built without -GR).
#pragma once
#include "compat41.h"

class QvField {
public:
    QvField() {}                       // vtbl 0x4a1570 store is inlined into the derived constructors
    virtual ~QvField();                // 0x488b90: mov [ecx],0x4a1570; ret  (out of line)
    int flags;                         // +4
    int owner;                         // +8
    void SetOwner(void *n);            // 0x488ba0: flags |= 1; owner = n
};

class QvFieldA : public QvField {      // vtbl 0x4a1578
public:
    QvFieldA();                        // 0x489570
    virtual ~QvFieldA();               // 0x489600, sdd 0x4895e0
};

class QvFieldB : public QvField {      // vtbl 0x4a1580
public:
    QvFieldB();                        // 0x489650
    virtual ~QvFieldB();               // 0x4896e0, sdd 0x4896c0
    int v0;                            // +0xc
    int v1;                            // +0x10
};
