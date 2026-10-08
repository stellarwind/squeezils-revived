// NavMesh: the walkable geometry of one surface (see NavMesh.h). Reconstructed from SQZ.EXE 0x47aec0-0x47d5f0:
// the object's memory (0x47aec0-0x47b180), the two queries (0x47b180 Trace, 0x47b490 Move with the pieces after it), the box tree
// tests (0x47d4f0, 0x47d560, 0x47c2a0) and the .nav file reader (0x47c630-0x47d4f0).
#include "NavMesh.h"
#include "SqueezilSoulInternal.h"
#include "Original.h"
#include "Simulation.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static Vector3f Vec(const float *p) { return Vector3f(p[0], p[1], p[2]); }
static void Put(float *to, const Vector3f &v) { to[0] = v.x; to[1] = v.y; to[2] = v.z; }
// The order the game adds the products in: z, then y, then x
static float DotZYX(const Vector3f &a, const Vector3f &b) { return a.z * b.z + a.y * b.y + a.x * b.x; }

// ---- the pieces ----

// @0047ad20
NavFace::NavFace() : normal(0.0f, 0.0f, 1.0f)
{
    vertex[0] = vertex[1] = vertex[2] = 0;
    neighbour[0] = neighbour[1] = neighbour[2] = -1;
    type = 0;
}

// @0047ade0
NavBox::NavBox() : xMin(0), xMax(0), yMin(0), yMax(0), zMin(0), zMax(0), faceCount(0), faces(NULL)
{
    for (int i = 0; i < 8; i++)
        child[i] = NULL;
}

// @0047ae10
NavBox::~NavBox()
{
    FreeFaces();
    for (int i = 0; i < 8; i++) {
        if (child[i]) {
            delete child[i];
            child[i] = NULL;
        }
    }
}

// @0047ae50
int NavBox::AllocFaces(unsigned count)
{
    FreeFaces();
    if (count) {
        faces = (int *)Operator_New(count * sizeof(int));
        if (!faces)
            return 0;
        faceCount = count;
    }
    return 1;
}

// @0047ae90
void NavBox::FreeFaces()
{
    if (faces)
        Operator_Delete(faces);
    faceCount = 0;
    faces = NULL;
}

// @0047d4f0
unsigned NavBox::Outcode(const Vector3f &p) const
{
    unsigned code = 0;
    if (!(p.x < xMax)) code |= 1;
    if (p.x < xMin)    code |= 2;
    if (!(p.y < yMax)) code |= 4;
    if (p.y < yMin)    code |= 8;
    if (!(p.z < zMax)) code |= 16;
    if (p.z < zMin)    code |= 32;
    return code;
}

// ---- the object's memory ----

// @0047aec0
NavMesh::NavMesh()
{
    pointCount = 0;
    points = NULL;
    faceCount = 0;
    faces = NULL;
    tree = NULL;
    unknown201c = 0;
    unknown2024[0] = unknown2024[1] = unknown2024[2] = 0;
    sizeSquared = 0.0f;
}

// @0047af00
NavMesh::~NavMesh()
{
    FreePoints();
    FreeFaces();
    FreeTree();
}

// @0047af30 Gives the mesh room for `count` points (the old ones go). The count only changes when it worked.
int NavMesh::AllocPoints(unsigned count)
{
    FreePoints();
    if (count) {
        points = NewArray<Vector3f>(count);
        if (!points)
            return 0;
    }
    pointCount = count;
    return 1;
}

// @0047aff0
int NavMesh::AllocFaces(unsigned count)
{
    FreeFaces();
    if (count) {
        faces = NewArray<NavFace>(count);
        if (!faces)
            return 0;
    }
    faceCount = count;
    return 1;
}

// @0047b0b0
void NavMesh::FreePoints()
{
    DeleteArray(points);
    pointCount = 0;
    points = NULL;
}

// @0047b0f0
void NavMesh::FreeFaces()
{
    DeleteArray(faces);
    faceCount = 0;
    faces = NULL;
}

