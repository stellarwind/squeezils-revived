// Texture creation: see Texture.h. Reconstructed from SQZ.EXE 0x4153f0-0x4156b9.
#include "Texture.h"


// The game's allocator wrappers (0x448350 / 0x448360 around its malloc and free), the renderer global at 0x4c75dc and
// the GIF decoder (shared with the sky loader), all still the original code:
extern void *GameAlloc(size_t size);
extern void  GameFree(void *p);
extern Renderer *GetRenderer();
extern int Image_Load(const char *path, const int *pixelFormat, void *allocator, int *outWidth, int *outHeight,
                      void **pixels, unsigned char *palette);       // @00477cc0

static int *s_byteCounter;     // @004ba2a4: the counter passed to the most recent CreateTextureFromGif call

// @004153f0  allocator handed to the image loader
static void *ImageAlloc(size_t size) { return GameAlloc(size); }

// @00415400  called by D3DRM when the texture is destroyed; arg is the D3DRMIMAGE
static void ImageDestroyed(LPDIRECT3DRMOBJECT, LPVOID arg)
{
    D3DRMIMAGE *img = (D3DRMIMAGE *)arg;
    if (s_byteCounter)
        *s_byteCounter -= img->height * img->bytes_per_line;
    GameFree(img->buffer1);
    GameFree(img->palette);
    GameFree(img);
}

// @00415440
IDirect3DRMTexture *CreateTextureFromGif(const char *path, const float *transparent, const int *fmt, int *byteCounter)
{
    int width = 0, height = 0;
    void *pixels = NULL;
    unsigned char planes[3][256];                   // palette from the GIF as three planes: R, G, B

    int ok = Image_Load(path, fmt, (void *)ImageAlloc, &width, &height, &pixels, &planes[0][0]);
    s_byteCounter = byteCounter;
    if (!ok)
        return NULL;

    D3DRMIMAGE *img = (D3DRMIMAGE *)GameAlloc(sizeof(D3DRMIMAGE));
    img->aspectx = 1;
    img->aspecty = 1;
    img->depth = fmt[0];
    img->rgb = (fmt[1] == 0);                       // not palettized
    img->buffer2 = NULL;
    img->red_mask   = fmt[2];
    img->green_mask = fmt[3];
    img->blue_mask  = fmt[4];
    img->alpha_mask = fmt[5];
    // the unpadded row size: ImageLoad pads each row to 4 bytes, which only matches when width * bytes per pixel is a multiple of
    // 4 (true for every texture the game ships; the original does the same)
    img->bytes_per_line = ((fmt[0] + ((fmt[0] >> 31) & 7)) >> 3) * width;
    img->width = width;
    img->height = height;
    *byteCounter += img->bytes_per_line * height;

    img->palette_size = 256;
    img->palette = (D3DRMPALETTEENTRY *)GameAlloc(256 * 4);
    unsigned char *pal = (unsigned char *)img->palette;     // the flags byte of each entry is left as allocated
    for (int i = 0; i < img->palette_size; i++) {
        pal[i * 4 + 0] = planes[0][i];
        pal[i * 4 + 1] = planes[1][i];
        pal[i * 4 + 2] = planes[2][i];
    }
    img->buffer1 = pixels;

    if (transparent) {
        // make the transparent colour palette index 0: swap it with whatever is at index 0, in the palette and the pixels
        int r = (int)((double)transparent[0] * 255.0);
        int g = (int)((double)transparent[1] * 255.0);
        int b = (int)((double)transparent[2] * 255.0);
        int found = -1;
        for (int i = 0; i < img->palette_size; i++) {
            if (pal[i * 4] == r && pal[i * 4 + 1] == g && pal[i * 4 + 2] == b) { found = i; break; }
        }
        if (found != -1) {
            pal[found * 4 + 0] = pal[0];
            pal[found * 4 + 2] = pal[2];
            pal[found * 4 + 1] = pal[1];
            pal[0] = (unsigned char)r;
            pal[1] = (unsigned char)g;
            pal[2] = (unsigned char)b;
            // (swaps the two colours as 8 bit indices even on a 16 bit display, as the original does)
            unsigned char *px = (unsigned char *)img->buffer1;
            for (int i = 0; i < img->width * img->height; i++) {
                if (px[i] == 0)
                    px[i] = (unsigned char)found;
                else if (px[i] == found)
                    px[i] = 0;
            }
        }
    }

    IDirect3DRMTexture *tex;
    if (GetRenderer()->d3drm->CreateTexture(img, &tex) < 0) {
        GameFree(img->buffer1);
        GameFree(img->palette);
        GameFree(img);
        return NULL;
    }
    tex->AddDestroyCallback(ImageDestroyed, img);
    return tex;
}
