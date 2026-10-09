// Array<T>::Get (byte-identical across every 4-byte-POD instantiation).
#include "ArrayGet/ArrayGet.h"

int ArrayGet::Get(int i)
{
    return data[i];
}
