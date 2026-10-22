/* Opt-in, one-frame native/GPU comparison. Never changes normal rendering.
 * Preserve raw execute buffers as well as decoded fields so interpretation can
 * be checked later. Native Render runs once; both images use the same RM pose. */
#include "scene_gpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *capture;
static char directory[64];
static unsigned execute_count;
static BOOL captured;

BOOL sqz_capture_requested(void)
{
    char value[8] = "";
    GetEnvironmentVariableA("SQZ_CAPTURE_SHADING", value, sizeof(value));
    return strcmp(value, "1") == 0;
}

static void inspect_group(IDirect3DRMFrame *frame, IDirect3DRMMesh *mesh, unsigned group,
                          const D3DRMVERTEX *vertices, unsigned count, D3DRMRENDERQUALITY quality)
{
    D3DVECTOR origin = {0}, world = {0};
    frame->lpVtbl->Transform(frame, &world, &origin);
    fprintf(capture, "FRAME_WORLD frame=%p origin=%g,%g,%g\n", frame, world.x, world.y, world.z);
    char frame_name[128] = "", mesh_name[128] = "";
    DWORD size = sizeof(frame_name);
    frame->lpVtbl->GetName(frame, &size, frame_name);
    size = sizeof(mesh_name);
    mesh->lpVtbl->GetName(mesh, &size, mesh_name);
    fprintf(capture,
            "GROUP frame=%p name=%s mesh=%p name=%s group=%u vertices=%u quality=%lx color=%08lx\n",
            frame, frame_name, mesh, mesh_name, group, count, (unsigned long)quality,
            (unsigned long)mesh->lpVtbl->GetGroupColor(mesh, group));
    IDirect3DRMMaterial *material = NULL;
    mesh->lpVtbl->GetGroupMaterial(mesh, group, &material);
    if (material)
    {
        float sr = 0, sg = 0, sb = 0, er = 0, eg = 0, eb = 0;
        material->lpVtbl->GetSpecular(material, &sr, &sg, &sb);
        material->lpVtbl->GetEmissive(material, &er, &eg, &eb);
        fprintf(capture, "MATERIAL power=%g specular=%g,%g,%g emissive=%g,%g,%g\n",
                material->lpVtbl->GetPower(material), sr, sg, sb, er, eg, eb);
        material->lpVtbl->Release(material);
    }
    else
        fprintf(capture, "MATERIAL none\n");
    for (unsigned i = 0; i < count; ++i)
    {
        const D3DRMVERTEX *v = &vertices[i];
        fprintf(capture, "RMVERT %u p=%g,%g,%g n=%g,%g,%g color=%08lx uv=%g,%g\n", i, v->position.x,
                v->position.y, v->position.z, v->normal.x, v->normal.y, v->normal.z,
                (unsigned long)v->color, v->tu, v->tv);
    }
}

