// 0x46a400 (ret 8): Array<_tag_PartOrientation>::Insert (24-byte element, field-wise copy).
#define CONTAINER_DEFINITIONS
#include "Container/ContainerArray.h"
struct _tag_PartOrientation {
    int a, b, c, d, e, f;
    _tag_PartOrientation& operator=(const _tag_PartOrientation& o) { a = o.a; b = o.b; c = o.c; d = o.d; e = o.e; f = o.f; return *this; }
};
template class Array<_tag_PartOrientation>;
