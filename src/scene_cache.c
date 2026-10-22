/* Retain local-space mesh data independently of placement/camera transforms.
 * Hooked native mutators invalidate owners. Unhooked oracle fixtures compare
 * source vertices/topology on each use, so direct native edits remain testable. */
#include "scene_gpu.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Expose the cache accessors for the DLL integration fixture. */
__declspec(dllexport) void (*sqz_geometry_install)(void *, BOOL);
typedef struct GeometryGroup
{
    SceneGeometry geometry;
    unsigned group, nv, nf, vpf;
    size_t words;
    D3DRMVERTEX *source;
    unsigned *indices;
    D3DRMRENDERQUALITY quality;
    BOOL dirty;
    struct GeometryGroup *next;
} GeometryGroup;
typedef struct GeometryOwner
{
    IUnknown *source;
    IDirect3DRMMesh *mesh;
    BOOL builder, tracked, dirty;
    GeometryGroup *groups;
    struct GeometryOwner *next;
} GeometryOwner;
static GeometryOwner *owners;

static GeometryOwner *find_owner(void *source)
{
    for (GeometryOwner *owner = owners; owner; owner = owner->next)
        if (owner->source == source)
            return owner;
    return NULL;
}
void sqz_scene_geometry_changed(void *source)
{
    if (!source)
    {
        /* Face edits belong to builders; the face API does not expose its
         * owning builder. Conservatively invalidate cached builders only. */
        for (GeometryOwner *owner = owners; owner; owner = owner->next)
            if (owner->builder)
                sqz_scene_geometry_changed(owner->source);
        return;
    }
    GeometryOwner *owner = find_owner(source);
    if (!owner)
        return;
    owner->dirty = TRUE;
    for (GeometryGroup *group = owner->groups; group; group = group->next)
        group->dirty = TRUE;
}
__declspec(dllexport) IDirect3DRMMesh *sqz_geometry_mesh(void *source, BOOL builder)
{
    GeometryOwner *owner = find_owner(source);
    if (!owner)
    {
        owner = calloc(1, sizeof(*owner));
        if (!owner)
            return NULL;
        owner->source = source;
        owner->builder = builder;
        owner->source->lpVtbl->AddRef(owner->source);
        if (sqz_geometry_install)
        {
            sqz_geometry_install(source, builder);
            owner->tracked = TRUE;
        }
        owner->dirty = TRUE;
        owner->next = owners;
        owners = owner;
    }
    if (!owner->mesh || (builder && (owner->dirty || !owner->tracked)))
    {
        IDirect3DRMMesh *mesh = NULL;
        if (builder)
        {
            IDirect3DRMMeshBuilder *native = source;
            if (FAILED(native->lpVtbl->CreateMesh(native, &mesh)))
                return NULL;
        }
        else
        {
            mesh = source;
            mesh->lpVtbl->AddRef(mesh);
        }
        if (owner->mesh)
            owner->mesh->lpVtbl->Release(owner->mesh);
        owner->mesh = mesh;
    }
    owner->dirty = FALSE;
    return owner->mesh;
}

