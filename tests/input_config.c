#include "input_config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static BYTE *mapped(const BYTE *file)
{
    const IMAGE_DOS_HEADER *dos = (const void *)file;
    const IMAGE_NT_HEADERS32 *nt = (const void *)(file + dos->e_lfanew);
    BYTE *image = VirtualAlloc(NULL, nt->OptionalHeader.SizeOfImage,
                              MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    assert(image);
    memcpy(image, file, nt->OptionalHeader.SizeOfHeaders);
    const IMAGE_SECTION_HEADER *s = IMAGE_FIRST_SECTION(nt);
    for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; ++i)
        memcpy(image + s[i].VirtualAddress, file + s[i].PointerToRawData, s[i].SizeOfRawData);
    return image;
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    char path[MAX_PATH], other[MAX_PATH], old_directory[MAX_PATH];
    assert(GetModuleFileNameA(NULL, path, sizeof(path)));
    strcpy(strrchr(path, '\\'), "\\input.cfg");
    assert(GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES);
    BYTE object[0xe00] = {0}, backup[sizeof(object)];
    uint32_t (*rows)[17] = (void *)(object + 0xc);
    strcpy((char *)object + 0xd9c, "Keyboard");
    for (unsigned i = 0; i < 51; ++i) {
        rows[i][3] = 65 + i;
        rows[i][4] = i % 5;
        rows[i][5] = i % 5;
        sprintf((char *)&rows[i][6], "Action %u", i);
    }
    memcpy(backup, object, sizeof(object));
    assert(!sqz_input_load(object) && !memcmp(object, backup, sizeof(object)));

    FILE *f = fopen(argv[1], "rb"); assert(f);
    fseek(f, 0, SEEK_END); long length = ftell(f); rewind(f);
    BYTE *raw = malloc(length); assert(raw);
    assert(fread(raw, 1, length, f) == (size_t)length); fclose(f);
    BYTE *image = mapped(raw);
    image[0x3ecf0] ^= 1;
    assert(sqz_input_patch_image(image, 0x101000) == -1 && image[0x3ec70] == 0x83);
    image[0x3ecf0] ^= 1;
    assert(sqz_input_patch_image(image, 0x101000) == 1);
    int (__fastcall *load)(void *) = (void *)(image + 0x3ec70);
    int (__fastcall *save)(void *) = (void *)(image + 0x3ecf0);
    assert(save(object));
    memset(object + 0xc, 0, 0xd8c);
    assert(load(object) && !memcmp(object, backup, sizeof(object)));
    rows[0][3] = 90; rows[0][4] = 4;
    assert(save(object));
    strcpy((char *)object + 0xd9c, "Joystick");
    rows[0][3] = 88;
    assert(save(object));
    strcpy((char *)object + 0xd9c, "Keyboard");
    assert(load(object) && rows[0][3] == 90 && rows[0][4] == 4);
    /* Relative launch directories cannot change the config location. */
    assert(GetCurrentDirectoryA(sizeof(old_directory), old_directory));
    assert(GetTempPathA(sizeof(other), other) && SetCurrentDirectoryA(other));
    assert(load(object) && rows[0][3] == 90);
    assert(SetCurrentDirectoryA(old_directory));
    /* Failed save keeps the previous on-disk controls intact. */
    assert(SetFileAttributesA(path, FILE_ATTRIBUTE_READONLY));
    rows[0][3] = 89;
    assert(!save(object));
    assert(SetFileAttributesA(path, FILE_ATTRIBUTE_NORMAL));
    assert(load(object) && rows[0][3] == 90);
    memcpy(backup, object, sizeof(object));
    assert(WritePrivateProfileStringA("Keyboard", "Command50", "broken", path));
    assert(!load(object) && !memcmp(object, backup, sizeof(object)));
    assert(save(object));
    rows[1][5] = 999;
    assert(!save(object));
    assert(load(object) && rows[1][5] == 1);
    assert(DeleteFileA(path));
    VirtualFree(image, 0, MEM_RELEASE); free(raw);
    puts("PASS: original x86 entry redirects; local input round trip, profiles, remapping, CWD independence, failed save preservation, corrupt file refusal");
    return 0;
}
