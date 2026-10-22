/* Numeric image comparison only: user controls all actual game comparisons. */
static void cached_pixels(IDirectDrawSurface *target, DWORD out[64 * 64])
{
    DDSURFACEDESC desc = {.dwSize = sizeof(desc)};
    assert(SUCCEEDED(target->lpVtbl->Lock(target, NULL, &desc, DDLOCK_WAIT, NULL)));
    assert(desc.ddpfPixelFormat.dwRGBBitCount == 32);
    for (unsigned y = 0; y < 64; ++y)
        memcpy(out + y * 64, (BYTE *)desc.lpSurface + y * desc.lPitch, 64 * 4);
    assert(SUCCEEDED(target->lpVtbl->Unlock(target, desc.lpSurface)));
}
static void cached_matches(IDirect3DRMViewport *viewport, IDirect3DRMFrame *root,
                           IDirectDrawSurface *target, int builds, int uploads)
{
    DWORD reference[64 * 64], cached[64 * 64];
    char message[512];
    DDBLTFX fill = {.dwSize = sizeof(fill), .dwFillColor = 0xff};
    SceneData scene = {0};
    assert(sqz_scene_extract(&scene, viewport, root));
    assert(SUCCEEDED(target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL, &fill)));
    assert(sqz_gpu_draw(&scene, target, message, sizeof(message)));
    cached_pixels(target, reference);
    sqz_scene_free(&scene);
    assert(sqz_scene_extract_cached(&scene, viewport, root));
    assert(scene.cached_vertices > 0);
    if (builds >= 0)
        assert(scene.geometry_builds == (unsigned)builds);
    assert(SUCCEEDED(target->lpVtbl->Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL, &fill)));
    BOOL ok = sqz_gpu_draw(&scene, target, message, sizeof(message));
    if (!ok)
        fprintf(stderr, "cached GPU: %s\n", message);
    assert(ok);
    if (uploads >= 0)
        assert(scene.geometry_uploads == (unsigned)uploads);
    cached_pixels(target, cached);
    unsigned differences = 0;
    for (unsigned i = 0; i < 64 * 64; ++i)
    {
        unsigned maximum = 0;
        for (unsigned shift = 0; shift < 24; shift += 8)
        {
            int delta = (int)((reference[i] >> shift) & 255) - (int)((cached[i] >> shift) & 255);
            unsigned difference = (unsigned)abs(delta);
            if (difference > maximum)
                maximum = difference;
        }
        if (maximum > 3)
            ++differences;
    }
    if (differences > 8)
        fprintf(stderr, "cached/reference differing pixels: %u\n", differences);
    assert(differences <= 8); /* Allow subpixel edge ties, not changed surfaces. */
    sqz_scene_free(&scene);
}
