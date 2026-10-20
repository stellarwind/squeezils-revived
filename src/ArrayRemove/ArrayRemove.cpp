// Array<T>::Remove (byte-identical across every instantiation).
#include "ArrayRemove/ArrayRemove.h"

void ArrayRemove::Remove(int i)
{
    int j;
    for (j = i; j < count - 1; j++)
        data[j] = data[j + 1];
    count = count - 1;
}
