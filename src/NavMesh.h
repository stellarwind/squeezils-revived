// NavMesh: the walkable geometry of one surface, loaded from a .nav file. Reconstructed from SQZ.EXE 0x47aec0-0x47d5f0. The class has no
// RTTI name; "NavMesh" is ours. A *loaded surface* (NavSurface.h) owns one, and so does the world (the level's own ground).
//
// A mesh is a list of points and a list of triangles (faces) over them, plus an octree of boxes over the faces so that a line only
// has to be tested against the few faces near it. Every face knows which face lies across each of its three edges (-1 for none), its
// normal, and a surface type number (written afterwards from the .nat file, see ReadSurfaceTypes).
//
// Two queries, both pure geometry on plain memory:
//   Trace: a straight line from a point; the nearest face it pierces.
//   Move:  slide along the faces from a point on a face for a distance, crossing into the neighbour face when an edge is reached
//          (turning the whole frame of reference about that edge), stopping at an edge nobody may cross.
// A *frame of reference* is the squeezil's right, up and forward vectors (up is the face normal while standing on a face).
#pragma once
#include <stdio.h>
#include "Container.h"
#include "Vector3f.h"

// 0x28 bytes. Made by @0047ad20, destroyed by the (empty) @004053e0.
struct NavFace {
    int      vertex[3];         // 0x00 point numbers
    Vector3f normal;            // 0x0c unit length, pointing out of the surface
    int      neighbour[3];      // 0x18 the face across the edge vertex[0]-vertex[1], vertex[1]-vertex[2], vertex[2]-vertex[0]; -1 for none
    int      type;              // 0x24 surface type number (0 = the first type of the .nat file)
    NavFace();                  // @0047ad20
};
static_assert(sizeof(NavFace) == 0x28, "NavFace layout");

// 0x40 bytes. One box of the octree: the faces whose centre lies in it (or, in a leaf, that touch it), and up to eight smaller boxes.
struct NavBox : GameHeapObject {
    float    xMin, xMax, yMin, yMax, zMin, zMax;    // 0x00 the box is half open: min <= p < max
    unsigned faceCount;                             // 0x18
    int     *faces;                                 // 0x1c face numbers
    NavBox  *child[8];                              // 0x20 NULL where there is none; a box without children is a leaf

    NavBox();                                       // @0047ade0
    ~NavBox();                                      // @0047ae10 frees the children too (not the box itself)
    int  AllocFaces(unsigned count);                // @0047ae50 nonzero when it worked
    void FreeFaces();                               // @0047ae90
    unsigned Outcode(const Vector3f &p) const;      // @0047d4f0 bit 0: beyond xMax, 1: below xMin, 2/3 the same for y, 4/5 for z
};
static_assert(sizeof(NavBox) == 0x40, "NavBox layout");

// 0x2030 bytes (new 0x2030)
struct NavMesh : GameHeapObject {
    unsigned  pointCount;       // 0x0000
    Vector3f *points;           // 0x0004 heap array with a count word (Container.h NewArray)
    unsigned  faceCount;        // 0x0008
    NavFace  *faces;            // 0x000c heap array with a count word
    NavBox   *tree;             // 0x0010 the octree's root, NULL when there is none
    struct Hit { int face; float t; } hits[1024];   // 0x0014 Trace's scratch list: faces the line pierces, nearest first
    int       unknown2014[2];   // 0x2014 never touched
    int       unknown201c;      // 0x201c cleared by the constructor, never used
    float     sizeSquared;      // 0x2020 the largest squared distance of a point from the origin (@0047af20 returns it)
    int       unknown2024[3];   // 0x2024 cleared by the constructor, never used

    NavMesh();                  // @0047aec0 clears the fields it knows (the hit list is left alone)
    ~NavMesh();                 // @0047af00 frees the points, the faces and the tree
    int  Load(const char *path);    // @0047c8e0 nonzero when the file was read (the errors go to the game's log)

    // @0047b180 A straight line from pos along `direction` for `length`. Only the faces in the box that holds the *end* of the line are
    // tested, each pierced once, and a face whose type has no flag in `flags` (one int per surface type) is looked through. On a hit
    // the position, the face's normal as up, a right vector across the line's forward, and forward are given; returns nonzero.
    int  Trace(const float *pos, const float *right, const float *up, const float *forward, const float *direction, float length,
               float *hitPos, float *hitRight, float *hitUp, float *hitForward, int *face, const int *flags);

    // @0047b490 (recursive) See the header comment. Returns nonzero when the whole distance was walked; 0 when the way was blocked
    // (then *newFace is -1 and newPos is the edge) or the recursion went deeper than 16 faces.
    int  Move(const float *pos, const float *right, const float *up, const float *forward, const float *direction, float distance,
              int face, float *newPos, float *newRight, float *newUp, float *newForward, float *newDirection, int *newFace,
              const int *flags);

    NavBox *FindLeaf(const Vector3f &point, NavBox *box);    // @0047d560 the leaf box holding the point (box NULL: start at the root)
    int  PointInFace(const NavFace *face, const Vector3f &point) const;   // @0047c2a0 inside the triangle (edges included), on its plane

private:
    int  AllocPoints(unsigned count);       // @0047af30 nonzero when it worked
    int  AllocFaces(unsigned count);        // @0047aff0
    void FreePoints();                      // @0047b0b0
    void FreeFaces();                       // @0047b0f0
    void FreeTree();                        // @0047b150
    int  ReadAscii(FILE *f);                // @0047ca10
    int  ReadAsciiBox(FILE *f, NavBox **out);   // @0047cc60 (recursive)
    int  ReadBinary(FILE *f);               // @0047cfe0
    int  ReadBinaryBox(FILE *f, NavBox **out);  // @0047d2b0 (recursive)
};
