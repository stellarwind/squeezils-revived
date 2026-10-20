// 0x425070: load a PPM or GIF texture and replace the named entry in the current table.
#include "Renderer2/Renderer2.h"
#include "String/String.h"
#include "Vector3f/Vector3f.h"
#include "Container/ContainerTable.h"
#include <string.h>
typedef Table<String, IDirect3DRMTexture *, Array<IDirect3DRMTexture *>, 0> ResourceTextureTable;
ResourceTextureTable *TopResourceTextureTable(); // 0x425bf0
extern Renderer *g_textureRenderer; // 0x4c75dc
extern int g_texturePixelFormat[6]; // 0x4c7168
extern int g_textureByteCounter; // 0x4bb5f4
IDirect3DRMTexture *LoadResourceGif(const char *path, const float *transparent,
    const int *pixelFormat, int *byteCounter); // 0x415440
void TextureResourceLog(const char *, ...); // 0x419290
IDirect3DRMTexture *LoadResourceTexture(const char *name, const char *path)
{
    TextureResourceLog("Loading texture %s (%s): ", name, path);
    IDirect3DRMTexture *texture = 0;
    if (strstr(path, ".ppm"))
        g_textureRenderer->d3drm->LoadTexture((char *)path, &texture);
    if (!texture) {
        Vector3f transparent(1, 0, 1);
        texture = LoadResourceGif(path, &transparent.x, g_texturePixelFormat, &g_textureByteCounter);
    }
    if (!texture) {
        TextureResourceLog("failed!\n");
        return 0;
    }
    TextureResourceLog("OK.\n");
    if (TopResourceTextureTable()->Contains(String(name)))
        TopResourceTextureTable()->Get(String(name))->Release();
    TopResourceTextureTable()->Insert(String(name), texture, -1);
    return texture;
}
