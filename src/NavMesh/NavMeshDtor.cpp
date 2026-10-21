// 0x47af00: destroy owned mesh arrays and spatial tree.
#include "NavMesh/NavMesh.h"
NavMesh::~NavMesh()
{
    FreeArrays();
    FreeTree();
}
