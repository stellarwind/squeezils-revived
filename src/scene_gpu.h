#ifndef SQZ_SCENE_GPU_H
#define SQZ_SCENE_GPU_H
#include "runtime.h"
#include <stdint.h>
typedef struct SceneVertex
{
    float clip[4], uv[2], color[4], normal[4];
} SceneVertex;

typedef struct SceneGeometry
{
    SceneVertex *vertices;
    unsigned count;
    uint64_t revision;
} SceneGeometry;
typedef struct SceneBatch
{
    unsigned first, count;
    IDirect3DRMTexture *texture;
    BOOL decal;
    SceneGeometry *geometry;
    void *gpu_geometry;
    D3DVECTOR basis[4], normals[3];
    float group_color[4];
    DWORD light_mask[8];
    BOOL lit;
} SceneBatch;
/* Light positions/directions and mesh normals share camera coordinates. Native
 * light objects remain owned by RM; only optional enable-frame refs are kept. */
typedef struct SceneLight
{
    D3DRMLIGHTTYPE type;
    float color[3], range, attenuation[3];
    D3DVECTOR position, direction;
    IDirect3DRMFrame *enable_frame;
} SceneLight;
typedef struct SceneVisit
{
    IDirect3DRMFrame *frame;
    D3DVECTOR basis[4];
} SceneVisit;

enum SceneProfileStage
{
    PROFILE_EXTRACT,
    PROFILE_INITIALIZE,
    PROFILE_BACKGROUND_READ,
    PROFILE_BACKGROUND_UPLOAD,
    PROFILE_TEXTURES,
    PROFILE_VERTEX_SETUP,
    PROFILE_DRAW_ENCODE,
    PROFILE_SUBMIT,
    PROFILE_FENCE_WAIT,
    PROFILE_READBACK_MAP,
    PROFILE_WRITEBACK,
    PROFILE_CLEANUP,
    PROFILE_STAGE_COUNT
};

typedef struct SceneData
{
    SceneVertex *vertices;
    size_t count, capacity;
    SceneBatch *batches;
    size_t batch_count, batch_capacity;
    unsigned frames, meshes, decals;
    char error[160];
    SceneVisit *visited;
    size_t visited_count, visited_capacity;
    SceneLight *lights;
    size_t light_count, light_capacity;
    D3DRMRENDERQUALITY default_quality;
    IDirect3DRMViewport *viewport;
    IDirect3DRMFrame *camera;
    float left, right, bottom, top, front, back;
    LONGLONG profile_tick, profile_ticks[PROFILE_STAGE_COUNT];
    unsigned texture_checks, texture_conversions, texture_reuses;
    BOOL cached_geometry;
    unsigned cached_vertices, geometry_builds, geometry_uploads;
} SceneData;
extern void (*sqz_geometry_install)(void *, BOOL);
IDirect3DRMMesh *sqz_geometry_mesh(void *, BOOL);
SceneGeometry *sqz_geometry_group(void *, unsigned, D3DRMRENDERQUALITY, BOOL *);
void sqz_scene_geometry_changed(void *);
void sqz_scene_cache_clear(void);
LONGLONG sqz_gpu_profile_clock(void);
void sqz_gpu_profile_mark(SceneData *, enum SceneProfileStage);
void sqz_gpu_profile_finish(const SceneData *);
/* Optional single-frame inspection callback; NULL during normal play. */
extern void (*sqz_inspect_group)(IDirect3DRMFrame *, IDirect3DRMMesh *, unsigned,
                                 const D3DRMVERTEX *, unsigned, D3DRMRENDERQUALITY);
BOOL sqz_capture_requested(void);
void sqz_capture_execute(IDirect3DExecuteBuffer *buffer, BOOL after);
HRESULT sqz_capture_render(const IDirect3DRMViewportVtbl *, IDirect3DRMViewport *,
                           IDirect3DRMFrame *, IDirectDrawSurface *);
BOOL sqz_scene_extract(SceneData *, IDirect3DRMViewport *, IDirect3DRMFrame *);
BOOL sqz_scene_extract_cached(SceneData *, IDirect3DRMViewport *, IDirect3DRMFrame *);
void sqz_scene_free(SceneData *);
BOOL sqz_gpu_enabled(void);
void sqz_gpu_bind_device(IDirect3DRMDevice *, IDirectDrawSurface *);
HRESULT sqz_gpu_render(const IDirect3DRMViewportVtbl *, IDirect3DRMViewport *, IDirect3DRMFrame *);
BOOL sqz_gpu_draw(SceneData *, IDirectDrawSurface *, char *, size_t);
void sqz_gpu_shutdown(void);
#endif
