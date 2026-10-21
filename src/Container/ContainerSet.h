// BufferObject, PrimitiveArray<T,ID> and Set<T,ID> of the game (see META/CONTAINERS.md).
#pragma once
#if defined(SQZ_INTEGRATION) && !defined(BUFFEROBJECT_SIGNATURES)
#define BUFFEROBJECT_SIGNATURES
#endif
#include "Container/ContainerArray.h"

// The save/load interface (vtable 0x49d52c): four pure virtuals. Their implementations are not reconstructed.
#ifdef BUFFEROBJECT_SIGNATURES
// The real slot signatures (slots 2 and 3 take the stream: "ret 4"). Declaring them changes the code generated for unrelated
// functions of other units (seen 2026-10-09: Table::RemoveAt, IniBackOverBlanks, GoodieList::Load), so only a unit that
// implements a BufferObject (Inventory) defines the macro.
class BufferObject {
public:
    virtual const char *TypeName() = 0;          // slot 0
    virtual BufferObject *Create() = 0;          // slot 1
    virtual void Write(void *stream) = 0;        // slot 2
    virtual void Read(void *stream) = 0;         // slot 3
};
#else
class BufferObject {
public:
    virtual void SaveSlot0() = 0;
    virtual void SaveSlot1() = 0;
    virtual void SaveSlot2() = 0;
    virtual void SaveSlot3() = 0;
};
#endif

// Array plus the BufferObject at +0x10; no members of its own.
template <class T, int ID>
class PrimitiveArray : public Array<T>, public BufferObject {
public:
    PrimitiveArray() {}
    PrimitiveArray(int n, const T *src) : Array<T>(n, src) {}
#ifdef BUFFEROBJECT_SIGNATURES
    virtual const char *TypeName();
    virtual BufferObject *Create();
    virtual void Write(void *stream);
    virtual void Read(void *stream);
#else
    virtual void SaveSlot0();
    virtual void SaveSlot1();
    virtual void SaveSlot2();
    virtual void SaveSlot3();
#endif
};

template <class T>
inline int Same(const T &a, const T &b)
{
    return !(a < b) && !(b < a);
}

// Sorted array of unique keys (case-insensitive order for String).
template <class T, int ID>
class Set : public PrimitiveArray<T, ID> {
public:
#ifdef SET_DEFAULT_CTOR_OUTOFLINE
    Set();
#endif
    virtual int Insert(const T &v, int hint);          // slot 1
    virtual void Replace(int i, const T &v);           // slot 3
    virtual int Contains(const T &v);                  // slot 6
    virtual int IndexOf(const T &v);                   // slot 7
    virtual void RemoveKey(const T &v);                // slot 8
    ~Set();                                            // 2-vtable complete dtor
#ifdef SET_CREATE
    virtual BufferObject *Create();                    // BufferObject slot 1: a fresh empty Set (Container2/PrimBuf.h)
#endif
#ifdef SET_INSERT_EXPERIMENT
    int Search(const T &v);                            // inline binary search: where v is or belongs
#endif
};

#ifdef CONTAINER_DEFINITIONS

#ifdef SET_DTOR
template <class T, int ID>
Set<T, ID>::~Set()
{
}
#endif

#ifdef SET_INSERT_EXPERIMENT
// Set::Insert (0x462380) is not matched yet (about 270 lines off); it is only compiled where this macro is defined, because
// having it in a unit changes the code generated for the neighbouring functions.
template <class T, int ID>
inline int Set<T, ID>::Search(const T &v)
{
    int lo = 0;
    int hi = count;
    if (hi == 0)
        return 0;
    while (lo != hi) {
        if (hi - lo == 1) {
            if (!(data[lo] < v))
                hi = lo;
            return hi;
        }
        int mid = (lo + hi) >> 1;
        if (data[mid] < v)
            lo = mid;
        else
            hi = mid;
    }
    return lo;
}

template <class T, int ID>
int Set<T, ID>::Insert(const T &v, int hint)
{
    if (hint != -1) {
        if (hint != 0) {
            if (hint == count) {
                if (data[hint - 1] < v)
                    return Array<T>::Insert(v, -1);
            } else if (data[hint - 1] < v && v < data[hint]) {
                return Array<T>::Insert(v, hint);
            } else if (Same(data[hint], v)) {
                data[hint] = v;
                return hint;
            }
        } else {
            if (count == 0 || v < data[0])
                return Array<T>::Insert(v, 0);
            if (Same(v, data[0])) {
                data[0] = v;
                return 0;
            }
        }
    }
    int pos = Search(v);
    if (pos < count && Same(Get(pos), v)) {
        data[pos] = v;
        return pos;
    }
    return Array<T>::Insert(v, pos);
}
#endif

template <class T, int ID>
void Set<T, ID>::Replace(int i, const T &v)
{
    Same(Get(i), v);
    data[i] = v;
}

template <class T, int ID>
int Set<T, ID>::Contains(const T &v)
{
    return IndexOf(v) != -1;
}

template <class T, int ID>
int Set<T, ID>::IndexOf(const T &v)
{
    int lo = 0;
    int hi = count;
    int pos;
    if (hi == 0) {
        pos = 0;
    } else {
        while (lo != hi) {
            if (hi - lo == 1) {
                if (!(data[lo] < v))
                    hi = lo;
                break;
            }
            int mid = (lo + hi) >> 1;
            if (data[mid] < v)
                lo = mid;
            else
                hi = mid;
        }
        pos = hi;
    }
    if (count > pos && !(v < data[pos]) && !(data[pos] < v))
        return pos;
    return -1;
}

template <class T, int ID>
void Set<T, ID>::RemoveKey(const T &v)
{
    int i = IndexOf(v);
    if (i != -1)
        Remove(i);
}
#endif

