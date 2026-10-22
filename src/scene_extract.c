/* Read native retained objects afresh each frame, including mutable mesh vertices. */
#include "scene_gpu.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void (*sqz_inspect_group)(IDirect3DRMFrame *, IDirect3DRMMesh *, unsigned, const D3DRMVERTEX *,
                          unsigned, D3DRMRENDERQUALITY);

static BOOL scene_error(SceneData *s, const char *message)
{
    snprintf(s->error, sizeof(s->error), "%s", message);
    return FALSE;
}
static BOOL reserve_array(void **data, size_t *capacity, size_t count, size_t item)
{
    if (count <= *capacity)
        return TRUE;
    size_t n = *capacity ? *capacity * 2 : 256;
    if (n < count)
        n = count;
    if (n > 4000000 || item > SIZE_MAX / n)
        return FALSE;
    void *p = realloc(*data, n * item);
    if (!p)
        return FALSE;
    *data = p;
    *capacity = n;
    return TRUE;
}
static void color_to_rgba(D3DCOLOR color, float out[4])
{
    out[0] = ((color >> 16) & 255) / 255.f;
    out[1] = ((color >> 8) & 255) / 255.f;
    out[2] = (color & 255) / 255.f;
    out[3] = ((color >> 24) & 255) / 255.f;
}
/* Multiply local matrices along the real parent chain, without probing unit
 * points in world space. At overworld distances those translated float points
 * lose low bits; subtracting them to recover axes makes nearby objects jitter. */
static BOOL world_matrix(SceneData *s, IDirect3DRMFrame *frame, double out[4][4])
{
    memset(out, 0, 16 * sizeof(double));
    for (unsigned i = 0; i < 4; ++i)
        out[i][i] = 1;
    frame->lpVtbl->AddRef(frame);
    for (unsigned depth = 0; frame; ++depth)
    {
        D3DRMMATRIX4D local;
        IDirect3DRMFrame *parent = NULL;
        if (depth >= 128 || FAILED(frame->lpVtbl->GetTransform(frame, local)) ||
            FAILED(frame->lpVtbl->GetParent(frame, &parent)))
        {
            frame->lpVtbl->Release(frame);
            if (parent)
                parent->lpVtbl->Release(parent);
            return scene_error(s, "frame parent matrix read failed");
        }
        double product[4][4] = {{0}};
        for (unsigned i = 0; i < 4; ++i)
            for (unsigned j = 0; j < 4; ++j)
                for (unsigned k = 0; k < 4; ++k)
                    product[i][j] += out[i][k] * local[k][j];
        memcpy(out, product, sizeof(product));
        frame->lpVtbl->Release(frame);
        frame = parent;
    }
    return TRUE;
}
static BOOL camera_basis(SceneData *s, IDirect3DRMFrame *frame, D3DVECTOR out[4])
{
    double world[4][4], camera[4][4], inverse[3][3];
    if (!world_matrix(s, frame, world) || !world_matrix(s, s->camera, camera))
        return FALSE;
    /* Cofactors of the camera's linear transform. Unlike a transpose, this also
     * handles scaled camera ancestors. Keep translation separate until after
     * subtracting camera position, and retain double precision until output. */
    for (unsigned i = 0; i < 3; ++i)
        for (unsigned j = 0; j < 3; ++j)
            inverse[j][i] = camera[(i + 1) % 3][(j + 1) % 3] * camera[(i + 2) % 3][(j + 2) % 3] -
                            camera[(i + 1) % 3][(j + 2) % 3] * camera[(i + 2) % 3][(j + 1) % 3];
    double determinant = 0;
    for (unsigned i = 0; i < 3; ++i)
        determinant += camera[0][i] * inverse[i][0];
    if (!isfinite(determinant) || fabs(determinant) < 1e-20)
        return scene_error(s, "singular camera transform");
    for (unsigned i = 0; i < 4; ++i)
    {
        double transformed[3] = {0};
        unsigned row = i ? i - 1 : 3;
        for (unsigned j = 0; j < 3; ++j)
            for (unsigned k = 0; k < 3; ++k)
                transformed[j] +=
                    (world[row][k] - (i ? 0 : camera[3][k])) * inverse[k][j] / determinant;
        out[i] = (D3DVECTOR){
            .x = (float)transformed[0], .y = (float)transformed[1], .z = (float)transformed[2]};
    }
    return TRUE;
}
static D3DVECTOR transform_position(const D3DVECTOR b[4], D3DVECTOR p)
{
    return (D3DVECTOR){.x = b[0].x + b[1].x * p.x + b[2].x * p.y + b[3].x * p.z,
                       .y = b[0].y + b[1].y * p.x + b[2].y * p.y + b[3].y * p.z,
                       .z = b[0].z + b[1].z * p.x + b[2].z * p.y + b[3].z * p.z};
}
/* Frames used as visuals are instances. Their cached world Transform is filled
 * by native Render, which our GPU path bypasses. Compose the local matrix along
 * the actual traversal path instead, including the placement frame's transform. */
