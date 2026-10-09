// Array<T> of the game (see META/CONTAINERS.md), VC++ 4.1 dialect. Member definitions appear only when CONTAINER_DEFINITIONS is defined.
#pragma once
#include "compat41.h"

template <class T>
class Array {
public:
    Array();
    ~Array();
    virtual T Get(int i);                              // slot 0
    virtual int Insert(const T &v, int at);            // slot 1
    virtual void Remove(int i);                        // slot 2
    virtual void Replace(int i, const T &v);           // slot 3
    virtual T *Data();                                 // slot 4
    virtual int Grow(int extra);                       // slot 5
    T At(int i);                                       // non-virtual get by value (0x40f310 for String)

    int count;       // +4
    int capacity;    // +8
    T *data;         // +0xc
};

#ifdef CONTAINER_DEFINITIONS

template <class T>
Array<T>::Array()
{
    count = 0;
    capacity = 2;
    data = new T[capacity];
}

template <class T>
Array<T>::~Array()
{
    delete [] data;
}

template <class T>
int Array<T>::Insert(const T &v, int at)
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

template <class T>
void Array<T>::Remove(int i)
{
    int j;
    for (j = i; j < count - 1; j++)
        data[j] = data[j + 1];
    count = count - 1;
}

template <class T>
void Array<T>::Replace(int i, const T &v)
{
    data[i] = v;
}

template <class T>
T *Array<T>::Data()
{
    return data;
}

template <class T>
T Array<T>::Get(int i)
{
    return data[i];
}

template <class T>
int Array<T>::Grow(int extra)
{
    if (count + extra >= capacity) {
        capacity += capacity >> 1;
        T *fresh = new T[capacity];
        int i;
        for (i = 0; i < count; i++)
            fresh[i] = data[i];
        delete [] data;
        data = fresh;
        return 1;
    }
    return 0;
}
#endif

