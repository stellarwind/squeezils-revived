#include "scene_gpu.h"
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cached_geometry.h"
void sqz_log(const char *format, ...)
{
    (void)format;
}
static IDirect3DRMFrame *camera;
static HRESULT WINAPI get_camera(IDirect3DRMViewport *v, IDirect3DRMFrame **out)
{
    (void)v;
    *out = camera;
    camera->lpVtbl->AddRef(camera);
    return S_OK;
}
static HRESULT WINAPI plane(IDirect3DRMViewport *v, float *l, float *r, float *b, float *t)
{
    (void)v;
    *l = *b = -1;
    *r = *t = 1;
    return S_OK;
}
static float WINAPI front(IDirect3DRMViewport *v)
{
    (void)v;
    return 1;
}
static float WINAPI back(IDirect3DRMViewport *v)
{
    (void)v;
    return 20;
}
static LONG WINAPI origin(IDirect3DRMViewport *v)
{
    (void)v;
    return 0;
}
static DWORD WINAPI dimension(IDirect3DRMViewport *v)
{
    (void)v;
    return 64;
}
static DWORD pixel(IDirectDrawSurface *surface, unsigned x, unsigned y)
{
    DDSURFACEDESC d = {.dwSize = sizeof(d)};
    DWORD value;
    assert(SUCCEEDED(surface->lpVtbl->Lock(surface, NULL, &d, DDLOCK_WAIT, NULL)));
    assert(d.ddpfPixelFormat.dwRGBBitCount == 32);
    memcpy(&value, (BYTE *)d.lpSurface + y * d.lPitch + x * 4, 4);
    assert(SUCCEEDED(surface->lpVtbl->Unlock(surface, d.lpSurface)));
    return value & 0xffffff;
}
int main(void)
{
    IDirect3DRM *rm;
    IDirect3DRMFrame *root, *model, *leaves;
    IDirect3DRMMesh *mesh;
    assert(SUCCEEDED(Direct3DRMCreate(&rm)));
    assert(SUCCEEDED(rm->lpVtbl->CreateFrame(rm, NULL, &root)));
    assert(SUCCEEDED(rm->lpVtbl->CreateFrame(rm, root, &camera)));
    assert(SUCCEEDED(rm->lpVtbl->CreateFrame(rm, root, &model)));
    assert(SUCCEEDED(model->lpVtbl->SetPosition(model, root, 0, 0, 4)));
    assert(SUCCEEDED(rm->lpVtbl->CreateMesh(rm, &mesh)));
    unsigned face[3] = {0, 2, 1};
    D3DRMGROUPINDEX group;
    assert(SUCCEEDED(mesh->lpVtbl->AddGroup(mesh, 3, 1, 3, face, &group)));
    D3DRMVERTEX vertices[3] = {0};
    vertices[0].position = (D3DVECTOR){.x = -2, .y = -2, .z = 0};
    vertices[1].position = (D3DVECTOR){.x = 2, .y = -2, .z = 0};
    vertices[2].position = (D3DVECTOR){.x = 0, .y = 2, .z = 0};
    for (unsigned i = 0; i < 3; ++i)
        vertices[i].color = 0xffffffff;
    assert(SUCCEEDED(mesh->lpVtbl->SetVertices(mesh, group, 0, 3, vertices)));
    assert(SUCCEEDED(mesh->lpVtbl->SetGroupColor(mesh, group, 0xffff0000)));
    assert(SUCCEEDED(mesh->lpVtbl->SetGroupQuality(mesh, group, D3DRMRENDER_UNLITFLAT)));
    assert(SUCCEEDED(model->lpVtbl->AddVisual(model, (IDirect3DRMVisual *)mesh)));
    /* The real game also attaches native frames as visuals. Already-parented
     * frames can be encountered through both GetVisuals and GetChildren. */
    assert(SUCCEEDED(root->lpVtbl->AddVisual(root, (IDirect3DRMVisual *)model)));
    BYTE indices[16] = {1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0};
    D3DRMPALETTEENTRY palette[256] = {{0, 255, 0, 0}, {255, 0, 255, 0}};
    D3DRMIMAGE image = {.width = 4,
                        .height = 4,
                        .aspectx = 1,
                        .aspecty = 1,
                        .depth = 8,
                        .rgb = 0,
                        .bytes_per_line = 4,
                        .buffer1 = indices,
                        .palette_size = 256,
                        .palette = palette};
    IDirect3DRMTexture *texture;
    assert(SUCCEEDED(rm->lpVtbl->CreateTexture(rm, &image, &texture)));
    assert(SUCCEEDED(texture->lpVtbl->SetDecalTransparency(texture, TRUE)));
    assert(SUCCEEDED(texture->lpVtbl->SetDecalTransparentColor(texture, 0xffff00ff)));
    assert(SUCCEEDED(texture->lpVtbl->SetDecalOrigin(texture, 2, 2)));
    assert(SUCCEEDED(texture->lpVtbl->SetDecalSize(texture, 2, 2)));
    assert(SUCCEEDED(texture->lpVtbl->SetDecalScale(texture, 1)));
    assert(SUCCEEDED(rm->lpVtbl->CreateFrame(rm, root, &leaves)));
    assert(SUCCEEDED(leaves->lpVtbl->SetPosition(leaves, root, 1, 0, 2)));
    /* A sprite resource is a parentless frame attached as a visual, just like
     * EndLevelSprite. Extraction must work before native Render initializes it. */
    IDirect3DRMFrame *sprite;
    assert(SUCCEEDED(rm->lpVtbl->CreateFrame(rm, NULL, &sprite)));
    assert(SUCCEEDED(sprite->lpVtbl->AddVisual(sprite, (IDirect3DRMVisual *)texture)));
    assert(SUCCEEDED(leaves->lpVtbl->AddVisual(leaves, (IDirect3DRMVisual *)sprite)));
    IDirect3DRMViewportVtbl table = {.GetCamera = get_camera,
                                     .GetPlane = plane,
                                     .GetFront = front,
                                     .GetBack = back,
                                     .GetX = origin,
                                     .GetY = origin,
                                     .GetWidth = dimension,
                                     .GetHeight = dimension};
    IDirect3DRMViewport viewport = {&table};
    SceneData scene = {0};
    BOOL extracted = sqz_scene_extract(&scene, &viewport, root);
    if (!extracted)
        fprintf(stderr, "extract: %s\n", scene.error);
    assert(extracted && scene.meshes == 1 && scene.decals == 1 && scene.count == 9);
    assert(fabsf(scene.vertices[0].clip[0] + 2) < 0.001f &&
           fabsf(scene.vertices[0].clip[3] - 4) < 0.001f);
    IDirectDraw *dd;
    IDirectDrawSurface *target;
    assert(SUCCEEDED(DirectDrawCreate(NULL, &dd, NULL)));
    assert(SUCCEEDED(dd->lpVtbl->SetCooperativeLevel(dd, NULL, DDSCL_NORMAL)));
    DDSURFACEDESC desc = {.dwSize = sizeof(desc),
                          .dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT,
                          .dwWidth = 64,
                          .dwHeight = 64,
                          .ddsCaps = {DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY}};
    assert(SUCCEEDED(dd->lpVtbl->CreateSurface(dd, &desc, &target, NULL)));
    DDBLTFX fill = {.dwSize = sizeof(fill), .dwFillColor = 0xff};
    assert(SUCCEEDED(
        target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &fill)));
    char message[512];
    BOOL rendered = sqz_gpu_draw(&scene, target, message, sizeof(message));
    if (!rendered)
        fprintf(stderr, "GPU: %s\n", message);
    assert(rendered);
    printf("pixels background=%06lx model=%06lx leaf=%06lx\n", pixel(target, 2, 2),
           pixel(target, 36, 32), pixel(target, 56, 32));
    assert(pixel(target, 2, 2) == 0xff && pixel(target, 36, 32) == 0xff0000 &&
           pixel(target, 56, 32) == 0x00ff00);
    assert(pixel(target, 48, 32) == 0x00ff00); /* Linear-filtered key edge has no magenta fringe. */
    assert(scene.texture_checks == 1 && scene.texture_conversions == 1);
    /* A repeated material must reuse one lookup/conversion, and a warm texture
     * must not be converted again. Mutations remain visible on the next draw. */
    assert(scene.batch_capacity > scene.batch_count);
    scene.batches[scene.batch_count] = scene.batches[1];
    ++scene.batch_count;
    assert(sqz_gpu_draw(&scene, target, message, sizeof(message)));
    assert(scene.texture_checks == 1 && scene.texture_conversions == 0 &&
           scene.texture_reuses == 2);
    D3DRMIMAGE *mutable_image = texture->lpVtbl->GetImage(texture);
    D3DRMPALETTEENTRY original_palette = mutable_image->palette[0];
    BYTE original_pixels[16];
    memcpy(original_pixels, mutable_image->buffer1, sizeof(original_pixels));
    mutable_image->palette[0] = (D3DRMPALETTEENTRY){255, 255, 0, 0};
    assert(SUCCEEDED(target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL, &fill)));
    assert(sqz_gpu_draw(&scene, target, message, sizeof(message)));
    assert(scene.texture_conversions == 1 && pixel(target, 56, 32) == 0xffff00);
    memset(mutable_image->buffer1, 1, sizeof(original_pixels));
    assert(SUCCEEDED(target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL, &fill)));
    assert(sqz_gpu_draw(&scene, target, message, sizeof(message)));
    assert(scene.texture_conversions == 1 && pixel(target, 56, 32) == 0xff);
    assert(SUCCEEDED(texture->lpVtbl->SetDecalTransparentColor(texture, 0xff00ffff)));
    assert(SUCCEEDED(target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL, &fill)));
    assert(sqz_gpu_draw(&scene, target, message, sizeof(message)));
    assert(scene.texture_conversions == 1 && pixel(target, 56, 32) == 0xff00ff);
    assert(SUCCEEDED(texture->lpVtbl->SetDecalTransparentColor(texture, 0xffff00ff)));
    assert(SUCCEEDED(texture->lpVtbl->SetDecalTransparency(texture, FALSE)));
    assert(SUCCEEDED(target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL, &fill)));
    assert(sqz_gpu_draw(&scene, target, message, sizeof(message)));
    assert(scene.texture_conversions == 1 && pixel(target, 56, 32) == 0xff00ff);
    mutable_image->palette[0] = original_palette;
    memcpy(mutable_image->buffer1, original_pixels, sizeof(original_pixels));
    assert(SUCCEEDED(texture->lpVtbl->SetDecalTransparency(texture, TRUE)));
    puts("PASS: warm/shared texture reuse and in-place palette, pixel, color-key and transparency "
         "updates");
    sqz_scene_free(&scene);
    cached_matches(&viewport, root, target, 1, 1);
    cached_matches(&viewport, root, target, 0, 0);
    assert(SUCCEEDED(model->lpVtbl->SetPosition(model, root, .25f, .1f, 4)));
    cached_matches(&viewport, root, target, 0, 0);
    assert(SUCCEEDED(model->lpVtbl->SetPosition(model, root, 0, 0, 4)));
    assert(SUCCEEDED(camera->lpVtbl->SetPosition(camera, root, .1f, 0, 0)));
    cached_matches(&viewport, root, target, 0, 0);
    assert(SUCCEEDED(camera->lpVtbl->SetPosition(camera, root, 0, 0, 0)));
    vertices[0].position.x = -1;
    assert(SUCCEEDED(mesh->lpVtbl->SetVertices(mesh, group, 0, 3, vertices)));
    assert(sqz_scene_extract(&scene, &viewport, root));
    assert(fabsf(scene.vertices[0].clip[0] + 1) < 0.001f);
    sqz_scene_free(&scene);
    cached_matches(&viewport, root, target, 1, 1); /* In-place morph vertex edit. */
    /* Lights are attached after the model: collection must finish before shading. */
    IDirect3DRMFrame *sun_frame;
    IDirect3DRMLight *ambient, *sun;
    assert(SUCCEEDED(rm->lpVtbl->CreateFrame(rm, root, &sun_frame)));
    assert(SUCCEEDED(rm->lpVtbl->CreateLightRGB(rm, D3DRMLIGHT_AMBIENT, .2f, .2f, .2f, &ambient)));
    assert(SUCCEEDED(rm->lpVtbl->CreateLightRGB(rm, D3DRMLIGHT_DIRECTIONAL, .6f, .6f, .6f, &sun)));
    assert(SUCCEEDED(root->lpVtbl->AddLight(root, ambient)));
    assert(SUCCEEDED(sun_frame->lpVtbl->AddLight(sun_frame, sun)));
    vertices[0].position.x = -2;
    vertices[0].normal = (D3DVECTOR){.z = -1};
    vertices[1].normal = (D3DVECTOR){.x = 1};
    vertices[2].normal = (D3DVECTOR){.z = 1};
    assert(SUCCEEDED(mesh->lpVtbl->SetVertices(mesh, group, 0, 3, vertices)));
    assert(SUCCEEDED(mesh->lpVtbl->SetGroupQuality(mesh, group, D3DRMRENDER_GOURAUD)));
    assert(sqz_scene_extract(&scene, &viewport, root));
    assert(scene.light_count == 2);
    assert(fabsf(scene.vertices[0].color[0] - .8f) < .01f);
    assert(fabsf(scene.vertices[1].color[0] - .2f) < .01f);
    assert(fabsf(scene.vertices[2].color[0] - .2f) < .01f);
    assert(scene.vertices[3].color[0] == 1); /* Decals stay unlit. */
    assert(SUCCEEDED(
        target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &fill)));
    assert(sqz_gpu_draw(&scene, target, message, sizeof(message)));
    unsigned left_red = pixel(target, 24, 40) >> 16, right_red = pixel(target, 40, 40) >> 16;
    printf("Gouraud pixels left=%u right=%u\n", left_red, right_red);
    assert(left_red > right_red + 20 && left_red < 204 && right_red >= 51);
    sqz_scene_free(&scene);
    cached_matches(&viewport, root, target, 1, 1);
    /* Changing the retained light frame changes illumination on the next frame. */
    assert(SUCCEEDED(sun_frame->lpVtbl->SetOrientation(sun_frame, root, 0, 0, -1, 0, 1, 0)));
    assert(sqz_scene_extract(&scene, &viewport, root));
    assert(fabsf(scene.vertices[0].color[0] - .2f) < .01f);
    assert(fabsf(scene.vertices[1].color[0] - .8f) < .01f);
    sqz_scene_free(&scene);
    cached_matches(&viewport, root, target, 0, 0); /* Light moves; mesh stays uploaded. */
    assert(SUCCEEDED(sun_frame->lpVtbl->SetOrientation(sun_frame, root, 0, 0, 1, 0, 1, 0)));
    vertices[0].normal = (D3DVECTOR){.x = 1, .z = -1};
    assert(SUCCEEDED(mesh->lpVtbl->SetVertices(mesh, group, 0, 3, vertices)));
    assert(SUCCEEDED(model->lpVtbl->AddScale(model, D3DRMCOMBINE_BEFORE, 2, 1, 1)));
    assert(sqz_scene_extract(&scene, &viewport, root));
    assert(fabsf(scene.vertices[0].color[0] - (.2f + .6f * .894427f)) < .01f);
    sqz_scene_free(&scene);
    cached_matches(&viewport, root, target, 1, 1); /* Changed normal, nonuniform scale. */
    IDirect3DRMLight *point;
    assert(SUCCEEDED(rm->lpVtbl->CreateLightRGB(rm, D3DRMLIGHT_POINT, .3f, .4f, .2f, &point)));
    assert(SUCCEEDED(point->lpVtbl->SetConstantAttenuation(point, 1)));
    assert(SUCCEEDED(point->lpVtbl->SetLinearAttenuation(point, .1f)));
    assert(SUCCEEDED(point->lpVtbl->SetRange(point, 10)));
    assert(SUCCEEDED(sun_frame->lpVtbl->AddLight(sun_frame, point)));
    cached_matches(&viewport, root, target, 0, 0);
    assert(SUCCEEDED(point->lpVtbl->SetEnableFrame(point, leaves)));
    cached_matches(&viewport, root, target, 0, 0); /* Scoped light excludes model. */
    assert(SUCCEEDED(sun_frame->lpVtbl->DeleteLight(sun_frame, point)));
    point->lpVtbl->Release(point);
    assert(SUCCEEDED(mesh->lpVtbl->SetGroupQuality(mesh, group, D3DRMRENDER_FLAT)));
    cached_matches(&viewport, root, target, 1, 1);
    assert(SUCCEEDED(mesh->lpVtbl->SetGroupQuality(mesh, group, D3DRMRENDER_GOURAUD)));
    cached_matches(&viewport, root, target, 1, 1);
    puts("PASS: cached/static mesh uploads, moving transforms, morphs, Gouraud/flat, point/scoped "
         "lights");
    /* The same sprite resource can have two distinct placements. */
    IDirect3DRMFrame *second;
    assert(SUCCEEDED(rm->lpVtbl->CreateFrame(rm, root, &second)));
    assert(SUCCEEDED(second->lpVtbl->SetPosition(second, root, -1, 0, 2)));
    assert(SUCCEEDED(second->lpVtbl->AddVisual(second, (IDirect3DRMVisual *)sprite)));
    assert(sqz_scene_extract(&scene, &viewport, root));
    assert(scene.decals == 2 && scene.count == 15);
    assert(SUCCEEDED(
        target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &fill)));
    assert(sqz_gpu_draw(&scene, target, message, sizeof(message)));
    assert(pixel(target, 56, 32) == 0x00ff00 && pixel(target, 24, 32) == 0x00ff00);
    sqz_scene_free(&scene);
    second->lpVtbl->Release(second);
    sprite->lpVtbl->Release(sprite);
    puts("PASS: unrendered sprite resource frames and two shared-resource placements");
    /* Overworld coordinates are hundreds of units from the origin, while the
     * camera can be less than one unit from the character. Compare extraction
     * against transforming the vertex directly, without reconstructing axes by
     * subtracting translated unit points. */
    assert(SUCCEEDED(model->lpVtbl->SetPosition(model, root, 522.2f, 89.1f, 547.3f)));
    assert(SUCCEEDED(camera->lpVtbl->SetOrientation(camera, root, .3f, .2f, 1, 0, 1, 0)));
    float max_position_error = 0;
    for (unsigned step = 0; step < 24; ++step)
    {
        if (step == 12)
        {
            assert(SUCCEEDED(root->lpVtbl->AddRotation(root, D3DRMCOMBINE_BEFORE, 0, 1, 0, .35f)));
            assert(SUCCEEDED(root->lpVtbl->SetPosition(root, NULL, 31, -14, 25)));
        }
        assert(SUCCEEDED(
            camera->lpVtbl->SetPosition(camera, root, 522.2f + step * .0001f, 89.1f, 545.3f)));
        assert(sqz_scene_extract(&scene, &viewport, root));
        D3DVECTOR world, expected;
        assert(SUCCEEDED(model->lpVtbl->Transform(model, &world, &vertices[0].position)));
        assert(SUCCEEDED(camera->lpVtbl->InverseTransform(camera, &expected, &world)));
        float error = fmaxf(fabsf(scene.vertices[0].clip[0] - expected.x),
                            fmaxf(fabsf(scene.vertices[0].clip[1] - expected.y),
                                  fabsf(scene.vertices[0].clip[3] - expected.z)));
        if (error > max_position_error)
            max_position_error = error;
        sqz_scene_free(&scene);
    }
    printf("Overworld camera maximum position error: %.8f\n", max_position_error);
    fflush(stdout);
    assert(max_position_error < .0002f);
    sqz_gpu_shutdown();
    sun->lpVtbl->Release(sun);
    ambient->lpVtbl->Release(ambient);
    sun_frame->lpVtbl->Release(sun_frame);
    target->lpVtbl->Release(target);
    dd->lpVtbl->Release(dd);
    texture->lpVtbl->Release(texture);
    mesh->lpVtbl->Release(mesh);
    leaves->lpVtbl->Release(leaves);
    model->lpVtbl->Release(model);
    camera->lpVtbl->Release(camera);
    root->lpVtbl->Release(root);
    rm->lpVtbl->Release(rm);
    puts("PASS: native RM extraction, mutable vertices, SDL GPU readback, filtered decals, "
         "background, Gouraud interpolation, retained light orientation, scaled normals");
    return 0;
}
