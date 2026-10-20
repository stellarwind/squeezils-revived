// Table<K,V,VArray,ID> and BufferObjectTable of the game (see META/CONTAINERS.md).
#pragma once
#include "Container/ContainerSet.h"

// Key -> value map: a sorted key Set and a parallel value array. No base class; keys at +4, values at +0x18 (PrimitiveArray values).
template <class K, class V, class VArray, int ID>
class Table {
public:
    Table();
    ~Table();
    virtual int Insert(const K &k, const V &v, int hint);   // slot 0
    virtual int Contains(const K &k);                       // slot 1
    virtual int IndexOf(const K &k);                        // slot 2
    virtual void RemoveAt(int i);                           // slot 3
    virtual void RemoveKey(const K &k);                     // slot 4
    V At(int i);                                            // non-virtual: values.At(i) (0x40f2a0 for String)
    V Get(const K &k);                                      // non-virtual: no check for a missing key (0x462cc0)
    K KeyAt(int i);                                         // non-virtual: keys.Get(i) (0x40ee50, 0x44de90 ...)

    Set<K, ID> keys;
    VArray values;
};

#ifdef CONTAINER_DEFINITIONS

template <class K, class V, class VArray, int ID>
Table<K, V, VArray, ID>::Table()
{
}

template <class K, class V, class VArray, int ID>
Table<K, V, VArray, ID>::~Table()
{
}

template <class K, class V, class VArray, int ID>
int Table<K, V, VArray, ID>::Insert(const K &k, const V &v, int hint)
{
    int had = keys.Contains(k);
    int i = keys.Insert(k, hint);
    if (had)
        values.Replace(i, v);
    else
        values.Insert(v, i);
    return i;
}

template <class K, class V, class VArray, int ID>
int Table<K, V, VArray, ID>::Contains(const K &k)
{
    return keys.Contains(k);
}

template <class K, class V, class VArray, int ID>
int Table<K, V, VArray, ID>::IndexOf(const K &k)
{
    return keys.IndexOf(k);
}

template <class K, class V, class VArray, int ID>
void Table<K, V, VArray, ID>::RemoveAt(int i)
{
    keys.Remove(i);
    values.Remove(i);
}

template <class K, class V, class VArray, int ID>
void Table<K, V, VArray, ID>::RemoveKey(const K &k)
{
    int i = keys.IndexOf(k);
    if (i != -1)
        RemoveAt(i);
}

template <class K, class V, class VArray, int ID>
V Table<K, V, VArray, ID>::At(int i)
{
    return values.At(i);
}

template <class K, class V, class VArray, int ID>
V Table<K, V, VArray, ID>::Get(const K &k)
{
    return At(keys.IndexOf(k));
}

template <class K, class V, class VArray, int ID>
K Table<K, V, VArray, ID>::KeyAt(int i)
{
    return keys.Get(i);
}
#endif

// Table plus the save/load interface at +0x2c.
template <class K, class V, class VArray, int ID>
class BufferObjectTable : public Table<K, V, VArray, ID>, public BufferObject {
public:
    BufferObjectTable() {}
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

