// 0x46a6c0: initialize the table's array of PrimitiveArray<float,2> pointers.
#include "Container/ContainerSet.h"

Array<PrimitiveArray<float, 2> *>::Array(int n, PrimitiveArray<float, 2> *const *src)
{
    count = n;
    capacity = n == 0 ? 2 : n;
    data = new PrimitiveArray<float, 2> *[capacity];
    for (int i = 0; i < count; ++i)
        data[i] = *src++;
}
