// ImageDestroyed, SQZ.EXE 0x415400 (cdecl callback).
// Spare declaration preserves VC++ 4.1 operand scheduling.
void ImageDestroyedPad();
#include <windows.h>
#include <d3drm.h>

extern int *gTextureByteCounter; // 0x4ba2a4
void GameFree(void *memory);     // 0x448360

void __cdecl ImageDestroyed(IDirect3DRMObject *object, void *argument)
{
    D3DRMIMAGE *image = (D3DRMIMAGE *)argument;
    if (gTextureByteCounter)
        *gTextureByteCounter -= image->bytes_per_line * image->height;
    GameFree(image->buffer1);
    GameFree(image->palette);
    GameFree(image);
}