void sqz_capture_execute(IDirect3DExecuteBuffer *buffer, BOOL after)
{
    if (!capture || execute_count >= 256)
        return;
    D3DEXECUTEDATA data = {.dwSize = sizeof(data)};
    D3DEXECUTEBUFFERDESC desc = {.dwSize = sizeof(desc)};
    if (FAILED(buffer->lpVtbl->GetExecuteData(buffer, &data)) ||
        FAILED(buffer->lpVtbl->Lock(buffer, &desc)))
        return;
    fprintf(capture, "EXEC %u %s bytes=%lu vertexOffset=%lu vertexCount=%lu instructions=%lu,%lu\n",
            execute_count, after ? "after" : "before", desc.dwBufferSize, data.dwVertexOffset,
            data.dwVertexCount, data.dwInstructionOffset, data.dwInstructionLength);
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/execute-%03u-%s.bin", directory, execute_count,
             after ? "after" : "before");
    FILE *raw = fopen(path, "wb");
    if (raw)
    {
        fwrite(desc.lpData, 1, desc.dwBufferSize, raw);
        fclose(raw);
    }
    BYTE *bytes = desc.lpData;
    size_t cursor = data.dwInstructionOffset, end = cursor + data.dwInstructionLength;
    if (end > desc.dwBufferSize)
        end = desc.dwBufferSize;
    while (cursor + sizeof(D3DINSTRUCTION) <= end)
    {
        D3DINSTRUCTION instruction;
        memcpy(&instruction, bytes + cursor, sizeof(instruction));
        cursor += sizeof(instruction);
        size_t length = (size_t)instruction.bSize * instruction.wCount;
        if (cursor + length > end)
            break;
        fprintf(capture, "OP %u size=%u count=%u\n", instruction.bOpcode, instruction.bSize,
                instruction.wCount);
        for (unsigned i = 0; i < instruction.wCount; ++i)
        {
            BYTE *record = bytes + cursor + i * instruction.bSize;
            if ((instruction.bOpcode == D3DOP_STATERENDER ||
                 instruction.bOpcode == D3DOP_STATELIGHT) &&
                instruction.bSize >= 8)
            {
                DWORD pair[2];
                memcpy(pair, record, 8);
                fprintf(capture, "STATE op=%u type=%lu value=%08lx\n", instruction.bOpcode, pair[0],
                        pair[1]);
            }
            if (instruction.bOpcode == D3DOP_PROCESSVERTICES &&
                instruction.bSize >= sizeof(D3DPROCESSVERTICES))
            {
                D3DPROCESSVERTICES process;
                memcpy(&process, record, sizeof(process));
                fprintf(capture, "PROCESS flags=%lx start=%u dest=%u count=%lu\n", process.dwFlags,
                        process.wStart, process.wDest, process.dwCount);
            }
        }
        cursor += length;
        if (instruction.bOpcode == D3DOP_EXIT)
            break;
    }
    /* Keep this interpretation explicitly labelled: PROCESS flags establish
     * whether the input really is TL, untransformed lit, or unlit vertices. */
    for (DWORD i = 0; i < data.dwVertexCount && i < 65536; ++i)
    {
        size_t offset = data.dwVertexOffset + (size_t)i * sizeof(D3DTLVERTEX);
        if (offset + sizeof(D3DTLVERTEX) > desc.dwBufferSize)
            break;
        D3DTLVERTEX v;
        memcpy(&v, bytes + offset, sizeof(v));
        fprintf(capture, "TLVIEW %u,%u,%lu,%g,%g,%g,%g,%08lx,%08lx,%g,%g\n", execute_count, after,
                i, v.sx, v.sy, v.sz, v.rhw, v.color, v.specular, v.tu, v.tv);
    }
    buffer->lpVtbl->Unlock(buffer);
    if (after)
        ++execute_count;
}

/* Snapshots store the surface description and tightly packed native rows. This
 * avoids GDI conversion when checking precisely what the renderer produced. */
static BYTE *surface_snapshot(IDirectDrawSurface *surface, const char *name, BOOL restore,
                              BYTE *saved)
{
    DDSURFACEDESC desc = {.dwSize = sizeof(desc)};
    if (!surface || FAILED(surface->lpVtbl->Lock(surface, NULL, &desc, DDLOCK_WAIT, NULL)))
        return NULL;
    unsigned row = desc.dwWidth * (desc.ddpfPixelFormat.dwRGBBitCount / 8);
    BYTE *pixels = restore ? saved : malloc((size_t)row * desc.dwHeight);
    if (pixels)
        for (unsigned y = 0; y < desc.dwHeight; ++y)
        {
            BYTE *native = (BYTE *)desc.lpSurface + (ptrdiff_t)y * desc.lPitch;
            if (restore)
                memcpy(native, pixels + (size_t)y * row, row);
            else
                memcpy(pixels + (size_t)y * row, native, row);
        }
    surface->lpVtbl->Unlock(surface, desc.lpSurface);
    if (!restore && pixels)
    {
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s/%s.surface", directory, name);
        FILE *file = fopen(path, "wb");
        if (file)
        {
            desc.lpSurface = NULL;
            fwrite(&desc, sizeof(desc), 1, file);
            fwrite(pixels, row, desc.dwHeight, file);
            fclose(file);
        }
    }
    return pixels;
}

