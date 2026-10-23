#include "runtime.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HRESULT sqz_create_surface(const IDirectDrawVtbl *v,IDirectDraw *dd,DDSURFACEDESC *d,IDirectDrawSurface **out,IUnknown *outer)
{ return v->CreateSurface(dd,d,out,outer); }
HRESULT sqz_surface_blt(const IDirectDrawSurfaceVtbl *v,IDirectDrawSurface *dst,RECT *d,IDirectDrawSurface *src,RECT *s,DWORD flags,DDBLTFX *fx)
{ (void)v;(void)dst;(void)d;(void)src;(void)s;(void)flags;(void)fx; return E_NOTIMPL; }

int main(int argc,char **argv)
{
    assert(argc == 2);
    FILE *f=fopen(argv[1],"rb"); assert(f);
    fseek(f,0,SEEK_END); long length=ftell(f); rewind(f);
    BYTE *file=malloc(length); assert(file);
    assert(fread(file,1,length,f) == (size_t)length); fclose(f);
    IMAGE_DOS_HEADER *dos=(void *)file;
    IMAGE_NT_HEADERS32 *nt=(void *)(file+dos->e_lfanew);
    BYTE *image=VirtualAlloc(NULL,nt->OptionalHeader.SizeOfImage,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    assert(image);
    memcpy(image,file,nt->OptionalHeader.SizeOfHeaders);
    IMAGE_SECTION_HEADER *sections=IMAGE_FIRST_SECTION(nt);
    for (unsigned i=0;i<nt->FileHeader.NumberOfSections;++i)
        memcpy(image+sections[i].VirtualAddress,file+sections[i].PointerToRawData,sections[i].SizeOfRawData);
    BYTE first=image[0x769a0]; image[0x769a0]^=1;
    assert(sqz_warp_patch_image(image,nt->OptionalHeader.SizeOfImage) == -1);
    image[0x769a0]=first;
    assert(sqz_warp_patch_image(image,nt->OptionalHeader.SizeOfImage) == 1);
    /* Original-load stand-in returns the actual depth supplied by production wrapper. */
    memcpy(image+0x79090,"\x8b\x44\x24\x0c\x8b\x00\xc2\x0c\x00",9);
    int32_t relative; memcpy(&relative,image+0x76a4d,4);
    typedef int (__attribute__((thiscall)) *Load)(void *,const char *,int,int *);
    Load load=(Load)(image+0x76a51+relative);
    int format[6]={32,0,0xff0000,0xff00,0xff,0};
    assert(load(NULL,"test",0,format) == 16 && format[0] == 32);
    format[0]=8;format[1]=1;
    assert(load(NULL,"test",0,format) == 8);

    IDirectDraw *dd;
    IDirectDrawSurface *source,*destination;
    assert(SUCCEEDED(DirectDrawCreate(NULL,&dd,NULL)));
    assert(SUCCEEDED(IDirectDraw_SetCooperativeLevel(dd,NULL,DDSCL_NORMAL)));
    DDSURFACEDESC request={.dwSize=sizeof(request),.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT,
                          .dwWidth=4,.dwHeight=2,.ddsCaps={DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY}};
    assert(SUCCEEDED(sqz_warp_create_surface(dd->lpVtbl,dd,&request,&source,NULL,image+0x768ae)));
    DDSURFACEDESC lock={.dwSize=sizeof(lock)};
    assert(SUCCEEDED(IDirectDrawSurface_Lock(source,NULL,&lock,DDLOCK_WAIT,NULL)));
    assert(lock.ddpfPixelFormat.dwRGBBitCount == 16);
    for (unsigned y=0;y<2;++y) {
        WORD *row=(void *)((BYTE *)lock.lpSurface+y*lock.lPitch);
        row[0]=row[1]=0xf800; row[2]=row[3]=0x1f;
    }
    assert(SUCCEEDED(IDirectDrawSurface_Unlock(source,lock.lpSurface)));
    request.dwWidth=8;request.dwHeight=4;
    assert(SUCCEEDED(IDirectDraw_CreateSurface(dd,&request,&destination,NULL)));
    assert(SUCCEEDED(sqz_warp_blt(destination->lpVtbl,destination,NULL,source,NULL,DDBLT_WAIT,NULL)));
    lock.dwSize=sizeof(lock);
    assert(SUCCEEDED(IDirectDrawSurface_Lock(destination,NULL,&lock,DDLOCK_WAIT,NULL)));
    assert(lock.ddpfPixelFormat.dwRGBBitCount == 32);
    for (unsigned y=0;y<4;++y) {
        DWORD *row=(void *)((BYTE *)lock.lpSurface+y*lock.lPitch);
        for (unsigned x=0;x<8;++x) assert((row[x]&0xffffff) == (x<4 ? 0xff0000 : 0xff));
    }
    assert(SUCCEEDED(IDirectDrawSurface_Unlock(destination,lock.lpSurface)));
    assert(sqz_warp_blt(destination->lpVtbl,destination,NULL,source,NULL,DDBLT_KEYSRC,NULL) == E_NOTIMPL);
    ULONG refs=IDirectDrawSurface_Release(source);
    sqz_warp_surface_released(source,refs);
    IDirectDrawSurface_Release(destination);IDirectDraw_Release(dd);
    VirtualFree(image,0,MEM_RELEASE);free(file);
    puts("PASS: warp signature guard, original loader ABI, RGB565 selection, real 16-to-32 scaled blit and release");
    return 0;
}