__declspec(dllexport) SceneGeometry *sqz_geometry_group(void *source, unsigned id,
                                                        D3DRMRENDERQUALITY quality, BOOL *rebuilt)
{
    *rebuilt = FALSE;
    GeometryOwner *owner = find_owner(source);
    if (!owner || !owner->mesh)
        return NULL;
    GeometryGroup *group;
    for (group = owner->groups; group && group->group != id; group = group->next)
        ;
    if (!group)
    {
        group = calloc(1, sizeof(*group));
        if (!group)
            return NULL;
        group->group = id;
        group->dirty = TRUE;
        group->next = owner->groups;
        owner->groups = group;
    }
    if (owner->tracked && !group->dirty && group->quality == quality)
        return &group->geometry;
    unsigned nv = 0, nf = 0, vpf = 0;
    DWORD bytes = 0;
    IDirect3DRMMesh *mesh = owner->mesh;
    if (FAILED(mesh->lpVtbl->GetGroup(mesh, id, &nv, &nf, &vpf, &bytes, NULL)) || nv > 1000000 ||
        nf > 1000000 || bytes > 4000000)
        return NULL;
    if (!nv || !nf)
    {
        free(group->source);
        free(group->indices);
        free(group->geometry.vertices);
        group->source = NULL;
        group->indices = NULL;
        group->geometry.vertices = NULL;
        group->geometry.count = 0;
        ++group->geometry.revision;
        group->quality = quality;
        group->dirty = FALSE;
        *rebuilt = TRUE;
        return &group->geometry;
    }
    size_t words = bytes > nf * (vpf ? vpf : 4) ? bytes : nf * (vpf ? vpf : 4);
    unsigned *indices = calloc(words + 1, sizeof(*indices));
    D3DRMVERTEX *vertices = calloc(nv ? nv : 1, sizeof(*vertices));
    if (!indices || !vertices ||
        FAILED(mesh->lpVtbl->GetGroup(mesh, id, &nv, &nf, &vpf, &bytes, indices)) ||
        FAILED(mesh->lpVtbl->GetVertices(mesh, id, 0, nv, vertices)))
    {
        free(indices);
        free(vertices);
        return NULL;
    }
    if (!group->dirty && group->quality == quality && group->nv == nv && group->nf == nf &&
        group->vpf == vpf && group->words == words &&
        !memcmp(group->source, vertices, nv * sizeof(*vertices)) &&
        !memcmp(group->indices, indices, words * sizeof(*indices)))
    {
        free(indices);
        free(vertices);
        return &group->geometry;
    }
    SceneVertex *expanded = NULL;
    size_t count = 0, capacity = 0, cursor = 0;
    for (unsigned face = 0; face < nf; ++face)
    {
        unsigned corners = vpf;
        if (!corners)
        {
            if (cursor >= words)
                goto failed;
            corners = indices[cursor++];
        }
        if (corners < 3 || corners > 4096 || cursor + corners > words)
            goto failed;
        for (unsigned k = 1; k + 1 < corners; ++k)
        {
            unsigned tri[3] = {indices[cursor], indices[cursor + k], indices[cursor + k + 1]};
            if (tri[0] >= nv || tri[1] >= nv || tri[2] >= nv || count + 3 > 4000000)
                goto failed;
            if (count + 3 > capacity)
            {
                capacity = capacity ? capacity * 2 : 256;
                SceneVertex *allocation = realloc(expanded, capacity * sizeof(*expanded));
                if (!allocation)
                    goto failed;
                expanded = allocation;
            }
            D3DVECTOR a = vertices[tri[0]].position, b = vertices[tri[1]].position,
                      c = vertices[tri[2]].position;
            D3DVECTOR face_normal = {.x = (b.y - a.y) * (c.z - a.z) - (b.z - a.z) * (c.y - a.y),
                                     .y = (b.z - a.z) * (c.x - a.x) - (b.x - a.x) * (c.z - a.z),
                                     .z = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)};
            for (unsigned j = 0; j < 3; ++j)
            {
                D3DRMVERTEX *v = &vertices[tri[j]];
                D3DVECTOR n =
                    (quality & D3DRMSHADE_MASK) == D3DRMSHADE_FLAT ? face_normal : v->normal;
                if (!isfinite(v->position.x) || !isfinite(v->position.y) ||
                    !isfinite(v->position.z))
                    goto failed;
                expanded[count++] =
                    (SceneVertex){.clip = {v->position.x, v->position.y, v->position.z, 1},
                                  .uv = {v->tu, v->tv},
                                  .color = {1, 1, 1, 1},
                                  .normal = {n.x, n.y, n.z, 0}};
            }
        }
        cursor += corners;
    }
    free(group->source);
    free(group->indices);
    free(group->geometry.vertices);
    group->source = vertices;
    group->indices = indices;
    group->nv = nv;
    group->nf = nf;
    group->vpf = vpf;
    group->words = words;
    group->quality = quality;
    group->geometry.vertices = expanded;
    group->geometry.count = (unsigned)count;
    ++group->geometry.revision;
    group->dirty = FALSE;
    *rebuilt = TRUE;
    return &group->geometry;
failed:
    free(expanded);
    free(vertices);
    free(indices);
    return NULL;
}
__declspec(dllexport) void sqz_scene_cache_clear(void)
{
    while (owners)
    {
        GeometryOwner *owner = owners;
        owners = owner->next;
        while (owner->groups)
        {
            GeometryGroup *group = owner->groups;
            owner->groups = group->next;
            free(group->source);
            free(group->indices);
            free(group->geometry.vertices);
            free(group);
        }
        if (owner->mesh)
            owner->mesh->lpVtbl->Release(owner->mesh);
        owner->source->lpVtbl->Release(owner->source);
        free(owner);
    }
}
