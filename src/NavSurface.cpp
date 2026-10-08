// NavSurface: see NavSurface.h. Reconstructed from SQZ.EXE 0x40bf90-0x40d520 (instances and the space conversions) and 0x40f380-0x412c10
// (the place's surface list queries).
#include "NavSurface.h"
#include "SqueezilSoulInternal.h"
#include "NavProp.h"

static const float kIdentity[16] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };

static void CopyMatrix(float *to, const float *from) { memcpy(to, from, 16 * sizeof(float)); }

// The matrix with its position row cleared
static void TurnOnly(float *to, const float *from)
{
    CopyMatrix(to, from);
    to[12] = 0.0f; to[13] = 0.0f; to[14] = 0.0f;
}

// The matrix with its turn flipped round: for a turn without scaling that is its inverse (@0047dbf0, in place)
static void Transpose(float *m)
{
    float t[16];
    CopyMatrix(t, m);
    for (int row = 0; row < 4; row++)
        for (int column = 0; column < 4; column++)
            m[row * 4 + column] = t[column * 4 + row];
}

// @0040c8a0 (p - position) turned by the inverse turn. With identity set the point comes back unchanged.
float *PointToLocal(float *out, const float *point, const float *m, int identity, const float *inverse)
{
    if (identity) {
        out[0] = point[0]; out[1] = point[1]; out[2] = point[2];
        return out;
    }
    float turn[16];
    if (!inverse) {
        TurnOnly(turn, m);
        Transpose(turn);
        inverse = turn;
    }
    Vector3f back = Row(m, 3) * -1.0f;
    Vector3f moved = Vector3f(point[0], point[1], point[2]) + back;
    Vector3f local = TransformPoint(moved, inverse);
    out[0] = local.x; out[1] = local.y; out[2] = local.z;
    return out;
}

// @0040ce30 p turned by the turn, then moved to the position
float *PointToWorld(float *out, const float *point, const float *m, int identity, const float *turn)
{
    if (identity) {
        out[0] = point[0]; out[1] = point[1]; out[2] = point[2];
        return out;
    }
    float own[16];
    if (!turn) {
        TurnOnly(own, m);
        turn = own;
    }
    Vector3f turned = TransformPoint(Vector3f(point[0], point[1], point[2]), turn);
    Vector3f world = turned + Row(m, 3);
    out[0] = world.x; out[1] = world.y; out[2] = world.z;
    return out;
}

// @0040cb30 the matrix a, which is in the world, written in the space of m
float *MatrixToLocal(float *out, const float *a, const float *m, int identity)
{
    if (identity) {
        CopyMatrix(out, a);
        return out;
    }
    float inverse[16], turn[16], product[16], position[3];
    TurnOnly(inverse, m);
    Transpose(inverse);
    TurnOnly(turn, a);
    MultiplyMatrix(product, turn, inverse);
    CopyMatrix(turn, product);
    PointToLocal(position, a + 12, m, 0, inverse);
    turn[12] = position[0]; turn[13] = position[1]; turn[14] = position[2];
    CopyMatrix(out, turn);
    return out;
}

// @0040d070 the matrix a, which is in the space of m, written in the world
float *MatrixToWorld(float *out, const float *a, const float *m, int identity)
{
    if (identity) {
        CopyMatrix(out, a);
        return out;
    }
    float turn[16], own[16], product[16], position[3];
    TurnOnly(turn, m);
    TurnOnly(own, a);
    MultiplyMatrix(product, own, turn);
    CopyMatrix(own, product);
    PointToWorld(position, a + 12, m, 0, turn);
    own[12] = position[0]; own[13] = position[1]; own[14] = position[2];
    CopyMatrix(out, own);
    return out;
}

// ---- InstancedSurface ----

