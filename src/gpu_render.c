/* SDL3 GPU scene renderer. RM remains the scene owner; this module owns only GPU
 * resources and cached texture references. Frame flow is deliberately synchronous:
 * DD background -> RGBA upload -> mesh/decal draws -> RGBA readback -> DD surface.
 * That preserves the original CPU warp and the HUD/GDI composition order. */
#define SDL_MAIN_HANDLED
#include "scene_gpu.h"
#include "scene_shader.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <d3dcompiler.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GPU_FUNCTIONS(X)                                                                           \
    X(SDL_Init)                                                                                    \
    X(SDL_WasInit)                                                                                 \
    X(SDL_QuitSubSystem)                                                                           \
    X(SDL_SetMainReady)                                                                            \
    X(SDL_CreateGPUDevice)                                                                         \
    X(SDL_DestroyGPUDevice)                                                                        \
    X(SDL_GetGPUDeviceDriver)                                                                      \
    X(SDL_GetError)                                                                                \
    X(SDL_CreateGPUShader)                                                                         \
    X(SDL_ReleaseGPUShader)                                                                        \
    X(SDL_CreateGPUGraphicsPipeline) X(SDL_ReleaseGPUGraphicsPipeline) X(SDL_CreateGPUTexture)     \
        X(SDL_ReleaseGPUTexture) X(SDL_CreateGPUSampler) X(SDL_ReleaseGPUSampler)                  \
            X(SDL_CreateGPUBuffer) X(SDL_ReleaseGPUBuffer) X(SDL_CreateGPUTransferBuffer)          \
                X(SDL_ReleaseGPUTransferBuffer) X(SDL_MapGPUTransferBuffer)                        \
                    X(SDL_UnmapGPUTransferBuffer) X(SDL_AcquireGPUCommandBuffer)                   \
                        X(SDL_BeginGPUCopyPass) X(SDL_EndGPUCopyPass) X(SDL_UploadToGPUTexture)    \
                            X(SDL_UploadToGPUBuffer) X(SDL_DownloadFromGPUTexture)                 \
                                X(SDL_SubmitGPUCommandBuffer)                                      \
                                    X(SDL_SubmitGPUCommandBufferAndAcquireFence)                   \
                                        X(SDL_WaitForGPUFences) X(SDL_ReleaseGPUFence)             \
                                            X(SDL_BeginGPURenderPass) X(SDL_EndGPURenderPass)      \
                                                X(SDL_BindGPUGraphicsPipeline)                     \
                                                    X(SDL_BindGPUVertexBuffers)                    \
                                                        X(SDL_BindGPUFragmentSamplers)             \
                                                            X(SDL_DrawGPUPrimitives)               \
                                                                X(SDL_SetGPUViewport)              \
                                                                    X(SDL_CancelGPUCommandBuffer)
#define DECLARE(name) static __typeof__(&name) p_##name;
DECLARE(SDL_PushGPUVertexUniformData)
GPU_FUNCTIONS(DECLARE)
/* Keep native source snapshots as well as GPU textures. A fast byte comparison
 * detects mutable pixels/palettes before conversion; each texture/decal pair is
 * resolved only once per draw. RM references protect against recycled pointers. */
typedef struct TextureRecord
{
    IDirect3DRMTexture *rm;
    SDL_GPUTexture *gpu;
    uint64_t resolved_frame;
    BYTE *source;
    D3DRMIMAGE description;
    DWORD key;
    BOOL keyed;
    unsigned width, height;
    BOOL decal;
} TextureRecord;
static SDL_GPUDevice *gpu;
static SDL_GPUGraphicsPipeline *pipeline;
static SDL_GPUGraphicsPipeline *decal_pipeline;
static SDL_GPUSampler *sampler;
static SDL_GPUTexture *color_target, *depth_target, *white;
static SDL_GPUBuffer *vertex_buffer;
static Uint32 buffer_size;
static unsigned target_width, target_height;
static TextureRecord textures[1024];
static unsigned texture_count;
static uint64_t texture_frame;
static char last_error[512];
static BOOL own_video;
typedef struct GeometryBuffer
{
    SceneGeometry *geometry;
    SDL_GPUBuffer *buffer;
    Uint32 size;
    uint64_t revision;
    BOOL pending;
    Uint32 upload_offset;
    struct GeometryBuffer *next;
} GeometryBuffer;
static GeometryBuffer *geometry_buffers;
typedef struct DrawUniform
{
    float basis[4][4], normals[3][4], color[4];
    unsigned mode[4], enabled[8];
} DrawUniform;
typedef struct LightUniform
{
    float vector[4], color[4], attenuation[4];
} LightUniform;
typedef struct ViewUniform
{
    float scale[4], offset[4];
    unsigned control[4];
    LightUniform lights[256];
} ViewUniform;

static BOOL gpu_error(const char *label)
{
    snprintf(last_error, sizeof(last_error), "%s: %s", label,
             p_SDL_GetError ? p_SDL_GetError() : "SDL GPU entry point unavailable");
    return FALSE;
}

