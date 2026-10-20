// 0x415440: CreateTextureFromGif - load an image, wrap it as a D3DRMIMAGE with a 256-entry palette and make a texture from it.
void CreateTexturePad();
#include <windows.h>
#include <d3drm.h>
#include "Renderer2/Renderer2.h"

extern int *gTextureByteCounter;                              // 0x4ba2a4
extern Renderer *g_renderer;                                  // 0x4c75dc
void *GameAlloc(size_t size);                                 // 0x448350
void GameFree(void *memory);                                  // 0x448360
void *ImageAlloc(size_t size);                                // 0x4153f0
void __cdecl ImageDestroyed(IDirect3DRMObject *object, void *argument);   // 0x415400
int ImageLoad(const char *path, const int *fmt, void *(*alloc)(size_t), int *width, int *height, void **pixels, unsigned char *planes);   // 0x477cc0

IDirect3DRMTexture *CreateTextureFromGif(const char *path, const float *transparent, const int *fmt, int *byteCounter)
{
    unsigned char blue[256];
    unsigned char green[256];
    unsigned char red[256];
    int hf[2];
    int g;
    int r;
    void *pixels;
    int width;
    IDirect3DRMTexture *tex;
    width = 0;
    pixels = 0;

    int ok = ImageLoad(path, fmt, ImageAlloc, &width, &hf[0], &pixels, red);
    gTextureByteCounter = byteCounter;
    if (!ok)
        return 0;

    D3DRMIMAGE *img = (D3DRMIMAGE *)GameAlloc(sizeof(D3DRMIMAGE));
    img->aspectx = 1;
    img->aspecty = 1;
    img->depth = fmt[0];
    img->rgb = (fmt[1] == 0);
    img->buffer2 = 0;
    img->red_mask = fmt[2];
    img->green_mask = fmt[3];
    img->blue_mask = fmt[4];
    img->alpha_mask = fmt[5];
    img->bytes_per_line = ((img->depth + ((img->depth >> 31) & 7)) >> 3) * width;
    img->width = width;
    img->height = hf[0];
    *byteCounter += img->bytes_per_line * hf[0];

    img->palette_size = 256;
    img->palette = (D3DRMPALETTEENTRY *)GameAlloc(256 * 4);
    unsigned char *pal = (unsigned char *)img->palette;
    for (int i = 0; i < img->palette_size; i++) {
        pal[i * 4 + 0] = red[i];
        pal[i * 4 + 1] = green[i];
        pal[i * 4 + 2] = blue[i];
    }
    img->buffer1 = pixels;

    if (transparent) {
        r = (int)((double)transparent[0] * 255.0);
        g = (int)((double)transparent[1] * 255.0);
        int b = (int)((double)transparent[2] * 255.0);
        hf[1] = -1;
        for (int i = 0; i < img->palette_size; i++) {
            if (pal[i * 4] == r && pal[i * 4 + 1] == g && pal[i * 4 + 2] == b) {
                hf[1] = i;
                break;
            }
        }
        if (hf[1] != -1) {
            pal[hf[1] * 4 + 0] = pal[0];
            pal[hf[1] * 4 + 2] = pal[2];
            pal[hf[1] * 4 + 1] = pal[1];
            pal[0] = (unsigned char)r;
            pal[1] = (unsigned char)g;
            pal[2] = (unsigned char)b;
            unsigned char *px = (unsigned char *)img->buffer1;
            for (int i = 0; i < img->width * img->height; i++) {
                if (px[i] == 0)
                    px[i] = (unsigned char)hf[1];
                else if (px[i] == hf[1])
                    px[i] = 0;
            }
        }
    }

    if (g_renderer->d3drm->CreateTexture(img, &tex) < 0) {
        GameFree(img->buffer1);
        GameFree(img->palette);
        GameFree(img);
        return 0;
    }
    tex->AddDestroyCallback(ImageDestroyed, img);
    return tex;
}
