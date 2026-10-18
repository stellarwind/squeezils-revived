// NavSurfaceList::FaceType, SQZ.EXE 0x40f380.
#include "NavSurfaceList.h"

struct NavFace {
    int vertex[3];
    float normal[3];
    int neighbour[3];
    int type;
};

struct NavMesh {
    char pad[0x0c];
    NavFace *faces;
};

int NavSurfaceList::FaceType(const Contact *contact)
{
    NavMesh *mesh = FaceMesh(contact);
    int index = contact->second;
    return mesh->faces[index].type;
}
