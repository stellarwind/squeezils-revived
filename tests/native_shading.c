/* Native RM is the oracle for color-source and Gouraud behavior. No window or input. */
#include "scene_gpu.h"
#ifdef SQZ_CACHED_ORACLE
#define sqz_scene_extract sqz_scene_extract_cached
#endif
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
void sqz_log(const char *f, ...)
{
    (void)f;
}
static DWORD pixel(IDirectDrawSurface *s, unsigned x, unsigned y)
{
    DDSURFACEDESC d = {.dwSize = sizeof(d)};
    DWORD p = 0;
    assert(SUCCEEDED(s->lpVtbl->Lock(s, NULL, &d, DDLOCK_WAIT, NULL)));
    memcpy(&p, (BYTE *)d.lpSurface + y * d.lPitch + x * (d.ddpfPixelFormat.dwRGBBitCount / 8),
           d.ddpfPixelFormat.dwRGBBitCount / 8);
    s->lpVtbl->Unlock(s, d.lpSurface);
    return p;
}

/* Use the game's original SQG vertex/groups, as its own loader submits them. */
static IDirect3DRMMesh *load_goodie(IDirect3DRM *rm, const char *path)
{
    FILE *f = fopen(path, "rb");
    assert(f);
    char header[15];
    assert(fread(header, 15, 1, f) == 1);
    unsigned n;
    assert(fread(&n, 4, 1, f) == 1);
    for (unsigned i = 0; i < n; ++i)
    {
        unsigned len;
        assert(fread(&len, 4, 1, f) == 1);
        assert(fseek(f, len, SEEK_CUR) == 0);
    }
    unsigned nv, nf, ni, ng;
    assert(fread(&nv, 4, 1, f) == 1);
    D3DRMVERTEX *v = calloc(nv, sizeof(*v));
    assert(v);
    assert(fread(v, sizeof(*v), nv, f) == nv);
    assert(fread(&nf, 4, 1, f) == 1);
    assert(fread(&ni, 4, 1, f) == 1);
    unsigned *indices = calloc(ni, 4);
    assert(indices);
    assert(fread(indices, 4, ni, f) == ni);
    assert(fread(&ng, 4, 1, f) == 1);
    IDirect3DRMMesh *mesh;
    assert(SUCCEEDED(rm->lpVtbl->CreateMesh(rm, &mesh)));
    for (unsigned g = 0; g < ng; ++g)
    {
        unsigned info[9];
        assert(fread(info, 36, 1, f) == 1);
        unsigned *local = calloc(info[8], 4);
        assert(local);
        memcpy(local, indices + info[7], info[8] * 4);
        unsigned c = 0;
        while (c < info[8])
        {
            unsigned corners = local[c++];
            for (unsigned j = 0; j < corners; ++j)
            {
                assert(c < info[8]);
                local[c++] -= info[4];
            }
        }
        D3DRMGROUPINDEX id;
        assert(SUCCEEDED(mesh->lpVtbl->AddGroup(mesh, info[5], info[6], 0, local, &id)));
        assert(SUCCEEDED(mesh->lpVtbl->SetVertices(mesh, id, 0, info[5], v + info[4])));
        assert(SUCCEEDED(mesh->lpVtbl->SetGroupColor(mesh, id, info[0])));
        free(local);
    }
    free(v);
    free(indices);
    fclose(f);
    return mesh;
}
static unsigned ink_count(IDirectDrawSurface *surface)
{
    unsigned ink = 0;
    for (unsigned y = 0; y < 64; ++y)
        for (unsigned x = 0; x < 64; ++x)
            if (pixel(surface, x, y))
                ++ink;
    return ink;
}
int main(void)
{
    IDirectDraw *dd;
    IDirectDrawSurface *target;
    IDirect3DRM *rm;
    IDirect3DRMDevice *device;
    IDirect3DRMFrame *root, *camera, *model;
    IDirect3DRMMesh *mesh;
    IDirect3DRMViewport *viewport;
    IDirect3DRMLight *ambient;
    assert(SUCCEEDED(DirectDrawCreate(NULL, &dd, NULL)));
    assert(SUCCEEDED(dd->lpVtbl->SetCooperativeLevel(dd, NULL, DDSCL_NORMAL)));
    DDSURFACEDESC desc = {
        .dwSize = sizeof(desc),
        .dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT,
        .dwWidth = 64,
        .dwHeight = 64,
        .ddsCaps = {DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY},
        .ddpfPixelFormat = {.dwSize = sizeof(DDPIXELFORMAT),
                            .dwFlags = DDPF_RGB,
                            .dwRGBBitCount = 16,
                            .dwRBitMask = 0xf800,
                            .dwGBitMask = 0x7e0,
                            .dwBBitMask = 0x1f}};
    assert(SUCCEEDED(dd->lpVtbl->CreateSurface(dd, &desc, &target, NULL)));
    assert(SUCCEEDED(Direct3DRMCreate(&rm)));
    HRESULT hr = rm->lpVtbl->CreateDeviceFromSurface(rm, (GUID *)&IID_IDirect3DRGBDevice, dd,
                                                     target, &device);
    printf("native device=%08lx\n", hr);
    assert(SUCCEEDED(hr));
    assert(SUCCEEDED(device->lpVtbl->SetQuality(device, D3DRMRENDER_GOURAUD)));
    assert(SUCCEEDED(rm->lpVtbl->CreateFrame(rm, NULL, &root)));
    assert(SUCCEEDED(rm->lpVtbl->CreateFrame(rm, root, &camera)));
    assert(SUCCEEDED(rm->lpVtbl->CreateFrame(rm, root, &model)));
    assert(SUCCEEDED(model->lpVtbl->SetPosition(model, root, 0, 0, 4)));
    assert(SUCCEEDED(rm->lpVtbl->CreateLightRGB(rm, D3DRMLIGHT_AMBIENT, 1, 1, 1, &ambient)));
    assert(SUCCEEDED(root->lpVtbl->AddLight(root, ambient)));
    assert(SUCCEEDED(rm->lpVtbl->CreateMesh(rm, &mesh)));
    unsigned face[3] = {0, 2, 1};
    D3DRMGROUPINDEX group;
    assert(SUCCEEDED(mesh->lpVtbl->AddGroup(mesh, 3, 1, 3, face, &group)));
    D3DRMVERTEX v[3] = {0};
    v[0].position = (D3DVECTOR){.x = -2, .y = -2};
    v[1].position = (D3DVECTOR){.x = 2, .y = -2};
    v[2].position = (D3DVECTOR){.y = 2};
    for (unsigned i = 0; i < 3; ++i)
        v[i].normal.z = -1;
    assert(SUCCEEDED(model->lpVtbl->AddVisual(model, (IDirect3DRMVisual *)mesh)));
    assert(SUCCEEDED(rm->lpVtbl->CreateViewport(rm, device, camera, 0, 0, 64, 64, &viewport)));
    assert(SUCCEEDED(viewport->lpVtbl->SetFront(viewport, 1)));
    assert(SUCCEEDED(viewport->lpVtbl->SetBack(viewport, 20)));
    assert(SUCCEEDED(viewport->lpVtbl->SetPlane(viewport, -1, 1, -1, 1)));
    DWORD cases[3][4] = {{0xffff0000, 0xffff0000, 0xffff0000, 0xffffffff},
                         {0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff0000ff},
                         {0xffff0000, 0xff00ff00, 0xff0000ff, 0xff808080}};
    for (unsigned c = 0; c < 3; ++c)
    {
        for (unsigned i = 0; i < 3; ++i)
            v[i].color = cases[c][i];
        assert(SUCCEEDED(mesh->lpVtbl->SetVertices(mesh, group, 0, 3, v)));
        assert(SUCCEEDED(mesh->lpVtbl->SetGroupColor(mesh, group, cases[c][3])));
        assert(SUCCEEDED(mesh->lpVtbl->SetGroupQuality(mesh, group, D3DRMRENDER_GOURAUD)));
        assert(SUCCEEDED(viewport->lpVtbl->Clear(viewport)));
        assert(SUCCEEDED(viewport->lpVtbl->Render(viewport, root)));
        DWORD native = pixel(target, 32, 32), left = pixel(target, 24, 40),
              right = pixel(target, 40, 40);
        SceneData scene = {0};
        assert(sqz_scene_extract(&scene, viewport, root));
        char error[512];
        assert(SUCCEEDED(viewport->lpVtbl->Clear(viewport)));
        assert(sqz_gpu_draw(&scene, target, error, sizeof(error)));
        printf("case %u group=%08lx vertex=%08lx/%08lx/%08lx native=%04lx left=%04lx right=%04lx "
               "GPU=%04lx\n",
               c, cases[c][3], v[0].color, v[1].color, v[2].color, native, left, right,
               pixel(target, 32, 32));
        assert(pixel(target, 32, 32) == native);
        sqz_scene_free(&scene);
    }

    IDirect3DRMLight *sun;
    IDirect3DRMFrame *sun_frame;
    assert(SUCCEEDED(ambient->lpVtbl->SetColorRGB(ambient, .2f, .2f, .2f)));
    assert(SUCCEEDED(rm->lpVtbl->CreateFrame(rm, root, &sun_frame)));
    assert(SUCCEEDED(rm->lpVtbl->CreateLightRGB(rm, D3DRMLIGHT_DIRECTIONAL, .6f, .6f, .6f, &sun)));
    assert(SUCCEEDED(sun_frame->lpVtbl->AddLight(sun_frame, sun)));
    v[0].normal = (D3DVECTOR){.z = -1};
    v[1].normal = (D3DVECTOR){.x = 1};
    v[2].normal = (D3DVECTOR){.z = 1};
    assert(SUCCEEDED(mesh->lpVtbl->SetVertices(mesh, group, 0, 3, v)));
    assert(SUCCEEDED(mesh->lpVtbl->SetGroupColor(mesh, group, 0xffb50021)));
    assert(SUCCEEDED(viewport->lpVtbl->Clear(viewport)));
    assert(SUCCEEDED(viewport->lpVtbl->Render(viewport, root)));
    DWORD native_center = pixel(target, 32, 32), native_left = pixel(target, 24, 40),
          native_right = pixel(target, 40, 40);
    SceneData lit = {0};
    assert(sqz_scene_extract(&lit, viewport, root));
    char message[512];
    assert(SUCCEEDED(viewport->lpVtbl->Clear(viewport)));
    assert(sqz_gpu_draw(&lit, target, message, sizeof(message)));
    printf(
        "lit native center=%04lx left=%04lx right=%04lx GPU center=%04lx left=%04lx right=%04lx\n",
        native_center, native_left, native_right, pixel(target, 32, 32), pixel(target, 24, 40),
        pixel(target, 40, 40));
    /* Native RGB565 dithering/rounding can differ by one component step. */
    DWORD native_samples[3] = {native_center, native_left, native_right};
    DWORD gpu_samples[3] = {pixel(target, 32, 32), pixel(target, 24, 40), pixel(target, 40, 40)};
    for (unsigned i = 0; i < 3; ++i)
    {
        int red = (int)((native_samples[i] >> 11) & 31) - (int)((gpu_samples[i] >> 11) & 31);
        int green = (int)((native_samples[i] >> 5) & 63) - (int)((gpu_samples[i] >> 5) & 63);
        int blue = (int)(native_samples[i] & 31) - (int)(gpu_samples[i] & 31);
        assert(red >= -1 && red <= 1 && green >= -1 && green <= 1 && blue >= -1 && blue <= 1);
    }
    puts("PASS: native group-color selection, mixed source colors ignored, Gouraud gradient within "
         "one RGB565 component step");
    sqz_scene_free(&lit);
    sun->lpVtbl->Release(sun);
    sun_frame->lpVtbl->Release(sun_frame);
    /* Compare reverse-wound geometry against RM, with identical vertices. */
    IDirect3DRMMesh *back_mesh;
    assert(SUCCEEDED(rm->lpVtbl->CreateMesh(rm, &back_mesh)));
    unsigned back_face[3] = {0, 1, 2};
    D3DRMGROUPINDEX back_group;
    assert(SUCCEEDED(back_mesh->lpVtbl->AddGroup(back_mesh, 3, 1, 3, back_face, &back_group)));
    assert(SUCCEEDED(back_mesh->lpVtbl->SetVertices(back_mesh, back_group, 0, 3, v)));
    assert(SUCCEEDED(back_mesh->lpVtbl->SetGroupColor(back_mesh, back_group, 0xffffffff)));
    assert(SUCCEEDED(model->lpVtbl->DeleteVisual(model, (IDirect3DRMVisual *)mesh)));
    assert(SUCCEEDED(model->lpVtbl->AddVisual(model, (IDirect3DRMVisual *)back_mesh)));
    assert(SUCCEEDED(viewport->lpVtbl->Clear(viewport)));
    DDBLTFX black = {.dwSize = sizeof(black)};
    assert(SUCCEEDED(
        target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &black)));
    assert(SUCCEEDED(viewport->lpVtbl->Render(viewport, root)));
    DWORD native_back = pixel(target, 32, 32);
    SceneData back_scene = {0};
    assert(sqz_scene_extract(&back_scene, viewport, root));
    assert(SUCCEEDED(viewport->lpVtbl->Clear(viewport)));
    assert(SUCCEEDED(
        target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &black)));
    assert(sqz_gpu_draw(&back_scene, target, message, sizeof(message)));
    printf("reverse winding native=%04lx GPU=%04lx\n", native_back, pixel(target, 32, 32));
    assert(native_back == 0 && pixel(target, 32, 32) == native_back);
    puts("PASS: native and GPU discard reverse-wound mesh triangles");
    sqz_scene_free(&back_scene);
    back_mesh->lpVtbl->Release(back_mesh);

    assert(SUCCEEDED(model->lpVtbl->DeleteVisual(model, (IDirect3DRMVisual *)back_mesh)));
    const char *assets[] = {"../game/SQZ/GOODIES/NUT1.SQG", "../game/SQZ/GOODIES/SPIT.SQG"};
    for (unsigned i = 0; i < 2; ++i)
    {
        IDirect3DRMMesh *goodie = load_goodie(rm, assets[i]);
        IDirect3DRMFrame *template_frame;
        assert(SUCCEEDED(rm->lpVtbl->CreateFrame(rm, NULL, &template_frame)));
        assert(SUCCEEDED(
            template_frame->lpVtbl->AddVisual(template_frame, (IDirect3DRMVisual *)goodie)));
        assert(SUCCEEDED(model->lpVtbl->AddVisual(model, (IDirect3DRMVisual *)template_frame)));
        assert(SUCCEEDED(model->lpVtbl->AddScale(model, D3DRMCOMBINE_REPLACE, 10, 10, 10)));
        assert(SUCCEEDED(model->lpVtbl->SetPosition(model, root, 0, -.5f, 4)));
        assert(SUCCEEDED(
            target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &black)));
        SceneData goodie_scene = {0};
        assert(sqz_scene_extract(&goodie_scene, viewport, root));
        assert(SUCCEEDED(viewport->lpVtbl->Clear(viewport)));
        assert(SUCCEEDED(viewport->lpVtbl->Render(viewport, root)));
        unsigned native_ink = ink_count(target);
        assert(SUCCEEDED(
            target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &black)));
        assert(sqz_gpu_draw(&goodie_scene, target, message, sizeof(message)));
        unsigned gpu_ink = ink_count(target);
        printf("asset %s native ink=%u GPU ink=%u\n", assets[i], native_ink, gpu_ink);
        assert(native_ink > 0 && gpu_ink > 0);
        assert(abs((int)native_ink - (int)gpu_ink) <= 10);
        sqz_scene_free(&goodie_scene);
        assert(SUCCEEDED(model->lpVtbl->DeleteVisual(model, (IDirect3DRMVisual *)template_frame)));
        goodie->lpVtbl->Release(goodie);
        template_frame->lpVtbl->Release(template_frame);
    }
    IDirect3DRMMeshBuilder *builder;
    IDirect3DRMFace *builder_face;
    assert(SUCCEEDED(rm->lpVtbl->CreateMeshBuilder(rm, &builder)));
    for (unsigned i = 0; i < 3; ++i)
    {
        assert(builder->lpVtbl->AddVertex(builder, v[i].position.x, v[i].position.y,
                                          v[i].position.z) >= 0);
        assert(builder->lpVtbl->AddNormal(builder, 0, 0, -1) >= 0);
    }
    assert(SUCCEEDED(builder->lpVtbl->CreateFace(builder, &builder_face)));
    assert(SUCCEEDED(builder_face->lpVtbl->AddVertexAndNormalIndexed(builder_face, 0, 0)));
    assert(SUCCEEDED(builder_face->lpVtbl->AddVertexAndNormalIndexed(builder_face, 2, 2)));
    assert(SUCCEEDED(builder_face->lpVtbl->AddVertexAndNormalIndexed(builder_face, 1, 1)));
    assert(SUCCEEDED(builder_face->lpVtbl->SetColorRGB(builder_face, 1, .5f, 0)));
    assert(SUCCEEDED(model->lpVtbl->AddScale(model, D3DRMCOMBINE_REPLACE, 1, 1, 1)));
    assert(SUCCEEDED(model->lpVtbl->SetPosition(model, root, 0, 0, 4)));
    assert(SUCCEEDED(model->lpVtbl->AddVisual(model, (IDirect3DRMVisual *)builder)));
    assert(SUCCEEDED(
        target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &black)));
    assert(SUCCEEDED(viewport->lpVtbl->Clear(viewport)));
    assert(SUCCEEDED(viewport->lpVtbl->Render(viewport, root)));
    unsigned builder_native = ink_count(target);
    SceneData builder_scene = {0};
    assert(sqz_scene_extract(&builder_scene, viewport, root));
    assert(SUCCEEDED(
        target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &black)));
    assert(sqz_gpu_draw(&builder_scene, target, message, sizeof(message)));
    float *builder_color = builder_scene.cached_geometry ? builder_scene.batches[0].group_color
                                                         : builder_scene.vertices[0].color;
    printf("builder native ink=%u GPU ink=%u groups=%lu color=%g,%g,%g,%g\n", builder_native,
           ink_count(target), (unsigned long)builder_scene.batch_count, builder_color[0],
           builder_color[1], builder_color[2], builder_color[3]);
    assert(builder_native > 0 && ink_count(target) > 0);
    sqz_scene_free(&builder_scene);
    builder_face->lpVtbl->Release(builder_face);
    builder->lpVtbl->Release(builder);
    assert(SUCCEEDED(model->lpVtbl->DeleteVisual(model, (IDirect3DRMVisual *)builder)));
    BYTE sprite_pixels[16] = {0};
    D3DRMPALETTEENTRY sprite_palette[1] = {{0, 255, 0, 0}};
    D3DRMIMAGE sprite_image = {.width = 4,
                               .height = 4,
                               .aspectx = 1,
                               .aspecty = 1,
                               .depth = 8,
                               .bytes_per_line = 4,
                               .buffer1 = sprite_pixels,
                               .palette_size = 1,
                               .palette = sprite_palette};
    IDirect3DRMTexture *sprite_texture;
    IDirect3DRMFrame *sprite_frame;
    assert(SUCCEEDED(rm->lpVtbl->CreateTexture(rm, &sprite_image, &sprite_texture)));
    assert(SUCCEEDED(sprite_texture->lpVtbl->SetDecalSize(sprite_texture, 2, 2)));
    assert(SUCCEEDED(sprite_texture->lpVtbl->SetDecalOrigin(sprite_texture, 2, 2)));
    assert(SUCCEEDED(sprite_texture->lpVtbl->SetDecalScale(sprite_texture, 1)));
    assert(SUCCEEDED(rm->lpVtbl->CreateFrame(rm, NULL, &sprite_frame)));
    assert(SUCCEEDED(
        sprite_frame->lpVtbl->AddVisual(sprite_frame, (IDirect3DRMVisual *)sprite_texture)));
    assert(SUCCEEDED(model->lpVtbl->AddVisual(model, (IDirect3DRMVisual *)sprite_frame)));
    const float sprite_scales[][3] = {{1, 1, 1}, {2, 2, 2}, {3, .5f, 1}, {.5f, .5f, .5f}};
    for (unsigned i = 0; i < 4; ++i)
    {
        assert(SUCCEEDED(model->lpVtbl->AddScale(model, D3DRMCOMBINE_REPLACE, sprite_scales[i][0],
                                                 sprite_scales[i][1], sprite_scales[i][2])));
        assert(SUCCEEDED(model->lpVtbl->SetPosition(model, root, 0, 0, 4)));
        SceneData sprite_scene = {0};
        assert(sqz_scene_extract(&sprite_scene, viewport, root));
        assert(SUCCEEDED(
            target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &black)));
        assert(SUCCEEDED(viewport->lpVtbl->Clear(viewport)));
        assert(SUCCEEDED(viewport->lpVtbl->Render(viewport, root)));
        unsigned native_pixels = ink_count(target);
        assert(SUCCEEDED(
            target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &black)));
        assert(sqz_gpu_draw(&sprite_scene, target, message, sizeof(message)));
        printf("sprite scale=%g,%g,%g native pixels=%u GPU pixels=%u\n", sprite_scales[i][0],
               sprite_scales[i][1], sprite_scales[i][2], native_pixels, ink_count(target));
        assert(native_pixels == 256 && ink_count(target) == native_pixels);
        sqz_scene_free(&sprite_scene);
    }
    sprite_frame->lpVtbl->Release(sprite_frame);
    sprite_texture->lpVtbl->Release(sprite_texture);
    sqz_gpu_shutdown();
    viewport->lpVtbl->Release(viewport);
    mesh->lpVtbl->Release(mesh);
    ambient->lpVtbl->Release(ambient);
    model->lpVtbl->Release(model);
    camera->lpVtbl->Release(camera);
    root->lpVtbl->Release(root);
    device->lpVtbl->Release(device);
    rm->lpVtbl->Release(rm);
    target->lpVtbl->Release(target);
    dd->lpVtbl->Release(dd);
    return 0;
}