// @0040bf90
InstancedSurface *InstancedSurface::Init(NavProp *owner, LoadedSurface *loaded, const float *start, IniValueTable *given)
{
    ::new (&properties) IniValueTable();
    ::new (&flags) IntList();
    surface = loaded;
    prop = owner;
    CopyMatrix(matrix, start);
    CopyMatrix(previous, matrix);
    identity = memcmp(matrix, kIdentity, sizeof matrix) == 0;
    previousIdentity = identity;
    float origin[3] = { 0.0f, 0.0f, 0.0f };
    float where[3];
    ToWorld(where, origin);
    for (int i = 0; i < 3; i++) {
        position[i] = where[i];
        previousPosition[i] = where[i];
    }
    moveDirty = 0;
    changed = 0;
    String none("none");
    for (int i = 0; i < loaded->typeNames.count; i++) {
        properties.Insert(loaded->typeNames.At(i), none, -1);
        flags.Insert(1, -1);
    }
    for (int i = 0; i < given->keys.count; i++)
        properties.Insert(given->keys.At(i), given->values.At(i), -1);
    return this;
}

// @0040c470
void InstancedSurface::Move(const float *newMatrix)
{
    CopyMatrix(previous, matrix);
    previousIdentity = identity;
    for (int i = 0; i < 3; i++)
        previousPosition[i] = position[i];
    moveDirty = 1;
    CopyMatrix(matrix, newMatrix);
    identity = memcmp(matrix, kIdentity, sizeof matrix) == 0;
    float origin[3] = { 0.0f, 0.0f, 0.0f };
    float where[3];
    ToWorld(where, origin);
    for (int i = 0; i < 3; i++)
        position[i] = where[i];
}

// @0040c700
bool InstancedSurface::IsNear(const float *point)
{
    Vector3f d = Vector3f(position[0], position[1], position[2]) - Vector3f(point[0], point[1], point[2]);
    float squared = d.x * d.x + d.y * d.y + d.z * d.z;
    return !(surface->sizeSquared < squared);
}

// @0040c7b0
bool InstancedSurface::WasNear(const float *point)
{
    Vector3f d = Vector3f(previousPosition[0], previousPosition[1], previousPosition[2]) - Vector3f(point[0], point[1], point[2]);
    float squared = d.x * d.x + d.z * d.z + d.y * d.y;
    return !(surface->sizeSquared < squared);
}

// @0040c860 the original compares the 64 bytes (repe cmpsb: previous against current) once per move
int InstancedSurface::Changed()
{
    if (moveDirty) {
        int result = 0;
        const unsigned char *a = (const unsigned char *)previous;
        const unsigned char *b = (const unsigned char *)matrix;
        for (int i = 0; i < 0x40; i++) {
            if (a[i] != b[i]) {
                result = a[i] < b[i] ? -1 : 1;
                break;
            }
        }
        moveDirty = 0;
        changed = result;
    }
    return changed;
}

// @0040f6f0
float *InstancedSurface::ToLocal(float *out, const float *point) { return PointToLocal(out, point, matrix, identity, NULL); }
// @0040f850
float *InstancedSurface::ToWorld(float *out, const float *point) { return PointToWorld(out, point, matrix, identity, NULL); }
// @004105f0
float *InstancedSurface::MatrixToWorld(float *out, const float *m) { return ::MatrixToWorld(out, m, matrix, identity); }
// @00410660
float *InstancedSurface::ToLocalBefore(float *out, const float *point) { return PointToLocal(out, point, previous, previousIdentity, NULL); }

// @0040d370 The point in the surface's space, put back into the world with the matrix it had before the last Move: that is
// where it would have been; the difference to where it is now is how far the surface carried it. (No identity shortcut.)
float *InstancedSurface::Carried(float *out, const float *point)
{
    float local[3];
    ToLocal(local, point);
    float turn[16];
    TurnOnly(turn, previous);
    Vector3f was = TransformPoint(Vector3f(local[0], local[1], local[2]), turn) + Row(previous, 3);
    out[0] = point[0] - was.x;
    out[1] = point[1] - was.y;
    out[2] = point[2] - was.z;
    return out;
}

// ---- NavSurfaceList ----

// @00412bc0
void *NavSurfaceList::FaceMesh(const Contact *c) { return Instance(c->face)->surface->mesh; }

