// NavBox outcode test (SQZ.EXE 0x47d4f0): bitmask of which faces of a half-open box [min,max) the point is outside.
//   bit 0: x >= xMax, 1: x < xMin, 2: y >= yMax, 3: y < yMin, 4: z >= zMax, 5: z < zMin.
// In the original this is a free function: FindLeaf (0x47d560) pushes the point and the box's six floats and calls it.
int Outcode(float *p, float xMin, float xMax, float yMin, float yMax, float zMin, float zMax)
{
    int code = 0;
    if (!(p[0] < xMax)) code |= 1;
    if (p[0] < xMin)    code |= 2;
    if (!(p[1] < yMax)) code |= 4;
    if (p[1] < yMin)    code |= 8;
    if (!(p[2] < zMax)) code |= 16;
    if (p[2] < zMin)    code |= 32;
    return code;
}