// @0047b150
void NavMesh::FreeTree()
{
    if (tree) {
        delete tree;
        tree = NULL;
    }
}

// ---- looking for a face ----

// @0047d560 The leaf box that holds the point. The original loops for ever when the mesh has no tree; this one finds nothing.
NavBox *NavMesh::FindLeaf(const Vector3f &point, NavBox *box)
{
    if (!box)
        box = tree;
    if (!box)
        return NULL;
    if (box->Outcode(point) != 0)
        return NULL;
    bool hasChildren = false;
    for (int i = 0; i < 8; i++) {
        if (box->child[i]) {
            hasChildren = true;
            NavBox *leaf = FindLeaf(point, box->child[i]);
            if (leaf)
                return leaf;
        }
    }
    return hasChildren ? NULL : box;
}

// @0047c2a0 The point is taken to be on the face's plane; it is inside when it is on the inner side of all three edges.
int NavMesh::PointInFace(const NavFace *face, const Vector3f &point) const
{
    Vector3f a = points[face->vertex[0]];
    Vector3f b = points[face->vertex[1]];
    Vector3f c = points[face->vertex[2]];
    Vector3f ab = b - a;
    Vector3f bc = c - b;
    Vector3f ca = a - c;
    Vector3f inwardAB = Cross(face->normal, ab);
    Vector3f inwardBC = Cross(face->normal, bc);
    Vector3f inwardCA = Cross(face->normal, ca);
    Vector3f fromA = point - a;
    Vector3f fromB = point - b;
    Vector3f fromC = point - c;
    if (DotZYX(fromA, inwardAB) < 0.0f || DotZYX(fromB, inwardBC) < 0.0f || DotZYX(fromC, inwardCA) < 0.0f)
        return 0;
    return 1;
}

// @0047d850 Where the line from `from` to `to` meets the plane through `onPlane` with the given normal, as a fraction of the line
// (0 at `from`, 1 at `to`). Not when the line runs along the plane.
static int LineMeetsPlane(const Vector3f &from, const Vector3f &to, const Vector3f &onPlane, const Vector3f &normal, float *fraction)
{
    Vector3f line = to - from;
    Vector3f toPlane = onPlane - from;
    float along = toPlane.y * normal.y + toPlane.z * normal.z + toPlane.x * normal.x;
    float speed = normal.x * line.x + normal.y * line.y + normal.z * line.z;
    if (speed == 0.0f)
        return 0;
    *fraction = along / speed;
    return 1;
}

static int CompareHits(const void *a, const void *b)
{
    float ta = ((const NavMesh::Hit *)a)->t;
    float tb = ((const NavMesh::Hit *)b)->t;
    if (tb > ta)
        return -1;
    if (tb < ta)
        return 1;
    return 0;
}

// @0047b180
int NavMesh::Trace(const float *pos, const float *right, const float *up, const float *forward, const float *direction, float length,
                   float *hitPos, float *hitRight, float *hitUp, float *hitForward, int *face, const int *flags)
{
    Vector3f start = Vec(pos);
    Vector3f dir = Vec(direction);
    Vector3f end = start + dir * length;
    NavBox *box = FindLeaf(end, NULL);
    if (!box)
        return 0;

    // The faces that face the line and that it crosses, a little before the start to a little beyond the end
    int count = 0;
    for (unsigned i = 0; i < box->faceCount; i++) {
        NavFace *candidate = &faces[box->faces[i]];
        float facing = candidate->normal.z * dir.z + candidate->normal.y * dir.y + candidate->normal.x * dir.x;
        if (facing < 0.001f) {
            float t;
            if (LineMeetsPlane(start, end, points[candidate->vertex[0]], candidate->normal, &t)) {
                if (!(t >= -0.1f && t <= 1.5f))
                    continue;
                if (count >= 1024)
                    break;
                hits[count].face = box->faces[i];
                hits[count].t = t;
                count++;
            }
        }
    }
    qsort(hits, count, sizeof(Hit), CompareHits);

    for (int i = 0; i < count; i++) {
        Vector3f at = start + dir * (hits[i].t * length);
        NavFace *candidate = &faces[hits[i].face];
        if (PointInFace(candidate, at) && flags[candidate->type] != 0) {
            Put(hitPos, at);
            Put(hitUp, candidate->normal);
            Vector3f across = Cross(Vec(hitUp), Vec(forward));
            Put(hitRight, across);
            Vector3f unit = Vec(hitRight);
            Normalize(unit);
            Put(hitRight, unit);
            Vector3f ahead = Cross(Vec(hitRight), Vec(hitUp));
            Put(hitForward, ahead);
            *face = hits[i].face;
            return 1;
        }
    }
    return 0;
}