// @0040f380
int NavSurfaceList::FaceType(const Contact *c) { return NavMeshFaceType(FaceMesh(c), c->second); }

// @0040f3c0 the loaded surface's name for the face's type number (String by value, built in *out)
String *NavSurfaceList::TypeName(String *out, const Contact *c)
{
    int type = FaceType(c);
    ::new (out) String(Instance(c->face)->surface->typeNames.At(type));
    return out;
}

// The five queries below all start the same way: the face's type name, what the instance binds that name to, and the place's
// description for the result. The instance's table is read without a test, as in the original.
SurfaceTypeDescription *NavSurfaceList::Description(const Contact *c)
{
    String name;
    TypeName(&name, c);
    String bound = Properties(c)->Get(name);
    if (!types.Contains(bound))
        return NULL;
    return types.Get(bound);
}

// @0040f610
int NavSurfaceList::BlocksWalking(const Contact *c)
{
    SurfaceTypeDescription *d = Description(c);
    return d ? d->blocksWalking : 0;
}

// @0040f770
int NavSurfaceList::IsSlippery(const Contact *c)
{
    SurfaceTypeDescription *d = Description(c);
    return d ? d->slippery : 0;
}

// @0040f8d0
int NavSurfaceList::HasTouchScript(const Contact *c)
{
    SurfaceTypeDescription *d = Description(c);
    return d ? d->hasTouchScript : 0;
}

// @0040f9b0
String *NavSurfaceList::TouchScript(String *out, const Contact *c)
{
    SurfaceTypeDescription *d = Description(c);
    if (d)
        ::new (out) String(d->touchScript);
    else
        ::new (out) String(kEmptyString);
    return out;
}

// @0040fb20
float NavSurfaceList::TypeValue(const Contact *c)
{
    SurfaceTypeDescription *d = Description(c);
    return d ? d->value : 0.0f;
}

// @00410890
int NavSurfaceList::IsGone(const Contact *c)
{
    return removedLater.Contains(c->face) || removed.Contains(c->face);
}

// @004102f0
void NavSurfaceList::Remove(int instance) { removed.Insert(instance); }

// @00410310
void NavSurfaceList::MoveInstance(int instance, const float *matrix) { Instance(instance)->Move(matrix); }

// The calls the rest of the reconstruction makes (Original.h)
void *NavSurfaceFaceOf(void *surfaces, const Contact *c) { return ((NavSurfaceList *)surfaces)->Instance(c->face); }
int   NavSurfaceIsGone(void *surfaces, const Contact *c) { return ((NavSurfaceList *)surfaces)->IsGone(c); }
String NavSurfaceTypeName(void *surfaces, const Contact *c)
{
    String name;
    ((NavSurfaceList *)surfaces)->TypeName(&name, c);
    return name;
}
IniValueTable *NavSurfaceProperties(void *surfaces, const Contact *c) { return ((NavSurfaceList *)surfaces)->Properties(c); }
int   NavSurfaceIsSlippery(void *surfaces, const Contact *c)     { return ((NavSurfaceList *)surfaces)->IsSlippery(c); }
int   NavSurfaceBlocksWalking(void *surfaces, const Contact *c)  { return ((NavSurfaceList *)surfaces)->BlocksWalking(c); }
int   NavSurfaceHasTouchScript(void *surfaces, const Contact *c) { return ((NavSurfaceList *)surfaces)->HasTouchScript(c); }
String NavSurfaceTouchScript(void *surfaces, const Contact *c)
{
    String name;
    ((NavSurfaceList *)surfaces)->TouchScript(&name, c);
    return name;
}
LoadedSurface *LoadedSurfaceAt(void *surfaces, int surface) { return ((NavSurfaceList *)surfaces)->Loaded(surface); }
void  RemoveSurfaceInstance(void *surfaces, int instance)   { ((NavSurfaceList *)surfaces)->Remove(instance); }
void  MoveSurfaceInstance(void *surfaces, int instance, const float *matrix) { ((NavSurfaceList *)surfaces)->MoveInstance(instance, matrix); }
