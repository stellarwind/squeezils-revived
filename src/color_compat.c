/* Verified original-EXE color compatibility changes. No disk patching. */
#include "runtime.h"
#include <stdint.h>
#include <string.h>

static void *remap_continue __attribute__((used));
static void *remap_skip __attribute__((used));
static int color_status;
static int warp_status;

/* At 0x415569: ESI is D3DRMIMAGE*, EDI is the optional transparent RGB.
 * Keep all registers/stack intact. Only indexed 8bpp needs index swapping. */
__attribute__((naked)) static void indexed_remap_guard(void)
{
    __asm__("test %edi,%edi\n\t"
            "jz 1f\n\t"
            "cmpl $0,0x14(%esi)\n\t"
            "jne 1f\n\t"
            "cmpl $8,0x10(%esi)\n\t"
            "jne 1f\n\t"
            "jmp *_remap_continue\n"
            "1: jmp *_remap_skip\n");
}

static uint32_t fingerprint(const BYTE *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < size; ++i)
        hash = (hash ^ bytes[i]) * 16777619u;
    return hash;
}

static BOOL write_code(BYTE *where, const BYTE *bytes, size_t size)
{
    DWORD protection, unused;
    if (!VirtualProtect(where, size, PAGE_EXECUTE_READWRITE, &protection))
        return FALSE;
    memcpy(where, bytes, size);
    FlushInstructionCache(GetCurrentProcess(), where, size);
    VirtualProtect(where, size, protection, &unused);
    return TRUE;
}

/* Also used by a fixture that maps original PE sections without executing them.
 * Return bit 1=RGB guard, bit 2=desktop gate, bit 4=HUD key; negatives refuse unknown code. */
int sqz_color_patch_image(BYTE *base, size_t size, BOOL rgb_fix, BOOL desktop_fix)
{
    int status = 0;
    (void)base;
    (void)size;
    (void)rgb_fix;
    (void)desktop_fix;
    (void)indexed_remap_guard;
    (void)fingerprint;
    (void)write_code;
#ifdef SQZ_FIX_16BPP_RGB_INDEX_REMAP
    if (rgb_fix && (size < 0x743fb || fingerprint(base + 0x15440, 0x280) != 0x70dd443f))
        return -1;
#endif
#ifdef SQZ_FIX_16BPP_DESKTOP_GATE
    if (desktop_fix && (size < 0x743fb || fingerprint(base + 0x743b0, 0x4b) != 0xa08a4c8d))
        return -2;
#endif
#ifdef SQZ_FIX_16BPP_COLORKEY_MASK
    if (desktop_fix && (size < 0x74310 || fingerprint(base + 0x74230, 0xe0) != 0x58f997d0))
        return -5;
#endif
#ifdef SQZ_FIX_16BPP_RGB_INDEX_REMAP
    if (rgb_fix)
    {
        BYTE jump[8] = {0xe9, 0, 0, 0, 0, 0x90, 0x90, 0x90};
        uint32_t distance =
            (uint32_t)(uintptr_t)indexed_remap_guard - (uint32_t)(uintptr_t)(base + 0x15569 + 5);
        memcpy(jump + 1, &distance, 4);
        remap_continue = base + 0x15571;
        remap_skip = base + 0x15655;
        if (!write_code(base + 0x15569, jump, sizeof(jump)))
            return -3;
        status |= 1;
    }
#endif
#ifdef SQZ_FIX_16BPP_DESKTOP_GATE
    if (desktop_fix)
    {
        /* Widen original 8..16 acceptance to 8..32; query and RGB masks remain real. */
        const BYTE maximum = 32;
        if (!write_code(base + 0x743d5, &maximum, 1))
            return -4;
        status |= 2;
    }
#endif
#ifdef SQZ_FIX_16BPP_COLORKEY_MASK
    if (desktop_fix)
    {
        /* mask = UINT32_MAX >> ((-bpp) & 31), preserving 8/16/24 and fixing 32.
         * Same original 20-byte block: EDI mask, EAX pixels, ECX first pixel,
         * and pushed NULL Unlock argument. No stack/register contract changes. */
        const BYTE mask[] = {0x83, 0xcf, 0xff, 0x8a, 0x4c, 0x24, 0x6c, 0xf6, 0xd9, 0xd3,
                             0xef, 0x8b, 0x44, 0x24, 0x3c, 0x6a, 0,    0x8b, 8,    0x90};
        if (!write_code(base + 0x742af, mask, sizeof(mask)))
            return -6;
        status |= 4;
    }
#endif
    return status;
}

void sqz_color_install(void)
{
    BYTE *base = (BYTE *)GetModuleHandleA(NULL);
    IMAGE_DOS_HEADER *dos = (void *)base;
    IMAGE_NT_HEADERS32 *nt;
    char rgb[8] = "", desktop[8] = "";
    if (!base || dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew < 0 || dos->e_lfanew > 4096)
        return;
    nt = (void *)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE || nt->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
        return;
    GetEnvironmentVariableA("SQZ_FIX_RGB_REMAP", rgb, sizeof(rgb));
    GetEnvironmentVariableA("SQZ_ALLOW_32BPP", desktop, sizeof(desktop));
    color_status = sqz_color_patch_image(base, nt->OptionalHeader.SizeOfImage,
                                         strcmp(rgb, "0") != 0, strcmp(desktop, "0") != 0);
    warp_status = sqz_warp_patch_image(base, nt->OptionalHeader.SizeOfImage);
}

void sqz_color_log(void)
{
    sqz_log("DIAG WARP_PATCH status=%d", warp_status);
    sqz_log("DIAG COLOR_PATCH status=%d rgb_remap_guard=%d desktop_32bpp=%d hud_key=%d "
            "(negative=unsupported-image-or-write-failure)",
            color_status, color_status >= 0 && (color_status & 1) != 0,
            color_status >= 0 && (color_status & 2) != 0,
            color_status >= 0 && (color_status & 4) != 0);
}
