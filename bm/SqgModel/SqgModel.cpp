// SqgModelConstruct (0x4719d0): clear the 0x2c-byte object, return it.
#include "SqgModel/SqgModel.h"

SqgModel *SqgModel::SqgModelConstruct()
{
    textureCount = 0;
    textureNames = 0;
    vertexCount = 0;
    vertices = 0;
    faceCount = 0;
    indexCount = 0;
    indices = 0;
    groupCount = 0;
    groups = 0;
    deltaListCount = 0;
    deltaLists = 0;
    return this;
}
