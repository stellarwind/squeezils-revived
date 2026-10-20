// Array<T>::Data (byte-identical across every instantiation).
#include "ArrayData/ArrayData.h"

int *ArrayData::Data()
{
    return data;
}
