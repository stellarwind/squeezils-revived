// NavMesh sizeSquared getter (0x47af20): the mesh's largest squared distance of a point from the origin.
// Kept in its own TU so the register-frozen Outcode (0x47d4f0) is not perturbed.
struct NavMeshGet {
    char  pad[0x2020];
    float sizeSquared;   // 0x2020

    float GetSizeSquared();   // 0x47af20
};

float NavMeshGet::GetSizeSquared()
{
    return sizeSquared;
}