// ---- moving ----

static int g_moveDepth;     // @004c3df4 how deep Move has recursed into neighbour faces

// -1, 0 or +1 for a number clearly below, near or clearly above zero
static int Side(float v)
{
    if (v > 1e-5f)
        return 1;
    if (v < -1e-5f)
        return -1;
    return 0;
}

// @0047b490
int NavMesh::Move(const float *pos, const float *right, const float *up, const float *forward, const float *direction, float distance,
                  int face, float *newPos, float *newRight, float *newUp, float *newForward, float *newDirection, int *newFace,
                  const int *flags)
{
    if (g_moveDepth > 16)
        return 0;
    if (distance > -0.005f && distance < 0.005f) {      // nothing to walk: stay as we are
        Put(newPos, Vec(pos));
        Put(newRight, Vec(right));
        Put(newUp, Vec(up));
        Put(newForward, Vec(forward));
        Put(newDirection, Vec(direction));
        *newFace = face;
        return 1;
    }

    const NavFace *here = &faces[face];
    const Vector3f &normal = here->normal;

    // The way along the face: the direction with what points out of the face taken away
    Vector3f across = Cross(normal, Vec(direction));
    Normalize(across);
    Vector3f slide = Cross(across, normal);
    Vector3f target = Vec(pos) + slide * distance;

    Vector3f corner[3] = { points[here->vertex[0]], points[here->vertex[1]], points[here->vertex[2]] };
    Vector3f edge[3] = { corner[1] - corner[0], corner[2] - corner[1], corner[0] - corner[2] };
    Vector3f inward[3] = { Cross(normal, edge[0]), Cross(normal, edge[1]), Cross(normal, edge[2]) };
    int side[3];
    for (int i = 0; i < 3; i++)
        side[i] = Side(DotZYX(target - corner[i], inward[i]));

    if (side[0] != -1 && side[1] != -1 && side[2] != -1) {      // the target is on this face
        Put(newPos, target);
        Put(newRight, Vec(right));
        Put(newUp, Vec(up));
        Put(newForward, Vec(forward));
        Put(newDirection, slide);
        *newFace = face;
        return 1;
    }

    // It is beyond at least one edge. Where does the path cross each of them (as a part of the path, and as a part of the edge)?
    float along[3] = { 1000000.0f, 1000000.0f, 1000000.0f };
    float onEdge[3] = { 1000000.0f, 1000000.0f, 1000000.0f };
    Vector3f start = Vec(pos);
    for (int i = 0; i < 3; i++)
        if (side[i] == -1)
            ClosestLineParams(&start, &target, &corner[i], &corner[(i + 1) % 3], &along[i], &onEdge[i]);
    int crossed = 0;
    for (int i = 1; i < 3; i++)
        if (along[i] < along[crossed])
            crossed = i;
    Vector3f edgePoint = corner[crossed] + edge[crossed] * onEdge[crossed];

    int next = here->neighbour[crossed];
    if (next == -1 || flags[faces[next].type] == 0) {           // a wall: stop at the edge
        Put(newRight, Vec(right));
        Put(newUp, Vec(up));
        Put(newForward, Vec(forward));
        Put(newPos, edgePoint);
        Put(newDirection, slide);
        *newFace = -1;
        return 0;
    }

    // Over the edge: turn the frame of reference about the edge by the angle between the two faces
    const NavFace *there = &faces[next];
    Vector3f axis = edge[crossed];
    Normalize(axis);
    volatile float cosine = DotZYX(normal, there->normal);     // (rounded to a float before acos, as the original stores it; near 1 the angle depends on it)
    if (cosine > 1.0f)
        cosine = 1.0f;
    volatile float angle = (float)acos((double)cosine);
    float slope = there->normal.z * slide.z + there->normal.x * slide.x + there->normal.y * slide.y;
    if (distance < 0.0f)
        slope = -slope;
    if (slope < 0.0f)
        angle = -angle;
    float turn[16], frame[16], turned[16];
    RotationAbout(turn, axis, angle);
    SetRows(frame, Vec(right), Vec(up), Vec(forward), Vector3f(0.0f, 0.0f, 0.0f));
    MultiplyMatrix(turned, frame, turn);
    Vector3f nextRight(turned[0], turned[1], turned[2]);
    Vector3f nextUp(turned[4], turned[5], turned[6]);
    Vector3f nextForward(turned[8], turned[9], turned[10]);
    Vector3f nextDirection = TransformPoint(slide, turn);

    float remaining = distance - along[crossed] * distance;
    if (remaining <= -0.005f || remaining >= 0.005f) {          // more to walk: on into the next face
        g_moveDepth++;
        int result = Move(&edgePoint.x, &nextRight.x, &nextUp.x, &nextForward.x, &nextDirection.x, remaining, next, newPos, newRight,
                          newUp, newForward, newDirection, newFace, flags);
        g_moveDepth--;
        return result;
    }
    Put(newRight, nextRight);
    Put(newUp, nextUp);
    Put(newForward, nextForward);
    Put(newPos, edgePoint);
    Put(newDirection, nextDirection);
    *newFace = next;
    return 1;
}