static SDL_GPUShader *compile_shader(const char *source, const char *profile,
                                     SDL_GPUShaderStage stage)
{
    HMODULE compiler = LoadLibraryA("d3dcompiler_47.dll");
    if (!compiler)
    {
        gpu_error("d3dcompiler_47.dll is required");
        return NULL;
    }
    typedef HRESULT(WINAPI * Compile)(const void *, SIZE_T, const char *, const D3D_SHADER_MACRO *,
                                      ID3DInclude *, const char *, const char *, UINT, UINT,
                                      ID3DBlob **, ID3DBlob **);
    Compile compile = (Compile)GetProcAddress(compiler, "D3DCompile");
    ID3DBlob *code = NULL, *messages = NULL;
    HRESULT hr = compile ? compile(source, strlen(source), "sqz-scene", NULL, NULL, "main", profile,
                                   D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &code, &messages)
                         : E_FAIL;
    if (FAILED(hr))
    {
        snprintf(last_error, sizeof(last_error), "%s shader compile: %.400s", profile,
                 messages ? (char *)messages->lpVtbl->GetBufferPointer(messages)
                          : "D3DCompile unavailable");
        if (messages)
            messages->lpVtbl->Release(messages);
        FreeLibrary(compiler);
        return NULL;
    }
    SDL_GPUShaderCreateInfo desc = {.code_size = code->lpVtbl->GetBufferSize(code),
                                    .code = code->lpVtbl->GetBufferPointer(code),
                                    .entrypoint = "main",
                                    .format = SDL_GPU_SHADERFORMAT_DXBC,
                                    .stage = stage,
                                    .num_samplers = stage == SDL_GPU_SHADERSTAGE_FRAGMENT ? 1 : 0,
                                    .num_uniform_buffers =
                                        stage == SDL_GPU_SHADERSTAGE_VERTEX ? 2 : 0};
    SDL_GPUShader *result = p_SDL_CreateGPUShader(gpu, &desc);
    code->lpVtbl->Release(code);
    if (messages)
        messages->lpVtbl->Release(messages);
    FreeLibrary(compiler);
    if (!result)
        gpu_error("SDL_CreateGPUShader");
    return result;
}
static SDL_GPUTexture *create_texture(unsigned w, unsigned h, SDL_GPUTextureFormat format,
                                      SDL_GPUTextureUsageFlags usage)
{
    SDL_GPUTextureCreateInfo desc = {.type = SDL_GPU_TEXTURETYPE_2D,
                                     .format = format,
                                     .usage = usage,
                                     .width = w,
                                     .height = h,
                                     .layer_count_or_depth = 1,
                                     .num_levels = 1,
                                     .sample_count = SDL_GPU_SAMPLECOUNT_1};
    return p_SDL_CreateGPUTexture(gpu, &desc);
}
static BOOL upload_texture(SDL_GPUTexture *to, const BYTE *rgba, unsigned w, unsigned h)
{
    SDL_GPUTransferBufferCreateInfo desc = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                                            .size = w * h * 4};
    SDL_GPUTransferBuffer *transfer = p_SDL_CreateGPUTransferBuffer(gpu, &desc);
    if (!transfer)
        return gpu_error("texture transfer allocation");
    void *mapped = p_SDL_MapGPUTransferBuffer(gpu, transfer, FALSE);
    if (!mapped)
    {
        p_SDL_ReleaseGPUTransferBuffer(gpu, transfer);
        return gpu_error("texture transfer mapping");
    }
    memcpy(mapped, rgba, w * h * 4);
    p_SDL_UnmapGPUTransferBuffer(gpu, transfer);
    SDL_GPUCommandBuffer *command = p_SDL_AcquireGPUCommandBuffer(gpu);
    if (!command)
    {
        p_SDL_ReleaseGPUTransferBuffer(gpu, transfer);
        return gpu_error("texture command allocation");
    }
    SDL_GPUCopyPass *copy = p_SDL_BeginGPUCopyPass(command);
    SDL_GPUTextureTransferInfo from = {.transfer_buffer = transfer};
    SDL_GPUTextureRegion region = {.texture = to, .w = w, .h = h, .d = 1};
    p_SDL_UploadToGPUTexture(copy, &from, &region, FALSE);
    p_SDL_EndGPUCopyPass(copy);
    BOOL ok = p_SDL_SubmitGPUCommandBuffer(command);
    p_SDL_ReleaseGPUTransferBuffer(gpu, transfer);
    return ok ? TRUE : gpu_error("texture upload submission");
}
static BOOL initialize_gpu(void)
{
    if (gpu)
        return TRUE;
    HMODULE library = LoadLibraryA("SDL3.dll");
    if (!library)
        return gpu_error("SDL3.dll load");
#define LOAD(name)                                                                                 \
    p_##name = (void *)GetProcAddress(library, #name);                                             \
    if (!p_##name)                                                                                 \
        return gpu_error(#name);
    GPU_FUNCTIONS(LOAD)
    LOAD(SDL_PushGPUVertexUniformData)
#undef LOAD
    if (!(p_SDL_WasInit(SDL_INIT_VIDEO) & SDL_INIT_VIDEO))
    {
        p_SDL_SetMainReady();
        if (!p_SDL_Init(SDL_INIT_VIDEO))
            return gpu_error("SDL video initialization");
        own_video = TRUE;
    }
    gpu = p_SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_DXBC, FALSE, "direct3d12");
    if (!gpu)
        return gpu_error("SDL GPU Direct3D12 device creation");
    /* Mesh buffers retain local positions/normals. Vertex uniforms carry current
     * placement, normal transforms and lights for per-vertex Gouraud shading.
     * Decals and the CPU oracle use the projected passthrough branch. */
    const char *ps =
        "Texture2D tex:register(t0,space2); SamplerState smp:register(s0,space2);"
        "float4 main(float4 pos:SV_Position,float2 uv:TEXCOORD0,float4 color:TEXCOORD1):SV_Target0"
        "{ float4 t=tex.Sample(smp,uv); t.rgb/=max(t.a,0.000001); float4 c=t*color; clip(c.a-0.5); "
        "return c; }";
    SDL_GPUShader *vertex =
        compile_shader(scene_vertex_shader, "vs_5_1", SDL_GPU_SHADERSTAGE_VERTEX);
    SDL_GPUShader *fragment = compile_shader(ps, "ps_5_1", SDL_GPU_SHADERSTAGE_FRAGMENT);
    if (!vertex || !fragment)
    {
        if (vertex)
            p_SDL_ReleaseGPUShader(gpu, vertex);
        if (fragment)
            p_SDL_ReleaseGPUShader(gpu, fragment);
        return FALSE;
    }
    SDL_GPUVertexBufferDescription buffer = {
        .slot = 0, .pitch = sizeof(SceneVertex), .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX};
    SDL_GPUVertexAttribute attrs[4] = {{.location = 0,
                                        .buffer_slot = 0,
                                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                                        .offset = 0},
                                       {.location = 1,
                                        .buffer_slot = 0,
                                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                                        .offset = 16},
                                       {.location = 2,
                                        .buffer_slot = 0,
                                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                                        .offset = 24},
                                       {.location = 3,
                                        .buffer_slot = 0,
                                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                                        .offset = 40}};
    SDL_GPUColorTargetDescription color = {.format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM};
    SDL_GPUGraphicsPipelineCreateInfo info = {
        .vertex_shader = vertex,
        .fragment_shader = fragment,
        .vertex_input_state = {.vertex_buffer_descriptions = &buffer,
                               .num_vertex_buffers = 1,
                               .vertex_attributes = attrs,
                               .num_vertex_attributes = 4},
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = {.fill_mode = SDL_GPU_FILLMODE_FILL,
                             .cull_mode = SDL_GPU_CULLMODE_BACK,
                             .front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
                             .enable_depth_clip = TRUE},
        .multisample_state = {.sample_count = SDL_GPU_SAMPLECOUNT_1},
        .depth_stencil_state = {.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
                                .enable_depth_test = TRUE,
                                .enable_depth_write = TRUE},
        .target_info = {.color_target_descriptions = &color,
                        .num_color_targets = 1,
                        .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
                        .has_depth_stencil_target = TRUE}};
    /* RM discards reverse-facing mesh triangles. Morphing can expose their
     * inner surfaces when all triangles are rendered double-sided. Billboards
     * retain the separate, two-sided pipeline used by the foliage path. */
    pipeline = p_SDL_CreateGPUGraphicsPipeline(gpu, &info);
    info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    decal_pipeline = p_SDL_CreateGPUGraphicsPipeline(gpu, &info);
    p_SDL_ReleaseGPUShader(gpu, vertex);
    p_SDL_ReleaseGPUShader(gpu, fragment);
    if (!pipeline || !decal_pipeline)
        return gpu_error("scene pipeline creation");
    SDL_GPUSamplerCreateInfo sampling = {.min_filter = SDL_GPU_FILTER_LINEAR,
                                         .mag_filter = SDL_GPU_FILTER_LINEAR,
                                         .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
                                         .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                                         .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                                         .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                                         .max_lod = 0};
    sampler = p_SDL_CreateGPUSampler(gpu, &sampling);
    white =
        create_texture(1, 1, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER);
    BYTE pixel[4] = {255, 255, 255, 255};
    if (!sampler || !white || !upload_texture(white, pixel, 1, 1))
        return gpu_error("default sampler/texture creation");
    return TRUE;
}
/* Convert using the image/surface's actual masks, never an assumed desktop bpp. */
static unsigned unpack_channel(DWORD value, DWORD mask)
{
    if (!mask)
        return 0;
    unsigned shift = 0;
    while (!(mask & 1))
    {
        mask >>= 1;
        ++shift;
    }
    return (((value >> shift) & mask) * 255u + mask / 2) / mask;
}
static DWORD pack_channel(unsigned value, DWORD mask)
{
    if (!mask)
        return 0;
    unsigned shift = 0;
    while (!(mask & 1))
    {
        mask >>= 1;
        ++shift;
    }
    return ((value * mask + 127) / 255) << shift;
}
static BOOL same_texture_source(const TextureRecord *record, const D3DRMIMAGE *image, BOOL keyed,
                                DWORD key)
{
    const D3DRMIMAGE *old = &record->description;
    if (!record->source || !record->gpu || old->width != image->width ||
        old->height != image->height || old->depth != image->depth || old->rgb != image->rgb ||
        old->red_mask != image->red_mask || old->green_mask != image->green_mask ||
        old->blue_mask != image->blue_mask || old->alpha_mask != image->alpha_mask ||
        record->keyed != keyed || record->key != key ||
        (!image->rgb && old->palette_size != image->palette_size))
        return FALSE;
    size_t row_bytes = (size_t)image->width * (image->depth / 8);
    for (int y = 0; y < image->height; ++y)
        if (memcmp(record->source + y * row_bytes,
                   (BYTE *)image->buffer1 + (ptrdiff_t)y * image->bytes_per_line, row_bytes))
            return FALSE;
    return image->rgb || !memcmp(record->source + row_bytes * image->height, image->palette,
                                 (size_t)image->palette_size * sizeof(*image->palette));
}

static SDL_GPUTexture *resolve_rm_texture(SceneData *scene, IDirect3DRMTexture *rm, BOOL decal)
{
    if (!rm)
        return white;
    TextureRecord *record = NULL;
    for (unsigned i = 0; i < texture_count; ++i)
        if (textures[i].rm == rm && textures[i].decal == decal)
        {
            record = &textures[i];
            break;
        }
    if (record && record->gpu && record->resolved_frame == texture_frame)
    {
        ++scene->texture_reuses;
        return record->gpu;
    }
    ++scene->texture_checks;
    D3DRMIMAGE *image = rm->lpVtbl->GetImage(rm);
    if (!image || !image->buffer1 || image->width <= 0 || image->height <= 0 ||
        image->width > 8192 || image->height > 8192 ||
        (image->depth != 8 && image->depth != 16 && image->depth != 24 && image->depth != 32))
    {
        gpu_error("unsupported RM image");
        return NULL;
    }
    if (!image->rgb && (!image->palette || image->palette_size <= 0))
    {
        gpu_error("indexed texture lacks palette");
        return NULL;
    }
    BOOL keyed = decal && rm->lpVtbl->GetDecalTransparency(rm);
    DWORD key = keyed ? rm->lpVtbl->GetDecalTransparentColor(rm) : 0;
    if (record && same_texture_source(record, image, keyed, key))
    {
        record->resolved_frame = texture_frame;
        ++scene->texture_reuses;
        return record->gpu;
    }
    ++scene->texture_conversions;
    unsigned kr = (key >> 16) & 255, kg = (key >> 8) & 255, kb = key & 255;
    DWORD key_packed = pack_channel(kr, image->red_mask) | pack_channel(kg, image->green_mask) |
                       pack_channel(kb, image->blue_mask);
    DWORD rgb_mask = image->red_mask | image->green_mask | image->blue_mask;
    size_t size = (size_t)image->width * image->height * 4;
    BYTE *rgba = malloc(size);
    if (!rgba)
    {
        gpu_error("texture conversion allocation");
        return NULL;
    }
    for (int y = 0; y < image->height; ++y)
        for (int x = 0; x < image->width; ++x)
        {
            BYTE *src = (BYTE *)image->buffer1 + (ptrdiff_t)y * image->bytes_per_line +
                        x * (image->depth / 8);
            BYTE *dst = rgba + ((size_t)y * image->width + x) * 4;
            DWORD value = 0;
            memcpy(&value, src, image->depth / 8);
            if (image->rgb)
            {
                dst[0] = unpack_channel(value, image->red_mask);
                dst[1] = unpack_channel(value, image->green_mask);
                dst[2] = unpack_channel(value, image->blue_mask);
                dst[3] = image->alpha_mask ? unpack_channel(value, image->alpha_mask) : 255;
                if (keyed && (value & rgb_mask) == key_packed)
                    dst[3] = 0;
            }
            else
            {
                if (value >= (unsigned)image->palette_size)
                {
                    free(rgba);
                    gpu_error("palette index out of bounds");
                    return NULL;
                }
                D3DRMPALETTEENTRY p = image->palette[value];
                dst[0] = p.red;
                dst[1] = p.green;
                dst[2] = p.blue;
                dst[3] = keyed && p.red == kr && p.green == kg && p.blue == kb ? 0 : 255;
            }
            /* Filter premultiplied texels, then unpremultiply in the shader. A
             * transparent magenta texel contributes no color to foliage edges. */
            for (unsigned c = 0; c < 3; ++c)
                dst[c] = (dst[c] * dst[3] + 127) / 255;
        }
    size_t row_bytes = (size_t)image->width * (image->depth / 8);
    size_t pixel_bytes = row_bytes * image->height;
    size_t palette_bytes = image->rgb ? 0 : (size_t)image->palette_size * sizeof(*image->palette);
    BYTE *source = malloc(pixel_bytes + palette_bytes);
    if (!source)
    {
        free(rgba);
        gpu_error("texture source snapshot allocation");
        return NULL;
    }
    for (int y = 0; y < image->height; ++y)
        memcpy(source + y * row_bytes,
               (BYTE *)image->buffer1 + (ptrdiff_t)y * image->bytes_per_line, row_bytes);
    if (palette_bytes)
        memcpy(source + pixel_bytes, image->palette, palette_bytes);
    if (!record)
    {
        if (texture_count == 1024)
        {
            free(rgba);
            free(source);
            gpu_error("prototype texture cache limit reached");
            return NULL;
        }
        record = &textures[texture_count++];
        record->rm = rm;
        record->decal = decal;
        rm->lpVtbl->AddRef(rm);
    }
    if (!record->gpu || record->width != (unsigned)image->width ||
        record->height != (unsigned)image->height)
    {
        if (record->gpu)
            p_SDL_ReleaseGPUTexture(gpu, record->gpu);
        record->gpu =
            create_texture(image->width, image->height, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                           SDL_GPU_TEXTUREUSAGE_SAMPLER);
        record->width = image->width;
        record->height = image->height;
    }
    BOOL ok = record->gpu && upload_texture(record->gpu, rgba, image->width, image->height);
    if (ok)
    {
        free(record->source);
        record->source = source;
        record->description = *image;
        record->keyed = keyed;
        record->key = key;
        record->resolved_frame = texture_frame;
    }
    else
        free(source);
    free(rgba);
    return ok ? record->gpu : NULL;
}

static SDL_GPUBuffer *geometry_buffer(SceneGeometry *geometry)
{
    GeometryBuffer *record;
    for (record = geometry_buffers; record && record->geometry != geometry; record = record->next)
        ;
    if (!record)
    {
        record = calloc(1, sizeof(*record));
        if (!record)
            return NULL;
        record->geometry = geometry;
        record->next = geometry_buffers;
        geometry_buffers = record;
    }
    Uint32 bytes = geometry->count * sizeof(SceneVertex);
    if (!record->buffer || record->size < bytes)
    {
        if (record->buffer)
            p_SDL_ReleaseGPUBuffer(gpu, record->buffer);
        SDL_GPUBufferCreateInfo info = {.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = bytes};
        record->buffer = p_SDL_CreateGPUBuffer(gpu, &info);
        record->size = bytes;
        record->revision = 0;
    }
    if (!record->buffer)
        return NULL;
    record->pending = record->revision != geometry->revision;
    return record->buffer;
}

/* Called after the last retained device releases its target. Release our RM
 * references before destroying SDL; never shut down video owned by platform.c. */
void sqz_gpu_shutdown(void)
{
    if (!gpu)
    {
        sqz_scene_cache_clear();
        return;
    }
    while (geometry_buffers)
    {
        GeometryBuffer *record = geometry_buffers;
        geometry_buffers = record->next;
        if (record->buffer)
            p_SDL_ReleaseGPUBuffer(gpu, record->buffer);
        free(record);
    }
    sqz_scene_cache_clear();
    for (unsigned i = 0; i < texture_count; ++i)
    {
        free(textures[i].source);
        if (textures[i].gpu)
            p_SDL_ReleaseGPUTexture(gpu, textures[i].gpu);
        textures[i].rm->lpVtbl->Release(textures[i].rm);
    }
    memset(textures, 0, sizeof(textures));
    texture_count = 0;
    if (vertex_buffer)
        p_SDL_ReleaseGPUBuffer(gpu, vertex_buffer);
    if (color_target)
        p_SDL_ReleaseGPUTexture(gpu, color_target);
    if (depth_target)
        p_SDL_ReleaseGPUTexture(gpu, depth_target);
    if (white)
        p_SDL_ReleaseGPUTexture(gpu, white);
    if (sampler)
        p_SDL_ReleaseGPUSampler(gpu, sampler);
    if (pipeline)
        p_SDL_ReleaseGPUGraphicsPipeline(gpu, pipeline);
    if (decal_pipeline)
        p_SDL_ReleaseGPUGraphicsPipeline(gpu, decal_pipeline);
    decal_pipeline = NULL;
    p_SDL_DestroyGPUDevice(gpu);
    gpu = NULL;
    pipeline = NULL;
    sampler = NULL;
    white = NULL;
    color_target = NULL;
    depth_target = NULL;
    vertex_buffer = NULL;
    buffer_size = target_width = target_height = 0;
    if (own_video)
    {
        p_SDL_QuitSubSystem(SDL_INIT_VIDEO);
        own_video = FALSE;
    }
}

static BOOL native_bgra(const DDPIXELFORMAT *format)
{
    return format->dwRGBBitCount == 32 && format->dwRBitMask == 0xff0000 &&
           format->dwGBitMask == 0xff00 && format->dwBBitMask == 0xff && !format->dwRGBAlphaBitMask;
}

/* The ordinary 32-bit DD layout already matches a BGRA GPU target. Copy rows
 * directly, including the CPU-generated warp; retain conversion for other masks
 * and 16-bit surfaces. The unused X byte has no DirectDraw alpha semantics. */
static BOOL read_background(IDirectDrawSurface *target, BYTE **bgra, unsigned *width,
                            unsigned *height)
{
    DDSURFACEDESC desc = {.dwSize = sizeof(desc)};
    if (FAILED(target->lpVtbl->Lock(target, NULL, &desc, DDLOCK_WAIT, NULL)))
        return gpu_error("background surface lock");
    unsigned w = desc.dwWidth, h = desc.dwHeight, bytes = desc.ddpfPixelFormat.dwRGBBitCount / 8;
    if (!w || !h || w > 8192 || h > 8192 || (bytes != 2 && bytes != 4))
    {
        target->lpVtbl->Unlock(target, desc.lpSurface);
        return gpu_error("unsupported render target format");
    }
    size_t pixels = (size_t)w * h * 4;
    BYTE *background = malloc(pixels);
    if (!background)
    {
        target->lpVtbl->Unlock(target, desc.lpSurface);
        return gpu_error("background allocation");
    }
    if (native_bgra(&desc.ddpfPixelFormat))
    {
        for (unsigned y = 0; y < h; ++y)
            memcpy(background + (size_t)y * w * 4,
                   (BYTE *)desc.lpSurface + (ptrdiff_t)y * desc.lPitch, (size_t)w * 4);
    }
    else
        for (unsigned y = 0; y < h; ++y)
            for (unsigned x = 0; x < w; ++x)
            {
                DWORD v = 0;
                memcpy(&v, (BYTE *)desc.lpSurface + (ptrdiff_t)y * desc.lPitch + x * bytes, bytes);
                BYTE *out = background + ((size_t)y * w + x) * 4;
                out[0] = unpack_channel(v, desc.ddpfPixelFormat.dwBBitMask);
                out[1] = unpack_channel(v, desc.ddpfPixelFormat.dwGBitMask);
                out[2] = unpack_channel(v, desc.ddpfPixelFormat.dwRBitMask);
                out[3] = 255;
            }
    target->lpVtbl->Unlock(target, desc.lpSurface);
    *bgra = background;
    *width = w;
    *height = h;
    return TRUE;
}

/* Return the BGRA readback directly to matching DD rows. Fallback conversion
 * preserves the existing RGB555/RGB565 and uncommon 32-bit mask paths. */
static BOOL write_frame(IDirectDrawSurface *target, const BYTE *bgra, unsigned width,
                        unsigned height)
{
    DDSURFACEDESC desc = {.dwSize = sizeof(desc)};
    if (FAILED(target->lpVtbl->Lock(target, NULL, &desc, DDLOCK_WAIT, NULL)))
        return gpu_error("frame surface lock");
    unsigned bytes = desc.ddpfPixelFormat.dwRGBBitCount / 8;
    if (native_bgra(&desc.ddpfPixelFormat))
    {
        for (unsigned y = 0; y < height; ++y)
            memcpy((BYTE *)desc.lpSurface + (ptrdiff_t)y * desc.lPitch,
                   bgra + (size_t)y * width * 4, (size_t)width * 4);
    }
    else
        for (unsigned y = 0; y < height; ++y)
            for (unsigned x = 0; x < width; ++x)
            {
                const BYTE *in = bgra + ((size_t)y * width + x) * 4;
                DWORD value = pack_channel(in[2], desc.ddpfPixelFormat.dwRBitMask) |
                              pack_channel(in[1], desc.ddpfPixelFormat.dwGBitMask) |
                              pack_channel(in[0], desc.ddpfPixelFormat.dwBBitMask);
                memcpy((BYTE *)desc.lpSurface + (ptrdiff_t)y * desc.lPitch + x * bytes, &value,
                       bytes);
            }
    return SUCCEEDED(target->lpVtbl->Unlock(target, desc.lpSurface))
               ? TRUE
               : gpu_error("frame surface unlock");
}

static BOOL render_frame(SceneData *scene, IDirectDrawSurface *target)
{
    if (!initialize_gpu())
        return FALSE;
    if (++texture_frame == 0)
    {
        for (unsigned i = 0; i < texture_count; ++i)
            textures[i].resolved_frame = 0;
        ++texture_frame;
    }
    scene->texture_checks = scene->texture_conversions = scene->texture_reuses = 0;
    scene->geometry_uploads = 0;
    unsigned w, h;
    BYTE *background = NULL;
    if (!read_background(target, &background, &w, &h))
        return FALSE;
    size_t pixels = (size_t)w * h * 4;
    if (target_width != w || target_height != h)
    {
        if (color_target)
            p_SDL_ReleaseGPUTexture(gpu, color_target);
        if (depth_target)
            p_SDL_ReleaseGPUTexture(gpu, depth_target);
        color_target =
            create_texture(w, h, SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM,
                           SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER);
        depth_target = create_texture(w, h, SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
                                      SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET);
        target_width = w;
        target_height = h;
    }
    BOOL ok = color_target && depth_target && upload_texture(color_target, background, w, h);
    free(background);
    if (!ok)
        return gpu_error("frame target creation/upload");
    /* Resolve textures before the render pass: uploads submit separate commands. */
    SDL_GPUTexture **bindings =
        calloc(scene->batch_count ? scene->batch_count : 1, sizeof(*bindings));
    if (!bindings)
        return gpu_error("texture binding allocation");
    for (size_t i = 0; i < scene->batch_count; ++i)
        if (!(bindings[i] =
                  resolve_rm_texture(scene, scene->batches[i].texture, scene->batches[i].decal)))
        {
            free(bindings);
            return FALSE;
        }
    for (size_t i = 0; i < scene->batch_count; ++i)
        if (scene->batches[i].geometry &&
            !(scene->batches[i].gpu_geometry = geometry_buffer(scene->batches[i].geometry)))
        {
            free(bindings);
            return gpu_error("cached geometry upload");
        }
    Uint32 vertex_bytes = (Uint32)(scene->count * sizeof(SceneVertex));
    Uint32 upload_bytes = vertex_bytes;
    for (GeometryBuffer *record = geometry_buffers; record; record = record->next)
        if (record->pending)
        {
            record->upload_offset = upload_bytes;
            upload_bytes += record->geometry->count * sizeof(SceneVertex);
        }
    if (!vertex_buffer || buffer_size < vertex_bytes)
    {
        if (vertex_buffer)
            p_SDL_ReleaseGPUBuffer(gpu, vertex_buffer);
        buffer_size = vertex_bytes ? vertex_bytes : sizeof(SceneVertex);
        SDL_GPUBufferCreateInfo info = {.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = buffer_size};
        vertex_buffer = p_SDL_CreateGPUBuffer(gpu, &info);
    }
    /* Pack changed mesh groups and billboard vertices into one transfer buffer
     * and one copy pass. Static groups retain their previous GPU contents. */
    SDL_GPUTransferBufferCreateInfo upload_desc = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                                                   .size = upload_bytes ? upload_bytes
                                                                        : sizeof(SceneVertex)};
    SDL_GPUTransferBufferCreateInfo download_desc = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD,
                                                     .size = (Uint32)pixels};
    SDL_GPUTransferBuffer *upload = p_SDL_CreateGPUTransferBuffer(gpu, &upload_desc),
                          *download = p_SDL_CreateGPUTransferBuffer(gpu, &download_desc);
    if (!vertex_buffer || !upload || !download)
    {
        free(bindings);
        if (upload)
            p_SDL_ReleaseGPUTransferBuffer(gpu, upload);
        if (download)
            p_SDL_ReleaseGPUTransferBuffer(gpu, download);
        return gpu_error("frame buffer allocation");
    }
    void *mapped = p_SDL_MapGPUTransferBuffer(gpu, upload, FALSE);
    if (!mapped)
    {
        free(bindings);
        p_SDL_ReleaseGPUTransferBuffer(gpu, upload);
        p_SDL_ReleaseGPUTransferBuffer(gpu, download);
        return gpu_error("vertex map");
    }
    if (vertex_bytes)
        memcpy(mapped, scene->vertices, vertex_bytes);
    for (GeometryBuffer *record = geometry_buffers; record; record = record->next)
        if (record->pending)
            memcpy((BYTE *)mapped + record->upload_offset, record->geometry->vertices,
                   record->geometry->count * sizeof(SceneVertex));
    p_SDL_UnmapGPUTransferBuffer(gpu, upload);
    SDL_GPUCommandBuffer *command = p_SDL_AcquireGPUCommandBuffer(gpu);
    if (!command)
    {
        free(bindings);
        p_SDL_ReleaseGPUTransferBuffer(gpu, upload);
        p_SDL_ReleaseGPUTransferBuffer(gpu, download);
        return gpu_error("frame command allocation");
    }
    SDL_GPUCopyPass *copy = p_SDL_BeginGPUCopyPass(command);
    SDL_GPUTransferBufferLocation from = {.transfer_buffer = upload};
    SDL_GPUBufferRegion region = {.buffer = vertex_buffer, .size = vertex_bytes};
    if (vertex_bytes)
        p_SDL_UploadToGPUBuffer(copy, &from, &region, FALSE);
    for (GeometryBuffer *record = geometry_buffers; record; record = record->next)
        if (record->pending)
        {
            SDL_GPUTransferBufferLocation mesh_from = {.transfer_buffer = upload,
                                                       .offset = record->upload_offset};
            SDL_GPUBufferRegion mesh_to = {.buffer = record->buffer,
                                           .size = record->geometry->count * sizeof(SceneVertex)};
            p_SDL_UploadToGPUBuffer(copy, &mesh_from, &mesh_to, FALSE);
            record->revision = record->geometry->revision;
            record->pending = FALSE;
            ++scene->geometry_uploads;
        }
    p_SDL_EndGPUCopyPass(copy);
    /* Preserve the uploaded sky, clear only depth, then draw meshes and cutouts. */
    SDL_GPUColorTargetInfo color = {
        .texture = color_target, .load_op = SDL_GPU_LOADOP_LOAD, .store_op = SDL_GPU_STOREOP_STORE};
    SDL_GPUDepthStencilTargetInfo depth = {.texture = depth_target,
                                           .clear_depth = 1,
                                           .load_op = SDL_GPU_LOADOP_CLEAR,
                                           .store_op = SDL_GPU_STOREOP_DONT_CARE,
                                           .stencil_load_op = SDL_GPU_LOADOP_DONT_CARE,
                                           .stencil_store_op = SDL_GPU_STOREOP_DONT_CARE};
    ViewUniform view = {0};
    view.scale[0] = 2 * scene->front / (scene->right - scene->left);
    view.scale[1] = 2 * scene->front / (scene->top - scene->bottom);
    view.scale[2] = scene->back / (scene->back - scene->front);
    view.offset[0] = -(scene->right + scene->left) / (scene->right - scene->left);
    view.offset[1] = -(scene->top + scene->bottom) / (scene->top - scene->bottom);
    view.offset[2] = -scene->front * scene->back / (scene->back - scene->front);
    view.control[0] = (unsigned)scene->light_count;
    for (size_t i = 0; i < scene->light_count; ++i)
    {
        const SceneLight *light = &scene->lights[i];
        D3DVECTOR vector =
            light->type == D3DRMLIGHT_DIRECTIONAL ? light->direction : light->position;
        memcpy(view.lights[i].vector, &vector, sizeof(vector));
        memcpy(view.lights[i].color, light->color, sizeof(light->color));
        view.lights[i].color[3] = light->type == D3DRMLIGHT_AMBIENT       ? 0
                                  : light->type == D3DRMLIGHT_DIRECTIONAL ? 1
                                                                          : 2;
        memcpy(view.lights[i].attenuation, light->attenuation, sizeof(light->attenuation));
        view.lights[i].attenuation[3] = light->range;
    }
    p_SDL_PushGPUVertexUniformData(command, 1, &view, sizeof(view));
    SDL_GPURenderPass *pass = p_SDL_BeginGPURenderPass(command, &color, 1, &depth);
    p_SDL_BindGPUGraphicsPipeline(pass, pipeline);
    SDL_GPUViewport viewport = {.x = (float)scene->viewport->lpVtbl->GetX(scene->viewport),
                                .y = (float)scene->viewport->lpVtbl->GetY(scene->viewport),
                                .w = (float)scene->viewport->lpVtbl->GetWidth(scene->viewport),
                                .h = (float)scene->viewport->lpVtbl->GetHeight(scene->viewport),
                                .min_depth = 0,
                                .max_depth = 1};
    p_SDL_SetGPUViewport(pass, &viewport);
    for (size_t i = 0; i < scene->batch_count; ++i)
    {
        SceneBatch *batch = &scene->batches[i];
        DrawUniform draw = {0};
        if (batch->geometry)
        {
            for (unsigned axis = 0; axis < 4; ++axis)
                memcpy(draw.basis[axis], &batch->basis[axis], sizeof(D3DVECTOR));
            for (unsigned axis = 0; axis < 3; ++axis)
                memcpy(draw.normals[axis], &batch->normals[axis], sizeof(D3DVECTOR));
            memcpy(draw.color, batch->group_color, sizeof(draw.color));
            memcpy(draw.enabled, batch->light_mask, sizeof(draw.enabled));
            draw.mode[0] = 1;
            draw.mode[1] = batch->lit;
        }
        p_SDL_PushGPUVertexUniformData(command, 0, &draw, sizeof(draw));
        SDL_GPUBufferBinding vertex_binding = {.buffer = batch->geometry ? batch->gpu_geometry
                                                                         : vertex_buffer};
        p_SDL_BindGPUVertexBuffers(pass, 0, &vertex_binding, 1);
        p_SDL_BindGPUGraphicsPipeline(pass, scene->batches[i].decal ? decal_pipeline : pipeline);
        SDL_GPUTextureSamplerBinding binding = {.texture = bindings[i], .sampler = sampler};
        p_SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);
        p_SDL_DrawGPUPrimitives(pass, scene->batches[i].count, 1, scene->batches[i].first, 0);
    }
    free(bindings);
    p_SDL_EndGPURenderPass(pass);
    copy = p_SDL_BeginGPUCopyPass(command);
    SDL_GPUTextureRegion source = {.texture = color_target, .w = w, .h = h, .d = 1};
    SDL_GPUTextureTransferInfo destination = {.transfer_buffer = download};
    p_SDL_DownloadFromGPUTexture(copy, &source, &destination);
    p_SDL_EndGPUCopyPass(copy);
    /* The original game consumes the DD pixels immediately after Render returns. */
    SDL_GPUFence *fence = p_SDL_SubmitGPUCommandBufferAndAcquireFence(command);
    ok = fence && p_SDL_WaitForGPUFences(gpu, TRUE, &fence, 1);
    if (fence)
        p_SDL_ReleaseGPUFence(gpu, fence);
    p_SDL_ReleaseGPUTransferBuffer(gpu, upload);
    BYTE *result = ok ? p_SDL_MapGPUTransferBuffer(gpu, download, FALSE) : NULL;
    if (!result)
    {
        p_SDL_ReleaseGPUTransferBuffer(gpu, download);
        return gpu_error("frame readback");
    }
    ok = write_frame(target, result, w, h);
    p_SDL_UnmapGPUTransferBuffer(gpu, download);
    p_SDL_ReleaseGPUTransferBuffer(gpu, download);
    return ok;
}
BOOL sqz_gpu_draw(SceneData *scene, IDirectDrawSurface *target, char *message, size_t size)
{
    BOOL ok = render_frame(scene, target);
    if (!ok)
    {
        snprintf(message, size, "%s", last_error);
        sqz_gpu_shutdown();
    }
    return ok;
}
