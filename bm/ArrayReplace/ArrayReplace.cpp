// Array<T>::Replace (byte-identical across every instantiation).
#include "ArrayReplace/ArrayReplace.h"

void ArrayReplace::Replace(int i, const int &v)
{
    data[i] = v;
}
