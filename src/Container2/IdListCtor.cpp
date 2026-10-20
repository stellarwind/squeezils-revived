// SQZ.EXE 0x45eef0: Set<int,10>, the simulation's integer ID list.
// Its Array and PrimitiveArray constructors are inline in this unit.
#include "compat41.h"

struct IdArray {
    virtual int Get(int i);
    virtual int Insert(const int &value, int at);
    virtual void Remove(int i);
    virtual void Replace(int i, const int &value);
    virtual int *Data();
    virtual int Grow(int extra);
    int count, capacity;
    int *data;
    IdArray() { count = 0; capacity = 2; data = new int[capacity]; }
    ~IdArray();                            // 0x45d900
};

struct IdBufferObject {
    virtual void slot0() = 0;
    virtual void slot1() = 0;
    virtual void slot2() = 0;
    virtual void slot3() = 0;
};

struct IdPrimitiveArray : IdArray, IdBufferObject {
    IdPrimitiveArray() {}
    ~IdPrimitiveArray();                   // 0x46ebf0
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
};

struct IdList : IdPrimitiveArray {
    IdList();
    virtual int Insert(const int &value, int hint);
    virtual void Replace(int i, const int &value);
    virtual int Contains(const int &value);
    virtual int IndexOf(const int &value);
    virtual void RemoveKey(const int &value);
};

IdList::IdList() {}