static BOOL placed_basis(SceneData *s, IDirect3DRMFrame *frame, const D3DVECTOR parent[4],
                         D3DVECTOR out[4])
{
    if (!parent)
        return camera_basis(s, frame, out);
    D3DRMMATRIX4D local;
    if (FAILED(frame->lpVtbl->GetTransform(frame, local)))
        return scene_error(s, "local frame transform read failed");
    out[0] = transform_position(parent,
                                (D3DVECTOR){.x = local[3][0], .y = local[3][1], .z = local[3][2]});
    for (unsigned i = 0; i < 3; ++i)
    {
        out[i + 1] = (D3DVECTOR){
            .x = parent[1].x * local[i][0] + parent[2].x * local[i][1] + parent[3].x * local[i][2],
            .y = parent[1].y * local[i][0] + parent[2].y * local[i][1] + parent[3].y * local[i][2],
            .z = parent[1].z * local[i][0] + parent[2].z * local[i][1] + parent[3].z * local[i][2]};
    }
    return TRUE;
}
static float dot(D3DVECTOR a, D3DVECTOR b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
static D3DVECTOR cross(D3DVECTOR a, D3DVECTOR b)
{
    return (D3DVECTOR){
        .x = a.y * b.z - a.z * b.y, .y = a.z * b.x - a.x * b.z, .z = a.x * b.y - a.y * b.x};
}
static D3DVECTOR normalized(D3DVECTOR v)
{
    float length = sqrtf(dot(v, v));
    if (length > 1e-12f)
    {
        v.x /= length;
        v.y /= length;
        v.z /= length;
    }
    else
        v = (D3DVECTOR){0};
    return v;
}
/* Normals need the inverse transpose, not the position matrix: otherwise a
 * nonuniformly scaled Squeezil is lit as if its surface were pointing elsewhere. */
static D3DVECTOR transform_normal(const D3DVECTOR b[4], D3DVECTOR n)
{
    D3DVECTOR x = cross(b[2], b[3]), y = cross(b[3], b[1]), z = cross(b[1], b[2]);
    float det = dot(b[1], x);
    if (fabsf(det) < 1e-12f)
        return (D3DVECTOR){0};
    return normalized((D3DVECTOR){.x = (n.x * x.x + n.y * y.x + n.z * z.x) / det,
                                  .y = (n.x * x.y + n.y * y.y + n.z * z.y) / det,
                                  .z = (n.x * x.z + n.y * y.z + n.z * z.z) / det});
}
static BOOL light_applies_to_frame(IDirect3DRMFrame *frame, IDirect3DRMFrame *scope)
{
    if (!scope)
        return TRUE;
    IDirect3DRMFrame *current = frame;
    current->lpVtbl->AddRef(current);
    for (unsigned depth = 0; current && depth < 128; ++depth)
    {
        if (current == scope)
        {
            current->lpVtbl->Release(current);
            return TRUE;
        }
        IDirect3DRMFrame *parent = NULL;
        current->lpVtbl->GetParent(current, &parent);
        current->lpVtbl->Release(current);
        current = parent;
    }
    if (current)
        current->lpVtbl->Release(current);
    return FALSE;
}
/* RM directional lights point along their owning frame's +Z axis; incoming
 * light is the opposite direction. Read it each frame so RM animation still works. */
static BOOL collect_lights(SceneData *s, IDirect3DRMFrame *frame, const D3DVECTOR b[4])
{
    IDirect3DRMLightArray *lights = NULL;
    if (FAILED(frame->lpVtbl->GetLights(frame, &lights)))
        return scene_error(s, "frame light array read failed");
    if (!lights)
        return TRUE;
    BOOL ok = TRUE;
    for (DWORD i = 0; ok && i < lights->lpVtbl->GetSize(lights); ++i)
    {
        IDirect3DRMLight *light = NULL;
        if (FAILED(lights->lpVtbl->GetElement(lights, i, &light)))
        {
            ok = FALSE;
            break;
        }
        if (s->light_count >= 256 || !reserve_array((void **)&s->lights, &s->light_capacity,
                                                    s->light_count + 1, sizeof(SceneLight)))
        {
            light->lpVtbl->Release(light);
            ok = scene_error(s, "scene light capacity exceeded");
            break;
        }
        SceneLight *out = &s->lights[s->light_count++];
        memset(out, 0, sizeof(*out));
        out->type = light->lpVtbl->GetType(light);
        float color[4];
        color_to_rgba(light->lpVtbl->GetColor(light), color);
        memcpy(out->color, color, sizeof(out->color));
        light->lpVtbl->GetEnableFrame(light, &out->enable_frame);
        if (out->type == D3DRMLIGHT_DIRECTIONAL || out->type == D3DRMLIGHT_POINT)
        {
            out->position = b[0];
            out->direction = normalized(b[3]);
            out->range = light->lpVtbl->GetRange(light);
            out->attenuation[0] = light->lpVtbl->GetConstantAttenuation(light);
            out->attenuation[1] = light->lpVtbl->GetLinearAttenuation(light);
            out->attenuation[2] = light->lpVtbl->GetQuadraticAttenuation(light);
        }
        else if (out->type != D3DRMLIGHT_AMBIENT)
            ok = scene_error(s, "unsupported light type (ambient/directional/point supported)");
        light->lpVtbl->Release(light);
    }
    lights->lpVtbl->Release(lights);
    return ok;
}
/* Gouraud: calculate diffuse illumination at the original mesh vertices. The
 * existing vertex-color stream carries it to the GPU for smooth interpolation. */
static void shade_vertex(const SceneData *s, const BOOL *enabled, D3DVECTOR p, D3DVECTOR normal,
                         float color[4])
{
    float illumination[3] = {0, 0, 0};
    for (size_t i = 0; i < s->light_count; ++i)
    {
        if (!enabled[i])
            continue;
        const SceneLight *light = &s->lights[i];
        float amount = 1;
        if (light->type == D3DRMLIGHT_DIRECTIONAL)
            amount = fmaxf(0, -dot(normal, light->direction));
        else if (light->type == D3DRMLIGHT_POINT)
        {
            D3DVECTOR delta = {.x = light->position.x - p.x,
                               .y = light->position.y - p.y,
                               .z = light->position.z - p.z};
            float distance = sqrtf(dot(delta, delta));
            if (light->range > 0 && distance > light->range)
                continue;
            float attenuation = light->attenuation[0] + light->attenuation[1] * distance +
                                light->attenuation[2] * distance * distance;
            amount = fmaxf(0, dot(normal, normalized(delta))) / (attenuation > 0 ? attenuation : 1);
        }
        for (unsigned c = 0; c < 3; ++c)
            illumination[c] += light->color[c] * amount;
    }
    for (unsigned c = 0; c < 3; ++c)
        color[c] = fminf(1, fmaxf(0, color[c] * illumination[c]));
}
static BOOL append_vertex(SceneData *s, D3DVECTOR p, float u, float v, const float color[4])
{
    if (!isfinite(p.x) || !isfinite(p.y) || !isfinite(p.z))
        return scene_error(s, "nonfinite retained vertex");
    if (!reserve_array((void **)&s->vertices, &s->capacity, s->count + 1, sizeof(SceneVertex)))
        return scene_error(s, "scene vertex capacity exceeded");
    SceneVertex *out = &s->vertices[s->count++];
    float n = s->front, f = s->back;
    out->clip[0] =
        2 * n * p.x / (s->right - s->left) - (s->right + s->left) * p.z / (s->right - s->left);
    out->clip[1] =
        2 * n * p.y / (s->top - s->bottom) - (s->top + s->bottom) * p.z / (s->top - s->bottom);
    out->clip[2] = f / (f - n) * p.z - n * f / (f - n);
    out->clip[3] = p.z;
    out->uv[0] = u;
    out->uv[1] = v;
    memcpy(out->color, color, 4 * sizeof(float));
    return TRUE;
}
static SceneBatch *append_batch(SceneData *s, IDirect3DRMTexture *texture, BOOL decal)
{
    if (!reserve_array((void **)&s->batches, &s->batch_capacity, s->batch_count + 1,
                       sizeof(SceneBatch)))
        return NULL;
    SceneBatch *b = &s->batches[s->batch_count++];
    *b = (SceneBatch){(unsigned)s->count, 0, texture, decal};
    if (texture)
        texture->lpVtbl->AddRef(texture);
    return b;
}
static BOOL extract_mesh(SceneData *s, IDirect3DRMMesh *mesh, IDirect3DRMFrame *frame,
                         IDirect3DRMTexture *inherited, const D3DVECTOR b[4])
{
    BOOL enabled[256];
    for (size_t i = 0; i < s->light_count; ++i)
        enabled[i] = light_applies_to_frame(frame, s->lights[i].enable_frame);
    unsigned groups = mesh->lpVtbl->GetGroupCount(mesh);
    if (groups > 10000)
        return scene_error(s, "mesh group count exceeds prototype limit");
    ++s->meshes;
    for (unsigned g = 0; g < groups; ++g)
    {
        unsigned nv = 0, nf = 0, vpf = 0;
        DWORD bytes = 0;
        if (FAILED(mesh->lpVtbl->GetGroup(mesh, g, &nv, &nf, &vpf, &bytes, NULL)) || nv > 1000000 ||
            nf > 1000000 || bytes > 4000000)
            return scene_error(s, "mesh group description read failed");
        if (!nv || !nf)
            continue;
        size_t words = bytes > nf * (vpf ? vpf : 4) ? bytes : nf * (vpf ? vpf : 4);
        unsigned *indices = calloc(words + 1, sizeof(unsigned));
        D3DRMVERTEX *vertices = calloc(nv, sizeof(*vertices));
        if (!indices || !vertices)
        {
            free(indices);
            free(vertices);
            return scene_error(s, "mesh allocation failed");
        }
        BOOL ok = SUCCEEDED(mesh->lpVtbl->GetGroup(mesh, g, &nv, &nf, &vpf, &bytes, indices)) &&
                  SUCCEEDED(mesh->lpVtbl->GetVertices(mesh, g, 0, nv, vertices));
        IDirect3DRMTexture *texture = NULL;
        if (ok)
            mesh->lpVtbl->GetGroupTexture(mesh, g, &texture);
        SceneBatch *draw = ok ? append_batch(s, texture ? texture : inherited, FALSE) : NULL;
        if (texture)
            texture->lpVtbl->Release(texture);
        if (!draw)
            ok = FALSE;
        float group[4];
        color_to_rgba(mesh->lpVtbl->GetGroupColor(mesh, g), group);
        D3DRMRENDERQUALITY quality = mesh->lpVtbl->GetGroupQuality(mesh, g);
        if (!quality)
            quality = s->default_quality;
        if (ok && sqz_inspect_group)
            sqz_inspect_group(frame, mesh, g, vertices, nv, quality);
        BOOL lit = (quality & D3DRMLIGHT_MASK) != D3DRMLIGHT_OFF;
        size_t cursor = 0;
        for (unsigned face = 0; ok && face < nf; ++face)
        {
            unsigned corners = vpf;
            if (!corners)
            {
                if (cursor >= words)
                {
                    ok = FALSE;
                    break;
                }
                corners = indices[cursor++];
            }
            if (corners < 3 || corners > 4096 || cursor + corners > words)
            {
                ok = FALSE;
                break;
            }
            for (unsigned k = 1; ok && k + 1 < corners; ++k)
            {
                unsigned tri[3] = {indices[cursor], indices[cursor + k], indices[cursor + k + 1]};
                if (tri[0] >= nv || tri[1] >= nv || tri[2] >= nv)
                {
                    ok = FALSE;
                    break;
                }
                D3DVECTOR a = transform_position(b, vertices[tri[0]].position),
                          edge1 = transform_position(b, vertices[tri[1]].position),
                          edge2 = transform_position(b, vertices[tri[2]].position);
                edge1.x -= a.x;
                edge1.y -= a.y;
                edge1.z -= a.z;
                edge2.x -= a.x;
                edge2.y -= a.y;
                edge2.z -= a.z;
                D3DVECTOR face_normal = normalized(cross(edge1, edge2));
                for (unsigned j = 0; j < 3; ++j)
                {
                    if (tri[j] >= nv)
                    {
                        ok = FALSE;
                        break;
                    }
                    D3DRMVERTEX *v = &vertices[tri[j]];
                    float color[4];
                    /* Native RM Mesh rendering uses the group color. GetVertices
                     * can return stale or mixed source colors after SetGroupColor;
                     * multiplying those colors darkens the mesh and creates seams. */
                    memcpy(color, group, sizeof(color));
                    D3DVECTOR p = transform_position(b, v->position);
                    if (lit)
                    {
                        D3DVECTOR normal = transform_normal(b, v->normal);
                        if ((quality & D3DRMSHADE_MASK) == D3DRMSHADE_FLAT ||
                            dot(normal, normal) < 1e-12f)
                            normal = face_normal;
                        shade_vertex(s, enabled, p, normal, color);
                    }
                    if (!append_vertex(s, p, v->tu, v->tv, color))
                    {
                        ok = FALSE;
                        break;
                    }
                }
            }
            cursor += corners;
        }
        if (draw)
            draw->count = (unsigned)s->count - draw->first;
        free(indices);
        free(vertices);
        if (!ok)
            return scene_error(s, "mesh polygon/vertex extraction failed");
    }
    return TRUE;
}
static BOOL extract_decal(SceneData *s, IDirect3DRMTexture *texture, const D3DVECTOR b[4])
{
    float width, height;
    LONG ox, oy;
    D3DRMIMAGE *image = texture->lpVtbl->GetImage(texture);
    if (!image || image->width <= 0 || image->height <= 0 ||
        FAILED(texture->lpVtbl->GetDecalSize(texture, &width, &height)) ||
        FAILED(texture->lpVtbl->GetDecalOrigin(texture, &ox, &oy)))
        return scene_error(s, "decal description read failed");
    if (!texture->lpVtbl->GetDecalScale(texture))
        return scene_error(s, "unscaled screen-space decal unsupported in first scene");
    /* Native RM uses the world dimensions stored by SetDecalSize. Frame scale
     * moves the anchor through the hierarchy but does not resize the billboard.
     * Multiplying its basis lengths here double-scales overworld tree sprites. */
    float left = b[0].x - width * ox / image->width, top = b[0].y + height * oy / image->height;
    D3DVECTOR p[4] = {{.x = left, .y = top, .z = b[0].z},
                      {.x = left + width, .y = top, .z = b[0].z},
                      {.x = left + width, .y = top - height, .z = b[0].z},
                      {.x = left, .y = top - height, .z = b[0].z}};
    unsigned tri[6] = {0, 1, 2, 0, 2, 3};
    float color[4] = {1, 1, 1, 1};
    SceneBatch *draw = append_batch(s, texture, TRUE);
    if (!draw)
        return scene_error(s, "decal batch allocation failed");
    for (unsigned i = 0; i < 6; ++i)
    {
        unsigned j = tri[i];
        if (!append_vertex(s, p[j], j == 1 || j == 2, j >= 2, color))
            return FALSE;
    }
    draw->count = 6;
    ++s->decals;
    return TRUE;
}
/* Deduplicate a child/visual alias at the same placement, but allow a shared
 * visual frame to be rendered under different placement transforms.
 * First collect all lights, then reset visitation and extract geometry, so sibling
 * ordering cannot change illumination. Inherited textures apply only to meshes. */
static BOOL walk_frames(SceneData *s, IDirect3DRMFrame *frame, IDirect3DRMTexture *inherited,
                        unsigned depth, BOOL lights_only, const D3DVECTOR parent[4])
{
    D3DVECTOR b[4];
    if (!placed_basis(s, frame, parent, b))
        return FALSE;
    for (size_t i = 0; i < s->visited_count; ++i)
        if (s->visited[i].frame == frame && memcmp(s->visited[i].basis, b, sizeof(b)) == 0)
            return TRUE;
    if (!reserve_array((void **)&s->visited, &s->visited_capacity, s->visited_count + 1,
                       sizeof(*s->visited)))
        return scene_error(s, "frame visitation allocation failed");
    SceneVisit *visit = &s->visited[s->visited_count++];
    visit->frame = frame;
    memcpy(visit->basis, b, sizeof(b));
    if (depth > 128 || ++s->frames > 20000)
        return scene_error(s, "scene hierarchy exceeds prototype limits");
    if (lights_only && !collect_lights(s, frame, b))
        return FALSE;
    IDirect3DRMTexture *own = NULL;
    if (!lights_only)
        frame->lpVtbl->GetTexture(frame, &own);
    if (own)
        inherited = own;
    IDirect3DRMVisualArray *visuals = NULL;
    IDirect3DRMFrameArray *children = NULL;
    BOOL ok = SUCCEEDED(frame->lpVtbl->GetVisuals(frame, &visuals));
    if (ok && visuals)
        for (DWORD i = 0; ok && i < visuals->lpVtbl->GetSize(visuals); ++i)
        {
            IDirect3DRMVisual *v = NULL;
            IDirect3DRMMesh *m = NULL;
            IDirect3DRMMeshBuilder *builder = NULL;
            IDirect3DRMTexture *t = NULL;
            IDirect3DRMFrame *nested = NULL;
            if (FAILED(visuals->lpVtbl->GetElement(visuals, i, &v)))
            {
                ok = FALSE;
                break;
            }
            if (lights_only)
            {
                if (SUCCEEDED(
                        v->lpVtbl->QueryInterface(v, &IID_IDirect3DRMFrame, (void **)&nested)))
                    ok = walk_frames(s, nested, inherited, depth + 1, TRUE, b);
            }
            else if (SUCCEEDED(v->lpVtbl->QueryInterface(v, &IID_IDirect3DRMMesh, (void **)&m)))
                ok = extract_mesh(s, m, frame, inherited, b);
            else if (SUCCEEDED(v->lpVtbl->QueryInterface(v, &IID_IDirect3DRMMeshBuilder,
                                                         (void **)&builder)))
            {
                ok = SUCCEEDED(builder->lpVtbl->CreateMesh(builder, &m));
                if (ok)
                    ok = extract_mesh(s, m, frame, inherited, b);
            }
            else if (SUCCEEDED(
                         v->lpVtbl->QueryInterface(v, &IID_IDirect3DRMFrame, (void **)&nested)))
                ok = walk_frames(s, nested, inherited, depth + 1, FALSE, b);
            else if (SUCCEEDED(v->lpVtbl->QueryInterface(v, &IID_IDirect3DRMTexture, (void **)&t)))
                ok = extract_decal(s, t, b);
            else
            {
                char name[80] = "unknown";
                DWORD size = sizeof(name);
                v->lpVtbl->GetClassName(v, &size, name);
                snprintf(s->error, sizeof(s->error), "unsupported visual %.70s", name);
                ok = FALSE;
            }
            if (t)
                t->lpVtbl->Release(t);
            if (m)
                m->lpVtbl->Release(m);
            if (builder)
                builder->lpVtbl->Release(builder);
            if (nested)
                nested->lpVtbl->Release(nested);
            v->lpVtbl->Release(v);
        }
    if (visuals)
        visuals->lpVtbl->Release(visuals);
    if (ok)
        ok = SUCCEEDED(frame->lpVtbl->GetChildren(frame, &children));
    if (ok && children)
        for (DWORD i = 0; ok && i < children->lpVtbl->GetSize(children); ++i)
        {
            IDirect3DRMFrame *child = NULL;
            ok = SUCCEEDED(children->lpVtbl->GetElement(children, i, &child));
            if (ok)
            {
                ok = walk_frames(s, child, inherited, depth + 1, lights_only, b);
                child->lpVtbl->Release(child);
            }
        }
    if (children)
        children->lpVtbl->Release(children);
    if (own)
        own->lpVtbl->Release(own);
    return ok;
}
void sqz_scene_free(SceneData *s)
{
    for (size_t i = 0; i < s->batch_count; ++i)
        if (s->batches[i].texture)
            s->batches[i].texture->lpVtbl->Release(s->batches[i].texture);
    if (s->camera)
        s->camera->lpVtbl->Release(s->camera);
    for (size_t i = 0; i < s->light_count; ++i)
        if (s->lights[i].enable_frame)
            s->lights[i].enable_frame->lpVtbl->Release(s->lights[i].enable_frame);
    free(s->lights);
    free(s->vertices);
    free(s->batches);
    free(s->visited);
    memset(s, 0, sizeof(*s));
}
BOOL sqz_scene_extract(SceneData *s, IDirect3DRMViewport *viewport, IDirect3DRMFrame *root)
{
    memset(s, 0, sizeof(*s));
    s->viewport = viewport;
    if (FAILED(viewport->lpVtbl->GetCamera(viewport, &s->camera)) || !s->camera)
        return scene_error(s, "viewport camera missing");
    s->front = viewport->lpVtbl->GetFront(viewport);
    s->back = viewport->lpVtbl->GetBack(viewport);
    if (FAILED(viewport->lpVtbl->GetPlane(viewport, &s->left, &s->right, &s->bottom, &s->top)) ||
        s->front <= 0 || s->back <= s->front || s->right == s->left || s->top == s->bottom)
        return scene_error(s, "invalid viewport projection");
    s->default_quality = D3DRMRENDER_GOURAUD;
    IDirect3DRMDevice *device = NULL;
    if (viewport->lpVtbl->GetDevice && SUCCEEDED(viewport->lpVtbl->GetDevice(viewport, &device)) &&
        device)
    {
        s->default_quality = device->lpVtbl->GetQuality(device);
        device->lpVtbl->Release(device);
    }
    /* Collect all lights before visiting meshes, so sibling traversal order
     * cannot make a character lose illumination from a later light frame. */
    if (!walk_frames(s, root, NULL, 0, TRUE, NULL))
        return FALSE;
    s->visited_count = 0;
    s->frames = 0;
    return walk_frames(s, root, NULL, 0, FALSE, NULL);
}
