/* Replace only Keylist's registry persistence. Original defaults and UI stay owners. */
#include "input_config.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { ROWS = 51, WORDS = 17, BYTES = 0xd8c };

static BOOL location(char *path, const void *object, char *section)
{
    DWORD length = GetModuleFileNameA(NULL, path, MAX_PATH);
    char *end = length && length < MAX_PATH ? strrchr(path, '\\') : NULL;
    const char *name = (const char *)object + 0xd9c;
    size_t n = 0;
    /* Registry value names become INI sections. Refuse malformed names. */
    while (n < 64 && name[n]) {
        unsigned char c = (unsigned char)name[n];
        if (!(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z') &&
            !(c >= '0' && c <= '9') && c != ' ' && c != '_' && c != '-')
            return FALSE;
        ++n;
    }
    if (!n || n == 64 || !end || (size_t)(end - path) + sizeof("\\input.cfg.tmp") > MAX_PATH)
        return FALSE;
    memcpy(section, name, n + 1);
    strcpy(end, "\\input.cfg");
    return TRUE;
}

static BOOL valid(const uint32_t data[ROWS][WORDS])
{
    for (unsigned i = 0; i < ROWS; ++i) {
        /* +0x18 key, +0x1c button, +0x20 command kind, +0x24 name in Keylist. */
        if (data[i][3] > 0xffff || data[i][4] > 4 || data[i][5] > 4 ||
            !memchr(&data[i][6], 0, 36))
            return FALSE;
    }
    return TRUE;
}

int __fastcall sqz_input_load(void *object)
{
    char path[MAX_PATH], section[64], key[24], text[180];
    uint32_t data[ROWS][WORDS];
    if (!location(path, object, section) ||
        GetPrivateProfileIntA(section, "Version", 0, path) != 1)
        return 0;
    for (unsigned i = 0; i < ROWS; ++i) {
        snprintf(key, sizeof(key), "Command%02u", i);
        DWORD n = GetPrivateProfileStringA(section, key, "", text, sizeof(text), path);
        if (!n || n >= sizeof(text) - 1)
            return 0;
        char *cursor = text;
        for (unsigned j = 0; j < WORDS; ++j) {
            while (*cursor == ' ') ++cursor;
            /* Exactly eight hex digits per word, with spaces between words. */
            for (unsigned k = 0; k < 8; ++k)
                if (!cursor[k] || !strchr("0123456789abcdefABCDEF", cursor[k]))
                    return 0;
            char *end;
            data[i][j] = strtoul(cursor, &end, 16);
            if (end != cursor + 8 || (*end && *end != ' '))
                return 0;
            cursor = end;
        }
        while (*cursor == ' ') ++cursor;
        if (*cursor) return 0;
    }
    if (!valid(data)) return 0;
    memcpy((BYTE *)object + 0xc, data, BYTES);
    return 1;
}

int __fastcall sqz_input_save(void *object)
{
    char path[MAX_PATH], temporary[MAX_PATH], section[64];
    char contents[12288], *out = contents;
    uint32_t data[ROWS][WORDS];
    if (!location(path, object, section)) return 0;
    memcpy(data, (BYTE *)object + 0xc, BYTES);
    if (!valid(data)) return 0;
    out += sprintf(out, "Version=1") + 1;
    for (unsigned i = 0; i < ROWS; ++i) {
        out += sprintf(out, "Command%02u=", i);
        for (unsigned j = 0; j < WORDS; ++j)
            out += sprintf(out, "%08lx%s", (unsigned long)data[i][j], j + 1 == WORDS ? "" : " ");
        ++out; /* Keep each entry's NUL; profile section ends with two NULs. */
    }
    *out = 0;
    strcpy(temporary, path);
    strcat(temporary, ".tmp");
    if (!DeleteFileA(temporary) && GetLastError() != ERROR_FILE_NOT_FOUND)
        return 0;
    /* Preserve other Keylist sections; never replace a good file with a partial write. */
    if (!CopyFileA(path, temporary, FALSE) && GetLastError() != ERROR_FILE_NOT_FOUND)
        return 0;
    /* CopyFile inherits read-only attributes. Only the scratch copy is writable. */
    if (GetFileAttributesA(temporary) != INVALID_FILE_ATTRIBUTES &&
        !SetFileAttributesA(temporary, FILE_ATTRIBUTE_NORMAL))
        return 0;
    BOOL ok = WritePrivateProfileSectionA(section, contents, temporary);
    /* The all-NULL cache flush returns zero even after a successful disk write. */
    WritePrivateProfileStringA(NULL, NULL, NULL, temporary);
    HANDLE file = ok ? CreateFileA(temporary, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL) : INVALID_HANDLE_VALUE;
    if (file == INVALID_HANDLE_VALUE) ok = FALSE;
    else {
        ok = FlushFileBuffers(file);
        CloseHandle(file);
    }
    if (ok) ok = MoveFileExA(temporary, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    if (!ok) DeleteFileA(temporary);
    return ok != FALSE;
}

static uint32_t fingerprint(const BYTE *p, size_t n)
{
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < n; ++i) hash = (hash ^ p[i]) * 16777619u;
    return hash;
}

int sqz_input_patch_image(BYTE *base, size_t size)
{
    if (size < 0x3ed79 || fingerprint(base + 0x3ec70, 0x73) != 0x3d941a78u ||
        fingerprint(base + 0x3ecf0, 0x89) != 0x1c864157u)
        return -1;
    DWORD protection, unused;
    if (!VirtualProtect(base + 0x3ec70, 0x85, PAGE_EXECUTE_READWRITE, &protection))
        return -2;
    BYTE *entries[] = {base + 0x3ec70, base + 0x3ecf0};
    void *targets[] = {sqz_input_load, sqz_input_save};
    for (unsigned i = 0; i < 2; ++i) {
        uint32_t delta = (uint32_t)(uintptr_t)targets[i] - (uint32_t)(uintptr_t)(entries[i] + 5);
        entries[i][0] = 0xe9;
        memcpy(entries[i] + 1, &delta, 4);
    }
    FlushInstructionCache(GetCurrentProcess(), base + 0x3ec70, 0x85);
    VirtualProtect(base + 0x3ec70, 0x85, protection, &unused);
    return 1;
}

void sqz_input_install(void)
{
    BYTE *base = (BYTE *)GetModuleHandleA(NULL);
    IMAGE_DOS_HEADER *dos = (void *)base;
    if (!base || dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew < 0 || dos->e_lfanew > 4096)
        return;
    IMAGE_NT_HEADERS32 *nt = (void *)(base + dos->e_lfanew);
    if (nt->Signature == IMAGE_NT_SIGNATURE && nt->FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
        sqz_input_patch_image(base, nt->OptionalHeader.SizeOfImage);
}
