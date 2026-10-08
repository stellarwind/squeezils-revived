// NavSurface: see NavSurface.h. Reconstructed from SQZ.EXE 0x40bf90-0x40d520 (instances and the space conversions) and 0x40f380-0x412c10
// (the place's surface list queries).
#include "NavSurface.h"
#include "SqueezilSoulInternal.h"
#include "NavProp.h"
#include "IniReader.h"
#include "World.h"
#include <stdio.h>
#include <stdlib.h>

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
int NavSurfaceList::Bounces(const Contact *c)
{
    SurfaceTypeDescription *d = Description(c);
    return d ? d->bounce : 0;
}

// @0040f770
int NavSurfaceList::IsSlippery(const Contact *c)
{
    SurfaceTypeDescription *d = Description(c);
    return d ? d->slippery : 0;
}

// @0040f8d0
int NavSurfaceList::FlyThrough(const Contact *c)
{
    SurfaceTypeDescription *d = Description(c);
    return d ? d->flyThrough : 0;
}

// @0040f9b0
String *NavSurfaceList::ActivateScript(String *out, const Contact *c)
{
    SurfaceTypeDescription *d = Description(c);
    if (d)
        ::new (out) String(d->activate);
    else
        ::new (out) String(kEmptyString);
    return out;
}

// @0040fb20
float NavSurfaceList::RebudGoodies(const Contact *c)
{
    SurfaceTypeDescription *d = Description(c);
    return d ? d->rebudGoodies : 0.0f;
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
int   NavSurfaceBounces(void *surfaces, const Contact *c)  { return ((NavSurfaceList *)surfaces)->Bounces(c); }
int   NavSurfaceFlyThrough(void *surfaces, const Contact *c) { return ((NavSurfaceList *)surfaces)->FlyThrough(c); }
String NavSurfaceActivateScript(void *surfaces, const Contact *c)
{
    String name;
    ((NavSurfaceList *)surfaces)->ActivateScript(&name, c);
    return name;
}
LoadedSurface *LoadedSurfaceAt(void *surfaces, int surface) { return ((NavSurfaceList *)surfaces)->Loaded(surface); }
void  RemoveSurfaceInstance(void *surfaces, int instance)   { ((NavSurfaceList *)surfaces)->Remove(instance); }
void  MoveSurfaceInstance(void *surfaces, int instance, const float *matrix) { ((NavSurfaceList *)surfaces)->MoveInstance(instance, matrix); }

// ---- loading ----

static const char *const kHeader = "#Nat V1.0 ascii\n";    // the original's string (0x4b9b1c) includes the newline
static const char *const kNone = "none";                // the string at 0x4b9cf4 (the default binding of every type)
static int *const kDebugSurfaceTypeOutput = (int *)0x4b9b30;   // the debug flag the ground loader sets from [Defaults]
static const char *const kFirstType = " none";      // the string at 0x4b9ef4 (a leading blank); the instance defaults use "none" (0x4b9cf4)

// @0040bc60
LoadedSurface *LoadedSurface::Init(void *navMesh, int faces, const char *folder, const char *natFile, StringList *types)
{
    ::new (&directory) String();
    ::new (&file) String();
    ::new (&typeNames) StringList(*types);
    ::new (&typeNumbers) SurfaceTypeNumbers();
    mesh = navMesh;
    sizeSquared = *(float *)((char *)navMesh + 0x2020);
    faceCount = faces;
    directory = String(folder);
    file = String(natFile);
    for (int i = 0; i < typeNames.count; i++)
        typeNumbers.Insert(typeNames.At(i), i, -1);
    return this;
}

// @0040eec0
int ReadSurfaceTypes(void *mesh, const char *, const char *natFile, StringList *types)
{
    if (natFile[0] == '\0')
        return 0;
    FILE *f;
    for (;;) {
        f = fopen(natFile, "r");
        if (f)
            break;
        if (GetRenderer()->FatalError("Could not read file %s.\n", natFile) == IDCANCEL) {
            GameLog("ERROR: Could not open surface file %s.\n", natFile);
            return 0;
        }
    }
    char line[256];
    const char *error = NULL;
    int count = 0;
    if (!fgets(line, 0xff, f))
        error = "ERROR: Could not read first line.\n";
    else if (!strstr(line, kHeader))
        error = "ERROR: File has invalid header.\n";
    else if (!fgets(line, 0xff, f))
        error = "ERROR: Could not read number of faces.\n";
    else if ((count = atoi(line)) <= 0)
        error = "ERROR: Invalid number of faces.\n";
    if (error) {
        GameLog(error);
        fclose(f);
        return 0;
    }

    // The type numbers of this file: the list's own, which only grows (" none" is number 0)
    SurfaceTypeNumbers numbers;
    numbers.Insert(String(kFirstType), 0, -1);
    unsigned faceCount = *(unsigned *)((char *)mesh + 8);
    char *faces = *(char **)((char *)mesh + 0xc);
    for (int i = 0; i < count; i++) {
        unsigned face;
        char name[64];
        if (!fgets(line, 0xff, f) || sscanf(line, "%d%63s", &face, name) != 2) {
            GameLog("ERROR: Could not read surface entry %d.\n", i);
            fclose(f);
            return 0;
        }
        if (face >= faceCount)
            continue;
        String key(name);
        int type;
        if (!numbers.Contains(key)) {
            type = types->count;
            numbers.Insert(key, type, -1);
            types->Insert(String(name), -1);
        } else {
            type = numbers.Get(key);
        }
        *(int *)(faces + 0x24 + face * 0x28) = type;
    }
    fclose(f);
    return count;
}

// @0040fc10 The same folder and .nat file as a surface already loaded gives that one back (the .nav file is not compared).
// Without a '\' in the .nav file name the game moves to the ini file's folder while it loads.
int NavSurfaceList::Load(IniReader *ini, const char *navFile, const char *natFile)
{
    String folder;
    ini->GetDirectory(&folder);
    GameLog("Loading surface %s (.nat file %s) from directory %s... ", navFile, natFile, folder.c_str());
    for (int i = 0; i < loaded.count; i++) {
        LoadedSurface *l = loaded.At(i);
        if (strcmp(folder.c_str(), l->directory.c_str()) == 0 && strcmp(natFile, l->file.c_str()) == 0) {
            GameLog("already loaded OK.\n");
            return i;
        }
    }

    char saved[128] = "";
    if (!strchr(navFile, '\\')) {
        GetCurrentDirectoryA(0x7f, saved);
        String again;
        ini->GetDirectory(&again);
        SetCurrentDirectoryA(again.c_str());
    }
    void *mesh = NavMeshConstruct(Operator_New(0x2030));
    if (!NavMeshLoad(mesh, navFile)) {
        GameLog("failed!\n");
        NavMeshDestroy(mesh);
        Operator_Delete(mesh);
        if (saved[0])
            SetCurrentDirectoryA(saved);
        return 0;
    }
    GameLog("OK.\n");
    StringList types;
    types.Insert(String(kFirstType), -1);
    String folderAgain;
    ini->GetDirectory(&folderAgain);
    int faces = ReadSurfaceTypes(mesh, folderAgain.c_str(), natFile, &types);
    if (saved[0])
        SetCurrentDirectoryA(saved);
    String last;
    ini->GetDirectory(&last);
    LoadedSurface *surface = (LoadedSurface *)Operator_New(sizeof(LoadedSurface));
    if (surface)
        surface->Init(mesh, faces, last.c_str(), natFile, &types);
    loaded.Insert(surface, -1);
    return loaded.count - 1;
}

// @004100b0 The copy takes the first free place in the list (a removed copy leaves a NULL); the place is then taken off the removal
// lists. Each surface type of the surface gets its flag from the description its binding names, 1 when that is not a known type.
int NavSurfaceList::AddInstance(NavProp *prop, int surface, const float *matrix, IniValueTable *properties)
{
    LoadedSurface *l = Loaded(surface);
    InstancedSurface *copy = (InstancedSurface *)Operator_New(sizeof(InstancedSurface));
    if (copy)
        copy->Init(prop, l, matrix, properties);
    for (int i = 0; i < l->typeNames.count; i++) {
        String name = l->typeNames.At(i);
        String bound = copy->properties.Get(name);
        int flag = 1;
        if (types.Contains(bound))
            flag = types.Get(bound)->walkable;
        copy->flags.Replace(i, flag);
    }
    int place = 0;
    while (place < instances.count) {
        if (instances.At(place) == NULL) {
            instances.Replace(place, copy);
            for (int k = removed.ids.count - 1; k >= 0; k--)
                if (removed.At(k) == place)
                    removed.RemoveAt(k);
            for (int k = removedLater.ids.count - 1; k >= 0; k--)
                if (removedLater.At(k) == place)
                    removedLater.RemoveAt(k);
            break;
        }
        place++;
    }
    if (instances.count == place)
        instances.Insert(copy, -1);
    return place;
}

int  LoadSurface(void *surfaces, IniReader *ini, const char *name, const char *natFile)
{
    return ((NavSurfaceList *)surfaces)->Load(ini, name, natFile);
}
int  AddSurfaceInstance(void *surfaces, NavProp *prop, int surface, const float *matrix, IniValueTable *properties)
{
    return ((NavSurfaceList *)surfaces)->AddInstance(prop, surface, matrix, properties);
}

// @0040e480 Loads the place's own ground. The [Defaults] SurfaceFile names its .nat file; a name without a backslash is taken
// relative to the ini's folder. Surface type 0 is " none", and the instance of the ground has every type bound to "none" unless
// the ini's bindings say otherwise. The ground becomes loaded surface 0 and instance 0 of this list.
// Not verified in play: the empty SurfaceFile branch.
int NavSurfaceList::LoadGround(IniReader *ini)
{
    IniReader current;                                           // the game's current ini file (IniReader())
    *kDebugSurfaceTypeOutput = current.GetInt("DebugSurfaceTypeOutput", 0);

    StringList typeList;
    StringList *types = &typeList;
    types->Insert(String(kFirstType), -1);

    String folder;
    ini->GetDirectory(&folder);
    String surfaceName(ini->GetString("SurfaceFile"));     // as written in the ini; this (not `path`) is what the loaded surface keeps
    const char *surfaceFile = surfaceName.c_str();
    int faces = 0;
    path[0] = '\0';
    if (!surfaceFile[0]) {
        // @0040e593 no SurfaceFile: the name becomes <ini folder>\placexxx.nat and no file is read
        snprintf(path, sizeof path, "%s\\placexxx.nat", folder.c_str());
    } else {
        strncpy(path, surfaceFile, sizeof path - 1);
        path[sizeof path - 1] = '\0';
        if (!strchr(path, '\\')) {
            GameLog("...surface file name relative, setting to %s.\n", path);
            char relative[sizeof path];
            snprintf(relative, sizeof relative, "%s\\%s", folder.c_str(), path);
            strncpy(path, relative, sizeof path - 1);
            path[sizeof path - 1] = '\0';
        }
        // The nav mesh of the world holds the .nat file's faces (the .nav file itself was loaded by the world, LoadNavMesh).
        faces = ReadSurfaceTypes(GetWorld()->navMesh, folder.c_str(), path, types);
    }

    IniValueTable bindings;
    ReadSurfaceBindings(ini, &bindings);

    // The instance's table: every type starts as "none", then the ini's bindings are laid over it.
    IniValueTable properties;
    for (int i = 0; i < types->count; i++)
        properties.Insert(types->At(i), String(kNone), -1);
    for (int i = 0; i < bindings.keys.count; i++)
        properties.Insert(bindings.keys.At(i), bindings.GetAt(i), -1);

    if (loaded.count == 0) {
        LoadedSurface *surface = (LoadedSurface *)Operator_New(sizeof(LoadedSurface));
        if (surface)
            surface->Init(GetWorld()->navMesh, faces, folder.c_str(), surfaceFile, types);
        loaded.Insert(surface, -1);
    }

    AddInstance(NULL, 0, kIdentity, &properties);

    if (!ReadSurfaceList(ini))
        GameLog("Error reading surface list.\n");
    ini->SetSection(kDefaultSectionName);
    return 1;
}
