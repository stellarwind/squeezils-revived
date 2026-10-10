// Container.h: the game's container templates. Reconstructed from SQZ.EXE 0x45d8d0-0x46f800, where every instantiation has
// its own copy of each method (META/CONTAINERS.md lists the addresses). The class names are the original's (RTTI).
//
//   Array<T>                      growable array: count, capacity and a heap block of elements
//   PrimitiveArray<T, ID>         an Array that can also be saved and loaded (BufferObject); ID is the save type tag
//   Set<T, ID>                    a PrimitiveArray kept sorted and free of duplicates (binary search)
//   Table<K, V, VArray, ID>       a map: a Set of keys and a parallel array of values
//   BufferObjectTable<...>        a Table that can be saved and loaded
//
// Original code uses these objects too: it calls their virtual methods and runs dynamic_cast on them, which reads the RTTI
// next to the vtable. So every constructor here ends by pointing the object at the original vtable(s) (OriginalVtables in
// Original.h), as the original constructors end by storing theirs. The virtual methods are declared in the original slot
// order, so a virtual call made here goes through the original vtable to the same slot.
//
// Get returns a String by value, and GCC passes a by-value class result differently from MSVC (result pointer in ecx, `this`
// on the stack). So nothing here calls Get through the vtable; code that needs an element uses the non-virtual At (the
// original's own helper, 0x40f310 for String). No class overrides Get.
#pragma once
#include <stddef.h>
#include <string.h>
#include <new>
#include <type_traits>
#include "GameString.h"
#include "Original.h"

// The save/load interface (abstract: type name, create, write, read; vtable 0x49d52c). Not on the INI path and not
// reconstructed yet, so only its vtable pointer is modelled; the slots are always the original's.
struct BufferObject {
    enum { SLOT_TYPE_NAME = 0, SLOT_CREATE = 1, SLOT_WRITE = 2, SLOT_READ = 3 };
    void **vtbl;
};

// Points obj at its class's original vtables. Called last in each constructor.
template<class C> inline void UseOriginalVtables(C *obj)
{
    void *primary = (void *)OriginalVtables<C>::primary;
    memcpy((void *)obj, &primary, sizeof primary);
    if constexpr (std::is_base_of<BufferObject, C>::value)
        static_cast<BufferObject *>(obj)->vtbl = (void **)OriginalVtables<C>::bufferObject;
}

// new T[n] and delete[] as the original compiles them, on the game's heap (original code frees these blocks too): element
// types with a destructor (String) get a count word in front, plain ones (int, pointers) do not.
template<class T> T *NewArray(int n)
{
    if constexpr (std::is_trivially_destructible<T>::value) {
        return (T *)Operator_New(n * sizeof(T));
    } else {
        int *block = (int *)Operator_New(n * sizeof(T) + sizeof(int));
        if (!block) return NULL;
        *block = n;
        T *p = (T *)(block + 1);
        for (int i = 0; i < n; i++) new (p + i) T();
        return p;
    }
}

template<class T> void DeleteArray(T *p)
{
    if constexpr (std::is_trivially_destructible<T>::value) {
        Operator_Delete(p);
    } else if (p) {
        int n = ((int *)p)[-1];
        for (int i = n - 1; i >= 0; i--) p[i].~T();
        Operator_Delete((int *)p - 1);
    }
}

// The containers themselves live on the game's heap too (the original's global operator new is the game's).
struct GameHeapObject {
    static void *operator new(size_t n) noexcept { return Operator_New(n); }
    static void operator delete(void *p) { Operator_Delete(p); }
};

// The same two operators for a class that cannot derive from GameHeapObject because its first member already does: an empty
// base may not share an address with another base of its own type, so the members would move.
#define GAME_HEAP_OPERATORS \
    static void *operator new(size_t n) noexcept { return Operator_New(n); } \
    static void operator delete(void *p) { Operator_Delete(p); }

