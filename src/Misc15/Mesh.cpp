#include "NavMesh/NavMesh.h"
MFace::MFace()
{
    c = 0; b = 0; a = 0;
    normal = MV3(0.0f, 0.0f, 1.0f);
    i2 = -1; i1 = -1; i0 = -1;
    pad = 0;
}

NavMesh::NavMesh()
{
    pointCount = 0; points = 0; faceCount = 0; faces = 0; tree = 0;
    p201c = 0; p2024 = 0; p2028 = 0; p202c = 0; radius = 0;
}

float NavMesh::Radius() { return radius; }

int NavMesh::AllocPoints(int n)
{
    FreeVerts();
    if (n) {
        points = new MVert[n];
        if (!points)
            return 0;
    }
    pointCount = n;
    return 1;
}

int NavMesh::AllocFaces(int n)
{
    FreeFaces();
    if (n) {
        faces = new MFace[n];
        if (!faces)
            return 0;
    }
    faceCount = n;
    return 1;
}

// 22 spare declarations (symbol count, see padscan.sh)
void PadDecl1(), PadDecl2(), PadDecl3(), PadDecl4(), PadDecl5(), PadDecl6(), PadDecl7(), PadDecl8(), PadDecl9(), PadDecl10(), PadDecl11(), PadDecl12(), PadDecl13(), PadDecl14(), PadDecl15(), PadDecl16(), PadDecl17(), PadDecl18(), PadDecl19(), PadDecl20(), PadDecl21(), PadDecl22();

void NavMesh::FreeVerts()
{
    if (points)
        delete [] points;
    pointCount = 0;
    points = 0;
}

// 10 more spare declarations
void PadDeclB1(), PadDeclB2(), PadDeclB3(), PadDeclB4(), PadDeclB5(), PadDeclB6(), PadDeclB7(), PadDeclB8(), PadDeclB9(), PadDeclB10();

void NavMesh::FreeFaces()
{
    if (faces)
        delete [] faces;
    faceCount = 0;
    faces = 0;
}

void NavMesh::FreeArrays()
{
    FreeVerts();
    FreeFaces();
}

void NavMesh::FreeTree()
{
    TreeNode *t = (TreeNode *)tree;
    if (t) {
        delete t;
        tree = 0;
    }
}