// ---- reading a .nav file ----

static const char kAsciiHeader[] = "#Surface V1.1 ascii\n";
static const char kBinaryHeader[] = "#Surface V1.1 binary\n";

// @0047c690 / 0047c6f0 / 0047c770 One 16 bit number, 32 bit number or float of a binary file. (The original can swap the bytes
// of a file written the other way round, but the test for that always says no.)
static int ReadShort(FILE *f, unsigned short *out)
{
    unsigned short v;
    if (fread(&v, 2, 1, f) != 1)
        return 0;
    *out = v;
    return 1;
}

static int ReadInt(FILE *f, int *out)
{
    int v;
    if (fread(&v, 4, 1, f) != 1)
        return 0;
    *out = v;
    return 1;
}

static int ReadFloat(FILE *f, float *out)
{
    int bits;
    if (!ReadInt(f, &bits))
        return 0;
    memcpy(out, &bits, 4);
    return 1;
}

// @0047c7b0 0 for an ascii file, 1 for a binary one, -1 for anything else, -2 when it could not be read
static int ReadHeaderType(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        GameLog("ERROR: can't open %s: %s\n", path, strerror(errno));
        return -2;
    }
    char line[128];
    if (!fgets(line, 128, f)) {
        GameLog("ERROR: can't read header from %s\n", path);
        fclose(f);
        return -2;
    }
    fclose(f);
    if (strcmp(line, kAsciiHeader) == 0)
        return 0;
    return strcmp(line, kBinaryHeader) == 0 ? 1 : -1;
}

// @0047c8e0
int NavMesh::Load(const char *path)
{
    int type = ReadHeaderType(path);
    if (type == -2)
        return 0;
    if (type == -1) {
        GameLog("ERROR: invalid file header in %s\n", path);
        return 0;
    }
    FILE *f = fopen(path, type == 0 ? "r" : "rb");
    if (!f) {
        GameLog("ERROR: can't open %s: %s\n", path, strerror(errno));
        return 0;
    }
    int ok = 0;
    switch (type) {
    case 0:
        ReadAscii(f);
        // (no break in the original: an ascii file goes on into the binary reader, which finds no header and says so, so that
        // loading one fails. The shipped files are all binary.)
        [[fallthrough]];
    case 1:
        ok = ReadBinary(f);
    }
    for (unsigned i = 0; i < pointCount; i++) {
        float size = points[i].y * points[i].y + points[i].z * points[i].z + points[i].x * points[i].x;
        if (size > sizeSquared)
            sizeSquared = size;
    }
    fclose(f);
    return ok;
}