template<class T> class Array : public GameHeapObject {
public:
    Array();                                          // String @45e1b0, int @45d8d0, pointers inlined: empty, room for 2
    Array(const Array &o);                            // String @45e0e0, float @45dc70: room for o.capacity, copies o's elements
    Array(int n, const T *src);                       // float @45dcd0, Vector3f @45e6c0: n elements copied from src, room for n (2 when n is 0)
    ~Array();                                         // String @45e250, int @45d900 (not virtual)

    virtual T    Get(int i);                          // slot 0  String @40ec10, int @40ec80
    virtual int  Insert(const T &v, int at = -1);     // slot 1  String @45e280, int @45d920: at -1 appends; returns the index used
    virtual void RemoveAt(int i);                     // slot 2  String @45e300, int @45d990
    virtual void Replace(int i, const T &v);          // slot 3  String @45e340, int @45d9d0
    virtual T   *Data();                              // slot 4  String @45e360, int @45d9f0
    virtual int  Grow(int extra);                     // slot 5  String @45e370, int @45da00: 1 when it made room

    T At(int i) const { return data[i]; }             // String @40f310, inlined elsewhere

    int count;          // 0x4
    int capacity;       // 0x8
    T  *data;           // 0xc
};

template<class T, int ID> class PrimitiveArray : public Array<T>, public BufferObject {
public:
    PrimitiveArray() { UseOriginalVtables(this); }   // inlined everywhere in the original
    PrimitiveArray(const PrimitiveArray &o) : Array<T>(o) { UseOriginalVtables(this); }   // inlined (float: the Array copy @45dc70, then the vtables)
    PrimitiveArray(int n, const T *src) : Array<T>(n, src) { UseOriginalVtables(this); }
};

template<class T, int ID> class Set : public PrimitiveArray<T, ID> {
public:
    Set() { UseOriginalVtables(this); }              // inlined in the Table constructors

    int  Insert(const T &v, int hint = -1) override;   // slot 1  ID 23 @462380, 50 @46d640, 51 @46e2d0: sorted; an equal key is overwritten
    void Replace(int i, const T &v) override;          // slot 3  ID 23 @4627e0, 50 @46daa0, 51 @46e740
    virtual int  Contains(const T &v);                 // slot 6  ID 23 @4626f0, 50 @46d9b0, 51 @46e650
    virtual int  IndexOf(const T &v);                  // slot 7  ID 23 @462710, 50 @46d9d0, 51 @46e670: -1 when missing
    virtual void RemoveKey(const T &v);                // slot 8  ID 23 @4627c0, 50 @46da80, 51 @46e720

    using Array<T>::count;
    using Array<T>::data;

private:
    int Search(const T &v) const;                      // first position not less than v (inlined in the original)
    static bool Same(const T &a, const T &b) { return !(a < b) && !(b < a); }
};

template<class K, class V, class VArray, int ID> class Table : public GameHeapObject {
public:
    Table() { UseOriginalVtables(this); }            // String values @4629a0, IniFileSection* values @46d2c0

    virtual int  Insert(const K &k, const V &v, int hint = -1);   // slot 0: adds or replaces; returns the index
    virtual int  Contains(const K &k);                            // slot 1
    virtual int  IndexOf(const K &k);                             // slot 2: -1 when missing
    virtual void RemoveAt(int i);                                 // slot 3
    virtual void RemoveKey(const K &k);                           // slot 4

    V Get(const K &k) { return GetAt(keys.IndexOf(k)); }          // not checked: callers test Contains first
    V GetAt(int i) { return values.At(i); }

    Set<K, ID> keys;     // 0x04 sorted
    VArray values;       // 0x18 values.data[i] belongs to keys.data[i]
};

template<class K, class V, class VArray, int ID>
class BufferObjectTable : public Table<K, V, VArray, ID>, public BufferObject {
public:
    BufferObjectTable() { UseOriginalVtables(this); }   // inlined
};

// Our names for the two lists the game passes around most (the INI reader returns them).
typedef PrimitiveArray<String, 3> StringList;
typedef PrimitiveArray<int, 1> IntList;

// ---- Array ----

template<class T> Array<T>::Array() : count(0), capacity(2), data(NewArray<T>(2))
{
    UseOriginalVtables(this);
}

template<class T> Array<T>::Array(const Array &o) : count(o.count), capacity(o.capacity), data(NewArray<T>(o.capacity))
{
    UseOriginalVtables(this);
    for (int i = 0; i < count; i++)
        data[i] = o.data[i];
}

template<class T> Array<T>::Array(int n, const T *src) : count(n), capacity(n ? n : 2), data(NewArray<T>(n ? n : 2))
{
    UseOriginalVtables(this);
    for (int i = 0; i < count; i++)
        data[i] = src[i];
}

