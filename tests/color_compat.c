/* Execute original x86 branch bytes plus production guard on a mapped PE copy.
 * Never run the game's entry point or write its executable on disk. */
#include "runtime.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int sqz_warp_patch_image(BYTE *base,size_t size) { (void)base; (void)size; return 0; }
static int depth;
static DWORD pixel;
static HRESULT WINAPI surface_lock(IDirectDrawSurface *self, RECT *rect, DDSURFACEDESC *desc, DWORD flags, HANDLE event)
{
    (void)self; (void)rect; (void)flags; (void)event;
    desc->lpSurface = &pixel;
    desc->ddpfPixelFormat.dwRGBBitCount = depth;
    return DD_OK;
}
static HRESULT WINAPI surface_unlock(IDirectDrawSurface *self, void *address)
{ (void)self; assert(address == NULL); return DD_OK; }
static void __cdecl query(int *width, int *height, int *bpp)
{ *width=640; *height=480; *bpp=depth; }

__attribute__((naked)) static int __cdecl branch(void *code __attribute__((unused)),
                                               void *image __attribute__((unused)),
                                               void *key __attribute__((unused)))
{
    __asm__("push %esi\n\tpush %edi\n\tmov 12(%esp),%eax\n\t"
            "mov 16(%esp),%esi\n\tmov 20(%esp),%edi\n\tcall *%eax\n\t"
            "pop %edi\n\tpop %esi\n\tret\n");
}

static BYTE *mapped(const BYTE *file)
{
    const IMAGE_DOS_HEADER *dos = (const void *)file;
    const IMAGE_NT_HEADERS32 *nt = (const void *)(file+dos->e_lfanew);
    BYTE *image = VirtualAlloc(NULL,nt->OptionalHeader.SizeOfImage,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    assert(image);
    memcpy(image,file,nt->OptionalHeader.SizeOfHeaders);
    const IMAGE_SECTION_HEADER *sections = IMAGE_FIRST_SECTION(nt);
    for (unsigned i=0; i<nt->FileHeader.NumberOfSections; ++i)
        memcpy(image+sections[i].VirtualAddress,file+sections[i].PointerToRawData,sections[i].SizeOfRawData);
    return image;
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    FILE *input = fopen(argv[1],"rb"); assert(input);
    fseek(input,0,SEEK_END); long length = ftell(input); rewind(input);
    BYTE *file = malloc(length); assert(file);
    assert(fread(file,1,length,input) == (size_t)length); fclose(input);
    for (int enabled=0; enabled<2; ++enabled) {
        BYTE *image = mapped(file);
        int expected = 0;
#ifdef SQZ_FIX_16BPP_RGB_INDEX_REMAP
        if (enabled) expected |= 1;
#endif
#ifdef SQZ_FIX_16BPP_DESKTOP_GATE
        if (enabled) expected |= 2;
#endif
#ifdef SQZ_FIX_16BPP_COLORKEY_MASK
        if (enabled) expected |= 4;
#endif
        assert(sqz_color_patch_image(image,0x101000,enabled,enabled) == expected);
        IDirectDrawSurfaceVtbl table = {.Lock=surface_lock,.Unlock=surface_unlock};
        IDirectDrawSurface surface = {&table};
        DWORD (WINAPI *key_color)(IDirectDrawSurface *,COLORREF) = (void *)(image+0x74230);
        pixel = 0xa1ff00ff;
        for (depth=8; depth<=32; depth+=8) {
            DWORD mask = depth == 32 ? ((expected & 4) ? 0xffffffff : 0) : (1u << depth)-1;
            assert(key_color(&surface,CLR_INVALID) == (pixel & mask));
        }
        memcpy(image+0x15571,"\xb8\x01\0\0\0\xc3",6);
        memcpy(image+0x15655,"\x31\xc0\xc3",3);
        D3DRMIMAGE texture = {.depth=8,.rgb=0};
        float key[3] = {1,0,1};
        assert(branch(image+0x15569,&texture,key) == 1);
        assert(branch(image+0x15569,&texture,NULL) == 0);
        for (int bits=16; bits<=32; bits+=8) {
            texture.depth=bits; texture.rgb=1;
            assert(branch(image+0x15569,&texture,key) == ((expected & 1) ? 0 : 1));
        }
        uint32_t relative = (uint32_t)(uintptr_t)query-(uint32_t)(uintptr_t)(image+0x743c7);
        memcpy(image+0x743c3,&relative,4);
        memcpy(image+0x743e1,"\x31\xc0\x83\xc4\x04\xc3",6);
        int (__cdecl *gate)(int *,int *) = (void *)(image+0x743b0);
        for (depth=0; depth<=40; ++depth) {
            int width=0,height=0;
            assert(gate(&width,&height) == (depth >= 8 && depth <= ((expected & 2) ? 32 : 16)));
            assert(width == 640 && height == 480);
        }
        VirtualFree(image,0,MEM_RELEASE);
    }
    BYTE *image = mapped(file);
    image[0x15440] ^= 1;
#ifdef SQZ_FIX_16BPP_RGB_INDEX_REMAP
    assert(sqz_color_patch_image(image,0x101000,TRUE,TRUE) == -1);
    assert(image[0x743d5] == 16);
#endif
    VirtualFree(image,0,MEM_RELEASE);
    free(file);
    puts("PASS: original x86 remap/gate, fixed RGB skip, indexed preservation, depth bounds, signature refusal");
    return 0;
}