// @0047ca10 Points and faces, one line each, then the boxes
int NavMesh::ReadAscii(FILE *f)
{
    char line[256];
    if (!fgets(line, 256, f)) {
        GameLog("ERROR: can't read file header\n");
        return 0;
    }
    fgets(line, 256, f);
    unsigned count;
    if (sscanf(line, "Points %d {", &count) != 1) {
        GameLog("ERROR: can't read points list\n");
        return 0;
    }
    if (!AllocPoints(count)) {
        GameLog("ERROR: can't allocate %d points\n", count);
        return 0;
    }
    for (unsigned i = 0; i < pointCount; i++) {
        fgets(line, 256, f);
        if (sscanf(line, "%f %f %f", &points[i].x, &points[i].y, &points[i].z) != 3) {
            GameLog("ERROR: can't read point[%d]\n", i);
            return 0;
        }
    }
    fgets(line, 256, f);
    fgets(line, 256, f);
    if (sscanf(line, "Faces %d {", &count) != 1) {
        GameLog("ERROR: can't read face list\n");
        return 0;
    }
    if (!AllocFaces(count)) {
        GameLog("ERROR: can't allocate %d faces\n", count);
        return 0;
    }
    for (unsigned i = 0; i < faceCount; i++) {
        fgets(line, 256, f);
        NavFace *face = &faces[i];
        if (sscanf(line, "%d %d %d %d %d %d %f %f %f %x", &face->vertex[0], &face->vertex[1], &face->vertex[2], &face->neighbour[0],
                   &face->neighbour[1], &face->neighbour[2], &face->normal.x, &face->normal.y, &face->normal.z, &face->type) != 10) {
            GameLog("ERROR: can't read face[%d]\n", i);
            return 0;
        }
    }
    fgets(line, 256, f);
    return ReadAsciiBox(f, &tree);
}

// @0047cc60 One box: a line with a label and "{" (or "NULL" for no box), its minimum corner, its maximum corner, the number of
// faces, one line per face, a blank line, and the eight children each behind a label.
int NavMesh::ReadAsciiBox(FILE *f, NavBox **out)
{
    char line[256], word[64];
    *out = NULL;
    fgets(line, 256, f);
    if (sscanf(line, "%63s", word) != 1) {
        GameLog("ERROR: can't get start token of bbox\n");
        return 0;
    }
    if (strcmp(word, "NULL") == 0)
        return 1;

    NavBox *box = new NavBox;
    if (!box)
        return 0;
    fgets(line, 256, f);
    if (sscanf(line, "%63s %f %f %f", word, &box->xMin, &box->yMin, &box->zMin) != 4) {
        GameLog("ERROR: can't read min values of bbox\n");
        delete box;
        return 0;
    }
    fgets(line, 256, f);
    if (sscanf(line, "%63s %f %f %f", word, &box->xMax, &box->yMax, &box->zMax) != 4) {
        GameLog("ERROR: can't read max values of bbox\n");
        delete box;
        return 0;
    }
    fgets(line, 256, f);
    int count;
    if (sscanf(line, "%63s %d", word, &count) != 2) {
        GameLog("ERROR: can't read number of faces of bbox\n");
        delete box;
        return 0;
    }
    box->AllocFaces(count);     // (the original does not look at the answer)
    for (unsigned i = 0; i < box->faceCount; i++) {
        fgets(line, 256, f);
        if (sscanf(line, "%d", &box->faces[i]) != 1) {
            GameLog("ERROR: can't read face index of bbox\n");
            delete box;
            return 0;
        }
    }
    fgets(line, 256, f);
    for (int i = 0; i < 8; i++) {
        fscanf(f, "%255s", line);
        if (!ReadAsciiBox(f, &box->child[i])) {
            delete box;
            return 0;
        }
    }
    fgets(line, 256, f);
    *out = box;
    return 1;
}