template<class T> Array<T>::~Array()
{
    DeleteArray(data);
}

template<class T> T Array<T>::Get(int i)
{
    return data[i];
}

template<class T> int Array<T>::Insert(const T &v, int at)
{
    Grow(1);
    if (at == -1) {
        data[count] = v;
        return count++;
    }
    for (int i = count; i > at; i--)
        data[i] = data[i - 1];
    data[at] = v;
    count++;
    return at;
}

template<class T> void Array<T>::RemoveAt(int i)
{
    for (; i < count - 1; i++)
        data[i] = data[i + 1];
    count--;                            // the old last slot keeps its value (and its String reference)
}

template<class T> void Array<T>::Replace(int i, const T &v)
{
    data[i] = v;
}

template<class T> T *Array<T>::Data()
{
    return data;
}

template<class T> int Array<T>::Grow(int extra)
{
    if (count + extra < capacity) return 0;
    capacity += capacity >> 1;          // once, by half: enough for the one element Insert adds
    T *grown = NewArray<T>(capacity);
    for (int i = 0; i < count; i++)
        grown[i] = data[i];
    DeleteArray(data);
    data = grown;
    return 1;
}

// ---- Set ----

template<class T, int ID> int Set<T, ID>::Search(const T &v) const
{
    int lo = 0, hi = count;
    while (lo != hi) {
        if (hi - lo == 1)
            return data[lo] < v ? hi : lo;
        int mid = (lo + hi) >> 1;
        if (data[mid] < v) lo = mid;
        else hi = mid;
    }
    return lo;
}

template<class T, int ID> int Set<T, ID>::Insert(const T &v, int hint)
{
    // The hint is used when v belongs exactly there (callers adding keys in order pass the end); otherwise search.
    if (hint != -1) {
        if (hint == 0) {
            if (count == 0 || v < data[0])
                return Array<T>::Insert(v, 0);
            if (Same(v, data[0])) {
                data[0] = v;
                return 0;
            }
        } else if (hint == count) {
            if (data[hint - 1] < v)
                return Array<T>::Insert(v, -1);
        } else if (data[hint - 1] < v && v < data[hint]) {
            return Array<T>::Insert(v, hint);
        } else if (Same(data[hint], v)) {
            data[hint] = v;
            return hint;
        }
    }
    int pos = Search(v);
    if (pos < count && Same(this->At(pos), v)) {
        data[pos] = v;
        return pos;
    }
    return Array<T>::Insert(v, pos);
}

template<class T, int ID> void Set<T, ID>::Replace(int i, const T &v)
{
    // The original first compares the old element with v and ignores the result (a check compiled without its report).
    data[i] = v;
}

template<class T, int ID> int Set<T, ID>::Contains(const T &v)
{
    return IndexOf(v) != -1;
}

template<class T, int ID> int Set<T, ID>::IndexOf(const T &v)
{
    int pos = Search(v);
    if (pos < count && Same(v, data[pos]))
        return pos;
    return -1;
}

template<class T, int ID> void Set<T, ID>::RemoveKey(const T &v)
{
    int i = IndexOf(v);
    if (i != -1)
        this->RemoveAt(i);
}

// ---- Table ----

template<class K, class V, class VArray, int ID> int Table<K, V, VArray, ID>::Insert(const K &k, const V &v, int hint)
{
    int had = keys.Contains(k);
    int i = keys.Insert(k, hint);
    if (had)
        values.Replace(i, v);
    else
        values.Insert(v, i);
    return i;
}

template<class K, class V, class VArray, int ID> int Table<K, V, VArray, ID>::Contains(const K &k)
{
    return keys.Contains(k);
}

template<class K, class V, class VArray, int ID> int Table<K, V, VArray, ID>::IndexOf(const K &k)
{
    return keys.IndexOf(k);
}

template<class K, class V, class VArray, int ID> void Table<K, V, VArray, ID>::RemoveAt(int i)
{
    keys.RemoveAt(i);
    values.RemoveAt(i);
}

template<class K, class V, class VArray, int ID> void Table<K, V, VArray, ID>::RemoveKey(const K &k)
{
    int i = keys.IndexOf(k);
    if (i != -1)
        RemoveAt(i);
}
