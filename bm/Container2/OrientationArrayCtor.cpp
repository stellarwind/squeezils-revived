// 0x46a230: copy a sequence of direction/up pairs into a new array.
#define CONTAINER_DEFINITIONS
#include "Container/ContainerArray.h"

struct _tag_PartOrientation {
    float directionX, directionY, directionZ;
    float upX, upY, upZ;
    _tag_PartOrientation(); // 0x46ed30
    ~_tag_PartOrientation(); // 0x41d790
    _tag_PartOrientation &operator=(const _tag_PartOrientation &other) {
        directionX = other.directionX;
        directionY = other.directionY;
        directionZ = other.directionZ;
        upX = other.upX;
        upY = other.upY;
        upZ = other.upZ;
        return *this;
    }
};

template class Array<_tag_PartOrientation>;