// @0047cfe0 The header, the points, the faces, then the boxes. Numbers are 16 bit counts and indices, 32 bit floats.
int NavMesh::ReadBinary(FILE *f)
{
    char header[sizeof kBinaryHeader];
    if (fread(header, sizeof kBinaryHeader - 1, 1, f) != 1) {
        GameLog("ERROR: can't read file header\n");
        return 0;
    }
    unsigned short number = 0;
    if (!ReadShort(f, &number)) {
        GameLog("ERROR: can't read number of points in surface\n");
        return 0;
    }
    pointCount = number;
    if (!AllocPoints(number)) {
        GameLog("ERROR: can't allocate %d points for surface\n", pointCount);
        return 0;
    }
    for (unsigned i = 0; i < pointCount; i++) {
        if (!ReadFloat(f, &points[i].x) || !ReadFloat(f, &points[i].y) || !ReadFloat(f, &points[i].z)) {
            GameLog("ERROR: can't read %d points for surface\n", pointCount);
            return 0;
        }
    }
    if (!ReadShort(f, &number)) {
        GameLog("ERROR: can't read number of faces in surface\n");
        return 0;
    }
    faceCount = number;
    if (!AllocFaces(number)) {
        GameLog("ERROR: can't allocate  %d faces\n", faceCount);
        return 0;
    }
    for (unsigned i = 0; i < faceCount; i++) {
        NavFace *face = &faces[i];
        unsigned short word = 0;
        int got = ReadShort(f, &word);
        face->vertex[0] = word;
        got += ReadShort(f, &word);
        face->vertex[1] = word;
        got += ReadShort(f, &word);
        face->vertex[2] = word;
        got += ReadShort(f, &word);
        face->neighbour[0] = (short)word;
        got += ReadShort(f, &word);
        face->neighbour[1] = (short)word;
        got += ReadShort(f, &word);
        face->neighbour[2] = (short)word;
        got += ReadFloat(f, &face->normal.x);
        got += ReadFloat(f, &face->normal.y);
        got += ReadFloat(f, &face->normal.z);
        got += ReadInt(f, &face->type);
        if (got != 10) {
            GameLog("ERROR: can't read face[%d] for surface\n", i);
            return 0;
        }
    }
    return ReadBinaryBox(f, &tree) != 0;
}

// @0047d2b0 One box: the corners, the faces, then for each of the eight children a flag (0xffff: none) and the child itself
int NavMesh::ReadBinaryBox(FILE *f, NavBox **out)
{
    *out = NULL;
    NavBox *box = new NavBox;
    if (!box)
        return 0;
    ReadFloat(f, &box->xMin);
    ReadFloat(f, &box->yMin);
    ReadFloat(f, &box->zMin);
    ReadFloat(f, &box->xMax);
    ReadFloat(f, &box->yMax);
    ReadFloat(f, &box->zMax);
    unsigned short number = 0;
    if (!ReadShort(f, &number)) {
        GameLog("ERROR: can't read number of faces for bbox\n");
        delete box;
        return 0;
    }
    box->faceCount = number;
    if (!box->AllocFaces(number)) {
        GameLog("ERROR: can't alloacte %d faces for bbox\n", box->faceCount);
        delete box;
        return 0;
    }
    for (unsigned i = 0; i < box->faceCount; i++) {
        if (!ReadShort(f, &number)) {
            GameLog("ERROR: can't read face[%d] of bbox\n", i);
            delete box;
            return 0;
        }
        box->faces[i] = number;
    }
    for (int i = 0; i < 8; i++) {
        box->child[i] = NULL;
        if (!ReadShort(f, &number)) {
            delete box;
            GameLog("ERROR: can't read child %d\n", i);
            return 0;
        }
        if (number != 0xffff && !ReadBinaryBox(f, &box->child[i])) {
            delete box;
            return 0;
        }
    }
    *out = box;
    return 1;
}