HRESULT sqz_capture_render(const IDirect3DRMViewportVtbl *native, IDirect3DRMViewport *viewport,
                           IDirect3DRMFrame *root, IDirectDrawSurface *surface)
{
    if (captured || !sqz_capture_requested())
        return native->Render(viewport, root);
    /* Delay is opt-in so gameplay can populate the scene before its snapshot.
     * Count Render calls, not wall-clock time; no polling/helper thread is used. */
    static unsigned rendered_frames;
    char requested_frame[16] = "";
    GetEnvironmentVariableA("SQZ_CAPTURE_FRAME", requested_frame, sizeof(requested_frame));
    unsigned delay = (unsigned)strtoul(requested_frame, NULL, 10);
    if (delay > 10000)
        delay = 10000;
    if (rendered_frames++ < delay)
        return native->Render(viewport, root);
    captured = TRUE;
    snprintf(directory, sizeof(directory), "shading-capture-%lu", GetCurrentProcessId());
    CreateDirectoryA(directory, NULL);
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/capture.txt", directory);
    capture = fopen(path, "w");
    if (!capture)
        return native->Render(viewport, root);
    SceneData scene = {0};
    sqz_inspect_group = inspect_group;
    BOOL extracted = sqz_scene_extract(&scene, viewport, root);
    sqz_inspect_group = NULL;
    fprintf(capture, "SCENE extracted=%d error=%s quality=%lx meshes=%u lights=%lu\n", extracted,
            scene.error, (unsigned long)scene.default_quality, scene.meshes,
            (unsigned long)scene.light_count);
    for (size_t i = 0; i < scene.light_count; ++i)
    {
        SceneLight *l = &scene.lights[i];
        fprintf(capture, "LIGHT type=%u rgb=%g,%g,%g direction=%g,%g,%g\n", l->type, l->color[0],
                l->color[1], l->color[2], l->direction.x, l->direction.y, l->direction.z);
    }
    snprintf(path, sizeof(path), "%s/gpu-vertices.csv", directory);
    FILE *csv = fopen(path, "w");
    if (csv)
    {
        fprintf(csv, "batch,decal,vertex,x,y,z,r,g,b,a\n");
        for (size_t b = 0; b < scene.batch_count; ++b)
            for (unsigned j = 0; j < scene.batches[b].count; ++j)
            {
                unsigned i = scene.batches[b].first + j;
                SceneVertex *v = &scene.vertices[i];
                float w = v->clip[3];
                fprintf(csv, "%lu,%d,%u,%g,%g,%g,%g,%g,%g,%g\n", (unsigned long)b,
                        scene.batches[b].decal, i,
                        viewport->lpVtbl->GetX(viewport) +
                            (v->clip[0] / w + 1) * viewport->lpVtbl->GetWidth(viewport) / 2,
                        viewport->lpVtbl->GetY(viewport) +
                            (1 - v->clip[1] / w) * viewport->lpVtbl->GetHeight(viewport) / 2,
                        v->clip[2] / w, v->color[0], v->color[1], v->color[2], v->color[3]);
            }
        fclose(csv);
    }
    IDirect3DRMDevice *device = NULL;
    IDirect3DDevice *immediate = NULL;
    if (SUCCEEDED(viewport->lpVtbl->GetDevice(viewport, &device)) && device)
    {
        if (SUCCEEDED(device->lpVtbl->GetDirect3DDevice(device, &immediate)) && immediate)
            sqz_hook(immediate, "IDirect3DDevice");
        device->lpVtbl->Release(device);
    }
    BYTE *background = surface_snapshot(surface, "background", FALSE, NULL);
    HRESULT result = native->Render(viewport, root);
    BYTE *native_pixels = surface_snapshot(surface, "native", FALSE, NULL);
    if (background && native_pixels && extracted)
    {
        surface_snapshot(surface, NULL, TRUE, background);
        char error[512] = "";
        BOOL ok = sqz_gpu_draw(&scene, surface, error, sizeof(error));
        fprintf(capture, "GPU_DRAW ok=%d error=%s\n", ok, error);
        free(surface_snapshot(surface, "gpu", FALSE, NULL));
        surface_snapshot(surface, NULL, TRUE, native_pixels);
    }
    fprintf(capture, "END native_hresult=%08lx execute_count=%u\n", result, execute_count);
    free(background);
    free(native_pixels);
    if (immediate)
        immediate->lpVtbl->Release(immediate);
    sqz_scene_free(&scene);
    fclose(capture);
    capture = NULL;
    return result;
}
